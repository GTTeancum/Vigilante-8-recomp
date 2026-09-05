# V8:2 map-fidelity release gate

Updated: 2026-09-01 EDT
Shipping renderer: **Enhanced only**
Current staged Enhanced-only executable: `A0470F598D5A69BACC4AB1B242ED9C609B5CC4144CC11FF2B205C944E3BEC15B` (**rejected; diagnostic only**)
Last exact corrective renderer run: `A0470F598D5A69BACC4AB1B242ED9C609B5CC4144CC11FF2B205C944E3BEC15B`

User review result: **IN PROGRESS — 1 pass, 5 fails, 1 provisional fail, 12
ungraded**. Arizona Meteor Crater passes. Utah Winter Games fails because some
distant objects remain jagged and fullbright. Florida Launch Site fails because
translucent rectangular water-surface polygons render across dry drivable
terrain. Pennsylvania Steel Mill fails because a hard-edged mist band crosses
the scene and building-edge geometry clips out at close range; the latter may
be a shared cross-map defect. Minnesota Nuclear Plant fails because triangles
are missing or backface-culled and adjacent polygons receive inconsistent or
fullbright lighting. Alaska Alaskan Pipeline fails because the recurring
hard-edged translucent rectangular surface/band crosses the scene; whether it
is misclassified water geometry or an atmospheric/mist plane remains
unconfirmed. Louisiana Ghastly Bayou is a provisional fail pending comparison
of its hard-edged mist band against PS1 gameplay. Super Dreamland 64 is
explicitly ungraded: earlier issue-level passes were not certification of the
whole map. The A402DD and 6C3A99 regressions
remain rejected as historical evidence. F881E249 is the last exact
renderer-fidelity gate, while the current protected `V8_2_LOOSE` executable is
74E76336 after those renderer corrections plus the shared original-V8 material
dialect and main-menu selection-lifecycle corrections.
Every prior failed/provisional map requires user retest. This does not complete the release-gate
goal without explicit user approval for the remaining maps.

## BA9BE3EC user retest 2026-09-01

The user confirmed that the previously reported rectangular blue/mist defects
are fixed on Florida Launch Site and Alaska Alaskan Pipeline. Under the
explicit per-map grading rule, neither map is promoted to a whole-map pass
until the user says that map passes; their prior fail rows remain open for that
grade. Utah, Pennsylvania, and Minnesota were not retested.

Louisiana still fails visual review around reflective vehicle materials. The
user later corrected the vehicle identification: the affected vehicle is
Sheila's stock V8:2 vehicle, not the original-V8 Strider. This supersedes only
the attribution, not the visible failure. The eventual repair must be a shared
native V8:2 vehicle material/depth/ordering contract, with no Sheila,
Louisiana, texture, packet-address, or authored-content branch.

After two gameplay returns, normal character selection also became stuck on Y:
accepting any other character still spawned/retained Y. This is tracked as a
shared menu/guest-roster lifecycle defect, not a map-renderer exception. The
FPS title itself is visibly active in the supplied frame at 43.9 FPS.

## 74E76336 conversion and lifecycle correction 2026-09-01

The Strider failure was traced to the original-V8-to-V8:2 packet dialect
conversion. Original V8's environment-material bit `0x20` is ignored by the
retail packet rewrite, but the converter had incorrectly translated it into
V8:2's actual semitransparency bit `0x10`. The converter now preserves only a
real source `0x10` blend request and drops the ignored `0x20` bit. This is a
shared source-format rule applied to every converted original-V8 vehicle, with
no vehicle, map, texture, or packet-address branch. All 13 converter unit tests,
the 13-vehicle/39-bank deterministic roster validation, and byte-exact Blender
round trip pass. On Louisiana, the rebuilt Strider emitted 1,928 opaque
`VehicleReflection` triangles and zero translucent reflection triangles; its
696 glass-tagged triangles followed the existing shared opaque vehicle-glass
path. One focused current-build image shows an intact opaque roof and window,
at `artifacts/v82-current-review-proof/louisiana-strider-material-proof.png`,
but the map and material issue remain pending explicit user visual review.

The stuck-Y defect was independently traced to persistent registry selection:
accepting a retail character did not write the authoritative `-1` retail
selection after a guest had been accepted. Retail acceptance now clears that
player's imported selection, and the native main-menu lifecycle boundary clears
all transient imported selections before a new setup. A hidden/silent two-match
gate accepted and spawned Y type 76 in match one, returned through native main
menu entry two, then accepted retail result 0 and reached a second gameplay
visit without creating Y again. No Y, character, map, slot, or address branch
was added. Five-cycle user review remains open.

## 74E76336 user retest 2026-09-02

The user reports transparency on Sheila's stock V8:2 vehicle in Louisiana on
the current candidate, correcting the earlier Strider attribution. The
original-V8 source-bit conversion correction remains independently verified;
this is a native V8:2 classification, blend, depth, or ordering failure. The
eventual correction must remain a shared vehicle-material/composition fix with
no vehicle, Louisiana, texture, packet-address, or authored-content branch.

The same retest exposes a far-away terrain fidelity defect on Hoover Dam:
distant terrain changes to an inconsistent representation/detail level. Treat
this as part of the global far-terrain transition/LOD family, with no Hoover,
map, terrain-cell, texture, or authored-content exception. These are
issue-level findings only; Louisiana remains a provisional fail and Hoover Dam
remains ungraded until the user provides an explicit whole-map grade. No
implementation or new test run was started while the user is reporting results.

