# V8:2 Dreamcast renderer reverse engineering

## Scope and acceptance

This is the evidence ledger for `V82-OPEN-024`. The target is the shared
Dreamcast behavior for terrain, water, sky/background, ordinary world
objects, texture/material selection, and buildings. The implementation target
is the PC Enhanced renderer only.

The following existing PC behavior is protected:

- widescreen projection and viewport coverage;
- near-plane and viewport-edge geometry retention;
- HD texture replacement;
- the accepted terrain and vehicle transparency corrections already present
  in the working candidate.

No map, object address, model, building, texture page, CLUT, or content-name
exception may be used. Technical tests cannot close the goal; retained
full-size proofs and user visual approval are required.

## Authoritative inputs

- Archive: `Vigilante 8 - 2nd Offense v1.000 (1999)(Activision)(US)[!] - Dreamcast.zip`
- Executable: `artifacts/dreamcast-render-distance-reference/1ST_READ.BIN`
- Executable SHA-256: `06F5A6FCDC9F8E0EF486A1CE1DF7FCF61F5A756DBEF7248139291AE520E3D3E7`
- Analysis image base: `0x8C010000`
- Processor: little-endian SH-4
- Validation authority: direct executable disassembly/decompilation, bounded
  emulation of individual recovered functions, and asset-direct inspection.
  Full-system Dreamcast emulators are not part of the workflow or acceptance
  evidence.
- Florida `LAUNCH.EXP` SHA-256: `5F047EF467FB2025B51649676703303EAF279F6083B349C2065B4DD6BBBBA530`
- Louisiana `BAYOU.EXP` SHA-256: `880C49E50C821B2CB01392A45B4E9DABCA1A7576011B69702B2F0FFA9F1203E0`
- `COMMON.EXP` SHA-256: `F321FEFE77AA54234E289020E588F8B8252AD5C13777EE9DB25E33C46313AEB3`
- `VEHICLES.EXP` SHA-256: `08EFEF8DFCBE46C36D32972CBC3973B8957B647140656D2A6DB3FE75618642A9`
- `REFLECT.PVR` SHA-256: `3DD6232F9613BC730A80DCE45EB39B82C34B5FA3E95E83C27F30BB8FE972A27`

The file named `1ST_READ.unscrambled.bin` is an incorrectly descrambled
artifact and is not authoritative.

## Frame pipeline

The bounded callee graph rooted at the frame's direct render calls contains 84
functions. The major passes are:

| Address | Recovered role | Confidence |
| --- | --- | --- |
| `0x8C095C40` | Camera-relative sky/background and horizon composition | HIGH |
| `0x8C0955A0` | Terrain visible-footprint construction and traversal | HIGH |
| `0x8C0A78C0` | Ordinary world-object list traversal and material submission | HIGH |
| `0x8C0A6F40` | Shared mesh clipping and primitive emission | HIGH |
| `0x8C0E2680` | Spatial world/entity traversal | HIGH |
| `0x8C0E1F80` | Linked entity-list traversal | HIGH |
| `0x8C09B380` | Entity draw dispatcher | HIGH |
| `0x8C0D8320` | Water visibility/height dispatch | HIGH |
| `0x8C0D72A0` | First specialized water surface pass | HIGH |
| `0x8C0D75C0` | Second specialized water surface pass | HIGH |

The terrain distance/subdivision contract is recorded separately in
`notes/v82_dreamcast_terrain_distance_re.md` and remains part of this goal.

## Level data boundary

Direct comparison of the Dreamcast and PS1 Florida level archives establishes
that world authoring is not the source of their visual differences. The
following chunks are byte-identical across the two versions: `AIMP`, `BSP`,
all `BSPI`, `COLS`, all `HEAD`, `JUNC`, `PLTX`, `RSEG`, `SUNA`, `TEXT`,
`TITL`, `XTIN`, `ZMAP`, and all `ZONE` chunks. Object count, placements,
terrain records, color tables, and authored identities are the same.

The changed chunks are the platform visual payloads: `ANM`, `BIN`, `SND`,
`XBGM`, `XBMP`, `XLSC`, `XRTP`, and `XWAT`. Therefore parity work must
recover the Dreamcast draw/material behavior and consume equivalent authored
data; it must not invent a different Florida map.

## Chunk-loader routing

The main Dreamcast level loader is `0x8C0876E0`. Relevant visual routes are:

| Chunk | Loader | Role |
| --- | --- | --- |
| `XLSC` | inline at `0x8C087D20` | scene texture payloads |
| `XBGM` | `0x8C085B20` | sky/background texture and geometry setup |
| `XBMP` | `0x8C084DE0` | terrain atlas/material source |
| `XENV` | `0x8C0866C0` | environment material source |
| `XWAT` | `0x8C085D40` | water texture/material source |
| `XRTP` | `0x8C0837C0` | route texture payloads |
| `XTIN` | `0x8C085100` | terrain material descriptors and averages |

`REFLECT.PVR` is loaded separately by `0x8C0BD160` and assigned to two
reflection material descriptors. It is not a terrain-distance texture.

## Sky/background contract

Florida's Dreamcast `XBGM` begins with three big-endian values: `6`, `512`,
and `184`. These are the six-segment wrap count and the 512-by-184 backdrop
dimensions. The Dreamcast asset is exactly twice the PS1 backdrop's 256-by-92
dimensions.

`0x8C085B20` loads the backdrop and constructs half-texel-inset UV bounds:

```
u_min = 1 / (2 * width)
u_max = 1 - u_min
v_min = 1 / (2 * height)
v_max = 1 - v_min
```

It initializes two four-vector source blocks at `0x8C28FA8C` and
`0x8C2888C0`, plus a screen/horizon block at `0x8C28FA4C`.

