# Local multiplayer completion audit

Current staged build: `9A169BB8D506647B0B1C18A0B1D718F73DBFB98B850AC52077A3350CBC73811E`.
Evidence directories below are under `artifacts/multiplayer-menu-20260914`.
Implementation and automated validation complete. User review remains in TO-DO.

| Requirement | Current authoritative evidence | Status |
| --- | --- | --- |
| Main menu 1 PLAYER / MULTIPLAYER / OPTIONS, approved placement | V82LocalMenu.Label, native main-menu captures, user acceptance | Implemented and accepted |
| Native main-menu background for join; A joins; P1 Start/Enter confirms; minimum two | V82LocalMenu.Join; LocalJoinState.Update; current ControlsContract output covers first-join ownership, non-owner rejection, minimum count, Enter and disconnected devices; join-confirm-owner native run | Implemented and verified |
| Pre-main-menu PRESS START | InputPromptLabels.Format preserves exact controller text; native title flow captures | Implemented |
| Independent 2/3/4P character/color selection | V82LocalMenuSelect; accepted layout; dedicated two/three/four and imported selector runs; three-player selector-feedback additionally checks native sound dispatch | Implemented and verified |
| Six-combatant Versus roster | SplitMenuArm fills six slots; 3P/4P runtime V82SplitVerified; 2P complete-roster verifies six initially and six after native rematch | Verified |
| Native 2P Cooperative/Quest choices and setup | coop-ready mode6 two humans + three selected enemies; quest-ack mode7 two humans + authored enemy; native captures reviewed including briefing dismissal | Entry/gameplay verified; do not replace authored quest opponents with random bots |
| Physical device ownership | LocalInputSession.RoutePolledDevices, current ControlsContract, first-join pad3 native selector run, extra-player pause/reconnect tests | Implemented and verified |
| Pause, quit and teardown | dedicated P3/P4 disconnect/reconnect/resume; three-player quit-reentry-arena exits cleanly after two fresh matches | Verified |
| Results, scoring, rematch | three-final-modal-results two rounds; four-paint-modal-win and modal-draw; imports-isolated-imported-results-owned; 2P complete-roster two rounds with physical B/A and scores1/2 | Verified: Versus2/3/4P; cooperative-rounds retains enemies and team scores1/2 with physical B/A; quest-grace reaches native results after the authored grace period and physical A continues |
| Solo preservation after multiplayer | solo-reentry: native menu result1, only one human spawn, no split setup, full-screen native gameplay capture after 3P quit; clean6000-poll exit with unchanged user files | Verified; startup-only mode log assertion corrected and evidence checked post-run |
| Native presentation and active texture pack | Reviewed selector/gameplay/results captures; stdout TexturePack loading and active mod paths; shared native font and sound bank | Verified for captured paths |
| Stage and record handoff | executable staged individually in protected V8_2_LOOSE; matching published/staged hash; progress.log and TO-DO updated | Matching staged/published hash verified; tracker pending user review |

No implementation gate remains open in this audit. Historical failed fixtures in
local_multiplayer_menu.md are not acceptance evidence. No five-hour soak or FPS
benchmark was used. Current changes since the earlier3P/4P evidence are gated to
joined2P result context/footer or explicit test-only fixtures; expanded roster
restoration retains the existing3P/4P record/score paths. Final native Quest run
uses lethal damage and the original grace period without objective/result flags
being forced. It verifies incomplete-objective summary and continuation, not a
new end-to-end test of every existing quest campaign.


## Independent keyboard follow-up — 2026-09-14

The earlier four-gamepad tests did not establish mixed keyboard/gamepad support.
That gap is now corrected: four physical gamepad identities plus one keyboard
identity feed at most four joined player ports. First join owns P1; keyboard Z
joins independently, and Enter confirms. Solo/legacy keyboard bindings remain
combined only outside a joined session. Keyboard gameplay remaps remain active;
keyboard ownership never supplies a gamepad's buttons or analog axes.

Final staged build: `9A169BB8D506647B0B1C18A0B1D718F73DBFB98B850AC52077A3350CBC73811E`.

| Evidence directory | Verified behavior |
| --- | --- |
| select-split-two-surface-keyboard-final | Keyboard host plus first gamepad; native join/background, independent character/color selection, contextual Z/X and gamepad labels, six-combatant match; exit0 |
| select-split-three-final-pause-keyboard-final | Two gamepads plus keyboard P3; independent ready/unready, three human viewports, keyboard pause/resume, simulation continues, unused fourth viewport clears; exit0 |
| select-split-four-paint-pause-keyboard-final | Three gamepads plus keyboard P4; four character previews, mixed color-default X/A labels, four human viewports, keyboard pause/resume and continued simulation; exit0 |
| select-split-four-paint-four-gamepads | Four gamepads still join/select/start six-combatant gameplay; exit0. Input implementation matches final build; this run predates only keyboard-aware join Back/color-footer wording fixes |
| keyboard-contract.log | Mixed join orders including keyboard P1/P2/P3/P4, four-gamepad roster with keyboard input ignored, four-player cap, gamepad disconnection with keyboard intact, keyboard driving remap, Enter, selector focus, prompt ownership and solo fallback |

Actual native captures were inspected individually: join/background at220;
2P selection440/gameplay300; 3P selection625 and pause520/resume740;
4P mixed selection440/600 and pause520/resume740; four-gamepad join220,
selection600 and gameplay300. Expected artwork, vehicles, panel layout,
keyboard/gamepad labels, pause overlay and its removal were checked, not merely
frame production. Texture pack remains loaded from enhanced_textures_2x.
All runs preserve staged settings/interface/save files. No test process remains.

Limits: devices were simulated entirely inside each targeted process. No physical
multi-gamepad hardware test or audio listening test was performed (harness muted).
This follow-up did not repeat every Quest/results/rematch case from the prior
acceptance matrix. The initial parallel three-player keyboard run hit its180s
wall timeout after reaching gameplay; it is not counted as a pass. The final3P
run completed normally with a300s wall allowance and the same3600-poll budget.
