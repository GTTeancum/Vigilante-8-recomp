# DLL Overlay Analysis

Each overlay file in `input/{TERRAIN,SHELL}/*.DLL` was pre-relocated
via `tools/dll_preproc.py` at synthetic base `0x80100000`, imported as
a raw `MIPS:LE:32` binary into the v8 Ghidra project, auto-analysed,
and exported here.

## Per-overlay totals

| File           | Image size | Functions | Notes                                    |
|----------------|-----------:|----------:|------------------------------------------|
| AIRGRAVE       |     0x17c4 |         6 | AirGrave level (Crane_1, b17_1, ...)     |
| CANYNLND       |     0x12b0 |         5 | Canyonlands level                        |
| CASNOCTY       |     0x2118 |        13 | Casino City level                        |
| HOOVRDAM       |     0x1b8c |        14 | Hoover Dam level                         |
| OILFIELD       |     0x114c |         9 | Oilfield level                           |
| SANDFACT       |     0x10f4 |         5 | Sand Factory level                       |
| SCRTBASE       |     0x2bd0 |        16 | Secret Base level                        |
| SKIRESRT       |     0x2158 |        12 | Ski Resort (with MarkDllEntry seed)      |
| VALLYFRM       |     0x130c |         8 | Valley Farm                              |
| WILDWEST       |     0x1eb8 |        12 | Wild West                                |
| SHELL          |    0x135a0 |       127 | Shell UI (char select, level select, options) |
| LOAD           |     0x7dac |        51 | Shared loader + MDEC video player        |

**Total recovered**: 278 functions across the 12 overlays after a
second analysis pass with `MarkDllEntry.java` seeding the entry from
the post-relocation pointer at `image[1]`.

## Content type

The terrain DLLs hold **per-object physics + AI** code (destructible
objects, guided projectiles, level-specific events). Example: AIRGRAVE
function `FUN_8010068c` is a tracking-physics loop that reads a target
position from a 0x29-offset pointer, computes the angle via the main
EXE's PSY-Q `ratan2` (call into `func_0x8004ecd4`), clamps the delta,
and applies it to a heading variable -- exactly the pattern we expect
for V8's homing missiles / drone vehicles.

This means **physics IS in the DLLs**, not (just) the main EXE.

## Cross-DLL function calls

Each DLL's `func_0x800XXXXX` references are call-throughs into the main
EXE. Pass 2 must wire those up:
- Build a single Ghidra project where the main EXE and the relocated
  DLLs coexist at their correct VAs, then re-run analysis to resolve
  cross-image calls.
- Or simply rename via a script: any `func_0x8004XXXX` outside the DLL
  base is a main-EXE call, and we already have `analysis/SLUS_005.10/
  functions.json` mapping addresses to names.
