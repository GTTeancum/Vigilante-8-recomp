# V8:2 Dreamcast water renderer — direct reconstruction

Source image: `artifacts/dreamcast-render-distance-reference/1ST_READ.BIN`

SHA-256: `06F5A6FCDC9F8E0EF486A1CE1DF7FCF61F5A756DBEF7248139291AE520E3D3E7`

This document distinguishes byte-proven behavior from hypotheses. A behavior
is not accepted merely because it resembles the PS1 port or improves a
screenshot. Addresses below are SH-4 addresses in the retail US Dreamcast
executable; delayed branch/call instructions include their delay slots.

## Complete binary boundaries

| Routine | Contiguous interval | SHA-256 |
| --- | --- | --- |
| Untextured pass | `8C0D72A0..8C0D7569` (`0x2CA` bytes) | `5C3D621487FD616B4DD356B645129320C52E1543DCD4FC7DD1B3A5F7100B48B9` |
| Textured XWAT pass | `8C0D75C0..8C0D82F7` (`0xD38` bytes) | `B542EB65BBB1AF1C69D4BA66BEC5E27F176E7499E65B7D8D496B58E466B14D77` |
| Dispatcher | `8C0D8320..8C0D837B` (`0x5C` bytes) | `A0CEEC021E47328778D4C1D02839CE5B6ED2CFED5403A170F86B08BCA097CCFF` |

The intervals include branch-skipped literal pools. The Ghidra function-body
sizes are smaller because data pools are not instructions. The complete hashes
are enforced by `tools/dreamcast_water_contract.py`.

## Dispatcher `8C0D8320`

The dispatcher copies three camera-matrix components into a local plane vector
(`8C0D8326..833E`), subtracts the authored water height at `8C28892C` from the
camera translation (`8C0D8340..8348`), and rejects a surface outside the exact
60-unit vertical band (`8C0D834A..8350`). It then applies the orientation gate
using exact `0.4` (`8C0D8352..8362`). When accepted it calls the untextured pass
first (`8C0D8364..836A`) and the textured pass second (`8C0D836C..8372`) with
the same plane vector and height delta. There is no alternate call order.

## Untextured pass `8C0D72A0`

### Inputs and fixed state

- `r4` points to four floats `(plane_x, plane_y, plane_z, plane_w)`, copied to
  the local stack at `8C0D72EA..7300`.
- `fr4` is the camera/water height delta, retained in `fr15` at `8C0D72BA`.
- View values come from the table beginning at `8C2747C4`: width, height,
  projection scale, centre X, centre Y, left, and top.
- `8C0D72CE..72E8` constructs vertex ARGB exactly as
  `0xB0RRGGBB` from bytes `8C28EBF0..8C28EBF2`.
- `8C0D7302..7354` reserves the PVR submission, copies the compiled material
  header from context `8C243900`, and aligns the store-queue write cursor.

### Exact four-vertex stream

The axis decision is `abs(plane_y) > abs(plane_x)` at
`8C0D7356..7364`. Every nonterminal vertex command is `0xE0000000`; the fourth
is `0xF0000000`. Each 32-byte vertex stores command, screen X, screen Y,
reciprocal depth, and ARGB at offsets `00,04,08,0C,18`.

For the width-spanning branch (`8C0D736A..744A`), with `t = fr4`:

```
y_far_left  = center_y + (41.666668*t + 0.5*plane_x*width
                            - plane_z*projection) / plane_y
y_far_right = center_y + (41.666668*t - 0.5*plane_x*width
                            - plane_z*projection) / plane_y
y_near_left = center_y + (7.8124995*t + 0.5*plane_x*width
                            - plane_z*projection) / plane_y
y_near_right= center_y + (7.8124995*t - 0.5*plane_x*width
                            - plane_z*projection) / plane_y

V0 = (left,         y_far_left,   37.5)
V1 = (left + width, y_far_right,  37.5)
V2 = (left,         y_near_left,  7.0312495)
V3 = (left + width, y_near_right, 7.0312495)
```

For the height-spanning branch (`8C0D7460..753E`):

