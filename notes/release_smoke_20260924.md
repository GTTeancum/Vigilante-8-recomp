# Release candidate smoke — 2026-09-24

Prepared the Windows x64 release locally on main. Nothing published, committed or
pushed in this task. [Inventory and archive identity](release_manifest.md).
[User instructions shipped in the ZIP](release/SETUP.md).

## Actual distribution tested

Extracted `package-full/Vigilante8Classic-2026-09-24-win-x64-rc.zip` to
`artifacts/release-20260924/smoke-full/Vigilante8Classic`, checked every manifest
file's size/hash and confirmed no installed game data or settings before launch.
The executable ran with no command-line arguments, no developer mod/data path
override, and normal install-local settings. A process-local fixture supplies the
source selection through the setup callback and game input; no desktop input or
screen capture was used. Captures came from the application's own renderer.

## Results

- Fresh setup: all 144 progress units completed (128 files and 16 music tracks),
  committed game_data, automatically entered menus and Meteor Crater gameplay,
  then exited cleanly. Inspected welcome, progress, completion, Classic background,
  main menu, loading card and gameplay captures for actual expected content.
- Installed relaunch: no source argument and no setup prompt; reached Molo's
  original V8 selector and gameplay. Inspected correct portrait, school bus,
  UI, textured scene and HUD captures. Installed loose files were the data source.
- Texture pack: active; 4,322 DDS files, 19 terrain atlases, 138 route regions,
  and three file-font atlases loaded from the extracted release.
- Audio: all 16 extracted OGG tracks decoded completely without errors. Captured
  Molo's selector voice ranked guest 10 first against all 13 source samples
  (correlation 0.1800, next candidate 0.0503). This checks identity, not merely
  nonzero audio. Physical speaker output was not tested.
- Wrong-title and missing-BIN runs showed readable GUI errors, exited cleanly,
  and did not leave a completed installation.
- Contracts passed: BIN resolves its matching CUE, wrong title rejected, same
  title with changed executable bytes accepted (no hash gate), canceled import
  never commits, all guest dependencies resolve, selector voice override present,
  and authored light palettes load from the supplied banks.
- ZIP CRC, per-file integrity and referenced mod-asset checks passed. No disc
  images, full base-game installation, saves, settings or test logs shipped.

Evidence is under `artifacts/release-20260924/`: `smoke-full/*/result.json`,
native PNG captures and logs, `smoke-full/guest-relaunch/voice-content.json`,
`music-decode.json`, and `contracts-installed/result.json`.

## Scope and remaining manual checks

This is a setup-to-gameplay smoke, not another all-level, multiplayer or full-quest
regression pass. Quest endings and full intro movies were not replayed in this run.
Native Windows file-picker clicks/cancel and real keyboard/controller interaction
remain manual checks; the harness substitutes process-local input. Physical
speaker output and operation on a separate clean Windows machine remain unverified.
TO-DO #1 stays pending review for the file picker and physical audio checks.

The first packaging attempt omitted 304 nested DDS assets. It was rejected before
handoff; recursive asset selection plus manifest-reference validation corrected it.
Only the corrected `package-full` ZIP was used for the completed smoke above.
