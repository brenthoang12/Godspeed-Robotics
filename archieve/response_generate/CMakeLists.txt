cmake_minimum_required(VERSION 3.14)
project(my_project)


find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(my_project main.cpp)
target_link_libraries(my_project PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)


