# Local multiplayer menu implementation

Goal active, 2026-09-14. Work is in the main checkout. No commit or push.

## Implemented so far

- Main menu retains native fonts/art and now reads 1 PLAYER, MULTIPLAYER,
  OPTIONS. At the user's request, text, cursor and backing rectangle are
  shifted 32 native pixels left (64 pixels in the 1280x720 capture).
- Controller title says PRESS START. Native proof:
  `artifacts/multiplayer-menu-20260914/join-second/recompone_present_v82_press_start_0024_1280x720_off.png`.
- Four input ports, separate pad profiles, stable physical port ownership
  across disconnects, process-local P3/P4 physical-button scripting.
- Join state consumes the entry press, rejects duplicate joins, assigns P1
  to the first joining device, requires two connected participants and only
  accepts Start from P1 (or keyboard Enter). Disconnected joined slots remain
  owned and block confirmation. Circle leaves; host Triangle cancels.
- 64 controls/export/join checks passed in
  `artifacts/multiplayer-menu-20260914/controls-four-port.log`.

## Native integration and remaining work

`V82LocalMenu.cs` uses the original shell main menu, fonts, backing rectangles,
and frame pump. Hooks are durable in `apply_native_options_patches.py`.
The main menu is `80103FA0`; its old 2 PLAYER branch is `801044A0`.
Native main-menu row drawing returns to `80104284`. Stage instrumentation
must distinguish that OPTIONS row from the actual Options page.

The first full-screen VRAM copy attempt exhausted available rectangle space
while loading the small font. Repeated whole-art decoding was also unsuitable
for the existing replacement-image path. The current join page reuses the
retail menu and footer backing rectangles. The rotating model changes the
draw environment; restore `8001AAFC` before each UI repaint.

The native four-join/cancel replay exits cleanly and logs four separate
devices. P2 Start does not confirm. Layout validation is still in progress.

Confirmation currently returns native mode 5 to the existing two-player shell.
**The full three/four-player menu-to-match path is not implemented yet.**
Required next: route joined devices to player ownership, independent character
and color selection, arena selection, six-combatant roster creation, actual
P3/P4 inputs, normal results/rematch/teardown. Preserve solo and existing
development harness coverage. Do not mark the goal or TO-DO complete.

Existing split support is in `V82SplitScreenHarness.cs`; it currently uses an
environment-selected player count, development-only direct launch, synthetic
extra inputs and a throw on shell return. Separate normal session state from
that development entry before making multiplayer menu launch available.

Existing registry already supports four player selection slots. Retail
character selectors are `8010669C` (single) and `80107AD4` (two); location
selector is `8010458C`. Arena and imported vehicle registries must remain in
the normal flow rather than duplicating their content lists.

## Earlier join-only increment (superseded below)

The seventh build fixes the draw environment before text, reuses the original
backing rectangles, and omits the unused rotating cursor wheel. Empty/full
join captures now look correct, including the exact requested message
REQUIRES AT LEAST TWO PLAYERS. User accepted the main-menu placement.
Current staged SHA256:
`ADC7C9F76131F0643B340AE542503DDD71577B75726685D821C6B525757ECC6F`.
Proofs: `artifacts/multiplayer-menu-20260914/join-seventh/`.

`LocalInputSession` now routes completed physical pad images to join-order
players after polling, neutralizing disconnected/unjoined slots without
renumbering. Rumble and prompt binding lookup use its device mapping. Reset
occurs at main-menu entry. The expanded controls contract has 69 passing
assertions in `controls-ownership.log` under the same artifact root.

Additional `join-confirm-owner` native run: pad 3 joined first, then pad 1;
pad 1 Start was ignored, pad 3 Start confirmed, and the native shell entered
`choose_location_1`. Exit 0. The intended location image did not fire because
the fixture omitted the generation suffix; fix its stage on the next run.
Four-device join/cancel replay also exited 0. No match was launched.

Remaining work above still applies. Confirmation temporarily selects mode 5
(Versus). Preserve native 2P mode choices during final integration: SHELL.DLL
has zero file/address bias relative to 0x80100000; table 0x1278 dispatches
mode 5 to F524, mode 6 Cooperative to F61C, mode 7 Quest to F8B4.
The native single selector's A1=0 skips enemy setup and writes selected type
to 0x8006B8F4 and -1 to the second slot. A sequential extension must preserve
each selection, input focus and registry context; A1 is not a player index.
InputManager.PromptUsesGamepad still needs its source lookup mapped to the
joined physical device, as prompt binding lookup already is.
No processes or builds remain running after these tests.

