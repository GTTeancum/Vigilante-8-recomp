# third_party/CMakeLists.txt
#
# Vendored dependencies for the V8 recomp.
#   - glad: OpenGL function loader (header-only + one .c)
#   - miniaudio: single-header audio mixer
#   - SDL2: window/input (sourced via FetchContent or system find)
#
# Strategy:
#   - glad and miniaudio are committed under third_party/{glad,miniaudio}/
#     because they are small and pinned.
#   - SDL2 is fetched via FetchContent at configure time (CMake 3.20+),
#     falling back to find_package if a system install is present.

include(FetchContent)

# ----- glad / gl3w (OpenGL loader) -----
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/gl3w/src/gl3w.c)
    add_library(glad STATIC gl3w/src/gl3w.c)
    target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/gl3w/include)
    target_compile_definitions(glad PUBLIC V8_GL_LOADER_GL3W=1)
    message(STATUS "Using gl3w as OpenGL loader")
elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c)
    add_library(glad STATIC glad/src/glad.c)
    target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)
    target_compile_definitions(glad PUBLIC V8_GL_LOADER_GLAD=1)
else()
    message(STATUS "No GL loader (gl3w or glad) found; renderer will be stub")
    add_library(glad INTERFACE)
endif()

# ----- miniaudio -----
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/miniaudio/miniaudio.h)
    add_library(miniaudio INTERFACE)
    target_include_directories(miniaudio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/miniaudio)
else()
    message(STATUS "miniaudio not vendored yet; audio will be a stub. "
                   "Place miniaudio.h under third_party/miniaudio to enable.")
    add_library(miniaudio INTERFACE)
endif()

# ----- SDL2 -----
# Prefer vendored third_party/SDL2 (the prebuilt VC dev pack); fall
# back to a system find_package if missing.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/SDL2/include/SDL.h)
    set(_SDL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/SDL2)
    add_library(SDL2::SDL2 STATIC IMPORTED GLOBAL)
    set_target_properties(SDL2::SDL2 PROPERTIES
        IMPORTED_LOCATION ${_SDL_ROOT}/lib/x64/SDL2.lib
        INTERFACE_INCLUDE_DIRECTORIES ${_SDL_ROOT}/include
        INTERFACE_LINK_LIBRARIES "user32;gdi32;winmm;imm32;ole32;oleaut32;version;uuid;advapi32;setupapi;shell32"
    )
    add_library(SDL2::SDL2main STATIC IMPORTED GLOBAL)
    set_target_properties(SDL2::SDL2main PROPERTIES
        IMPORTED_LOCATION ${_SDL_ROOT}/lib/x64/SDL2main.lib
        INTERFACE_INCLUDE_DIRECTORIES ${_SDL_ROOT}/include
    )
    set(SDL2_RUNTIME_DLL ${_SDL_ROOT}/lib/x64/SDL2.dll CACHE INTERNAL "")
    message(STATUS "Using vendored SDL2 from third_party/SDL2")
else()
    find_package(SDL2 QUIET)
    if(SDL2_FOUND)
        message(STATUS "Using system SDL2: ${SDL2_LIBRARIES}")
    else()
        message(STATUS "SDL2 not found; build without window support")
    endif()
endif()
