# Native controls and contextual prompts

2026-09-10. Former TO-DO #6 accepted and closed by the user.

Options > Controllers > configure opens the existing native remap page.
Choose controller 1/2, preset, device, action, then binding. Gamepad bindings
support replace, add, and clear; keyboard bindings support replace and clear.
Escape cancels capture; unattended capture times out. Modern, Trigger Drive,
Classic, and Southpaw presets affect only the selected pad, preserving the
other player's pad and both keyboards. Custom labels persist per player.
Adding an already assigned button preserves the other assignments.

Driving bindings are separate from standard menu navigation, so clearing or
remapping gas cannot lock out settings. The one-shot loading ready message
establishes a driving-input context until the native ready wait actually ends.
The displayed gas binding therefore matches the button that starts play:
Trigger Drive uses RT. Keyboard prompts use the assigned key. Menu face-button
labels use the menu mapping. Loading tips use gameplay bindings; an unassigned
action is labeled Unbound rather than inventing a button.

Text replacement preserves native RGB commands and uses scoped guest stack
storage, restoring the caller's stack and pointer. Footer strings are replaced
before the native compositor measures and spaces them. The persistent patcher
installs these seams when generated sources are rebuilt.

## Verification

- `tests/ControlsContract`: player isolation, save/reload, keyboard menu access,
  custom labels, native RGB preservation, stack restoration, packed footers,
  and the actual physical RT/LT/A resolver in a headless process.
- `run_contextual_controls_proof.py`: native menu remapping, P2 Southpaw,
  P1 custom LB/RT, duplicate add, clear/rebind RT, and saved settings checks.
- The same runner with `--ready`: ordinary selectors into Meteor Crater,
  a delayed physical RT pulse at the ready screen, gameplay and pause captures.
  The fixture removes the old virtual Cross acceptance pulse.
- Evidence lives in `artifacts/contextual-controls-20260910/`; each native
  proof records the staged executable SHA256 and captures. Tests use only
  process-local input and native presentation captures, with live input
  disabled. Original settings are restored byte-for-byte after each run.

Physical hardware feel/hot-plug/rumble remains under the separate user signoff
item. Players 3/4 menu integration remains TO-DO #4. Controls acceptance and
commit/push were explicitly requested by the user.