`0x8C095C40` derives camera angles from the camera matrix at `0x8C2443A8`,
builds a rotation matrix, and transforms the two four-vector blocks at
`0x8C28FA8C` and `0x8C2888C0`. Each becomes one four-vertex PVR triangle
strip with commands `E,E,E,F`. The later screen/horizon path uses
`0x8C28FA4C`, the projection scalars at `0x8C2747C4` and `0x8C2747CC`, and a
final six-vertex wrap strip whose explicit strip ends occur after vertices
three and six. Constants used by this path include `-70.0`, `70.0`, `1/80`,
and `1/70`.

Consequently the Dreamcast sky is a camera-relative multi-quad panorama with
separate horizon coverage. It is not one fixed fullscreen sprite and must not
be reproduced by stretching XBGM over the framebuffer.

`tools/dreamcast_sky_contract.py` verifies the executable hash, loader entry,
three XBGM header destinations, PVR texture-loader call, half-texel UV
construction, all three vector blocks, renderer entry, two four-vertex strip
loops, final six-vertex loop and its divide-by-three strip breaks, material
context, texture binding, and the constants above directly against the retail
binary. This closes the static sky topology contract; translation still has to
preserve the PC renderer's accepted widescreen horizon coverage.

## Terrain texture and distance contract

`XBMP` is loaded once as the terrain atlas/descriptor source. Runtime reads of
its metadata lead to `0x8C095040`, which builds the terrain PVR headers before
the terrain walk. No depth-dependent texture *identity* or material selection
occurs in the terrain draw loop. The texture asset itself is different from an
ordinary object texture: every observed XBMP tile is a 64-by-64, 8-bpp
paletted `PAL8_TWIDDLED_MIPMAP` PVRT texture. Florida contains 44 such tiles
and Louisiana contains 40. The PVR therefore chooses continuously within each
tile's authored mip chain from its projected footprint; it does not jump to a
second low-resolution terrain map.

Dreamcast distance detail combines that ordinary hardware mip selection with
recursive 4/2/1 geometry selection, height morphing, and base/offset colour
contribution. A PC translation must preserve the per-tile texture identity
across all three geometry bands and provide a complete mip chain for the HD
replacement of that same tile. It must not substitute a tile-boundary colour,
fallback texture, or different material at distance.

This supersedes any interpretation of the bright distant region as an authored
low-resolution terrain map. The exact recursive thresholds, color equations,
and translated PC implementation are in
`notes/v82_dreamcast_terrain_distance_re.md`.

## Water contract

Dreamcast `XWAT` is a 64-by-64, 4-bpp paletted texture with an opaque palette.
Surface transparency therefore comes from PVR material state and submitted
vertex alpha, not texture alpha.

The level loader stores XWAT texture metadata at `0x8C223F8C`,
`0x8C2744D4`, and `0x8C228908`. `0x8C08A3A0` constructs two distinct material
contexts at `0x8C243900` and `0x8C2439C0`. `0x8C0D8320` compares the authored
water plane with camera height, uses exact `60.0` and `0.4` thresholds, and
dispatches both specialized CPU vertex generators when the surface is in its
visible band.

Both water materials are submitted to the translucent polygon list, disable
culling, keep the default reversed-depth `GREATER` comparison, and keep depth
writes enabled. They use conventional alpha blending:

```
source = SRCALPHA
destination = INVSRCALPHA
```

The mapping is direct from the executable's Kamui-to-PVR table at
`0x8C05923C`: Kamui value `8` maps to PVR source mode `4` (`SRCALPHA`), and
Kamui value `6` maps to PVR destination mode `5` (`INVSRCALPHA`). Context
`0x8C243900` is an untextured alpha-enabled pass. Context `0x8C2439C0` binds
XWAT and is the textured alpha-enabled pass. The textured pass uses bilinear
filtering, normal `1.00` mip bias with mipmapping disabled, unclamped and
unflipped UVs, texture alpha enabled, and `MODULATEALPHA` (`texture * vertex
ARGB`) shading. Both use table fog and neither enables colour clamp.

The packed result was verified by running the retail header compiler at
`0x8C041E60` in Ghidra's SH-4 p-code emulator after applying the exact default
initializer and water overrides. The retained output is
`artifacts/dreamcast-render-full-re/water-material-headers-emulated.txt`:

| Pass | Dirty mask | PCW | ISP | TSP |
| --- | ---: | ---: | ---: | ---: |
| untextured | `0x002004CA` | `0x828E0103` | `0x80000000` | `0x9411A440` |
| textured XWAT | `0x00AA1EC2` | `0x828E010B` | `0x80000000` | `0x941024C5` |

The TCW texture-address bits in the emulator fixture are intentionally
synthetic; the state words above and every decoded material field are from the
retail compiler. The dirty mask resides at context offset `+0x00` (not `-4`).

The Dreamcast water surface is therefore not an ordinary world-object quad or
a screen-space translucent rectangle. Its two passes, height gate, depth
behavior, vertex generation, and texture motion must be implemented together.

### PS1-to-Enhanced ownership seam

The PS1 level overlays all reach the same water dispatcher,
`func_80017EB8`. It always calls the shared base generator at `0x80015F28`
and conditionally calls the near-surface generator at `0x80016664`. Host hooks
now record the exact packet arena interval written by each call. The Enhanced
renderer therefore identifies water solely by function-owned packet
provenance; it never tests a map, address, texture page, CLUT, colour, or
screen rectangle.

A hidden Florida trace on executable
`C94AA4CBA9AF5D15477B8104F255C56F0430CBEEF361B67824D13893EC3524AD`
established the PS1 packet contracts:

- base packets are untextured, flat, semitransparent six-word triangles that
  form screen-wide horizontal gradient strips;
- surface packets are raw-textured, semitransparent seven- and nine-word
  triangles using 4-bpp page `0x000F`, CLUT `0x7F85`, animated wrapping
  `0..63` UVs, and projected per-vertex depth.

The trace contained 392 base and 120 surface scopes within the bounded sample,
with no ownership overlap or heuristic classification. It ran Enhanced-only,
hidden and silent, in one process at 59.98 median / 58.07 minimum FPS. This
exact seam is the global translation point for the recovered Dreamcast alpha,
depth, filtering, and shoreline behavior.

