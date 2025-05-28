#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <csignal>
#include <ctime>
#include <iomanip>
#include <vector>


using json = nlohmann::json;

bool running = true;

void signal_handler(int signal) {
    if (signal == SIGINT) { 
        std::cout << "\nCtrl+C detected. Exiting gracefully.\n";
        running = false;
        std::cin.setstate(std::ios::eofbit); // mark std::cin as EOF to break getline
    }
}

std::string load_api_key() {
    const char* key = std::getenv("OPENAI_API_KEY");
    if (key) return std::string(key);

    std::ifstream file(".env");
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find("OPENAI_API_KEY=");
        if (pos != std::string::npos) 
            return line.substr(pos + std::string("OPENAI_API_KEY=").size());
    }

    std::cerr << "load_api_key(): API key not found in file.\n";
    return "";
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    s->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string send_prompt(const std::string& api_key, const json& messages) {
    std::string response;
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "send_prompt(): unable to initialize curl.\n";
        return "";
    }

    json payload = {
        {"model", "gpt-4o-mini"},
        {"messages", messages}
    };
    std::string payload_str = payload.dump();

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}

void log_tokens(const json& response) {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    std::ofstream file("token_used.txt", std::ios::app);
    if (file.is_open()) {
        file << "==== New Request ====\n";
        file << "Timestamp: " << std::put_time(now, "%Y-%m-%d %H:%M:%S") << "\n";
        // modified to not create new variables
        if (response.contains("usage")) {
            file << "Prompt Tokens: " << response["usage"].value("prompt_tokens", 0) << "\n";
            file << "Completion Tokens: " << response["usage"].value("completion_tokens", 0) << "\n";
            file << "Total Tokens: " << response["usage"].value("total_tokens", 0) << "\n";
        } else {
            file << "log_tokens(): no token usage info available.\n";
        }
        file << std::endl;
        file.close();
    } else {
        std::cerr << "log_tokens(): can't open \"token_used.txt\".\n";
    }
}

void chat_loop(const std::string& api_key) {
    json messages = json::array({
        {{"role", "system"}, {"content", "You are a helpful assistant."}}
    });
    std::cout << "Start chatting! (Ctrl+C to exit)\n";

    while (running) {
        std::cout << "\nYou: ";
        std::string user_input;
        if (!std::getline(std::cin, user_input) || user_input.empty()) {
            if (!running) break; // handle Ctrl+C triggered
            continue;
        }

        messages.push_back({{"role", "user"}, {"content", user_input}});
        std::string json_response = send_prompt(api_key, messages);

        if (!json_response.empty()) {
            try {
                json parsed = json::parse(json_response);

                if (parsed.contains("choices") && !parsed["choices"].empty()) {
                    if (parsed["choices"][0]["message"].contains("content") &&
                        !parsed["choices"][0]["message"]["content"].is_null()) {

                        std::string content = parsed["choices"][0]["message"]["content"];
                        messages.push_back({{"role", "assistant"}, {"content", content}});
                        std::cout << "Assistant: " << content << std::endl;

                        log_tokens(parsed);
                    } else {
                        std::cerr << "chat_loop(): assistant response is empty.\n";
                    }
                } else {
                    std::cerr << "chat_loop(): unexpected OpenAI response format.\n";
                }
            } catch (json::parse_error& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            } catch (std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        } else {
            std::cerr << "chat_loop(): json_response is empty.\n";
        }
    }
}

int main() {
    std::signal(SIGINT, signal_handler);

    std::string api_key = load_api_key();
    if (api_key.empty()) return 1;

    chat_loop(api_key);

    return 0;
}




// cmake -B build
// cmake --build build
// ./build/my_project
// Let's say you stuck in a falling elevator. There's been saying that if you wait just before the elavator hit the ground, you step outside and you will survive the crash. Why wouldn't it work?

// chat_loop(): [json.exception.type_error.302] type must be string, but is null
// Say 'Hi', if you receive this message. 

