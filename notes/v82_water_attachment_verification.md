# Water attachment verification — current coverage

Current follow-up: the user approved whole-assembly chassis anchoring. See `notes/v82_rigid_water_attachments.md`; the old vertex-stretching helper remains inactive. Historical findings below are retained chronologically.

Scope: all 30 installed vehicles (stock 0–17, imported 64–75). Y76 excluded.
Roster/source hashes: `artifacts/water-attachment-fit-20260905/roster.json`.
Current runtime has removed the experimental support fitting from object rendering (2026-09-08, user correction). It changed mesh geometry and was not a recovered native animation. Historical fit images and checks below do not establish canonical behavior.

This is a chronological evidence ledger: later entries explicitly supersede
earlier rejected or pre-correction runs. The current conclusion and remaining
gate are summarized after the per-vehicle entries.

Current correction: Convoy's six-support image exposed incoherent nearest-point
targets (rear bumper versus narrow chassis). The candidate now prefers an
inward ray from the end-face center and translates that cross-section together,
falling back to the nearest body point only when the ray misses. The
Torque, Trio, Houston and imported Chassey views near the start of this ledger
predate that correction; their later shared-rule regression entries complete
the required verification.

## Individually inspected candidate views

- **0 / Sheila, inward-rule regression:** `fit-sheila-inward-oblique`, build
  `2E61D9594B936370B28CCDA696C707999508597FE253156839212EC19BC90FB3`.
  Full-size Florida frame150, yaw60/pitch-25 degrees: visible near-side front
  and rear roots continuously join the body; far-side roots partly occluded,
  not an all-angle inspection. Capture47.94FPS, preceding53.18; run
  median58.47/min42.86. Complete initial/repeat water/dry cycles, exit0,
  no fatal diagnostics.8 cached meshes stay within assembly bounds and all
  source pointers are restored. User review/final enabled-build check pending.
- **1 / Torque:** full assembly in frame and all four visible connections
  continuous in `fit-stock01-inspection`, Florida frame150, native water mode2,
  transition0. Capture window60.07FPS; run median57.99/min43.71. Complete first
  and repeated water/dry checks, exit0, no fatal diagnostics.
  Build `412E1363F32998AE5E5470BB91246661ABA418457089FE879EDAE7A10482F2BB`.
- **64 / imported Chassey:** clean textures and visible connected arms in
  `fit-chassey-selected` (09556D24), after matching real carousel registry
  selection/reservations. Both cycles complete, but concurrent-game guard
  interrupted the run: **clean exit still unverified**. Lower assembly partly
  water-tinted. Earlier corrupt-guest images were INVALID fixture evidence.
- **2 / Trio:** motorcycle/sidecar assembly and all supports in frame in
  `fit-stock02-inspection`; visible arm/body connections continuous, with no
  floating gaps. Florida frame150, mode2/transition0; capture55.65FPS,
  preceding58.87; run median59.20/min44.73. Both complete water/dry cycles
  passed, exit0, no fatal diagnostics. Build412E1363...482F2BB.
- **3 / Houston:** complete truck and all four supports in frame in
  `fit-stock03-inspection`; visible roots join the cab/chassis sides without
  floating gaps, although water tint partly covers the assembly. Florida
  frame150, mode2/transition0; capture58.72FPS, preceding59.86; run
  median59.12/min43.03. Both complete water/dry cycles passed, exit0, no fatal
  diagnostics. Build412E1363...482F2BB. This is not an all-angle inspection.
- **4 / Convoy, corrected inward rule:** `fit-stock04-coordinates`, build
  `B3EE46F7281188F84066438645C902EDAADAF5FD77E833104F2FDD768D3E780C`.
  Individually inspected full-size Florida frame150 image: all six visible
  supports now continuously reach the narrow chassis; rear floating gaps and
  incoherent diagonal extensions are absent. Water tint remains over lower
  components. Capture55.36FPS, preceding58.87; median56.59/min43.65. Both full
  water/dry cycles passed, exit0, no fatal diagnostics. User approval pending.
  Read-only cached-copy audit:12 meshes within original assembly bounds,
  all source pointers restored, all primary source arrays match the native
  snapshot. Three nearby settled-water AI vehicles also passed these scoped
  numeric checks; this does not count as individual visual approval for them.
- **5 / Cultsmen, oblique view:** `fit-stock05-oblique`, same2E61D959 build.
  Full-size Florida frame150, yaw60/pitch-25: visible near-side front and rear
  supports continuously join the body at the wheel openings. Far side is
  naturally occluded; this is not an all-angle inspection. Capture51.21FPS,
  preceding49.29; median59.78/min41.53. Complete initial/repeat water/dry cycles,
  exit0, no fatal diagnostics. Supersedes the inconclusive rear-view proof.
  User review remains pending.