Tests must remain hidden, process-local, use native captures and the active
texture pack. No desktop input, no performance benchmark, no five-hour soak.


## Simultaneous character selector (current implementation)

The user rejected sequential character pages and requested a simplified split
selector. `V82LocalMenuSelect.cs` replaces the sequential implementation with
simultaneous panels: two horizontal rows for 2P, four quadrants for 3P/4P.
Each owned device changes its vehicle, opens native body-color editing, and
readies independently. The panels show player number, vehicle name/preview,
color or ready status. The original roster restrictions and unique selections
are preserved. Imported vehicles use their owned runtime banks and paint masks.

A first visual run exposed black reflective body surfaces. The native 2P
selector initializes environment-material sprites at 8006B638/8006B648 and
clears the light matrix before creating previews. Restoring that initialization
fixes the bodies; both native `multiplayer_select_0220` and `_0440` captures
in `select-split-material` were opened individually. The second shows independent
P2/P4 color changes. SHA 7BC5E1E72298A773B13962997361248DEDD3DBF511E5E6F495E8DBC2F59E4EA0.
Exit 0; real root settings/interface/cards unchanged. Final acceptance runs add
contextual bindings, navigation arrows, Back handling and Ready handoff.

The earlier 2P submenu carryover was a stale native pad ring sample, not a
physical held button. Draining GP+CF8 into GP+D08 after release restores the
Versus/Cooperative/Quest submenu; `selection-two-queue-drain` verifies it with
physical pad 3 owning P1 and pad 1 owning P2. 73 controls contracts pass.

Normal menu launch has also been connected to existing split gameplay through
session state separate from the development environment flag. `match-four-launch`
(before the new shared selector) verified four selected humans and two bots,
real owned pad images for P3/P4, and a normal Route 66 launch; gameplay 120/300
FXAA captures opened individually. That run proves launch only. The earlier
`match-four-first` fixture is INVALID (malformed input token prevented capture).

Remaining goal work: final shared-selector acceptance including imported cars,
2P six-combatant launch, 3P layout, extra-player pause/feedback, original results
and scoring inside the extended render loop, rematch and full teardown. The
earlier sequential proofs are superseded and are not UI acceptance. No goal
completion, commit or push is authorized by this increment.


### Paint handoff correction and latest staged build

`select-split-four-final` and `select-split-two-final` on 6F2F23FB both reached
normal gameplay with six combatants; 2P join order was physical 3 then 1.
Their selector images were inspected. Gameplay revealed P2 paint reset.
Native LOAD reads `8006B9E8 + player*180 + type*10`, whereas the first shared
selector wrote all retail paint to P1's row. The selector now uses the proper
per-player records; P3/P4 records live in existing extended session storage.
LOAD's paint pass covers only two humans, so the extra-human construction
hook extends the same 800365E0/80036558/8001EF34 calls before spawning P3/P4.
No shared gameplay palette is guessed or substituted.

Current staged SHA256:
`AAC65A6E14C8B202F075FEDC7E330DCE40728748239A23769425D8373066BB93`.
`select-split-four-paint`: exit 0, six combatants, active pack, user files unchanged.
Native gameplay 120/300 opened individually: P2 brown and P4 salmon paint survive
loading. 73 controls assertions pass in `controls-shared-selector.log`.
`select-split-cancel` (6F2F23FB): P1 Back returns to location generation 2.
Its capture at delay 60 catches the native flip transition, not the settled arena
screen; the durable runner now requests 220 for a settled visual proof.
`tools/recompone-v8-2/run_local_multiplayer_menu_proof.py` provides hidden,
user-data-isolated shared-selector runs and checks required final stages plus
six-combatant verification. It has --two, --three, --imports and --cancel modes.


### Accepted shared layout and independent preview texture ownership

User approved 3P/4P character select, conditional on proper 3P validation.
The layout remains as approved. The last failed tests and their fixes:

* 7F94C352 `select-split-imports-owned` and its diagnostic timed out after
  repeated swaps. Managed stack: B1F4 -> 20A80 -> native fatal 15D9C.
* 29B0A8C1 `select-split-imports-surface` reserved/reused one native 280x104
  drawing surface, but then reached the same fatal allocator during a vehicle
  texture upload. This run is also failed, not acceptance.
