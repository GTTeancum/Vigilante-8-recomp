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

RecompOne is vendored at `tools/recompone-reference/` using `git subtree`, so
this checkout remains one ordinary repository with no submodule setup.

## Asset placement

Put a legally obtained Vigilante 8 BIN/CUE set under:

```text
reference/assets/disc/
```

The entire `reference/assets/` tree is ignored by Git. Do not commit disc
images, extracted copyrighted assets, generated recompiler output, or traces.

The default expected cue name is:

```text
reference/assets/disc/Vigilante8.cue
```

Another name can be passed to the preparation script.

## Prepare the reference configuration

From the repository root:

```powershell
python tools/recompone-v8/prepare_reference.py
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

The preparation script currently emits 1,112 main-executable functions and 278
overlay functions from the checked-in inventories. Generation has been
validated without assets. A full RecompOne build was not possible on the
staging machine because it has .NET 9 rather than the required .NET 10 SDK.

See `REFERENCE_PLAN.md` for the order of work when the project is unparked.
