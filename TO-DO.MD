# Vigilante 8 PC - Open Work Only

Last updated: 2026-09-04 EDT
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
4. **User** - Install and prefer original V8 music; balance the default music,
   voice, and SFX mix.
5. **User** - Add four-player local multiplayer after release.
6. **User** - Add a persistent option to disable transformation powerups.
7. **User — PARTIAL** - Finish the sole original-V8-style UI; review remaining fonts and menu layouts.
8. **User** - Move player-facing features from the Windows menu bar (Mods, Cheats, Guest Vehicles, etc.) into controller-friendly in-game menus, then remove that external bar.
9. **User** - Add customizable controls with Modern, Classic, and Southpaw presets.
10. **User** - Fix shared near-object viewport-edge clipping, including scenery and other vehicles.
11. **Codex** - Complete physical-controller and arena/split-screen user signoff.
12. **User** - Fix incorrect audio during character and AI selection.
13. **User** - Wire original Vigilante 8 quests into the existing quest system and audit every quest-bearing character.
14. **User** - Fix repeated-match texture loss and severe framerate degradation.
    Latest UI proof run still fails the performance floor (52.51 median /
    10.65 minimum FPS); see `notes/v82_pause_style.md`. UI changes do not close this.
15. **User** - Correct Houston's converted special-weapon enemy tracking.
16. **User — PENDING REVIEW** - Verify the shared main-menu reset that prevents a prior imported character from remaining stuck on Y.

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

- [ ] Install the original Vigilante 8 music and make it the preferred
      soundtrack, retaining the V8:2 music as the fallback when the original
      tracks are unavailable.
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

- [ ] Finish the original-V8-style presentation covering menus, selectors,
      loading, HUD, pause/objectives, results, fonts, icons, and transitions.
- [ ] Ship this as the sole final UI, not an alternate/selectable theme;
      there is no user-facing switch back to the V8:2 presentation.
- [ ] Review remaining fonts and shared-GAME.FNT layouts outside the approved
      pause menu. See `notes/v82_pause_style.md` for the accepted smaller
      Rockwell Extra Bold font and panel. Keep the cyan
      font unchanged; cursive source fonts remain in scope but become
      non-cursive. Use the shared per-file font hook, with original-font fallback
      where an approved HD replacement is unavailable.
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
- [ ] Perform a full quest audit across native V8:2 and imported original-V8
      characters. Every character with at least one quest must appear in the
      normal character-select carousel, cycle in the correct order, remain
      selectable, and route to that character's correct quest set.
- [ ] Fix the currently broken quest character-select eligibility/cycling
      through the shared roster and quest tables. Do not hide missing entries,
      inject a character-specific menu branch, or treat direct mission loading
      as proof that normal character selection works.

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
candidate `F881E249...AB418` passed two consecutive four-match technical gates
covering Route 66, Olympic, Bayou, and Florida. Both runs retained every HD
asset and renderer contract, completed all setup/gameplay/return transitions,
and kept every measured 60-frame window above 40 FPS. The prior third-match
performance collapse and fourth-match stale-overlay crash are technically
resolved. Per-map review is now 1 pass, 5 fails, 1 provisional fail, and 12
ungraded: Arizona Meteor Crater passes; Utah Winter Games fails for distant
objects that remain jagged and fullbright; Florida Launch Site fails because
translucent rectangular water-surface polygons draw across dry drivable
terrain; Pennsylvania Steel Mill fails for a hard-edged mist band and
close-range building-edge/geometry clipping that may affect other maps;
Minnesota Nuclear Plant fails for missing or backface-culled triangles and
inconsistently bright adjacent polygons; Alaska Alaskan Pipeline fails for the
recurring hard-edged translucent rectangular surface/band, whose mist-versus-
water identity remains unconfirmed; Louisiana Ghastly Bayou is a provisional
fail pending comparison of its hard-edged mist band against PS1 gameplay;
  Super Dreamland 64 is ungraded because earlier issue-level passes were not
  whole-map certification. Shared screen-effect depth and monotonic-NCLIP
  corrections plus restored native translucent ordering were staged in exact
  candidate `BA9BE3EC...B29C`; the user reports the Florida and Alaska
  rectangular-band defects fixed, but neither has an explicit whole-map pass,
  and Louisiana exposed interiors through reflective vehicle surfaces. The
  user subsequently identified the affected vehicle as Sheila's stock V8:2
  vehicle, not the original-V8 Strider. Current staged candidate
  `A0470F59...C15B` is rejected: its own Hoover proof visibly retains Sheila's
  transparent/missing body surfaces and shows a serrated detailed terrain band
  ending at a hard boundary against flat dark distant terrain. The earlier
  focused visual-pass claims are withdrawn. Both two-map lifecycle orderings
  also fell below the release performance floor.
  All prior failed/provisional maps remain pending user retest. Continue to correct any
  remaining failures through shared renderer/conversion contracts with no
  map-, object-, cell-, address-, or authored-content-specific workaround.

