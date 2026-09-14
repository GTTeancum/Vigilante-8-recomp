# Vigilante 8 PC - Open Work Only
Last updated: 2026-09-14 EDT
Status: PENDING REVIEW
This tracker contains only open or explicitly deferred work. Remove an entry when the user accepts or closes it; completed work, evidence history, and superseded attempts do not belong here.

## Open items at a glance
This is one line per major open deliverable or defect. Detailed implementation and acceptance work remains below. `Codex` marks work added by Codex. **Work-selection rule:** When the user asks Codex to find or choose something to do, start with the highest `NEXT PRIORITY` item before considering any other open work.

1. **PENDING REVIEW** - Local multiplayer menus and controller integration.

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

### V82-OPEN-015 - Local multiplayer menu integration

- **PENDING REVIEW**: Local multiplayer menus and controller integration are
      implemented and staged in `V8_2_LOOSE`. Review the join screen, 2P/3P/4P
      selection and match flow, including keyboard plus gamepads or four gamepads.
      Automated acceptance is recorded in
      `notes/local_multiplayer_acceptance.md`.

## Deliberate boundaries

- The reference executable remains Windows x64; the shipping Xbox game is a
  separate 32-bit x86 implementation.
- Standalone loose mode must never discover, open, or fall back to BIN/CUE data.
- Renderer, raw controls, and host audio output are rewrite seams; gameplay
  state and timing remain the fidelity target.
