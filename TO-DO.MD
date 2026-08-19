# Vigilante 8 PC - Master To-Do and Issue Tracker

Last updated: 2026-08-16 EDT
Status: ACTIVE

This is the canonical tracker for the original Vigilante 8 reference work and
Vigilante 8: 2nd Offense. Numbered V8:2 issues, acceptance conditions, signoff
history, and required verification all live here.

## User-entered list

1. Widescreen support
2. Support for higher resolution textures. Possibly make UI shapes runtime-drawn instead of textured
3. Support for modded content, including V8: Second Offense and Star Wars Demolition content, and ports of Interstate 76 cars.
   Interstate '76 `I76.ZFS` source staging is complete in ignored `I76_ZFS/`; conversion of the 34 inventoried `.vdf` vehicle candidates remains deferred. User has character images for vehicle select screen.
4. Windows detection of controller type (Xbox, PS, etc) and a custom model for each in controls (wishlist)
5. "advanced" (PS2 era) lighting options. Bettter lights generally, but also shadows
6. Quest mode completion: each of the fifteen quests must be playable to the end and must unlock the
   character it awards. Entering quest mode is verified for all fifteen (see notes T74); stage-by-stage
   progression and the unlock itself are not started.
7. Port Y the Alien from the original Vigilante 8. (Assigned to Codex, not this agent.)

## Closed
- Updating of in-game menus so that options like resolution, controls, etc are not external
- Game will occasionally freeze for 2-4 seconds
- Pause menu layout broken in widescreen: entries offset from the border and overlapping, and the
  whole modal off-centre. Widescreen HUD anchoring was moving whatever passed its top-of-screen
  test and leaving the border, which is drawn as flat 2D triangles rather than rectangles. Modals
  are now exempt from anchoring and their border is shifted by one margin. Verified headlessly at
  both aspects: 16:9 reproduces the 4:3 layout to within half a percent of border width, border
  centred. See notes/v82_pause_menu_widescreen.md.
- Vehicles will not reverse; brakes stop but do not reverse. The engine's reverse was never broken.
  `settings.json` resolved against the launch directory, so the bindings in force depended on where
  the game was started from; a Classic copy leaves brake/reverse on the stick and D-pad only, with
  face buttons on "Brake" and LT on fire. Config now resolves next to the executable and the active
  profile is logged at startup (2a791d0). See notes/v82_reverse_input_investigation.md.
- Game froze when starting Chassey Blue's quest; smoke quest mode with each character.
  Quest mode killed the process on a NULL quest-briefing record (the code then read the
  PS1 exception vector and used a BIOS instruction word as a string pointer). Fixed in
  85e7517 by making unmapped reads return zero while unmapped writes still throw. All
  fifteen Quest-capable characters now enter and run, Chassey Blue included. Playing each
  quest through to its unlock is tracked separately as open item 9.
- Fix PS1 texture projection
- Add LOD options - draw terrain at highest LOD always, same with vehicles and weapons. Also add a draw distance option
- V8: 2nd Offense movies/FMVs appeared broken on a stale/earlier deployment: screenshot showed mostly black with a gray block/partial frame. Fresh 2026-07-27 deployment decodes full MDEC frames and captured valid ACTLOGO frames 60/120/180/260; keep broader FMV audit in the sequel UI/presentation goal.

## Current goal

Run a five-hour standalone loose-files crash campaign with the following hard
coverage gates:

- [x] Play every match as Y the Alien.
- [x] Exercise all ten retail levels, explicitly including Sand Factory and
      Secret Base.
- [x] Let every launched level reach the original result screen; elapsed-time
      survival or forced pause exits do not count as completed matches.
- [x] Programmatically attach all six retail weapon kinds through the original
      allocation, callback, bone, and weapon-slot routines.
- [x] Fire every attached weapon in selected-weapon pairs and retain exact
      hit-counter WHAMMY confirmation for kinds 1 through 6.
- [x] Enter both original directional-plus-machine-gun special-attack commands
      for each of the five standard weapons and retain callback/ammo-consumption
      proof for all ten attacks.
- [x] Continue launching varied-map coverage for at least five wall-clock hours,
      then let the final bounded match finish.
- [x] Preserve and classify every crash, hang, unmapped call, early exit,
      liveness false positive, and unmet coverage gate.
- [x] Commit coherent harness or source-backed defect corrections frequently;
      publish the final validated executable byte-identically to `PS1 game` and
      `BINCUE` without changing cards or settings.

2026-07-23 post-soak evidence: the five-hour campaign ran 18,013.526 seconds,
completed 75 bounded runs, naturally completed every arena at least four times,
confirmed all six WHAMMY kinds, and proved all ten special attacks. Retained
failures were classified as heap-validator false liveness, old manual weapon
attachment heap-header overwrites, and an Aircraft Graveyard frame-pacing
liveness sensitivity; no exact `Unmapped call` occurred. Focused fixed-binary
regressions passed Ghost Town, Casino City, Ski Resort, and Aircraft Graveyard
to natural result screens, with Aircraft requiring a 60-second liveness window
because its sampled stack was sleeping in `FrameClock.Throttle`.

2026-07-23 closeout: post-soak fixes removed the test-induced manual weapon
allocation path, gated/sampled expensive heap validation, disabled periodic
display probes by default, and cached tiny VRAM reads. The final published
single-file `Vigilante8PC.exe` is deployed byte-identically to `PS1 game` and
`BINCUE` at SHA256
`7628FBFD9E1B701F0D55B6F0BC016D8ACE0425DD5E95FAC6CFCDFB22D61D07DF`. Source
probes confirmed `PS1 game` selects standalone loose files with 69 assets and
does not open disc media, while `BINCUE` selects the direct CUE. Both deployed
copies passed independent Oil Fields gameplay smoke runs with clean in-game
exits.

## Previous soak goal

Run deterministic soak coverage across multiple retail maps, validate split-screen
gameplay separately, close every actionable reference-handoff gap that does not
require subjective user evaluation, and leave a short explicit sign-off queue.

## Acceptance criteria

- [x] A committed soak runner can select multiple named maps, run them for a
      bounded duration, retain per-map evidence, and fail on crashes, hangs,
      missing gameplay entry, unmapped calls, heap/terrain faults, or stale logs.
- [x] At least one substantial soak pass covers every selectable arena; any
      failing arena is fixed or documented with reproducible evidence.
- [x] Cooperative and Versus split-screen each reach active gameplay in clean,
      independent runs and remain stable under simultaneous two-pad input.
- [x] Direct BIN/CUE and standalone loose-file deployments use the same
      executable and pass focused regressions after any runtime changes; the
      loose deployment must work with the entire `BINCUE` tree unavailable.
- [x] Temporary test processes and handoff logs are cleaned; cards and settings
      are preserved; coherent changes are committed.

## V8: 2nd Offense Issue Register

Last updated: 2026-08-17
Test and deployment target: `V8_2_LOOSE` only. `PS1 game` is reserved for original Vigilante 8 and must not contain `Vigilante82PC.exe` or V8:2 mods.
Reported build SHA-256: `67A272D9DEC7E6891DF702E4BB5044EEFEE6B396D6D36956FEC2D76B8269D53A`
Current staged SHA-256: `A967F82CF252D684B1B469A5D62B2E226F40909D94CF537F59BDBFF2EAE50834`
Current staged size: `16,695,618` bytes in `V8_2_LOOSE`; no V8:2 executable is staged in `PS1 game`.
Current rebaseline: `artifacts/v82-stock-quest-rollback/`

