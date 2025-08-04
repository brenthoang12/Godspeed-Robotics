The project folder contains eye mechanism voice assistant feature and Arduino code for servos control. 


Below is reserved to modify whisper.cpp CMakeFile
# if (WHISPER_SDL2)
#     find_package(SDL2 REQUIRED)

#     # Define target for common.cpp, common-whisper.cpp
#     add_library(common STATIC
#         examples/common.cpp
#         examples/common-whisper.cpp
#     )

#     target_include_directories(common PUBLIC
#         ${CMAKE_CURRENT_SOURCE_DIR}/examples
#         ${CMAKE_CURRENT_SOURCE_DIR}/src
#         ${CMAKE_CURRENT_SOURCE_DIR}  # for whisper.h
#         ${SDL2_INCLUDE_DIRS}        
#     )

#     target_link_libraries(common PRIVATE whisper)

#     # Define common-sdl target
#     add_library(common-sdl STATIC
#         examples/common-sdl.cpp
#     )

#     target_include_directories(common-sdl PUBLIC
#         ${SDL2_INCLUDE_DIRS}
#     )

#     target_link_libraries(common-sdl PRIVATE ${SDL2_LIBRARIES})

#     set_target_properties(common PROPERTIES FOLDER "libs")
#     set_target_properties(common-sdl PROPERTIES FOLDER "libs")
# endif()