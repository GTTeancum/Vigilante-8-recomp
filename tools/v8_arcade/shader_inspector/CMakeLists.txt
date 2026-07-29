cmake_minimum_required(VERSION 3.25)
project(v8_arcade_shader_inspector LANGUAGES CXX)

find_package(rexglue 0.8.0 EXACT CONFIG REQUIRED)

add_executable(v8_arcade_shader_inspector main.cpp)
target_compile_features(v8_arcade_shader_inspector PRIVATE cxx_std_23)
target_compile_options(v8_arcade_shader_inspector PRIVATE /utf-8)
target_link_libraries(v8_arcade_shader_inspector PRIVATE rex::runtime)
