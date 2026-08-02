# Terrain EXP Runtime Notes

Source-driven notes from `analysis/dll/LOAD` and `analysis/SLUS_005.10`.

## Height Sources

Runtime driving height is not a single flat heightmap.

1. `Terrain_InitFlatWorld` (`SLUS 800251fc`) initializes the full `DAT_800911a0`
   table to a valid flat 0x3000-byte chunk.
2. `ZONE` chunks are converted by `LOAD 801057f0` into runtime 0x3000-byte
   height/material chunks.
3. `ZMAP` installs those detailed ZONE chunks into selected 32x32 table slots.
4. `Terrain_HeightAndProbe` (`SLUS 8001d748`) samples `Terrain_HeightAt`, then
   calls object obstacle trees at `obj+0x74` and `obj+0x78`.
5. `Object_FindObstacleAt` / `ObstacleLeaf_Test` (`SLUS 8001f51c/8001ef74`)
   can override the height with object/patch geometry.

The host cannot treat arbitrary visual XOBF triangles as the final physics
surface. Headless probes show that doing so can put the vehicle/camera inside
placed structure geometry. The original path is stricter: ZONE/table terrain is
the base, and object obstacle leaf streams reached through the runtime object
tree may override that height. The current host also keeps a diagnostic
ground-triangle query so XOBF patch coverage can be audited headlessly while the
exact runtime chain attachment is refined.

## Chunk Roles

Important tooling note: `LOAD.DLL` must be pre-relocated with 32-bit overflow
for relocation tag 3. The source expression is effectively
`word += (((uint32_t)(base << 4)) >> 6)`. If the overflow is not preserved,
synthetic bases such as `0x80100000` turn real `j`/`jal` opcodes into
`slti`/`sltiu`, which hides calls such as `80100e98 -> 801006f0` and corrupts
the inferred format.

Corrected `LOAD 8010167c` dispatches all observed top-level terrain chunks:

| Chunk | Handler | Runtime role |
| --- | --- | --- |
| `FORM OBJ ` | `80100e98` | placed object HEAD/STRN/LGHT dispatcher |
| `FORM XOBF` | `80100408` | object/geometry bank append |
| `HEAD` | `801002ac` | top-level TERR setup and flat terrain init |
| `TEXT` | `801003a0` | renderer texture/text metadata |
| `XBMP` | `8010542c` | renderer bitmap upload metadata |
| `TINF` | `80105550` | terrain material/texture info |
| `ZONE` | `801057f0` | source height/material chunk conversion |
| `ZMAP` | `801058f4` | installs ZONE runtime chunks into terrain table |
| `AIMP` | `801005e8` | AI/navigation quadtree raw copy |
| `RECT` | `8010063c` | 14-byte map/region record |
| `XRTP` | `80104a94` | route-surface descriptor and optional texture |
| `JUNC` | `80104d1c` | route graph node and optional patch mesh |
| `RSEG` | `80105060` | route graph edge and route-strip build |
| `BSP ` | `801005c0` | static-object kd-tree root |
| `SUNA` | `80100470` | renderer/environment sun setup |
| `COLS` | `80105318` | color/light/fog setup |
| `XENV` | `80100594` | environment blob loader |
| `XBGM` | `80105a18` | renderer/background image metadata |

Under this dispatcher, the gameplay-area carriers are `ZONE/ZMAP`, object
obstacle streams reached through `FORM XOBF/BIN` and `FORM OBJ/HEAD`, `BSP`,
`AIMP`, `JUNC`, `RSEG`, and `XRTP`. The other observed chunks feed renderer,
texture, color, environment, or audio-adjacent seams.

- `ZONE`: 64x64 source cells, 4 bytes per cell. Converted by `LOAD 801057f0`.
- `ZMAP`: 32x32 big-endian u16 table. File rows are chunk-Z, columns are chunk-X.
- Top-level `HEAD` under `FORM TERR`: fixed terrain setup record consumed by
  `LOAD 801002ac`. Source reads five big-endian i16 header words, then copies
  remaining i16 table words to `DAT_800658e8`. Header word 0 is passed to
  `Terrain_InitFlatWorld`; header words 1 and 2 are passed to `0x80041d40`;
  header words 3 and 4 are consumed and discarded. The fourth copied table
  word is difficulty-scaled in place.
