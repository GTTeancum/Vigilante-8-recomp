# Boot-to-game-over vertical slice

## Purpose

Produce one reproducible, fidelity-checked path through the RecompOne reference
from process start to the post-match results flow. The slice is the first
trusted oracle for extracting behavior and gameplay data for the separate
native 32-bit x86 Xbox game.

## Fixed slice

- Region: Vigilante 8 USA (`SLUS_005.10`).
- Mode: one player, deterministic menu path.
- Arena: Oilfield until a different arena is justified and recorded.
- Inputs: a committed poll-indexed fixture, with no dependence on wall-clock
  timing or a physical controller.
- End condition: the original game-over/results state is visible and accepts
  its next menu input.

## Acceptance gates

| Gate | Required behavior | Required evidence |
|---|---|---|
| Boot | BIOS-facing setup, CD initialization, and overlay loading complete without a bypass that changes game state | focused startup log and overlay sequence |
| Intros | all boot movies/logos play, skip, and transition with original timing rules | stage markers and representative captures |
| Menus | title, mode, player/vehicle, level, options used by the slice, pause, and results paths draw and navigate correctly | committed input fixture, menu-state trace, captures |
| Match load | selected mode/vehicle/arena survive the SHELL to LOAD to terrain-overlay transition | selection and overlay-state trace |
| Physics | spawn/drop, suspension contact, stationary settling, acceleration, steering, braking, collision, and recovery are trustworthy | PS1-versus-reference frame/state comparison with first-divergence reporting |
| Weapons | pickup or loadout, selection, firing, ammo, projectile lifecycle, impact, damage, destruction, and retirement execute | event/object trace and visible runtime proof |
| Graphics | movies, menus, HUD, vehicles, terrain, props, particles, weapons, pause, and results are legible and behaviorally faithful | representative captures plus framebuffer/stage markers |
| Audio | music, SFX, voice, channel/voice allocation, looping, stop/fade, menu cues, weapon/impact cues, and results transitions execute | SPU/CD event trace plus audible output capture/check |
| Match end | win/loss condition, teardown, game-over presentation, results accounting, and return/continue input work | end-state trace and capture |
| Stability | the complete fixture runs from a clean launch without unmapped calls, unhandled exceptions, deadlocks, heap exhaustion, or stale-log ambiguity | clean end-to-end log and exit/end marker |

## Fidelity rule

Stable execution is necessary but not sufficient. Data from a subsystem becomes
portable only after that subsystem's relevant state agrees with an original PS1
run or after any remaining difference is proven to be presentation-only.

The first physics comparison starts at vehicle creation and records, per frame:

- frame/tick and input poll index;
- RNG state or the closest proven RNG counter;
- vehicle position, velocity, rotation matrix, and relevant flags;
- wheel position, suspension compression, contact state, and terrain sample;
- weapon/projectile and damage events once those systems enter the slice.

Comparison stops at the first differing field. Fix or classify that divergence
before treating later derived values as source truth.

## Current boot-movie status

The host CD stream now preserves disc pacing while blocking the recompiled
version of the original polling loop until a frame arrives. A deterministic
no-live-input run reached the authored frame-number reset for all three boot
streams: `ACTLOGO.STR` at queued frame 227, `LUXOFLUX.STR` at 91, and
`INTRO.STR` at 1,267. The run then reached the original `PRESS START` text.

The flat movie-surface defect is resolved. SHELL's MDEC quantization and scale
tables live inside its relocated overlay; the generated code retained their
linked addresses when handing them to DMA, so the hardware model read zeroes.
The narrow SHELL MDEC-input bridge now materializes the active overlay delta
only for DMA sources inside that overlay. On the restored original disc, the
runtime loads the retail quantization and cosine tables, decodes 300
macroblocks per 320x240 frame, transfers non-constant output through MDEC-out
and GPU DMA, uploads all 20 strips, and flips the original double-buffered VRAM
pages. A timed capture shows the decoded Activision movie frame rather than the
earlier untouched black page.

Representative original-disc captures show all three authored sequences: the
Activision shatter logo, the rendered Luxoflux logo and slogan, and the
letterboxed opening road shot from `INTRO.STR`. A clean uninterrupted repeat
then played all 227, 91, and 1,267 queued frames, retained changing nonblack
framebuffer hashes, and reached the retail `PRESS START` renderer with no
runtime error marker. The Intros gate is complete for the fixed slice. The
broader Graphics gate still includes menus, gameplay, pause, and results.

## Current physics status

The original player's first integration tick isolated the severe spawn/drop
instability to the host GTE `OP` implementation, before suspension or terrain
collision could create the visible motion. `OP` was updating IR1 while
computing MAC1, then incorrectly consuming that new value while computing MAC2
and MAC3. Snapshotting IR1-IR3 before all three calculations preserves the
original instruction's simultaneous source reads.

With that correction, the vehicle remains upright and its rotation matrix stays
near orthonormal throughout spawn/drop, stationary settling, acceleration,
steering, and braking. The committed `oilfield_physics_smoke.txt` fixture starts
its control timing on the first player physics tick and reproduces the sequence
independently of movie, menu, CD, or host timing. A 55-second corrected run
executed all three control phases, presented changing Oil Fields frames, and
reported no runtime fault. The user visually accepted these motions as correct.
This completes the visual baseline for those Physics sub-gates; collision and
recovery were then exercised by the same fixture. The first focused
vehicle-to-vehicle response occurred at tick 572: the original collision code
changed player velocity from `(-7463,73,-13293)` to
`(23413,-13219,551395)` and angular velocity from `(-213,321,290)` to
`(-85,3431,-181)`. Despite that impulse and continued vehicle/projectile
contacts, the vehicle stayed upright and returned to near-level matrices at
ticks 720, 780, 840, and 900 (`upY` 4072, 4093, 4038, and 4097). The corrected
35-second focused run reached tick 900 with no runtime fault marker. This passes
the runtime collision-and-recovery behavior needed before weapon work; the
PS1-versus-reference state comparison remains open before the full fidelity
gate is closed.

## Commit discipline

Commit after each coherent gate or fix with its focused validation evidence in
`progress.log`. Keep disc images, generated recompilation output, traces, audio
captures, and screenshots local. Keep the verified runnable deployment current
under `PS1 game\RecompOneReference` after builds intended for handoff.
