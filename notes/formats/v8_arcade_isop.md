# Vigilante 8 Arcade ISOP model format

## Status

Static extraction is working for all ten vehicle bundles in the supplied Xbox
360 Arcade package. No game execution is required. The complete Blender
implementation is `tools/v8_arcade/extract_vehicle_blends.py`; the older
position-only OBJ path remains in `extract_vehicle_lods.py` for inspection.

The supplied executable identifies itself as an Isopod Labs release build via
the embedded path:

```text
c:\Dev\IsopodBuild\V8Arcade\X360\Release\V8arcade.pdb
```

ReXGlue 0.8.0 discovered and translated 8,038 functions from `default.xex`.
The generated sources and memory-image dump live under `V8_ARCADE_WORK/`.

## Containers

### IBZ — CONFIRMED

```text
0x00  uint32_le  uncompressed_size
0x04  byte[]     zlib stream
```

The decompressed output size must equal `uncompressed_size` and the output
begins with ASCII `ISOP`.

### ISOP image — PARTIAL

```text
0x00  char[4]    "ISOP"
0x04  uint32_le  content hash or identifier (purpose not proven)
0x08  uint32_le  relocatable memory-image limit
0x0C  ...        remaining header fields not yet named
```

The serialized object graph occupies the region below the limit at `+0x08`.
Bulk shader/texture data may follow it.

Primitive geometry data is Xbox-native big-endian. Serialized pointer fields
are little-endian file offsets. Array allocation counts appear in the four
bytes immediately before the pointed-to element data.

## Reflected layouts

The executable registers these layouts in `sub_820D2EF0` (`Model3`) and
`sub_820D2A58` (`SubMesh`). Field names are literal strings from the XEX.

### Model3 — HIGH confidence

```c
typedef struct Model3 {
    float minext[3];          /* +0x00, big-endian */
    float maxext[3];          /* +0x0C, big-endian */
    SubMesh *submeshes;       /* +0x18, little-endian file offset */
} Model3;                     /* reflected size: 0x1C */
```

In the serialized vehicle images, a four-byte submesh count follows `Model3`
and the `submeshes` pointer targets the immediately following array:

```text
model + 0x1C  uint32_le  submesh_count
model + 0x20  SubMesh[]  submeshes
```

### SubMesh — HIGH confidence

```c
typedef struct SubMesh {
    float minext[3];          /* +0x00, big-endian */
    float maxext[3];          /* +0x0C, big-endian */
    uint32_t flags;           /* +0x18, big-endian */
    void *technique;          /* +0x1C, little-endian file offset */
    void *parameters;         /* +0x20, little-endian file offset */
    VertexArray *verts;       /* +0x24, little-endian file offset */
    uint16_t *indices;        /* +0x28, little-endian file offset */
} SubMesh;                    /* reflected size: 0x2C */
```

The two previously unknown reflected names at `+0x18` and `+0x1C` were
recovered from XEX addresses `0x8200178C` (`flags`) and `0x82001AF0`
(`technique`).

### Vertex and index streams — HIGH confidence for positions/faces

The `verts` reference points to:

```c
typedef struct VertexArray {
    void *data;               /* uint32_le file offset */
    uint32_t vertex_count;    /* little-endian */
} VertexArray;
```

Vertex data begins with three big-endian floats (`x`, `y`, `z`). The remaining
attributes depend on shader technique. Observed strides are 12, 20, 24, 32,
44, 48, 60, and 92 bytes.

For an inline vertex allocation, the vertex bytes end immediately before the
index allocation count, making the stride recoverable without interpreting
the vertex declaration:

```text
stride = (indices_offset - 4 - vertex_data_offset) / vertex_count
```

`indices_offset - 4` contains a little-endian triangle count.
`indices_offset` contains `triangle_count * 3` big-endian `uint16_t` indices.
All indices are validated against the associated vertex count before export.

Skin variants commonly reuse a previously serialized `VertexArray` while
owning distinct index and material allocations. These submeshes do not place
their index data immediately after the shared vertices. The scanner therefore
performs a second pass: it transfers the proven stride from an inline
submesh with the exact same `VertexArray` reference, then independently
validates the variant's indices. Supporting shared arrays increased the
complete vehicle inventory from the provisional 315 objects to 678.

### Scene and Node — HIGH confidence

The per-image scene table begins at file offset `0xC0`:

```text
0xC0  uint32_le  scene_count
0xC4  SceneEntry scene[scene_count]  /* stride 0x10 */

SceneEntry:
  +0x00  Scene *scene
  +0x04  float skin_r                /* big-endian */
  +0x08  float skin_g                /* big-endian */
  +0x0C  float skin_b                /* big-endian */
```

A reflected `Scene` contains its root-node pointer at `+0x00`. A `Node`
contains a 48-byte affine transform followed by its child reference at
`+0x30`. The transform is twelve big-endian floats: a 3x3 basis in values
0–8 and translation in values 9–11.

The RGB field was provisionally named ambient color. It is now confirmed as
the selectable paint/skin color: for every native scene, it equals the main
material's decoded `paintColor0`.

Recovered node types reference child nodes, `Model3` objects, or arrays of
those references. Traversal recreates the authored part placement. Every
valid `Model3` allocation that is not reached through a known node type is
retained in the vehicle's `auxiliary_unresolved` Blender file.

### Vertex declarations — MEDIUM/HIGH confidence

Compiled Xbox vertex shaders begin with the `0x102A11xx` container signature
and embed their input semantic table. Observed semantics include `POSITION`,
`TEXCOORD`, `NORMAL`, and `TANGENT`. The game patches Xenos fetch format
fields at runtime, so the exporter combines the recovered shader semantics
with the exact stream stride to select the practical attribute layout.

The input semantic table is in fetch/result-register order, not serialized
memory order. Treating it as sequential caused the former UV fan: `NORMAL.xy`
at byte 12 was being read as UV. The exact observed big-endian float layouts
are:

| Stride | Serialized fields |
|---:|---|
| 12 | `POSITION float3 @ 0` |
| 20 | `POSITION @ 0`, `TEXCOORD0 float2 @ 12` |
| 24 | `POSITION @ 0`, then `NORMAL float3 @ 12` or `TEXCOORD0 float3 @ 12` |
| 32 | `POSITION @ 0`, `NORMAL @ 12`, `TEXCOORD0 float2 @ 24` |
| 44 | `POSITION @ 0`, `NORMAL @ 12`, `TEXCOORD1 float3 @ 24`, `TEXCOORD0 float2 @ 36` |
| 48 | `POSITION @ 0`, `NORMAL @ 12`, `TEXCOORD0 float2 @ 24`, `TANGENT0 float4 @ 32` |
| 60 | `POSITION @ 0`, `NORMAL @ 12`, `TEXCOORD1 float3 @ 24`, `TEXCOORD0 float2 @ 36`, `TANGENT0 float4 @ 44` |
| 92 | `POSITION @ 0`, `NORMAL @ 12`, `TEXCOORD3 float3 @ 24`, `TEXCOORD0/1/2 float2 @ 36/44/52`, `TANGENT0/1 float4 @ 60/76` |

Positions, faces, UVs, and normals are validated against stream bounds.
Normals are preserved as the point-domain Blender `NORMAL_0` attribute and
UVs as `TEXCOORD_0`. A few untextured PS1-classic submeshes deliberately fill
their unused UV slot with `0x7FFFFFFF`/`0xFFC00000` NaN sentinels. Because
their decoded material has no sampler, the exporter replaces only those
non-consumed values with `(0,0)` and records the count on the mesh.

### Compiled shader constants — HIGH confidence

The compiled Xbox shader containers embed a D3DX constant table at blob
offset `+0x28`. Its 20-byte constant records recover parameter names,
register sets and indices, type rows/columns/elements, and default offsets.
Material parameter values are packed in declaration order: texture pointers
are little-endian and float constants are big-endian.

For the native vehicle paint technique, the pixel-shader sampler registers
are `_shadowmap` (`tf0`), `_envmap` (`tf1`), `_microflakemap` (`tf2`),
`diffuse_samp` (`tf3`), and `normal_samp` (`tf4`). Pixel-shader disassembly
also proves the body-color operation:

```text
lerp(paintColor0, diffuse.rgb, diffuse.a)
```

The Blender extraction graph deliberately does not reproduce that runtime
paint operation. It connects only `diffuse_samp` RGB to an opaque matte Base
Color. Diffuse alpha is a paint mask rather than general mesh opacity; wiring
it to Blender transparency caused black dither noise. `paintColor0`,
normal/detail, gloss, clearcoat, environment, PCF shadow, and microflake
resources remain packed and labeled as unconnected data for later renderer
work.

### Xbox360Texture — HIGH confidence

