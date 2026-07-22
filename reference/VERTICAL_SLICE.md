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

This proves movie transport and transition timing, but does not yet close the
Intros or Graphics gates: the current presented movie surface is flat gray.
MDEC decode/upload fidelity remains the next boot-path defect.

## Commit discipline

Commit after each coherent gate or fix with its focused validation evidence in
`progress.log`. Keep disc images, generated recompilation output, traces, audio
captures, and screenshots local. Keep the verified runnable deployment current
under `PS1 game\RecompOneReference` after builds intended for handoff.
