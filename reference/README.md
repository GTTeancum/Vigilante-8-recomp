# RecompOne reference lane

This directory stages RecompOne as an executable source-of-truth lane beside
the existing native C port. It does **not** replace `src/`, `platform/`, the
asset loaders, or the shipping runtime.

## Intended use

- Recompile the original PS1 executable and its overlays without translating
  their behavior by hand first.
- Trace gameplay, physics, AI, pickups, weapons, damage, object lifecycles,
  HUD state, menu flow, and UI primitive submission.
- Replay the same abstract input in the reference and native runtimes.
- Compare original PS1 RAM/state transitions with the clean C implementation.
- Rewrite renderer, controls, audio presentation, HUD, and menus cleanly while
  keeping their state rules and timing source-backed.

The active reference milestone is broader than the original decompilation
charter's shipping seams: it is a complete boot-to-game-over vertical slice,
including intros, full menu flow, physics, weapons, graphics, music, sound
effects, voices, pause, and results. This gives the separate native Xbox game a
trustworthy oracle even where its eventual renderer and audio engine differ.

RecompOne is vendored at `tools/recompone-reference/` using `git subtree`, so
this checkout remains one ordinary repository with no submodule setup.

## Asset placement

This checkout uses the legally obtained Vigilante 8 BIN/CUE set under:

```text
BINCUE/Vigilante 8 (USA).cue
```

Pass a different cue path to the preparation script when needed. Disc images,
extracted copyrighted assets, generated recompiler output, and runtime traces
remain local and must not be committed.

## Build and run

From the repository root:

```powershell
python tools/recompone-v8/prepare_reference.py --cue "BINCUE/Vigilante 8 (USA).cue"
dotnet run --project tools/recompone-reference/RecompOne.Recompiler/RecompOne.Recompiler.csproj -c Release --no-build -- reference/generated/v8.recompone.json
dotnet build reference/generated/recompiled/Vigilante8PC.csproj -c Release --no-restore
& "reference/generated/recompiled/bin/Release/net10.0/Vigilante8PC.exe" "BINCUE/Vigilante 8 (USA).cue"
```

The same self-contained executable is deployed to two locations:

- `BINCUE/Vigilante8PC.exe` tests the direct BIN/CUE setup.
- `PS1 game/Vigilante8PC.exe` tests the standalone extracted files in
  `PS1 game`; it does not consult the sibling `BINCUE` directory.

Both copies have the same binary hash. Their neighboring data selects direct
disc or standalone loose mode. The executable defaults Vigilante 8's own Music
and Sound Effects controls to 0 without changing Windows volume or the host
mixer. `LOOSE_FILES.md` documents the modding boundary, stream formats, and
optional command-line controls.

Loose files are also the supported mod surface for the deployed single-file
executable. The experimental C# runtime-hook loader remains usable in the
framework-dependent development build, but is explicitly disabled in the
single-file handoff because MonoMod RuntimeDetour does not support that publish
layout. The host reports this boundary instead of silently accepting inactive
hooks.

### High-resolution presentation

The host Display settings now provide 1280x720, 1920x1080, 2560x1440, and
3840x2160 output presets, a **High-resolution 3D (4x)** switch, and Off/FXAA
anti-aliasing. Fullscreen uses the desktop resolution. The game image remains
at its original 4:3 aspect inside the selected output, and the settings persist
in `interface.ini`. New or reset configurations default to 1080p, 4x 3D, and
FXAA.

High-resolution 3D sends the original PS1 primitive stream through the host GPU
rasterizer at four times the native dimensions: 320x240 gameplay is rasterized
at 1280x960 before presentation. FXAA then operates on those genuinely
higher-resolution polygon edges. Fixed-point simulation, primitive submission,
VRAM data, UI coordinates, and timing remain unchanged; source textures and 2D
shell artwork naturally retain their original detail. This is not MSAA or a
high-resolution texture replacement.

