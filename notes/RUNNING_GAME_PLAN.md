# Plan — Vigilante 8 Recomp to Running Game

**Goal:** Produce a playable build that wires the decompiled C against a
modern renderer / audio / controls layer. Full-match scope: vehicle +
AI opponent + weapons firing + destructibles working. Visual fidelity
target is "whatever's fastest" — solid-colored geometry to start,
texture/dither/affine fidelity bumps deferred. Bit-exactness explicitly
deferred to a later RE pass.

**Stack (locked):** SDL2 + OpenGL 3.3 core + miniaudio + CMake.

---

## Operating mode (locked)

**Fully autonomous.** No clarifying questions, no permission prompts,
no "should I continue?" check-ins. Same protocol as the original
CLAUDE.md operating rules.

- **Decisions:** when ambiguous, pick the most reasonable
  interpretation, log to `decisions.log` with `[timestamp] [phase]
  [chose X over Y because Z]`, continue.
- **Smoke tests:** each phase ends with a defined smoke test (below).
  Pass → next phase. Fail → diagnose and retry. The user is NOT
  consulted on smoke test design or pass/fail interpretation.
- **Progress log:** `progress.log` keeps the per-step append-only
  record (already established convention from CLAUDE.md).
- **Audit log:** `notes/audit_log.md` records per-file findings
  during the line-by-line audit pass.