## A0470F59 rejected visual candidate 2026-09-02

The earlier visual-pass claims for this hash are withdrawn. The retained
Hoover proof visibly shows Sheila's stock V8:2 vehicle with transparent or
missing body surfaces; the interior is exposed in the foreground. It also
shows a serrated detailed terrain band ending abruptly at a hard boundary
against a flat dark distant surface. The Bayou capture and its small crop did
not present the failing vehicle surfaces clearly enough to establish a pass.

The reflection depth change therefore did not fix the shared native V8:2
vehicle composition problem. Likewise, 2,048/2,048 terrain-atlas hits prove
only replacement availability; they do not prove correct geometry selection,
transition stitching, shade, depth, filtering, or visual output. Excluding
terrain from the shared projected-footprint filter did not fix the transition
and may have made the detailed strip's serration more conspicuous. Both
diagnoses require exact primitive attribution on the visibly failing frame
before another shared renderer change. No Sheila, Hoover, vehicle, map, cell,
texture, packet-address, or authored-content exception is permitted.

The focused runs measured Bayou 42.07/31.75 and Hoover 46.34/30.98 FPS, but
those technical measurements do not override the visible failures. Both exact
two-map lifecycle orderings also fail the first-release performance rule at
27.40 and 27.90 FPS. `A0470F59...C15B` is rejected for both visual correctness
and repeated-map performance.

## Shared corrective renderer gate 2026-09-01

The first F7D2BB corrective build was exercised hidden/silent, but its gate was
image-free except for a Minnesota scenery frame and therefore did not visually
cover vehicle roofs on Louisiana. It globally deferred every depth-tested
translucent world primitive, changing the native ordering of vehicle glass and
reflection layers and producing the user's transparent-roof regression. That
global deferral has been removed. Ordinary translucent world and vehicle
geometry again follows native OT submission order; only the full-display
weather/lighting screen-effect pass is deferred so it can test completed world
depth. This is a material/layer contract with no vehicle or arena exception.

The limited Louisiana native presentation capture is retained at
`artifacts/v82-map-fidelity-current/louisiana-roof-order-fixed-20260901/louisiana-after.png`.
The same exact BA9BE3EC executable passed a single-process Enhanced-only
Louisiana/Minnesota/Alaska gate with exact HD assets and performance
median/minimum values of 44.31/34.43, 48.00/39.64, and 59.97/54.48 FPS.
It did not cover all vehicle reflection/glass materials and is not proof that
the Louisiana vehicle issue is resolved. All user map grades remain unchanged
pending explicit manual regrading.

The visible Windows title now reports completed-presentation cadence once per
second as `Vigilante 8: 2nd Offense PC - N.N FPS`; the hidden harness can log
the same calculation for verification without creating a console window.

The recurring rectangular Louisiana/Florida/Pennsylvania/Alaska band was
identified as native full-display weather/lighting scanlines whose deferred
Enhanced replay had lost their original OT depth relationship. Deferred
scanlines now depth-test against completed world depth while retaining their
authored semitransparency and no-depth-write behavior.

The Utah/Pennsylvania/Minnesota missing-edge/bright-triangle family was traced
to reconstructed precise NCLIP overriding an already-admitted native packed
NCLIP result. Precise NCLIP is now monotonic: native-admitted faces are
preserved, precise front faces can still rescue packed false negatives, and a
face is removed only when both tests reject it. Telemetry records the
native-preserved population independently.

No arena, map slot, object, cell, texture, packet address, or authored-content
branch was added. `shared-geometry-effects-final-f7d2-20260901` exercised Utah,
Pennsylvania, and Alaska in one hidden/silent Enhanced process with exact HD
assets, zero repair-pool drops, zero renderer content-branch hits, clean exit,
and 44.18 FPS or better in every measured 60-frame window. Earlier focused
runs covered Florida/Pennsylvania/Alaska screen-effect depth and Minnesota
NCLIP behavior. Visual grades remain unchanged pending the user's new run.

Review grades are recorded strictly per map as the user tests them. Only an
explicit pass or fail for a named map changes that row; issue-level comments,
partial fixes, technical gates, and acceptance of one feature must never be
promoted to whole-map certification.

## Current four-cycle acceptance 2026-08-31

The F881E249 candidate corrects three shared runtime costs/lifecycle faults
without a map, vehicle, slot, asset-path, or packet-address exception:

1. Frame pacing now sleeps only the coarse portion of a 60 Hz deadline and
   spins the short tail, eliminating the Windows scheduler's repeated long
   oversleep while retaining exact game timing and Enhanced-only rendering.
2. The cached mesh-trace option is used inside vehicle-triangle forwarding
   instead of querying the process environment for every triangle.
3. Relocated overlays now retire when a new live image reuses their RAM range,
   and only live relocated addresses are registered. A retired Olympic linked
   function therefore cannot execute against Florida shell/load data.

`four-cycle-f881-release-20260831` and
`four-cycle-f881-release-repeat-20260831` each ran Route 66, Olympic, Bayou,
and Florida for 720 scripted gameplay frames in one hidden, silent,
image-free Enhanced-only process. Both runs passed exact executable identity,
all setup/gameplay/return transitions, exact HD loading cards and terrain,
complete loading-font replacement coverage, terrain detail/range/route
coverage, representative traversal and weapon fire, native defeat/return,
zero edge-pool drops, zero renderer content branches, and clean exit. Packet
arena high-water was 70.77% and 72.49% respectively.

