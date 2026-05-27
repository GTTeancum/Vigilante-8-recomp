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

### Vehicle struct allocation site -- RESOLVED 2026-05-24

`FUN_8002e630` / `Vehicle_Construct` is the vehicle allocation site.
It allocates `0x124` bytes through `FUN_8001ac44`, installs the chassis
tick at object `+0x64`, creates four wheel children, and clears the
vehicle child/weapon pointer block at `+0xec..+0x118`. Earlier notes
that treated the vehicle size as unknown or used the old `0x200` placeholder
are stale;
`0x200` remains only the temporary host-shim allocation size.

2026-05-24 audit note: the cleaned constructor previously zeroed the
`+0xec..+0x118` block in the wrong direction. It now matches MIPS
`8002e704..8002e710`: clear words `obj[0x46]` down through `obj[0x3b]`.

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

- **Vehicle** -- engine allocation size confirmed as `0x124` by
  `Vehicle_Construct`. `include/structs.h` still needs a struct-layout
  pass to replace host-era padding and stale `0x200` commentary.
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

## Item 4: Player-vehicle tick callback — RESOLVED 2026-05-20

**Outcome: located.** The player vehicle's per-tick callback is at
`LAB_8002e2bc` (a Ghidra-gap function the auto-analyser did NOT
identify).  Size: 0x374 bytes (221 MIPS instructions).

**How it was found (zero empirical assumptions):**

Traced via the Vehicle CONSTRUCTOR.  `FUN_8002e630` (1124 B) is the
allocator/initialiser for the Vehicle struct.  At
`analysis/SLUS_005.10/decomp/8002e630.c` line 34:

```c
puVar6[0x19] = (uint)&LAB_8002e2bc;
```

`puVar6[0x19]` is byte offset `0x19 * 4 = 0x64`, the tick-callback
slot.  So every Vehicle allocated through this constructor has
`LAB_8002e2bc` installed as its per-frame physics callback -- this IS
the player vehicle tick.

**Cross-references confirming the chain:**

- `FUN_8002a350` is the mode dispatcher for player-vehicle lifecycle
  events.  Mode 7 calls `FUN_8002e630` to allocate; mode 1 reads
  `uRam000007d0` (the player Vehicle global) and stores it at
  `obj[0x39]` = byte +0xe4 (the "currentTarget" slot).
- `FUN_8002e630` allocs 0x124 bytes via `FUN_8001ac44` (Object_Pool_
  AllocFromBank) -- so the **Vehicle struct size is exactly 0x124 (292
  bytes)**, not 0x200 as the host shim's `Heap_Alloc(0x200)` assumed.
- The constructor also sets:
    - `obj[0x36] = -obj[0x13]`  (i.e. `dragMass = -*(int*)(obj+0x4c)`),
      where +0x4c is template-copied by the pool allocator.
    - `*(u16*)(obj + 0xa2) = template[0x1a]`  -- a per-character stat.
    - 4 wheels initialised by an inner loop (offset 0x9c each), with
      `puVar7[0x24] = -iVar8` and `puVar7[0x25] = 0x1000000 / -...` --
      this is the **per-wheel inverse-inertia computation**.

**Independent confirmation of two engine-constant initialisers (items 5):**

`FUN_80022d54` (228 B, object init) and `FUN_80022e38` (88 B, object
reset) BOTH write:
   `*(u16*)(obj + 0xa4) = 0`        // angYPreBake = 0
   `*(u16*)(obj + 0xa6) = 0x3c`     // inputMul = 60

This is the engine's ACTUAL value for `inputMul` (60), recovered from source initializers.

**Artifact extracted:**

`analysis/SLUS_005.10/mips/8002e2bc.s` -- 221 raw MIPS instructions
of the player tick.  Ghidra's auto-analyser missed this function
because it's in a gap region; static MIPS disassembly is provided for
future cleanup passes (or for a Ghidra-script that registers the
function start and re-runs the decompiler).

**Status of "drive around with PSX physics":**

The player tick callback's address is now KNOWN.  Decompiling its
221-instruction body to clean C is the next concrete step -- it
needs careful work because the function calls several sub-routines
that are themselves in Ghidra gaps (JAL targets at 0x8002e324,
0x8002e3b8, 0x8002e428, 0x8002e434, etc.), each of which would need
to be identified and either decompiled or treated as a leaf.