The unchecked summary list contains every genuinely open acceptance item. The
2026-08-14 rebaseline closes 12 of the original 13 reported issues; OPEN-011 was
reopened by a current-build HUD regression and signed off on 2026-08-15,
OPEN-012 remains, and OPEN-014 through OPEN-022 record the current development
backlog except accepted OPEN-020. No open item is considered complete until the
user explicitly signs it off.

### Open items

- [ ] **V82-OPEN-012 - Balance default music volume**
- [ ] **V82-OPEN-014 - Finish regression testing before Xbox C++ generation**
- [ ] **V82-OPEN-015 - Add four-player local support (deferred post-release)**
- [ ] **V82-OPEN-016 - Add generalized N64 map support**
- [ ] **V82-OPEN-017 - Add a UI option to disable transformation powerups**
- [ ] **V82-OPEN-018 - Port the original Vigilante 8 UI**
- [ ] **V82-OPEN-021 - Add the five-car Interstate '76 pack**
- [ ] **V82-OPEN-022 - Add a Star Wars: Demolition landspeeder and level**

### Item details

#### V82-OPEN-011 - Enemy-status HUD backing is missing its left extension

  - Accepted 2026-08-15 by explicit user signoff after visual review of `artifacts/v82-open011-hud/open011_guest_target_hud_before_after.png`.
  - Reopened 2026-08-15 after user testing of the accepted NPC build: the green enemy-vehicle silhouette extends left of its status backing because the accepted six-pixel backing extension is not being applied in this gameplay state.
  - Root cause: the Enhanced renderer identified the backing by retail texture state (`uv=64,90`, `tpage=0x005`, `clut=0x7804`). Tracking an imported NPC reuses the same authored `80,20 84x34` screen rectangle with guest texture state (`uv=100,36`, `tpage=0x006`, `clut=0x7FC8`), so the SVG replacement was skipped and the native backing began at x=80 while the target icon began at x=76.
  - Fix: classify the backing by its stable authored rectangle, viewport-local position, raw/semitransparent UI material, and size instead of volatile VRAM coordinates. The accepted six-pixel extension and all weapon/ammo/health atlas coordinates remain unchanged.
  - Verification: `artifacts/v82-open011-hud/candidate10-acceptance/acceptance.json` and the fresh current deployed-run proof at `artifacts/v82-open011-hud/current-review-20260815/acceptance.json` both pass all 12 imported NPC previews, one guest NPC object, gameplay, and clean exit. Their synchronized HUD traces record the corrected backing at x=74 width=90 and the guest target icon at x=76 width=40. `artifacts/v82-open011-hud/open011_guest_target_hud_before_after.png` shows the rejected guest-texture result beside the corrected complete backing.
  - Review packet: `notes/v82_open011_review_packet.md`.
  - Handoff gate: `artifacts/v82-open011-hud/open011_handoff_verification.json` passes with SHA-256 `A9A6F826FF05E1A7E0EAA24A4A725C846E925B23B1FB5B2B20D290FBA89C6E3E`; it verifies candidate10 acceptance, fresh current deployed-run acceptance, both deployed `Vigilante82PC.exe` hashes, the authored-geometry/material classifier, visual proof dimensions/hash, review packet, unchecked tracker state, zero stale `v8*.log` files, and an explicit objective-requirement matrix. This V8:2 item uses `Vigilante82PC.exe`; `Vigilante8PC.exe` is separate original-V8 Dreamland work and is not the acceptance binary for OPEN-011.
  - Status: CLOSED by user signoff.
  - Pass condition: all measured SVG HUD backings align exactly with their live contents and retain the accepted colors, bevels, spacing, left anchor, opacity, and two-digit ammunition layout across stock and imported player/NPC combinations.

#### V82-OPEN-012 - Default music volume is excessively loud

  - Observed: in-match music is dramatically louder than the rest of the mix at default settings.
  - Pass condition: a clean/default configuration starts with a balanced music level, preserves user changes, and does not clip or overpower voice/SFX.
  - 2026-08-14 status: OPEN. The current clean settings still use host `MasterVolume=1.0`; no measured V8:2 CDDA/voice/SFX balance correction or audible acceptance exists.

#### V82-OPEN-014 - Complete regression testing before Xbox-compatible C++ generation

  - Current groundwork: resolution selection, FXAA, texture-projection corrections, extended draw distance, and maximum LOD are integrated into the game UI. The Enhanced presentation removes dithering, uses the high-resolution texture pack, and includes vectorized UI elements.
  - Remaining work: exercise clean installs, saved-setting migration, every relevant graphics permutation, menu-to-game transitions, all arenas, stock/imported vehicles, multiplayer, Quest flows, pause/results, and long-duration renderer stability before freezing behavior for C++ generation.
  - Pass condition: an exact-build acceptance package covers the complete supported feature matrix with no crashes, stale state, visual corruption, settings mismatch, or unresolved high-severity regression. Xbox-compatible C++ generation remains blocked until explicit user signoff on this item.

#### V82-OPEN-015 - Four-player local support

  - Status: **DEFERRED TO POST-RELEASE (2026-08-16).**
  - The incomplete direct-launch/four-pane prototype was rolled back from runtime source, generated output, runnable settings, deployment, scripted probes, and retained evidence. Release scope remains on the validated 1P/2P behavior.
  - Post-release acceptance: normal-UI 2P/3P/4P setup, independent devices and input, stable 2x2 and 3P layouts, correct camera/HUD ownership, pause/results/scoring/teardown, representative-arena soak testing, and sequentially inspected visual proof.

