# V8:2 map-fidelity user review

Acceptance status: **1 pass, 5 fails, 1 provisional fail, 12 ungraded**.
Arizona Meteor Crater passes. Utah Winter Games fails because some distant
objects remain jagged and fullbright. Florida Launch Site fails because
translucent rectangular water-surface polygons render across dry drivable
terrain. Pennsylvania Steel Mill fails because a hard-edged mist band crosses
the scene and building-edge geometry clips out at close range; the latter may
be a shared cross-map defect. Minnesota Nuclear Plant fails because triangles
are missing or backface-culled and adjacent polygons receive inconsistent or
fullbright lighting. Alaska Alaskan Pipeline fails because the recurring
hard-edged translucent rectangular surface/band crosses the scene; its
mist-versus-water identity remains unconfirmed. Louisiana Ghastly Bayou is a
provisional fail pending comparison of its hard-edged mist band against
baseline PS1 gameplay. Super Dreamland 64 and the remaining 11 maps are
ungraded.

BA9BE3EC user retest update: the previously reported rectangular blue/mist
defects are fixed on Florida and Alaska, but the user did not explicitly grade
either whole map as passing, so both prior fail rows remain open. Louisiana
still exposes vehicle interiors through reflective/glass regions; the Strider
windows are the reported example. Utah, Pennsylvania, and Minnesota were not
tested. The screenshot also visibly confirms the title-bar FPS counter at
43.9 FPS. After two returns to the main menu, character selection became stuck
on Y regardless of the newly accepted carousel choice; that lifecycle defect
is tracked separately from map fidelity.

Record the user's explicit pass/fail grade one map at a time as testing occurs.
Do not infer a map pass from accepted sub-issues, prior fixes, automated proof,
or wording that does not explicitly grade the named map.

Status: **A0470F59 staged but rejected; corrective diagnosis in progress**
Staged executable: `V8_2_LOOSE/Vigilante82PC.exe`
SHA-256: `A0470F598D5A69BACC4AB1B242ED9C609B5CC4144CC11FF2B205C944E3BEC15B`
Shipping renderer: **Enhanced only**

The current 74E76336 candidate adds two shared corrections after BA9BE3EC:
original V8 material bit `0x20` is no longer mistranslated into V8:2
semitransparency, and imported character selection is cleared at the native
main-menu boundary plus whenever a retail carousel result is accepted. Focused
hidden/silent gates pass, but neither correction changes a user map grade or
counts as visual acceptance.

74E76336 user retest update: transparency remains visible on Sheila's stock
V8:2 vehicle in Louisiana (correcting the earlier Strider attribution), so the
focused converted-Strider trace/proof does not cover the separate native V8:2
reflection composition defect. Hoover Dam also shows a far-away terrain fidelity
problem in the current build: distant terrain changes to an inconsistent
representation/detail level. These are issue-level findings only. Louisiana
remains a provisional fail and Hoover Dam remains ungraded until the user gives
an explicit whole-map grade. No implementation or new test run was started
while the user is reporting results.

A0470F59 rejection update: the retained Hoover screenshot itself disproves the
earlier visual-pass claims. Sheila's body is visibly transparent/hollow in the
foreground. The distant terrain has a serrated detailed strip that terminates
at a hard boundary into a flat dark surface. Full HD-atlas hit coverage did not
establish correct geometry, transition, shade, depth, filtering, or visible
output, and the Bayou framing did not expose the failing vehicle surfaces well
enough to certify them. Exact primitive attribution is required before another
shared renderer change. The two staged two-map lifecycle orderings also reached
27.40 and 27.90 FPS; A0470F59 is rejected visually and for repeated-map
performance.

## Corrective candidate staged 2026-09-01

The earlier F7D2BB candidate globally deferred depth-tested translucent world
geometry. Although its hidden/silent technical gate passed, the retained image
coverage did not include Louisiana vehicle roofs and the user found that some
roofs became transparent. BA9BE3EC removes that global reorder: ordinary
vehicle/world translucency uses native OT order again, while only the native
weather/lighting screen-effect layer remains deferred for completed-depth
testing. A limited native post-fix Louisiana frame is retained at
`artifacts/v82-map-fidelity-current/louisiana-roof-order-fixed-20260901/louisiana-after.png`.
The exact staged build then passed Louisiana, Minnesota, and Alaska in one
hidden/silent Enhanced process at 34.43 FPS or better in every measured window.
It did not exercise all reflective/glass materials and the user's Strider test
overrules any broader inference from it. This technical result does not change
any user grade.

