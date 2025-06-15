#pragma once

#include <string>
#include <nlohmann/json.hpp>

// returns an empty string on failure.
std::string load_api_key();

// sends a prompt to OpenAI's Chat API using the provided API key and message list.
// returns the raw JSON response as a string.
std::string send_prompt (const std::string& api_key, const nlohmann::json& messages);

// print response and update context for message
std::string print_response(const std::string& json_response, nlohmann::json& messages, bool full_output = false);

// EXPERIMENTAL
// modify messages to limit input token to improve program processing time
void modify_messages(nlohmann::json& messages);
