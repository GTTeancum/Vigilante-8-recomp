# Vigilante 8 PC - Open Work Only
Last updated: 2026-09-16 EDT
Status: PENDING REVIEW
This tracker contains only open or explicitly deferred work. Remove an entry when the user accepts or closes it; completed work, evidence history, and superseded attempts do not belong here.

## Open items at a glance
This is one line per major open deliverable or defect. Detailed implementation and acceptance work remains below. `Codex` marks work added by Codex. **Work-selection rule:** When the user asks Codex to find or choose something to do, start with the highest `NEXT PRIORITY` item before considering any other open work.

1. **PENDING REVIEW** - Full GUI first-run setup implemented; fresh-install and relaunch smoke passed. Native file picker / physical audio review remains.

## Standing implementation rules

- Correct source assets and conversion output before changing runtime
      behavior. Do not add content-name, content-ID, model-name, source-game,
      or map-specific runtime branches.
- Runtime extensions may generalize tables, metadata, formats, or missing
      engine capabilities for all mod content. If a content-specific code
      exception appears unavoidable, block the goal and present the exact plan
      for user review before implementing it.
- Stage every V8:2 test build and mod only in `V8_2_LOOSE`; never place
      `Vigilante82PC.exe` or V8:2 mods in `PS1 game`.

## V8: 2nd Offense issue register

Test and deployment target: `V8_2_LOOSE` only.

### V82-OPEN-016 - Full GUI first-run setup smoke test

- **PENDING REVIEW**: Fresh-install extraction, automatic game launch, installed relaunch,
      guest selection/audio, and negative-source checks passed. Evidence and limits:
      `notes/first_run_setup_validation.md`. Native Windows file-picker interaction
      and physical audio output remain unverified.
- The user launches `Vigilante8PC.exe` normally. No terminal, command-line
      arguments, developer harness setup, or manual extraction is required.
- On first run, a GUI asks the user to select the Vigilante 8: 2nd Offense
      BIN/CUE source. The user only needs the appropriate BIN/CUE files.
- Accept the supported Second Offense title ID (`SLUS-00868` / `SLUS_008.68`).
      Do not require disc/file hashes to match and do not use hash detection
      as an eligibility gate. Parse/read errors still need useful GUI feedback.
- Display a GUI progress bar while the game files are extracted.
- Once extraction completes, launch the game automatically in the same
      user-facing flow, without a second manual launch or terminal step.
- Smoke test a clean install with no pre-extracted game files or developer
      paths available. Inspect the actual GUI, progress display, extraction
      result, transition into the game, expected visuals and audio; rendered
      frames alone are not sufficient. Report anything not verified.
- Verify that a subsequent launch uses the completed installation without
      asking for the disc again. Preserve the protected `V8_2_LOOSE` root;
      use isolated test data for the fresh-install check.

## Deliberate boundaries

- The reference executable remains Windows x64; the shipping Xbox game is a
  separate 32-bit x86 implementation.
- After GUI setup imports the user-selected disc, standalone loose gameplay
  must use the installed files and never fall back to BIN/CUE data.
- Renderer, raw controls, and host audio output are rewrite seams; gameplay
  state and timing remain the fidelity target.