#### V82-OPEN-016 - Generalized N64 map support

  - Status: **WIP / USER REVIEW HOLD (2026-08-18).** Super Dreamland 64 remains staged only in `V8_2_LOOSE` as a work-in-progress public-release candidate. Its original selected-frame evidence was insufficient and the level does not close OPEN-016 until explicit user review signs off the remaining vehicle-glass and terrain/distance-band limitations.
  - Current groundwork: Super Dreamland 64 is an append-only native location slot in the isolated `mods/v82_n64_super_dreamland` package. It uses the original N64 selector art and loading image, native V8:2 title/tip/prompt flow, converted N64 arena data, original Dreamland overlay behavior, and V8:2-native shared items.
  - Acceptance evidence: Release soak from `V8_2_LOOSE` passed 3240 gameplay frames with weapon kinds 1-7 armed and fired, all 18 applicable scripted special-command variants, all power-up states, 31 callback classes, and zero collision-stream rejections. A second playable passed 900 frames and stock Route 66 passed 360 frames. After removing the misplaced PS1-root payload, a fresh sole-root run from `V8_2_LOOSE` passed 360 Dreamland frames with 17 callback classes and zero collision-stream rejections; its selector, loading, and gameplay proofs in `artifacts/v82-dreamland-v82-loose-staging-proof` were inspected sequentially. The former `PS1 game` smoke is invalid deployment evidence and is superseded by this corrected proof.
  - User-reported first defect batch: the transition briefly shows another map; the Dreamland loading preview corrupts and changes from 4:3 to 16:9 at the end despite 16:9 gameplay; vehicle windows intermittently disappear and expose the vehicle interior/world; distance lighting flips between dark and light states; weapon effects render as squares; and the USA-map icon plus Dreamland loading textures are not upscaled. These are blocking failures, not accepted limitations.
  - 2026-08-17 HUD/text regression fix: after the full-display rectangle repair, user review caught gameplay/story text rendering as large white blocks. The fix keeps true alpha-bearing replacement fonts but falls back to native PS1 glyph alpha for opaque RGB replacement font cells. Deployed `V8_2_LOOSE/Vigilante82PC.exe` SHA-256 `AA7D4AFE6834E2F9870AFD1CCCE999E5CA041E885506F741288DC3E797D78BC2`; exact-root Dreamland proof `artifacts/v82-open016-hud-text-regression-deployed-proof` passes 180 frames with clean caption visual SHA-256 `6539E147896C894D699C8348094317B71463759563B3814723B0A7D4B785B41C`; stock Route66 smoke also passes. This does not close OPEN-016.
  - 2026-08-17 loading-card transition fix: pre-tick Dreamland loading presentation now stays on the native 4:3 loading/UI source and switches to 16:9 only after real gameplay frames. The Dreamland loading-card replacement now matches the authored `320x112` MDEC frame as a `1280x448` asset instead of the rejected `320x96`/`1280x384` crop, and it uses the stock centered loading-card rectangle with a title-preservation mask. Deployed `V8_2_LOOSE/Vigilante82PC.exe` SHA-256 `86CE789DF95B739A3B6A506711620E856CAD07BF979DAFF4461F1276853F2009`; replacement card SHA-256 `F765D61598422949D8F1961B12BEFE4EA0154D2344B6634F56AA7EAE2C1A86B1`; exact-root proof `artifacts/v82-open016-loading-card-layout-deployed-proof` passes 180 frames and captures frames 2568-2582 with the overlay selected between 2572 and 2573, summary SHA-256 `2BE54FB878060C6DB4AFD25173697A4EA61D924D9706A8881B4B20D6E4AED233`, visual SHA-256 `FD521206205A71B5480E8321E4B1767BCFFF368A483676DF846A5CB2BF2650F6`. This does not close OPEN-016.
  - 2026-08-17 current review candidate: Dreamland's highlighted USA-map marker is now replaced in the front-end presentation pass while the stock markers remain native. Current deployed `V8_2_LOOSE/Vigilante82PC.exe` SHA-256 `D50BDA1770D2DC99B207EB7BD78FC40D26467C1AA8769EC0315E660F77B2858D`; selector proof `artifacts/v82-open016-current-location-marker-proof` passes 360 frames and captures 1280x720 frames 1900/1960/1990/2010 with a clean Dreamland preview and crisp marker; focused Dreamland loading acceptance `artifacts/v82-open016-current-dreamland-loading-after-marker` passes with correlation `0.999640`, full-width tire tread, and visible `Press X to start...`. Current-binary Dreamland weapons proof `artifacts/v82-open016-current-marker-full-weapons` passes 2700 gameplay frames with weapons 1-7 armed/fired, 18 special-command variants, `powerups=[]`, 31 callbacks, and zero collision-stream rejections; sequential visual contact SHA-256 `E18C6D9FAF807E7F73773800E8A403D8A7DE845C3A412AA865B29C3548CA8136` shows no hard square card or transform pickup, but frame 3060 still requires user review for the bright weapon/light wash. Current-binary selector/window proof `artifacts/v82-open016-current-marker-selector-window-proof-long` passes the complete native selector lifecycle with all 12 guest captures, complete double roster, enemy stage/capture, clean exit, and `vehicle_materials_passed=true`; contact SHA-256 `A229D14E9AC6CE96EFBA12CA86F884FF176D4EA56153D67FA88675409A7E89CE`. Current-binary NPC/gameplay proof `artifacts/v82-open016-current-marker-npc-gameplay-proof-long` passes the complete guest NPC selector/gameplay lifecycle with all 12 previews, one injected guest NPC, gameplay, and clean exit; visual contact SHA-256 `8033086D2E5188E6F3ABBFFFA19118B86982F88C8DFA4DE72E492F42C5DDD82A`. The process priority remains `BelowNormal`, `RECOMPONE_UNTHROTTLED` was not run or enabled, and `PS1 game/Vigilante82PC.exe` remains absent. This does not close OPEN-016 until explicit user review.
  - 2026-08-17 font-cell current candidate: after the marker build, focused trace caught the in-match story caption falling back to opaque replacement font cells. Opaque RGB font replacements now use the native glyph contour path instead of drawing their replacement cell. Current deployed `V8_2_LOOSE/Vigilante82PC.exe` SHA-256 `74DFB9165D32BD67FAE283C1A7EEB23E9B8E50CBBF12806B922842D350BF34BC`; focused caption/effect proof `artifacts/v82-open016-current-fontcell-fix-proof` passes 900 frames and visually clears the white caption-cell artifact; full weapons proof `artifacts/v82-open016-current-fontcell-full-weapons` passes 2700 gameplay frames with weapons 1-7 armed/fired, 18 special-command variants, `powerups=[]`, 31 callbacks, zero collision-stream rejections, and visual contact SHA-256 `B20EF0098E6C3FF2375658E0D8887645A4DE7A911CF10FA831CC3C771CA5A3DE`; Dreamland loading acceptance `artifacts/v82-open016-current-fontcell-dreamland-loading` passes with correlation `0.999640`, full-width tread, and visible `Press X to start...`; selector/window proof `artifacts/v82-open016-current-fontcell-selector-window-proof` passes complete lifecycle with all 12 captures, clean exit, and `vehicle_materials_passed=true`; NPC/gameplay proof `artifacts/v82-open016-current-fontcell-npc-gameplay-proof` passes complete guest NPC selector/gameplay lifecycle with all 12 previews, gameplay, and clean exit; location marker proof `artifacts/v82-open016-current-fontcell-location-marker-proof` passes and shows the crisp Dreamland USA-map marker. `RECOMPONE_UNTHROTTLED` was not run or enabled, all proof runs were one active game process at a time, `V8_2_LOOSE/v8_latest.log` was blanked for fresh triage, and `PS1 game/Vigilante82PC.exe` remains absent. This does not close OPEN-016 until explicit user review.
  - 2026-08-17 failed user review additions: the selector still labels the fantasy location `California` instead of `Super Dreamland 64`; its Dreamland preview is a failed line-art reinterpretation rather than a faithful upscale of the N64 image; the hard rectangular preview edges do not match the other map icons; and the overlay must never cover any part of the USA map, including at its upper-right corner. The loading image has the same failed art treatment, while its tire tread and `Press X to start...` composition do not match the native presentation used by other maps. Gameplay still renders water tiles through land, omits large terrain-texture regions, and ends terrain/texture layers at conspicuous hard-clipped boundaries; the post-match view preserves abrupt terrain/backdrop boundaries. These four user screenshots supersede prior automated selector/loading acceptance claims. All previously reported in-match issues remain open, including the unresolved character-8 vertical sky/backdrop edge. The work is explicitly resumed under the existing unfinished goal; runs remain limited to one game process at a time, `RECOMPONE_UNTHROTTLED` is forbidden, and the final step is `Pause or Block the goal for review`.
  - 2026-08-18 vehicle-glass attempt reopened by user review: vehicle windows use a pane-sized projected `OpaqueVehicleGlass` material instead of the prior semitransparent glass path, with no-projection HUD/icon glass left on the old path and oversized projected false positives rejected. Deployed `V8_2_LOOSE/Vigilante82PC.exe` SHA-256 `46544CA040B1954C4CCEE67C5B7F64AC550E2BE46E6D18BE26F9F2D5671368B3`; focused proof `artifacts/v82-open016-pane-sized-opaque-glass-focus` and full weapons proof `artifacts/v82-open016-pane-sized-opaque-glass-full-weapons` passed mechanically and looked acceptable in selected gameplay frames, but user review says glass is still broken: it reflects from some angles and then drops out from others. Current hypothesis is a shader/material rendering issue rather than model geometry alone, with the selector/vehicle-preview glass also needing high-resolution visual proof before any closure. Treat the glass item as OPEN, not fixed.
  - 2026-08-18 terrain-band no-2x control: `artifacts/v82-open016-no2x-terrain-band-control` passed a 900-frame Super Dreamland 64 run with high-resolution textures disabled, weapons 1-3 armed/fired, and zero collision-stream rejections. Sequential review of frames 2860/3060/3300/3600 still showed the hard stepped brown terrain/backdrop bands and dark/warm distance split, so the remaining terrain-band issue is not caused by the replacement/upscale texture pack.
  - 2026-08-18 terrain/backdrop release cleanup: follow-up geometry traces at frames 2858/2860 showed the flagged backdrop packets were degenerate below-frame panorama strips, not a safe visible-coverage fix. The earlier backdrop-guard relaxation and route-fog candidates are rejected for public-release staging because they reintroduced sky slabs, HUD/caption contamination, or worse horizon lighting. The current `46544CA040B1954C4CCEE67C5B7F64AC550E2BE46E6D18BE26F9F2D5671368B3` glass build remains the staged WIP baseline; residual Dreamland terrain/distance bands are documented limitations, not closed acceptance.
  - 2026-08-18 wrap-up revert: rejected the far-route-fog handoff candidate `B41E06252E20A4E2EE17FA9693AD153B14B238674B84F86670F06811FF7696A9` after focused proof frames 2860/3060 blackened the bus body/materials. Restored and redeployed the proven pane-sized opaque-glass baseline `46544CA040B1954C4CCEE67C5B7F64AC550E2BE46E6D18BE26F9F2D5671368B3`; Super Dreamland 64 remains WIP for public release with terrain/distance bands documented instead of chased further.
  - Review rule: proof must cover consecutive transition and gameplay frames rather than isolated chosen screenshots, exercise multiple vehicles and all weapon/effect classes, and separately demonstrate the native UI texture and replacement-resolution paths. Further defects found in later user testing remain in scope.
  - Known limitation: V8 dynamic actor-model swaps remain pinned static because the V8 and V8:2 model-list layouts are incompatible. Mushroom projectile detachment uses a fresh native object rather than unlinking the legacy actor model, preventing model-list corruption while retaining its gameplay callback path.
  - Remaining work: turn the Dreamland-specific conversion into a documented reusable N64 arena pipeline with format detection, dependency closure, geometry/material/render-mode translation, collision and AI/navigation data, routes, animated surfaces, shell metadata, loading art, validation, and actionable rejection of unsupported source features.
  - Pass condition: supported N64 arenas can be imported reproducibly without arena-specific runtime patches, survive selector/gameplay/teardown flows, and pass source-versus-port visual, geometry, collision, and packet/material traces. Each imported arena remains open until separately signed off.