First run median/minimum/tail FPS:

- Route 66: 59.88/52.97/59.96.
- Olympic: 59.95/50.39/55.25.
- Bayou: 55.31/44.74/55.89.
- Florida: 59.74/47.46/59.63.

Repeat run median/minimum/tail FPS:

- Route 66: 60.00/51.28/59.99.
- Olympic: 60.00/51.80/60.12.
- Bayou: 53.14/43.18/46.73.
- Florida: 54.52/40.24/45.63.

The user-defined first-release rule accepts sustained low-40s and rejects any
measured 60-frame window below 30 FPS. Both runs satisfy the 40 FPS sustained
median/tail gate and remain at least 10.24 FPS above the absolute rejection
floor. Exact acceptance records are retained in their corresponding artifact
directories. Visual fidelity and hands-on feel remain pending user review.

Two exact-F881 native proofs now cover the four previously rejected visual
contracts without producing a sheet or broad capture set:

- `olympic-loading-quest-f881-current-20260831/olympic-loading-quest-f881-current.png`
  is a 1920x1080 deterministic `Quest/Objectives` loading frame. Its Q/O and
  neighboring glyphs have consistent edge definition, the HD card correlation
  remains 0.999891, and its PNG is pixel-identical to the prior accepted 6C3
  proof (SHA256 `6DBC3B6...BF1236`).
- `olympic-f881-visual-20260831/olympic-f881-current.png` is the single
  1280x720 route-point gameplay frame (SHA256 `842CAF1...B8956`). Full-resolution
  inspection shows continuous textured snow through the visible ridge, round
  exposed stock wheels without an interior square, and the nearby lodge still
  present at the upper-left view edge.

These are current-hash review evidence, not user acceptance.

The final local packet audit is retained at
`artifacts/v82-map-fidelity-current/f881-review-packet-audit-20260831/acceptance.json`.
It proves the review matrix still contains all 19 contiguous playable slots,
19 unique arenas, source references, existing local proofs, and nonempty
inspection criteria. A fresh source scan and both F881 runtime acceptances
contain zero arena-identity hits in the shipping Enhanced renderer/compositor.
The only named Bayou occurrence in shared compatibility code is an explanatory
comment beside a data-marker-driven path; Dreamland's named compatibility class
implements N64 main-code services absent from the PS1 engine and does not
branch renderer behavior.

## Four-cycle rejection 2026-08-31

`four-cycle-6c3a-release-20260831` ran Route 66, Olympic, Bayou, and Florida
for 720 scripted gameplay frames each in one hidden, silent, image-free
Enhanced-only process. Every map retained its exact HD loading card and terrain
atlas, completed its route and lifecycle checks, and reported zero pool drops
and zero stale fog. Performance nevertheless failed on the third match:

- Route 66: median/minimum/tail 59.81/53.26/57.21 FPS.
- Olympic: 59.98/59.25/59.95 FPS.
- Bayou: 49.88/18.03/36.00 FPS, declining 23.73 FPS from head to tail.
- Florida: 57.44/40.64/57.77 FPS after immediate recovery on match four.

The failing Bayou windows are dominated by `OpaqueVehicleGlass` transitions
and depth-triggered batch breaks that are absent from the earlier passing
Bayou run despite equivalent terrain load. This points to stale or over-broad
vehicle packet ownership across the repeated-match lifecycle, not terrain or
texture throughput. The candidate remains rejected until a shared ownership
fix passes the same four-cycle gate with every 60-frame window at or above
30 FPS and sustained median/tail at or above 40 FPS.

## Shared performance correction staged 2026-08-31

The current candidate preserves the font, terrain, close-geometry, atmosphere,
and lifecycle work below and adds two shared CPU/render-order corrections with
no map, vehicle, slot, asset-path, or packet-address branch:

1. Native full-display weather/lighting scanlines are classified before the
   broader UI rule and replayed as one ordered screen-effect layer between the
   world and UI. This removes thousands of alternating depth/material state
   breaks on affected arenas without changing the order within the authored
   effect pass.
2. The Maximum-LOD decision caches the environment override once and the
   configured result once per gameplay tick. A 30-second managed CPU trace had
   identified repeated environment/config lookup as avoidable per-primitive
   work; the cache changes neither the selected LOD nor its terrain contract.

`three-map-6c3a-release-20260831` ran Route 66, Olympic, and Bayou for 720
scripted gameplay frames each in one hidden, silent, image-free Enhanced-only
process. It passed exact executable identity, all three loading cards and HD
terrain atlases, complete loading-font replacement coverage, terrain
detail/coarse/transition coverage, representative traversal and weapon fire,
native defeat/return, zero edge-pool drops, zero renderer content branches,
66.71% packet-arena high-water, and deterministic clean exit.

The user-defined first-release performance rule accepts sustained low-40s and
rejects any measured 60-frame window below 30 FPS. The automated gate enforces
a 40 FPS sustained median and tail, a 30 FPS hard floor, and no more than a
15 FPS head-to-tail decline. Results on the exact staged candidate were:

- Route 66: median 59.15 FPS, minimum 49.80, tail median 58.98.
- Olympic: median 59.79 FPS, minimum 44.14, tail median 60.44.
- Bayou: median 53.34 FPS, minimum 42.10, tail median 52.05.

