#pragma once

#include <string>

// text-to-speech
// sends a text to OpenAI's Chat API 
// create a mp3 file and use adf in terminal to play the mp3 file
void tts_openai(const std::string& api_key, const std::string& text, const std::string& voice = "alloy");

