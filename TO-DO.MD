# Vigilante 8 PC - Open Work Only

Last updated: 2026-09-10 EDT
Status: ACTIVE

This tracker contains only open or explicitly deferred work. Remove an entry
when the user accepts or closes it; completed work, evidence history, and
superseded attempts do not belong here.

## Open items at a glance

This is one line per major open deliverable or defect. Detailed implementation
and acceptance work remains below. `Codex` marks
work added by Codex.

**Work-selection rule:** When the user asks Codex to find or choose something
to do, start with the highest `NEXT PRIORITY` item before considering any other
open work.

1. **IN PROGRESS / NEXT PRIORITY** - Finish shared renderer visual correctness using PS1 runtime assets; Dreamcast is an RE reference only. Preserve accepted widescreen and geometry fixes.
2. Reusable cross-game vehicle/map conversion infrastructure with no content-specific runtime hacks.
3. **PARTIAL / PENDING REVIEW** - Soundtrack choice implemented; balance the
   default music, voice, and SFX mix.
4. **PENDING MENU UPDATES** - Four-player local multiplayer.
5. **PARTIAL** - Finish the sole original-V8-style UI; review remaining fonts and menu layouts.
6. **Codex** - Complete physical-controller and arena/split-screen user signoff.
7. **ACCEPTED** - Character/AI carousel audio repair accepted; dedicated two-player coverage remains.
8. Wire original Vigilante 8 quests into the existing quest system and audit every quest-bearing character.
9. **PENDING REVIEW** - Verify the shared main-menu reset that prevents a prior imported character from remaining stuck on Y.

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

## V8: 2nd Offense issue register

Test and deployment target: `V8_2_LOOSE` only.

### V82-OPEN-012 - Balance default music volume

- [ ] User acceptance of Audio > Soundtrack: Vigilante 8 (default), Second
      Offense, or Both. Original music is installed; selection, playback,
      full-song fidelity, and native menu checks pass.
- [ ] Measure the default CDDA, voice, and SFX mix and choose a balanced music
      level for a clean configuration.
- [ ] Preserve user volume changes across restart.
- [ ] Obtain audible acceptance that music neither clips nor overpowers voice
      or SFX; muted visual automation cannot close this item.

### V82-OPEN-015 - Four-player local support (pending menu updates)

- [ ] Add normal-UI setup for two, three, and four local players.
- [ ] Support independent devices/input, stable 2x2 and three-player layouts,
      correct camera/HUD ownership, pause, results, scoring, and teardown.
- [ ] Pass representative-arena soak testing and sequential visual review.

### V82-OPEN-018 - Port the original Vigilante 8 UI (PARTIAL)

- [ ] Finish the original-V8-style presentation covering menus, selectors,
      loading, HUD, pause/objectives, results, fonts, icons, and transitions.
- [ ] Ship this as the sole final UI, not an alternate/selectable theme;
      there is no user-facing switch back to the V8:2 presentation.
- [ ] Review remaining fonts and shared-GAME.FNT layouts outside the pause
      menu. The pause menu now uses the preferred earlier Rockwell Regular
      replacement through the corrected all-position file-font hook; see
      `notes/v82_pause_style.md`. Keep the cyan
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

### V82-OPEN-023 - Character and AI selector audio (repair accepted)

- [x] User accepted the carousel audio repair ("That works," September 8).
      Native source identity, timing, pitch and cancellation checks are recorded.
- [ ] Cover two-player use and repeated return-to-selector on user hardware.
      The live-bank overwrite and delayed/random-pitch imported acceptance
      defects are repaired; see `notes/v82_selector_audio_repair.md` for the
      staged build, native recordings and verification limits.

### V82-OPEN-024 - Map-by-map renderer fidelity release gate

**CURRENT GOAL: shared renderer visual correctness with PS1 runtime assets.**
Dreamcast is an RE reference only, not the asset source or sole visual authority.
Preserve accepted widescreen, geometry and texture fixes.