- **6 / Dallas:** `fit-stock06-oblique`, same2E61D959 build. Individually
  inspected Florida frame150 yaw60/pitch-25 image: visible near-side front
  and rear supports join the body. Far-side roots are occluded, not an
  all-angle inspection. Complete initial/repeat water/dry checks, exit0,
  no fatal diagnostics. Capture48.83FPS, preceding47.33; median57.42/min44.95.
  All8 fitted primary/LOD meshes stay within original assembly bounds and
  all pointers restore correctly. Maximum endpoint displacements are2816
  (front) and3529.30 (rear) fixed units; outboard geometry is preserved.
  Large triangular wheel-opening covers prompted a same-build fit-OFF
  comparison, `native-stock06-oblique`: they are also present without the
  candidate, so are not introduced by it. All four root slots63/70/59/77 have
  identical source vertices, local positions and matrices between captures.
  Baseline capture44.40FPS, preceding48.46; median59.39/min44.40; cycles and
  exit0 also pass. World pose/water tint differ between these independent
  native runs; this is not a pixel-identical paired simulation. User review
  remains pending.
- **7 / Nina:** `fit-stock07-oblique`, same2E61D959 build. Individually
  inspected Florida frame150 yaw60/pitch-25: visible near-side front/rear
  support roots join the body at the wheel openings; far-side roots occluded.
  Capture42.49FPS, preceding43.50; median57.44/min42.49. Initial and repeat
  water/dry checks complete, exit0, no fatal diagnostics. User review pending.
- **8 / Molo, usable retest:** `fit-stock08-near-trace`, diagnostic-only build
  `00926348A1CC78B96712F40F54178556059615DE9719E4130380F5F3B1272789`.
  Florida frame150 yaw60/pitch-25: all three visible near-side support roots
  join the body at the wheel openings. Far-side roots are occluded; rear ski
  tip meets the lower image edge, but the root connection is fully visible.
  Capture40.88FPS, preceding54.03; median59.99/min40.88. Both complete native
  water/dry cycles, exit0, no fatal diagnostics. The obstructing foreground
  geometry did not recur; do not call the earlier anomaly fixed.2335 near
  triangle records fit within the8192 budget; all have coherent valid
  view-space projections. This does not validate prior rejected packets.
  User review pending.
- **8 / Molo, earlier rejected capture:** `fit-stock08-oblique`, same2E61D959 build. **Rejected visual
  evidence:** large foreground polygons obscure the bus and support joints.
  Florida frame150 yaw60/pitch-25, capture44.18FPS, preceding45.39;
  median53.88/min41.01. Both native water/dry cycles complete, exit0 and no
  fatal diagnostics do not pass this view. Read-only cached-copy audit of
  the player and four water-mode AI vehicles finds all48 replacement meshes
  within original assembly bounds and all pointers restored (player12,
  maximum displacement6592 fixed units). This rules out oversized cached
  coordinates in this snapshot, not a rendered-polygon/provenance problem.
  Trace packet ownership before retrying the visual proof; cause unproven.
  Follow-up log-only `fit-stock08-packet-trace` (same build) completes both
  cycles and exit0, median59.91/min50.88FPS. The frame150 triangle/ownership
  trace reached its4096-line cap, including149 player triangles but stopping
  at OT163, so it is incomplete for near-camera geometry. No image was taken
  and this independent run does not reproduce or dismiss the rejected image.
  Next visual retry needs packet ownership plus view-space/source provenance
  for near-camera triangles, without consuming the budget on distant terrain.

- **9 / Clyde:** `fit-stock09-oblique`, same00926348 build. Individually
  inspected full-size Florida frame150 yaw60/pitch-25: visible near-side
  front and rear support roots join the wheel openings/body. Far side is
  occluded, not an all-angle inspection. Capture49.51FPS, preceding56.97;
  median59.95/min48.01. Complete initial and repeat water/dry checks, exit0,
  no fatal diagnostics. All8 player copies bounded and source pointers
  restored.1510 near-triangle records below8192 cap, all coherent/valid
  projections. No foreground obstruction in this image. User review pending.