- [ ] Fix Sheila's stock V8:2 vehicle roof/body transparency through the
      shared vehicle primitive, culling, reflection, glass, depth, and texture-
      keying contract. Candidate `5348CEE1...84A0B` was rejected. The exact
      deployed replacement `71618E6B89D311CC47522B0AAB1A8EAB873E5E054B6E3A5D426ECBBE4B0FA8DD`
      restores the native ordered-table composition through the shared renderer
      and shows an opaque roof/body in the retained Louisiana proof, but remains
      open pending the user's visual approval. Do not add a Sheila-,
      vehicle-, map-, CLUT-, texture-, packet-address-, or authored-content
      exception.
- [ ] After the roof fix, produce approximately five useful visual proofs—one
      retained screenshot per selected map—from the exact deployed Enhanced
      build in `V8_2_LOOSE`. Use Louisiana, Hoover Dam, Utah Winter Games,
      Florida Launch Site, and Minnesota Nuclear Plant unless later evidence
      makes another map more diagnostic. Each frame must deliberately expose
      near, middle, and horizon terrain so the distance transition, texture
      detail, color/fog composition, jagged boundaries, and object draw can be
      judged; incidental driving frames are not acceptable. Louisiana must
      also show Sheila's roof/body clearly enough to verify that it is opaque.
      Current candidate `71618E6B...FA8DD` proof set is under
      `artifacts/v82-shared-sheila-distance-goal/subdivision-final-20260904/`.
      Superseded `13316E77` images do not establish terrain parity. No map
      is accepted until the user grades the current full-size image.
- [ ] Record the executable SHA-256, map identity, capture point, and measured
      FPS beside each of the five proofs. Keep every map and the terrain-
      distance decision pending until the user can grade the full-size images;
      logs, hash matches, and automated gate results do not substitute for the
      requested visual evidence. Metadata is recorded for candidate
      `71618E6B...FA8DD`: every proof is gameplay capture poll 601 with the
      proof-only transform hold established at frame 30; exact player addresses,
      authored coordinates, and median/minimum/tail FPS remain with each proof's
      acceptance artifact and must accompany presentation to the user.

- [ ] Visually re-review loading-screen glyph quality on the current candidate.
      The file-font hook and exact live replacement-provenance gate pass, but
      technical evidence does not substitute for user acceptance. Exact F881
      proof: `olympic-loading-quest-f881-current-20260831`; its PNG is pixel-
      identical to the prior accepted 6C3 loading proof.
- [ ] Visually re-review the prior hard terrain-cell transition into the lighter
      distant representation and associated terrain/scene pop-in. The current
      gates pass the shared detail/coarse/transition contracts; exact F881
      proof: `olympic-f881-visual-20260831/olympic-f881-current.png`.
- [x] Add verbose, image-light diagnostics before the next attempt: record
      per-glyph replacement/provenance decisions and per-frame, per-terrain-cell
      visibility, detail/LOD, atlas, modulation, transition, frame pacing,
      renderer batching, vehicle ownership, and overlay-lifecycle decisions in
      detailed log files.
- [ ] Re-review tires with the corrected build. They appeared improved in the
      rejected candidate, but are explicitly not accepted yet. The exact F881
      Olympic proof exposes round stock wheels without the square interior.