- `COLS`: fixed 28-byte color/light/fog setup record consumed by hidden LOAD
  code at `0x80105318`. It is seven big-endian u32 words copied to globals
  around `DAT_80065b58`, `_DAT_80065b00`, `DAT_80065b2c`, `DAT_80065b54`,
  `_DAT_80065b08`, `_DAT_80065b10`, and `_DAT_80065af8`; the source then
  derives renderer color deltas from these bytes.
- `TINF`: terrain material/texture info. `LOAD 80105550` builds `DAT_8008f020`.
- `XLSC`, `XBMP`, `XBGM`: texture/image upload paths (`LOAD 80105a18`,
  `80105ca4`, `8010542c`), not heightmaps. XLSC chunks have PSX image/rect-like
  headers (`0x40017000`, width/height/CLUT fields) and are read by renderer
  texture upload code, not by `Terrain_HeightAt`.
  `tools/extract_terrain_images.py` decodes `XBMP` and `XBGM` as 8bpp paletted
  source images for headless presentation audits.
- `tools/terrain_material_audit.py` cross-checks ZONE material bytes against
  TINF records for all terrain EXPs. It writes
  `analysis/terrain_material_audit.txt`.
- `XRTP`: route-surface descriptor plus optional texture payload consumed by
  `LOAD 80104a94`. Each chunk appends one 0x34-byte runtime descriptor to
  `_DAT_80065bd4`; the first 12 payload bytes are `i32 width16`, `i32 step16`,
  `i16 texture_id`, `u16 flags`. RSEG `type` indexes this descriptor array.
  If a TIM payload is present, `800187e4` fills the descriptor's PSX texture
  rectangle fields (`width_pixels`, `height_pixels`, base UV, tpage, clut).
  `LOAD 8010246c` then builds route primitive templates: flags bit `0x0002`
  selects textured `POLY_GT4` packets (`80106214` stamps packet length/code
  `0x0c/0x3c`), while the clear path builds untextured `POLY_FT4`-sized
  templates (`801061f4` stamps `0x09/0x2c`). Flags bit `0x0100` ORs the PSX
  semitrans bit into the primitive code. The template code also creates four
  quadrant UV variants per texture rectangle, so route drawing is packet-based
  and semitransparent; it is not an opaque host decal layer.
- `FORM XOBF/BIN`: level visual geometry, slot tree, and obstacle leaf tables.
- `FORM OBJ/HEAD`: placed object records consumed by `LOAD 80100e98 ->
  801006f0`. HEAD X/Z are 16.16 cell coordinates; HEAD Y is stored in the
  same height unit returned by `Terrain_HeightAt`, with source subtracting
  `0x100000` before writing object `+0x4c`. HEAD and JUNC store their
  coordinate pairs as X then Z. Object HEAD layout is:
  `u8 script`, `u8 type`, `i16 id`, `u32 flags`, `i32 x`, `i32 y`,
  `i32 z`, `i16 rot0/rot1/rot2`, `i16 xobf_index`, `i16 slot`,
  `i32 initial_strength`, then a NUL-terminated callback/name string.
  `xobf_index` is biased by `+0x12` before indexing `DAT_800737a0`.
  The HEAD type byte is dispatched by the corrected switch table at
  `LOAD 80100008`: type 0 builds a full placed static object, type 1 returns
  no object, types 2/3/4 use the general object build path, type 5 builds a
  real 0x80 object and inserts it into `DAT_80065a50`, and type 6 builds a
  light/list object in `DAT_80107d90`. Type 5 stores the bank pointer and slot,
  but it does not call `FUN_80021b80`/`FUN_8001ac44`, so host diagnostics do
  not draw it as a normal static slot-tree until the runtime callback draw path
  is decoded.
- `FORM OBJ/STRN`: optional per-object strength/health override consumed by
  `LOAD 80100d1c`. It reads one big-endian i32, and a second i32 when the
  payload is longer than four bytes; the values write object `+0x0c/+0x0e`.
