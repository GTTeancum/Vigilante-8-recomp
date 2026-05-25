# Phase audit log

Per-phase findings during the autonomous build-up. Each entry is a
divergence between the cleaned src/ code and either (a) the Ghidra
reference (via `#if 0 // GHIDRA REF`) or (b) the runtime behavior.

## Phase 2

### Asset_LoadFile name collision (file_loader.c vs quest_loader.c)
- FUN_80015948 (no-arg, "load previously-opened file") and FUN_80015f80
  (path-arg, "load named file") were both cleaned as `Asset_LoadFile`.
- Symptom: Quest_Load called the no-arg version, which then called
  Asset_OpenFile (panic-stubbed), so Quest.bin never loaded.
- Fix: renamed FUN_80015948 -> Asset_LoadFromOpened. Path-arg version
  lives in platform/host_asset.c and reads from input/ on disk.
- Files touched: src/assets/file_loader.c, platform/host_asset.c
- Status: RESOLVED

### Overlay_Open return type truncated pointers on x64
- main_loop.c had `extern int Overlay_Open(...)` and `int h = ...;`
  then `(h + 4)` and dereferenced. On x64 the returned pointer is
  64 bits; casting through int truncated the high 32 bits.
- Fix: Overlay_Open returns uintptr_t; main_loop.c uses uintptr_t h.
- Also: shell_stub's handle struct needs pack(4) so the function
  pointer field sits at offset 4 (not 8 under default x64 alignment).
- Files touched: src/gameplay/main_loop.c, platform/shell_stub.c
- Status: RESOLVED

## Phase 2/3 boot fixes (2026-05-22)

### src/gameplay/match_state.c — Match_ResetState stubbed
**Finding:** Function writes to DAT_800a28xx..DAT_800a2xxx globals in a loop with
7-uint32_t strides. On PSX these are contiguous GPU display-list RAM (0x800a28a4..0x800a3100).
In the host build they are scattered individual uint32_t globals in panic_stubs.c — writes
at offset+7 go out-of-bounds after first iteration → segfault.
**Fix:** Replaced body with `return;` stub, real body preserved in `#if 0` for Phase 9.
**Confidence:** HUD GPU display-list init is renderer-adjacent, correct to stub for Phase 2.

### src/gameplay/level_load.c — Level_LoadByName stubbed
**Finding:** Calls `Overlay_LoadAndRelocate_Named("Shell\Load.dll")` which routes to
`FUN_80011adc` in panic_stubs.c (no-arg stub returning 0). The real path requires
PSX ISO filesystem (Iso_OpenPath/iRam000006b4) not available on host.
**Fix:** `#if 0` around DLL load body; `(void)` no-op for Phase 2.
**Confidence:** Terrain pre-loaded by Host_TerrainLoad; vehicle by Host_VehicleInit.
Phase 4 will re-enable when ISO CD path is backed by fopen.

### platform/screenshot.c — PPM → PNG via stb_image_write
**Change:** Added stb_image_write.h (MIT) to third_party/; screenshot.c now writes
real PNG. Vertical flip preserved (GL bottom-up → PNG top-down).
**Smoke:** tools/smoke/check_screenshot.py updated to handle both PPM and PNG via
stdlib-only PNG decoder (zlib + filter reconstruction, no PyPI deps).
