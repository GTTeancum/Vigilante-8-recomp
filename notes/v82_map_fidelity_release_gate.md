# V8:2 map-fidelity release gate

Updated: 2026-08-28 EDT
Shipping renderer: **Enhanced only**
Current staged Enhanced-only candidate: `33654EC621424889B46336F14496CD9F378685965B8DCD0FD63FCB4AA8248AB7` (**pending user review**)
Last exact renderer-fidelity map run: `3FD18A949F3703D83929DD68C65531B4B5115CDB779ABD218D90391FF444D3E0`

User review result: **PENDING**. The A402DD regression remains rejected. The
33654E font-source correction is installed in the protected `V8_2_LOOSE` root
for a fresh visual check; technical evidence does not close the goal.

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
| 0 | `LEVELS_ROUTE66` | Arizona Meteor Crater | https://www.youtube.com/watch?v=UloIG8RtWoA | Clean live-player frame at `02:52` establishes warm pale sky, dark brown canyon walls, grey-brown road, yellow road markings, and distance haze | `arizona-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 1,564,945, path 2,661,403, 27 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0, authored fog RGB 1.000000/0.835294/0.603922, packet high-water 52.49%, one retained compact proof at `proof/arizona-final-b6a123.png` | Source-matched visual pass: current warm pale sky, brown rock/terrain palette, canyon/building silhouettes, distance treatment, and road/ground relationships agree; Enhanced clarity and HD surface detail preserve the authored composition; no white-distance fallback, global wash, tile seam, clipping hole, or malformed presentation; user review pending |
| 1 | `LEVELS_OLYMPIC` | Utah Winter Games | https://www.youtube.com/watch?v=dAdRV-fYTnA | `02:29` snow/terrain/sky/tree reference inspected | `route-gate-native-00-07-atmosphere-lifecycle-text`: exact current-hash span 1,283,707, path 1,825,641, 24 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0; retained proof `review-proofs/utah-current-enhanced.png` | Source-matched visual pass; user review pending |
| 2 | `LEVELS_BAYOU` | Louisiana Ghastly Bayou | https://www.youtube.com/watch?v=awgSqlSw8b4 | `01:21` sky/water/terrain/tree reference inspected | `route-gate-native-00-07-atmosphere-lifecycle-text`: exact current-hash span 1,594,937, path 2,374,432, 27 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0; retained proof `review-proofs/bayou-current-enhanced.png` | Source-matched visual pass; native buoyant-water behavior verified; user review pending |
| 3 | `LEVELS_LAUNCH` | Florida Launch Site | https://www.youtube.com/watch?v=PlaHZTzbBnA | `02:05-02:15` terrain/horizon reference inspected | `route-gate-native-00-07-atmosphere-lifecycle-text`: exact current-hash span 1,828,671, path 3,185,229, 29 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0; retained proof `review-proofs/florida-final-enhanced.png` | Shared minification and double-fog defects corrected; user review pending |
| 4 | `LEVELS_STEELMIL` | Pennsylvania Steel Mill | https://www.youtube.com/watch?v=DScJreRo-uU | `00:37` exterior and `01:46` interior palette/material references inspected sequentially | `route-gate-steelmill-nuclear-atmosphere-lifecycle-text`: exact current-hash span 1,443,399, path 2,178,223, 29 unique positions, exact HD card/terrain, pool drops 0, 29 valid authored fog samples, stale samples 0; retained visual proof remains `steelmill-visual-current` | Source-matched visual pass: dark ground, red-brown brick with pale structural trim, black industrial ridge, desaturated blue-grey sky, localized contrast, and mild industrial haze agree; no white fallback, terrain seam, or scene-wide wash; user review pending |
| 5 | `LEVELS_NUCLEAR` | Minnesota Nuclear Plant | https://www.youtube.com/watch?v=WXnIsHw3--M | `01:15-01:16` dark interior/parking reference inspected | `nuclear-visual-current`: exact current-hash span 2,501,703, path 4,412,302, 28 unique positions, exact HD card/terrain, pool drops 0, valid fog samples 0, stale samples 0, exactly one retained compact proof at `proof/nuclear-current-enhanced.png` | Source-matched visual pass: deep black road, subdued concrete, crisp pale markings/structure, localized lamps/signage, and high local contrast agree; no inherited Steel Mill tint, scene-wide grey/white wash, texture fallback, or clipping hole; user review pending |
| 6 | `LEVELS_OILFIELD` | Alaska Alaskan Pipeline | https://www.youtube.com/watch?v=BWe5Lpj3CD4 | `00:25` snow/ice, pipeline, sky, and distant-structure reference inspected | `alaska-visual-current`: exact current-hash span 1,686,855, path 2,493,786, 28 unique positions, exact HD card/terrain, pool drops 0, 29 authored fog samples at RGB 0.839216/0.882353/0.886275, stale samples 0, exactly one retained compact proof at `proof/alaska-current-enhanced.png` | Source-matched visual pass: blue-grey snow/ice, dark detailed rock, pale overcast sky, orange-yellow pipeline accents, and strong local contrast agree; far cliffs remain textured rather than flat white, with no tile-pop seam, clipping hole, fallback, or global wash; user review pending |
| 7 | `LEVELS_HARBOR` | California Pacific Harbor | https://www.youtube.com/watch?v=ebeSEbHPMog | `00:25` pavement, containers, industrial structures, and atmosphere reference inspected | `harbor-visual-current`: full-presentation-parity proof at gameplay poll 601; exact current-hash span 1,783,593, path 3,070,880, 29 unique positions, exact HD card/terrain, pool drops 0, 29 authored fog samples at RGB 0.184314/0.188235/0.286275, stale samples 0, exactly one retained compact proof at `proof/harbor-current-enhanced.png` | Source-matched visual pass: near-black pavement, subdued grey structures, warm brown terrain, purple/brown-grey atmosphere, pale localized clouds/lights, and strong vehicle contrast agree; vehicle and road geometry are intact, with no clipping hole, white-distance fallback, tile seam, or global haze; user review pending |
| 8 | `LEVELS_V8_SCRTBASE` | V8 Secret Base | https://www.youtube.com/watch?v=MI6zdx42kyU | `00:25` ground, mountains, overcast sky, industrial structures, and distance reference inspected | `secret-base-visual-current`: exact current-hash span 1,665,425, path 3,004,748, 29 unique positions, exact HD card/terrain, pool drops 0, 29 authored fog samples at RGB 0.627451/0.658824/0.647059, stale samples 0, packet high-water 54.13%, one retained compact proof at `proof/secret-base-current-enhanced.png` | Source-matched visual pass: pale blue-grey overcast sky, dark brown mountain silhouettes, muted tan/brown ground, subdued grey industrial structures, and localized bright fixtures agree; no white-distance fallback, tile-pop seam, clipping hole, global wash, or broken road/vehicle geometry; no renderer or asset change; user review pending |
| 9 | `LEVELS_V8_SANDFACT` | V8 Sand Factory | https://www.youtube.com/watch?v=Gol2vmOk5Qc | `00:25` ground, embankment, fencing, industrial structures, sky, and distance reference inspected | `sand-factory-visual-current`: exact current-hash span 2,086,520, path 2,982,581, 29 unique positions, exact HD card/terrain, pool drops 0, 29 authored fog samples at RGB 0.149020/0.098039/0.047059, stale samples 0, packet high-water 51.58%, one retained compact proof at `proof/sand-factory-current-enhanced.png` | Source-matched visual pass: dark warm-brown terrain, black-brown embankments/fencing, pale industrial structures, yellow-grey horizon, blue-grey storm clouds, and localized highlights agree; no white-distance fallback, global fog wash, terrain seam, clipping hole, or damaged vehicle/road geometry; no renderer or asset change; user review pending |
| 10 | `LEVELS_V8_OILFIELD` | V8 Oil Fields / Oil Refinery | https://www.youtube.com/watch?v=MUeLW9Sin8I | Clean original-PS1 Beezwax Quest 1 source; decoded `01:20` and `01:41` establish very dark olive-brown ground, sparse pale geometry, black objects/shadows, and isolated lights; decoded `02:05` establishes the bright yellow-white horizon band, purple upper sky, and dark industrial skyline | `oil-fields-visual-current`: exact current-hash span 1,422,794, path 2,805,355, 29 unique positions, exact HD card/terrain, pool drops 0, 29 authored fog samples at RGB 0.062745/0.050980/0.031373, stale samples 0, packet high-water 57.81%, one retained compact proof at `proof/oil-fields-current-enhanced.png` | Source-matched visual pass: ground, contrast, sparse industrial geometry, isolated lighting, bright yellow-white horizon, purple upper sky, and dark skyline agree; no global wash, white-distance fallback, terrain seam, or clipping hole; user review pending |
| 11 | `LEVELS_V8_AIRGRAVE` | V8 Aircraft Graveyard | https://www.youtube.com/watch?v=PEY-IQUJPas | `00:25` rejected as the uploader intro; decoded `01:01` gameplay establishes tan-green ground, pale aircraft wreckage, dark ridge silhouettes, blue-grey upper sky, and warm yellow-orange horizon/cloud light | `aircraft-graveyard-visual-current-c496`: exact staged SHA256 `C4969D...`, span 1,666,534, path 3,147,524, 26 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0, authored fog RGB 0.172549/0.231373/0.298039, packet high-water 63.65%, one retained compact proof at `aircraft-graveyard-current-enhanced.png` | Source-matched visual pass: terrain palette, wreckage, ridge silhouettes, storm cover, and warm horizon/cloud lighting agree; no white-distance fallback, global wash, tile seam, clipping hole, or damaged wreck geometry; user review pending |
| 12 | `LEVELS_V8_WILDWEST` | V8 Ghost Town / Wild West | https://www.youtube.com/watch?v=dS8gJbLQwog | `01:00` sunset, mesas, rail line, structures, stone embankment, and localized-light reference inspected | `wild-west-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 1,216,929, path 2,084,841, 26 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0, authored fog RGB 0.223529/0.094118/0.070588, packet high-water 57.06%, one retained compact proof at `proof/wild-west-final-b6a123.png` | Source-matched visual pass after the shared direct-ordering-target correction: orange-red layered sunset, dark mesa silhouettes, rail line, weathered structures, stone embankment, dark terrain, and localized highlights agree; stable full gameplay viewport and HUD, with no VRAM mosaic, malformed crop, white-distance fallback, global wash, tile seam, or clipping hole; user review pending |
| 13 | `LEVELS_V8_HOOVRDAM` | V8 Hoover Dam | https://www.youtube.com/watch?v=GrjoFPbMnYo | `01:00` rain-heavy presentation inspected; user accepts missing rain as non-blocking because that weather may be Dreamcast-specific and the PS1 provenance is uncertain | `hoover-dam-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 1,845,945, path 2,941,959, 24 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0, authored fog RGB 0.427451/0.392157/0.376471, packet high-water 49.48%, one retained compact proof at `proof/hoover-dam-final-b6a123.png` | Source audit closed with no code or asset change: coherent grey-brown storm cover, dark brown mountain silhouettes, pale concrete dam face, blue-white towers, localized weapon/explosion lighting, intact HUD/vehicle, and no white-distance fallback, tile seam, clipping hole, global wash, or malformed presentation; missing rain explicitly accepted as non-blocking; final aggregate user review remains pending |
| 14 | `LEVELS_V8_VALLYFRM` | V8 Valley Farms | https://www.youtube.com/watch?v=0EwvCSxw-PI | Clean original-PS1 frame at `01:00` establishes pale blue-grey sky, olive-green cultivated hills, tan dirt/rock cuts, grey-green distance haze, and planted field objects | `valley-farms-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 1,518,370, path 1,675,005, 29 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0, authored fog RGB 0.529412/0.525490/0.505882, packet high-water 51.83%, one retained compact proof at `proof/valley-farms-final-b6a123.png` | Source-matched visual pass: sky, cultivated green hillside grid, tan exposed earth, grey distance mountains/haze, and field-object relationships agree; current terrain remains textured continuously to the visible horizon with no jagged per-tile fade, white-distance fallback, global wash, clipping hole, or malformed presentation; user review pending |
| 15 | `LEVELS_V8_CASNOCTY` | V8 Casino City | https://www.youtube.com/watch?v=51irxiOdSng | Clean original-PS1 frame at `01:00` establishes a deep black/night sky, bright cool streetlamp heads, warm cream/yellow building facades, dark blue-grey road, green verge, and bright central casino/neon area | `casino-city-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 2,306,210, path 3,793,019, 27 unique positions, exact HD card/terrain, pool drops 0, stale fog samples 0, packet high-water 62.20%, one retained compact proof at `proof/casino-city-final-b6a123.png`; an image-free diagnostic rerun independently passed with span 2,352,939, path 4,033,120, 29 unique positions, pool drops 0, and packet high-water 61.72% | Source-matched visual pass: night sky, warm lit facades, cool lamp heads, road, verge, and central lighting relationships agree; source and converted EXPs both contain all 72 type-6 `LGHT` records with identical radii, cone angles, intensities, RGB, positions, and nearest-light ordering at the tested route position; the broad soft fringe around low-resolution reference lamp heads is not an omitted billboard or conversion record, so no artificial bloom or map-specific code was added; user review pending |
| 16 | `LEVELS_V8_CANYNLND` | V8 Canyonlands | https://www.youtube.com/watch?v=3OlmTRyUlcw | Clean original-PS1 gameplay frame at `01:00` establishes warm brown canyon faces, dark grey asphalt with yellow center markings, pale atmospheric sky, grey structural metal, and strong vehicle/effect contrast | `canyonlands-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 1,889,293, path 2,408,394, 24 unique positions, exact HD card/terrain, pool drops 0, 29 valid authored-fog samples at RGB 0.905882/0.760784/0.619608, stale samples 0, packet high-water 51.71%, one retained compact proof at `proof/canyonlands-final-b6a123.png` | Source-matched visual pass: canyon palette, road, bridgework, pale sky, terrain layering, and localized effects agree; source and converted EXPs retain the exact same seven authored `COLS` words, including far RGB `0xb7967d00` and ambient `0xb39a8800`, plus identical terrain grid, road-segment, route-type, and placed type-0 object counts; no white-distance fallback, tile-pop seam, clipping hole, global wash, or malformed presentation; no renderer/conversion change; user review pending |
| 17 | `LEVELS_V8_SKIRESRT` | V8 Ski Resort | https://www.youtube.com/watch?v=23VrlGSRksg | Clean original-PS1 gameplay frame at `01:00` establishes pale grey overcast atmosphere, white and blue-grey snow, dense falling flakes, dark snow-covered conifers, lift infrastructure, and strong local shadows | `ski-resort-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 2,506,931, path 3,265,856, 26 unique positions, exact HD card/terrain, pool drops 0, 29 valid authored-fog samples at RGB 0.905882/0.905882/0.858824, stale samples 0, packet high-water 48.45%, one retained compact proof at `proof/ski-resort-final-b6a123.png` | Source-matched visual pass: overcast sky, snow palette and blue-grey relief shading, snowfall, conifers, mountains, lift cables/towers, structures, and local contrast agree; source and converted EXPs retain the exact same seven authored `COLS` words, including far RGB `0xb6bac500` and ambient `0xe0e0d300`, plus identical 3x3 terrain grid, three route types, and 216 type-0 placed objects; bright distance is authored snow with the HD terrain atlas active, not a missing-texture white fallback; no tile-pop seam, clipping hole, global wash, malformed presentation, or renderer/conversion change; user review pending |
| 18 | `LEVELS_N64_DREAMLND` | Super Dreamland 64 | https://www.youtube.com/watch?v=W_bGdsff-QA | Clean actual-N64 split-screen frame at `01:00` establishes green tiled hills, orange floral borders, pink/orange clouds, purple sky, warm distance haze, planted props, and rounded terrain relief | `super-dreamland-64-final-b6a123-visual`: exact staged SHA256 `B6A123...`, span 2,230,267, path 3,855,468, 29 unique positions, exact HD card/terrain, pool drops 0, 29 valid authored-fog samples at RGB 0.996078/0.784314/0.498039, stale samples 0, packet high-water 59.24%, one retained compact proof at `proof/super-dreamland-64-final-b6a123.png` | Source-matched visual pass: green tiled terrain, orange floral edging, purple upper sky, pink/orange cloud bank, warm haze, mushrooms/props, hills, and local terrain shading agree; source and converted EXPs retain the exact same seven authored `COLS` words, 2x2 terrain grid, two route types, 35 road segments, 36 junctions, and identical AIMP/BSP topology; the one removed source object is exactly `DreamlandWater`, replaced by the conversion's shared `XWAT` water contract already accepted for rendering and lethal-water lifecycle; no tile-pop seam, water leak, clipping hole, global wash, malformed presentation, renderer change, or map-specific runtime exception; user review pending |

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