- `FORM OBJ/LGHT`: optional per-object light record consumed by
  `LOAD 80100e20`. Corrected MIPS shows it writes `i32 +0x80`, `i32 +0x84`,
  `i32 +0x88`, `i16 +0x8c`, `i16 +0x8e`, and `i16 +0x90` on the placed
  object. `LOAD 801011a0` then walks the `DAT_80107d90` light list at draw
  time, calls `80100fa8` for cone/radius attenuation, and adds
  `(attenuation * obj[+0x80..+0x82]) >> 12` to the base vertex color. The
  first word is therefore RGB/pad bytes, not a local-space center; the next
  words are inner radius, outer radius, cone min/max, and intensity. These
  records occur on type-6 HEAD objects in Casino City and Wild West. They are
  grouped under their parent OBJ and are not missing far terrain patches,
  occluders, or collision boxes.
- `AIMP`: AI/navigation quadtree. The payload is an array of 10-byte
  little-endian cells. Each cell is five u16 values: flags/weight at `+0`,
  then four child entries at `+2,+4,+6,+8`. Child `0` means empty; child with
  bit `0x8000` set is a leaf/cost/neighbor flag value; otherwise child base is
  `current_cell + child_offset`. Runtime walkers `SLUS 800244c4/800247dc`
  start at root cell 0 and consume X/Z sign bits from integer terrain-cell
  coordinates. The root covers 2048x2048 cells.
- `JUNC`: route graph node. `LOAD 80104d1c` reads `i32 x`, `i32 z`, `u8
  flags`, and `u8 edge_count`. If `flags&2` is set, an explicit `i32 y`
  follows, but runtime still refreshes node Y from `Terrain_HeightAt(x,z)`.
  Longer records then contain `i16 bank`, `i16 slot`, `i16 rot`, followed by an
  ASCII patch name. The slot is resolved through the XOBF slot table to a render
  group. Runtime rotates that group's local X/Z vertices by `rot`, samples
  `Terrain_HeightAt` for every transformed vertex, and replaces the group's
  vertex array with this terrain-conformed road/junction patch.
- `RSEG`: route graph edge. `LOAD 80105060` allocates a 0x20-byte edge record:
  `i16 route_type`, `i16 order_or_ai`, `u16 flags`, `i16 node_a`, `i16 node_b`,
  then four `i32` Bezier control offsets (`start_xz`, `end_xz`). It links the
  record into both endpoint JUNC edge arrays. `LOAD 80104550` builds the four
  control points, optionally adjusts endpoints against JUNC patch meshes, and
  tail-calls `LOAD 80102bd4` with the XRTP descriptor selected by `route_type`.
  `LOAD 80102bd4` tessellates a terrain-conformed road/route strip: it steps
  along the cubic by descriptor `step16`, offsets left/right by `width16/2`,
  samples `Terrain_HeightAt` for both strip vertices, and stores material bits
  from `DAT_800911a0`.
- `BSP `: recursive big-endian static-object kd-tree consumed by `LOAD
  801005c0 -> 80100148`. The stream is preorder:
  `i16 kind`, with kind `0` as a leaf and kind `1/2` as split-X/split-Z
  followed by `i32 coord16`, then left and right child subtrees. Runtime allocates
  one 0x10-byte node per stream node. Leaf nodes do not contain object ids on
  disc; they initialize empty list sentinels and `LOAD 801001ec` inserts each
  placed type-0 object list node by comparing object `+0x48` (X) or `+0x50` (Z)
  against the split coordinate. The tree is then used by collision, AI target
  scans, object broadcasts, and renderer/streaming eviction through
  `uRam000006fc` / `_DAT_80065a00`.
  The host runtime now decodes this same stream and leaf-links placed obstacle
  objects by HEAD root X/Z before probing `Object_FindObstacleAt`. This removes
  the previous loose global obstacle scan and matches the source's leaf-scoped
  static-object organization. Current headless sweep:
  AirGrave `39/20/20`, CanyonLands `67/34/34`, CasinoCity `105/53/53`,
  HooverDam `63/32/32`, OilField `45/23/23`, SandFactory `47/24/24`,
  SecretBase `29/15/15`, SkiResort `117/59/59`, ValleyFarm `123/62/62`,
  WildWest `53/27/27` for `nodes/leaves/occupied_leaves`.