#### V82-OPEN-017 - UI option to disable transformation powerups

  - Current groundwork: the host cheat menu exposes `DRIVE_ONLY`, and deterministic automation can suppress transformation pickups through a test-only environment setting.
  - Remaining work: add a normal in-game option, persist it in user settings, apply it through the authoritative gameplay path, and keep all unrelated powerups and match rules unchanged.
  - Pass condition: the option is discoverable and correctly reported in the game UI; disabling it prevents transformation pickup/activation in every mode and for human/AI vehicles, survives restart, and re-enabling it restores native behavior.

#### V82-OPEN-018 - Port the original Vigilante 8 UI

  - Remaining work: bring the original V8 front-end and in-game presentation into the port, including menus, selectors, loading presentation, HUD, pause/objective surfaces, results, fonts, icons, and transitions, while retaining a clean route back to native V8:2 presentation.
  - Pass condition: the V8 UI is selectable through the normal game interface, covers every reachable gameplay/menu state without mixed-theme leakage or broken layout, supports imported and V8:2 content, persists its selection, and passes complete 4:3/widescreen visual sweeps.

#### V82-OPEN-021 - Interstate '76 five-car pack with light AO bake

  - Requested roster: Groove = `I76_picard_piranha`; Taurus = `NP_ransom_marshal`; Jade = `NP_dover_rampage`; Skeeter = `I76_skeeters_van`; Antonio = `NP_leopard_xlc`.
  - Current groundwork: I-76 extraction/conversion, texture, atlas, and AO experiments exist for portions of this roster, including the Picard Piranha and Ransom Marshal.
  - Remaining work: finish all five conversions with a restrained ambient-occlusion bake, correct materials/glass, wheel anchors, collision bounds, weapons/hardpoints, stats, selector assets, names/identity, audio mapping where available, and player/NPC integration.
  - Pass condition: all five cars pass repeated selector preview, gameplay, transformation/powerup, damage, AI/NPC, teardown, texture/material, and visual checks; the AO adds readable contact shading without muddying source colors or baking directional lighting.

#### V82-OPEN-022 - Star Wars: Demolition landspeeder and arena mod

  - Requested content: one playable landspeeder and one Star Wars: Demolition level.
  - Remaining work: extract and document the source assets, convert the vehicle and arena through reusable mod pipelines, translate materials/effects/collision/navigation, add shell/loading/selector presentation, define handling/weapons and identity, and integrate player plus NPC use.
  - Pass condition: the landspeeder and selected arena are installable as a self-contained mod, selectable through normal UI, playable through complete match and teardown flows, visually faithful to the source, collision/AI stable, and isolated from stock content when the mod is disabled.

### Closed items

#### Guest NPC integration

- [x] **V82-OPEN-020 - Imported vehicles are selectable as NPCs**
  - User acceptance: signed off 2026-08-15.
  - Fix: all four native enemy rows extend through the common registered guest roster while retaining isolated retail proxy bytes for sequel-only tables. The gameplay handoff replaces selected proxies with custom identities before COMMON object construction, preserving native stats, weapons, AI, scoring, and teardown behavior.
  - Verification: `artifacts/v82-open020-npc/acceptance-proxy-isolated/acceptance.json` passes all 12 guest traversals and captures, one type-74 Molo participant and guest AI object from the native `x1` row, gameplay entry, and clean exit. The standalone collision probe preserves an unrelated stock participant, the full selector regression passes, and the Hoover Dam regression completes 360 gameplay frames.
  - Visual proofs: `artifacts/v82-open020-npc/visual-proofs/open020_npc_roster_proof.png`, `open020_selector_cleanup_before_after.png`, `open020_proxy_isolation_before_after.png`, and `open020_npc_gameplay_handoff_proof.png`.

#### Crash and flow

- [x] **V82-OPEN-001 - Arcade defeat/quit crash (critical regression)**
  - Reproduction: lose an Arcade match, then choose to quit the match.
  - Observed: the game crashes.
  - Pass condition: the defeat-to-quit path returns through the native front-end flow without a crash, hang, stale match state, or corrupted UI.
  - Root cause and fix: the guest runtime vehicle correctly retained identity through destruction, but `func_80036C2C` had also assigned the custom callback to its native source object without registering that source. The global event-4 quit sweep therefore reached a valid custom callback with no registry entry. Identity is now registered on both callback-owning objects at assignment time and retired only at their actual allocator lifetime boundary.
  - Verification status: fresh muted loose-files runs exercise the authentic `func_80039DCC -> func_80038C40 -> func_80038870` lethal path for both guest and stock vehicles, wait through the native 301-tick result lock, press the documented X-to-quit action, reload `SHELL_SHELL`, and accept Up/Down input on the returned main menu. The exact staged executable completes with no crash, hang, stale match state, or corrupted return UI.
  - 2026-08-14 rebaseline: PASS on the current staged hash in all 13 stock/imported defeat-result-return cases.

