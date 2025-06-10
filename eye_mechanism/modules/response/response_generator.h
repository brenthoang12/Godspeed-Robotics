#pragma once

#include <string>
#include <nlohmann/json.hpp>

// Returns an empty string on failure.
std::string load_api_key();

// Sends a prompt to OpenAI's Chat API using the provided API key and message list.
// Returns the raw JSON response as a string.
std::string send_prompt (const std::string& api_key, const nlohmann::json& messages);

// Print respone and update context for message
void print_response(const std::string& json_response, nlohmann::json& messages, bool full_output = false);