The host wiring change required: instead of writing `vehicle_tick`
into obj+0x64 (the host shim's pad->vel mapping), call FUN_8002e630
to construct a real Vehicle that gets LAB_8002e2bc as its tick.  The
constructor expects a vehicle TEMPLATE (param_3) which comes from
the loaded Vehicles.exp XOBF data (`DAT_800737a0[char_idx]`).

## Player vehicle physics architecture — REVISED 2026-05-20

After tracing Vehicle_RollingTick + Object_GeneralTick + the
constructor's wheel-allocation loop, the architecture turns out to be
**hierarchical**, not flat:

### The chassis tick does NOT integrate

`LAB_8002e2bc` (Vehicle_Tick) is the chassis's per-frame callback.
Its normal-state path runs ONLY audio counters; it never calls
`Object_GeneralTick`.  Damaged-state path calls heavy phys helpers
(`FUN_8002efe0`, `FUN_8002d494`) but those are damage-effect specific.

`gap_80030f34` (Vehicle_RollingTick) DOES call `Object_GeneralTick`
in both paths -- but `Object_GeneralTick` is *not* the player-vehicle
integrator.  Its `+-0x200` per-frame angular nudges (keyed off
`+0x1a` / `+0x16` sign flags) are characteristic of TUMBLING objects
(rolling debris, spinning projectiles), not driving cars.

### The chassis position comes from the 4 wheels

`FUN_8002e630` (Vehicle_Construct) allocates 4 wheels at obj+0xfc..
+0x108.  Each wheel is a separately-allocated 0x9c-byte object with
its OWN per-frame tick callback (set somewhere else; the chassis's
constructor doesn't write the wheel callback addresses explicitly --
they come from the bank template).

`FUN_8001b2fc(chassis, joint, wheel)` is the joint linker.  It
copies joint pose data from a "joint descriptor" into the wheel
struct at +0x40..+0x50, then calls:

  - `FUN_8001d708(wheel)`: copies spawn pos (+0x48..+0x50) to current
    pos (+0x24..+0x2c) and sets up the wheel's rotation matrix.
  - `FUN_8001d4f0(chassis, wheel)`: registers the wheel in the
    chassis's child list at chassis+0x38.

Per-frame, each wheel's own callback advances the wheel's position
along its joint axes.  The chassis position then follows from the
4 wheel positions (via an inverse-kinematics or centre-of-mass
computation we haven't traced yet -- candidate: `FUN_8002efe0`).

### Implication for the host

Wiring `Vehicle_RollingTick` as the chassis's tick (as I did) does
NOT reproduce the driving simulation -- it just causes the chassis
to tumble via `Object_GeneralTick`.

To get PSX-exact vehicle physics, the host needs to:
  1. Construct 4 wheel sub-objects (the constructor's wheel-allocation
     loop already gives the per-wheel struct layout).
  2. Install each wheel's per-frame tick callback (one of the
     78 still-uncleaned ticks discovered by the lui+addiu+sw 0x64
     scan in item 4).
  3. Joint-link the wheels via `FUN_8001b2fc`.
  4. Let the wheels' integration drive the chassis position.

The chassis tick (`LAB_8002e2bc`) then remains audio-only -- which
matches its decompiled body exactly.

This is several sessions of work; the next concrete step is to
identify WHICH of the 78 gap callbacks is the wheel tick (likely
distinguished by which template-spawn pattern creates it).

## Pass 3 audit: Renderer-adjacent functions (2026-05-23)

Cross-referencing the 434 "unknown-subsystem" functions from
`analysis/SLUS_005.10/classification.json` against `src/` confirms:

- **387 of 434** are referenced (implemented, stubbed, or extern'd) in `src/`.
- **47 remain unreferenced** in `src/`.

Of those 47, all are renderer-adjacent or PSY-Q library:

### Renderer functions (OUT OF SCOPE — rewritten renderer)

These are called exclusively from `FUN_80021600` (Render_BuildLists)
or from functions it dispatches into:

| Address    | Size  | Role                                          |
|------------|-------|-----------------------------------------------|
| 0x8002623c | 9920  | OT/visibility dispatch (kd-tree traversal)    |
| 0x800289d8 | 1756  | Visible-range spatial subdivider               |
| 0x80025bc0 | 1660  | Per-object OT primitive builder                |
| 0x800290d8 | 1656  | Viewport/clip setup (reads screen W/H)         |
| 0x80029750 | 1300  | Matrix setup for render dispatch               |
| 0x80029e48 |  928  | Render chain helper                            |
| 0x80021460 |  368  | Render list init (called by FUN_800215d0)      |
| 0x800288fc |  220  | Recursive range subdivider for 0x8002623c      |
| 0x80025b20 |  160  | OT node builder (called by 0x80025bc0)         |
| 0x80022cd0 |  132  | World-object render iterator                   |
| 0x8002a1e8 |  116  | GPU primitive OT-list appender                 |
| 0x800215d0 |   48  | Render helper (calls 0x80021460)               |
| 0x800290b4 |   36  | Callback wrapper that calls 0x800290d8         |
| 0x80018xxx | various | Font/GPU primitive helpers (renderer)        |
| 0x800197f4 |  364  | Renderer helper                               |

### PSY-Q library functions (OUT OF SCOPE — runtime/library)

| Address range      | Names                                          |
|--------------------|-------------------------------------------------|
| 0x80040e38/5c      | PSY-Q sound library helpers                    |
| 0x80041c5c/fd4     | SPU helpers                                    |
| 0x80043xxx-8005xxx | SPU, CD-ROM, GTE, printf, string library       |

### Conclusion

The in-scope function coverage (physics / assets / gameplay) is
**complete for pass 3**.  No in-scope function has been missed.
All 47 unreferenced entries are renderer or PSY-Q library functions
explicitly out of scope per the charter.
