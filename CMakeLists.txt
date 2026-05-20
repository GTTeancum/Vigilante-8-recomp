cmake_minimum_required(VERSION 3.20)
project(v8 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)  # GNU extensions for typeof, statement expressions

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# ----- compiler flags -----
if(MSVC)
    add_compile_options(/W3 /wd4244 /wd4101 /wd4267 /wd4146 /wd4018)
    # Treat the recomp as C even for .c files compiled as C; allow the
    # K&R-ish style and signed/unsigned mismatches inherited from Ghidra.
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wno-unused-variable -Wno-unused-but-set-variable
                        -Wno-unused-function -Wno-pointer-sign -Wno-int-conversion)
endif()

# ----- third party -----
add_subdirectory(third_party)

# ----- v8 game core (decompiled C) -----
file(GLOB_RECURSE V8_CORE_SRC
    src/assets/*.c
    src/gameplay/*.c
    src/physics/*.c
)
# skipped/ contains stub seam docs that are NOT compiled into the binary.

add_library(v8core STATIC ${V8_CORE_SRC})
target_include_directories(v8core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/platform
    ${CMAKE_SOURCE_DIR}/platform/psyq
)
target_link_libraries(v8core PUBLIC psyq_stubs)

# ----- platform layer (PSY-Q stubs + SDL/GL/audio plumbing + main) -----
add_subdirectory(platform)

# ----- the executable -----
add_executable(v8 platform/main.c)
target_link_libraries(v8 PRIVATE v8core v8platform)

if(WIN32)
    set_target_properties(v8 PROPERTIES WIN32_EXECUTABLE FALSE)  # console for logs
endif()

# ----- smoke tests -----
enable_testing()
add_test(NAME selftest COMMAND v8 --selftest)