### Water vertex generation

`0x8C0D72A0` emits the untextured first pass as one four-vertex PVR triangle
strip. Each vertex uses the standard 32-byte Dreamcast layout: command at
`+0x00`, projected `x/y/z` at `+0x04/+0x08/+0x0C`, and ARGB at `+0x18`.
The first three commands are `0xE0000000` and the fourth is `0xF0000000`.
ARGB is exactly `0xB0RRGGBB`, with RGB read from `0x8C28EBF0..F2`; the pass
therefore supplies its own approximately 69-percent vertex alpha. The quad's
orientation follows the larger absolute camera/water-plane axis and its
trapezoid/horizon coverage is derived with the retail constants `41.666668`,
`0.5`, `37.5`, `7.8124995`, and `7.0312495`.

`0x8C0D75C0` is the textured XWAT surface, not a flat authored polygon. It
clips the visible water rectangle to the view, aligns it to 32-world-unit
cells, and caps each grid dimension at 32. A 32-byte-stride visibility field
at `0x8C2BA940` classifies grid corners against the terrain height field. A
parallel 32-float/128-byte-stride displacement field at `0x8C2BAD40` stores
the animated surface. The first wave is the product of two periodic sine
terms scaled by `0.15625`; the second is another product scaled by `0.03125`
and added to the first. Both phases are derived from the frame/tick value at
`0x8C1133EC`. The first uses magic signed division by 240 and a
`13981.014`-radian phase step per 32-unit cell, yielding a 240-tick period
and a 150-world-unit wavelength. The second uses magic signed division by
420 and a `23301.69`-radian cell step, yielding a 420-tick period and a
90-world-unit wavelength. The sine/cosine helper is `0x8C0D7260`.

Every textured vertex uses this layout:

```
+0x00 command       0xE0000000, or 0xF0000000 at a strip end
+0x04 projected x
+0x08 projected y
+0x0C projected reciprocal depth * 0.9
+0x10 u
+0x14 v
+0x18 base ARGB     0x80FFFFFF
+0x1C offset ARGB   unused by the material
```

World height is the authored water plane at `0x8C28892C` plus the generated
displacement. UVs are not planar world coordinates: each is the absolute
difference between neighboring displacement samples, multiplied by
`2.6666667`. The routine emits row triangle strips continuously; terrain and
the depth buffer conceal the submerged portion. The corner-classification
field affects the wave field at the shoreline rather than cutting the water
into independent authored rectangles. This continuous grid plus depth-tested
coverage is the behavior to reproduce for smoother land/water boundaries.

`tools/dreamcast_water_contract.py` checks the executable hash, the key SH-4
instructions, both exact time dividers, all constants above, the two scratch
grid addresses, terrain-height source, water-plane and tick sources, dispatch
height/orientation gates, material-context pointer, vertex colors, and strip
command words directly against the retail Dreamcast binary.

The PS1 V8:2 surface generator at `0x80016664` contains several matching
anchors: the same 32-cell cap, 240/420-tick phase dividers, 873/1456
turn/4096 spatial steps, parallel classification/displacement grids, and a
terrain-height lookup. Those anchors prove common lineage, but they do **not**
prove complete vertex-stream equivalence. In particular, they do not prove
identical clipping, grid-bound rounding, shoreline classification, strip
termination, projection, submission order, or depth behavior. The previous PC
decision to retain the PS1 packet mesh and translate only Dreamcast material
state was therefore incomplete and is rejected as a parity claim after the
Louisiana visual failure.

No further water rendering change may be accepted from isolated constants or
visual symptom tuning. The complete Dreamcast dispatcher at `0x8C0D8320`,
untextured pass at `0x8C0D72A0`, and textured XWAT pass at `0x8C0D75C0` must be
reconstructed instruction-by-instruction, including delay slots and every
branch-skipped literal pool. `tools/dreamcast_water_contract.py` now hashes
the entire contiguous retail intervals in addition to its semantic anchors;
the eventual host implementation must be traceable to that complete
reconstruction. `tools/ps1_water_equivalence_contract.py` remains useful only
as a shared-lineage guard, not as authorization to reuse the PS1 draw stream.

Earlier analysis briefly compared the hooks with `analysis/SLUS_005.10`,
which is the first Vigilante 8 and made `0x80016664` appear not to be a
function. The actual V8:2 executable is `SLUS_008.68`; its generated code and
binary both contain complete water functions at `0x80015F28` and
`0x80016664`, called together by `0x80017EB8`. The ownership seam is valid.

## Ordinary world objects and buildings

`0x8C0A78C0` walks the world-object list at `0x8C2B2A04`, culls each object by
its center/radius, transforms its mesh, updates the PVR header only when the
material identity changes, and emits geometry through `0x8C0A6F40`. There is
no distance-driven texture switch inside the Dreamcast material loop.

The object center and radius are stored at `+12` and `+24`; visibility is
tested through `0x8C0A6620`. A transformed-depth guard rejects objects past
`2097152.0`, but that guard skips the object—it does not replace its material.
The material identity is read through `object+8`, cached across submissions,
and recompiled through `0x8C041E60` only when that identity changes. The
binary-anchored `tools/dreamcast_world_object_contract.py` verifies those
instructions, calls, offsets, and literal values directly.

The PS1 path at `func_8002D9E0_Impl` does contain an authored whole-mesh LOD
branch:

- high mesh pointer: object `+0x40`;
- low mesh pointer: object `+0x68`;
- distance threshold: object `+0x6C`;
- measured distance: stack `+0x2C`.

With the current Enhanced `Maximum` LOD setting, the threshold is forced to
zero and Florida diagnostics recorded 1,476 observed decisions, all selecting
the high mesh. Florida's reported proximity-dependent building appearance is
therefore not caused by the retail PS1 high/low mesh branch in this
configuration. The remaining candidates are shared material identity,
animated atlas coordinates, or HD-replacement resolution at the object
material boundary.

