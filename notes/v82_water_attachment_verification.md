# Water attachment verification — current coverage

Scope: all 30 installed vehicles (stock 0–17, imported 64–75). Y76 excluded.
Roster/source hashes: `artifacts/water-attachment-fit-20260905/roster.json`.
Candidate remains opt-in, not release-approved. User visual approval is pending.

Current correction: Convoy's six-support image exposed incoherent nearest-point
targets (rear bumper versus narrow chassis). The candidate now prefers an
inward ray from the end-face center and translates that cross-section together,
falling back to the nearest body point only when the ray misses. The
Torque, Trio, Houston and imported Chassey views below predate this correction
and require regression verification.

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

Latest candidate build: `E2D1192BB44209F161EBE8FAD59B94A17691086470BA1DF9F0FAE9BD79B2E5D5`.
Adds shared asset-role qualification: match the native/registry transformation
bank AND mode2 table slot before fitting a support. Native8003E4A8 prefers
vehicle-authored transformation overrides; those are not generic ski arms.
Obake's first run exposed an incorrectly fitted central authored rear assembly
(body bank,slot35;one mesh/four vertices). It is no longer eligible. This is
not an exclusion of Obake from the required roster audit or a vehicle-ID fix.

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

- Ten vehicles have usable inspected inward-rule views (0,4,5,6,7,8,9,11,12,13); four earlier
  vehicles need inward-rule regression checks (1,2,3,64).
- Stock14–17 and imported65–75: **15 vehicles not yet individually inspected**.
  Garbage13 alternate-site retest is usable; prior obscured view remains
  rejected evidence.
  Obake10 now has a corrected authored-preservation view; water tint limits
  obscured-joint inspection. Next: Garbage13 retry then Chase14.
- Earlier large foreground polygons recurred in Molo's first image after
  Convoy's rejected capture. Both later usable views do not establish cause
  or resolution; keep near-camera diagnostics with further roster captures.
- Clean-exit confirmation for imported Chassey using corrected fixture.
- Final all-roster acceptance, default enablement/deployment and user review.
- No per-vehicle authored exceptions; existing transformation settings and
  protected loose root must remain intact. Paused renderer goal stays paused.

## Reusable inspection fixture

Use `--profile attachment --attachment-trace --attachment-fit --images
--inspection-camera`, zero camera cycles. One hidden/silent game at a time.
The opt-in native chase camera uses at least196608 follow distance. Defaults
remain yaw0/pitch-45 degrees. For an exposed oblique view use
`--inspection-yaw-degrees 60 --inspection-pitch-degrees -25` (native angles
683/-284). Normal shipping camera parameters are unchanged. First-person
cycle1 was rejected. Respect at most two captures per turn.

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