- [x] **V82-OPEN-002 - 16:9 gameplay to 4:3 menu return exposes HOM side bands**
  - Reproduction: leave 16:9 gameplay and return to any authored 4:3 front-end menu; the Quest defeat/X-to-return path is one deterministic reproduction.
  - Observed: the newly exposed left/right host regions retain stale imagery as the presentation contracts from 16:9 to 4:3.
  - Pass condition: every pixel in both newly exposed side bands is explicitly black from the first 4:3 return frame through the settled menu. The authored 4:3 menu remains centered and unstretched, with no stale framebuffer/VRAM content or transient HOM.
  - Candidate fix: `SHELL_SHELL` load now retires only the host-owned expanded 16:9 render target and invalidates the last wide output texture. The host backbuffer and complete output-panel region are painted black before the centered 4:3 image is submitted. Native PS1 VRAM is deliberately preserved; clearing it was rejected as an incorrect layer boundary that could erase valid shell assets.
  - 2026-08-14 rebaseline: PASS. The exact current build produced 41 synchronized host-composition samples; every returned-menu side-band pixel was black, the 4:3 menu remained visible and centered, shell reload completed, and menu music recovered.

#### Guest vehicle integration

- [x] **V82-OPEN-003 - V8 character portraits are broken (current-build regression)**
  - Observed: imported character portrait art is absent/corrupt; only the lower name remains on the left panel.
  - Pass condition: every imported V8 character uses the V8:2 selector lifecycle and displays the complete approved left-side banner without seams, corruption, overlap, or persistence into enemy selection.
  - Evidence: `codex-clipboard-5124ca47-f170-4c82-930a-81e7fa9c3d4c.png`.
  - Regression found 2026-07-30: selecting Sid Burn by wrapping left crashed in `func_8002CB74` after the full-size raw portrait was copied through SHELL's fixed VLC scratch address and overwrote the live object-bank table.
  - Fix: all 12 complete original V8 banners are uploaded as exact RGB555 pixels into the V8:2 selector's native VRAM target; the imported pose layer is skipped because the completed banner already contains it, and context zero is excluded so enemy selection remains native.
  - Verification status: a fresh full-roster loose-files sweep shows all 12 complete banners without seams, corruption, overlap, or clipping. A separate fresh acceptance run shows the native Select Enemies screen with no imported portrait persistence.
  - 2026-08-14 rebaseline: PASS after two complete imported-roster traversals and the native enemy-selector handoff.

- [x] **V82-OPEN-004 - V8 vehicle window/material artifacts**
  - Suspected area: guest-vehicle texture transparency/material interpretation.
  - Observed: window polygons render as holes in both selector previews and gameplay.
  - Pass condition: every imported V8 vehicle retains its original window geometry/materials and correct transparency without making opaque bodywork transparent.
  - Evidence: Palamino selector preview in `codex-clipboard-5124ca47-f170-4c82-930a-81e7fa9c3d4c.png`.
  - Candidate fix: converted glass now uses V8:2's retail `flags=0x10, environment=0x7FFE/0x8080` material; reflection overlays use the separate retail `flags=0, environment=0x3FFF/0x8080` family.
  - Reopened 2026-08-01: the current staged build still shows substantial window artifacts on original V8 vehicles in the native character-select menus, especially after scrolling through multiple vehicles. The earlier isolated package tests and limited selector capture did not prove repeated native menu lifecycle behavior.
  - Revised pass condition: repeatedly cycle the complete imported roster through the unchanged V8:2 selector path for multiple full loops and inspect every vehicle from changing preview angles. Glass must remain correctly textured, ordered, and translucent after VRAM churn and object teardown/recreation; no one-pass or isolated-model proof is sufficient.
  - 2026-08-14 rebaseline: PASS. The strengthened current-hash selector gate constructed every imported preview at least twice (34 constructions total), produced a clean 12-vehicle contact sheet after churn, passed the 200,000-triangle material audit, entered the native enemy selector, and exited cleanly. All 12 imported vehicles also completed separate 720-frame Enhanced gameplay cases.

- [x] **V82-OPEN-005 - V8 character voiceovers are missing**
  - Observed: imported characters do not play their original V8 selection/gameplay voice assets.
  - Pass condition: each imported character is mapped into the existing V8:2 voice-event system using the corresponding original V8 voice assets, with no replacement of shared V8:2 voices.
  - Fix and evidence: all 12 imported selection mappings and all 24 victory/defeat banks use the native V8:2 event/XA path. The decoded result payloads are byte-exact to the original V8 sources across 13,502,400 bytes.
  - 2026-08-14 rebaseline: PASS. All 12 imported defeat cases opened the expected `V8VOICE/D00` through `D11` path, completed the full voice dwell, returned to shell, and exited cleanly.

#### Rendering and graphics settings

- [x] **V82-OPEN-006 - Severe terrain jagged spasms/flicker remain**
  - Observed: terrain geometry exhibits frequent jagged, spasmodic movement/flicker during gameplay.
  - Pass condition: moving-camera smoke tests on every map show stable terrain edges and surfaces, with no turn-dependent polygon popping, projected-vertex spasms, or ordering-table corruption.
  - Root cause and fix: the speculative traversal-polygon refit starved the retail terrain walker on roughly 6% of frames. It was removed; bounded row-span padding preserves widescreen coverage without starving terrain submission, and terrain uses authoritative primitive ordering-table depth.
  - 2026-08-14 rebaseline: PASS. All 18 stock maps and all 12 imported-vehicle/map cases completed 720 Enhanced frames with no renderer failure; maximum visible-world fallback was 0.192702%.

- [x] **V82-OPEN-007 - Enhanced settings UI does not match active rendering**
  - Observed: settings report the Enhanced preset while gameplay visibly uses PS1-style settings.
  - Pass condition: the displayed preset and every individual graphics toggle reflect the actual renderer state; Enhanced applies and persists its intended non-PS1 settings after startup, menu transitions, and match loads.
  - Fix: V8:2 identity is configured before preset reconciliation; named presets are authoritative; the active HLE renderer, native-resolution mode, internal scale, VRAM texture, render targets, and shader scale update together without restart. The original port's accelerated/software dither gates remain authoritative, and a V8:2-only enhanced-gameplay presentation cleanup removes the residual ordered pattern without altering menus, fidelity mode, or the accepted original V8 renderer.
  - Reopened 2026-08-01: the current staged build still visibly exhibits the same PS1-style rendering defects while the UI and configuration probe report Enhanced 3x. The previous proof established selected values and two limited scenes, but did not establish that every active primitive, fallback, menu-to-match transition, and map renderer actually consumes those values.
  - Revised pass condition: trace each Enhanced option from the saved setting through runtime application to every world/model/UI draw path, including affine/projective fallback, native-resolution framebuffer copies, overlay reloads, and menu-to-match transitions. Prove sustained motion on every map; configuration logs or a clean isolated angle do not close this item.
  - 2026-08-14 rebaseline: PASS. The staged configuration is Enhanced, HLE, 4x internal scale, high-resolution textures, perspective-correct texture/color interpolation, enhanced depth/fog/shadows/particles, FXAA plus 2x MSAA, and widescreen. The exact staged executable passed 30/30 sustained 720-frame renderer cases across all maps and imported vehicles.