Four large Florida objects (`0x806BF048`, `0x806BEE68`, `0x806BEF58`, and
`0x806C0000`) produced continuously changing UV rectangles and dominated
replacement misses. Their common height, large footprint, animated UVs, and
material cadence identify them as the PS1 water quadrants, not buildings.
These addresses are diagnostic evidence only and may never appear in runtime
logic.

### Asset-direct texture layout

The retained `tools/dreamcast_pvr_inventory.py` scanner inventories embedded
PVRT headers directly from the Dreamcast EXP/PVR files. It does not infer
formats from VRAM captures. Across the two representative levels and shared
archives it found:

| Source | Textures | Mipmapped | Meaning |
| --- | ---: | ---: | --- |
| Florida `LAUNCH.EXP` | 226 | 46 | 44 XBMP terrain tiles; 2 XRTP route textures |
| Louisiana `BAYOU.EXP` | 215 | 41 | 40 XBMP terrain tiles; 1 XRTP route texture |
| `COMMON.EXP` | 1,352 | 0 | shared scene/building/object textures |
| `VEHICLES.EXP` | 742 | 0 | all vehicle textures |
| `REFLECT.PVR` | 1 | 0 | environment/reflection texture |

The level-specific ordinary scene textures live in nested `FORM` chunks and
use non-mipmapped TWIDDLED, VQ, or RECTANGLE_TWIDDLED layouts. `XLSC` textures
are also non-mipmapped. Thus the Dreamcast does not improve building distance
appearance by swapping or mipmapping those materials. Any Florida building
that changes texture as the camera approaches is a PC material/atlas identity
or sampling bug, not behavior to clone from Dreamcast.

The asset inventory is retained in
`artifacts/dreamcast-render-full-re/pvr-inventory.txt` and the level parent
classification in
`artifacts/dreamcast-render-full-re/pvr-level-parent-inventory.txt`.

The Enhanced fallback sampler previously applied its synthetic derivative
footprint filter to every native-VRAM world texture. That was not a Dreamcast
material rule: the inventory above proves the building/object images have no
mip chain at all. The translated sampler now permits that fallback footprint
only for function-owned `TerrainRoute` geometry. Ordinary scene and building
textures remain bilinear, non-mipped, and on one replacement identity at every
distance. A 1,200-tick Florida owner trace followed 137 native objects and
recorded zero same-surface source, replacement-key, hit/miss, or resolution
changes while the camera traversed the arena.

### Ordinary-object mesh clipping and submission

The material-stable walker submits each visible object through the shared
Dreamcast mesh emitter at `0x8C0A6F40`. The mesh count is read at `mesh+0x1C`;
source records begin at `mesh+0x20` and advance in 32-byte units. Vertices are
projected through `0x8C0A6CC0` using the global projection scale at
`0x8C2747CC`. The emitter classifies all three triangle vertices against the
exact single-precision near plane `0.01`, dispatches all six nontrivial
inside/outside masks through the shared edge interpolators `0x8C0A6D00` and
`0x8C0A6EC0`, preserves triangle-strip parity, and emits 32-byte PVR vertices
with `0xE0000000` for ordinary vertices and `0xF0000000` at strip termination.
Partly visible triangles are therefore rebuilt at the near plane rather than
discarded wholesale when an object crosses the camera or viewport edge.

`tools/dreamcast_world_object_contract.py` now hash-gates these mesh-layout,
projection, near-plane, clip-mask, interpolation, strip-parity, and PVR-command
anchors in addition to the walker and material identity. The corresponding
PC seam retains exact camera-space endpoints and authored UV/color data before
the Enhanced renderer performs its shared view-space clip. The focused
`MeshClipContract` passes 159 packet/provenance assertions for flat, textured,
Gouraud, and textured-Gouraud triangles, including mixed in-front/behind-camera
geometry. This connects the already accepted widescreen near-edge repair to
the directly recovered Dreamcast object-submission behavior without an object,
vehicle, map, or texture exception.

### Packed scene material state

The retail initializers and header compiler produce these stable PVR words
(texture addresses in TCW are fixture-specific):

| Material | List | Texture | Depth / cull / writes | Blend | Fog | Filter / mip bias |
| --- | --- | --- | --- | --- | --- | --- |
| terrain base | opaque | none | GREATER / none / on | ONE, ZERO | table | bilinear / 0.25 |
| terrain textured | opaque | yes | GREATER / CCW / on | ONE, ZERO | table | bilinear / 0.50 |
| ordinary world object | translucent | yes | GREATER / CW / on | SRCALPHA, INVSRCALPHA | table | bilinear / 0.25 |
| sky XBGM | opaque | yes | GREATER / CCW / on | ONE, ZERO | table | bilinear / 1.00 |

The corresponding packed PCW/ISP/TSP words are retained in
`artifacts/dreamcast-render-full-re/scene-material-headers-emulated.txt`.
Texture descriptors are bound separately at `0x8C069160`; that binding carries
the PVRT layout's mipmap bit into TCW. This is why the terrain context can use
the XBMP mip chains while the same material compiler leaves ordinary object
textures non-mipmapped.

The initializer stores KallistiOS/PVR mip-bias enumerants, not literal floating
point values: `1`, `2`, and `4` mean `0.25`, `0.50`, and `1.00`
respectively. Only the textured terrain row consumes an authored mip chain in
the audited retail assets; the ordinary-object and sky values do not enable
mipmapping on their own.

## Reflection contract

`REFLECT.PVR` is one 64-by-64 ARGB1555 TWIDDLED texture without mipmaps.
`0x8C0BD160` binds it to two reflection contexts. Retail header emulation shows
that both contexts compile to the same opaque, reversed-depth-writing,
bilinear material. The contexts differ only in their geometry and UV
generation call paths; neither changes reflection texture or sampler state by
distance. A building that appears to swap textures as the camera approaches
therefore cannot be implementing a Dreamcast reflection LOD rule.

## Exact table-fog contract

