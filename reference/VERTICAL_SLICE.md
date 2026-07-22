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
runtime error marker. The Intros gate is complete for the fixed slice.

## Current menu status

The deterministic one-player route captures the original player-count,
Arcade/Quest, location, player, and enemy screens at their retail input-ready
boundaries. The Options hub now traverses all eight original pages: Game Status,
Memory Card, Difficulty, Controller 1, Controller 2, Audio, Screen, and Credits.
Focused captures show each page. The Audio editor changed Music, Sound Effects,
and stereo/mono in both directions before cancel restored the initial state.
Screen Adjustment moved both axes in both directions and restored offsets
`(0,0)`. Selecting Credits started `VIDEO/CREDITS.STR`; the run presented its
full 1,251-frame loop through the authored frame reset and normal restart without
a runtime-fault marker.

The Oil Fields pause fixture exercises CD-track changes, Resume, the original
Are You Sure? confirmation, cancellation, a second gameplay resume, and a
confirmed Yes/quit return. The confirmation loop polls PS1 pad state without a
VSync, so the host advances one presentation/input frame only at that original
poll site. This preserves the retail decision logic while supplying the
asynchronous input edge the statically recompiled process otherwise cannot see.

The remaining front-end branches are also covered by clean fixtures. Quest
selects a player and reaches the route map, including route movement and the
original completion-state text. Two-player automation supplies a neutral,
test-only controller-2 connection, then separately anchors and accepts Versus
and Cooperative, enters Oil Fields, and reaches each mode's visibly distinct
two-player selector layout. Normal launches still use live controller discovery.
Together with Options and pause, this completes the Menus gate for the reference
vertical-slice phase.

## Current graphics status

All Graphics evidence comes from the running original-disc reference, not from
mockups. Representative captures cover decoded movies; 640x480 title, selection,
Options, Quest, Cooperative, and Versus layouts; 320x240 Oil Fields terrain,
vehicles, props, HUD, radar, particles, projectiles, impacts, and destruction;
the pause/confirmation overlays; and the presented `YOU LOSE!` result screen.
Stage-bound captures are requested only after the retail text or gameplay state
identifies the target screen, and changing nonblack framebuffer hashes continue
through gameplay, destruction, result dismissal, teardown, and replay loading.

This completes the original-resolution Graphics gate for the vertical slice.
Higher output resolutions and anti-aliasing are deliberately queued as the first
post-goal presentation work so they cannot hide or perturb the fidelity baseline.

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
the runtime collision-and-recovery behavior needed before weapon work, and the
user has visually accepted the complete vehicle-physics presentation as
correct. The
PS1-versus-reference state comparison remains open before the full fidelity
gate is closed.

## Current weapon status

The retail digital-pad mapping was recovered directly from `SLUS_005.10` at
`0x80056784`. The game byte-swaps the two button bytes before applying mapping
words `A8563412 B9C81007`; in the resulting abstract command word, R2 holds the
built-in-gun bit, L2 holds the selected-weapon bit, and R1/L1 produce the two
weapon-cycle edges. The committed `oilfield_weapon_smoke.txt` fixture uses those
original mappings instead of host-button guesses.

A focused 35-second run collected a weapon pickup, built-in-gun firing, selected
weapon firing, ammo change, projectile registration, impact, and retirement.
The trace recorded 34 player projectile spawns, 34 registrations, 34 retirements,
and three impacts. The selected missile slot changed from 12 rounds to 11 after
firing.

The committed `oilfield_match_smoke.txt` fixture extends that route through the
original damage, destruction, and post-match logic. A clean boot-to-results run
recorded the trace's full 128-transition damage sample, an enemy soft kill and
full destruction, then the player's full destruction at tick 7,424. The retail
result builder ran at tick 7,459 with mode 1, the original alive flag cleared,
and all three player damage zones at zero. Player-owned projectiles continued to
use the original spawn, registration, impact, and retirement callbacks; no test
hook changed health, damage, AI, or match state. This completes the fixed-slice
Weapons gate at runtime, subject to the same PS1-versus-reference state audit as
the final fidelity gate.

## Current audio status

The CUE reader now builds the retail multi-file table of contents: track 1 is
the data track and tracks 2-13 are raw 44.1 kHz stereo CD audio. The host
implements `CdlGetTN`, `CdlGetTD`, and `CdlPlay`, streams 2,352-byte audio
sectors into a paced ring buffer, and mixes them through the original SPU CD
left/right volume registers. CD playback owns a position separate from the
latest `CdlSetloc`, matching the drive behavior needed for music to continue
while the shell locates ordinary assets.

An audible Oil Fields run presented the 1-13 TOC, started menu track 2 at its
authored pregap, retained playback through later data seeks, and started a
gameplay-selected track without an unmapped-sector stop. The pause regression
then changed track 2 to track 3 and back through the original CD Track row.
Focused sector traces recorded nonzero source peaks, proving that the runtime is
consuming PCM from the audio BIN files rather than mixing a silent placeholder.
The host now also applies the retail CD input matrix, mute/demute state, and the
original SPU CD-volume registers to both CDDA and XA input.

The clean boot-to-results audio regression played menu track 2 and gameplay
track 5, while the original SPU path allocated menu, weapon, impact, and vehicle
voices. Its bounded lifecycle trace recorded 576 key-ons, 24 key-offs, 417
sample ends, and one loop before the trace limit. The paced result transition
selected `DEFEAT.XA` file 1/channel 0, decoded nonzero sectors with peaks above
30,000, and delivered absolute-sector ready reports through the retail
`0x80043D94` callback. Mixed-output summaries independently showed active CDDA
and active XA buffers.

The standard-library capture checker validated 201.363 seconds of 44.1 kHz
stereo signed-16 output: peak 32,766, RMS -8.55 dBFS, and zero clipped samples.
This completes the Audio gate, including music, SFX/voice allocation, looping,
stop/end behavior, weapon/impact cues, and the result transition.

## Current match-end status

The result-stage capture is delayed 60 input polls after `ResultScreen_Build`,
because the builder creates the layout before a later ordering table presents
it. The resulting original-disc capture visibly contains `YOU LOSE!`, the
player-versus-enemies score, and the `Press O to replay, X to quit` prompt. The
fixture waits through the original 300-tick minimum, sends the mapped Circle
edge at result poll 320, and reaches normal teardown, arena cleanup, and disc
reload. The complete run had no unmapped call, unhandled exception, fatal,
terrain, or heap-exhaustion marker. The Match end gate is complete for the
fixed slice.

## Current stability status

The final clean regression launched from boot, navigated the deterministic
retail route, ran the complete AI-controlled match, reached the result builder,
accepted the stage-relative replay input after the original minimum interval,
completed teardown, reloaded LOAD and Oil Fields, and continued presenting the
new match. It ran for 201 seconds with no unmapped call, unhandled exception,
invalid-animation-pointer diagnostic, heap corruption/exhaustion, terrain fault,
deadlock, or stale-log ambiguity. The newly recovered original callback at
`0x8003B3C8-0x8003B8D4` covers the deferred object event reached during replay
construction. This completes the fixed-slice Stability gate.

## Commit discipline

Commit after each coherent gate or fix with its focused validation evidence in
`progress.log`. Keep disc images, generated recompilation output, traces, audio
captures, and screenshots local. Keep the verified runnable deployment current
under `PS1 game\RecompOneReference` after builds intended for handoff.