Automation may override the saved choices with `RECOMPONE_GPU_HLE=1`,
`RECOMPONE_OUTPUT_RESOLUTION=1920x1080`, and
`RECOMPONE_ANTI_ALIASING=FXAA`.
`RECOMPONE_PRESENTATION_CAPTURE=1` adds post-presentation PPMs beside the
unchanged native-framebuffer captures. The test-only
`RECOMPONE_PRESENTATION_RESOLUTION` and
`RECOMPONE_PRESENTATION_CAPTURE_FRAME` overrides exercise an exact offscreen
surface independently of the current monitor size.

Host audio uses the bundled SDL2 runtime to queue the reference SPU, XA, and
Red Book CD-audio mix as 44.1 kHz stereo signed-16 output, so the handoff does
not require a separate OpenAL installation. Direct-disc mode reads the mixed
mode TOC and raw CD audio. Standalone mode presents the same retail track
positions from the named `music/*.ogg` files and decodes XA from full loose
Mode 2 sectors; both routes retain the original CD-volume path.

For deterministic automation, set `RECOMPONE_INPUT_FILE` to a committed
poll-indexed fixture. The current reachability baseline is
`tools/recompone-v8/input-scripts/oilfield_gameplay_smoke.txt`; it proves the
path into gameplay but does not yet carry a PS1-fidelity claim.

Stage-relative sections such as `[press_start]` reset their local poll counter
when the original SHELL text renderer draws a recognized menu heading. The
validated `oilfield_stage_smoke.txt` uses this mechanism so movie/CD wall-clock
variation cannot move button pulses onto a different menu. It accepts the arena
only after the original shell renders `Oil Fields` rather than relying on the
current cursor or a fixed delay.

Set `RECOMPONE_DISABLE_LIVE_INPUT=1` for deterministic validation runs. This
neutralizes attached keyboards/gamepads before applying a scripted fixture, so
real controller state cannot skip boot movies or perturb the replay. It is a
test-only switch; normal launches retain the original movie-skip controls.

Focused movie diagnostics are available without changing normal output:
`RECOMPONE_TRACE_MDEC=1` records table loads and decoded-frame hashes,
`RECOMPONE_TRACE_DMA=1` records MDEC/GPU transfer summaries, and
`RECOMPONE_TRACE_GPU=1` records image uploads and display-page flips. Automated
capture runs can lower `RECOMPONE_DISPLAY_PROBE_INTERVAL` from its default 120
presentations without changing movie timing.

Stage-synchronized fixtures also save `recompone_capture_<stage>.ppm` at each
recognized menu input boundary, so menu evidence comes from the exact retail
screen that accepted the scripted action.
`RECOMPONE_CAPTURE_SCRIPTED_STAGE=<stage>` additionally captures every pulse
within one selected stage with its local poll number; this is useful for
decoding multi-control screens such as the enemy and passcode editors without
changing game state.

The focused vehicle regression is
`tools/recompone-v8/input-scripts/oilfield_physics_smoke.txt`. Its `gameplay`
stage begins at the first original player-vehicle physics tick, waits 180 ticks
for spawn/drop and suspension settling, then exercises acceleration,
acceleration plus steering, and braking. Set `RECOMPONE_TRACE_VEHICLE=1` to log
the player's fixed-point position, velocity, angular velocity, force, torque,
and rotation matrix for the first 900 physics ticks and to request captures at
the selected settle and control boundaries.

The same trace records the first 32 player collision callbacks before and after
the original response code, including other-object kind, contact data,
velocity, angular velocity, and rotation matrix. It saves one focused capture
at the first vehicle-to-vehicle contact without flooding the trace directory
during persistent contact.

The focused weapon regression is
`tools/recompone-v8/input-scripts/oilfield_weapon_smoke.txt`. It uses the
digital-pad mapping recovered from the retail executable: R2 fires the built-in
gun, L2 fires the selected weapon, and R1/L1 cycle weapon slots. Set
`RECOMPONE_TRACE_WEAPONS=1` to record player loadout changes, child-projectile
creation, final registration, impacts, and retirement. The trace is deliberately
scoped to player-owned projectiles so sustained firing does not bury failures in
unrelated arena-object logging.