* Root cause: ActiveEntries recognized a constructing preview through the old
  sequential selector index. The shared selector uses the authoritative
  `_constructingEntry`, so its textures were untracked. Additionally the old
  reusable texture pool belonged to one preview, not multiple live panels.
* The registry now recognizes the authoritative constructing entry. Shared
  previews own separate reservation sets keyed by their unique selected type;
  changing a panel retires only that set. The existing single-selector pool
  keeps its prior default path. Actual native descriptor/backing lifetime
  checks prevent double-free. Scene exit drains any remaining local owners.

Final staged SHA:
`5658CA79EF775BA6D89F29ACF47657118E198966DB2E688F510B1F0892BD0771`.
`select-split-imports-isolated` completed 4500 polls, exit 0, user files unchanged,
active texture pack, four imported humans and two bots. Four simultaneous left
navigation rounds exercise repeated swaps; final previews are Beezwax, Molo,
Sid Burn and Y. Native selector 700/900 and gameplay 120/300 opened individually:
independent textures remain correct and Molo's edited paint survives loading.
At handoff all local preview texture reservations and live bank pointers retire.
97 VramPackingContract scenarios and 17 GuestTextureContract checks pass.
The first GuestTextureContract invocation omitted its required CUSTOM.EXP
argument; that invocation failed before the package checks and was rerun with
`V8_2_LOOSE/mods/v8_to_v82_guest_roster/CUSTOM.EXP` successfully.

`select-split-three-surface` (same final SHA) completed with 3 humans/3 bots;
selector600 and gameplay300 reviewed. A second 3P run `select-split-three-final`
adds P3 paint, P3 ready/unready/re-ready, and attempted input from an unjoined
physical fourth pad. Final capture review is recorded below.

Do not treat selection acceptance as completion of the multiplayer goal.
Remaining runtime work includes pause/results/scoring/rematch and feedback;
3P currently uses the large native HUD in quarter views, with vehicle-icon
ownership requiring correction/verification. 4P retains the approved minimal
HUD. Two-player alternate modes and final session teardown need completion.
No new FPS benchmark, five-hour soak, commit or push was performed.

### 3P selector approval and dedicated verification

User approved the 3P/4P selector subject to proper 3P testing. The final
`select-split-three-final` replay passed on staged SHA 5658CA79: 3600 polls,
exit 0, active texture pack, unchanged root user files, three human vehicles
and three bots. P1/P3 navigated independently, P2/P3 edited and accepted paint,
P3 readied/unreadied/re-readied, and an unjoined fourth device's A/right input
neither created a fourth player nor altered the active selection. Match launch
waited for the last joined player (P2) to ready. Native logs confirmed the two
edited 000008 colors at match handoff.

Native selector captures 440 and 625 and gameplay captures 120 and 300 were
opened individually. The selectors show independent paint and ready state;
the match retains the chosen vehicles and paint across the three cameras.
The fourth quadrant stays empty. Selection approval is now recorded in the
tracker. Staged and published executable hashes still match.

The gameplay captures also show the separately recorded native HUD problem:
vehicle silhouettes are incorrect/changing across player views and the 3P HUD
is oversized. This is not selector failure and is not signed off. The overall
multiplayer goal stays active for HUD ownership and remaining lifecycle work.

### Native HUD interpretation correction and pause integration

The earlier claim about incorrect vehicle silhouettes was an interpretation
error. Native 80037A00 at 80037E4C loads vehicle+F0 (current target), then the
target's +7C icon and health. A different/changing silhouette is expected and
is not evidence of cross-player HUD ownership corruption. No asset or targeting
change is warranted. Three quarter views now use the same approved compact HUD
as four quarter views, eliminating the oversized native HUD layout.

The expanded renderer previously jumped past the native pause routine entirely.
It now draws native 80011C58 once in the full display and returns its pause state
to the main loop; P3/P4 menu input and the native connected-pad check use owned
extended storage. The host build now applies split seams after quest seams,
including SplitRelease, which was missing in the actual compiled main source.

First test 41487118 (`select-split-three-final-pause`) exited cleanly and reached
3 humans/3 bots; gameplay300 compact HUD inspected. Pause assertion failed:
the first implementation fed mapped driving controls into the menu accumulator,
which loses Start. Native15540 retains raw menu edges separately. The correction
uses those raw edges; this first replay remains a failed pause test.