The retail fog state is flushed by `0x8C046840`. `0x8C043C80` packs density,
`0x8C043CC0` sets RGB, and `0x8C043D00` converts 129 fog-opacity samples into
the 128 adjacent 8-bit pairs consumed by the PVR fog-table registers. The sole
game-side table builder is `0x8C094680`.

The level's fog RGB is `COLS` word 1. Florida therefore uses `(255,255,254)`,
not either endpoint of the terrain color ramp. The requested density is
`0.27600002`; the retail packer produces `0x8DFE`, which the PVR interprets as
`0.275390625`. For table index `i` in `0..128`, the game computes:

```
inverseDepth = 4.4160004 / (((i & 15) + 16) << (i >> 4))
opacity = clamp((inverseDepth - 0.080000006) /
                (0.13800001 - 0.080000006), 0, 1)
table[i] = trunc(opacity * 255)
```

The terrain transform at `0x8C101DC0` submits PVR reciprocal depth as
`0.9 * projectionScale / viewDepth`; a process-local read of the retail build
confirms that the 640-pixel renderer stores a 512-pixel projection scale at
`0x8C2747CC`. The adjacent 320/240 values are the screen centre, not the
projection scale. The Enhanced implementation converts PS1 depth back to the
shared Dreamcast unit scale, evaluates the PVR index from that reciprocal
depth and the packed density, interpolates the same adjacent 8-bit opacity
samples, and applies the level-authored RGB through the shared world-material
shader. The previous smoothstep fog is
retained only as a non-V8:2 fallback. The exact mode does not classify maps,
textures, buildings, or packet addresses. The verifier
`tools/dreamcast_fog_contract.py` hash-gates the authoritative executable and
checks the retail code, constants, packed density, table values, and COLS
mappings directly.

The process-local retail oracle retained at
`artifacts/dreamcast-render-full-re/runtime-oracle/route66-projection-fog-oracle.log`
(SHA-256
`6D87DCC96C3BEF05C51B6D2A3E1F050BB8149B773A551BC32F5199B06F8197D6`)
confirms those values during actual gameplay rather than only statically:
`0x8C2747CC=512.0`, `0x8C2747D0=320.0`, and
`0x8C2747D4=240.0`. It also records live PVR fog color `0x00FFD59A`, an
exact byte-for-byte match for Route 66's `COLS` word 1 payload (mode byte
`0x07`, RGB `FF,D5,9A`). Florida's corresponding authored word is
`0xFFFFFE07`, Louisiana's is `0x01070307`, and Olympic's is
`0xDBE4ED07`. This rules out a universal white or backdrop-sampled fog color;
the shared rule is the level's authored `COLS` word 1.

Direct loader/runtime tracing establishes the PS1 mirror of the COLS fields:
terrain-ramp low/high are `gp+0xE04` and `gp+0xDAC`; the fog color is
`gp+0xDA4`. A rejected, never-deployed candidate briefly used unrelated
dynamic values at `gp+0xE00` and `gp+0xDB4`; the corrected translation retains
the loader-proven ramp and keeps the fog word separate.

## Terrain/water depth composition

Retail material-header emulation establishes that Dreamcast terrain and both
water passes use the reversed-depth `GREATER` comparison and update the depth
buffer. The first water translation enabled that state for water but exposed a
host mismatch: native V8:2 `TerrainRoute` geometry was marked depth-writing,
yet Enhanced left depth testing disabled. OpenGL ignores the depth mask while
the depth test is disabled, so land never populated the depth image that was
supposed to conceal the continuous water grid at the shoreline.

The shared translation now enables compare and write for coherent
function-owned terrain geometry and compare/write for both function-owned
water passes. Enhanced uses the equivalent conventional `LEQUAL` direction.
It does not identify a map, water texture, building, packet address, or world
coordinate, and it leaves the established widescreen projection and outer-edge
coverage code untouched. A hidden held Florida diagnostic on executable
`E7CAF87F856119CEE278860A8654A96BFE48477C08E67CAE5D54A39BD80FD956`
completed in one Enhanced-only process at 60.01 median / 57.25 minimum FPS.
The image was rejected as approval evidence because the fixture camera sat
under launch machinery and did not frame the shoreline; the technical result
is retained only as a depth-contract gate.

The current held integration candidate is
`artifacts/v82-dreamcast-full-re/fog-scale-candidate/Vigilante82PC.exe`
(SHA-256
`B587A02C8E9D99D06AE14665C00D872E8A10B9DB6199EB8DC3070841A62B240C`).
One hidden, silent, Enhanced-only process completed Florida, Louisiana, and
Hoover Dam with every functional route, HD loading card, HD terrain atlas,
fog, and terrain contract present. Florida measured 49.05 median / 28.97
minimum FPS, Louisiana 59.22 / 38.17, and Hoover Dam 32.11 / 16.45. The
candidate is therefore withheld from `V8_2_LOOSE`: it violates the release
floor even though its water no longer draws as a rectangular shoreline band.
Hoover's slow windows contain roughly 1,800--1,970 generated terrain
triangles per presented frame plus ordinary object/vehicle load and frequent
opaque/translucent state changes. Performance qualification must be repeated
after reducing shared renderer overhead without weakening the recovered
Dreamcast subdivision or widescreen contracts.

Per the user's 2026-09-07 scope decision, those FPS results are retained for
the separate performance run rather than used to judge Dreamcast visual
parity. Visual qualification for this goal requires correct scene composition
and HD replacement use; only genuinely unplayable performance blocks a proof.

Two internal 1920-by-1080 visual checks on the same held executable establish
the current baseline. Florida capture poll 601 uses the
`levels/launch` 1760-by-640 HD terrain atlas and shows water terminating at the
authored land silhouette instead of crossing dry land as a rectangular band.
Hoover Dam capture poll 601 uses the `levels/v8/hoovrdam` 1920-by-768 HD
terrain atlas and shows the recovered smooth 4/2/1 terrain transition through
the horizon without the rejected serrated detail cutoff. Both retain the
widescreen margins. They are internal checks, not final approval proofs:
Florida still needs a paired near/far view of the same building material, and
the final proof set must come from the deployed runnable-root executable.

