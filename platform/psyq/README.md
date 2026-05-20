# platform/psyq

Stub implementations of PSY-Q SDK functions referenced by the
decompiled v8core. Phase 0 ships empty/trivial bodies so the project
*links*. Phase 1 fills in real semantics for the in-scope subset
(libgte math, libcd file I/O); the renderer/audio/controls primitives
stay stubbed because they're being rewritten anyway (per CLAUDE.md
out-of-scope).

Files:
- `libgs_stub.c`   — drawing env / OT chain / primitive emission
- `libgte_stub.c`  — GTE coprocessor instructions (math; Phase 1 real)
- `libcd_stub.c`   — CD I/O (Phase 1 backs by host fopen/fread)
- `libapi_stub.c`  — VSync, RCnt, ResetGraph, Spu*, misc
- `psyq_types.h`   — DRAWENV, DISPENV, MATRIX, VECTOR, RECT, DR_ENV