**2026-09-08 status:** The user accepted rigid deployed pontoon placement;
that repair is closed. Remaining visual follow-up includes the unreproduced
Oilfield sky sliver, residual faceted shoreline
intersections, and broader arena/edge-view acceptance. Current repair evidence
and limits are in `notes/v82_visual_issue_fixes.md` and
`notes/v82_rigid_water_attachments.md`. Older build reports below are historical
and do not describe the current executable. Do not start performance runs
without a new user request; other-level efficiency work remains deferred.

**Current acceptance focus:** visual correctness and complete HD-texture-pack
use. Record FPS with proofs, but do not hold back visually correct evidence
for a performance score unless the build is unplayable. Further performance
work requires a new user request.

**Current review build (2026-09-07):** `V8_2_LOOSE/Vigilante82PC.exe`
SHA-256 `D8065C9E4C9B0F931A8D604C95BB9C5D275A8F304D73477F03E7F70591317F0E`.
Direct executable contracts now cover the Dreamcast sky, fog, two-pass water,
terrain-mip, stable ordinary-object/building material rules, and the complete
ordinary-object mesh near-plane clipping/submission boundary. A shared
function-owned panorama batch fixes the newly exposed white widescreen sky
margin without map or texture exceptions. Florida, Louisiana, and Hoover Dam
completed one hidden/silent Enhanced-only process with every functional, HD,
terrain, fog, route, edge-pool, and no-content-branch gate passing; 1,294
traced panorama frames had zero edge-coverage failures. Florida's full-size
visual run measured 58.01 median / 39.48 minimum FPS. A separate 4:3 Florida
gate passed at 60.01 / 59.15, proving the widescreen-only continuation leaves
the original-aspect path unchanged. Two additional full-size proofs were then
captured directly from that deployed executable on the same deterministic
Florida route: poll 150 (near/mid/horizon, 39.28 median FPS with a 4.81 startup
capture transient) and poll 300 (mid/horizon, 46.15 median / 31.20 minimum
FPS, every gate passing). Both use the HD terrain/loading replacements and
show stable ordinary-object sampling; the separate 1,200-tick owner trace
recorded zero same-surface material or replacement-identity changes across
137 objects. Deployed-build poll-300 proofs now also cover Louisiana and
Hoover Dam: both passed their functional/HD/route/edge checks and frame the
near/mid/horizon scene, while the rejected Louisiana rectangular overlay and
Hoover serrated detail cutoff are absent. Their isolated proof runs were slow
(26.75/14.37 and 30.27/11.25 median/minimum FPS), so they are visual-only
evidence pending the separate performance work. The five-map set now also has
deployed Utah and Minnesota poll-300 frames. Utah no longer shows the prior
isolated fullbright/jagged distant triangle, but remains user-ungraded.
Minnesota's frame is too close to its billboard and contains ambiguous dark
geometry at the upper viewport boundary, so it does not clear the previous
missing/backface-cull failure; a better mid-distance proof is still required.
Their isolated proof runs measured 34.47/10.25 and 17.10/5.66 median/minimum
FPS respectively, with Minnesota independently unplayable. User visual
approval remains open.

- [ ] Recover and document the complete relevant Dreamcast render contract,
      including terrain traversal/subdivision and distance transitions;
      shoreline geometry and land/water composition; water surface, blending,
      depth, and fog behavior; sky/horizon composition; texture/material/LOD
      selection; and building submission, culling, lighting, and near/far
      texture behavior. Record executable addresses, tables, state fields, and
      confidence for every recovered rule.
- [ ] Implement the recovered behavior through shared Enhanced-renderer paths.
      Preserve HD texture replacement support and all accepted widescreen draw,
      projection, near-edge, and viewport-coverage fixes. Validate both 4:3 and
      widescreen so Dreamcast parity cannot regress the wider field of view.
- [ ] Correct land/water boundaries so shorelines meet cleanly without hard
      rectangular bands, gaps, overdraw across drivable land, or incompatible
      near/far representations.