- `RECT`: 14-byte region record consumed by `LOAD 8010063c`: `i16 x0`, `i16
  z0`, `i16 x1`, `i16 z1`, discarded `i16`, `i16 attrA`, `i16 attrB`. Runtime
  stores inclusive width/depth as `(x1-x0)+1`, `(z1-z0)+1` and links each
  record into the `DAT_80065aa4` region list.

## XOBF BIN Header

All terrain XOBF BINs audited so far share this header:

- `+0x00 u32`: render group count
- `+0x04 u32`: render group pointer-table offset
- `+0x08 u32`: obstacle stream count. N64 banks carry one additional
  boundary offset after these entries; it marks the end of the collision
  region and is not a stream. The PS1 loader relocates exactly `count`
  entries.
- `+0x0c u32`: obstacle table offset
- `+0x10 u32`: secondary table/count, still texture/animation-adjacent
- `+0x14 u32`: first offset after the obstacle table region
- `+0x18 u32`: slot count
- `+0x1c`: slot records, `0x1c` bytes each

The older derived slot count `(group_table - 0x1c) / 0x1c` happens to match
the audited files, but `+0x18` is the field the format actually carries.

## N64 XOBF Texture Scanlines

The N64 archive stores texture rows in 64-bit TMEM units, padded independently
to an eight-byte boundary. Every odd row swaps the two 32-bit halves within
each unit. A linear decode without that odd-row swap creates alternating
four-byte horizontal bands; on Dreamland this visibly corrupts doors, roofs,
castle masonry, foliage, and effect sprites.

`parse_n64_texture` restores ordinary left-to-right texel order before CI or
RGBA conversion. The source-to-native audit decodes the resulting PS1 TIMs
independently, crops only native row padding, and compares every visible RGB5
texel. It also compares every converted packet's vertex order, UV triplets,
and final texture-slot assignment, including palette and translucency
variants.

## XOBF Polygon Packets

`FUN_8001b49c` builds cached PSX draw packets from raw BIN polygon packets.
The loose-file source packet kind is `type_byte & 0x0f`. Before cached packets
are built, `FUN_8001a640` rewrites that byte in place: it shifts the source
kind into bits 2..5 and maps the source flag bits into the runtime low bits.
`FUN_8001b49c` therefore decodes `(expanded_type_byte >> 2) & 0x0f`, but a
tool or host path reading the unexpanded loose bytes must use the low nibble.

For source kind 13, texture id `0xffff` is the explicit engine-owned dynamic
image descriptor at `DAT_80065a28`. Values with upper bits `0x4000` or
`0x8000` are still ordinary XOBF texture references: the loader masks with
`0x3fff` for the table index and transfers the upper bits into PSX texture-page
state. Treating `0x4000` as the dynamic-image sentinel corrupts ordinary
materials and is not source behavior.

## Super Dreamland 64 RECT Water

Dreamland's sole N64 `RECT` payload is
`0354052c0391056800000043ffff`. Its authored cell rectangle is therefore
X `[852,913)`, Z `[1324,1384)`, and selector `-1`. The overlay function at
N64 `0x80159900` draws this as a world surface at
`Terrain_HeightAt(center) - 16` height samples and advances XBMP cells
`0x23,0x24,0x25,0x26,0x27,0x26,0x25,0x24`, six ticks per cell.
The records consumed by that routine are authored world rectangles, not
screen-clipped spans: the complete 61-by-60 surface exists in world space and
ordinary scene occlusion determines which parts are visible.

The original V8 PS1 port represents the effect with an append-only third XOBF
bank rather than a presentation overlay or borrowed retail object:

- One model-free root plus 64 hierarchy patches covers the exact 61 by 60 cell
  rectangle with native 8-by-8 patches and smaller source-boundary patches.
- Four render groups cover the four patch dimensions. Casino City's shipped
  `water_1` group 113 is the structural reference: its 50 geometric triangles
  are stored as 100 kind-15 packets, one of each winding. Dreamland stores
  each forward/reverse pair adjacently under the same texture descriptor, so
  looking through the surface from below does not make it disappear.
- The five CI8 source frames are independently quantized to five CI4 TIMs.
  Their converted mean luminance differs from source by at most 0.89/255 and
  RGB RMSE is 4.58-5.22/255; the conversion does not brighten the palette.
