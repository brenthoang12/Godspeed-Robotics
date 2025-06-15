#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <filesystem> 
#include <vector>


using json = nlohmann::json;

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

std::string send_prompt (const std::string& api_key, const json& messages) {
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

std::string get_dated_filename() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);    

    std::ostringstream oss;
    oss << std::put_time(now, "%b-%d-%Y_%H-%M-%S") << "_response.json";
    return oss.str();
}

void save_jsons(const json& parsed) {
    std::string folder = "json_output";
    std::filesystem::create_directories(folder);  // Ensure the folder exists
    
    std::string filename = folder + "/" + get_dated_filename();

    std::ofstream out(filename);
    if (out.is_open()) {
        out << parsed.dump(2);
        out.close();
    } else {
        std::cerr << "save_jsons(): failed to open file for writing: " << filename << "\n";
    }
}


std::string print_response(const std::string& json_response, json& messages, bool full_output = false) {
    if (!json_response.empty()) {
        try {
            json parsed = json::parse(json_response);

            if(full_output) {
                save_jsons(parsed);
            }

            if (parsed.contains("choices") && !parsed["choices"].empty()) {
                if (parsed["choices"][0]["message"].contains("content") &&
                    !parsed["choices"][0]["message"]["content"].is_null()) {

                    std::string content = parsed["choices"][0]["message"]["content"];
                    messages.push_back({{"role", "assistant"}, {"content", content}}); 
                    std::cout << "Assistant: " << content << std::endl;
                    return content;
                } else {
                    std::cerr << "chat_loop(): assistant response is empty.\n";
                }    
            } else {
                std::cerr << "chat_loop(): unexpected OpenAI response format.\n";
            }
        } catch (json::parse_error& e) {
            std::cerr << "chat_loop(): JSON parse error " << e.what() << std::endl;
        } catch (std::exception& e) {
            std::cerr << "chat_loop(): error " << e.what() << std::endl;
        }
    } else {
        std::cerr << "chat_loop(): json_response is empty.\n";
    }
}

// EXPERIMENTAL
void modify_messages(json& messages) {
    if (!messages.is_array()) {
        std::cerr << "modify_json(): messages is not an array\n";
        return;
    }

    json modified = json::array();

    // Always keep the first element (system prompt)

    if (!messages.empty()) {
        modified.push_back(messages[0]);
    } else if (messages.empty()) {
        std::cerr << "modify_json(): messages is empty\n";
        return;
    }

    // Keep the last 10 entries from the conversation (excluding system prompt)
    int start = std::max(1, static_cast<int>(messages.size()) - 10);
    for (int i = start; i < messages.size(); ++i) {
        modified.push_back(messages[i]);
    }

    messages = modified;      
}