- [x] **V82-OPEN-008 - Textures are too low-resolution**
  - Observed: visible low-resolution texture presentation across the game, especially loading screens.
  - Pass condition: upscale source textures without stretching or changing authored layout; world/model textures remain at or below 512x512 and menu/loading UI may use up to 1024x1024, per the established limits.
  - Evidence: `codex-clipboard-f6a192f7-fc42-46c0-8bdc-0bb619c1befa.png`.
  - Fix: the deployed pack validates 4,227 DDS files and 33,529 mapped regions, including all 18 terrain atlases, 2,289 vehicle textures, 30 route textures, 251 runtime UI textures, and the accepted GAME.FNT replacement. The loading-card builder reconstructs each arena's native art from exactly twenty MDEC-to-VRAM strips, preserves the accepted Route 66 asset, and supplies arena-keyed 4x overlays for all 18 maps. Sixteen cards retain the native 320x96 contract; Secret Base and Canyonlands retain their authored 320x112 cinematic contract and aspect ratio.
  - 2026-08-14 acceptance: PASS. The exact staged executable loaded and selected the correct card on all 18 hidden 1920x1080 runs. Programmatic live-to-source correlation was 0.997747-0.999877 with maximum mean RGB error 1.466821; every capture was unique. The full contact sheet at `artifacts/v82-open008-loading-acceptance/loading_cards_1080p_contact_sheet.png` passed visual review for title preservation, layout, frame bleed, prompts, and menu/loading text.

#### UI and HUD

- [x] **V82-OPEN-009 - Defeat/game-over text spacing is corrupt**
  - User acceptance: signed off 2026-08-04.
  - Observed: "PLAYER 1 destroyed!" has a large mid-word gap (`PL    AYER`).
  - Pass condition: all defeat/game-over strings use correct glyph advance, kerning, and centering at every supported resolution and aspect ratio.
  - Evidence: `codex-clipboard-5cf6fd17-27ca-4fb0-95f0-8d48f18e7105.png`.

- [x] **V82-OPEN-010 - Pause and objective menus are not centered**
  - Observed: pause/objective overlays and text are offset; objective text is split or displaced inside the panel.
  - Pass condition: the complete authored overlay, heading, body text, and control prompts are centered as one 4:3 UI composition without stretching.
  - Evidence: `codex-clipboard-3dfa69ad-7a6a-48e7-a480-58b5ad1e20f8.png`.
  - 2026-08-14 rebaseline: PASS. Fresh 1280x720 pause and objective captures both passed composition analysis with no tall neutral-gray corruption; visual proofs are under `artifacts/v82-rebaseline-20260814/ui/proofs`.

#### Audio

- [x] **V82-OPEN-013 - Menu music does not play**
  - Observed: menus are silent while in-match CD music does play.
  - Pass condition: the correct converted OGG tracks start, loop, transition, and resume through the existing menu audio state machine.
  - Root cause and fix: the loose manifest now preserves track 2's index-0 LBA at 147300 and shell reload uses the native CDDA recovery state rather than a wrapper-owned player.
  - 2026-08-14 rebaseline: PASS. Track 2 started after the widescreen-to-menu return and after every one of the 13 stock/imported defeat-result-return cases.

### Required verification

- Exact-current renderer matrix: PASS 30/30 at 720 frames per case, covering all 18 stock maps and all 12 imported vehicles.
- Exact-current transition matrix: PASS 13/13, covering stock plus every imported defeat/result/voice/shell-return flow.
- Exact-current selector gate: PASS after two full roster traversals, material audit, enemy-selector handoff, and clean exit.
- Exact-current widescreen return gate: PASS across 41 composed menu samples with black side bands.
- Exact-current pause/objective composition gate: PASS 2/2.
- Texture-pack structural validator: PASS with 4,227 DDS files and 33,529 mappings.
- Exact-current loading-card matrix: PASS 18/18 at 1920x1080 with exact arena selection, unique captures, and live-to-source pixel correlation.
- Keep audio-muted visual automation separate from audible OPEN-012 acceptance. Do not close OPEN-012 without measured and human-audible balance evidence.

## Work queue

### Additional V8: 2nd Offense roadmap

Numbered V8:2 work is tracked in the issue register above. These are broader
or unnumbered follow-ups that remain outside that register.

- [ ] Expand the initial Star Wars: Demolition vehicle-and-arena deliverable
      into a curated bonus-content pack through the reusable guest-vehicle and
      arena systems.
- [ ] Internalize more wrapper functionality into the game UI so the external
      header/menu bar can be removed.
- [x] Ensure menu music starts reliably on boot and resumes after gameplay,
      results, movies, quests, and returns to the main menu. The native CDDA
      path now has an idle-shell recovery guard and passed the 13-case
      stock/imported-vehicle transition matrix.
- [ ] Add fully customizable controls for keyboard and controllers, together
      with user-friendly Modern, Classic, and Southpaw presets, integrated
      into the native V8:2 Options/Controls shell at stock visual and
      interaction parity. The rejected wrapper-font prototype does not count.
- [x] Change Square driving behavior so it brakes the vehicle to a stop and,
      when held while already stopped, engages reverse. Modern routes physical
      Square to the existing native Down command used by stick-down.

### User-found defect remediation

- [ ] Revisit Enhanced-renderer close-range scenery clipping in V8: 2nd
      Offense. Building and other static-object polygons can disappear or
      corrupt as the camera approaches them. The 2026-08-11 selective NCLIP/
      packet-span rescue experiment failed visual verification and is not an
      accepted fix; resume from a deterministic same-camera reproduction and
      retain the known-good loose-files build until a replacement passes fresh
      gameplay proof.
- [ ] Fix locked-arena standalone loose-file launch for Sand Factory and Secret
      Base. The 2026-07-28 visual-smoke pass still exits before gameplay with
      `unmapped call: 0x00000000` from `FUN_80110c7c` during the SHELL-to-level
      transition while eight selectable arenas pass.
- [x] Recheck V8: 2nd Offense movie/FMV playback on the fresh deployed sequel
      build. Screenshot evidence from 2026-07-27 originally showed the movie
      window mostly black with a gray vertical partial frame/block; the fresh
      build now decodes full 320x240 MDEC frames and captured valid ACTLOGO
      presentation frames at 60, 120, 180, and 260. Broader all-FMV visual
      audit remains part of the sequel UI/presentation polish goal.
- [x] Recover the Aircraft Graveyard crash, translate the omitted native
      callback, and pass a sustained arena regression under movement and
      weapon load.
- [x] Verify reverse from original pad input through vehicle motion and make
      the retail Down=brake/reverse and Square=handbrake mapping explicit in
      the host input UI.
- [x] Apply the enhanced-mode dithering default to both accelerated and
      software PS1 raster paths, retaining a live fidelity toggle.
- [x] Reproduce and correct the Canyonlands `FUN_80024998` crash at
      `0x380C110F`. Register capture proved the diagonal-neighbor lookup
      returned null at original return address `0x80024C70`, unlike the four
      axial cases which check null; the compatibility boundary now represents
      that exact missing neighbor with a read-only empty pool cell. Ten fresh
      Canyonlands matches passed through 70,200 aggregate gameplay ticks.