The reflected `Xbox360Texture` payload is 52 bytes. Its six-dword big-endian
Xenos texture fetch constant begins at payload `+0x1C`. It supplies tiled
layout, pitch, dimensions, format, endian mode, and base/mip addresses.

The little-endian value at ISOP header `+0x08` is both the relocatable object
image limit and the start of physical shader/texture data. Fetch base
addresses are relative to that physical-data offset:

```text
base_file_offset = u32_le(file + 0x08) + fetch.base_address
```

Material parameter blocks contain direct little-endian pointers to
`Xbox360Texture` objects. The exporter decodes Xenos 2D tiling and the
observed formats RGBA8, BC1/DXT1, BC2/DXT3, BC3/DXT5, BC5/DXN,
DXT3A/DXT5A, and CTX1. Every referenced resource becomes a material image
node, is written to PNG, and is packed into the `.blend`.

## Vehicle extraction result

The complete shared-vertex-aware scan yields:

| Vehicle | Model3 objects | Native skins | Native triangles per skin | PS1 classic |
|---|---:|---:|---:|---:|
| FartyDog | 11 | 1 | 10,702 | no |
| GrooVan | 81 | 5 | 20,531 | 260 |
| Incarcerator | 57 | 5 | 16,539 | 356 |
| Jefferson | 87 | 5 | 11,322 | 256 |
| Leprechaun | 97 | 5 | 11,233 | 212 |
| Mammoth | 98 | 5 | 22,341 | 448 |
| Manta | 92 | 5 | 13,061 | 222 |
| Piranha | 69 | 5 | 14,468 | 203 |
| Saucer | 14 | 5 | 6,422 | no |
| Stag | 72 | 5 | 12,884 | 250 |

Eight standard vehicle bundles also contain a consistent late-image cluster of
vehicle-scale models with roughly 90–250 triangles (Mammoth reaches 248).
The user identified these as the unlockable PS1 bonus vehicles, not Xbox 360
distance LODs. They are now classified as `probable_ps1_bonus_models`.

The five native scenes have identical model-instance and triangle counts
within each vehicle. Their scene RGB values and decoded `paintColor0`
constants change together. They are therefore selectable skin variants, not
reduced meshes or distance LODs. No separate sub-1,000-triangle complete Xbox
360 LOD exists in the recovered data; the sub-1,000 complete scenes are the
PS1 classics.

Tracing the exact Node behind the old numbered candidate 2 display proves
that it is not a distance LOD. Its Node is referenced inside an additional
serialized 13-18-entry damage-part array outside the public Scene table.
Eight standard vehicle bundles contain this pattern. Most have five
skin-local arrays; Incarcerator, Mammoth, and Piranha reuse one or more arrays
across multiple skins.

The exporter reconstructs each complete secondary array as
`XBOX360_DAMAGE_ASSEMBLY`, including the wreck body and its matching damaged
or detached panels. The assemblies range from 4,308 to 7,454 triangles. Their
missing/displaced panels, shared generic grime/damage material, and lack of
`paintColor0` identify them as destruction states. Static XEX inspection
registers only `minext`, `maxext`, and `submeshes` on the 28-byte serialized
`Model3`; it exposes no LOD pointer or chain, and game-owned LOD metadata was
not found. Saucer's separate reduced/broken hull is likewise damage geometry.

## Blender extraction result

The definitive export contains 64 separate `.blend` files:

- 46 native Xbox 360 skin files (five each except FartyDog's one);
- 8 PS1 classic files;
- 10 auxiliary/unresolved files.

A purpose-built `vehicle_lods_only` handoff omits the full native assemblies
entirely. It contains only eight separate complete PS1 classic scene files
(203-448 triangles). The former ten one-object Xbox 360 candidates were
retracted because they are incomplete wreck bodies, not distance LODs.

Each skin or classic opens directly as one visible `VEHICLE_ASSEMBLY`; no
collection switching is required. Standard Xbox 360 skin files also carry a
hidden `XBOX360_DAMAGE_ASSEMBLY` collection for isolated inspection. All referenced
textures are decoded and packed. The union of scene-referenced, secondary
array, and auxiliary model offsets equals all 678 discovered `Model3`
allocations.

Part names are useful inferred labels based on bounds, world placement, and
instance reuse. They include confidence and evidence custom properties
because the stripped release bundles contain no authoritative authored part
strings. Source offsets and node paths remain attached so every inference can
be audited or renamed later without losing binary identity.