- [ ] Visually review the corrected original-V8 reflection/glass conversion.
      Source material bit `0x20` is now treated as the original game did—as an
      ignored dialect bit—rather than being mistranslated into V8:2's `0x10`
      semitransparency bit. The resulting Strider trace contains 1,928 opaque
      reflection triangles and no translucent reflection triangles; its 696
      glass-tagged triangles use the shared opaque vehicle-glass path. The
      later user correction establishes that the reported transparent vehicle
      was Sheila's stock V8:2 vehicle, not Strider, so it does not invalidate
      the converter diagnosis. It did expose a separate shared native V8:2
      reflection-composition failure. Candidate `A0470F59...C15B` attempted to
      make semitransparent reflection packets depth-test the opaque body without
      replacing its depth, but the Hoover proof visibly shows Sheila's body
      hollow/transparent; that screenshot is a failure, not proof. Keep the
      issue open, with no vehicle-,
      Louisiana-, texture-, packet-address-, or authored-content exception.
- [ ] Fix and visually review the shared distant-terrain transition/LOD
      contract. On Hoover Dam in current candidate `74E76336...74C8E`, far
      terrain visibly changes to an inconsistent representation/detail level.
      Trace coverage proved all 2,048 sampled detail/coarse/transition cells
      selected the HD atlas, but that alone did not identify the geometry or
      transition failure. Candidate `A0470F59...C15B` is rejected: its Hoover
      proof shows a serrated/jagged detailed strip terminating abruptly into a
      flat dark coarse surface. Attribute the exact primitives, geometry route,
      depth, shade, and texture source on both sides before changing the shared
      contract again. No Hoover-, map-, terrain-cell-, texture-, or authored-
      content exception. Replacement candidate `13316E77...D29C4` implements
      the Dreamcast-authored complete-quad diagonal and preserves truly clipped
      single halves globally; five near/mid/horizon proofs exist, but this item
      remains open until the user visually approves the distance treatment.
      Subsequent binary audit found a missing 4/2/1 recursive subdivision and
      height/color-morph path at Dreamcast `0x8C10238C`; forced four-unit
      geometry in `13316E77...D29C4` is therefore NOT qualified for parity.
      Current `71618E6B...FA8DD` implements recovered shared subdivision,
      intermediate height/color morph and per-leaf materials; numerical and
      binary-anchored tests pass. User visual approval remains outstanding.
      Held Hoover proof recorded 56.11 median / 29.21 minimum FPS and fails
      the performance floor, despite the ordinary Hoover traversal passing at
      59.96 / 55.70. Retain and investigate that failure; do not substitute the
      ordinary run's FPS for the proof's.
      The current Minnesota proof also fails at 21.38 median / 7.41 minimum
      FPS and still shows broken building polygons/bright distant objects.
      These failures remain open; the candidate is not release-ready. See
      `notes/v82_dreamcast_terrain_distance_re.md` and the binary-anchored
      decision reference `tools/dreamcast_terrain_contract.py`.
- [x] Fix the repeated-match lifecycle regression reported on 2026-08-29:
      during the third match setup, vehicle textures dropped out and framerate
      became atrocious; textures recovered when the third match entered
      gameplay, and the fourth setup appeared normal. Verify at least four
      consecutive setup/gameplay/return cycles without texture loss or
      progressive frame-time degradation. Exact `F881E249...AB418` completed
      that sequence twice; relocated overlays are retired when their live RAM
      region is reused, so a prior level can no longer pollute the shell or the
      next level's dispatch table.
- [x] Retain the first-release performance gate: sustained median and tail
      windows must remain at or above 40 FPS, and any measured 60-frame window
      below 30 FPS is an automatic failure. Two exact four-cycle runs passed;
      their worst windows were 44.74/40.24 FPS in the first/repeat runs, with
      all per-map median and tail values above 40 FPS. User visual/play review
      is still required. The current `A0470F59...C15B` focused runs pass
      individually (Bayou 42.07/31.75 FPS median/minimum; Hoover 46.34/30.98),
      but both two-map orderings fail the gate, reaching 27.40 and 27.90 FPS.
      Do not promote this exact build to a release candidate until the shared
      repeated-map performance failure is resolved; do not weaken the gate.