Second pause replay (`select-split-three-final-pause-raw`, SHA52393224) is also
failed: state transitions alone were insufficient. The native paused branch
polls input and jumps straight to rendering, bypassing the normal tick aggregate;
its extra-player menu image remained stale. The split renderer also omitted the
VBlank presentation supplied by the original draw/display callback. It consumed
the stale Start and then stalled at tick402. Harness timeout=-1; user files
unchanged. No acceptance is inferred from those two state trace lines.

Current correction consumes fresh extra menu input on the native paused path
and explicitly presents a paused split frame (including the resume frame).
The proof now requires both paused520 and resumed740 native captures as well
as correct owner/state transitions and a clean finite exit.

Next lifecycle RE locations: native mode5 result branch 80012CDC only chooses
between the first two humans and updates GP+CB8/CBC. Native 80012E44 contains a
four-human result variant, but changing the gameplay mode to select it would
alter rules; extend/generalize the result presentation and score storage from
that source instead. Main-loop 80013FC8 waits for message queue8006BB90 and
GP+C74 before constructing results. The split renderer currently bypasses this
result path, so full results/rematch remains incomplete. Native pause split-row
visibility also needs adapting for fixed 3P/4P quarter views.

Additional results RE: native vehicle destruction at 800389F8 sets GP+C74
when any human dies in modes below9. Merely enabling the skipped result draw
would therefore finish a 3P/4P match on the first human loss. The generalized
six-combatant versus flow must account for all joined humans and remaining
bots before producing results. Mode5 also increments GP+CC4 per destroyed
vehicle (800389D0), so reusing GP+CC4 as a fourth score would conflict; keep
extended session scores in owned storage. No result-rule patch is made yet.

17EC0C21 pause-state lifecycle tests completed for P3 and P4: correct owner,
menu capture, resume capture, continued simulation and exit0 with root files
unchanged. P4 screenshots520/740 individually reviewed. 3P screenshots520/740
caught stale modal pixels in the unrendered fourth quadrant and a 2P-only split
orientation row. F721E4E3 now clears that quadrant on both buffers before the
full-screen modal and disables the native split row for three/four players.
Native two-player orientation controls remain unchanged. Final 3P visual check
is running; the compact HUD uses original sprites without extra backgrounds.

F721E4E3's automated pause lifecycle checks passed, but individual review of
520/740 still found stale Resume pixels in the fourth quadrant after closing.
This is a failed visual check despite the then-current result.json booleans.
ClearImage cleared VRAM only; the enhanced display retained the modal pixels.
49A20A9E replaces that clear with native8001ADF8's untextured TILE packet drawn
in the fourth viewport, using the normal GPU draw path. Verification running.
The runner now also checks the unused quadrant's interior is black at resumed740.

Remaining device lifecycle detail for follow-up: SplitHarnessInput currently
writes processed type2 even when an extra physical controller disconnects, and
SplitMergeMenuInput forces Start on every disconnected merge, including paused
merges. Match the native behavior: disconnected processed type0; synthesize the
initial pause trigger only in the unpaused aggregate, while paused input remains
neutral and the native reconnect message uses the correct device type.

Final pause/HUD milestone: `select-split-three-final-pause-tile` passed on
49A20A9E65D1E9C26CE735D73C59D2FEA1EA3C35552FB48938C4EEC491A9E0BF.
3600 polls, exit0, active texture pack, 3 humans/3 bots, root user files unchanged.
P3 paused/resumed at tick402, then simulation advanced past900. Native520/740
captures opened individually: native full-display pause menu has no split row,
resume removes it completely, compact HUD remains correct, empty fourth view
returns to black. Additional pixel assertion on the resumed unused quadrant
interior returned RGB extrema (0,0) in every channel; result.json records it.
Staged and published executable hashes match. P4 native pause/resume was verified
on17EC0C21 earlier in this same milestone; subsequent rendering cleanup applies
to the empty 3P quadrant and the shared fixed-layout pause row.

All launched build/harness processes have completed. Goal remains active for
reconnect/quit, six-combatant end conditions, results/scoring/rematch and remaining
mode/session lifetime verification. Do not mark the overall goal complete.

