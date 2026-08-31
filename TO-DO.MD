# Vigilante 8 PC - Open Work Only

Last updated: 2026-08-31 EDT
Status: ACTIVE

This tracker contains only open or explicitly deferred work. Remove an entry
when the user accepts or closes it; completed work, evidence history, and
superseded attempts do not belong here.

## Open items at a glance

This is one line per major open deliverable or defect. Detailed implementation
and acceptance work remains below. `User` marks user-added work; `Codex` marks
work added by Codex.

**Work-selection rule:** When the user asks Codex to find or choose something
to do, start with the highest `NEXT PRIORITY` item before considering any other
open work.

1. **User — NEXT PRIORITY** - Verify every map's renderer fidelity against original gameplay before release.
2. **User** - Reusable cross-game vehicle/map conversion infrastructure with no content-specific runtime hacks.
3. **User** - Windows controller-family detection and matching controls models.
4. **User** - Balance the default music, voice, and SFX mix.
5. **User** - Add four-player local multiplayer after release.
6. **User** - Add a persistent option to disable transformation powerups.
7. **User — PARTIAL** - Port the original Vigilante 8 UI as a selectable presentation.
8. **User** - Move remaining wrapper functionality into the native game UI.
9. **User** - Add customizable controls with Modern, Classic, and Southpaw presets.
10. **User** - Fix Enhanced-renderer close-range scenery clipping.
11. **Codex** - Complete physical-controller and arena/split-screen user signoff.
12. **User** - Fix incorrect audio during character and AI selection.
13. **User** - Wire original Vigilante 8 quests into the existing per-character/vehicle quest system.
14. **User** - Fix repeated-match texture loss and severe framerate degradation.
15. **User** - Correct Houston's converted special-weapon enemy tracking.

## Goal and conversion rules

- [ ] Build reusable conversion pipelines so original Vigilante 8 and Nintendo
      64 cars/maps work through the same V8:2 infrastructure as native content.
- [ ] Correct source assets and conversion output before changing runtime
      behavior. Do not add content-name, content-ID, model-name, source-game,
      or map-specific runtime branches.
- [ ] Runtime extensions may generalize tables, metadata, formats, or missing
      engine capabilities for all mod content. If a content-specific code
      exception appears unavoidable, block the goal and present the exact plan
      for user review before implementing it.
- [ ] Keep the goal open. It may be paused or blocked for regular user review,
      but only the user may declare it complete.
- [ ] Stage every V8:2 test build and mod only in `V8_2_LOOSE`; never place
      `Vigilante82PC.exe` or V8:2 mods in `PS1 game`.

## User-entered priorities

- [ ] Detect the connected Windows controller family (Xbox, PlayStation, and
      supported alternatives) and show the corresponding controller model in
      the controls UI. This remains a wishlist item.
## V8: 2nd Offense issue register

Test and deployment target: `V8_2_LOOSE` only.

### V82-OPEN-012 - Balance default music volume

- [ ] Measure the default CDDA, voice, and SFX mix and choose a balanced music
      level for a clean configuration.
- [ ] Preserve user volume changes across restart.
- [ ] Obtain audible acceptance that music neither clips nor overpowers voice
      or SFX; muted visual automation cannot close this item.

### V82-OPEN-015 - Four-player local support (deferred post-release)

- [ ] Add normal-UI setup for two, three, and four local players.
- [ ] Support independent devices/input, stable 2x2 and three-player layouts,
      correct camera/HUD ownership, pause, results, scoring, and teardown.
- [ ] Pass representative-arena soak testing and sequential visual review.

### V82-OPEN-017 - UI option to disable transformation powerups

- [ ] Add a normal in-game option and persist it in user settings.
- [ ] Apply it through the authoritative gameplay path for human and AI
      vehicles in every mode while leaving unrelated pickups/rules unchanged.
- [ ] Report the active setting in the UI and verify that re-enabling it fully
      restores native behavior.

### V82-OPEN-018 - Port the original Vigilante 8 UI (PARTIAL)

- [x] Add the first reusable presentation slice as a loose mod: editable
      Classic main-menu artwork, content-addressed backplate replacement, and
      the original V8 tire converted into V8:2's shared selector asset.
- [ ] Add a selectable original-V8 presentation covering menus, selectors,
      loading, HUD, pause/objectives, results, fonts, icons, and transitions.
- [ ] Retain a clean route back to the native V8:2 presentation.
- [ ] Cover every reachable state without mixed-theme leakage or broken layout,
      including imported/native content and 4:3/widescreen presentation.

### V82-OPEN-019 - Original Vigilante 8 quests in the native quest system

