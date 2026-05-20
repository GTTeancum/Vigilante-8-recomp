# Unknowns

Functions and structs that could not be fully RE'd. Updated at the
end of pass 2.

## Status snapshot at end of pass 2

The "277 auto-stubs" originally listed are **all resolved** -- every
in-scope function has a hand-cleaned C file in `src/`. The list below
is the residual set, separated by what's *deliberately* out of scope
(safe to leave) and what's actually still UNKNOWN.

## Resolved during pass 2 (was UNKNOWN, now HIGH)

- **`V8_RandNext()`** -- located at FUN_800171f0; bit-exact xorshift
  on `g_rngSeed` and `g_rngCounter`. See `src/gameplay/rng.c`.
  Closes the critical 1:1 contract for AI/weapon/quest RNG.

- **15 cross-DLL call targets in main-EXE gaps** -- recovered via
  `tools/ghidra_scripts/MarkAndDisassembleGaps.java` (run during
  pass 2). The 15 promoted addresses are now in `rename_log.md`:
    - 0x8001fd9c -> Spawner_FindSlot
    - 0x80021888 -> HD_PathLookup
    - 0x80022320 -> Damage_StandardVehicle
    - 0x8002239c -> Damage_FromImpulse
    - 0x80022c54 -> Object_Pool_Alloc2
    - 0x80024718 -> Path_ArmAt
    - 0x80042724 -> Path_Sample
    - 0x80042cdc -> Path_NearestWaypoint
    - 0x8002185c -> Damage_VsImpactor (the universal hit-test)
    - 0x80021924 -> Damage_VsImpactor_Burst
    - 0x80019e20 -> Font_LayoutWord
    - 0x80019e7c -> Vram_BinPacker_Reset
    - 0x8001a2cc -> Font_MoveCursor
    - 0x8001a4f8 -> Font_DrawLine2D
    - 0x80043224 -> GTE_GetCurrentPos

- **Vehicle struct +0xba/+0xbb (weight/skill)** -- promoted MED-HIGH
  via MatchScore_AppendLine + result_screen.c team-mode delta.

- **Vehicle +0xc (health) + 0xe (maxHealth)** -- promoted HIGH via
  Damage_AccumulateOrFire + XOBF_LoadHealth (LOAD.DLL).

- **Vehicle +0xe4 (currentTarget)** -- promoted HIGH via
  Vehicle_TryAcquireTarget.

## Remaining unknowns (pass 3 active surface)

### Vehicle struct allocation site

Vehicle objects are referenced via the globals `puRam000007d0` (P1)
and `puRam000007d4` (P2). The malloc/init site that fills them is
**not** in any cleaned function -- it likely lives in a static BSS
pool reachable indirectly through the level loader.

Pass 3 task: open Ghidra interactively, look for stores to those two
globals using References > Find Writes To, then trace the producing
function.

### `g_playerSlotIdx` table semantics (DAT_80065674[8])

Eight-byte table iterated 0..7 in `V8_MainLoop` with `1 << slot`
masked into the audio bank mask. Bytes 0 and 1 always contribute;
2..7 only when `UNK_8006567a[i] != 0`. The exact meaning of the
secondary table (and what each byte stores: vehicle ID? player flag?
seat slot?) is still unclear.

Pass 3 task: cross-reference the writes (only from shell DLL).

### Disc-data formats

- **QUEST.BIN** -- HIGH. Full payload schema documented in
  src/assets/quest_loader.c: nCharacters + groups[].{nQuests,recsRel}
  + records[] of 16 bytes ({u8 questId, u8 bannerKind, u16 configA,
  u16 configB, u16 pad, void *fld2, char *bannerText}). Consumer
  wiring confirmed via V8_MainLoop's `cRam00000600 * 0x10` indexing.

- **LOCATNS.BIN** (Shell/Locatns.BIN) -- consumed only by SHELL.DLL
  (FUN_80102bdc, FUN_80103544) which is out-of-scope renderer/UI per
  CLAUDE.md. The rewritten shell can use any format.
- **OPTIONS.BIN** (Shell/Options.BIN) -- consumed only by SHELL.DLL
  (FUN_8010c1c4). Same.
- **CURSOR.bin**   (Shell/Cursor.bin)  -- consumed only by SHELL.DLL
  (FUN_8010c784). Same.

- **MAP.BSD, LEGAL.BS** -- compressed format. Decompressor lives in
  LOAD.DLL but the MDEC streamer is out of scope per CLAUDE.md.
- ***.STR videos** -- out of scope per CLAUDE.md.

### Renderer-adjacent

The renderer is explicitly out-of-scope per CLAUDE.md; the seam
contracts are documented in `src/skipped/renderer.md`. No further
analysis warranted.

## Structs (still partial)

- **Vehicle** -- 22 fields known and confidence-tagged in
  `include/structs.h`. Total size still unknown (likely ~0x140-0x200
  based on access pattern but no alloc site to confirm).
- **DLL overlay header** -- 8-12 u32 fields + string table; reloc
  table layout HIGH (tag 0/1/2/3 = abs32/HI16/abs16/J26). Some early
  fields (image_size, entry, bss?) still tentative -- see
  `notes/formats/dll_format.md`.

## The 0x20000000-range "functions"

Ghidra exposes GTE coprocessor instructions as pseudo-functions at
0x2000xxxx VAs. Stripped from the in-scope queue; declared in
`include/gte.h`.

## DAT_80060db4 — naming offset, not table base (added 2026-05-20)

PSY-Q's interleaved (sin, cos) LUT lives at **`0x800607b4`** in
SLUS_005.10 (4096 entries × 2 i16). Ghidra named the highest-referenced
spot `DAT_80060db4`, which is `+0x600` (entry 384) into the table, not
the base.

Cleaned files that declare `extern int16_t DAT_80060db4[]` and index
into it:
- `src/physics/canynlnd/spawner.c`
- `src/physics/casnocty/spawner.c`
- `src/physics/casnocty/manhole_tick.c`

Their access pattern is `DAT_80060db4[aimIdx * 2 + 0]` with
`aimIdx ∈ [0, 4095]`, which overruns the table by 768 entries when
treated literally. Audit pending: either retarget those references to
`&g_v8_sincostbl[768]` (the +0x600-into-base alias) and accept the
33.75° phase bias is intentional, OR re-read the original MIPS to
confirm the stride/base. Not on the per-tick vehicle integrator path;
the bit-exact rsin/rcos path via `g_v8_sincostbl` is independent and
verified against the EXE byte-for-byte at boot.