- N64 function `0x80159900` supplies neutral white RGB and alpha `0xa0`
  (160/255). PS1's shipped water uses neutral packet RGB and fixed ABR mode
  zero (1/2 source plus 1/2 destination). A second set of five CI4 frames
  reserves transparent index zero and covers exactly half the texels. Drawing
  that coverage pass with the base pass gives a spatially averaged 5/8 source
  contribution (0.625), within 0.25% of the N64 alpha, without guessed color
  correction or a host-only blend mode.
- A native `ANM ` stream applies both flag-`0x10` texture bindings to all 64
  patch slots. The decoded phase sequence and the base/coverage passes remain
  synchronized in one stream and retain the original 65-node hierarchy. Every
  water group sets BIN `+0x19` to ten, causing `FUN_8001b49c` to cache all ten
  possible animated descriptors at `cached_group+0x2c`. Declaring only one
  descriptor while packets reference the coverage slot at index five makes
  the native renderer read beyond that pointer array; this was the cause of
  the earlier build's inconsistent water rendering.
- The new `DreamlandWater` HEAD uses the next free ordinary object id and does
  not replace, alias, or borrow any source object.

A rendered Casino City control placed the camera below the native water plane:
the water polygon remains visible, while the world and HUD retain their normal
map colors. Original V8 therefore provides no separate underwater full-screen
tint to copy into Dreamland; adding one would be a new effect.

A non-deployed Dreamland sidedness control changes only the
`DreamlandWater` HEAD Y field, raising the unchanged converted surface above
the normal gameplay camera. The rendered game shows its animated texture
overhead from below. This verifies that the reverse packets reach and pass the
native renderer, while the shipped loose-files asset retains the authored
`22.6640625` world-unit plane height.

The enhanced renderer depth-tests semitransparent triangles carrying GTE depth
against the opaque world while leaving depth writes disabled for them. This
keeps transparent ordering and blending intact but prevents the valid full
water plane from painting over nearer terrain. Timed capture sequences showed
the old failure only while the camera was inside the authored RECT footprint;
the same route no longer shows terrain takeover after this correction.

The later 2026-08-01 regression showed that three independent screen-coordinate
depth matches were still insufficient: each water vertex could select a valid
but unrelated overlapping projection, while opaque land wrote one flat
ordering-table depth for the whole triangle. Raster depth now accepts a
per-vertex triplet only when all three vertices carry the same native GTE
projection-group identity. Opaque and semitransparent textured triangles then
use those coherent `SZ` values on the same normalized scale; unmatched packets
retain their native ordering-table fallback. The bounded same-group lookup is
separate from the broader perspective-texture recovery so dense terrain does
not incur the combinatorial fallback search.

The reported terrain-colored wedges were not a heightfield or clipping defect.
N64 flag-`0x40` morph records for `mushroom_bad` contain 26 changing vertices
for a 36-vertex render group; the N64 path retains the ten static tail
vertices, while the PS1 path replaces the vertex-table pointer. Reading beyond
the short converted keyframe interpreted animation metadata as coordinates and
created the spikes. Conversion now expands every partial morph record with its
own source group's static tail and rebuilds relative loop offsets. Rendered
regressions through ticks 1740, 2220, and 4620 show no wedges with the authored
faces and animation retained.

`tools/audit_v8_n64_model_conversion.py` is the source-to-native structural
gate for Dreamland. It verifies every original OBJ/HEAD payload and hierarchy
slot, exact group vertex triplets, expanded PS1 polygon counts, scale fields,
every N64 display-list texture address, every base texture texel, and every
packet's UV/texture binding. The current 150-group audit has no conversion
failures, texel mismatches, packet-mapping mismatches, or unresolved texture
addresses.

N64 RGBA5551 texels whose authored alpha bit is clear are emitted as PS1
palette index/color zero even on otherwise opaque materials. This is derived
directly from the source texture data and restores cutout transparency for
foliage and billboards without object-name heuristics or borrowed material
metadata.

## XOBF Slot Record

`FUN_8001ac44` walks slots recursively from HEAD `slot`:

