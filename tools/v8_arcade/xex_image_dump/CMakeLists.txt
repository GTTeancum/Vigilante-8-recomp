cmake_minimum_required(VERSION 3.25)
project(v8_arcade_xex_image_dump LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(rexglue 0.8.0 REQUIRED CONFIG)

add_executable(v8_arcade_xex_image_dump main.cpp)
target_link_libraries(v8_arcade_xex_image_dump PRIVATE rex::runtime)

if(MSVC)
    target_compile_options(v8_arcade_xex_image_dump PRIVATE /utf-8)
endif()

if(WIN32)
    add_custom_command(
        TARGET v8_arcade_xex_image_dump
        POST_BUILD
        COMMAND
            "$<$<BOOL:$<TARGET_RUNTIME_DLLS:v8_arcade_xex_image_dump>>:${CMAKE_COMMAND};-E;copy_if_different;$<TARGET_RUNTIME_DLLS:v8_arcade_xex_image_dump>;$<TARGET_FILE_DIR:v8_arcade_xex_image_dump>>"
        COMMAND_EXPAND_LISTS
        VERBATIM
    )
endif()