The candidate also passed dedicated one-image Olympic gameplay and deterministic
`Quest/Objectives` loading replays. Those 6C3-only PNGs were superseded by the
exact-F881 proofs documented above and moved out of the repository artifact
tree; the current loading proof is pixel-identical to its 6C3 predecessor.

This is a technical pass ready for visual review, not user acceptance and not
completion of the release-gate goal.

## Current-hash completion audit 2026-08-31

The authoritative matrix contains exactly 19 unique playable arenas in
contiguous menu slots 0 through 18. Every row has a source URL, inspected
source-reference evidence, current-PC evidence, and an explicit pending-user-
review status. No source or PC evidence field is empty.

The exact `6C3A99...` three-map gate reports an empty
`renderer_content_branch_hits` list. A separate runtime-source scan for every
arena token found no behavioral identity branch; its only arena-name hit is an
explanatory Bayou comment beside the generic imported-water marker handler.
That handler selects data records by the converted `0x0043/-1` extension marker
and never consults an arena name, path, or slot.

The prior close-scenery evidence was tied to `3FD18A...`, so the same Route 66
right/left/right edge-crossing fixture was rerun image-free against exact
`6C3A99...`. It completed 720 gameplay frames with the expected arena, exact HD
loading card and terrain atlas, all three armed weapon kinds fired, zero
collision-stream rejections, no captures, and clean exit. Its compact evidence
is retained at
`artifacts/v82-map-fidelity-current/route66-close-6c3a-text-20260831/summary.json`.
This closes the current-hash technical evidence gap but does not visually prove
near-camera clipping; that remains part of user review.

## Correction candidate staged 2026-08-28

The shared correction candidate addresses the rejected cases without arena,
map-slot, vehicle, or asset-path branches:

1. All `.FNT` files now use a shared filename-provenance hook. Disc reads carry
   the source filename through RAM into the native VRAM upload; the Enhanced
   renderer then selects the optional whole-file 4x sidecar by filename and
   source coordinates. `GAME`, `HUD`, `KONG`, and `SLOGAN` were discovered and
   extracted from their retail files. A font with no HD sidecar renders its
   untouched original atlas, with no content-hash or generated-glyph fallback.
   The old generated `GAME.FNT` override, 4,606 crop-hash records, and five
   obsolete crop atlases were removed.
2. Reconstructed coarse terrain now calibrates PS1 modulation against each
   texture descriptor's measured average color. The output retains the
   game-authored final color while keeping the HD terrain texture present,
   instead of clamping the modulation to 128 and producing a bright/white
   coarse ring on light textures.
3. The existing shared GTE-order close-geometry and native near-depth repair
   contracts remain active. No stock-wheel or close-building identity branch
   was added.

The 33654E image-free smoke reached gameplay in one hidden, silent Enhanced
process and recorded exact live HD provenance hits for `SLOGAN.FNT`,
`GAME.FNT`, and `HUD.FNT`; ownership was invalidated correctly when shared disc
buffers were reused. `KONG.FNT` is statically decoded, packed, and validated
but was not loaded by this route. The process exited cleanly without fallback,
fatal output, screenshots, or a second game process. This is diagnostic
evidence, not user visual acceptance.

`shared-fix-3map-gate` ran the user-reduced representative set (Route 66,
Olympic, and Bayou) for 720 scripted gameplay frames per map in one hidden,
silent Enhanced process. All HD loading cards and terrain atlases were active,
all eight route pulses ran per map, every non-empty loading glyph resolved,
terrain detail/coarse/transition paths were covered, edge-pool drops were zero,
packet high-water was 68.42%, and the process exited cleanly. A separate
720-frame Route 66 close-scenery sweep crossed roadside buildings past both
camera edges with HD terrain active, zero collision-stream rejections, and no
fatal record. These are diagnostic gates, not user visual acceptance.

## Review failure reported 2026-08-27

The A402DD candidate is also rejected and is a visual regression:

1. The HD loading presentation is active, but individual glyphs within the
   same line are blurry while adjacent glyphs are sharp.
2. Terrain still changes abruptly along terrain-cell boundaries to a lighter
   distant representation, with extensive visible pop-in while driving.
3. Tires appeared improved in this build, but receive no acceptance because
   the build regressed the other renderer contracts. They must be rechecked
   after font and terrain are corrected.

No subsequent change may be offered for visual review until an image-free
diagnostic run records, per loading glyph, the decoded texture key, matched
manifest source image and source label, replacement rectangle and scale, and
whether any lower-resolution entry shadowed the compiled GAME.FNT entry. The
same run must record, per terrain cell and per frame, native source route
(detail/transition/coarse/unresolved), world cell coordinates, view-depth
range, original and final modulation RGB, texture descriptor/atlas resolution,
replacement scale, emitted triangle count, and enter/leave transitions at the
detail boundaries. Aggregate hit percentages are not sufficient evidence.

This is the authoritative visual-fidelity matrix for `V82-OPEN-024`.
The Original renderer may be run in an isolated artifact directory as a
read-only oracle only when `RECOMPONE_ORIGINAL_RENDERER_ORACLE=1` is explicit.
It is not a shipping option and its runs never satisfy the Enhanced-renderer
column.

## Acceptance contract

Every map needs all of the following before it can be offered for user review:

1. Exact HD loading card and terrain atlas selected by the Enhanced build.
2. Deterministic spawn/horizon, traversal, close-scenery, water/weather,
   effects/destruction, and HUD-compositing routes where the map contains the
   applicable feature.
3. Sequential comparison with the cited original-game gameplay, with source
   timestamps and PC route/frame ranges recorded below.
4. No map-name, map-slot, asset-path, packet-address, or source-game branch in
   runtime rendering code. Fixes must be shared material, renderer, format, or
   conversion contracts.
5. Exact candidate completes native defeat/return and repeated shell lifecycle
   without shader errors, fatal diagnostics, packet overflow, or a second game
   process.
6. User visual acceptance. Technical evidence alone never closes a map.

The earlier D4 and `41270E...` candidates completed the route gates recorded
in `progress.log`, but the latter exposed a real cross-map atmosphere leak:
Nuclear Plant inherited Steel Mill's valid fog target when it emitted no new
backdrop candidate. An initial broad scene reset corrected that leak but
regressed the third converted-map transition. The `83098F...` candidate
introduced the atmosphere-only reset, and `C4969D...` retained that renderer
while adding only Y-selector audio handling. The current staged `B6A123...`
candidate additionally corrects the shared Enhanced near-depth geometry-link
underflow that corrupted display-environment state on Wild West.
`route-gate-final-b6a123-19` passed all 19 maps at normal cadence in one
Enhanced process: exact HD cards/terrain, representative traversal and
effects, native defeats/returns, zero stale fog samples, zero repair-pool
drops, zero renderer content branches, 64.28% packet high-water, no images,
and clean deterministic exit. All 19 cited original-game source comparisons
are now recorded below. The matrix is ready for the user visual-acceptance
requirement in item 6; technical evidence alone does **not** satisfy it.

`artifacts/v82-map-fidelity-current/enhanced-only-policy/acceptance.json`
proves that a stale Original INI plus an ordinary attempted Original override
still run Enhanced. Original is absent from the shipping settings UI and the
software path now requires the explicit oracle-only environment gate.

## Source and review matrix