- [ ] Load original Vigilante 8 quest records through the existing V8:2 quest
      system and select them from the current character/vehicle identity.
- [ ] Do not add a separate original-V8 quest menu. Reuse the native quest
      route, mission-selection, briefing, gameplay, results, and progression
      flow for both native and imported character/vehicle quest sets.
- [ ] Preserve native V8:2 quests unchanged while mapping every imported quest's
      arena, objectives, actors, success/failure state, rewards/unlocks, save
      state, teardown, and return-to-menu behavior.
- [ ] Validate the complete original-V8 quest roster through normal UI entry,
      repeated mission transitions, success and failure paths, restart, save and
      reload, and switching between native and imported vehicles.

### V82-OPEN-023 - Character and AI selector audio is incorrect

- [ ] Trace stock V8:2 character-selection and AI-selection audio events,
      including navigation, confirmation, voice, music, and transition state.
- [ ] Restore the native V8:2 event ordering, sound identities, channel use,
      interruption, and teardown without character-specific substitutions.
- [ ] Validate stock and imported rosters in one-player and two-player flows,
      repeated traversal, player-to-AI handoff, confirmation, cancellation,
      match loading, and return to the selector.

### V82-OPEN-024 - Map-by-map renderer fidelity release gate

Current matrix and evidence index:
`notes/v82_map_fidelity_release_gate.md`. The current Enhanced-renderer
candidate is rejected as a regression. Do not treat its improved tire rendering
as accepted until the other regressions are corrected and the complete result
is reviewed again.

- [ ] Fix mixed loading-screen glyph quality: HD loading artwork is active, but
      individual letters still render blurry among sharp neighbouring letters.
- [ ] Fix the hard terrain-cell transition into the lighter distant
      representation and the associated heavy terrain/scene pop-in.
- [ ] Add verbose, image-light diagnostics before the next attempt: record
      per-glyph replacement/provenance decisions and per-frame, per-terrain-cell
      visibility, detail/LOD, atlas, modulation, and transition decisions in a
      detailed log file.
- [ ] Re-review tires with the corrected build. They appeared improved in the
      rejected candidate, but are explicitly not accepted yet.
- [ ] Fix the repeated-match lifecycle regression reported on 2026-08-29:
      during the third match setup, vehicle textures dropped out and framerate
      became atrocious; textures recovered when the third match entered
      gameplay, and the fourth setup appeared normal. Verify at least four
      consecutive setup/gameplay/return cycles without texture loss or
      progressive frame-time degradation.

- [ ] Compare every playable map against original-game gameplay reference at
      representative spawn, traversal, close-range, horizon, weather/fog,
      water, effects, destruction, and lighting views before release.
- [ ] Record source-reference timestamps and deterministic matching PC routes;
      assess geometry visibility/clipping, palette and lighting, authored fog,
      draw distance, transparency, depth ordering, terrain seams, sky/horizon,
      water, props, and HUD compositing.
- [ ] Correct shared renderer or conversion contracts rather than adding
      map-name, slot, asset-path, or packet-address exceptions. Treat Florida
      Launch Site's reported grey/foggy presentation and clipping as the first
      open comparison case.
- [ ] Keep each map open until automated diagnostics and sequential visual
      comparison both pass; retain compact evidence and verbose logs without
      accumulating disposable captures.

## Additional V8:2 work

- [ ] Internalize remaining wrapper functionality into the game UI so the
      external header/menu bar can be removed.
- [ ] Add fully customizable keyboard/controller bindings plus Modern, Classic,
      and Southpaw presets in the native Options/Controls UI at stock visual and
      interaction parity.
- [ ] Fix Enhanced-renderer close-range scenery clipping where static polygons
      disappear or corrupt as the camera approaches. Begin from a deterministic
      same-camera reproduction; the rejected selective-NCLIP experiment is not
      an accepted basis.
- [ ] Correct Houston's converted special weapon so its projectiles acquire and
      track enemy targets with original-V8 behavior. Fix conversion metadata or
      the shared imported-weapon contract; do not branch on Houston's name,
      vehicle ID, or weapon ID in runtime code.
## User review and signoff

- [ ] Review physical-controller feel on the user's hardware: buttons, sticks,
      triggers, dead zones, hot-plug, and rumble.
- [ ] Visually accept every arena and both split-screen layouts after automated
      stability gates pass.

## Deliberate boundaries

- The reference executable remains Windows x64; the shipping Xbox game is a
  separate 32-bit x86 implementation.
- Standalone loose mode must never discover, open, or fall back to BIN/CUE data.
- Renderer, raw controls, and host audio output are rewrite seams; gameplay
  state and timing remain the fidelity target.
