# First-run setup validation - 2026-09-15

Status: setup implemented and automated first-run/relaunch smoke passed; pending
manual review of the native Windows file picker and physical audio output.

## Tested build and package

- SHA-256: `C8FE23C9B290DD223E06A27DF0ACE25DCC7DCC2CD6441FDA17F0A5A31819152F`.
- Same bytes staged as `V8_2_LOOSE/Vigilante8PC.exe` and `Vigilante82PC.exe`.
- Clean package: `artifacts/setup-20260915/user-install`. Initially executable
  plus all five manifest-selected mods; no loose base game, developer source
  argument, MOD_DIR override, LOOSE_DIR override, or copied personal settings.
- Launched `Vigilante8PC.exe` with no command-line arguments. The process-local
  fixture invokes the same selection callback as the setup button; it supplies
  the user's source path without generating any host input.
- Selected the Second Offense Track 01 BIN; its adjacent matching CUE resolved.
  Imported 128 files and 16 music tracks into `game_data`.
- Final inventory: 4,417 files, 595,975,531 bytes, including SETUP.md, executable,
  all five mods and their dependencies. Original base-game trees are excluded.

## Verified

- App-native captures visually inspected: setup welcome, extraction progress,
  completion, wrong-title error, missing-BIN error, intro movie, main menu,
  native/guest character selection, enhanced loading card, gameplay.
- Final fresh extraction completed and handed off to the game in the same process.
  `final-first-run/result.json` records a clean exit and all automated checks passing.
- Installed relaunch entered gameplay without setup or a selected disc source.
  `guest-relaunch/result.json` records all automated checks passing, with Molo
  selected, correct portrait and model, and guest voice loaded from the mod.
- Enhanced textures active: 4,322 DDS files, 29,843 regions and 19 terrain atlases
  loaded from the package's own mod folder.
- Captured Molo selector audio was compared against all 13 decoded voice samples.
  Molo ranked first (correlation 0.145247; threshold 0.05). This checks audio
  identity rather than merely testing for nonzero PCM. See `voice-content.json`.
- Contract tests pass: wrong title rejected; correct title with an altered EXE
  accepted; cancellation does not commit; BIN resolves its CUE; all 40 guest
  dependencies resolve; mod voice bank and both authored light banks load exactly.
- Build succeeded; `git diff --check` passed.

## Fixes found by the smoke

- Added the GUI importer; removed exact-file hash and fixed-LBA eligibility gates.
  Identity comes from SYSTEM.CNF; file extents come from the selected disc.
- Fixed a setup-to-game crash caused by releasing GL resources after context
  destruction. Resources now dispose during the window's Closing event.
- Fixed a race that could briefly hide the error text behind an extraction bar.
- Packaged missing guest selector/result audio and 12 quest-ending files under
  the guest mod. Voice loading now uses the mod-aware disc filesystem. Additional
  SND banks receive normal virtual file extents, without a content-specific branch.
- Packaged the two authored light-palette banks under the texture mod, preserving
  previous color-customization fixes on a fresh install.

## Limits and separate observations

- No host input or desktop capture was used. Clicking the native Windows file
  picker, canceling it interactively, and sound through physical speakers were
  not tested. Setup selection, import, UI rendering, and PCM output were tested.
- This is a first-install smoke, not every quest/ending, every vehicle, every map,
  controller hardware, or a long soak. Quest media dependencies resolve, but the
  12 endings were not each replayed in this run.
- A Second Offense-only install has Second Offense music. Original V8 remains
  the saved preference, but its optional soundtrack is not on this source disc.
- An additional comparison against the existing development installation logged
  native `Out of VRAM` messages during guest match loading. The final clean-package
  runs did not. Recorded separately for follow-up; this is not a claim that the
  entire staged game is defect-free. Molo's rear-angle view was similar in both
  installations; it does not establish wheel visibility from other angles.
- No release archive was published and no commit/push was performed.

## Reproduce

`python tools/recompone-v8-2/run_first_run_setup_proof.py final-first-run` requires
an empty `game_data` destination in the candidate folder. Use `guest-relaunch`
for an installed run; `wrong-title` and `missing-bin` use separate fixture folders.
The script simulates input only inside the target process and captures through
the application's own GL framebuffer.

`dotnet run --project tools/recompone-v8-2/setup-contract-tests -c Release -- . artifacts/setup-20260915/contracts artifacts/setup-20260915/user-install`

`python tools/recompone-v8-2/stage_guest_mod_dependencies.py` stages the explicitly
listed runtime dependencies from the prepared development install into their
owning mod folders. No base-game directory tree is copied into the release.