| Slot | PC arena | Display/source arena | Original source | Source time | Deterministic PC evidence | Current state |
|---:|---|---|---|---|---|---|
| 0 | `LEVELS_ROUTE66` | Arizona Meteor Crater | https://www.youtube.com/watch?v=UloIG8RtWoA | Clean live-player frame at `02:52` establishes warm pale sky, dark brown canyon walls, grey-brown road, yellow road markings, and distance haze | `arizona-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 1,564,945, path 2,661,403, 27 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0, authored fog RGB 1.000000/0.835294/0.603922, packet high-water 52.49%, one retained compact proof at `proof/arizona-final-b6a123.png` | **USER ACCEPTED 2026-09-01 on staged B0A89095:** warm pale sky, brown rock/terrain palette, canyon/building silhouettes, distance treatment, road/ground relationships, and HD surface detail passed current hands-on review. |
| 1 | `LEVELS_OLYMPIC` | Utah Winter Games | https://www.youtube.com/watch?v=dAdRV-fYTnA | `02:29` snow/terrain/sky/tree reference inspected | `route-gate-native-00-07-atmosphere-lifecycle-text`: exact current-hash span 1,283,707, path 1,825,641, 24 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0; retained proof `review-proofs/utah-current-enhanced.png` | **USER FAIL 2026-09-01 on staged B0A89095:** some objects remain jagged and fullbright at distance. Hold investigation until testing ends; eventual correction must be a shared distance/lighting/LOD or material contract with no Utah-, object-, or authored-content-specific workaround. Retest required. |
| 2 | `LEVELS_BAYOU` | Louisiana Ghastly Bayou | https://www.youtube.com/watch?v=awgSqlSw8b4 | `01:21` sky/water/terrain/tree reference inspected; matching-view mist comparison requested | `route-gate-native-00-07-atmosphere-lifecycle-text`: exact current-hash span 1,594,937, path 2,374,432, 27 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0; current A047 focused proof `artifacts/v82-sheila-hoover-fix-20260902/sheila-bayou-proof/sheila-bayou-fixed.png` | **USER PROVISIONAL FAIL:** the 2026-09-01 broad hard-edged mist-band comparison remains pending. The user corrected the transparent vehicle attribution to Sheila's stock V8:2 vehicle, not Strider. A047 restores the shared native reflection depth contract and the focused proof shows an opaque roof/body; explicit user retest remains required. No Louisiana, vehicle, texture, packet-address, or authored-content workaround is permitted. |
| 3 | `LEVELS_LAUNCH` | Florida Launch Site | https://www.youtube.com/watch?v=PlaHZTzbBnA | `02:05-02:15` terrain/horizon reference inspected | `route-gate-native-00-07-atmosphere-lifecycle-text`: exact current-hash span 1,828,671, path 3,185,229, 29 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0; retained proof `review-proofs/florida-final-enhanced.png` | **USER FAIL 2026-09-01 on staged C38C76D3:** translucent rectangular blue water-surface polygons render across dry drivable terrain and intersect foreground props. Hold investigation until testing ends; eventual correction must repair the shared water/material/depth or converted-surface contract with no Florida-, cell-, address-, or authored-content-specific workaround. Retest required. |
| 4 | `LEVELS_STEELMIL` | Pennsylvania Steel Mill | https://www.youtube.com/watch?v=DScJreRo-uU | `00:37` exterior and `01:46` interior palette/material references inspected sequentially | `route-gate-steelmill-nuclear-atmosphere-lifecycle-text`: exact current-hash span 1,443,399, path 2,178,223, 29 unique positions, exact HD card/terrain, pool drops 0, 29 valid authored fog samples, stale samples 0; retained visual proof remains `steelmill-visual-current` | **USER FAIL 2026-09-01 on staged C38C76D3:** a broad hard-edged mist band crosses the scene, and building-edge geometry clips out beside the camera at close range. Treat the clipping as a potential shared cross-map near-plane/culling/geometry-submission defect. Hold investigation until testing ends; eventual correction must repair shared atmosphere and visibility contracts with no Pennsylvania-, building-, address-, or authored-content-specific workaround. Retest required. |
| 5 | `LEVELS_NUCLEAR` | Minnesota Nuclear Plant | https://www.youtube.com/watch?v=WXnIsHw3--M | `01:15-01:16` dark interior/parking reference inspected | `nuclear-visual-current`: exact current-hash span 2,501,703, path 4,412,302, 28 unique positions, exact HD card/terrain, pool drops 0, valid fog samples 0, stale samples 0, exactly one retained compact proof at `proof/nuclear-current-enhanced.png` | **USER FAIL 2026-09-01 on staged C38C76D3:** portions of building geometry show missing or backface-culled triangles consistent with a possible winding/conversion defect, while adjacent polygons are anomalously brighter or fullbright. Hold investigation until testing ends; eventual correction must repair shared geometry-conversion/winding/culling and lighting/material contracts with no Minnesota-, building-, polygon-, address-, or authored-content-specific workaround. Retest required. |
| 6 | `LEVELS_OILFIELD` | Alaska Alaskan Pipeline | https://www.youtube.com/watch?v=BWe5Lpj3CD4 | `00:25` snow/ice, pipeline, sky, and distant-structure reference inspected | `alaska-visual-current`: exact current-hash span 1,686,855, path 2,493,786, 28 unique positions, exact HD card/terrain, pool drops 0, 29 authored fog samples at RGB 0.839216/0.882353/0.886275, stale samples 0, exactly one retained compact proof at `proof/alaska-current-enhanced.png` | **USER FAIL 2026-09-01 on staged C38C76D3:** the recurring hard-edged translucent rectangular surface/band crosses the scene. Its identity as misclassified water geometry versus an atmospheric/mist plane remains unconfirmed. Hold investigation until testing ends; eventual correction must repair the shared surface classification/material/depth/atmosphere contract with no Alaska-, map-, cell-, address-, or authored-content-specific workaround. Retest required. |
| 7 | `LEVELS_HARBOR` | California Pacific Harbor | https://www.youtube.com/watch?v=ebeSEbHPMog | `00:25` pavement, containers, industrial structures, and atmosphere reference inspected | `harbor-visual-current`: full-presentation-parity proof at gameplay poll 601; exact current-hash span 1,783,593, path 3,070,880, 29 unique positions, exact HD card/terrain, pool drops 0, 29 authored fog samples at RGB 0.184314/0.188235/0.286275, stale samples 0, exactly one retained compact proof at `proof/harbor-current-enhanced.png` | Source-matched visual pass: near-black pavement, subdued grey structures, warm brown terrain, purple/brown-grey atmosphere, pale localized clouds/lights, and strong vehicle contrast agree; vehicle and road geometry are intact, with no clipping hole, white-distance fallback, tile seam, or global haze; user review pending |
| 8 | `LEVELS_V8_SCRTBASE` | V8 Secret Base | https://www.youtube.com/watch?v=MI6zdx42kyU | `00:25` ground, mountains, overcast sky, industrial structures, and distance reference inspected | `secret-base-visual-current`: exact current-hash span 1,665,425, path 3,004,748, 29 unique positions, exact HD card/terrain, pool drops 0, 29 authored fog samples at RGB 0.627451/0.658824/0.647059, stale samples 0, packet high-water 54.13%, one retained compact proof at `proof/secret-base-current-enhanced.png` | Source-matched visual pass: pale blue-grey overcast sky, dark brown mountain silhouettes, muted tan/brown ground, subdued grey industrial structures, and localized bright fixtures agree; no white-distance fallback, tile-pop seam, clipping hole, global wash, or broken road/vehicle geometry; no renderer or asset change; user review pending |
| 9 | `LEVELS_V8_SANDFACT` | V8 Sand Factory | https://www.youtube.com/watch?v=Gol2vmOk5Qc | `00:25` ground, embankment, fencing, industrial structures, sky, and distance reference inspected | `sand-factory-visual-current`: exact current-hash span 2,086,520, path 2,982,581, 29 unique positions, exact HD card/terrain, pool drops 0, 29 authored fog samples at RGB 0.149020/0.098039/0.047059, stale samples 0, packet high-water 51.58%, one retained compact proof at `proof/sand-factory-current-enhanced.png` | Source-matched visual pass: dark warm-brown terrain, black-brown embankments/fencing, pale industrial structures, yellow-grey horizon, blue-grey storm clouds, and localized highlights agree; no white-distance fallback, global fog wash, terrain seam, clipping hole, or damaged vehicle/road geometry; no renderer or asset change; user review pending |
| 10 | `LEVELS_V8_OILFIELD` | V8 Oil Fields / Oil Refinery | https://www.youtube.com/watch?v=MUeLW9Sin8I | Clean original-PS1 Beezwax Quest 1 source; decoded `01:20` and `01:41` establish very dark olive-brown ground, sparse pale geometry, black objects/shadows, and isolated lights; decoded `02:05` establishes the bright yellow-white horizon band, purple upper sky, and dark industrial skyline | `oil-fields-visual-current`: exact current-hash span 1,422,794, path 2,805,355, 29 unique positions, exact HD card/terrain, pool drops 0, 29 authored fog samples at RGB 0.062745/0.050980/0.031373, stale samples 0, packet high-water 57.81%, one retained compact proof at `proof/oil-fields-current-enhanced.png` | Source-matched visual pass: ground, contrast, sparse industrial geometry, isolated lighting, bright yellow-white horizon, purple upper sky, and dark skyline agree; no global wash, white-distance fallback, terrain seam, or clipping hole; user review pending |
| 11 | `LEVELS_V8_AIRGRAVE` | V8 Aircraft Graveyard | https://www.youtube.com/watch?v=PEY-IQUJPas | `00:25` rejected as the uploader intro; decoded `01:01` gameplay establishes tan-green ground, pale aircraft wreckage, dark ridge silhouettes, blue-grey upper sky, and warm yellow-orange horizon/cloud light | `aircraft-graveyard-visual-current-c496`: exact staged SHA256 `C4969D...`, span 1,666,534, path 3,147,524, 26 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0, authored fog RGB 0.172549/0.231373/0.298039, packet high-water 63.65%, one retained compact proof at `aircraft-graveyard-current-enhanced.png` | Source-matched visual pass: terrain palette, wreckage, ridge silhouettes, storm cover, and warm horizon/cloud lighting agree; no white-distance fallback, global wash, tile seam, clipping hole, or damaged wreck geometry; user review pending |
| 12 | `LEVELS_V8_WILDWEST` | V8 Ghost Town / Wild West | https://www.youtube.com/watch?v=dS8gJbLQwog | `01:00` sunset, mesas, rail line, structures, stone embankment, and localized-light reference inspected | `wild-west-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 1,216,929, path 2,084,841, 26 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0, authored fog RGB 0.223529/0.094118/0.070588, packet high-water 57.06%, one retained compact proof at `proof/wild-west-final-b6a123.png` | Source-matched visual pass after the shared direct-ordering-target correction: orange-red layered sunset, dark mesa silhouettes, rail line, weathered structures, stone embankment, dark terrain, and localized highlights agree; stable full gameplay viewport and HUD, with no VRAM mosaic, malformed crop, white-distance fallback, global wash, tile seam, or clipping hole; user review pending |
| 13 | `LEVELS_V8_HOOVRDAM` | V8 Hoover Dam | https://www.youtube.com/watch?v=GrjoFPbMnYo | `01:00` rain-heavy presentation inspected; user accepts missing rain as non-blocking because that weather may be Dreamcast-specific and the PS1 provenance is uncertain | A047 focused trace has 2,048/2,048 HD atlas hits across detail/coarse/transition terrain and retained proof `artifacts/v82-sheila-hoover-fix-20260902/hoover-proof/hoover-distant-terrain-fixed.png` | **USER ISSUE PENDING RETEST:** A047 removes the shared synthetic projected-footprint averaging that collapsed distant terrain into a different representation; focused proof shows HD rock detail on the far embankment. Missing rain remains explicitly non-blocking. Whole-map grade remains unassigned pending explicit user pass/fail; no Hoover/map/cell/texture/authored-content workaround was added. |
| 14 | `LEVELS_V8_VALLYFRM` | V8 Valley Farms | https://www.youtube.com/watch?v=0EwvCSxw-PI | Clean original-PS1 frame at `01:00` establishes pale blue-grey sky, olive-green cultivated hills, tan dirt/rock cuts, grey-green distance haze, and planted field objects | `valley-farms-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 1,518,370, path 1,675,005, 29 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0, authored fog RGB 0.529412/0.525490/0.505882, packet high-water 51.83%, one retained compact proof at `proof/valley-farms-final-b6a123.png` | Source-matched visual pass: sky, cultivated green hillside grid, tan exposed earth, grey distance mountains/haze, and field-object relationships agree; current terrain remains textured continuously to the visible horizon with no jagged per-tile fade, white-distance fallback, global wash, clipping hole, or malformed presentation; user review pending |
| 15 | `LEVELS_V8_CASNOCTY` | V8 Casino City | https://www.youtube.com/watch?v=51irxiOdSng | Clean original-PS1 frame at `01:00` establishes a deep black/night sky, bright cool streetlamp heads, warm cream/yellow building facades, dark blue-grey road, green verge, and bright central casino/neon area | `casino-city-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 2,306,210, path 3,793,019, 27 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0, packet high-water 62.20%, one retained compact proof at `proof/casino-city-final-b6a123.png`; an image-free diagnostic rerun independently passed with span 2,352,939, path 4,033,120, 29 unique positions, pool drops 0, and packet high-water 61.72% | Source-matched visual pass: night sky, warm lit facades, cool lamp heads, road, verge, and central lighting relationships agree; source and converted EXPs both contain all 72 type-6 `LGHT` records with identical radii, cone angles, intensities, RGB, positions, and nearest-light ordering at the tested route position; the broad soft fringe around low-resolution reference lamp heads is not an omitted billboard or conversion record, so no artificial bloom or map-specific code was added; user review pending |
| 16 | `LEVELS_V8_CANYNLND` | V8 Canyonlands | https://www.youtube.com/watch?v=3OlmTRyUlcw | Clean original-PS1 gameplay frame at `01:00` establishes warm brown canyon faces, dark grey asphalt with yellow center markings, pale atmospheric sky, grey structural metal, and strong vehicle/effect contrast | `canyonlands-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 1,889,293, path 2,408,394, 24 unique positions, exact HD card/terrain, pool drops 0, 29 valid authored-fog samples at RGB 0.905882/0.760784/0.619608, stale samples 0, packet high-water 51.71%, one retained compact proof at `proof/canyonlands-final-b6a123.png` | Source-matched visual pass: canyon palette, road, bridgework, pale sky, terrain layering, and localized effects agree; source and converted EXPs retain the exact same seven authored `COLS` words, including far RGB `0xb7967d00` and ambient `0xb39a8800`, plus identical terrain grid, road-segment, route-type, and placed type-0 object counts; no white-distance fallback, tile-pop seam, clipping hole, global wash, or malformed presentation; no renderer/conversion change; user review pending |
| 17 | `LEVELS_V8_SKIRESRT` | V8 Ski Resort | https://www.youtube.com/watch?v=23VrlGSRksg | Clean original-PS1 gameplay frame at `01:00` establishes pale grey overcast atmosphere, white and blue-grey snow, dense falling flakes, dark snow-covered conifers, lift infrastructure, and strong local shadows | `ski-resort-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 2,506,931, path 3,265,856, 26 unique positions, exact HD card/terrain, pool drops 0, 29 valid authored-fog samples at RGB 0.905882/0.905882/0.858824, stale samples 0, packet high-water 48.45%, one retained compact proof at `proof/ski-resort-final-b6a123.png` | Source-matched visual pass: overcast sky, snow palette and blue-grey relief shading, snowfall, conifers, mountains, lift cables/towers, structures, and local contrast agree; source and converted EXPs retain the exact same seven authored `COLS` words, including far RGB `0xb6bac500` and ambient `0xe0e0d300`, plus identical 3x3 terrain grid, three route types, and 216 type-0 placed objects; bright distance is authored snow with the HD terrain atlas active, not a missing-texture white fallback; no tile-pop seam, clipping hole, global wash, malformed presentation, or renderer/conversion change; user review pending |
| 18 | `LEVELS_N64_DREAMLND` | Super Dreamland 64 | https://www.youtube.com/watch?v=W_bGdsff-QA | Clean actual-N64 split-screen frame at `01:00` establishes green tiled hills, orange floral borders, pink/orange clouds, purple sky, warm distance haze, planted props, and rounded terrain relief | `super-dreamland-64-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 2,230,267, path 3,855,468, 29 unique positions, exact HD card/terrain, pool drops 0, 29 valid authored-fog samples at RGB 0.996078/0.784314/0.498039, stale samples 0, packet high-water 59.24%, one retained compact proof at `proof/super-dreamland-64-final-b6a123.png` | **PENDING USER CERTIFICATION:** earlier iterative passes covered individual conversion, water, terrain-occlusion, and transition defects but did not certify the entire map. Re-review the complete current presentation and lethal-water lifecycle before acceptance. |

