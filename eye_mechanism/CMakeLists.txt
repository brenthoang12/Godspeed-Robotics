cmake_minimum_required(VERSION 3.12)
project("voice_assistant"  C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(WHISPER_SDL2 ON CACHE BOOL "Enable SDL2 support for whisper.cpp" FORCE)

if (WHISPER_SDL2)
    find_package(SDL2 REQUIRED)

    # Define target for common.cpp, common-whisper.cpp
    add_library(common STATIC
        modules/whisper.cpp/examples/common.cpp
        modules/whisper.cpp/examples/common-whisper.cpp
    )

    target_include_directories(common PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/whisper.cpp/examples
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/whisper.cpp/src
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/whisper.cpp  # for whisper.h
        ${SDL2_INCLUDE_DIRS}        
    )

    target_link_libraries(common PRIVATE whisper)

    # Define common-sdl target
    add_library(common-sdl STATIC
        modules/whisper.cpp/examples/common-sdl.cpp
    )

    target_include_directories(common-sdl PUBLIC
        ${SDL2_INCLUDE_DIRS}
    )

    target_link_libraries(common-sdl PRIVATE ${SDL2_LIBRARIES})

    set_target_properties(common PROPERTIES FOLDER "libs")
    set_target_properties(common-sdl PROPERTIES FOLDER "libs")
endif()

add_subdirectory(modules/whisper.cpp)
add_subdirectory(modules/response)
add_subdirectory(modules/tts)

set(TARGET voice_assistant)
add_executable(${TARGET} voice_assistant.cpp)

target_link_libraries(${TARGET} PRIVATE whisper common common-sdl response tts)