- **10 / Obake, authored-transformation preservation:**
  `fit-stock10-asset-role-visual`,E2D1192B build. Individually inspected full-size
  Florida frame150 yaw60/pitch-25: the complete visible authored assembly is
  unobstructed and no generic-arm deformation is applied. Rear slot35 source
  vertices match the fit-OFF native comparison;lastRenderFit is null.
  Low/partly water-tinted presentation also occurs in the baseline; do not
  declare its flotation height or every obscured joint approved from this view.
  Capture48.70FPS,preceding52.83;median59.91/min33.45. Both full native water/dry
  cycles complete,exit0,no fatal diagnostics. User visual review pending.

- **11 / Boogie:** `fit-stock11-oblique`,sameE2D1192B build. Individually
  inspected full-size Florida frame150 yaw60/pitch-25: visible near-side
  front and rear roots continuously join the wheel openings/body. Far-side
  roots occluded, not an all-angle approval. Capture45.04FPS,preceding52.56;
  median58.91/min45.04. Complete initial/repeat water/dry checks,exit0,no fatal
  diagnostics. All8 player render copies remain within assembly bounds and
  source pointers restore;max endpoint displacement3042.53 fixed units.
  No large foreground obstruction in this capture. User approval pending.

- **12 / Bob O:** `fit-stock12-oblique`,sameE2D1192B build. Individually
  inspected full-size Florida frame150 yaw60/pitch-25: three visible near-side
  supports reach the rover chassis continuously; far-side roots partly hidden,
  not an all-angle approval. Capture44.74FPS,preceding60.88;
  median59.96/min43.49. Both initial/repeat water/dry checks complete,exit0,
  no fatal diagnostics. A recognizable shark swims beside/behind the vehicle
  without blocking its visible support connections. This suggests a possible
  source for earlier close-up polygon obstructions, but their exact packet
  ownership is not yet established; do not retroactively approve them.
  User approval pending.

Run directories above are under `artifacts/water-attachment-fit-20260905/`.
Each has full-size native PNG and `proof.json` with complete hash, capture
point/state and FPS. No sheet/composite inspection substituted for these views.

## Rejected captures and follow-up

- Convoy / stock4: `fit-stock04-inspection` (412E1363) is **not a visual pass**:
  rear supports appear disconnected despite all native cycle checks passing.
  All six roots did receive fitted copies (372 body triangles,12 meshes,
  48 vertices); direct source/runtime geometry confirms the old nearest-point
  rule selects the rear bumper for rear roots and splits middle root corners
  across different parts. First capture54.11FPS, median59.23/min48.97, exit0.
- Convoy shared inward-rule retest `fit-stock04-inward`, build
  `68DEE38016FC162D9CCF0C6B2B69B1E980E486C6607FFD6CFFC8DA320CCE328E`:
  **rejected/inconclusive visual evidence**. Large foreground polygons obscure
  the truck and supports; do not assert these are unrelated AI geometry or
  prove a fit regression without tracing their ownership/vertices. Root cause
  remains unproven. Both complete water/dry cycles and clean exit0; no fatal
  diagnostics. Florida frame150, capture40.24FPS, preceding48.78; run
  median59.05/min40.19. Native body372 triangles,12 fitted meshes,48 vertices.
  Next: trace fitted coordinates/rendered polygon ownership, resolve the
  obstructed/deformed view, then obtain a valid Convoy proof before expanding
  coverage. Candidate remains opt-in. Two images inspected this turn.
- The large foreground polygon failure from `fit-stock04-inward` did not
  reproduce in `fit-stock04-coordinates`; ownership/cause remains unconfirmed.
  Do not retroactively pass the rejected image or call the anomaly fixed.
- **5 / Cultsmen:** `fit-stock05-inward`, same B3EE46F7 build: cycle checks
  complete, exit0, no fatal diagnostics;8 fitted primary/LOD meshes, restored
  pointers, no fitted bounds expansion. Florida frame150 capture54.20FPS,
  preceding59.87; median59.87/min52.58. **Visual fit is inconclusive**: the
  wide RV body hides the support roots in the rear/elevated view. Retake an
  oblique view; do not certify joints from this image. Native camera routine
  `8004AFB8`, branch `8004B1B0`, adds camera+0x92 yaw to vehicle yaw; camera+0x90
  controls elevation. Add explicit fixture-only yaw/pitch options for the
  next capture without changing vehicle physics or the shipping camera.
  Follow-up: implemented bounded fixture yaw/pitch options, verified through
  the native camera and337 focused assertions; `fit-stock05-oblique` above
  supplies the missing visible near-side mount evidence.

## Remaining

Fixture-only update deployed as
`E64E81B41235B4CF9EFA67C2B0D7BD34F8A8F136DB10439A05B972FB2E83226D`:
optional `--water-site-separation-units 32` chooses another ClearWater-validated
point at least32 world units from the nearest site. Default0 preserves prior
behavior; bound0..128, fixed16.16 conversion,349 assertions pass. This relocates
only the fixture player once; no sharks/AI are hidden, frozen or modified.
The existing nearest site is repeatedly visited by sharks, compromising proofs.