- `+0x00 i16`: render group key. `>=0` means primary group id. `0xcxxx` keys
  are secondary groups found by `FUN_8001b0c4`.
- `+0x02 i16`: obstacle stream index. `>=0` indexes the table at header `+0x0c`.
- `+0x04 i32`: local X, 16.16
- `+0x08 i32`: local Y, Terrain_HeightAt-compatible height units (`/131072`
  for host display)
- `+0x0c i32`: local Z, 16.16
- `+0x10 i16`: Y rotation, 4.12 angle
- `+0x12 i16`: X rotation, 4.12 angle
- `+0x14 i16`: Z rotation, 4.12 angle
- `+0x16 i16`: object flags/id copied into object fields
- `+0x18 i16`: next sibling slot, stored in runtime object `+0x34`
- `+0x1a i16`: first child slot, stored in runtime object `+0x38`

HEAD root transforms override the root slot local transform. Child and sibling
traversal follows the original flag rules in `FUN_8001ac44`.

`FUN_8001b0c4` starts at an object's first-child slot (`+0x1a`) and follows
the next-sibling chain (`+0x18`) looking for key0 values with high nibble
`0xc000`. Those slots are secondary visual groups: the group id is
`key0 & 0x7ff`, and the group is rendered with the parent object's matrix
rather than as a separately transformed child.

## XOBF Render Group Scale

The complete 28-byte native render-group descriptor is:

- `+0x00 u32`: vertex count
- `+0x04 u32`: relative vertex-table offset
- `+0x08 u32`: normal count
- `+0x0c u32`: relative normal-table offset
- `+0x10 u32`: polygon packet count
- `+0x14 u32`: relative polygon-stream offset
- `+0x18 u8`: render scale
- `+0x19 u8`: local animated-texture descriptor count
- `+0x1a u16`: render extent copied to the cached runtime group

The polygon count at `+0x10` is one 32-bit field, not a 16-bit count followed
by a texture base. `func_8001E914` reads it with `lw` and uses it as the packet
loop bound. Treating `+0x12` as a separate base turned a 36-packet transformed
wheel group into 131,108 packets and walked beyond the source bank. Ordinary
textured packets therefore carry their compacted bank texture directly.
Kind-15 packets in groups with a nonzero `+0x19` instead retain their local
0-based animated-material slot; the ANM stream populates those cached slots.

Group descriptor byte `+0x18` is the render scale used by `SLUS 8001be5c`.
The source computes `shift = 16 - scale`, right-shifts the object MATRIX
translation by `shift`, then transforms raw `SVECTOR` vertices. Host display
therefore maps local X/Y/Z as `raw / (1 << scale)` after object transform.
Physics still keeps the original integer height units for `Terrain_HeightAt`
and obstacle tests.

Cross-level audit shows most terrain XOBF render groups use scale 8, with
smaller counts at 9, 10, and 12. The older host `/16` visual scale was a
pre-source placeholder and made placed groups too large relative to source
matrices.

Vehicle BINs also carry descriptor scale bytes. `tools/scale_audit.py` compares
the current descriptor-scale vehicle rendering against placed OilField
`OilPump_1` instances: vehicles average `0.82 x 0.54 x 1.62`, while OilPump_1
averages `0.97 x 1.30 x 3.18`. That ratio is plausible for the pump field, but
the renderer still needs the original vehicle draw/object-tree path before any
scale claim should be treated as final.

## JUNC Patch Meshes

Long JUNC records are not extra heightmaps. They are source-driven visual patch
instances:

- Base record: `i32 x`, `i32 z`, `u8 flags`, `u8 edge_count`.
- Optional explicit `i32 y` when `flags&2`; runtime immediately replaces it
  with `Terrain_HeightAt(x,z)`.
- Patch tail: `i16 bank`, `i16 slot`, `i16 rot`, ASCII name.
- Runtime resolves `slot` through the XOBF slot table, takes that slot's
  `key0 & 0x7ff` render group, and uses the group's descriptor byte `+0x18`
  as the source scale (`shift = 16 - scale`).
- Each local X/Z vertex is rotated by the JUNC `rot` angle, shifted into
  16.16 world coordinates, sampled through `Terrain_HeightAt`, and written
  back as a terrain-conformed local vertex array.

