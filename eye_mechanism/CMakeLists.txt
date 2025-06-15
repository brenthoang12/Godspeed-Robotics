cmake_minimum_required(VERSION 3.12)
project("voice_assistant"  C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(WHISPER_SDL2 ON CACHE BOOL "Enable SDL2 support for whisper.cpp" FORCE)

add_subdirectory(modules/whisper.cpp)
add_subdirectory(modules/response)
add_subdirectory(modules/tts)

set(TARGET voice_assistant)
add_executable(${TARGET} voice_assistant.cpp)

target_link_libraries(${TARGET} PRIVATE whisper common common-sdl response tts)