Rematch RE follow-up: native main800135C8 performs normal level initialization,
then reads SP+6C and jumps directly to80013640 when nonzero, bypassing SHELL.
Results main80013D58 sets SP+6C from Triangle's edge (00200000). Preserve this
native rematch entry rather than inventing another shell loop. A local-session
rearm at80013640 must recreate extended state after SplitRelease, retaining
joined device ownership, selected vehicles, and the extra humans' 360-byte
upgrade/color storage (SplitStorage+800). New joins should reset session scores;
level teardown must free per-level resources while preserving rematch data.
Normal menu return calls V82LocalMenu.Enter and resets LocalInputSession; the
existing rematch skip correctly avoids that reset. No rematch patch yet.

### Reconnect correction

Extra controller processed type now reports0 when absent and2 when connected.
The synthesized disconnected Start belongs only to the unpaused aggregate,
matching the native two-pad path; paused input remains neutral instead of
repeatedly forcing a resume. Test-only PHYS:DISCONNECTED pulses neutralize that
physical device before LocalInputSession routing and restore it at pulse end.
They generate no OS input and do not operate a desktop window.

3P replay `select-split-three-final-pause-reconnect` passed on
429812D84B51DA1C814E7E1163ED533D7BF21A803610BD373D1047BC014657D3:
3+3 combatants, pause at401, disconnect/reconnect recorded, owner3 resumes at401,
subsequent simulation advances, exit0, root user files unchanged. Native520/740
opened individually: REPLACE CONTROLLER3 is correct, resumed game is clean,
and unused fourth quadrant pixel check remains black. P4 equivalent running.

P4 equivalent `select-split-four-paint-pause-reconnect` also passed on429812D8:
4 humans/2 bots, disconnected and reconnected the fourth physical fixture pad,
owner4 paused/resumed at tick401, simulation advanced afterward, exit0, and
root user files unchanged. Native520/740 individually reviewed: REPLACE
CONTROLLER4 appears; no obsolete split row or modal residue after resume.
Both final reconnect runs include the approved selector and active texture pack.
Staged/published SHA429812D84B51DA1C814E7E1163ED533D7BF21A803610BD373D1047BC014657D3
match. No build or harness is left running. Previous goal turn was progress;
this turn completed the extra-device reconnect behavior and gathered native
rematch/result RE for the next implementation. Overall goal remains active.

### 3P quit and fresh match in one process (2026-09-14)
The process-local `--three --quit --tag quit-reentry-arena` replay passed on staged 429812D84B51DA1C814E7E1163ED533D7BF21A803610BD373D1047BC014657D3: exit 0 after 6000 polls, root user files unchanged, two separately constructed six-combatant matches (3 humans + 3 bots), native P3 Quit/Yes, resource release, fresh join and arena selection, second gameplay. Individually inspected native quit confirmation and second-match frame 300; compact HUD and black unused quadrant correct, texture pack active. No game-code change was needed for this quit path. Initial fixtures failed because returning SHELL ignores input during its native transition and the second arena uses choose_location_2; both fixture omissions corrected, original failures retained. This is fresh menu reentry, not results-screen rematch. Results/scoring/rematch remain open.

### Native expanded results and rematch (2026-09-14)
Implemented versus last-human-survivor completion through the original death transition and post-tick end check. Native 80038870 no longer ends an expanded round on the first human death; native 80012930 uses its existing four-player result branch with joined-player bounds, scoped human-pointer storage and separate score words (GP+CC4 remains destroyed-car count). Original message queues drain before construction; main S6 owns the result object and its native teardown. Native SP+6C restart now restores only roster/profile/score values into new split storage before LOAD, retaining actual joined device ownership. A fresh SHELL session discards rematch snapshots.
7030D2FC2DBD861A777EC139053E1359F3A93EF4F11A8B4518D036245BFB7CB0 passed dedicated 3P and 4P two-round process-local fixtures: first human dies at180, remaining losers at240, P3/P4 wins respectively, scores1 then2, native restart, six combatants reconstructed, results quit back to SHELL, exit0 and root user files unchanged. Captures of both rounds for both runs individually inspected. Paths select-split-three-final-results-rematch and select-split-four-paint-results-rematch-p4. Native result text currently renders with uneven column alignment and PS1 button glyphs; these are outstanding presentation/prompt work, not final acceptance.
Follow-up source audit found GP+C88's eight-byte round-award accumulator also assumed two humans. Added proven collector seam in80049D54 and result-loop extension to all joined humans, with P3/P4 profile address routing. Award storage F30-F37 is separate from score F00-F0F and result-pointer F20-F2F. Current staged/published03B43CD7AF85ABD4144B754104BBF89090FE14327D87DEB906FEF51DBCCD1248 has these changes; dedicated P3 award/rematch fixture running. No claim of full goal completion: results alignment/contextual prompts, draw cases, imported winner/rewards, 2P modes, selection audio and final regression audit remain.
Final award follow-up: both select-split-three-final-awards-p3 and select-split-four-paint-awards-p4 completed5500polls exit0 on03B43CD7; P3/P4 awards0->3->6 verified by native result-loop assertions, scores0->1->2, rematch roster6 and rootfilesunchanged. Four final result captures individually inspected; known alignment/glyph presentation issues remain. No process/build remains running.

