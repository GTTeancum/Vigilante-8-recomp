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

## ~~DAT_80060db4 — naming offset, not table base~~ (RESOLVED 2026-05-20)

**Outcome:** Ghidra's `DAT_80060db4` symbol was a hex
mis-interpretation of the signed-i32 literal `-0x7ff9f84c`, which is
actually the unsigned address `0x800607b4` -- the correct PSY-Q
(sin, cos) LUT base. Verified in
`analysis/dll/CANYNLND/mips/80100940.s` @0x80100a60-0x80100a64:

```
lui   v1, 0x8006
addiu v1, v1, 0x7b4    ; v1 = 0x800607b4  (NOT 0x80060db4)
andi  v0, s2, 0xfff    ; aimIdx & 0xfff
sll   v0, v0, 0x2      ; *4 bytes per (i16, i16) pair
addu  a0, v0, v1
lh    v1, 0x0(a0)      ; sin = base[aimIdx*2 + 0]
```

The cleaned C access pattern `DAT_80060db4[aimIdx*2+0]` was bit-exact
against the MIPS the whole time, *iff* the symbol resolves to the
table base.  The earlier "+0x600 phase bias" hypothesis was wrong; the
+0x600 offset was a coincidence between Ghidra's hex typo (`db4` vs
`7b4`) and an unrelated array stride.

**Fix landed:** the 3 cleaned files (`canynlnd/spawner.c`,
`casnocty/spawner.c`, `casnocty/manhole_tick.c`) now reference
`g_v8_sincostbl` directly with the same index math.  No phase bias,
no OOB.  No-op at runtime (those launchers don't fire during the
current smoke), but correct from the next time they do.

## Item 4: Player-vehicle tick callback (2026-05-20)

**Status:** STATIC ANALYSIS INSUFFICIENT — needs Ghidra-interactive
("Find References To" on the symbol).

**What was tried:**
- Direct lw/sw `0x7d0($zero)` scan of the EXE text: **0 hits**.  The
  symbol `puRam000007d0` lives at very low RAM (kernel scratch area);
  the EXE accesses it via the `gp` register + offset rather than
  $zero-relative.  The PSX EXE header reports `gp = 0`, so the gp
  value is set up at runtime (likely from `SystemCnf` / by PSY-Q boot),
  and tracing it from a non-interactive disassembler is impractical.
- `analysis/SLUS_005.10/xrefs.json` does not have an entry for
  `0x000007d0` or `0x800007d0` -- Ghidra's auto-xref pass didn't
  label the location.
- Scanned MIPS for the alternative pattern `lui rA, 0x80hh; addiu rA,
  rA, lo; sw rA, 0x64(rB)` (writing a function pointer to obj+0x64,
  the per-object tick callback slot).  Found **79 distinct call-back
  registration sites**, with these resolved against the known-
  functions table:

      1 / 79 : 0x8003eab0  Projectile_GravityTick (cleaned)
     78 / 79 : addresses inside Ghidra "gap" regions (functions Ghidra
               did not auto-identify; not in functions.json)

  The 78 unresolved pointers are the tick callbacks for the engine's
  full roster of dynamic objects (AI vehicles, guided projectiles,
  per-level destructibles, debris).  Tying a specific one to the
  PLAYER vehicle requires either:
    - tracing `puRam000007d0 = <alloc>->tickCallback` via Ghidra UI,
    - OR finding the player-spawn code which is plausibly in
      `SHELL.DLL` (out-of-scope per CLAUDE.md).

**Implication for the runtime port:** the `host_vehicle.c` shim
currently sets its own tick callback that calls `Object_GeneralTick`.
This is the engine's *universal movable-object* tick, not the player-
specific tick.  Until the player tick is identified, the host shim's
pad-input -> engine-input-field mapping (vehicle struct offsets
+0x14/0x16/0x1a/0x20/0xa4/0xa6/0xd8) remains a best-effort interpretation
of fields the player-tick would normally populate.
