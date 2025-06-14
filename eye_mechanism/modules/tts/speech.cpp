#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

using json = nlohmann::json;

void tts_openai(const std::string& api_key, const std::string& text, const std::string& voice = "echo") {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "tts_openai(): unable to initialize curl";
        return;
    }

    // Prepare JSON payload
    json payload = {
        {"model", "tts-1"}, // may change to o4-mini-tts
        {"input", text},
        {"voice", voice} // "alloy", "echo", "fable", "onyx", "nova", "shimmer"
    };
    std::string payload_str = payload.dump();
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Prepare for binary data (audio)
    std::string audio_filename = "tts_output.mp3";
    FILE* audio_file = fopen(audio_filename.c_str(), "wb");
    if (!audio_file) {
        std::cerr << "tts_openai(): failed to open file for writing audio.\n";
        curl_easy_cleanup(curl);
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/audio/speech");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL); // Default fwrite
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, audio_file);

    CURLcode res = curl_easy_perform(curl);
    fclose(audio_file);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "tts_openai(): CURL error: " << curl_easy_strerror(res) << std::endl;
        return;
    }

    // Play the MP3 file using afplay (macOS)
    std::string play_cmd = "afplay " + audio_filename;
    system(play_cmd.c_str());
}



int main() {
    std::string example0 = "Here are your appointments for today: At 9 AM, a meeting with Dr. Lee. At 2 PM, a project review with your team. And at 5 PM, a call with Sarah.";
    std::string example1 = "Sure, I can do that for you.";


    return 0;
}