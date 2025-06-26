// TODO: increase vad_thold for more silence tolerant

#include "response_generator.h"
#include "speech.h"
#include "common-sdl.h"
#include "common.h"
#include "common-whisper.h"
#include "whisper.h"

#include <nlohmann/json.hpp>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using json = nlohmann::json;

// command-line parameters
struct whisper_params {
    int32_t n_threads  = 6;
    int32_t step_ms    = 0;
    int32_t length_ms  = 30000;
    int32_t keep_ms    = 200;
    int32_t capture_id = -1;
    int32_t max_tokens = 0;
    int32_t audio_ctx  = 0;
    int32_t beam_size  = -1;

    float vad_thold    = 0.55f;
    float freq_thold   = 100.0f;

    bool translate     = false;
    bool no_fallback   = false;
    bool print_special = false;
    bool no_context    = false;
    bool no_timestamps = true;
    bool tinydiarize   = false;
    bool save_audio    = false;
    bool use_gpu       = false;
    bool flash_attn    = false;

    std::string language  = "en";
    std::string model     = "./models/ggml-base.en.bin";
    std::string fname_out;
};

int main(int argc, char ** argv) {
    bool limit_messages = false;
    std::string api_key = load_api_key();
    json messages = json::array({
        {{"role", "system"}, {"content", "You are a helpful voice assistant. Reply concisely, using short spoken sentences suitable for reading aloud."}}
    });

    whisper_params params;

    const int n_samples_step = (1e-3 * params.step_ms) * WHISPER_SAMPLE_RATE;
    const int n_samples_len  = (1e-3 * params.length_ms) * WHISPER_SAMPLE_RATE;
    const int n_samples_30s  = (1e-3 * 30000.0) * WHISPER_SAMPLE_RATE;

    audio_async audio(params.length_ms);
    if (!audio.init(params.capture_id, WHISPER_SAMPLE_RATE)) {
        fprintf(stderr, "%s: audio.init() failed!\n", __func__);
        return 1;
    }

    audio.resume();

    struct whisper_context_params cparams = whisper_context_default_params();
    cparams.use_gpu = params.use_gpu;
    cparams.flash_attn = params.flash_attn;
    struct whisper_context * ctx = whisper_init_from_file_with_params(params.model.c_str(), cparams);

    std::vector<float> pcmf32(n_samples_30s, 0.0f);
    std::vector<float> pcmf32_new(n_samples_30s, 0.0f);

    std::vector<whisper_token> prompt_tokens;

    int n_iter = 0;
    bool is_running = true;

    printf("[Start speaking]\n");

    auto t_last = chrono::high_resolution_clock::now(); 
    const auto t_start = t_last;

    while (is_running) {

        is_running = sdl_poll_events();
        if (!is_running) break;

        const auto t_now = chrono::high_resolution_clock::now();
        const auto t_diff = chrono::duration_cast<chrono::milliseconds>(t_now - t_last).count();

        if (t_diff < 2000) {
            this_thread::sleep_for(chrono::milliseconds(100));
            continue;
        }

        audio.get(2000, pcmf32_new);

        if (::vad_simple(pcmf32_new, WHISPER_SAMPLE_RATE, 1500, params.vad_thold, params.freq_thold, false)) {
            audio.get(params.length_ms, pcmf32);
        } else {
            this_thread::sleep_for(chrono::milliseconds(100));
            continue;
        }

        t_last = t_now;

        whisper_full_params wparams = whisper_full_default_params(params.beam_size > 1 ? WHISPER_SAMPLING_BEAM_SEARCH : WHISPER_SAMPLING_GREEDY);
        wparams.print_progress = false;
        wparams.print_special = params.print_special;
        wparams.print_realtime = false;
        wparams.print_timestamps = false;
        wparams.translate = params.translate;
        wparams.single_segment = true;
        wparams.max_tokens = params.max_tokens;
        wparams.language = params.language.c_str();
        wparams.n_threads = params.n_threads;
        wparams.beam_search.beam_size = params.beam_size;
        wparams.audio_ctx = params.audio_ctx;
        wparams.tdrz_enable = params.tinydiarize;
        wparams.temperature_inc = params.no_fallback ? 0.0f : wparams.temperature_inc;
        
        wparams.prompt_tokens    = prompt_tokens.data();
        wparams.prompt_n_tokens  = prompt_tokens.size();


        if (whisper_full(ctx, wparams, pcmf32.data(), pcmf32.size()) != 0) {
            fprintf(stderr, "%s: failed to process audio\n", argv[0]);
            return 6;
        }

        std::string full_text;
        const int n_segments = whisper_full_n_segments(ctx);
        for (int i = 0; i < n_segments; ++i) {
           full_text += whisper_full_get_segment_text(ctx, i);
        }

        full_text.erase(full_text.find_last_not_of(" \n\r\t")+1); 

        printf("User: %s", full_text.c_str());
        fflush(stdout);

        if (full_text.find("Reset context") != std::string::npos) { 
            prompt_tokens.clear();
            printf("[Context cleared at user request]\n");
        }

        // context retention
        std::vector<whisper_token> tokens_temp(full_text.size() + 16);
        int n_prompt_tokens = whisper_tokenize(ctx, full_text.c_str(), tokens_temp.data(), tokens_temp.size());

        if (n_prompt_tokens > 0) {
            // limit context size to avoid overflow (e.g., last 256 tokens)
            const int MAX_CONTEXT_TOKENS = 256;
            if (prompt_tokens.size() + n_prompt_tokens > MAX_CONTEXT_TOKENS) {
                // Keep only the most recent tokens within the limit
                int excess = prompt_tokens.size() + n_prompt_tokens - MAX_CONTEXT_TOKENS;
                prompt_tokens.erase(prompt_tokens.begin(), prompt_tokens.begin() + excess);
            }
            prompt_tokens.insert(prompt_tokens.end(), tokens_temp.begin(), tokens_temp.begin() + n_prompt_tokens);
        }
          
        if (!full_text.empty()) {
            printf("\n[Waiting for OpenAI response...]\n");
            fflush(stdout);
            messages.push_back({{"role", "user"}, {"content", full_text}});
            std::string json_respo  nse = send_prompt(api_key, messages);
            std::string content = print_response(json_response, messages);
            tts_openai(api_key, content);
            if (limit_messages) {
                modify_messages(messages);
            }
        }

        // printf("\n### Transcription %d END\n", n_iter);
        printf("\n");

        ++n_iter;
        fflush(stdout);

        audio.clear();
    }

    audio.pause();
    whisper_free(ctx);
    printf("Bye bye.\n");
    return 0;
}

// ./build/voice_assistant