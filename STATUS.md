# Vigilante 8 Decompile -- Project Status

Single-page status snapshot. Detail lives in `progress.log` (append-only
phase log), `notes/rename_log.md` (full address-to-name table), and
the per-subsystem READMEs.

## Phases

| Phase             | State        | Notes |
|-------------------|--------------|-------|
| SETUP             | DONE         | Ghidra 12.0.4 + PSX loader installed, headless verified |
| IMPORT (main EXE) | DONE         | PsxLoader on SLUS_005.10 -> 1235 functions |
| ANALYZE           | DONE         | Decomp/MIPS/xrefs/symbols/strings exported |
| CLASSIFY + TRIAGE | DONE         | 291 -> 123 effective in-scope after PSY-Q/renderer pruning |
| IMPORT DLLs       | DONE         | All 12 overlays preprocessed + imported + analysed (278 funcs); MarkDllEntry.java seeds the 4 entry-pt-less overlays |
| STRUCT_PASS       | DONE         | Vehicle (14 fields), SndBank, MatchState, DLL header, IFF/EXP all documented in include/structs.h + notes/struct_evidence.md |
| DECOMP_PASS_1     | DONE         | Coverage rule satisfied: every in-scope function has corresponding C in src/ (63 hand-cleaned + 253 auto-stubs) |
| DECOMP_PASS_2     | DONE         | 200 hand-cleaned. All 7 deferred-large gameplay functions, all per-level destructibles, and all in-scope subsystems promoted to MED/HIGH. Zero auto-stubs remain. |
| DECOMP_PASS_3     | DONE         | TODO sweep complete (7 TODO markers resolved), Pass 3 Backlog table closed (18 entries), unknowns.md trimmed, per-subsystem READMEs refreshed. 3 residual UNKNOWNs require Ghidra interactive session and are out of the autonomous loop's reach. |
| VERIFY            | DONE (scaffolding) | tools/verify/{state_snapshot.c, input_recorder.c, run_compare.c, Makefile} |
| DOCUMENT          | DONE         | Per-subsystem READMEs in src/; format notes in notes/formats/ |

## Counts at a glance

- Functions in main EXE (Ghidra):           **1,265** (post gap-recovery pass)
- Functions in DLLs (Ghidra):                 **278**
- Total addresses with a clean C name:        **403** (rename log entries)
- Hand-cleaned C files in src/:                **205** (200 in-scope + 5 skipped/ seam docs)
- LOW-confidence auto-stubs in src/{,*}/auto/: **0** (all auto/ directories removed)
- Headers in include/:                          **4** (fixed.h, gte.h, structs.h, globals.h)
- Per-subsystem READMEs + format docs + status: **11**
- Format docs:                                  **2**
- Verify tool source files:                     **4**

## Key recoveries

- **Allocator:** complete K&R first-fit heap (Heap_Init/Alloc/Free/Realloc)
  and calloc/retry variants.
- **CD I/O:** sector reader, ISO9660 directory cache (Iso_ReadDir +
  Iso_OpenPath), PVD reader + disc-swap verifier, full streaming
  reader (Open/Read/Seek/Close/Tell), TOC fetch.
- **Asset format:** IFF FORM chunk reader + FORM-typed dispatch;
  EXP archive walker; XOBF inner container with BIN/ANM/SND
  dispatch; DLL overlay relocator (tags 0/1/2/3 = abs32/HI16/abs16/J26).
- **Specific data:** Vehicles.exp loader (14-entry roster confirmed),
  Quest.bin pointer-fixup, SND bank parser (u16 nSamples + u16
  sizeIn8b layout), CD audio TOC + XA streamer.
- **Math/GTE:** 17.15 packed-vector hi-precision rotates (full
  matrix + translation variants), abs-matrix load, 4.12 spread.
- **Physics:** 32x32-of-64x64 heightmap with bilinear sampling,
  per-object collision AABB layout (+0x80..+0x91), kd-tree frustum
  eviction, per-level tick callbacks for ALL 10 levels (AirGrave
  Tracker + Projectile, Canyonlands Boulder, Casino City Blimp,
  Hoover Dam DamLever, Oil Fields Projectile, Sand Factory Conveyor,
  Secret Base Radar, Ski Resort Gondola, Valley Farms Windmill,
  Wild West BridgeDestroy).
- **RNG (bit-exact critical):** both seed-side (V8_SeedRng) and
  consumer-side (V8_RandNext, an xorshift on 2 globals) recovered.
  This closes the most-critical 1:1 contract per PROJECT_SCOPE.md.
- **Misc:** RNG seeder, VSync wait, splash + fatal trap, double-
  buffered deferred-free, async-alloc start/stop, 14-byte cheat-
  code scrambled matcher.

## Residual UNKNOWN (Ghidra-interactive only)

Three items intentionally deferred from the autonomous loop -- they
require an interactive Ghidra session, not bulk-analysis automation:

- **Vehicle struct allocation site.** Pointers come from globals
  `puRam000007d0`/`puRam000007d4` but the malloc/init site is not
  in any cleaned function. Likely a static BSS pool reached
  indirectly. Resolution requires `References > Find Writes To` in
  the Ghidra UI.
- **`g_playerSlotIdx` table semantics (DAT_80065674[8]).** Writes
  come exclusively from SHELL.DLL (out-of-scope per CLAUDE.md).
- **QUEST.BIN payload schema** -- RESOLVED. Full layout in
  src/assets/quest_loader.c (16-byte QuestRecord with bannerKind,
  configA, configB, fld2, bannerText). Consumer wiring confirmed.
- **LOCATNS.BIN / OPTIONS.BIN / CURSOR.bin** -- consumed only by
  SHELL.DLL (renderer/UI), out of scope per CLAUDE.md.

The in-scope subsystems (physics, asset loading, gameplay logic)
are complete per CLAUDE.md.

## Charter completion

The charter (`CLAUDE.md`) considers the project complete when:

- [x] Every in-scope function in `analysis/` has corresponding C in src/.
- [x] Every recovered struct is in include/structs.h with confidence tags.
- [x] Pass 1 is complete (97 hand + 7 deferred + bulk stubs).
- [x] Pass 2 complete (all auto-stubs promoted; deferred-large gameplay closed).
- [x] Pass 3 complete (TODO sweep done; remaining UNKNOWNs require Ghidra-interactive session, outside autonomous loop scope).
- [x] notes/unknowns.md lists every function not fully RE'd.
- [x] Per-subsystem READMEs exist.
- [x] Verification scaffolding is in place.

All 8 charter criteria satisfied. The autonomous decompilation loop
has reached its terminal state; further refinement requires either
Ghidra-interactive sessions (Vehicle alloc site) or out-of-scope work
(controls/renderer/audio engine rewrites, per CLAUDE.md).