The host diagnostic mesh now mirrors this path headlessly. Current cross-level
counts from `analysis/terrain_probe_sweep_latest.txt`: AirGrave `4/80`
patches/tris, CanyonLands `11/336`, OilField `11/168`, SandFactory `24/472`,
SecretBase `7/152`, SkiResort `10/120`, ValleyFarm `2/32`, WildWest `8/160`;
CasinoCity and HooverDam have no JUNC patch tails.

## Obstacle Leaf Streams

`FUN_8001aaa8` stores `obj+0x5c = obstacle_table[slot.key1]` when `key1 >= 0`.
The table entries are u32 offsets relative to the obstacle table base. The
record stream is consumed by `FUN_8001ef74`:

- kind `0`: terminator.
- kind `1`: 0x1c-byte AABB/flat override.
  - `+0x04 i32`: min X
  - `+0x08 i32`: override Y
  - `+0x0c i32`: min Z
  - `+0x10 i32`: max X
  - `+0x14 i32`: max/local Y ceiling
  - `+0x18 i32`: max Z
- kind `2`: convex sloped patch.
  - `+0x02 u16`: plane count
  - then `plane_count` records of 12 bytes:
    `i16 nx, i16 ny, i16 nz, i16 pad, i32 plane_offset`
  - `SLUS 8001ef74` clips a vertical probe against the patch planes by tracking
    the maximum intersection for planes whose rotated Y normal is negative and
    the minimum intersection for planes whose rotated Y normal is positive.
    The hit is accepted only when `lower_t < upper_t`, the result is above the
    current terrain height, within `0x2800` of the probe Y, and the selected
    normal Y is less than `-0x800`.

`tools/xobf_collision_audit.py` decodes these tables without unknown record
kinds across all current terrain EXPs and writes the cross-level report to
`analysis/xobf_collision_audit.txt`.

## Runtime Bounds Cross-Check

`tools/terrain_runtime_decode.py` decodes the runtime-relevant chunks headlessly
and writes the current cross-level report to
`analysis/terrain_runtime_decode_latest.txt`.

RSEG/XRTP route strips decode cleanly using Bezier control offsets, route
widths, and route steps. The host renderer now uploads only route types with
decoded TIM payloads and treats XRTP `0x0100` as source semitransparency. Route
types without a payload are left audit-only until their material source is
identified; drawing them as flat fallback geometry produces unsupported slabs
that are not source evidence.

`tools/terrain_visual_format_audit.py` is the durable visual ownership audit.
For Wild West it currently reports:

- XOBF banks: 2. Bank 0 has 223 render groups, 699 slots, and 131 texture
  slots; bank 1 has 15 render groups, 15 slots, and 16 texture slots.
- Raw loose-file XOBF packet iteration must use the source kind
  `packet[3] & 0x0f`. `FUN_8001a640` expands that low nibble into bits 2..5
  in-place before `FUN_8001b49c` builds cached renderer packets; applying the
  runtime shift directly to the loose bytes loses packet alignment.
- The eight JUNC visual patches are not flat route polygons. They resolve to
  bank 0 groups 215 (`JUNCTION_TSection`) and 216 (`JUNCTION_Ending`), and both
  groups contain only source kind `5` textured triangle packets with valid
  texture refs. Rendering those as flat colored geometry is a renderer bug.
- Wild West XRTP type 0 carries the rail/track TIM payload, type 1 carries the
  dirt road TIM payload, and type 2 has no TIM payload. Type 2 is therefore not
  the train-track texture source.
- XRTP `0x0100` belongs to PSX primitive semitransparency/blending. It should
  not be translated into blanket per-texel OpenGL alpha for every nonzero
  palette entry; doing so creates pale route strips instead of readable road and
  rail textures. Transparent black remains per-texel alpha.

`tools/terrain_route_surface_audit.py` independently rebuilds the ZMAP/ZONE
runtime height table and samples `Terrain_HeightAt` along JUNC/RSEG route data
and AIMP leaf centers. This is the current preferred headless check for "is the
gameplay terrain decoded?" because it follows the original navigation graph
instead of the renderer camera. The latest report is
`analysis/terrain_route_surface_audit.txt`.

For OilField, the independently decoded structures agree:

