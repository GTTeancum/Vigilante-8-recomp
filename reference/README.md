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
dotnet build reference/generated/recompiled/Vigilante8Reference.csproj -c Release --no-restore
& "reference/generated/recompiled/bin/Release/net10.0/Vigilante8Reference.exe" "BINCUE/Vigilante 8 (USA).cue"
```

The verified handoff is also copied to `PS1 game/RecompOneReference/`. Launch
it with `PS1 game/Run RecompOne Reference.cmd`.

That launcher sets Vigilante 8's own Music and Sound Effects controls to 0 for
the working handoff without changing Windows volume or the host mixer. Direct
test launches remain audible unless `RECOMPONE_V8_GAME_VOLUME=0` is set.

Host audio uses the bundled SDL2 runtime to queue the reference SPU/XA mix as
44.1 kHz stereo signed-16 output, so the handoff does not require a separate
OpenAL installation.

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

The preparation script currently emits 2,399 main-executable functions and 504
overlay functions; runtime discovery brings the generated total to 2,981. The
Release build has been validated with .NET 10.0.302. Scripted navigation,
steering, acceleration, and weapon inputs sustained active Oilfield gameplay
for 272.7 seconds without a runtime error, followed by a separate deployed-copy
smoke test. The stage-aware fixture also completed two generated-build runs and
one deployed-build run, each remaining active in Oilfield past 55 seconds with
no fatal runtime marker. The latest visual proof is
`reference/traces/recompone_gameplay_deployed.png`.

See `REFERENCE_PLAN.md` for the architecture and `VERTICAL_SLICE.md` for the
active acceptance gates.