```
x_far_top    = center_x + (41.666668*t + 0.5*plane_y*height
                            - plane_z*projection) / plane_x
x_far_bottom = center_x + (41.666668*t - 0.5*plane_y*height
                            - plane_z*projection) / plane_x
x_near_top   = center_x + (7.8124995*t + 0.5*plane_y*height
                            - plane_z*projection) / plane_x
x_near_bottom= center_x + (7.8124995*t - 0.5*plane_y*height
                            - plane_z*projection) / plane_x

V0 = (x_far_top,     top,          37.5)
V1 = (x_far_bottom,  top + height, 37.5)
V2 = (x_near_top,    top,          7.0312495)
V3 = (x_near_bottom, top + height, 7.0312495)
```

The two branches converge at `8C0D7540`; `8C0D7540..754C` advances the PVR
write cursor by the exact emitted byte count. This is a direct translation of
the complete pass, not an inferred screen-space approximation.

## Textured XWAT pass `8C0D75C0`

Reconstruction status: **complete at the Dreamcast world/grid/PVR boundary;
PC coordinate-seam translation is not yet implemented**.

The following reconstruction comes from the SH-4 instructions and their
literal pools. The PS1 routine was used only to suggest names after the
Dreamcast operations had been independently recovered.

### Input, matrix, and viewport (`8C0D75C0..764E`)

- `r4` points to `(plane_x, plane_y, plane_z, plane_w)` and `fr4` is the
  camera/water height delta. The function copies the plane to `S+00..0C` and
  retains the height delta in `fr12` (`8C0D75E0`, `8C0D7620..7636`).
- It copies width, height, and projection from `8C2747C4..8C2747CC` to
  `S+90..98` (`8C0D75E2..75F8`).
- It loads the complete transform at `8C2B2800` into `xmtrx`
  (`8C0D75FA..761E`). Helper `8C0D6E40` appends `w=1`, executes
  `ftrv xmtrx,fv4`, and returns transformed X/Y/Z; it is not a guessed PC
  projection.
- The primary construction branch is selected by the exact test
  `plane_z > 0.9` (`8C0D7640..764E`).

### Four world-space boundary points (`8C0D7652..79C8`)

For `plane_z > 0.9`, let `W`, `H`, and `P` be the copied viewport values,
`T` the height delta, and `n=(nx,ny,nz)` the plane. Four camera-space points
are formed and passed through `8C0D6E40`:

```
for sx,sy in (-1,-1), (+1,-1), (-1,+1), (+1,+1):
    z = T*P / (nz*P + 0.5*sx*nx*W + 0.5*sy*ny*H)
    q = (sx*W*z/P, sy*H*z/P, z)
    boundary[sx,sy] = transform_point(q)
```

The four denominators and signed coordinates are emitted literally in the
four blocks `8C0D7652..76AA`, `76AC..76F2`, `76F4..773A`, and
`773C..7788`; this formula is a compact transcription of those blocks.

When `plane_z <= 0.9`, the function avoids an unstable solve by selecting the
larger horizontal normal component (`abs(ny) > abs(nx)` at
`8C0D77C4..77D4`):

```
near = 0.001 * W
far  = 0.012 * W
near_z = 0.002 * P
far_z  = 0.024 * P

if abs(ny) > abs(nx):
    for x,z in (-near,near_z), (+near,near_z),
               (-far, far_z),  (+far, far_z):
        y = (T - nx*x - nz*z) / ny
        boundary[] = transform_point(x,y,z)
else:
    for y,z in (-near,near_z), (+near,near_z),
               (-far, far_z),  (+far, far_z):
        x = (T - ny*y - nz*z) / nx
        boundary[] = transform_point(x,y,z)
```

The constants are the exact floats `-0.001`, `+0.001`, `-0.012`, `+0.012`,
`0.002`, and `0.024` in the pools at `8C0D78C4..78DC` and
`8C0D7AB4..7ACC`. The `mov.w` stack displacements at `8C0D77DE` and
`8C0D77EC` prove these multiply saved width (`S+90`) and projection
(`S+98`) respectively; height is not used by this fallback branch. The two
solve branches are
`8C0D77D6..78B4` and `8C0D78E0..79C8`.

### Grid bounds (`8C0D79CE..7C2E`)

Only transformed X and Z are used. Across all four boundary points the
function computes:

```
min_x = floorf(min(boundary[i].x))
min_z = floorf(min(boundary[i].z))
max_x = ceilf (max(boundary[i].x))
max_z = ceilf (max(boundary[i].z))
```