### Results prompts and draw verification (2026-09-14)
Results now refresh the native menu input context and format the original footer through InputPromptLabels using the joined P1 device. Actual resolved gamepad prompts are B restart / A quit. Process-local physical P3 B successfully restarts and physical P1 A quits on staged533F3905A7821F6391138486FA1E7AE796A8F93463C8517524CFC7AA7442488D; two rounds retain score1->2 and awards3->6, clean5500poll exit, rootfilesunchanged. Native captures from both rounds individually reviewed. Four-player draw fixture also passes: all humans eliminated, winner0, scores0, no native13268 result-voice path, DRAW capture reviewed, clean4500poll return to menu. Draw currently omits the scoreboard; retaining scores visibly is part of pending result presentation work.
Alignment hypothesis rejected: prefixing native font control15 did not change layout and was removed. C7F5767F font-prompts run failed because its fixture pressed X while the actual resolved restart button was B; it timed out and must not be counted as a pass. Result overlay now uses the HUD draw scope, but column alignment remains unchanged and open. Native19614 measures until a non-color control byte; native198B8 control7 positions relative to S6, control5 subtracts half measured width, control15 resets FP only. Layout needs concrete packet/text evidence or a clean native layout adaptation, not more speculative font-mode changes.
Current imported-winner fixture is running: select-split-imports-isolated-imported-result. No changes to runtime after533F3905A staging. Goal remains active.
Imported-winner follow-up: the initial imported-result fixture was falsely green for end-flow only. Source/log review found TraceResultFormat limited the voice override to players0/1 (P4 fell through with invalid native stem0xD03), and the award fixture/result both referenced an out-of-range retail row for a non-quest imported driver. These do not count as imported acceptance. Extended voice override to joined human count; versus result awards now use Registry.UpgradeStatusForObject per-vehicle records, captured before native333D0 and restored after rematch spawns. Quest save schema remains unchanged (its three slots are not a versus allocation). New imported voice assertions added. Staged71B746110704ECC663F5BB3DDA13129D843F964318B63A1719FF2A8B8B54886A running imported-results-owned two-round fixture. Durable patch composition adjusted so native quest and split patchers remain idempotent; helper after A3 assignment is semantically equivalent to this build's helper before it.
Final imported-results-owned verification: clean5500poll exit, six combatants each round, imported P4 Y wins twice, owned-record awards0->3->6 and score0->1->2, physical P3 B rematch/P1 A quit, userfilesunchanged. Both native results350 captures inspected. Logs route guest.v8.y_the_alien to Shared/V8VOICE/V12.xa and normalized XA channel0 in both rounds; loose V12.xa exists429824bytes. Voice route assertion evaluated post-run and recorded separately in result.json because the strengthened harness was saved after this process started. Muted test verifies routing, not an audible listening test. No live build/process remains. Results alignment and visible draw scoreboard still pending, along with remaining full-goal checks.

### Results positioning correction
The explicit native-font layout initially still shifted titles and labels left: EnhancedGlBackend classified result glyphs as gameplay HUD/location captions. Scoped the existing NativeModalHold around result packet submission, restoring the previous hold afterward. Build 84DC6C97553838280F3B791C4BE803CEA6EEF38E7D4414134B32538E3790A067 is staged. select-split-three-final-modal-results passed two rounds; independently inspected results1/results2 at tick350: aligned header, statistics, three player scores, B restart/A quit. Scores advance 1 to2. This verifies the 3P result correction; remaining full-goal gates are still open.
The same staged build passed select-split-four-paint-modal-win: native P4 win, score0,0,0,1, award3, six combatants, exit0 and unchanged user files. Results350 capture independently reviewed: header/statistics/footer centered correctly and four score columns aligned. Background remains the live split match; no additional background treatment introduced.
select-split-four-paint-modal-draw also passed on84DC: winner0, all scores0, clean return to menu, unchanged user files. Results350 independently inspected: DRAW and all four scores are visible and aligned. This test starts with zero scores; a draw following prior wins remains an edge-case gate. No test process remains.