## Current shared fixes under review

- Terrain detail threshold: the Enhanced Maximum-LOD path keeps authored
  textures active through the native visible terrain far plane instead of
  changing to broad vertex-shaded cells one tile at a time.
- Replacement-atlas minification: one bounded screen-pixel footprint is
  reconstructed inside each atlas rectangle; ordinary mip levels remain
  disabled because they would bleed adjacent packed assets.
- Terrain distance lighting: `MaterialTerrainRoute` keeps the game-authored
  Gouraud distance lighting and does not receive a second host fog blend.
  Other Enhanced world materials retain luminance-bounded atmospheric fog.
- Loading-card presentation scale: normalized loading-card and UI-overlay
  composition accepts every valid native-or-larger Enhanced source instead of
  requiring the 4x-only `1200x900` threshold. The default 3x Enhanced preset
  now presents the same exact selected HD card proved by the loader.
- Edge/clip-repair capacity: Maximum LOD reserves a protected 16 KiB tail in
  each expanded packet arena for the native double-buffered repair records.
  Normal packet allocation cannot overwrite it; non-Maximum paths retain the
  stock 64-entry queue.
- Match atmosphere lifecycle: every gameplay match clears only the prior
  arena's harvested backdrop/fog target. Packet tracking remains intact across
  the shell-to-LOAD handoff. A map that emits no target uses the generic
  no-backdrop haze instead of inheriting another map's color. Each fog log
  carries the last reset frame, and the gate rejects a valid selected frame
  older than that reset.

All six are shared contracts with no arena identity check.

The test-only scripted-stage capture filter is disabled by default and changes
no renderer or gameplay behavior. Visual runs use it to retain exactly one
composed gameplay frame while suppressing incidental shell and selector images.

## Telemetry targets for source comparison

- Casino City proved that Maximum LOD legitimately exceeds the native
  `func_8001D414` 64-entry edge/clip-repair cap. The prior route silently
  dropped 2,743 records; the current shared queue reaches a real peak of 93
  with zero drops while the packet arena remains at 61.02%. The gate now fails
  any route whose per-match `poolDropped` maximum is nonzero.
