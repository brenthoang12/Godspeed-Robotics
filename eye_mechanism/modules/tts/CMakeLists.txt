cmake_minimum_required(VERSION 3.12)
project(speech_tts)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(speech speech.cpp)

target_link_libraries(speech
    PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
)