### 2P native modes audit
Previous turn classified as progress: result positioning corrected and three native result runs validated. This turn added --two --mode cooperative/quest harness routes. coop-audit initially failed because fixture did not press at native ready prompt (the Versus-only multiplayer_match stage does not occur); no game defect claimed. coop-ready on84DC passes native mode6 and gameplay, two human spawn variants -1/-2, three selected enemy spawns, unchanged user files. gameplay300 native capture reviewed: correct two vehicle views and native HUD in configured vertical split. This is five total selected combatants, not a six-combatant proof; native selectable enemy maximum and remaining mode flow still require audit. Native source select interception is restricted to mode5, so Cooperative/Quest retain their existing selectors. Quest fixture now running.
Quest-audit and quest-late verified mode7, two human spawns plus the authored single enemy, separate native split views and advancing simulation with user files unchanged. Both reviewed captures retain the objective briefing; these passes do not prove briefing dismissal. quest-dismiss used physical Start and entered native pause (late capture reviewed), so its simulation assertion failed as expected; retained as a fixture failure. Source 80011FC0 tests08400000 but outer match Start handling takes precedence. Testing a fresh virtual Cross edge after loading. ControlsContract rerun on current source exits0, including Enter confirmation, first-join P1 ownership, non-P1 Start rejection, disconnect neutralization and selector routing.
quest-ack passes on84DC with a fresh P1 virtual Cross edge at aftergameplay400. Individually reviewed aftergameplay1000: briefing gone, both HUDs restored, independent vehicles visible and simulation advances beyond frame300. This is virtual native-action acknowledgment, not a physical remapping proof; physical control labels for briefing remain part of contextual-input audit. No runtime change needed. All test processes terminal.

### Selector sound feedback
Added native navigation/accept/back effects to the simplified selector using func8001E14C, SHELL bank pointer80116738 and native color channel2. Samples0/6/11 match native calls at80104414/80104468/80104518 and80107564. Per-frame priority coalesces simultaneous player events so lower-priority navigation cannot interrupt confirmation. Build0A11B1C074233F72C179DF29FD62CCB44616E0A96723CBF9FB6BD230C116D8BC staged. Three-player selector-feedback fixture exercises color/ready/unready, with SPU key-on trace immediately preceding every event: sample0 start0202,6 start1454,11 start1FDE, each pitch0800 and volume1000/1000. Muted process-local test verifies native bank dispatch, not audible listening. Fixture still completing gameplay at time of this entry.
Selector-feedback run completed exit0 with six combatants and unchanged root user files; samples0,0,0,6,6,11,6,6. All eight corresponding SPU key-ons verified post-run and recorded separately in result.json (strengthened assertion was saved after process launch). Selector680 native capture independently reviewed; approved three-panel layout preserved. No live build or harness remains. Overall menu integration goal remains active.

Solo reentry on0A11B1C0: three humans+three bots, P3 native Quit/Yes, main menu, solo Arcade selection, original character/enemy setup, second gameplay. Native main-menu exit entry2 records result1, only one variant-1 human spawn follows and no split setup occurs. aftergameplay2_0300 individually reviewed: full-screen solo vehicle, native HUD and active pack. Process exit0 at6000polls; rootfilesunchanged. Initial harness assertion falsely required the once-per-process callee-repair log to repeat with mode1; corrected to native menu return value. Original failed assertion retained in result.json and post_run_solo_mode_verified=true added after validating the actual evidence. Requirement audit now lives in notes/local_multiplayer_acceptance.md; remaining2P roster/results coverage is explicit. No live jobs.