- [x] Classify the Ski Resort post-defeat presentation stall as a
      result-screen automation false positive. With result/title transitions
      excluded from the gameplay framebuffer gate, two fresh matches reached
      3,960 and 6,480 ticks and returned cleanly to the title screen.
- [x] Diagnose Casino City's fragmented emulated-heap exhaustion: allocation
      failure is recoverable pressure, not a fatal condition; a subsequent
      Canyonlands run logged it and continued normally through 9,900 ticks and
      result/title teardown.
- [x] Isolate the intermittent cross-arena Object_PreTick stall. A Ghost Town
      live stack exposed main callback `0x800378D0` event 5 dispatching through
      `0x80037ADC`, clearing the child cursor, and returning continue; the exact
      cleared-child state now aborts the outer animation walk. Casino City,
      Hoover Dam, and Ghost Town subsequently completed long result/title runs.
- [x] Regenerate and regression-test the newly recovered main-executable
      entrypoint `0x8003DFFC`, reached from Hoover Dam gameplay at tick 720.
- [x] Fix Hoover Dam callback `0x80100A18` clearing its animation cursor on
      event 5 while returning success; three independent post-fix matches
      reached 7,380, 5,940, and 4,500 ticks with clean result/title teardown.

### Multi-map soak

- [x] Recover and document the retail arena-selection order and stable menu
      synchronization points.
- [x] Create parameterized/stage-aware fixtures for all arenas.
- [x] Implement a bounded soak runner with timeout, responsiveness, stage,
      framebuffer-change, audio-init, and fatal-marker checks.
- [x] Run an initial all-map reachability matrix.
- [x] Run extended multi-map soak coverage and triage/fix failures.

### Split-screen

- [x] Make Player 2 gamepad defaults usable without manual remapping while
      preserving explicit user bindings.
- [x] Create separate Cooperative and Versus gameplay fixtures.
- [x] Validate simultaneous Player 1/Player 2 movement and weapon inputs.
- [x] Validate split-screen rendering, match progression, teardown, and clean
      relaunch independently from single-player soak results.

### Other autonomous coverage

- [x] Replace the CUE-backed override layer with a standalone loose filesystem
      that boots, streams STR/XA, plays loose music, and reaches gameplay with
      no BIN/CUE files available.
- [x] Exercise exact-path content mutation, size changes, missing-file errors,
      ambiguous basename rejection, and cooked/raw stream reads in an isolated
      standalone tree suitable for future mods.
- [x] Add non-invasive live-controller diagnostics and validate SDL/XInput
      discovery against the attached XInput-compatible device.
- [x] Exercise hot-plug state handling and rumble calls where automation permits.
- [x] Test the victory/result path without directly writing gameplay state.
- [x] Extend gameplay coverage into Quest where deterministic automation permits.
- [x] Audit and either fix or explicitly disable the unsupported single-file
      C# mod/runtime-detour path.
- [x] Refresh stale reference documentation and deployed-path instructions.
- [x] Audit available state-trace/comparison scaffolding and advance the
      PS1-versus-reference fidelity work as far as local inputs permit.

### Future QOL and presentation

- [ ] Trace and upscale Route 66's road/route overlay separately from the 4x
      terrain atlas. Fresh same-frame A/B proof shows a strong 4x improvement
      in the surrounding ground while the road changes much less, consistent
      with an XRTP/route material layer being drawn above the terrain cells.
      Initial support now decodes XRTP TIM payloads into named 4x route DDS
      files and quadrant metadata entries in the live texture pack; keep this
      open until a longer or better-positioned runtime trace proves an actual
      XRTP route key hit in gameplay.
- [ ] Add widescreen gameplay support by expanding the horizontal 3D view;
      preserve the original aspect ratio and placement of HUD/UI elements, and
      pillarbox intros and menus rather than stretching them.
- [x] Add a PS1 color-dithering toggle and default the enhanced presentation
      mode to undithered output so flat textures and gradients remain clean;
      retain authentic ordered dithering in the fidelity/reference mode.
- [x] Add optional perspective-correct texture mapping to eliminate the
      PlayStation's affine texture warping/projection distortion, while
      retaining a fidelity mode for exact reference comparisons. V8:2 enhanced
      mode now uses recovered recent GTE screen depth for textured 3D triangles
      whose vertices can be matched safely.
- [x] Add enhanced PS1 texture smoothing to reduce chunky texture pixelation
      while preserving nearest-sampled transparency/STP decisions for cutouts
      and semi-transparent primitives.
- [ ] Add a geometry-edge anti-aliasing option (prefer MSAA or an equivalent
      edge-only pass) that preserves crisp source textures and PS1 dithering;
      retain full-frame FXAA and Off as separately selectable modes.

## Deliberate boundaries

- The reference executable remains Windows x64; the shipping Xbox game is a
  separate 32-bit x86 implementation.
- Standalone loose mode must never discover, open, or fall back to BIN/CUE data.
  Its path-stable asset tree is also the supported foundation for future mods.
- Renderer, raw controls, and host audio output are rewrite seams; gameplay
  state and timing remain the fidelity target.

## User sign-off queue

The retained screenshots and controller checklist are collected in
`reference/SIGNOFF.md`.

- [ ] Physical-controller feel: buttons, sticks, triggers, dead zones, hot-plug,
      and rumble on the user's actual controller.
- [ ] Visual acceptance of every arena and both split-screen layouts after the
      automated stability gates pass.

## Evidence log

- 2026-07-22: Goal created from deployed commit `9da3806`; both current
  `Vigilante8PC.exe` copies are byte-identical and the Oil Fields one-player
  vertical slice is the known-good baseline.
- 2026-07-22: `tools/recompone-v8/run_reference_soak.py` now generates
  deterministic stage-aware fixtures for all ten retail arenas and independent
  Arcade, Cooperative, and Versus paths. The current memory-card state exposes
  eight arenas; Sand Factory and Secret Base remain locked bonus selections.
- 2026-07-22: Initial eight-arena reachability exposed four omitted indirect
  callbacks plus a relocated jump-table defect. After source-backed repairs,
  `reference/generated/soak/all-arenas-reachability-fixed/summary.json` records
  8/8 passes with correct overlays, SDL audio, gameplay heartbeats, and changing
  HLE framebuffer hashes.
- 2026-07-22: Separate clean processes proved Cooperative as internal match
  mode 4 and Versus as mode 3. Each run rendered two distinct live viewports,
  changed both player positions under simultaneous pad scripts, initialized
  audio, remained responsive, and quit through the original pause confirmation.
  The host also discovered the attached `Xbox One Controller` through SDL.
- 2026-07-22: Extended coverage now spans all eight selectable arenas. Ski
  Resort, Valley Farms, Hoover Dam, Ghost Town, and Aircraft Graveyard ran the
  full 120-second gameplay window; Canyonlands, Casino City, and Oil Fields
  completed matches, dismissed results normally, reloaded their arenas, and
  resumed physics. Casino City's reproducible scatter-animation spin was fixed
  at its arena-specific callback boundary and passed the corrected 120-second
  gate. Liveness now uses continuous framebuffer probes while wheel-body ticks
  remain diagnostic telemetry.
- 2026-07-22: The first isolated loose-file suite proved override mechanics,
  but its deletion/raw-sector cases depended on disc fallback. The clarified
  standalone/modding requirement invalidated that acceptance result; the item
  is reopened until the same executable passes with `BINCUE` wholly absent.
- 2026-07-22: The attached Xbox controller survived an automated SDL
  close/reopen rescan with the same Player 1 assignment. Suppressed motor tests
  routed independent start/stop requests to both controller slots; physical
  vibration strength and real unplug/replug feel remain user sign-off items.