- ZMAP detail footprint: chunk X `[13..15]`, chunk Z `[18..19]`.
- HEAD placements: X `[828.2..985.0]`, Z `[1178.2..1282.6]`.
- AIMP nav leaves: X `[852..988]`, Z `[1186..1254]`.
- JUNC route nodes: X `[853.7..983.8]`, Z `[1188.8..1246.7]`.
- BSP kd-tree: 45 nodes / 23 leaves; split planes X `[871.4..970.7]`, Z
  `[1196.9..1237.9]`; all 93 type-0 placed objects insert into the tree.
- The host renderer now draws the ZMAP/ZONE surface at 2-cell sampling instead
  of the previous 8-cell debug mesh, and suppresses broad upward XOBF faces
  from the visual upload while keeping them in the CPU triangle set. This keeps
  collision/source geometry available without letting debug patches masquerade
  as authored ground.
- Vehicle host mesh display now uses the same descriptor scale byte at group
  `+0x18` as the terrain XOBF path. With descriptor scaling, the current
  vehicle average is `0.82 x 0.54 x 1.62`, while OilField `OilPump_1` averages
  `0.97 x 1.30 x 3.18`. This matches the source render path better than the
  old hard-coded `1/160` display scale and removes the visible "Godzilla car"
  error against placed buildings.
- OilField presentation images decode headlessly as:
  - `XBMP`: 432 x 192 pixels, flags `0x9`, CLUT rect `(0,480,256,1)`, image
    rect `(0,0,216,192)` in 16-bit VRAM words.
  - `XBGM`: 256 x 92 pixels, flags `0x9`, CLUT rect `(0,480,256,1)`, image
    rect `(0,-8,128,92)` in 16-bit VRAM words.
  These are the source brown terrain atlas and backdrop data missing from the
  current flat-shaded renderer path.
- OilField ZONE material usage is broad, not a single material: the six ZONE
  chunks contain 246 distinct material ids. Top ids include `0` (16892 cells),
  `245` (999), `247` (932), and `9` (902). The flat host terrain color is
  therefore still only a debug stand-in until TINF/XBMP texture selection is
  wired.

For Ski Resort, HEAD placements include far scenery/object placements, but the
drive/navigation core is still the AIMP/JUNC/RSEG cluster:

- HEAD placements: X `[140.2..801.4]`, Z `[890.2..1873.8]`.
- AIMP nav leaves: X `[664..800]`, Z `[916..1076]`.
- JUNC route nodes: X `[684.5..783.5]`, Z `[937.5..1069.5]`.
- Full XOBF visual extents after placement: X `[131.2..801.6]`,
  Z `[890.2..1882.8]`, but upward ground candidates are limited to
  X `[656.8..801.6]`, Z `[906.3..1103.6]`. The far-Z outlier is a named tree
  placement (`pine1_1`), not a driveable terrain sheet.
- BSP kd-tree: 117 nodes / 59 leaves; split planes X `[684.1..782.5]`, Z
  `[940.4..1873.2]`; all 216 type-0 placed objects insert into the tree, so the
  long-Z BSP span is static-object/scenery placement rather than missing
  drivable surface.
- Route-surface audit: all 59 JUNC nodes, all 275 sampled RSEG edge points, and
  all 2152 sampled AIMP leaf centers are inside detailed ZMAP chunks. The Ski
  Resort route height span is about 17.1 m on RSEG edges and about 18.1 m across
  AIMP leaf centers, so the runtime gameplay surface is not only the starting
  patch and is not supplied by the far-Z scenery objects.

## OilField Audit

OilField ZMAP detail footprint:

- chunk X `[13..15]`, chunk Z `[18..19]`
- cell X `[832..1024]`, cell Z `[1152..1280]`

OilField main XOBF BIN, with HEAD/slot display Y using the same post-GTE scale
as X/Z and secondary
`0xcxxx` groups included:

- X `[781.2..1002.4]`
- Z `[1119.0..1282.7]`
- upward driveable-candidate triangles span X `[800.4..1002.4]`,
  Z `[1119.0..1282.7]`, GL Y about `[-117..+55]`

The previous host filter kept only triangles in GL Y `[-28..-17]`, which
discarded most of the sloped terrain.