The full match regression is
`tools/recompone-v8/input-scripts/oilfield_match_smoke.txt`. It follows the same
stage-synchronized retail route, continuously exercises both fire paths while
the original AI and damage systems remain in control, and continues until a
real vehicle destruction reaches `ResultScreen_Build`. The result-screen input
is stage-relative: after the original 300-tick minimum it sends Circle at poll
320, exercising normal result dismissal and match teardown. With
`RECOMPONE_TRACE_WEAPONS=1`, the run also records aggregate/zone damage,
soft/full vehicle destruction, and the result state. No fixture input writes
gameplay state directly.

The complementary input-only win regression is
`tools/recompone-v8/input-scripts/dual_cheat_victory_smoke.txt`. It enters
`I_WILL_NOT_DIE` and `DEADLY_MISSILE` through the original Game Status passcode
editor, selects one opponent, and runs with
`RECOMPONE_V8_VICTORY_AUTOPILOT=1`. That switch only contributes normal digital
pad masks for steering, braking, and weapons; it observes damage callbacks to
choose a target and never writes game state. The validated run destroyed the
opponent through the retail damage path, rendered `YOU WIN!`, and dismissed the
result normally.

The front-end Options regressions are
`tools/recompone-v8/input-scripts/options_audio_smoke.txt` and
`tools/recompone-v8/input-scripts/options_full_smoke.txt`. The first enters the
original Audio editor, changes Music, Sound Effects, and stereo/mono in both
directions, then cancels and proves that the retail settings were restored. The
second traverses Game Status, Memory Card, Difficulty, both Controller pages,
Audio, Screen Adjustment, and Credits. It moves both screen-offset axes in both
directions, cancels to the original offsets, and launches the complete retail
credits movie.

The pause regression is
`tools/recompone-v8/input-scripts/oilfield_pause_smoke.txt`. It changes the CD
Track row, resumes gameplay, enters and cancels the quit confirmation, resumes
again, then selects Yes and exits the match through the original pause return.
The reference advances host input only inside the retail confirmation loop,
whose asynchronous PS1 pad polling otherwise has no scheduling point in a
statically recompiled host process.

The remaining front-end branches use `quest_menu_smoke.txt`,
`two_player_coop_menu_smoke.txt`, and `two_player_versus_menu_smoke.txt`. They
cover the Quest vehicle and route screens, active Quest gameplay and pause/quit
flow, plus both two-player mode layouts and the shared player selector.
Two-player automation sets
`RECOMPONE_FORCE_PAD2_CONNECTED=1`; this test-only switch reports a neutral
second controller without changing normal live controller discovery.

## Prepare the reference configuration

From the repository root:

```powershell
python tools/recompone-v8/prepare_reference.py --cue "BINCUE/Vigilante 8 (USA).cue"
```

The script converts the checked-in Ghidra function inventories into
RecompOne's function-map format and writes:

```text
reference/generated/v8.recompone.json
reference/generated/function-maps/*.json
```

It does not modify the original analysis exports. If the disc uses different
case or directory names, edit `tools/recompone-v8/overlay_paths.json` after
checking the disc directory. The known paths for `Shell\\Shell.dll` and
`Shell\\Load.dll` come directly from the original executable's strings; the
terrain paths follow the existing `input/TERRAIN/*.DLL` layout and are marked
as assumptions in the manifest.

## Build boundary

The preparation step is safe without assets. Running or debugging the
generated reference requires the disc and the .NET 10 SDK currently targeted
by the vendored RecompOne projects. RecompOne is
an immature upstream tool, so bring-up fixes belong in the vendored reference
lane unless they reveal a defect in the native V8 implementation.