`8C0F681C` is the retail `floorf` wrapper and `8C0F677C` is the retail
`ceilf` wrapper; their sign/fraction corrections were verified through their
callees `8C0F7BEC` and `8C0F7B8C`. `max_z` is clamped down to the live limit
at `8C2748A4`; the pass exits when the clamped bound is not greater than
`min_z` (`8C0D7B9E..7BB2`).

Each span is then capped to 31, not 32:

```
if int(max_x - min_x) > 31: max_x = min_x + 31.0
if int(max_z - min_z) > 31: max_z = min_z + 31.0
```

This yields at most 32 grid vertices on an axis and at most 31 cell strips.
The exact cap blocks are `8C0D7BB4..7C04` and `8C0D7C08..7C2E`.

### Terrain classification (`8C0D7C30..7D16`)

The inclusive X/Z grid is classified into bytes at `8C2BA940`, with a
32-byte row stride. The chunk lookup performs signed floor division by 64,
uses the terrain table at `8C28D160`, a `0xAC` chunk-record stride, and a
20-byte height-record stride. The byte is exactly:

```
classified[row][column] = (water_plane * 32.0f) > terrain_height;
```

This is proven by the lookup at `8C0D7CB0..7CF2`, compare at `7CF4`, and
byte store at `7CFC`. No shoreline polygon is authored here.

### Two displacement fields (`8C0D7D18..7F3C`)

The displacement grid begins at `8C2BAD40`, with 32 floats (128 bytes) per
row. Both loops include both bounds. `phase_ticks(period)` is the signed
integer result of the retail magic-multiply sequence, equivalent to the
fixed-angle advance `tick * 65536 / period` with the original truncation.
`sin16` is helper `8C0D7260`, which truncates to 16 bits and executes SH-4
`fsca`.

```
for row = 0 .. int(max_z-min_z):
    py_a = phase_ticks(240) + (min_z + row) * 13981.014f
    py_b = phase_ticks(420) + (min_z + row) * 23301.69f
    for col = 0 .. int(max_x-min_x):
        px_a = phase_ticks(240) + (min_x + col) * 13981.014f
        px_b = phase_ticks(420) + (min_x + col) * 23301.69f
        displacement[row][col] =
            sin16(px_a) * sin16(py_a) * 0.15625f +
            sin16(px_b) * sin16(py_b) * 0.03125f
```

The first field is stored at `8C0D7E12`; the second is fused into the stored
value at `8C0D7EF8..7F00`. Their exact tick divisors are represented by
`0x88888889`/shift 7 and `0x9C09C09D`/shift 8. The corresponding wavelengths
are 150 and 90 world units because the phase constants advance over 32-unit
terrain cells.

Before calculating the first field at a grid point, the routine ANDs the four
adjacent classification bytes. If all four are nonzero it ORs `0x80` into
the upper-left byte (`8C0D7DE2..7DFC`). The textured pass does not read that
bit during its own PVR emission; the Dreamcast surface is submitted as a
continuous rectangle and terrain/depth composition supplies the visible
shoreline. Treating the byte as an instruction to delete water triangles is
therefore not equivalent.

### Material and projection (`8C0D7F3E..7FBC`, helper `8C0D70A0`)

The function compiles/reserves context `8C2439C0`, copies its PVR material
header into the store queue, and begins 32-byte vertices after the aligned
header. The material words are the separately hash-guarded XWAT contract;
face culling is disabled and the vertex color is exactly `0x80FFFFFF`.

For every grid vertex, helper `8C0D70A0` transforms `(x,y,z,1)` with the
current `xmtrx`, clamps view Z to at least `0.001`, and returns:

```
screen_x = center_x + view_x * projection / max(view_z, 0.001)
screen_y = center_y + view_y * projection / max(view_z, 0.001)
depth    = projection / max(view_z, 0.001)
view_z   = the unclamped transformed Z
```

The caller stores `depth * 0.9f` in the PVR vertex (`8C0D8136..8142` and
`8C0D823A..8242`). There is no OT-bucket depth in this pass.

### Exact strip and UV stream (`8C0D7FBE..82D4`)

Let `R=int(max_z-min_z)` and `C=int(max_x-min_x)`. If `R <= 0`, no surface
strip is emitted. Otherwise the function emits `R` independent row strips;
each strip contains `2*(C+1)` alternating vertices:

```
for row = 0 .. R-1:
    for col = 0 .. C:
        emit(row,   col, E0000000)
        emit(row+1, col, col == C ? F0000000 : E0000000)
```

