# Three transformation policies

Status: implemented; native integration evidence below, user visual approval pending.
Current candidate `E3F11E82CF0B7433CF9CB6954A0375412478BC52A365D685DE6D52886A744163`
was individually deployed to `V8_2_LOOSE/Vigilante82PC.exe`. The previous binary
is retained at `artifacts/transformation-modes-20260904/pre-transformations.exe`.

User-facing location: **OPTIONS → Gameplay → Transformations**. Full choices:
All transformations (default), No transformations, Auto Waterski. The saved
`settings.json` property is `V82Transformations`; absent/invalid values retain
the native All policy. The Gameplay page shares the native Video page input,
footer, and text compositor, not an external Windows menu.

## Recovered native seams

- `80049D54`, message 3: collision pickup handler; item kinds 7–9 request
  transformation modes 1–3 with damage shield 500. Reject before mutating either
  participant, while leaving unrelated pickups unchanged.
- `8003E32C(vehicle, mode, shield)`: authoritative transition request.
  `vehicle+AC` is mode; `+B4` is 32-tick transition; `+12C` is a damage shield
  depleted by `80039DCC`, not a lifetime clock. Auto requests zero shield to
  avoid continuously granting the powerup's extra damage absorption.
  Busy transitions return without changing mode. Mode zero restores wheels.
- `8003E774`: native animation updates six support children, rebuilding their
  native mode models at transition tick 16, completing/reparenting at zero.
- `80041B0C`: common movement dispatch reads mode after servicing transitions.
  Mode 0 calls `8003EDC4`, mode 1 calls `8003FCC4`, mode 2 calls `80040598`,
  mode 3 calls `80040F10`.
- Mode 2 is the water/float movement path (terrain-height versus water-plane
  branch at `800405E8`–`80040610`); mode 3 is separately named Ski in the
  asset tools. Do not confuse the user's Auto Waterski label with that byte.
  Native water attachment visuals were inspected in the separate full-size
  water/land captures below; user visual acceptance remains pending.
- Native ground/hover drowning branches compare global water plane `GP+DB0`,
  vehicle Z against `GP+DA0`, and vehicle Y against `plane+5000`, calling
  `800391AC`. Converted original-V8 arenas also have shared region-driven
  drowning in `V82Compat.UpdateImportedWaterDrowning` (RECT 0043/-1).
  A global plane-only approximation would incorrectly treat other areas as
  dangerous; both native and converted hazard contracts must be respected.
- Flying capability comes from `V82VehicleRegistry.UsesFlyingController`,
  not a Y/name/type exception.

## Implemented, not yet accepted

- Shared pickup and activation gates preserve native All behavior; modes None
  and Auto reject powerup/direct nonzero transformations. Only a scoped native
  request for the current vehicle can authorize automatic mode 2.
- Common `80041B0C` pre-physics controller probes current position and four
  samples along the next 32 ticks of motion; native terrain/water boundaries
  and converted hazard regions select water approach. The native transition
  owns all model animation and physics. No position/velocity overwrites.
- A 20-tick dry-ground interval plus spatial margin delays retraction; being
  airborne above water is not treated as returning to land. Native transition
  busy state is honored. Other transformation modes retract in None/Auto.
- Match/object lifecycle clears ownership state. Converted drowning now
  recognizes native water movement as well as inherent flight.
- `TransformationContract`: **237 assertions pass**, using actual runtime
  policy and OPTIONS row code. The controller tests substitute only native
  terrain lookup/transition calls to test request timing/state; they do **not**
  establish native buoyancy, model animation, or gameplay correctness.

## Initial acceptance checklist (subsequent evidence below)

- First native OPTIONS run exited cleanly, cycled All → None → Auto, and
  serialized `V82Transformations: 2`. Original user settings were restored
  byte-for-byte (SHA256 FB0C93BD6575C23E7A9DF4710AEC89807DA90532A8829D60DE6EC870C9A43E36).
  Inspected full-size native presentation at stage poll 700 / absolute poll
  2184: Gameplay and Auto Waterski are legible. Nearest preceding 60-frame
  performance window reports 59.49 FPS, capture window 60.22 FPS; these are
  menu-only measurements, not gameplay performance certification.
  The hint was too close to the bottom border; candidate 8AC6 moves it up 32
  native layout units. The final native recapture is documented below.
- Native water attachment/physics, predictive timing before drowning,
  shoreline hysteresis, ground retraction, human/AI and native/imported paths.
- Damage/respawn and repeated-match reset handling.
- Hidden silent, single-game integration tests in V8_2_LOOSE; OPTIONS cycling
  and persisted restart; native full-size OPTIONS and water/land proofs.
- Exact executable hashes, capture points and measured FPS for proofs.
- Do not mark complete on unit tests alone or resume the paused renderer goal.

## First native gameplay evidence