The preparation script currently emits 2,409 main-executable functions and 506
overlay functions; runtime discovery brings the generated total to 2,993. The
Release build has been validated with .NET 10.0.302. Scripted navigation,
steering, acceleration, and weapon inputs sustained active Oilfield gameplay
for 272.7 seconds without a runtime error, followed by a separate deployed-copy
smoke test. The stage-aware fixture also completed two generated-build runs and
one deployed-build run, each remaining active in Oilfield past 55 seconds with
no fatal runtime marker. The latest visual proof is
`reference/traces/recompone_gameplay_deployed.png`.

The original-disc boot path also completes all three unskipped movie streams
with visible decoded output and reaches the retail `PRESS START` renderer. The
handoff launcher retains normal movie-skip input while setting only Vigilante
8's in-game Music and Sound Effects controls to zero.

The vehicle spawn/drop instability was traced to the host GTE `OP` operation:
writing MAC1 also changed IR1 before MAC2 and MAC3 read their source vector.
The runtime now snapshots all three source IR registers before producing any
result, matching the instruction's simultaneous source reads. The deterministic
Oil Fields exercise retains an upright, near-orthonormal vehicle matrix through
settling, acceleration, steering, and braking, and completed a 55-second active
gameplay run without a runtime error marker. The user visually accepted those
motions as correct. Focused collision tracing then captured the original impulse
response and upright recovery through tick 900 without a runtime error marker;
the user visually accepted the complete vehicle-physics presentation.

The committed match fixture has also completed a clean original-disc run from
boot through damage, enemy and player destruction, the visibly rendered retail
`YOU LOSE!` result overlay, its accepted Circle input, and normal teardown. The
run logged no unmapped call or runtime-fault marker.

The complementary win fixture completed the same path without state writes:
two passcodes were entered through the original UI, ordinary pad input defeated
the sole opponent, and the retail result screen rendered `YOU WIN!` with one
player and zero enemies before accepting Circle. A separate Quest run selected
its route, loaded Ski Resort, sustained 900 active gameplay ticks with movement
and weapons, and quit through the original pause confirmation.

The original Options hub now completes all eight pages with visible captures.
The Audio editor restores its initial values after bidirectional changes, the
Screen Adjustment editor restores `(0,0)` after moving both axes, and the
Credits selection played all 1,251 queued frames through its authored reset and
normal loop restart. The pause fixture completed CD-track selection, resume,
confirmation cancel, a second resume, and confirmed quit with no runtime-fault
marker. Separate clean fixtures also exercised Cooperative, Versus, and both
two-player selector layouts. See `VERTICAL_SLICE.md` for the retained evidence;
the reference's state trace is the oracle for later native-port comparison.

The original CD-audio path now sees the retail 1-13 TOC rather than a synthetic
single data track. Focused audible runs started menu track 2 and gameplay track
8, continued menu music across unrelated asset seeks, and used the pause CD
Track row to move from track 2 to track 3 and back. The completed boot-to-results
audio regression then played menu track 2, gameplay track 5, the original SPU
menu/weapon/impact voices, and the `DEFEAT.XA` result stream. Its bounded SPU
trace recorded 576 key-ons, 24 key-offs, 417 sample ends, and a looping voice
before reaching its diagnostic limit. XA reports advanced through the retail
ready callback while CDDA and XA both appeared in mixed-output summaries.

`tools/recompone-v8/check_reference_audio.py` validated the final 201.363-second
44.1 kHz stereo signed-16 capture at -8.55 dBFS RMS, peak 32,766, with zero
clipped samples. The same clean launch reached results, accepted the replay
input, completed teardown, reloaded Oil Fields, and remained active without an
unmapped call, unhandled exception, invalid-animation-pointer diagnostic, heap
failure, or terrain fault. This completes the vertical-slice Audio and
Stability gates.

See `REFERENCE_PLAN.md` for the architecture and `VERTICAL_SLICE.md` for the
active acceptance gates.