- [ ] Verify whether the crate
      appearing beside the damaged tree is its authored destruction reward or
      an incorrect object substitution; do not assume either from a still image.
- [ ] Eliminate Florida buildings changing textures or material appearance as
      the camera approaches. Prove the underlying shared texture/material/LOD
      rule and check representative buildings on other maps; do not patch the
      Florida assets or identities.
- [ ] Regression-test terrain distance transitions, sky/horizon, water,
      buildings, transparency/reflections, winding/culling, close and
      viewport-edge geometry, HD replacements, and the prior alternate-frame
      corruption on representative maps using hidden, silent, Enhanced-only
      runs with at most one game process.
- [ ] Retain full-size near/mid/horizon visual proofs from the exact deployed
      `V8_2_LOOSE` executable, with executable SHA-256, capture point, map,
      aspect ratio, and FPS recorded. Technical gates cannot close this goal;
      final visual approval is the user's.

Current matrix and evidence index:
`notes/v82_map_fidelity_release_gate.md`. The current Enhanced-renderer
build `5DB44B52...5AC0B` corrects the global terrain A/B flash caused by
reconstructed GTE projection coordinates omitting the active PS1 framebuffer
origin. Florida and Utah each completed a 20-frame fixed-camera/pause burst
with zero large world swaps while Dreamcast terrain reconstruction remained
active. The exact executable deployed to `V8_2_LOOSE` then repeated the
Florida burst Enhanced-only in one process: clean exit, no fatal diagnostics,
59.86 median / 52.49 minimum FPS, and settled adjacent frames changed only
0-2,211 pixels instead of the broken build's 840,000+ pixel A/B swaps. Keep
this correction pending hands-on user confirmation; automated frame parity
does not close the visual gate.

The earlier Enhanced-renderer
candidate `F881E249...AB418` passed two consecutive four-match technical gates
covering Route 66, Olympic, Bayou, and Florida. Both runs retained every HD
asset and renderer contract, completed all setup/gameplay/return transitions,
and kept every measured 60-frame window above 40 FPS. The prior third-match
performance collapse and fourth-match stale-overlay crash are technically
resolved. Per-map review is now 1 pass, 6 fails, and 12
ungraded: Arizona Meteor Crater passes; Utah Winter Games fails for distant
objects that remain jagged and fullbright; Florida Launch Site fails because
translucent rectangular water-surface polygons draw across dry drivable
terrain; Pennsylvania Steel Mill fails for a hard-edged mist band and
close-range building-edge/geometry clipping that may affect other maps;
Minnesota Nuclear Plant fails for missing or backface-culled triangles and
inconsistently bright adjacent polygons; Alaska Alaskan Pipeline fails for the
recurring hard-edged translucent rectangular surface/band, whose mist-versus-
water identity remains unconfirmed; Louisiana Ghastly Bayou fails because its
water edge remains visibly jagged;
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
- [ ] Investigate original-game texture-allocation failures during guest
      loading: `artifacts/transformation-modes-20260904/guest-recovery/runtime.log`
      reports `Out of VRAM` while explicitly running Enhanced. The existing
      recovery can omit affected textures; this is not PC GPU exhaustion or an
      old-renderer run. Keep separate from the previously verified dispatch
      lifecycle fix below; guest visual texture integrity is not passed.
      Reproduced during water-attachment verification on 2026-09-05:
      `artifacts/water-attachment-fit-20260905/fit-chassey-visual/` shows
      corrupted body/attachment textures. Allocation errors occur before
      gameplay and the attachment-fit hook; imported attachment visual
      confirmation remains blocked by this issue, not passed by buoyancy logs.
      Subsequent source inspection found that the test's forced type bypassed
      carousel registry selection and therefore guest reservations. Correct
      that fixture before treating this run as evidence of a normal-launch
      failure; this allocation issue remains open pending valid reproduction.
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