- **13 / Garbage, rejected proof:** `fit-stock13-oblique`,E2D1192B build.
  Individually inspected Florida frame150 screenshot: a clearly recognizable
  shark crosses the foreground and obscures the truck/support roots. **Not a
  visual fit pass.** Capture51.26FPS,preceding61.23;median59.97/min44.14.
  Both native water/dry cycles complete,exit0,no fatal diagnostics;12 copies
  bounded and pointers restored do not override the obscured view. Unlike the
  earlier ambiguous close-ups, this specific obstruction is visibly a shark.
  Retake at the alternate validated water site after log-only validation.
  `fit-stock13-alternate-validation`,E64E81B4,log-only: exit0,both native
  water/dry cycles complete,no fatal diagnostics. Nearest site57671680,85327872
  replaced by58458112,87293952 (at least32 units away),from1209 validated
  alternative points. No image taken; shark-free framing remains unproven
  until the required next visual retest. Settings restored and no live game.
- **13 / Garbage, usable alternate-site retest:**
  `fit-stock13-alternate-visual`,E64E81B4 build. Individually inspected
  full-size Florida frame150 yaw60/pitch-25 at alternate clear-water point:
  three visible near-side supports continuously join the truck body; no shark
  or giant foreground geometry obscures the roots. Far-side roots remain
  occluded,not an all-angle approval. Capture50.46FPS,preceding61.15;
  median59.99/min50.46. Both native water/dry cycles complete,exit0,no fatal
  diagnostics.12 copies stay within assembly bounds and pointers restore.
  User approval pending; supersedes only the prior obscured proof.

Latest candidate build: `5DB44B529726B55DD6820E71E87B8467FC9E9F3BB902BF937AB2E9CC74C5AC0B`.
Adds shared asset-role qualification: match the native/registry transformation
bank AND mode2 table slot before fitting a support. Native8003E4A8 prefers
vehicle-authored transformation overrides; those are not generic ski arms.
Obake's first run exposed an incorrectly fitted central authored rear assembly
(body bank,slot35;one mesh/four vertices). It is no longer eligible. This is
not an exclusion of Obake from the required roster audit or a vehicle-ID fix.
The earlier A879 build also contains the display-target-relative terrain
projection correction. The later 5DB44B52 build adds only an opt-in
attachment-fixture exit immediately after the final frame1050 lifecycle
sample; shipping behavior is unchanged without the test environment flag.
This avoids forcing the unrelated defeat/menu path merely to stop a completed
water audit and does not change the shared attachment fit rule.

- Obake10 `fit-stock10-oblique`,00926348: **rejected/obscured** by large
  foreground polygons. Frame150 capture40.33FPS, preceding47.07;
  median55.74/min40.33, both full water/dry cycles complete,exit0. No fatal
  diagnostics. One fitted mesh stayed bounded/restored but belongs to the
  authored rear assembly, so numerical bounds did not establish a correct
  asset role.1524 near-triangle records retained without reaching the cap.
- Obake10 `native-stock10-oblique`,same build fit-OFF: body/transform assembly
  visible without the giant obstruction in this independent run. The low,
  partly submerged presentation remains and is not introduced by the fit.
  Frame150 capture46.77FPS;median59.92/min46.77,cycles/exit0,no fatal diagnostics.
  This is not a deterministic pixel-paired run and cannot establish the
  cause of the intermittent foreground polygons. Corrected candidate visual
  verification is still required; two images used,Boogie deferred.
- Corrected `fit-stock10-asset-role-validation` (E2D1192B), log-only:
  native initial/repeat water/dry checks all complete,exit0,no fatal diagnostics.
  Obake frame150 has no cached fitted copies, preserving its authored assembly.
  Generic supports still fitted on Nina,Torque,Molo,Trio AI (8/8/12/8 meshes).
  AI observations are not individual visual approvals. No third image taken;
  corrected-candidate Obake visual proof remains required.

- **14 / Chase:** `fit-stock14-a879-oblique`, A87920E7. Individually inspected
  full-size Florida frame150 yaw60/pitch-25 at the alternate clear-water point:
  all four ski assemblies sit against the body and the visible support roots
  continuously meet the chassis; none float away. Capture52.44FPS,
  preceding59.50; median59.60/min45.13. Initial and repeat water/dry lifecycle
  checks complete, exit0, no fixture failures or native fatal diagnostics.
  Cached fitted geometry stays within the original assembly bounds and source
  pointers restore. User visual approval pending.
