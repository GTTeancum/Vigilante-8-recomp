# platform/CMakeLists.txt
#
# The platform layer:
#   - psyq_stubs: implementations of PSY-Q libgs/libgte/libcd/libapi
#     that v8core's extern declarations refer to. Phase 1 fills these in;
#     Phase 0 just has empty bodies that link cleanly.
#   - v8platform: SDL+GL+audio plumbing, smoke-test harness, main entry.

# ----- psyq_stubs -----
file(GLOB PSYQ_SRC psyq/*.c)
add_library(psyq_stubs STATIC ${PSYQ_SRC})
target_include_directories(psyq_stubs PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/platform
)

# ----- v8platform -----
add_library(v8platform STATIC
    platform_init.c
    smoke_harness.c
    host_heap.c
    host_asset.c
    panic_stubs.c
    shell_stub.c
    shell_aliases.c
    heap_shim.c
    sched_shim.c
    vehicle_stub.c
    pad_shim.c
    screenshot.c
    renderer.c
    projectile_shim.c
    ai_opponent.c
    destructibles.c
)
target_include_directories(v8platform PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/platform
)
target_link_libraries(v8platform PUBLIC psyq_stubs)

# Optional: SDL/GL/audio. Linked only when third_party has them and we
# get past Phase 0.
if(TARGET SDL2::SDL2)
    target_link_libraries(v8platform PUBLIC SDL2::SDL2)
    target_compile_definitions(v8platform PUBLIC V8_HAVE_SDL=1)
endif()
if(TARGET glad)
    target_link_libraries(v8platform PUBLIC glad)
    target_compile_definitions(v8platform PUBLIC V8_HAVE_GL=1)
endif()
if(TARGET miniaudio)
    target_link_libraries(v8platform PUBLIC miniaudio)
    target_compile_definitions(v8platform PUBLIC V8_HAVE_MINIAUDIO=1)
endif()