The same build adds an actual presentation FPS reading to the visible Windows
title bar, updated once per second.

All six reported failures remain failed/provisional until the user retests this
candidate. Two shared renderer contracts were corrected without recognizing an
arena, object, texture, cell, packet address, or authored asset:

1. Native V8:2 full-display weather/lighting scanlines remain deferred for
   correct transparency ordering, but now retain their original OT-derived
   depth test against the completed world. This prevents all of the scanlines
   from flattening into one hard rectangular overlay in Louisiana, Florida,
   Pennsylvania, and Alaska while preserving scene-authored haze/water color.
2. Precise reconstructed NCLIP is now a monotonic extension of the game's
   packed NCLIP result: a face admitted by either native packed NCLIP or the
   precise test is retained, and a face is culled only when both reject it.
   This prevents reconstructed precision from deleting native-admitted scenery
   faces, which exposed brighter underlying triangles and caused close building
   edges or parts of Minnesota/Utah scenery to disappear.

The final image-free Utah -> Pennsylvania -> Alaska run is recorded at
`artifacts/v82-map-fidelity-current/shared-geometry-effects-final-f7d2-20260901`.
It passed one hidden/silent Enhanced process, exact HD loading-card and terrain
selection, scripted traversal and weapon use, native return lifecycle, zero
repair-pool drops, zero content-specific renderer branches, and clean exit.
Measured 60-frame windows were 59.14-60.00 FPS on Utah, 48.56-55.96 FPS on
Pennsylvania, and 44.18-58.16 FPS on Alaska. These technical checks do not
change any user map grade.

## Review this correction candidate

Review these four previously rejected visual contracts in the staged build:

1. Loading, menu, and HUD text must consistently use the full-file 4x atlas
   for the retail `.FNT` that supplied it. `GAME`, `HUD`, `KONG`, and `SLOGAN`
   have sidecars; any future font without one must remain exactly native.
2. On Utah Winter Games, distant snow must remain textured without a hard
   per-tile switch into a flat bright/white band while driving.
3. Stock V8:2 wheels must not expose square interior/back-face polygons during
   motion or close camera angles.
4. Route 66 roadside buildings must not clip away when they pass close beside
   either camera edge.

F881 passed the Route 66/Olympic/Bayou/Florida sequence twice for 720 scripted
gameplay frames per map. Both exact-build runs retained every HD loading card,
terrain atlas, loading glyph, route, effect, lifecycle, and no-content-branch
contract. Their worst 60-frame windows were 44.74 and 40.24 FPS, above the
30 FPS rejection floor. The former fourth-match stale-overlay crash is fixed.

`notes/v82_map_fidelity_visual_proofs.md` now contains two exact-F881 images:
one deterministic `Quest/Objectives` loading proof and one Olympic gameplay
proof covering the terrain-distance, wheel, and view-edge geometry failures.
No contact sheet or additional capture set was produced.

The compact packet audit at
`artifacts/v82-map-fidelity-current/f881-review-packet-audit-20260831/acceptance.json`
verifies 19 contiguous/unique arena rows, 19 unique original references, 19
existing unique local proofs, no missing evidence or inspection prompt, and
zero arena-identity hits in current shipping renderer/compositor source.

## Review failure reported 2026-08-27

The A402DD candidate is not accepted. HD loading artwork is active, but some
letters are visibly blurry among otherwise sharp glyphs. Distant terrain still
cuts hard along terrain-cell boundaries into a lighter representation and
exhibits substantial pop-in. Tires looked improved but remain unaccepted until
they are revalidated in a build that does not regress font or terrain.

The next candidate requires verbose, image-free per-glyph and per-terrain-cell
provenance logging; aggregate texture-hit and route counters cannot be used as
a visual-pass proxy.

## Review failure reported 2026-08-26

The B6A123 candidate is not accepted. The user reported four release-blocking
shared defects:

1. Loading-screen text still uses native-resolution glyphs instead of the
   upscaled font replacements.
2. Terrain changes abruptly along tile boundaries between detailed nearby
   snow and flat white distance terrain.
3. Close camera placement exposes square/flat wheel polygons.
4. Buildings clip or disappear when the camera is beside them at close range.

These invalidate the prior aggregate visual-pass language. Corrections must be
shared font, terrain/material, geometry/near-clip, or visibility contracts;
map- and vehicle-specific runtime exceptions remain prohibited.

## What this review accepts

For each map, compare the retained current proof with the cited original-game
frame and, where useful, drive the staged build. Review these specific points:

1. The authored palette, sky, fog/haze, water/weather, and local lighting read
   like the original rather than a washed-out or globally tinted substitute.
2. Terrain stays textured to every visible edge with no jagged per-tile detail
   cutoff, white fallback, hole, or view-edge clipping.
3. Roads, scenery, props, vehicles, effects, and HUD compose correctly; nearby
   geometry occludes objects that are behind or below it.
4. The HD loading card and terrain replacement are visibly active.
5. Map transitions do not leave remnants from the previous loading card or
   atmosphere.

Technical map coverage is complete at the 19-map B6A baseline: one hidden
Enhanced process exercised every arena with every HD card/terrain atlas active,
traversal and weapon effects, native defeat/return, zero stale-fog samples,
zero repair-pool drops, no map-specific renderer branch, and clean exit. The
subsequent shared visual corrections are covered by the reduced representative
set, and the F881 pacing/overlay corrections were then covered twice by the
four-map sequence above. This review is the remaining subjective acceptance
gate; it is not claimed that F881 itself reran all 19 maps after the user's
explicit reduction of the repeated smoke set.

## Map checklist

| Slot | Map | Current proof | Original reference | What to inspect |
|---:|---|---|---|---|
| 0 | Arizona Meteor Crater | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/arizona-final-b6a123-visual/proof/arizona-final-b6a123.png) | [PS1 at 02:52](https://www.youtube.com/watch?v=UloIG8RtWoA&t=172s) | Warm pale sky, brown canyon walls, dark road, yellow markings, distance haze, continuous terrain. |
| 1 | Utah Winter Games | [current F881 proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/olympic-f881-visual-20260831/olympic-f881-current.png) | [PS1 at 02:29](https://www.youtube.com/watch?v=dAdRV-fYTnA&t=149s) | Blue-grey snow, trees, pale sky, mountain relief, no white fallback; also inspect the exposed stock wheels and nearby lodge at the view edge. |
| 2 | Louisiana Ghastly Bayou | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/review-proofs/bayou-current-enhanced.png) | [PS1 at 01:21](https://www.youtube.com/watch?v=awgSqlSw8b4&t=81s) | **PROVISIONAL FAIL:** dark humid palette, water visibility/behavior, trees, and terrain-water boundaries still require review; current 74E76336 user retest also shows transparency on some vehicles, so the shared vehicle material/composition defect remains open. |
| 3 | Florida Launch Site — **FAIL** | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/review-proofs/florida-final-enhanced.png) | [PS1 at 02:05](https://www.youtube.com/watch?v=PlaHZTzbBnA&t=125s) | User found translucent rectangular blue water-surface polygons across dry drivable terrain on staged C38C76D3. Investigation is held until map testing ends; eventual correction must be shared, not Florida/cell/address/authored-content-specific. |
| 4 | Pennsylvania Steel Mill — **FAIL** | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/steelmill-visual-current/proof/steelmill-current-enhanced.png) | [PS1 exterior at 00:37](https://www.youtube.com/watch?v=DScJreRo-uU&t=37s) | User found a broad hard-edged mist band plus close-range building-edge geometry clipping on staged C38C76D3. Treat clipping as potentially cross-map. Investigation is held until map testing ends; eventual correction must repair shared atmosphere and visibility contracts, not add Pennsylvania/building/address/authored-content exceptions. |
| 5 | Minnesota Nuclear Plant — **FAIL** | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/nuclear-visual-current/proof/nuclear-current-enhanced.png) | [PS1 at 01:15](https://www.youtube.com/watch?v=WXnIsHw3--M&t=75s) | User found missing or backface-culled building triangles plus anomalously bright adjacent polygons on staged C38C76D3. Investigation is held until map testing ends; eventual correction must repair shared geometry-conversion/winding/culling and lighting/material contracts, not add Minnesota/building/polygon/address/authored-content exceptions. |
| 6 | Alaska Alaskan Pipeline — **FAIL** | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/alaska-visual-current/proof/alaska-current-enhanced.png) | [PS1 at 00:25](https://www.youtube.com/watch?v=BWe5Lpj3CD4&t=25s) | User found the recurring hard-edged translucent rectangular surface/band on staged C38C76D3. Its identity as water geometry versus an atmospheric/mist plane remains unconfirmed. Investigation is held until map testing ends; eventual correction must repair shared surface classification/material/depth/atmosphere contracts, not add Alaska/map/cell/address/authored-content exceptions. |
| 7 | California Pacific Harbor | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/harbor-visual-current/proof/harbor-current-enhanced.png) | [PS1 at 00:25](https://www.youtube.com/watch?v=ebeSEbHPMog&t=25s) | Dark pavement, containers/industrial geometry, purple-brown atmosphere, intact roads and vehicle silhouette. |
| 8 | V8 Secret Base | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/secret-base-visual-current/proof/secret-base-current-enhanced.png) | [PS1 at 00:25](https://www.youtube.com/watch?v=MI6zdx42kyU&t=25s) | Pale overcast sky, dark mountain silhouettes, tan ground, subdued structures, localized bright fixtures. |
| 9 | V8 Sand Factory | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/sand-factory-visual-current/proof/sand-factory-current-enhanced.png) | [PS1 at 00:25](https://www.youtube.com/watch?v=Gol2vmOk5Qc&t=25s) | Warm dark-brown terrain, fencing/embankments, pale structures, yellow-grey horizon, storm clouds. |
| 10 | V8 Oil Fields / Oil Refinery | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/oil-fields-visual-current/proof/oil-fields-current-enhanced.png) | [PS1 at 02:05](https://www.youtube.com/watch?v=MUeLW9Sin8I&t=125s) | Dark olive ground, sparse pale geometry, isolated lights, yellow-white horizon band, purple upper sky. |
| 11 | V8 Aircraft Graveyard | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/aircraft-graveyard-visual-current-c496/aircraft-graveyard-current-enhanced.png) | [PS1 at 01:01](https://www.youtube.com/watch?v=PEY-IQUJPas&t=61s) | Tan-green terrain, pale wreckage, dark ridges, blue-grey storm cover, warm horizon/cloud light. |
| 12 | V8 Ghost Town / Wild West | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/wild-west-final-b6a123-visual/proof/wild-west-final-b6a123.png) | [PS1 at 01:00](https://www.youtube.com/watch?v=dS8gJbLQwog&t=60s) | Orange-red sunset, mesas, rail/structures, stone embankment, stable full viewport with no mosaic corruption. |
| 13 | V8 Hoover Dam | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/hoover-dam-final-b6a123-visual/proof/hoover-dam-final-b6a123.png) | [reference at 01:00](https://www.youtube.com/watch?v=GrjoFPbMnYo&t=60s) | Grey-brown storm cover, dark mountains, pale dam, blue-white towers. Missing rain is explicitly non-blocking. Current 74E76336 user retest reports inconsistent far-away terrain; whole-map grade remains unassigned pending explicit pass/fail. |
| 14 | V8 Valley Farms | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/valley-farms-final-b6a123-visual/proof/valley-farms-final-b6a123.png) | [PS1 at 01:00](https://www.youtube.com/watch?v=0EwvCSxw-PI&t=60s) | Pale sky, olive cultivated hills, tan earth cuts, planted objects correctly occluded by terrain, continuous horizon detail. |
| 15 | V8 Casino City | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/casino-city-final-b6a123-visual/proof/casino-city-final-b6a123.png) | [PS1 at 01:00](https://www.youtube.com/watch?v=51irxiOdSng&t=60s) | Black night sky, warm facades, cool lamp heads, road/verge, local light volumes; no artificial bloom requirement. |
| 16 | V8 Canyonlands | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/canyonlands-final-b6a123-visual/proof/canyonlands-final-b6a123.png) | [PS1 at 01:00](https://www.youtube.com/watch?v=3OlmTRyUlcw&t=60s) | Brown canyon faces, dark asphalt/yellow lines, pale atmosphere, grey bridgework, terrain layering. |
| 17 | V8 Ski Resort | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/ski-resort-final-b6a123-visual/proof/ski-resort-final-b6a123.png) | [PS1 at 01:00](https://www.youtube.com/watch?v=23VrlGSRksg&t=60s) | Overcast sky, white/blue-grey snow, dense flakes, conifers, mountain/lift structures; bright distance is authored snow. |
| 18 | Super Dreamland 64 | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/super-dreamland-64-final-b6a123-visual/proof/super-dreamland-64-final-b6a123.png) | [actual N64 at 01:00](https://www.youtube.com/watch?v=W_bGdsff-QA&t=60s) | Green tiled hills, orange floral borders, pink/orange clouds, purple sky, props/terrain relief; spot-check visible lethal water and respawn. |

## Acceptance response

Record each named map's explicit pass/fail grade as it is tested. A failed map
should retain the reported palette, distance, geometry, water/weather, effect,
HUD, or transition defect. The overall goal remains active until every map has
an explicit grade and every failed map is corrected and subsequently passed.