### Final2P coverage
current-roster on0A11B1C0 exits0 with LocalMatch humans2/bots4 and V82SplitVerified combatants6; gameplay300 native capture independently reviewed. Added test-only two-player branch inside SplitCheckRound, gated by RECOMPONE_V82_SPLIT_RESULT_TEST and joined count2/mode5: calls original80038870 on the losing human after180ticks and observes native mainS6/result scores. Does not replace native win rules, scoring, input or rematch. Uses current native pointers each round so it can inspect native rematch after extended storage is released. C3E98DB50D677853A4D04CE537ED542DCB33F8A8E3D02949B19D661817780F5B staged and two-player winner2/rematch replay running.
Native2P result replay onC3E98DB5 failed: first score0,1 appears, but physical B did not restart; first350 capture shows legacy PS1 glyphs and HUD anchoring shifts. This is a real integration gap, not a passing fixture. Extended shared result text layout to joined2P/mode5, reading native CB8/CBC scores; added ongoing MenuContext and native modal hold while mainS6 owns a2P result. Native gameplay/scoring/rematch remains intact. Fix build running.
8435696B first2P fix still failed physical restart. Its shared layout constructs successfully, but context was refreshed from simulation check, which does not run for every displayed results frame. Moved ongoing MenuContext/modal hold to proven native result draw sites RA80013FC0/8001465C inside main800132CC via durable patcher. Two-result context build running. Original failed run retained.
The8435 first fix capture was reviewed: explicit layout is aligned and displays B restart/A quit, but ongoing input failed. Actual native2P result draw calls at main13FC0/1465C now refresh MenuContext and the native modal hold. 1B15DA12356CA6355F7BDFAAD036DCF5E59A86EEE898EC5EA736E21F2212FDF6 staged; context replay running with explicit second native ready-prompt pulse. No native scoring/death/rematch path replacement introduced.
native-results-context on1B15 exits0, physical B starts rematch and physical A returns to main menu; native score0,1 then0,2. Both result layouts are aligned (second350 inspected). Roster assertion correctly fails: second native round spawns only the two humans. Extended value-only roster snapshot/rearm to joined2P; native2P upgrade rows and native CB8/CBC scores remain untouched. Extended3/4P record restoration still guarded >=3. Roster fix build running.
complete-roster on3401327A86A9081F9B6904F10972A35ACA5437A7BE055CC0AAF5F7A53DE18928 passes all2P assertions: six combatants on both initial/rematch spawns, twelve vehicle constructions total, scores0,1 then0,2, physical B restart/A quit, clean5500poll exit, userfilesunchanged. Both result350 captures inspected individually and aligned. Final audit must also check existing Cooperative/Quest result continuation: their entry/gameplay was verified but native result input context currently has a mode5 guard. Do not claim these result paths verified from the Versus tests. No jobs remain.

### Cooperative and Quest result continuation
Extended native result context from2P Versus to every joined2P result draw. Added process-local COOP_RESULT_TEST fixture: enumerate real native active vehicle objects, exclude both human pointers, call native80038870 on enemies after300ticks; observe native result object. Cooperative-end onD986 exits0 and physical P1 A continues to SHELL, with native team victory. First350 capture reviewed: team stats preserved but old PS1 footer glyphs remained. Extended existing footer label resolver to2P native modes using temporary192byte buffer copied by native strcat and freed before font packet construction; no objective/stat layout replacement. F3D47480F9E61C3DE27FA9B576B4E1BEFBEA59521DAD5368A245DC36D772EF01 staged; Cooperative two-round fixture running.
cooperative-rounds onF3D47480 exits0, ten total vehicle constructions across two native rounds (two humans and same three selected enemies each), physical B restart/A quit, and unchanged userfiles. Both350 native result captures individually reviewed: player-team scores1 then2, contextual B/A footer, native team statistics preserved. Quest end/continue fixture is now running on the same staged build.
Quest-end onF3D47480 failed to enter results after the fixture directly invoked38870. The prior inference about mission-objective gating was premature. Source comparison with V82Compat.CompleteQuestCombatFixture/UpdateDefeatRegression shows the complete weapon/collision transition is80039DCC(vehicle,-0x7FFF,vehicle+34,1); direct destruction callbacks bypass damage bookkeeping. Updated only the test trigger to call that lethal-damage entry, without forcing victory or objective flags. Quest-lethal build running.
quest-grace on78A582AC exits0 at7000polls: native lethal damage, no forced objective/result flags, original quest grace period, native quest_result at4639 and observed result at4641, physical P1 A at5061 continues to SHELL. Results350 individually reviewed: YOU WIN, A No/B Yes/C Yes and contextual Press A to continue. Rootfilesunchanged. Earlier4500poll tests ended before result entry and are not passes. Published/staged hashes match78A582ACE49EEE37DA43C76301442FF1AA6DC9A4D765E29F9A08E6B4268497AA. Requirement audit complete; TO-DO pending user review. No test/build process remains.
