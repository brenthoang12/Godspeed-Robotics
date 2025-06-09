find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(response STATIC response_generator.cpp)

target_include_directories(response PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(response
    PUBLIC
    CURL::libcurl
    nlohmann_json::nlohmann_json
)