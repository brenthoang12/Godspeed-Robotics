// The end goal is to make this polish with the least amount of code and simplicity of use. 
// There will be no argument taken from commmand but all default option. 
// Amen to that.

#include "common-sdl.h"
#include "common.h"
#include "common-whisper.h"
#include "whisper.h"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

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

    float vad_thold    = 0.6f;
    float freq_thold   = 100.0f;

    bool translate     = false;
    bool no_fallback   = false;
    bool print_special = false;
    bool no_context    = true;
    bool no_timestamps = true;
    bool tinydiarize   = false;
    bool save_audio    = false;
    bool use_gpu       = false;
    bool flash_attn    = false;

    string username = "User: "; // implement later or not
    string language  = "en";
    string model     = "./models/ggml-base.en.bin";
    string fname_out;
};

int main(int argc, char ** argv) {
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

    ofstream fout;
    if (!params.fname_out.empty()) {
        fout.open(params.fname_out);
        if (!fout.is_open()) {
            fprintf(stderr, "%s: failed to open output file '%s'!\n", __func__, params.fname_out.c_str());
            return 1;
        }
    }

    struct whisper_context_params cparams = whisper_context_default_params();
    cparams.use_gpu = params.use_gpu;
    cparams.flash_attn = params.flash_attn;
    struct whisper_context * ctx = whisper_init_from_file_with_params(params.model.c_str(), cparams);

    vector<float> pcmf32(n_samples_30s, 0.0f);
    vector<float> pcmf32_new(n_samples_30s, 0.0f);

    int n_iter = 0;
    bool is_running = true;

    wav_writer wavWriter;
    if (params.save_audio) {
        time_t now = time(0);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", localtime(&now));
        string filename = string(buffer) + ".wav";
        wavWriter.open(filename, WHISPER_SAMPLE_RATE, 16, 1);
    }

    printf("[Start speaking]\n");

    auto t_last = chrono::high_resolution_clock::now();
    const auto t_start = t_last;

    while (is_running) {
        if (params.save_audio) {
            wavWriter.write(pcmf32_new.data(), pcmf32_new.size());
        }

        is_running = sdl_poll_events();
        if (!is_running) break;

        const auto t_now = chrono::high_resolution_clock::now();
        const auto t_diff = chrono::duration_cast<chrono::milliseconds>(t_now - t_last).count();

        if (t_diff < 2000) {
            this_thread::sleep_for(chrono::milliseconds(100));
            continue;
        }

        audio.get(2000, pcmf32_new);

        if (::vad_simple(pcmf32_new, WHISPER_SAMPLE_RATE, 1000, params.vad_thold, params.freq_thold, false)) {
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
        wparams.prompt_tokens = nullptr;
        wparams.prompt_n_tokens = 0;

        if (whisper_full(ctx, wparams, pcmf32.data(), pcmf32.size()) != 0) {
            fprintf(stderr, "%s: failed to process audio\n", argv[0]);
            return 6;
        }

        // printf("\n### Transcription %d START\n\n", n_iter);

        const int n_segments = whisper_full_n_segments(ctx);
        for (int i = 0; i < n_segments; ++i) {
            const char * text = whisper_full_get_segment_text(ctx, i);
            printf("Quan: ");
            printf("%s", text);
            fflush(stdout);
            if (!params.fname_out.empty()) {
                fout << text;
            }
        }

        // add ChatGPT response here * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
        std::this_thread::sleep_for(std::chrono::seconds(2));
        printf("\nTheo: received transcription\n");
        // add ChatGPT response here * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

        
        if (!params.fname_out.empty()) {
            fout << endl;
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


// cmake -B build -DWHISPER_SDL2=ON
// cmake --build build --config Release
// ./build/bin/personal-stream