# V8:2 map-fidelity user review

Status: **33654E font-source correction staged; fresh user review required**
Staged executable: `V8_2_LOOSE/Vigilante82PC.exe`
SHA-256: `33654EC621424889B46336F14496CD9F378685965B8DCD0FD63FCB4AA8248AB7`
Shipping renderer: **Enhanced only**

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

The reduced technical gate passed Route 66, Olympic, and Bayou for 720 scripted
gameplay frames each with exact HD loading cards/terrain, complete live loading
glyph coverage, zero edge-pool drops, no map-specific renderer branch, one
Enhanced process, and clean exit. A separate Route 66 close-scenery fixture
also passed 720 gameplay frames with zero collision-stream rejections. The
goal remains active until this visual review is reported.

The two existing 1920x1080 images in `notes/v82_map_fidelity_visual_proofs.md`
document the preceding 3FD18A candidate and are not proof of the 33654E font
hook. No new screenshots were produced during the image-free provenance smoke.

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

Technical coverage is already complete: the exact candidate ran all 19 maps in
one hidden Enhanced process with every HD card/terrain atlas active, traversal
and weapon effects, native defeat/return, zero stale-fog samples, zero
repair-pool drops, no map-specific renderer branch, and clean exit. This review
is the remaining subjective visual-acceptance gate.

## Map checklist

