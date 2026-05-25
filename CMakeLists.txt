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
    # Force-include Ghidra placeholder type aliases (undefined4 etc.) into
    # every translation unit.  168 decompiled files use these names; a
    # forced include is cleaner than touching each file individually.
    add_compile_options(/FI "${CMAKE_SOURCE_DIR}/include/ghidra_types.h")
else()
    add_compile_options(-Wall -Wno-unused-variable -Wno-unused-but-set-variable
                        -Wno-unused-function -Wno-pointer-sign -Wno-int-conversion)
    add_compile_options(-include "${CMAKE_SOURCE_DIR}/include/ghidra_types.h")
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

# Files whose bodies are shadowed by platform/ implementations
# (structurally PSX-32-bit; replaced by host-portable versions).
list(REMOVE_ITEM V8_CORE_SRC
    "${CMAKE_SOURCE_DIR}/src/assets/heap.c"
    "${CMAKE_SOURCE_DIR}/src/assets/heap_retry.c"
    "${CMAKE_SOURCE_DIR}/src/assets/heap_retry2.c"
    "${CMAKE_SOURCE_DIR}/src/assets/heap_calloc.c"
)

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
target_link_libraries(v8 PRIVATE v8core v8platform v8core)
set(V8_RUNTIME_DIR "${CMAKE_SOURCE_DIR}/PS1 game")
set_target_properties(v8 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${V8_RUNTIME_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${V8_RUNTIME_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${V8_RUNTIME_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${V8_RUNTIME_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${V8_RUNTIME_DIR}"
)

if(EXISTS "${CMAKE_SOURCE_DIR}/input")
    add_custom_command(TARGET v8 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/input"
            "${V8_RUNTIME_DIR}"
        COMMENT "Copy game assets to PS1 game runtime directory"
    )
endif()

if(MSVC)
    # MSVC moved sprintf et al. into legacy_stdio_definitions.lib post-VS2015;
    # decompiled code declares them as bare externs, so link against it.
    target_link_libraries(v8 PRIVATE legacy_stdio_definitions)
    target_link_options(v8 PRIVATE "/MAP")
endif()

if(WIN32)
    set_target_properties(v8 PROPERTIES WIN32_EXECUTABLE FALSE)  # console for logs

    # Copy SDL2.dll next to the executable in PS1 game so it can run
    # natively from the asset directory.
    if(SDL2_RUNTIME_DLL)
        add_custom_command(TARGET v8 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SDL2_RUNTIME_DLL}"
                "${V8_RUNTIME_DIR}"
            COMMENT "Copy SDL2.dll next to PS1 game/v8.exe"
        )
    endif()
endif()

# ----- smoke tests -----
enable_testing()
add_test(NAME selftest COMMAND v8 --selftest)