- **15 / Chassey:** `fit-stock15-a879-oblique`, A87920E7. Individually inspected
  full-size Florida frame150 yaw60/pitch-25 at the alternate clear-water point:
  the visible near-side front and rear supports meet the body continuously and
  the ski plates are not suspended away from it. Capture54.76FPS,
  preceding59.71; median59.98/min54.76. Initial and repeat water/dry lifecycle
  checks complete, exit0, no fixture failures or native fatal diagnostics.
  User visual approval pending.
- **16 / Padre:** `fit-stock16-a879-oblique`, A87920E7. Individually inspected
  full-size Florida frame150 yaw60/pitch-25 at the alternate clear-water point:
  the visible front and rear struts terminate directly at the body with no
  detached gap. Capture52.09FPS, preceding53.19; median54.92/min46.24.
  Initial and repeat water/dry lifecycle checks complete, exit0, no fixture
  failures or native fatal diagnostics. Eight fitted meshes remain within the
  original assembly bounds and all source pointers restore. User visual
  approval pending.
- **17 / Dusty:** `fit-stock17-a879-oblique`, A87920E7. Individually inspected
  full-size Florida frame150 yaw60/pitch-25 at the alternate clear-water point:
  both visible near-side struts meet the wheel-well/body structure and their
  plates remain immediately beneath them. Capture42.06FPS, preceding46.62;
  median52.53/min42.06. Initial and repeat water/dry lifecycle checks complete,
  exit0, no fixture failures or native fatal diagnostics. Eight fitted meshes
  remain within the original assembly bounds and all source pointers restore.
  User visual approval pending.
- **65 / Slick Clyde:** first `fit-guest65-a879-oblique` attempt was rejected
  after the isolation guard observed another game-named PID. The clean
  `fit-guest65-a879-oblique-retry`, A87920E7, completed in one isolated process.
  Individually inspected full-size Florida frame150 yaw60/pitch-25: the visible
  front and rear support roots meet the wheel wells and both plates remain
  aligned beneath the body. Capture53.90FPS, preceding58.84;
  median58.41/min34.30. Initial and repeat water/dry lifecycle checks complete,
  exit0, no fixture failures or native fatal diagnostics. Eight fitted meshes
  stay within the original assembly bounds and all source pointers restore.
  The 34.30FPS minimum is retained as a performance concern. User visual
  approval pending.
- **66 / Sheila:** two A87920E7 attempts completed the water lifecycle but
  crashed later in the unrelated defeat/menu path with a managed stack
  overflow; the isolation diagnostic showed the game-named transient PID was
  a direct child of the crashing fixture process, not an independently launched
  test. Both are rejected as clean-exit evidence. The 5DB44B52
  `fit-guest66-5db4-oblique` run used the shared frame1050 fixture exit and
  completed in one isolated process, exit0, with no stack overflow, fixture
  failure or native fatal diagnostic. Individually inspected full-size Florida
  frame150 yaw60/pitch-25: visible front and rear struts meet the body and the
  plates remain beneath them. Eight fitted meshes remain within original
  assembly bounds and all source pointers restore. Initial and repeat
  water/dry lifecycle checks complete. Capture54.27FPS, preceding59.83;
  median59.95/min31.85, but the performance gate **fails** due to a 26.49FPS
  tail decline. Attachment fit/behavior is usable; performance and user visual
  approval remain pending.
- **67 / John Torque:** `fit-guest67-5db4-oblique`, 5DB44B52. Individually
  inspected full-size Florida frame150 yaw60/pitch-25: both visible supports
  meet the wheel wells with no floating gap and the ski plates remain aligned
  beneath them. Capture54.43FPS, preceding59.83; median59.99/min54.43.
  Initial and repeat water/dry lifecycle checks complete through the shared
  frame1050 fixture exit, exit0, no fixture failures or native fatal
  diagnostics. Eight fitted meshes remain within original assembly bounds and
  all source pointers restore. User visual approval pending.
- **68 / Dave:** `fit-guest68-5db4-oblique`, 5DB44B52. Individually inspected
  full-size Florida frame150 yaw60/pitch-25: the visible front and rear struts
  meet their wheel wells and stay aligned with the plates. Capture55.86FPS,
  preceding44.61; median59.97/min43.02. Initial and repeat water/dry lifecycle
  checks complete through the shared frame1050 fixture exit, exit0, no fixture
  failures or native fatal diagnostics. Eight fitted meshes remain within
  original assembly bounds and all source pointers restore. User visual
  approval pending.