| Slot | Map | Current proof | Original reference | What to inspect |
|---:|---|---|---|---|
| 0 | Arizona Meteor Crater | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/arizona-final-b6a123-visual/proof/arizona-final-b6a123.png) | [PS1 at 02:52](https://www.youtube.com/watch?v=UloIG8RtWoA&t=172s) | Warm pale sky, brown canyon walls, dark road, yellow markings, distance haze, continuous terrain. |
| 1 | Utah Winter Games | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/review-proofs/utah-current-enhanced.png) | [PS1 at 02:29](https://www.youtube.com/watch?v=dAdRV-fYTnA&t=149s) | Blue-grey snow, trees, pale sky, mountain relief, no white fallback. |
| 2 | Louisiana Ghastly Bayou | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/review-proofs/bayou-current-enhanced.png) | [PS1 at 01:21](https://www.youtube.com/watch?v=awgSqlSw8b4&t=81s) | Dark humid palette, water visibility/behavior, trees, terrain-water boundaries. |
| 3 | Florida Launch Site | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/review-proofs/florida-final-enhanced.png) | [PS1 at 02:05](https://www.youtube.com/watch?v=PlaHZTzbBnA&t=125s) | Green-brown terrain, pale coastal sky, distant structures remain textured instead of foggy white, stable minification. |
| 4 | Pennsylvania Steel Mill | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/steelmill-visual-current/proof/steelmill-current-enhanced.png) | [PS1 exterior at 00:37](https://www.youtube.com/watch?v=DScJreRo-uU&t=37s) | Dark ground, red-brown brick, pale trim, blue-grey sky, mild industrial haze; also spot-check the dark interior. |
| 5 | Minnesota Nuclear Plant | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/nuclear-visual-current/proof/nuclear-current-enhanced.png) | [PS1 at 01:15](https://www.youtube.com/watch?v=WXnIsHw3--M&t=75s) | Near-black pavement, subdued concrete, crisp markings/lights, no inherited Steel Mill tint. |
| 6 | Alaska Alaskan Pipeline | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/alaska-visual-current/proof/alaska-current-enhanced.png) | [PS1 at 00:25](https://www.youtube.com/watch?v=BWe5Lpj3CD4&t=25s) | Blue-grey snow/ice, dark rock, orange pipeline, textured far cliffs rather than flat white. |
| 7 | California Pacific Harbor | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/harbor-visual-current/proof/harbor-current-enhanced.png) | [PS1 at 00:25](https://www.youtube.com/watch?v=ebeSEbHPMog&t=25s) | Dark pavement, containers/industrial geometry, purple-brown atmosphere, intact roads and vehicle silhouette. |
| 8 | V8 Secret Base | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/secret-base-visual-current/proof/secret-base-current-enhanced.png) | [PS1 at 00:25](https://www.youtube.com/watch?v=MI6zdx42kyU&t=25s) | Pale overcast sky, dark mountain silhouettes, tan ground, subdued structures, localized bright fixtures. |
| 9 | V8 Sand Factory | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/sand-factory-visual-current/proof/sand-factory-current-enhanced.png) | [PS1 at 00:25](https://www.youtube.com/watch?v=Gol2vmOk5Qc&t=25s) | Warm dark-brown terrain, fencing/embankments, pale structures, yellow-grey horizon, storm clouds. |
| 10 | V8 Oil Fields / Oil Refinery | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/oil-fields-visual-current/proof/oil-fields-current-enhanced.png) | [PS1 at 02:05](https://www.youtube.com/watch?v=MUeLW9Sin8I&t=125s) | Dark olive ground, sparse pale geometry, isolated lights, yellow-white horizon band, purple upper sky. |
| 11 | V8 Aircraft Graveyard | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/aircraft-graveyard-visual-current-c496/aircraft-graveyard-current-enhanced.png) | [PS1 at 01:01](https://www.youtube.com/watch?v=PEY-IQUJPas&t=61s) | Tan-green terrain, pale wreckage, dark ridges, blue-grey storm cover, warm horizon/cloud light. |
| 12 | V8 Ghost Town / Wild West | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/wild-west-final-b6a123-visual/proof/wild-west-final-b6a123.png) | [PS1 at 01:00](https://www.youtube.com/watch?v=dS8gJbLQwog&t=60s) | Orange-red sunset, mesas, rail/structures, stone embankment, stable full viewport with no mosaic corruption. |
| 13 | V8 Hoover Dam | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/hoover-dam-final-b6a123-visual/proof/hoover-dam-final-b6a123.png) | [reference at 01:00](https://www.youtube.com/watch?v=GrjoFPbMnYo&t=60s) | Grey-brown storm cover, dark mountains, pale dam, blue-white towers. Missing rain is explicitly non-blocking. |
| 14 | V8 Valley Farms | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/valley-farms-final-b6a123-visual/proof/valley-farms-final-b6a123.png) | [PS1 at 01:00](https://www.youtube.com/watch?v=0EwvCSxw-PI&t=60s) | Pale sky, olive cultivated hills, tan earth cuts, planted objects correctly occluded by terrain, continuous horizon detail. |
| 15 | V8 Casino City | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/casino-city-final-b6a123-visual/proof/casino-city-final-b6a123.png) | [PS1 at 01:00](https://www.youtube.com/watch?v=51irxiOdSng&t=60s) | Black night sky, warm facades, cool lamp heads, road/verge, local light volumes; no artificial bloom requirement. |
| 16 | V8 Canyonlands | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/canyonlands-final-b6a123-visual/proof/canyonlands-final-b6a123.png) | [PS1 at 01:00](https://www.youtube.com/watch?v=3OlmTRyUlcw&t=60s) | Brown canyon faces, dark asphalt/yellow lines, pale atmosphere, grey bridgework, terrain layering. |
| 17 | V8 Ski Resort | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/ski-resort-final-b6a123-visual/proof/ski-resort-final-b6a123.png) | [PS1 at 01:00](https://www.youtube.com/watch?v=23VrlGSRksg&t=60s) | Overcast sky, white/blue-grey snow, dense flakes, conifers, mountain/lift structures; bright distance is authored snow. |
| 18 | Super Dreamland 64 | [proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/super-dreamland-64-final-b6a123-visual/proof/super-dreamland-64-final-b6a123.png) | [actual N64 at 01:00](https://www.youtube.com/watch?v=W_bGdsff-QA&t=60s) | Green tiled hills, orange floral borders, pink/orange clouds, purple sky, props/terrain relief; spot-check visible lethal water and respawn. |

## Acceptance response

Approve the map-fidelity gate only if the staged build and the checklist above
look acceptable as a whole. Any reported failure should name the map and the
specific palette, distance, geometry, water/weather, effect, HUD, or transition
problem; the goal remains active until that review is received.