## Widescreen panorama material batching

A full-size Florida capture on the direct-executable candidate exposed a hard
white rectangle at the left edge underneath the radar.  Per-frame
`BackdropCoverage` tracing proved this was not authored fog or a missing sky
asset: the registered `func_80050B38` panorama packets were split by ordinary
texture/material flushes, and the old safety check rejected each incomplete
batch (`span=Infinity..-Infinity`, `wrong-side>0`, `covered=0`).  The flat
arena backdrop consequently remained visible in the uncovered widescreen
margin.

Enhanced now buffers only the function-owned sky packets until the complete
panorama strip is present, measures the global projected extent across all of
its material batches, extends only the two true outer edges while extrapolating
their native U slope, and replays the panorama before the first non-sky world
primitive.  This is a shared submission rule selected exclusively by the
registered sky-emitter packet range; it contains no arena, texture, material,
or object exception and preserves the existing 4:3 path.

The corrected hidden/silent Florida capture used executable SHA-256
`D8065C9E4C9B0F931A8D604C95BB9C5D275A8F304D73477F03E7F70591317F0E`
at gameplay poll 601, 1920 by 1080.  Every traced panorama frame at the capture
route reached both widescreen boundaries (`covered=1`, `wrong-side=0`), the
white edge rectangle is absent, and the one-map integration gate passed at
58.01 median / 39.48 minimum FPS.  The proof PNG is
`artifacts/v82-dreamcast-full-re/sky-batch-florida-proof/florida_visual_0601.png`
(SHA-256
`3E713AD828FAE2F004ED87CCC90A9CEFC7CF517BBF5500085095F148557C15C6`).
This remains an internal proof until deployment and user visual approval.

The same executable then completed a 360-frame-per-map Florida, Louisiana, and
Hoover Dam regression in one hidden, silent, Enhanced-only process. Every
functional, HD loading-card, HD terrain, fog-reset, route, packet-headroom,
edge-pool, clean-exit, and no-content-branch check passed. Across 1,294 traced
panorama frames, `covered=1` for every frame and `wrong-side=0`. The only red
gate was the separately scoped performance floor: Florida recorded one 24.31
FPS transient while retaining a 59.84 median; Louisiana was 57.02/40.51 and
Hoover Dam 59.87/45.09 median/minimum FPS.

With `RECOMPONE_WIDESCREEN=0`, a separate 360-frame Florida Enhanced run passed
every gate at 60.01 median / 59.15 minimum FPS. The sky batch therefore remains
dormant in 4:3 and does not alter the original-aspect draw path. The exact
candidate SHA-256 above is now deployed as the individual
`V8_2_LOOSE/Vigilante82PC.exe`; all other protected runnable-root content was
left in place.

## Deployed Florida visual pair

The exact deployed executable SHA-256
`D8065C9E4C9B0F931A8D604C95BB9C5D275A8F304D73477F03E7F70591317F0E`
was exercised twice on the same deterministic Florida route at 1920 by 1080.
Both runs used one hidden, silent, Enhanced-only game process and preserved
the executable hash through clean exit.

At gameplay poll 150, the near/mid/horizon frame is retained at
`artifacts/v82-dreamcast-full-re/deployed-florida-near-proof/florida_near_0150.png`
(SHA-256
`CC6C04ED0AC614165E874E8F489EC8800E1C0338927080AAE94C5D0EA3B29394`).
It measured 39.28 median / 4.81 minimum FPS. The isolated low sample occurred
in the startup/capture portion, so this frame is visual evidence only rather
than a performance pass.

At gameplay poll 300, the later mid/horizon frame is retained at
`artifacts/v82-dreamcast-full-re/deployed-florida-mid-proof/florida_mid_0300.png`
(SHA-256
`E86AAAE82865CC200D1F104AC21AEF3AF75F6B7D60CE52B4282F3BF46D0669CA`).
It measured 46.15 median / 31.20 minimum FPS and passed every functional, HD,
terrain, route, packet-headroom, no-content-branch, and sustained-performance
gate. Together the images expose foreground terrain, Sheila, launch-site
structures at near and middle distances, shoreline/horizon composition, and
complete widescreen sky coverage. The existing 1,200-tick ordinary-object
trace remains the exact material-identity evidence: 137 objects produced zero
same-surface source, replacement-key, hit/miss, or resolution changes. These
frames still require the user's visual judgment; they do not close the goal.

## Deployed Louisiana and Hoover visual proofs

The exact deployed executable SHA-256
`D8065C9E4C9B0F931A8D604C95BB9C5D275A8F304D73477F03E7F70591317F0E`
was then exercised in separate, single-process hidden/silent Enhanced-only
runs on Louisiana and Hoover Dam. Both used the same deterministic route and
captured gameplay poll 300 at 1920 by 1080; both completed the functional,
HD-loading-card, HD-terrain, representative-route, packet-headroom,
edge-pool, clean-exit, and no-content-branch checks.

Louisiana is retained at
`artifacts/v82-dreamcast-full-re/deployed-louisiana-proof/louisiana_mid_0300.png`
(SHA-256
`ADA3B099B134F4A7449A4B8E280C029658CC17169F830B93F42F127946FD21FC`).
The frame places detailed ground in the foreground, bayou water and trees at
middle distance, and the fogged treeline/sky at the horizon. The rejected
screen-wide rectangular mist/water overlay is absent. The run measured 26.75
median / 14.37 minimum FPS and is therefore visual evidence only, not a
performance or release pass.

Hoover Dam is retained at
`artifacts/v82-dreamcast-full-re/deployed-hoover-mid-proof/hoover_mid_0300.png`
(SHA-256
`52ADE566279F9ED35CD817F18CB06FE3BC033C3647C1D0C3103A2BC1CFADAA40`).
The frame exposes foreground dam geometry, middle-distance structures and
terrain, and the complete mountain horizon without the formerly rejected
serrated high-detail cutoff. The run measured 30.27 median / 11.25 minimum
FPS and likewise remains visual-only evidence. These isolated capture runs
are materially slower than the same executable's combined three-map gate
(Louisiana 57.02/40.51 and Hoover 59.87/45.09); performance remains a separate
open investigation and no visual-pass claim is inferred from either result.