The first command is written at `8C0D80D6..80D8`. The second command choice
is `8C0D81BC..8222`. The column and row transitions are
`8C0D828A..82A8` and `8C0D82AA..82C2`. Thus the terminal bit closes every
row strip, not only the final grid row.

For vertex `(r,c)`:

```
world = (min_x+c, water_plane+displacement[r][c], min_z+r)
u = 2.6666667f * abs(displacement[r][c+1] - displacement[r][c-1])
v = 2.6666667f * abs(displacement[r+1][c] - displacement[r-1][c])
```

The alternating `(r+1,c)` vertex uses the same centered differences around
its own row:

```
u = 2.6666667f * abs(displacement[r+1][c+1]
                     - displacement[r+1][c-1])
v = 2.6666667f * abs(displacement[r+2][c]
                     - displacement[r][c])
```

These are slope-derived environment coordinates, not ordinary tiled world
UVs. The first pair is recovered at `8C0D814C..816C`; the second at
`8C0D824C..826E`. The code deliberately addresses the padded rows/columns
surrounding `8C2BAD40`; replacing those centered differences with forward
differences changes the surface.

Every vertex is exactly:

```
00 command, 04 screen_x, 08 screen_y, 0C 0.9*projection/view_z,
10 u,       14 v,        18 0x80FFFFFF, 1C unused/compiled state
```

After all strips, `8C0D82C4..82D4` advances the owning PVR submission cursor
using the masked store-queue address. This is one function-owned submission,
not a set of independently sortable PS1 water polygons.

## Consequence for the PC renderer

The Louisiana result disproves the earlier strategy of retaining the native
PS1 water packet mesh while translating only Dreamcast material state. A
direct implementation must reproduce the Dreamcast continuous row-strip
surface, its exact grid bounds, centered-difference UVs, continuous projected
depth, and terrain depth intersection through one shared Enhanced path. The
remaining engineering question is the measured PS1-to-Dreamcast coordinate
seam; it must be derived from live/native game values and cannot be filled by
visual tuning.

## Direct PS1/Dreamcast coordinate seam

The remaining coordinate seam is now measured rather than inferred.  The
Dreamcast viewport initializer at `8C0A6320` stores its integer arguments as
the floating viewport state at `8C2747C4..8C2747E4`.  Its normal gameplay
callers load `r4=640`, `r5=480`, `r6=320`, and `r7=240`; therefore the retail
gameplay viewport is exactly 640x480 with center (320,240).  Initializer
`8C08AFC0` writes the exact float `512.0` to projection state `8C2747CC`.

The corresponding PS1 water path was traced at the native
`func_80016664 -> func_80024008` seam without changing emulated state.  The
camera plane normal is signed 12-bit fixed (`/4096`), world coordinates and
camera translation are signed 16.16 fixed (`/65536`), and the installed GTE
camera basis is signed 12-bit fixed (`/4096`).  For every traced point, the
native helper is reproduced exactly by:

```
world = camera_translation / 65536
      + (camera_basis / 4096) * (local_ray / 65536)
```

At Louisiana tick 1 the installed basis was:

```
-2255  -414  3394
    0  4066   495
-3420   272 -2239
```

and translation was `(57564424,2954907,82733044)`.  Applying that equation
to input `(-54656,130609,65536)` produces
`(57635616,3092479,82751528)`, the native helper's exact output.  The other
three boundary points and subsequent frames obey the same equation.  The
output divided by 65536 is already in the terrain-grid coordinate system used
by Dreamcast XWAT; equivalently one terrain-grid unit is 256 PS1 render units.
There is no additional axis swap or empirically chosen scale.

Enhanced widescreen changes only the horizontal logical viewport.  The live
PS1 trace was 427x240 at projection 256, which maps to 854x480 at the exact
Dreamcast projection 512 while retaining the Dreamcast 480-line vertical
field.  The 4:3 case remains the retail 640x480 tuple.  This preserves the
existing widescreen draw extension without changing the recovered water-grid
world-space algorithm.

Trace build SHA-256 was
`20B583EBED293F0C74D01FC643F3A48D76909575F6699A8BE67474A18778FFF0`.
The hidden, silent, one-process Enhanced-only evidence is retained under
`artifacts/dreamcast-render-full-re/ps1-water-camera-installed-louisiana`.
It was a trace probe, not a visual or performance acceptance run, and was not
deployed.
