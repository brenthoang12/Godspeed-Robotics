find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(tts STATIC speech.cpp)

target_include_directories(tts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(tts
    PUBLIC
    CURL::libcurl
    nlohmann_json::nlohmann_json
)