- **69 / Convoy:** `fit-guest69-5db4-oblique`, 5DB44B52. Individually
  inspected full-size Florida frame150 yaw60/pitch-25: the exposed attachment
  arms terminate at the body/undercarriage and their plates remain aligned
  beneath the converted truck. Capture54.86FPS, preceding59.52;
  median60.00/min54.86. Initial and repeat water/dry lifecycle checks complete
  through the shared frame1050 fixture exit, exit0, no fixture failures or
  native fatal diagnostics. Eight fitted meshes remain within original
  assembly bounds and all source pointers restore. User visual approval
  pending.
- **70 / Loki:** `fit-guest70-5db4-oblique`, 5DB44B52. Individually inspected
  full-size Florida frame150 yaw60/pitch-25: the visible front and rear arms
  meet the underside/wheel-well structure and the plates remain directly
  beneath them. Capture43.25FPS, preceding50.20; median58.35/min36.06. Initial
  and repeat water/dry lifecycle checks complete through the shared frame1050
  fixture exit, exit0, no fixture failures or native fatal diagnostics. Eight
  fitted meshes remain within original assembly bounds and all source pointers
  restore. The 36.06FPS minimum is retained as a performance concern, though
  it stays above the sub-30 hard rejection threshold. User visual approval
  pending.
- **71 / Houston 3:** `fit-guest71-5db4-oblique`, 5DB44B52. Individually
  inspected full-size Florida frame150 yaw60/pitch-25: both exposed struts meet
  the body continuously and their plates remain aligned beneath the converted
  car. Capture60.85FPS, preceding59.11; median59.81/min36.72. Initial and
  repeat water/dry lifecycle checks complete through the shared frame1050
  fixture exit, exit0, no fixture failures or native fatal diagnostics. Eight
  fitted meshes remain within original assembly bounds and all source pointers
  restore. The performance gate fails because the tail declined22.55FPS from
  the head window; this remains a separate concern. User visual approval
  pending.
- **72 / Boogie:** `fit-guest72-5db4-oblique`, 5DB44B52. Individually inspected
  full-size Florida frame150 yaw60/pitch-25: the visible front and rear struts
  join the wheel-well/underside structure without a floating gap and the ski
  plates stay immediately beneath them. Capture60.12FPS, preceding59.93;
  median59.96/min36.37. Initial and repeat water/dry lifecycle checks complete
  through the shared frame1050 fixture exit, exit0, no fixture failures or
  native fatal diagnostics. Eight fitted meshes remain within original
  assembly bounds and all source pointers restore. The performance gate fails
  because the tail declined20.19FPS from the head window; this remains a
  separate concern. User visual approval pending.
- **73 / Beezwax:** `fit-guest73-5db4-oblique`, 5DB44B52. Initial and repeat
  water/dry lifecycle checks complete through the shared frame1050 fixture
  exit, exit0, no fixture failures or native fatal diagnostics. Eight fitted
  meshes remain within original assembly bounds and all source pointers
  restore. The full-size Florida frame150 yaw60/pitch-25 proof exposes the
  front and rear joints meeting the body, but the scene is badly overexposed;
  retain it as attachment evidence only, not final visual acceptance.
  Capture48.41FPS, preceding51.02; median47.15/min27.47. The sub-30 minimum and
  19.89FPS tail decline are hard performance failures for that rejected
  Florida run. Superseding proof `fit-guest73-f80c-oilfield-clear`, F80C1449,
  uses the same shared fit path on Alaska/Oilfield's valid water plane. Its
  neutral-exposure full-size frame150 yaw60/pitch-25 view clearly exposes the
  front and rear supports joining the body without a floating gap. The player
  has eight bounded/restored fitted meshes; repeated water/dry lifecycle checks
  pass through frame1050, exit0. Capture52.66FPS, preceding60.25;
  median59.97/min52.66 with0.12FPS tail decline. User visual approval pending.
- **74 / Molo:** `fit-guest74-5db4-oblique`, 5DB44B52. Individually inspected
  full-size Florida frame150 yaw60/pitch-25: the bus's visible front and rear
  struts meet the underside/wheel-well structure and their plates stay aligned
  beneath the body. Initial and repeat water/dry lifecycle checks complete
  through the shared frame1050 fixture exit, exit0, no fixture failures or
  native fatal diagnostics. Eight fitted meshes remain within original
  assembly bounds and all source pointers restore. Capture40.13FPS,
  preceding47.62; median48.40/min26.43. The sub-30 minimum and18.90FPS tail
  decline are hard performance failures. User visual approval pending.