## Deployed Utah and Minnesota visual proofs

The final two members of the requested five-map set were captured from the
same deployed executable SHA-256
`D8065C9E4C9B0F931A8D604C95BB9C5D275A8F304D73477F03E7F70591317F0E`.
Each used a separate hidden/silent Enhanced-only process, the shared
deterministic route, gameplay poll 300, and 1920-by-1080 presentation. Both
completed the functional, exact-HD-loading-card, exact-HD-terrain, route,
packet-headroom, edge-pool, clean-exit, and no-content-branch checks.

Utah is retained at
`artifacts/v82-dreamcast-full-re/deployed-utah-mid-proof/utah_mid_0300.png`
(SHA-256
`0C363680DEC6C49EC117985056608902A2584D8786722B918D584586B493A37D`).
It exposes detailed foreground snow, middle-distance rock/snow transitions and
trees, and a fogged horizon without the earlier isolated fullbright/jagged
distant triangle. It measured 34.47 median / 10.25 minimum FPS and remains
visual-only evidence pending user judgment.

Minnesota is retained at
`artifacts/v82-dreamcast-full-re/deployed-minnesota-mid-proof/minnesota_mid_0300.png`
(SHA-256
`376317A088DDB8A2D6DF7C4F3585FFACF86ADFA740D5E39AB4BBC930EA566DA3`).
It confirms HD terrain and detailed object textures, but it does not clear the
map's earlier missing/backface-cull failure: the authored title camera is too
close to the foreground billboard, and its upper edge overlaps ambiguous dark
geometry at the viewport boundary. A better mid-distance angle is required.
The run measured 17.10 median / 5.66 minimum FPS, which is independently
unplayable and prevents any release-pass claim even if a later frame resolves
the visual ambiguity.

### Minnesota held-view isolation

A better held view on the same deployed executable reproduces the previously
reported diagonal facade/sign corruption.  The full-size frame is retained at
`artifacts/v82-dreamcast-full-re/deployed-minnesota-held-proof/recompone_present_gameplay_0601_1920x1080_fxaa.png`
(SHA-256
`9A76126DFB71BC682DDAD031B8CB3603123063CD238565E2D88C53EBAF2AADC9`).
It is a failure image, not a proof of parity.

An isolated A/B run then disabled only the Enhanced packet-NCLIP rejection
while keeping the exact deployed executable, held camera, map, texture pack,
and presentation path unchanged.  The diagonal corruption remained.  Its
capture is
`artifacts/v82-dreamcast-full-re/deployed-minnesota-cull-off-diagnostic/recompone_present_gameplay_0601_1280x720_fxaa.png`
(SHA-256
`E6FBA75E4DB81C05FBC1BFDA1E9E09828099B1B77877A5F211107FB1D994A64B`).
This rules out a simply rejected triangle as the immediate cause.

The paired frame-4222 geometry dump shows the affected large ordinary-object
submissions contain coincident triangle pairs with opposite winding and
different texture state.  The current packet-NCLIP association supplies one
shared positive native test to both members of such a pair, so the existing
monotonic rule can retain both and let the wrong-facing material win.  The
next correction must reproduce the Dreamcast ordinary-object `PVR_CULLING_CW`
rule from the submitted triangle's own post-near-clip coordinates.  It must be
function-provenance based and shared across ordinary world objects; no
Minnesota object, address, packet, material, or texture exception is allowed.

Louisiana is an explicit user visual fail on this deployed renderer because
the water edge remains jagged.  That is retained as a separate shared
water-grid/shoreline/depth-composition failure, not a Louisiana-specific
content defect.

### Ordinary-object winding integration candidate

The first shared translation of the recovered Dreamcast
`PVR_CULLING_CW` rule is held at
`artifacts/v82-dreamcast-full-re/minnesota-world-object-cull-candidate/`
on executable SHA-256
`680A73A6515275CD282B1E5BB200DFBF8692C5FABB30FC34F47C245FA1793CDE`.
It applies only to packets emitted by the ordinary-object function seam and
uses each triangle's own exact post-near-clip projection.  Vehicle packets
are excluded at both the provenance assignment and cull boundary so the rule
cannot change Sheila or any other vehicle body/reflection ordering.  The
focused contract now passes 162 assertions.

The held Minnesota capture at poll 601 is
`recompone_present_gameplay_0601_1280x720_fxaa.png` (SHA-256
`56568428C606F156DE10DF8F917C38C90AF966FF8116F7E5D1C96DBCC4782B03`).
Compared with the exact deployed D8065C9E frame, the oppositely-wound
diagonal facade/sign impostor is absent while the intended wall face remains.
This is internal evidence only: the candidate is not deployed and still
requires representative-map regression plus user visual approval.

### Louisiana shoreline depth isolation

The current shared water/terrain composition normally writes the coarse PS1
ordering-table bucket into the Enhanced depth image.  That is visibly capable
of producing a polygon/tile-shaped shoreline even though the native water
surface itself is a continuous grid.  A held diagnostic therefore ran the
same SHA-256 680A73A6 candidate with the existing global exact-vertex-depth
control enabled, on Louisiana at poll 300.  It remained Enhanced-only,
hidden/silent, used one process, loaded the exact HD card and terrain atlas,
and produced
`artifacts/v82-dreamcast-full-re/louisiana-true-depth-diagnostic/recompone_present_gameplay_0300_1280x720_fxaa.ppm`
(SHA-256
`F9BF74220586D4389BD45A4EC4B707A2D2A511933EF191B148578BBB595C5B3E`).
The run measured 59.94 median / 36.88 minimum FPS.  Its harness route gate is
not an acceptance result because the held fixture restarted gameplay during
the requested 1,200-frame window.  The image is retained only to compare the
shoreline before narrowing exact depth to the directly recovered
terrain/water material contract; no runnable-root deployment occurred.