`artifacts/transformation-modes-20260904/auto-native-01` retains a clean-exit
Florida run on candidate 8AC6. The process-local fixture places player and one
AI once at native deep-water coordinates at gameplay frame 90, then restores
their initial poses once at 360. It does not hold/heal vehicles or replace
movement, buoyancy, animations, damage callbacks, or native controller logic.
This tests entry and retraction, not a continuous driven shoreline crossing.

- Sheila requests water mode 2 at tick 90, settles with native transition zero,
  and retains health 1024 and her initial callback through the water samples.
  No pickup shield is granted (`+12C=0`).
- The selected AI independently deploys mode 2 and remains afloat. Its initial
  pose was already over water, so restoring that pose does **not** prove its
  dry-ground retraction. A later natural AI dry-ground request at tick 643 is
  logged; do not mislabel the frame-360 fixture as a dry AI placement.
- Sheila requests mode 0 after sustained dry ground at tick 369, completes
  native retraction by sample 390, and remains on wheels through 600.
- Individually inspected full-size 1920x1080 native presentations at gameplay
  frames 270 (water attachments visible, floating) and 540 (wheels restored on
  road). No sheets or desktop capture. User approval remains pending.
- Run gameplay performance windows: median 48.90 / minimum 37.87 FPS. This is
  a bounded fixture measurement, not certification of every map/route.
- Exact executable hash is recorded above and in the run's `proof.json`.
- Repeated-map validation runs separately without images. Any concurrent
  compilation makes its FPS unsuitable for performance acceptance.

Follow-up candidate `FAF7DC0A620154D49814B2FC7DEFE07A0E99D48C2432A47869131B3328DDD8F4`
changes only diagnostic fixture handling and OPTIONS source comments relative
to 8AC6, not shipping transformation logic. The first repeated run failed its
Louisiana position search; `auto-repeat-01/failure.json` records that failure,
including the process guard outcome. The corrected fixture samples terrain
more densely against native drowning depth, selects genuinely dry land for
both actors, and logs fixture failures without throwing. Focused coverage now
passes **227 assertions**, including reset/reallocation/death-state checks.

The corrected `auto-repeat-02` completed Florida → Louisiana → Florida in one
process, exit 0, with fresh actor addresses on each match. Both actors retain
health and their normal callback throughout water testing. Player returns to
normal wheels in all three matches; AI also retracts on dry land and deploys
again when it drives back into water. Time-window summaries deliberately flag
samples after actors have driven onto a different surface; those booleans are
not reliable pass/fail substitutes for following the position and transition
sequence. Generation 3 AI retracts at tick 369, is on wheels at 390, re-enters
water at 406, floats through 600, and retracts again at 712.

`native-all` exercised the actual native transformation action for player plus
four AI: modes 1, 2, 3 and normal 0, with 20/20 expected modes and native 500
pickup shield (zero for cleanup). The physical collision gate is separately
unit-tested; this fixture must not be described as physically collecting a
pickup. Its 28.35 minimum FPS is retained, with concurrent build contamination;
not a performance acceptance run. No renderer work undertaken.

Focused tests now pass **237 assertions**, including predictive detection of
a narrow water crossing before the end of the animation, no movement writes,
and actual linked RECT traversal outside the native Z boundary.

Candidate `5885C00DBE849E73FCC62E50DCC82338A1FCA8677580CCBD1FA4DCA7B78219F5`
adds only an explicit native-recovery event to the opt-in probe. Shipping
transformation logic remains unchanged. `native-none` exited 0: 20 requests
across the player/four AI reject modes 1–3 and allow normal cleanup, with zero
pickup shield throughout. Its measured gameplay median/minimum is 58.30/44.92
FPS. Existing real-runtime modal (42) and mesh-clipping (159) assertions pass.

The `guest-recovery` grouped probe finished exit 0. Florida/Y (type 76)
retains flight, health and its normal callback over water without skis. The
Hoover Dam/imported Chassey (64) water fixture is **inconclusive**: the loaded
map exposes plane=0 and boundary=0. No map-specific water plane was invented.
Florida/Sheila (0) completes a forced native recovery, but her skis had already
retracted before the event. That does **not** prove recovery while equipped.
Follow-up uses Florida for the imported ground vehicle and triggers recovery
only after the native water attachment animation has actually settled.

The same run logs original-game `Out of VRAM` allocation failures during guest
loading. Enhanced is explicitly confirmed at startup; this is not a switch to
the old renderer or proof of physical PC GPU memory exhaustion. The existing
loader recovery can omit affected textures, so this remains a real issue under
TO-DO #14, not a clean visual pass. No additional images were taken in this run.

## Equipped recovery and imported ground vehicle

Candidate `E3F11E82CF0B7433CF9CB6954A0375412478BC52A365D685DE6D52886A744163`
changes only the opt-in test event timing relative to 5885, not shipping
transformation behavior. `guest-equipped-recovery` exits 0 after two Florida
matches, imported Chassey (64), then Sheila (0), in one hidden/muted process.

- Chassey: all water samples retain mode 2, health 675, normal callback and
  zero pickup shield. Dry-ground samples have mode 0 with animation complete.