- **Real blockers (only valid stop conditions for user input):**
  1. A required input file is missing that I can't extract or
     reconstruct (e.g. the CD bin/cue is not the version I expect
     and I can't identify the format).
  2. An unrecoverable third-party tool failure after one retry
     (e.g. CMake refuses to configure with a clear error I can't
     work around).
  3. The plan is complete — running game delivered, smoke tests pass.
  4. A licensing / legality question (e.g. how to redistribute
     decoded VAG samples).

  Nothing else justifies pausing. Hit a confusing Ghidra block?
  Pick the likely interpretation, log it, move on. Vehicle struct
  recovered size turns out wrong? Re-audit the allocation/stride evidence,
  adjust the padding, log it, continue.

- **Commit cadence:** as agreed — per logical unit of work, never
  commit a non-building tree after Phase 1, `phaseN:` prefix in
  subject. Squash `wip:` commits before pushing to a clean point.

### Smoke test failure protocol

Failures are normal in a project this size. Default action is
**diagnose and fix, not escalate.** The protocol:

1. **Classify the failure:**
   - **Hard fail** — segfault, exit ≠ 0, no smoke artifact produced.
   - **Soft fail** — clean run, but checker predicate didn't match
     (e.g. screenshot too dark, position delta too small).
   - **Flake** — passes on second run with no code change. Re-run once
     to confirm; if it then passes 3x in a row, dismiss as flake and
     note in `progress.log`. If it recurs, treat as real fail.

2. **Diagnose:**
   - Hard fail → core dump / address sanitizer / read the log line
     before the crash. Bisect via `git bisect run ./tools/smoke/run_phaseN.sh`
     if the regression range is non-obvious.
   - Soft fail → read what the checker measured vs expected; widen
     instrumentation (more frame log, dump intermediate state).

3. **Fix attempts, bounded:**
   - **Attempt 1:** the obvious fix from the diagnosis. Commit, re-run
     smoke. Log to `notes/audit_log.md`.
   - **Attempt 2:** if attempt 1 missed, broaden the diagnosis (often
     the bug is in a different MED-confidence file than first
     suspected). Re-check Ghidra pseudo-C for any function on the
     suspect path. Commit, re-run.
   - **Attempt 3:** if still failing, restore the relevant
     MED-confidence file from its `auto/` Ghidra-pseudo-C `#if 0`
     block — i.e. roll back the cleanup and use the literal Ghidra
     output (which is verbose but correct). Commit, re-run.
   - **Attempt 4+:** if still failing, the bug is architectural
     (e.g. Vehicle struct field at wrong offset, or a missing
     PSY-Q runtime semantic like GTE saturation). Take it as such:
     widen the search to data-layout / runtime-primitive bugs, not
     just the function I'm looking at.

4. **Loosen the smoke test, never:**
   - Do not lower the bar on a checker to make a phase pass.
     `notes/audit_log.md` gets the rationale if a smoke test is
     re-scoped, but the bar moves only sideways (different signal,
     not weaker signal). Cosmetic-only failures (e.g. exact pixel
     count in a histogram) may be re-scoped; behavioral failures
     (vehicle didn't move, no audio, crash) never are.

5. **Escalate to user (only when):**
   - 6+ failed attempts on the same smoke test with no new diagnosis
     angle. (Genuine architectural dead-end.)
   - Failure points at a file path or data file that doesn't exist
     in the project tree and that I can't reconstruct (missing
     input).
   - Failure can only be resolved by changing a locked decision
     (e.g. "the SDL2 abstraction is fundamentally wrong, need
     native Win32").

   Escalation format: single message summarizing what was tried,
   what the data says, and the proposed unblock. Wait for response,
   then continue.

6. **Phase cannot pass on a wedged retry loop.** If 6+ attempts hit,
   the phase is **blocked**, not "passed with workaround." The
   progress log records the block; work cannot proceed to next phase.

The whole point is: failures are expected, the audit IS the act of
fixing them, and the user is only consulted when I've genuinely
exhausted the autonomous angles.

---

## Approach

Phased build-up. Each phase ends with a smoke test (defined below)
and a concrete runnable artifact. Each phase audits every line
touched per the user's mandate; when a MED-confidence cleanup
misbehaves, the recovery procedure is documented in Phase 0.

The "full match" scope is ambitious because it exercises every
MED-confidence file I wrote during pass 2. The plan assumes a lot of
crash → diff-against-Ghidra → fix cycles. That's not a bug, it's the
audit the user mandated, just done against runtime behavior instead
of static review.

---

## Phase 0 — Foundation (~1 day)

- `git init`; commit current tree as baseline so the audit can use
  `git diff` to find what I changed.
- Recover deleted auto-stubs: every MED-confidence rewrite that
  superseded an `auto/` stub needs the Ghidra pseudo-C re-attached as
  an `#if 0` reference block in the cleaned file. Source:
  `analysis/<dll>/decomp/<addr>.c`. This is the line-level audit
  source-of-truth the user asked for.
- Add `CMakeLists.txt` skeleton.
- Vendor SDL2, miniaudio, glad (single-file deps under `third_party/`).
- Decide C standard: **C11 with GNU extensions** (project uses GCC-ish
  idioms in cleaned code).
- Create `platform/` for the new layer; `include/v8_seams.h` will hold
  the renderer/audio/controls contracts.

**Endpoint:** `cmake -B build && cmake --build build` invokes the
compiler. Doesn't link yet.

**Smoke test:**
- `git log --oneline` shows the baseline commit + Ghidra-ref restoration commits.
- `cmake -B build` succeeds.
- `find third_party -name "*.h" | wc -l` > 0 (vendored deps present).
- At least one cleaned MED-confidence file has its `#if 0` Ghidra reference block re-attached (sample check).

---

## Phase 1 — Compile-pass (1–2 weeks; largest single phase)

Goal: `cmake --build` links successfully. Binary runs and reaches
`V8_MainLoop`; controlled-panic on first missing feature is acceptable.

### 1a — Canonical header

Generate `include/v8_internal.h` with one declaration for every
cross-file function. Process:
1. Grep all `extern` declarations in `src/`.
2. Cluster by name; resolve conflicts by inspecting the actual
   definition (or Ghidra pseudo-C).
3. Move the canonical decl to `v8_internal.h`; replace per-file
   `extern` blocks with `#include "v8_internal.h"`.

This pass alone will expose dozens of signature inconsistencies. Each
mismatch is an audit hit — log to `notes/audit_log.md` with file+line.

### 1b — Vehicle struct

Pad to **0x200 bytes** (max observed access is at +0x150, +0x80 buffer
for safety) with explicit fields at known offsets and padding bytes
between. Mark each known field; flag the pad regions:

```c
typedef struct Vehicle {
    uint32_t inputFlags;          /* +0x00  HIGH */
    uint8_t  pad0_0c[0x0c - 4];
    uint16_t health;              /* +0x0c  HIGH */
    uint16_t maxHealth;           /* +0x0e  HIGH */
    /* ... etc per include/structs.h ... */
    uint8_t  pad_tail[0x200 - 0x158];
} Vehicle;
_Static_assert(sizeof(Vehicle) == 0x200, "Vehicle size");
```

The hunt for the real alloc site (and therefore the real size) is
**deferred** — runtime corruption tells us if 0x200 isn't enough.

### 1c — PSY-Q stubs

Three files under `platform/psyq/`:
- `libgs_stub.c` — `SetDefDrawEnv`, `SetDefDispEnv`, `MargePrim`,
  `ClearOTagR`, `DrawOTag`, `DrawSync`, `PutDispEnv`, `PutDrawEnv`,
  `SetDrawEnv`, `ClearImage`. OT chain is just a `std::vector`-style
  array; primitive types defined as plain structs.
- `libgte_stub.c` — `gte_ldv0`, `gte_RTPS`, `gte_stsxy0/1/2`, all
  `RotMatrix*`, `MulMatrix0`, `MatrixNormal`, `CompMatrixLV`,
  `VectorNormalSS`, `ratan2`. Implement using host i32/i64 math
  faithful to the 4.12 / 17.15 fixed-point semantics in
  `include/fixed.h` and `include/gte.h`. **No saturation modeling
  yet** — note in code where it might matter.
- `libcd_stub.c` — `CdInit`, `CdRead`, `CdControl`, `CdReadSync`,
  `CdIntToPos`. Backed by `fopen`/`fread` against extracted CD files.

### 1d — Extract CD files

One-time tool: `tools/cd_extract.py` reads the `bin-cue/` image,
emits files into `data/` matching the original directory structure
(SHELL/, TRACK/, SOUNDS/, etc.). `CdRead` later reads from `data/`.

### 1e — Out-of-scope subsystems

- `SHELL.DLL` entry: stub returns a hardcoded path
  `"Track\\OilField.TER"` (or whatever level we pick for MVP) so
  `V8_MainLoop` has something to load.
- `Pad_Tick`: skeleton implementation reads SDL keyboard, packs into
  `uRam0000062c` / `uRam00000630` per the bit layout in
  `src/skipped/pad_input.c`.
- `Audio_PlaySfx` / `Audio_PlayXa`: log-and-no-op stubs.
- `Render_*` / all the renderer-adjacent calls in `main_loop.c`: empty
  stubs that record what was queued (we'll consume that later).

### 1f — Cross-DLL resolution

Every `func_0xXXXXXXXX` reference in cleaned code must resolve to
either the cleaned function or a panic stub. `tools/resolve_xrefs.py`
already named most; this pass cleans up the residual.

### 1g — Smoke test harness

Built once, used by every subsequent phase:

- `--selftest` flag in the binary: runs deterministic unit-style
  checks (RNG seed/consume, terrain bilinear at known coords,
  Vehicle struct sizeof). Exits 0 on pass, non-zero on fail.
- `--frames N --headless` flag: run N ticks with no window.
- `--replay <input.log>` flag: feed pre-recorded pad-byte stream
  into Pad_Tick instead of SDL.
- `--screenshot <path.png>`: dump backbuffer at end of run.
- `--audio-capture <path.wav>`: dump mixer output as PCM WAV.
- `--report-heap`: print alloc/free balance on shutdown.
- `tools/smoke/`: Python checkers (`check_screenshot.py`,
  `check_centered.py`, `check_audio.py`) used by smoke tests.
- `tools/smoke/run_phaseN.sh`: per-phase driver script that
  invokes the binary, runs checkers, exits 0 on pass.

Each phase's smoke test reduces to one shell command:
`./tools/smoke/run_phase3.sh` → exit 0 means phase passed.

**Endpoint:** Binary links. `./build/v8` runs, prints "entering main
loop," panics on first unimplemented feature inside `V8_MainLoop`.

**Smoke test:**
- `cmake --build build` exits 0 (links).
- `./build/v8 --selftest` exits 0 — selftest runs heap init, RNG seed,
  one V8_RandNext call against a known seed, terrain bilinear sample
  against a known input. Hardcoded expected values.
- Running `./build/v8` (no args) reaches the "entering main loop"
  log line before any panic.
- `notes/audit_log.md` has entries for every file touched in 1a.

---

## Phase 2 — Boot through main loop without crashing (~3-5 days)

- Get `Heap_Init` running against a static `host_heap[16*1024*1024]`
  buffer.
- Get `Quest.bin` loading via the now-real `CdRead`.
- Get one level's `.TER` loading.
- Get terrain heightmap into memory.
- Get a `Vehicle` allocated (likely via the level loader path).
- All renderer/audio calls remain stubs.
- Run for 60 frames without crashing.

Expect: a torrent of bugs from MED-confidence files. Each one is a
mini-audit: read the cleaned C, read the Ghidra pseudo-C (now restored
as `#if 0` blocks per Phase 0), identify the divergence, fix.

**Endpoint:** Window opens (via SDL even though renderer is stub),
black screen, main loop ticks at 60 Hz, no crashes.

**Smoke test:**
- `./build/v8 --frames 300 --headless` runs 300 frames (5s @60Hz)
  without crashing, exits 0.
- Frame log records ≥ 300 ticks of `V8_MainLoop`.
- Heap leak check: `--report-heap` shows alloc/free balance after
  shutdown (within tolerance, since some frees are deferred).
- Asset load log shows ≥ 1 `.TER`, `Quest.bin`, and `Vehicles.exp`
  loaded successfully.

---

## Phase 3 — Terrain on screen (~3-5 days)

Renderer seam contract:

```c
/* include/v8_seams.h */
typedef struct { float pos[3], col[3]; } RenderVertex;
void Renderer_Init(int w, int h);
void Renderer_BeginFrame(const float view[16], const float proj[16]);
void Renderer_DrawTriangleSoup(const RenderVertex *v, int count);
void Renderer_EndFrame(void);
```

- Implement against OpenGL 3.3: VAO/VBO, single shader (vertex color
  passthrough), depth test on.
- Convert physics camera matrix (Camera_BuildMatrix output, 4.12-fixed
  16-bit matrix) to host float view matrix.
- Tessellate terrain heightmap into triangle soup, color by height
  band (cheap shading).
- Hook the new renderer behind the existing `Render_BuildLists` /
  `Render_PointCameraAt` stubs from Phase 1.

**Endpoint:** Terrain visible. Camera moves with whatever the engine
is doing.

**Smoke test:**
- `./build/v8 --frames 300 --screenshot phase3.png` produces a PNG.
- `phase3.png` is not solid black: pixel histogram has >2 distinct
  color buckets above noise floor.
- Asserted via `tools/smoke/check_screenshot.py` (non-black, non-
  uniform-color predicate).

---

## Phase 4 — Vehicle visible + controllable (~5-7 days)

- Vehicle render: draw as a colored OBB pulled from
  `Vehicle.+0x80..+0x91` hitbox. Solid color per player slot.
- Wire SDL keyboard into the pad bit layout (Phase 1e). Use the bits
  V8_MainLoop actually reads:
  - Up/Down → accel/brake
  - Left/Right → steer
  - Space → fire weapon (`0x8000000`)
  - Enter → start/pause (`0x100`)
- Hook the physics tick (`Physics_Step`, `object_integrate.c`,
  terrain probes) to actually run.
- Camera-follow vehicle via the (now real) Camera_BuildMatrix output.

**Endpoint:** **Semi-believable checkpoint.** Drive a colored box
around terrain. This is the user's "running game" definition met at
minimum scope; the next phases push toward "full match" MVP.

**Smoke test:**
- `./build/v8 --replay tools/smoke/phase4_drive.input --frames 600
  --screenshot phase4.png` runs a recorded input (steer + accel for
  10s) and produces a screenshot.
- Vehicle position delta between frame 0 and frame 600 > 0x10000
  (4.12-fixed world units): proves the input → physics → motion path
  works.
- Camera follow assertion: vehicle within central 60% of screen in
  phase4.png (CV check via tools/smoke/check_centered.py).

---

## Phase 5 — Weapons + projectiles (~3-5 days)

- Wire space → `Pool_AllocProjectile` → projectile spawn.
- Projectile tick callbacks (`FUN_8004042c` and friends) need to
  actually run — these are auto-generated tick callbacks scattered
  across DLLs. Audit each MED-confidence one against its Ghidra
  pseudo-C as it gets exercised.
- Render projectiles as colored points or small OBBs.
- Hook impact: `Damage_AccumulateOrFire` is HIGH and ready.

**Endpoint:** Fire and see things explode (well, disappear).

**Smoke test:**
- Replay `phase5_fire.input` (drive forward, fire 5 projectiles).
- Frame log records ≥5 `Pool_AllocProjectile` calls.
- Frame log records ≥1 `Damage_AccumulateOrFire` call (something
  got hit, either terrain object or self-impact).
- No crashes across 600 frames.

---

## Phase 6 — AI opponent (~5-7 days)

- Spawn a second `Vehicle` at level start (already supported by
  `V8_MainLoop` versus-mode path).
- Hook `Vehicle_TryAcquireTarget` (HIGH, ready) and the per-level AI
  tick callbacks (the per-level destructible cleanups in
  `src/physics/<level>/`).
- Confirm AI applies steering/throttle through the same input-bit
  layout.

**Endpoint:** Second vehicle drives around and shoots at you.

**Smoke test:**
- Boot with `--versus`; player 1 idle 600 frames.
- AI vehicle position delta over 600 frames > 0x20000 (AI is moving).
- Vehicle.+0xe4 (currentTarget) non-zero on AI for ≥10% of frames
  (AI is acquiring targets).
- No crashes.

---

## Phase 7 — Destructibles (~3-5 days)

- All 10 levels' destructible handlers are already wired in the
  per-level files (`tracker_dish.c`, `cruise_missile.c`, `boulder_roll.c`,
  `blimp_main.c`, `siren_strobe.c`, etc.). They just need their FX/sound
  hooks to not be no-ops.
- Effects: spawn a colored particle burst at impact site for now.
- Verify each handler against its `#if 0` Ghidra block as it's
  exercised.

**Endpoint:** Hit a destructible, it animates/breaks per its handler.

**Smoke test:**
- Replay `phase7_destructible.input` against each level's primary
  destructible (10 levels = 10 sub-tests).
- For each: log records the destructible's mode-3 (impact) handler
  invocation followed by mode-2 (retire) or substate transition.
- No crashes across full set.

---

## Phase 8 — Audio (~2-3 days)

- 4-bit SPU-ADPCM decoder for VAG (~50 lines of C).
- miniaudio mixer setup: stereo, 22050 or 44100 Hz, 8 channels.
- Hook `Audio_PlaySfx` → enqueue decoded VAG.
- `Audio_PlayXA` (music): either implement XA-ADPCM (~150 lines) or
  swap for pre-converted OGG of the original tracks.

**Endpoint:** Engine sounds, hit sounds, music playing.

**Smoke test:**
- Replay `phase4_drive.input`; capture audio output to WAV (via
  miniaudio loopback or `--audio-capture phase8.wav`).
- WAV is not silent: RMS > -60 dBFS (tools/smoke/check_audio.py).
- ≥3 distinct sound triggers in audio log (engine + at least one
  hit-sound + at least one cue).

---

## Phase 9 — Polish to "looks like V8" (~3-5 days, partial)

- Load XOBF mesh data, render vehicle as actual mesh (still flat-
  shaded, no textures).
- HUD: speed, health bar, weapon counters from Vehicle fields.
- Pause menu functional via existing `Menu_Pause` code.

**Endpoint:** Recognizable V8.

**Smoke test (final = "running game" delivered):**
- Boot with `--versus`, 60-second autoplay (player input from
  `phase9_match.input`, AI from engine).
- Capture full 60s screenshot every 5s (12 frames).
- All 12 frames non-black, vehicle visible in ≥10/12, HUD overlay
  visible in ≥10/12.
- Audio capture WAV continuous for full 60s, RMS > -50 dBFS.
- Frame rate ≥30 FPS sustained (relaxed from PSX 30 native).
- Final match-end ResultScreen rendered (Vehicle.+0xc → 0 for one
  side, or timer expired).
- Zero crashes; clean shutdown exit code 0.

This is the **acceptance test for "running game"**. Pass → done.

---

## Audit (parallel to every phase)

Per the user's mandate: **audit every line we touch.**

- `notes/audit_log.md` — append-only log: per-file findings,
  divergences from Ghidra ground truth, fixes applied.
- Every MED-confidence file gets a `#if 0` Ghidra reference block
  re-attached during Phase 0; consulting that block IS the audit
  procedure during phases 2–9.
- Files that were cleaned without an auto-stub source (the very
  earliest ones) get extra scrutiny — Ghidra re-export if needed.

---

## Risk register

| Risk | Likelihood | Mitigation |
|------|------------|------------|
| Vehicle struct size wrong (0x200 padding too small or too big) | Medium | First runtime corruption tells us. Real fix via Ghidra. |
| MED rewrites have logic bugs | High | Phase 0 restores Ghidra refs. Audit per phase. |
| GTE saturation not modeled, physics drifts | Medium | Acceptable per user (bit-exactness deferred). Note where it matters. |
| Asset format assumption wrong (e.g. XOBF inner layout) | Medium | Cross-check with hexdumps of actual files; loaders are MED. |
| `func_0xXXXXXXXX` cross-DLL targets we never named are actually critical | Low-Medium | Panic-stub them all in Phase 1; fix as runtime hits them. |
| Build system bikeshed / vendor dep mismatch (Windows toolchain) | Low | CMake + vendored deps removes most of this. |
| Endianness | Low | Both PSX and host (x86/ARM) are little-endian. |

---

## Deliverable

`./build/v8(.exe)` that, given the extracted CD data, opens a window
and plays a versus match — player vs AI, weapons firing, destructibles
breaking, audio playing. Visual fidelity is "primitives," not textured.

`README.md` updated with build instructions and a 60-second demo
capture.

---

## What this plan does NOT promise

- Bit-exactness vs original — explicitly deferred.
- Texture/material fidelity — stylized primitives only.
- Multi-player split-screen — code path exists in `V8_MainLoop` but
  not wired.
- Demo playback / replay determinism — verify scaffolding stays in
  place but won't pass yet.
- Full menu/shell UI — boots straight into a level via Phase 1e stub.

These are all post-MVP and trackable in a follow-up phase document.