The held frame did not expose water and is therefore not usable for the
shoreline A/B.  A second diagnostic repeated the exact deployed-proof route
without the hold at 1920 by 1080, still on SHA-256 680A73A6 and with only the
global exact-vertex-depth switch changed.  Every functional, Enhanced-only,
single-process, HD loading/terrain, route, packet-headroom, and clean-exit gate
passed.  The retained poll-300 frame is
`artifacts/v82-dreamcast-full-re/louisiana-true-depth-route/louisiana_true_depth_0300.png`
(SHA-256
`08836D73BF6BECE1BDBCBA2784CE5C28374312265E09B912976B78F5E6FB605F`).
It measured 33.99 median / 5.84 minimum FPS and is a visual isolation artifact,
not a release-performance pass.  The corresponding deployed comparison uses
the same map, player, route and poll at
`artifacts/v82-dreamcast-full-re/deployed-louisiana-proof/louisiana_mid_0300.png`.
Visual comparison of that exact pair is the next gate before any source-depth
behavior is made default.

That exact-route visual A/B showed no material shoreline change: the same
jagged/striped Louisiana water edge remains with exact per-vertex depth.  The
depth hypothesis is therefore rejected and the global exact-depth control must
not become the default from this evidence.

The next shared mismatch found in the direct Dreamcast water contract is face
culling.  Both retail water passes compile with culling disabled, while the PC
Enhanced path was still applying generic packet-NCLIP rejection to both
`WaterBase` and `WaterSurface`.  The held correction exempts exactly those two
material classes from packet-NCLIP culling, preserving every triangle in their
continuous row strips without any map, cell, address, or texture exception.
MeshClipContract passes all 165 assertions.  A route-identical Louisiana run on
held executable SHA-256
`0A764E480878DE858B1E1C8BF164911E84A0380F82FCDA0F781E2DD2B0CDF6A9`
completed cleanly with one Enhanced-only process, exact HD loading/terrain,
and retained poll-300 proof
`artifacts/v82-dreamcast-full-re/louisiana-water-no-cull-route/louisiana_water_no_cull_0300.png`
(SHA-256
`147003337691DE60699106424F1DB1A5B86C88C8DBE1C997415E8789500BD520`).
Its 25.28 median / 10.21 minimum FPS fails the separate performance gate; the
image is held for visual inspection and cannot be accepted or deployed without
that inspection and user approval.

### Louisiana base-water provenance correction

The first continuous-base-plane candidate did not actually reach the recovered
Dreamcast water path.  Packet traces proved that the native function seam
correctly owned the scanline packets as `WaterBase` and attached the exact
four-vertex plane, but `GpuHleForward` subsequently matched the same
full-width, one-pixel primitives with its generic screen-effect heuristic and
overwrote the function-owned material.  This explains why the earlier
continuous-plane screenshot still showed the PS1 approximation.  The shared
classification order now preserves explicit `WaterBase`/`WaterSurface`
ownership ahead of the fallback weather-strip heuristic; there is no map,
cell, texture, address, or authored-content condition.

A hidden, silent, single-process Enhanced-only Louisiana route then passed
every functional, HD loading/terrain, route, packet, clean-exit, performance,
and no-content-branch gate on held executable SHA-256
`B196AE81AD1753AC51220F4F92CA6550900CB93ABE5F8DCB79B94728BD6457B7`.
The trace records 64 base-water replacements and retained the full-size
poll-300 proof at
`artifacts/v82-dreamcast-full-re/louisiana-water-owner-precedence-proof/recompone_present_gameplay_0300_1920x1080_fxaa.png`
(SHA-256
`06CE27FA8A62A64ADDBD6336DD20B72161617C5770C037A8E50FDF3E6BD3548E`).
It measured 51.52 median / 35.41 minimum FPS.

That image is still not a visual pass.  The PS1 approximation changes state
partway through one native base-water call, so one recovered call was replayed
as two overlapping Dreamcast quads with different inherited colours.  The
result is overly dark and retains horizontal banding.  The current held source
coalesces all batches with the same function-call provenance before emitting
one replacement strip, matching the Dreamcast's single PVR submission.  This
coalescing change has passed the 172-assertion mesh/provenance contract and both
hash-gated water contracts, but it is not yet built, visually qualified, or
deployed. Louisiana therefore remains **USER FAIL**.

### Direct-water RE reset

The Louisiana failure demonstrates that the preceding depth, culling, and
packet-coalescing experiments were hypothesis-driven adaptations around the
PS1 draw stream. The recovered Dreamcast material words, authored colour,
constants, call order, and broad grid structure remain valid binary facts, but
those experiments are not a full reverse engineering of the Dreamcast water
renderer and must not be described or shipped as such. Exact-vertex depth was
visually rejected; the other held candidates remain non-deployed diagnostics.

The work resumes from the retail executable itself. First, the complete
four-vertex pass is being translated into address-cited pseudocode. Second,
the complete XWAT routine will be reconstructed through its view-plane
intersection, 32-unit grid bounds, terrain-corner classification, two-wave
field, projection, UV generation, row-strip emission, and PVR submission.
Only the resulting shared algorithm may replace the PS1 surface stream.

## Required remaining validation work

- Validate the translated sky, water, fog, terrain-mip, reflection, and
  ordinary-object material contracts together on representative maps.
- Obtain well-framed near/mid/horizon proof that the now-stable Florida
  building replacement identity also has stable visible sampling at close and
  distant viewpoints.
- Validate the shared PS1/Dreamcast continuous water-grid equivalence and the
  translated Dreamcast alpha/depth composition at several shorelines; retain
  the screen-space base pass only as the matching first pass, never as a
  substitute for the textured grid.
- Use shared Enhanced paths and explicit 4:3/widescreen regression checks.
- Retain proof metadata: executable SHA-256, map, capture point, aspect ratio,
  and FPS. User visual approval remains mandatory.
