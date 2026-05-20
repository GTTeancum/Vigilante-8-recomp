# Project Scope — 1:1 Match Requirements

This decompilation must produce **bit-exact** behavior with the original PS1 game for all in-scope subsystems (physics, asset loading, gameplay). The renderer, controls, and audio engine will be rewritten and are explicitly NOT required to be 1:1.

## Bit-Exact Discipline

### Fixed-Point Math (Critical)

PS1 games use fixed-point integers, not floats. Vigilante 8 almost certainly uses **16.16** (sign + 15.16) for most physics quantities and possibly **4.12** for trig.

- **Do not convert any fixed-point math to float during decomp.** Even "for readability." Floats will introduce precision drift and the port will not be 1:1.
- Preserve original integer types exactly. Use `int32_t` / `uint32_t` not `int` for portability.
- Wrap fixed-point types in typedefs for readability:
  ```c
  typedef int32_t fixed16_t;   // 16.16 signed
  typedef int16_t angle12_t;   // 4.12 angle (4096 = 360°)
  ```
- All multiply/divide operations on fixed-point must preserve the shift semantics. Document the format on every function that operates on fixed-point.

### GTE Operations (Critical)

The PlayStation Geometry Transformation Engine performs matrix/vector math with **specific saturation, rounding, and overflow behavior**. If physics or gameplay code uses GTE (it almost certainly does for transforms, dot products, distance checks), the port must use a **bit-exact software GTE**, not a naive C reimplementation.

- Identify every GTE instruction in physics/gameplay code. Common ones: `RTPS`, `RTPT`, `MVMVA`, `OP`, `DPCS`, `SQR`, `NCLIP`, `AVSZ3`, `AVSZ4`, `GPF`, `GPL`.
- Generate a `gte.h` declaring all needed GTE operations as C functions with PS1-accurate semantics.
- Reference implementation: PCSX-Redux's GTE is well-tested. Either port its GTE C code or document which implementation will be linked.
- **Never** replace a GTE op with "equivalent" plain C math. The saturation behavior is part of the contract.

### Random Number Generation

If V8 uses an internal RNG for AI, weapon spread, or anything gameplay-affecting, the exact algorithm and seed sequence matter.

- Locate the RNG function early (look for: small function returning a value, called from many sites, mutates a single global seed).
- Decompile exactly. Do not substitute `rand()`.
- Preserve seed initialization location and value.

### Frame Timing

PS1 NTSC games run physics at 60Hz, PAL at 50Hz. Vigilante 8 (US) is NTSC, so assume **60Hz physics tick** unless the binary proves otherwise.

- Physics ticks at fixed 60Hz regardless of renderer FPS.
- Do not couple physics tick to renderer frame rate.
- If V8 uses a frame counter or `VSync()` callback as timing, preserve that pattern. The new control/render layers must respect this tick rate.

## Asset Loading 1:1

- Load original binary asset formats **directly**. Do not pre-convert to glTF / PNG / WAV during the port.
- The new renderer/audio layers consume whatever struct the original loader produced.
- This is non-negotiable: pre-conversion introduces precision loss and divergence risk, and it forfeits the ability to verify byte-for-byte that the asset pipeline matches.
- Asset format documentation is a deliverable. Each format gets a doc in `notes/formats/`.

## The Seam Contract

Define the boundary between original (1:1) code and new (rewritten) code at these interfaces:

### Physics → Renderer
- Physics outputs vehicle transforms in **fixed-point** (position, rotation matrix).
- Renderer converts to float **for display only**, at the last possible moment.
- The renderer never feeds anything back into physics state.

### Controls → Gameplay
- Original gameplay reads input as abstract commands (accelerate, brake, fire weapon, etc.) from an input state struct.
- The new control layer translates raw input (keyboard, gamepad, whatever) into that exact struct.
- The gameplay code does not change. The input struct is the contract.
- Identify this struct during decomp and document it precisely.

### Audio Engine → Game Logic
- Original gameplay emits sound events (play SFX N, play music M, stop channel C).
- The new audio layer subscribes to those events and handles actual playback.
- Identify the sound-trigger functions during decomp. They become event-emit calls in the port.
- Original sound asset loaders (VAG, SEQ, etc.) stay 1:1; only the playback engine is rewritten.

### Frame Loop
- Original main loop: input poll → game tick → render → audio update → wait for vsync.
- New main loop: input poll (new) → game tick (1:1) → render (new) → audio update (new) → frame pacing (new).
- The game tick boundary is the bit-exact zone.

## Verification

Bit-exact claims require bit-exact verification.

### Verification Scaffolding (deliverable)
Create `tools/verify/` with:
1. **State snapshot serializer** — dumps the full game state struct (all vehicles, all projectiles, RNG state, frame counter) to a binary blob.
2. **Input recorder/replayer** — records the abstract input struct per frame, replays from a log.
3. **State comparator** — diffs two state snapshots, reports field-level mismatches.

### Verification Procedure (documented, not required to execute)
1. Record an input sequence on the original game (TAS-style, frame-perfect input log).
2. Replay through the port using the input replayer.
3. Compare state snapshots frame-by-frame.
4. Any divergence = port is not 1:1. Investigate.

The decomp's job is to make the scaffolding exist and document how to run it. Actually running the verification requires the original game running in an instrumented emulator, which is outside the decomp scope.

## What Counts as a 1:1 Violation

- Replacing fixed-point with float anywhere in physics or gameplay.
- Substituting `rand()` for the original RNG.
- Reordering operations in a way that changes integer overflow / saturation behavior.
- Pre-converting assets (textures, models, audio) during loading.
- Coupling physics tick to renderer frame rate.
- "Fixing" original bugs without explicit instruction to do so. Bugs are part of the 1:1 contract.

If you encounter any of these patterns in Ghidra's pseudo-C output and find them in `src/`, fix them. They are decomp errors, not intentional ports.