- **75 / Sid Burn:** `fit-guest75-5db4-oblique`, 5DB44B52. Individually
  inspected full-size Florida frame150 yaw60/pitch-25: the visible front and
  rear struts meet the body continuously and the ski plates stay aligned
  beneath them. Initial and repeat water/dry lifecycle checks complete through
  the shared frame1050 fixture exit, exit0, no fixture failures or native fatal
  diagnostics. Eight fitted meshes remain within original assembly bounds and
  all source pointers restore. Capture50.10FPS, preceding55.63;
  median55.63/min30.44. The minimum remains just above the sub-30 hard cutoff,
  but a23.08FPS tail decline fails the performance gate. User visual approval
  pending.
- **1 / retail V8:2 type1 shared-rule regression:**
  `fit-stock01-5db4-oblique-regression`, 5DB44B52. Individually inspected
  full-size Florida frame150 yaw60/pitch-25: both visible attachment roots meet
  the body/wheel wells and their plates stay aligned underneath. Initial and
  repeat water/dry lifecycle checks complete through the shared frame1050
  fixture exit, exit0, no fixture failures or native fatal diagnostics. Eight
  fitted meshes remain within original assembly bounds and all source pointers
  restore. Capture54.48FPS, preceding60.02; median60.01/min54.48. This closes
  type1's inward-rule regression check; user visual approval remains pending.
- **2 / retail V8:2 type2 shared-rule regression:**
  `fit-stock02-5db4-oblique-regression`, 5DB44B52. Initial and repeat water/dry
  lifecycle checks complete through the shared frame1050 fixture exit, exit0,
  no fixture failures or native fatal diagnostics. Eight fitted meshes remain
  within original assembly bounds and all source pointers restore.
  Capture52.06FPS, preceding54.77; median59.96/min52.06. The full-size Florida
  frame150 yaw60/pitch-25 proof is technically valid, but the motorcycle-scale
  vehicle is too small in this framing for a confident final visual fit
  decision. Superseding proof `fit-stock02-f80c-close2`, F80C1449, uses the
  shared fixture-only 2-unit camera-distance control at the same yaw/pitch.
  The full motorcycle/sidecar assembly is now large enough to inspect: visible
  support roots meet its underside and the plates remain aligned beneath it.
  Its player has eight bounded/restored fitted meshes; repeated water/dry
  lifecycle checks pass through frame1050, exit0, with median/min60.00/57.39
  FPS. The original wider image remains technical evidence only. User visual
  approval pending.
- **3 / retail V8:2 type3 shared-rule regression:**
  `fit-stock03-5db4-oblique-regression`, 5DB44B52. Initial and repeat water/dry
  lifecycle checks complete through the shared frame1050 fixture exit, exit0,
  no fixture failures or native fatal diagnostics. Eight fitted meshes remain
  within original assembly bounds and all source pointers restore.
  Capture30.11FPS, preceding34.02; median44.74/min30.11. The full-size Florida
  frame150 yaw60/pitch-25 proof exposes attachment roots meeting the body, but
  severe overexposure prevents final visual acceptance. Retain it as
  attachment evidence only and obtain a clearer proof. The minimum remains
  just above the sub-30 hard rejection threshold. Superseding proof
  `fit-stock03-f80c-oilfield-clear`, F80C1449, uses the identical shared fit on
  Alaska/Oilfield's valid water plane. Its neutral-exposure full-size frame150
  yaw60/pitch-25 view exposes the front and rear supports continuously joining
  the underside. The player has eight bounded/restored fitted meshes; repeated
  water/dry lifecycle checks pass through frame1050, exit0. Capture52.34FPS,
  preceding60.06; median60.00/min52.34 with0.05FPS tail decline. The earlier
  overexposed Florida image remains rejected. User visual approval pending.
- **64 / imported Chassey Blue shared-rule regression:**
  `fit-guest64-5db4-oblique-regression`, 5DB44B52. Individually inspected
  full-size Florida frame150 yaw60/pitch-25: the visible front and rear support
  roots meet the body and their plates remain aligned beneath the imported
  car. Initial and repeat water/dry lifecycle checks complete through the
  shared frame1050 fixture exit, exit0, no fixture failures or native fatal
  diagnostics. Eight fitted meshes remain within original assembly bounds and
  all source pointers restore. Capture45.20FPS, preceding49.35;
  median49.35/min21.85. The sub-30 minimum and24.78FPS tail decline are hard
  performance failures separate from attachment fit. This supersedes the
  earlier incomplete clean-exit evidence. User visual approval pending.