- 2026-07-22: Standalone loose mode now resolves the retail ISO layout from an
  embedded metadata-only manifest, requires all 69 content assets at exact
  paths, streams 23 full-sector STR/XA files, and maps twelve named OGG tracks
  to their original CD positions. With the entire `BINCUE` directory renamed
  away, the published single-file host streamed ACTLOGO, loaded SHELL/LOAD and
  OILFIELD, initialized SDL audio, and sustained active gameplay.
- 2026-07-22: The replacement loose regression suite passed live content and
  length changes, exact-path precedence over decoys, explicit missing data and
  music errors with no fallback, and cooked/full-sector stream hashes. A long
  result run decoded `SOUNDS/DEFEAT.XA` from loose sectors; incremental OGG
  sector decoding then passed the normal 15-second liveness gate for 90 seconds.
- 2026-07-22: Final direct-disc and standalone deployments are byte-identical
  at SHA256 `C8B218A06905FB672443128B827A148495958B3F6899B56E98D30CA90647BFE9`.
  Default probes selected CUE mode in `BINCUE` and standalone mode in `PS1 game`;
  both cards and settings remained byte-identical through media conversion and
  executable deployment.
- 2026-07-22: The loose exact-path tree is confirmed as the deployed mod
  foundation, with no disc fallback. The C# loader now compiles source from
  loaded assembly metadata and a framework-dependent build verified a real
  `main/start` pre-hook. MonoMod declares single-file applications unsupported,
  so the deployed layout now rejects C# source mods with an explicit diagnostic
  instead of reporting inactive hooks as loaded; loose asset mods remain active.
- 2026-07-22: The input-only victory fixture entered `I_WILL_NOT_DIE` and
  `DEADLY_MISSILE` through the original passcode editor, launched a one-enemy
  Oil Fields match, and supplied only ordinary controller input afterward. The
  enemy reached real soft/full destruction at gameplay ticks 11,710/12,010;
  the retail result logic reported the player alive at tick 12,521, rendered
  `YOU WIN!` with `PLAYER 1 vs ENEMIES 0`, and accepted its Circle dismissal.
- 2026-07-22: Quest automation selected its vehicle and route, confirmed the
  original post-load continue wait, loaded Ski Resort, and sustained 900 active
  gameplay ticks with moving position/velocity, acceleration, weapon fire, and
  weapon cycling before quitting through the original pause confirmation.
- 2026-07-22: The reference now emits pointer-neutral per-physics-tick JSONL
  state. The comparator reproduced the 16,260-tick victory trace and 900-tick
  Quest trace exactly against themselves, then correctly found Arcade-versus-
  Quest divergence at tick 1. Native snapshot scaffolding now records the
  instruction-confirmed one-byte PRNG carry. Comparison against an externally
  instrumented PS1/emulator is explicitly outside this goal; the local
  trace/comparator audit is complete and supplies the later native-port oracle.
- 2026-07-22: Final x64 single-file publication produced byte-identical
  `Vigilante8PC.exe` deployments at SHA256
  `1DFD56EFA33760E6E18072DFFCDB22775AF2520D9215FF7976971E302DA1D9C7`.
  The `BINCUE` copy selected direct CUE mode and the `PS1 game` copy selected
  standalone loose mode with 69 exact-path assets. Each independently completed
  Oil Fields gameplay, two pause/resume cycles, confirmation cancel, and normal
  confirmed quit with zero fatal markers. Cards and settings retained their
  pre-test hashes; all 34 temporary deployment captures were removed.
- 2026-07-22: Windows crash evidence identified Aircraft Graveyard event 17
  dispatching omitted main-executable wrapper `0x80021830`. Raw retail MIPS
  established its exact body, regenerated output now registers it, and the
  corrected arena passed 3,600 gameplay ticks under movement and weapons.
- 2026-07-22: A sustained input trace confirmed Square/X is the retail
  handbrake, while Down (D-pad or left stick) is brake/reverse. Down changed
  the active-low pad word to `0xFFBF` and established strong backward vehicle
  motion; the host input labels now expose both functions without changing
  original gameplay controls.
- 2026-07-22: PS1 color dithering is now gated in both accelerated and
  software rasterizers. Enhanced mode reports dithering Off by default;
  enabling the Display setting restores the original ordered-dither path,
  and both configurations passed independent Oil Fields gameplay runs.
- 2026-07-22: The corrected x64 single-file build is deployed byte-identically
  to both layouts at SHA256
  `6DF36FCEA232032F0CEE6AE9AA22BBDE4637CD44844E1B3B6D03946C0292AEEB`.
  The direct copy selected its CUE; the standalone copy selected exactly 69
  loose assets and did not open disc media. Both reached Oil Fields, reported
  dithering Off, completed 360 gameplay ticks, and exited through the original
  pause confirmation. Cards and settings retained their prior hashes.
- 2026-07-22: The three-hour crash campaign found Hoover Dam direct target
  `0x8003DFFC` inside Ghidra's overlapping `0x8003DFD8` extent. Registering the
  independent entry exposed a second defect: overlay callback `0x80100A18`
  cleared animation cursor `+0x60` on event 5 while returning success, causing
  `Object_PreTick` to spin. The exact cleared-cursor boundary now returns the
  animation walker's abort result, and three fresh matches passed.
- 2026-07-22: Long-soak liveness now captures a live `dotnet-stack` report
  before terminating a stalled process. Natural result/title return is accepted
  without requiring an immediate same-arena relaunch, and heap-allocation
  pressure is recorded rather than treated as fatal by itself.
- 2026-07-22: The three-hour loose-file crash hunt ran from 16:12 through the
  19:12 cutoff and retained 64 bounded cases across nine campaigns, totaling
  376,380 gameplay ticks. The early matrix deliberately remains in the evidence
  set: its failures separate genuine unmapped-call, collision, and animation
  defects from corrected result-transition false positives and recoverable heap
  pressure.
- 2026-07-22: A live Ghost Town stack isolated the common nested-animation
  stall at callback `0x800378D0`; the source-backed cleared-child termination
  fix subsequently completed long Casino City, Hoover Dam, and Ghost Town
  matches with normal result/title teardown.
- 2026-07-22: Fault-time registers reduced Canyonlands address `0x380C110F` to
  the diagonal-only null return at `0x80024C70`. After the non-mutating empty-
  neighbor compatibility fix, ten consecutive Canyonlands matches passed for
  70,200 aggregate gameplay ticks. The closing post-cutoff bounded runs also
  passed Ghost Town at 6,120 ticks and Hoover Dam at 7,380 ticks.
- 2026-07-22: The final self-contained build is deployed byte-identically to
  `BINCUE/Vigilante8PC.exe` and `PS1 game/Vigilante8PC.exe` at SHA256
  `FCA1987F77D0C3151FAC54C6B8386F1D1F9A8EE7863DC0B7E327BC0C8604174A`.
  No stale handoff logs were present, and both memory cards plus all settings
  retained their pre-deployment hashes. Autonomous work is exhausted; only the
  two subjective checks in the user sign-off queue remain.

- 2026-07-27: V8:2 visual polish: Smooth PS1 textures and Fix PS1 texture
  projection are default-on in enhanced 3D. The projection fix uses recent GTE
  SX/SY/SZ correlation for textured 3D triangles only; sprites, menus, and
  2D rectangles retain ordinary screen-space interpolation.