- Sheila: explicit native recovery begins at gameplay frame 120 with mode 2
  and its animation settled. Native recovery chooses the destination. At 180
  the normal callback is restored and water equipment is retracting; at 210
  retraction is complete. Normal mode, callback and health 1024 persist through
  600, without a fixture land placement or healing.
- AI independently transitions as it drives through water and dry ground;
  sample-window booleans are not a substitute for that position history.
- Gameplay windows measure median 59.78 / minimum 44.90 FPS. Native texture
  allocation warnings recur during loading; this is **not** a visual texture
  integrity pass. No new screenshots or renderer changes in this sequence.
- Focused 237 assertions were rerun and passed on this source.

## Final native OPTIONS proof

`options-final` uses candidate E3F11E82 and exits 0. The full-size native
1920x1080 presentation was inspected individually: Gameplay is selected,
Transformations / Auto Waterski are legible, the instruction is clear of the
bottom separator, and the native footer remains visible. Saved mode is 2;
the user settings were restored byte-for-byte to the hash recorded above.
Capture stage poll 700 / absolute poll 2184; preceding window 60.21 FPS,
capture window 60.07 FPS. `options-final/proof.json` records the full executable
hash and output path. This is a menu proof, not gameplay FPS certification.

User-facing proof set: final OPTIONS above plus the separate full-size native
water and land captures in `auto-native-01`. The latter retain their actual
8AC6 executable hash; later E3F11E82 changes affect the opt-in fixtures only,
not shipping transformation behavior. No images have been composited into a
sheet. Visual approval remains the user's decision.

`native-auto-policy` completed exit 0 on E3F11E82: all 20 native transformation
requests for the player/four AI leave normal mode and zero shield in Auto,
including allowed mode-zero cleanup. No fixture failures or native fatal
diagnostics occurred. Gameplay median/minimum was 60.04/53.63 FPS. This
exercises the native pickup action, not physically collecting pickups; the
collision gate has separate real-runtime unit coverage. All fixture processes
have exited and the original settings are restored. No commit/push performed.

## Implementation-goal completion audit (2026-09-05)

The implementation goal requires deployment and native proofs **for user
review**, not an invented user approval. Delivery is complete; visual signoff
remains pending in TO-DO #6. This does not close renderer, texture-integrity,
or release-performance work.

| Goal requirement | Current authoritative evidence |
| --- | --- |
| Three persistent native OPTIONS choices | Config enum/labels, ConfigManager load/save normalization, actual Gameplay row saves, `options-final/settings.after.json` mode 2 and inspected native presentation; 237-assertion contract covers all three serialized values and bidirectional cycling. |
| All retains original pickup behavior | Shared gates return to the original routines; `native-all` exercises native modes 1/2/3/0 for player and four AI, all 20 requests preserve expected modes and native shields. |
| No disables pickups/effects | Real collision-gate tests reject only transformation kinds 7–9 before mutation; direct gate and controller cleanup are covered; `native-none` verifies 20 blocked/cleanup native requests. |
| Auto equips native water mode without pickups and retracts on land | Actual pre-physics hook invokes shared controller, scoped authority, native mode 2 and zero pickup shield; `auto-native-01` water/land telemetry and separately inspected full-size proofs; `auto-repeat-02` confirms repeated player/AI transitions. |
| Activate before sinking; avoid shoreline animation churn | Controller tests cover current/predictive hazard detection, narrow crossings, native animation lock and 20-tick dry debounce; native water samples retain ordinary callback and health rather than entering drowning recovery. These are bounded integration fixtures, not an exhaustive driven-shoreline audit. |
| Inherently water-safe capabilities | Registry-authored flying capability bypasses transformation; `guest-recovery` generation 1 verifies Y over Florida water without skis or drowning. No map/name/type exception in the policy. |
| Shared player/AI and imported support | The common movement and authoritative action hooks have no human/AI branch; native action tests cover both, repeated water tests cover both, and `guest-equipped-recovery` verifies imported Chassey water/land transitions. Converted RECT traversal has focused runtime coverage. |
| Respawn and match reset | Factory Forget and match Reset calls are present; focused reallocation/death/reset assertions pass; three-match native sequence is clean. Equipped native recovery begins at 120 and is back to normal mode/callback/finished animation at 210 in `guest-equipped-recovery`. |
| Deployment, isolation and proofs | Deployed E3F11E82 SHA256 matches the final candidate; harnesses use V8_2_LOOSE, Enhanced, hidden window, dummy/muted audio, process-local input and single-process guard. Native proofs and metadata delivered; user settings hash restored and no live game remains. |
| Preserve unrelated work and paused goal | No renderer changes in this goal continuation, no cleanup of the protected root, no commit/push, prior dirty work preserved. Texture-allocation failures remain explicitly tracked under #14. |

The original-game texture-allocation warnings are not excused by successful
transformation tests. Guest visual texture integrity and overall release
acceptance remain unapproved. Evidence reports with zero native action
requests belong to the water profile and are not action-policy tests.