- Stock0–9,11–17 and imported64–75 have usable individually inspected
  shared-fit views; stock10 has its individually inspected authored-preservation
  view. Every enumerated non-Y vehicle has complete lifecycle/geometry evidence
  and a native full-size proof suitable for user review.
- All imported types64–75 have lifecycle/geometry evidence and usable proofs.
  Garbage13 alternate-site retest is usable; prior obscured view remains
  rejected evidence.
  Obake10 now has a corrected authored-preservation view; water tint limits
  obscured-joint inspection. Every one of the30 enumerated non-Y vehicles now
  has lifecycle/geometry evidence and an individually inspected review image.
- Earlier large foreground polygons recurred in Molo's first image after
  Convoy's rejected capture. Both later usable views do not establish cause
  or resolution; keep near-camera diagnostics with further roster captures.
- Final all-roster user visual approval remains required; technical gates alone
  cannot pass the images.
- No per-vehicle authored exceptions; existing transformation settings and
  protected loose root must remain intact. Paused renderer goal stays paused.

## Reusable inspection fixture

Use `--profile attachment --attachment-trace --attachment-fit --images
--inspection-camera`, zero camera cycles. One hidden/silent game at a time.
The opt-in native chase camera uses at least196608 follow distance unless the
test-only shared `--inspection-distance-units 1..16` control is specified.
Defaults remain yaw0/pitch-45 degrees. For an exposed oblique view use
`--inspection-yaw-degrees 60 --inspection-pitch-degrees -25` (native angles
683/-284). Normal shipping camera parameters are unchanged. First-person
cycle1 was rejected. Respect at most two captures per turn.

Florida is the standard fixture map. Alaska/Oilfield slot6 is the validated
neutral-exposure alternative for reflective/fullbright subjects. Bayou slot2
has no safe separated water fixture position; Hoover Dam slot13 exposes no
valid native water bounds in this PS1 build. Neither is valid attachment-proof
evidence.

349 focused assertions pass, including bounded alternate-water-site selection,
shared bank/slot asset-role qualification,
vehicle-authored override preservation, fixture angle conversion/range checks,
narrow-chassis/outboard-bumper selection,
coherent end-face translation, mirrored 180-degree support roots,
atomic runtime mesh parsing, preserved source vertices and outboard geometry.
These checks do not substitute for native fit images or all-roster coverage.

Geometry diagnostics: `audit_water_fit_coordinates.py` reads native JSON
snapshots and the prior render's cached copies. It excludes inactive caches
outside settled water mode and includes original alternate-LOD bounds. A
stale cache from a vehicle that has returned to wheels is not evidence of an
unrestored active pointer. No VRAM capture or host UI input is used.

Add `--packet-trace` to retain frame150 vehicle/near-camera triangle ownership,
source packet addresses and view-space projections. Separate8192 record
budget avoids the generic trace's distant-geometry exhaustion. Read-only
`audit_water_near_triangles.py` clips/ranks projected area; it does not model
depth, materials or occlusion and must never be used as a visual pass.

## 2026-09-08 � shipping default restored

The user reported detached water attachments in normal gameplay. The shared fit was still gated behind environment value 1, so ordinary launches omitted it. It now runs unless explicitly disabled with value 0. The transformation proof harness also respects this shipping default rather than silently forcing it off.

Native default-only proof: `artifacts/visual-issues-attachment-default/proof.json` and adjacent runtime log/capture. Player 0 completed repeated water/dry transitions through tick 1050, clean exit and no fixture failures. The individually inspected oblique capture shows visible support roots reaching the body; far-side roots are occluded. This supplements the prior all-30 shared-rule evidence; it is not a fresh all-30 review. Metadata explicitly records removal of the old harness environment override.

## 2026-09-08 � user rejected noncanonical fitting

The user explicitly requested removal if attachment continuity came from a workaround. It did: body-facing support vertices were translated to intersect the chassis, extending the authored mesh. Removed Begin/Dispose fitting calls and scope storage from object rendering entirely; environment flags cannot reactivate it through normal rendering. Historical helper remains for analysis only. Native transformation routines remain in control. Previous default-on images prove the experiment only and are superseded as shipping acceptance. The underlying separation is unresolved; no native-equivalence claim is made.

Native unstretched validation: `artifacts/visual-issues-attachments-unstretched/proof.json`, executable 6C1B926F. Exit 0; all initial and repeated water/dry samples pass through tick 1050; no fitting log entries. Individually inspected native frame150: original support geometry, partly obscured by water. This is removal/lifecycle evidence, not proof that the underlying gap is fixed. 349 transformation assertions pass after removal.
