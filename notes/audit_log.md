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