- [ ] Visually re-review close-range scenery clipping on the current exact
      candidate. Exact F7D2 preserves every native-admitted NCLIP face while
      retaining precise false-negative rescue, and its image-free Utah ->
      Pennsylvania -> Alaska run passes with HD terrain and no repair-pool
      drops; this does not substitute for user acceptance at the reported close
      Pennsylvania/Minnesota/Utah angles.

### V82-OPEN-025 - Repeated-return character selection sticks on Y

- [x] Reproduce selecting Y, completing/exiting a match, returning to the
      main menu, and then choosing a different character through the normal UI.
- [x] Clear the imported/guest vehicle selection override at the authoritative
      return-to-main-menu lifecycle boundary so the next accepted carousel
      choice becomes authoritative.
- [x] Preserve intentionally selected characters within one setup flow and do
      not add a Y-, character-, map-, slot-, or address-specific reset.
- [ ] Verify at least five setup/gameplay/return cycles while alternating
      native and imported characters; each selected portrait, vehicle, quest
      identity, and in-game spawn must agree. The focused two-match gate passes:
      match one accepted/spawned Y type 76, main-menu entry two cleared the
      imported override, and match two accepted/spawned retail type 0 with no
      second Y construction. User review remains required.

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

- [ ] Move player-facing features currently accessed through the Windows menu
      bar above the game (including Mods, Cheats, and Guest Vehicles) into the
      game's own controller-friendly menus. Once those features are reachable
      in-game, remove the external menu bar. This is functional integration,
      separate from #7's visual redesign; developer-only diagnostics need not
      become player-facing menu entries.
- [ ] Add fully customizable keyboard/controller bindings plus Modern, Classic,
      and Southpaw presets in the native Options/Controls UI at stock visual and
      interaction parity.
- [ ] Fix Enhanced-renderer close-range object clipping where polygons
      disappear or corrupt as the camera approaches. Begin from a deterministic
      same-camera reproduction; the rejected selective-NCLIP experiment is not
      an accepted basis. The current exact candidate passes the image-free
      Route 66 edge-crossing fixture, but user visual acceptance remains open.
      User reconfirmed side clipping on 2026-09-04: the orange safety fence in
      original V8 Ski Resort loses triangular sections along the left view
      edge. Reference: `C:/Users/smmel/AppData/Local/Temp/codex-clipboard-e44f8f48-ea70-46b1-b88f-05c1adc8587f.png`;
      reproducible full-size capture on executable `71618E6B...FA8DD` is
      `artifacts/v82-shared-sheila-distance-goal/subdivision-final-20260904/utah/recompone_present_gameplay_0601_1920x1080_fxaa.png`
      (this directory contains Ski Resort slot 17, not Winter Games).
      Revisit the shared object clipping/projection contract, not a fence-,
      map-, or authored-polygon exception. The roof/distance proof goal was
      paused by the user to prioritize this shared defect. This issue remains
      open and is not passed by image-free technical checks.
      User clarification: this affects ANY nearby object at the viewport
      edges, including other vehicles, not only fences/static scenery.
      Current focus is the shared defect; validate near-edge buildings,
      fences and other vehicles, preserving roof/body opacity and normal
      offscreen rejection. The fence is a reproduction, not the fix's scope.
      2026-09-04: shared exact-mesh submission bypasses the native PS1
      screen-space subdivider for all four triangle packet forms. The
      same-camera full-size fence proof is corrected without shifting the
      cables or reopening Sheila's roof; 159 packet/provenance tests pass.
      A full-size native AI proof now includes a bus at the left edge and
      another vehicle at the right edge without missing edge wedges;
      59.94 FPS median / 51.04 minimum, executable `523CC0F9...493AEB`.
      Dedicated building-edge proof and user approval are still pending.
      Evidence and rejected isolations: `notes/v82_near_object_side_clipping.md`.
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
