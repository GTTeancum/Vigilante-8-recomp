# V8:2 Widescreen Terrain Completeness Investigation

This file is the canonical, append-only investigation record for the missing
terrain visible at the outer edges of 16:9 gameplay. Read it before changing
or testing the terrain renderer. Do not repeat a rejected experiment unless
new evidence changes its premise.

## Scope and pass criteria

- The defect is in **16:9 gameplay**. A 4:3 run is only a control.
- Both outer edges must remain continuously filled while the camera moves and
  turns. A single still image is not sufficient.
- Proof must use full-resolution 16:9 presentation captures. Downscaled contact
  sheets can hide thin holes.
- Final verification must cover Route 66 plus at least one other arena with a
  materially different terrain layout.
- The accepted HUD, texture reconstruction, and game-over text work are frozen
  and are not part of this investigation.

## Confirmed symptom

- Route 66 produces sky/background-coloured holes and thin missing strips at
  the outer edge of the 16:9 view.
- The holes move with the camera and expose the background through missing
  world terrain.
- Representative capture:
  `artifacts/terrain-completeness/route66-gap-probe-details/gameplay.bmp`

## Native call path established

The terrain path is:

`func_8001C134 -> func_8001C158 -> func_8001BECC ->
func_8001BE68 -> func_800288E0`

- `func_8001C158` constructs the visible terrain traversal polygon.
- `func_8001BECC` walks the polygon and selects terrain cells.
- `func_8001BE68` submits selected cells.
- `func_800288E0` transforms terrain and writes GPU packets.
- `func_800290A8` performs native packet-level screen clipping and emits
  subdivided terrain primitives.

## Experiments already completed

### T01 — 4:3 control

- Change: Disabled widescreen and captured Route 66.
- Result: The control is clean.
- Evidence:
  `artifacts/terrain-completeness/route66-four-three-valid-control/contact.bmp`
- Conclusion: The arena data is present. This does **not** prove a widescreen
  fix; it only isolates the regression to the widened view.
- Status: Control retained; do not present as the fix.

### T02 — Enhanced depth buffer disabled

- Change: Ran the same widescreen scene without the Enhanced depth path.
- Result: Edge holes remained.
- Evidence:
  `artifacts/terrain-completeness/route66-depth-disabled-control`
- Conclusion: The holes are not caused by depth testing or depth precision.
- Status: Rejected as root cause.

### T03 — Oversized-triangle rejection audit

- Change: Counted Enhanced renderer triangle-size rejections.
- Result: Zero relevant world triangles were rejected while the hole was
  visible.
- Conclusion: The Enhanced backend's large-triangle safety rejector is not
  deleting this terrain.
- Status: Rejected as root cause.

### T04 — Widen native terrain traversal by output aspect

- Change: Temporarily scale `gp+0xEDC`, the horizontal extent consumed by
  `func_8001C158`, from 320 to 427 for 16:9.
- Implementation:
  `V82Compat.ExpandTerrainFrustum` and `V82Compat.RestoreTerrainFrustum`.
- Result: Considerably more edge terrain appears than with the native value,
  but holes remain.
- Evidence:
  `artifacts/terrain-completeness/route66-frustum-candidate-1`
- Conclusion: Native traversal width is a real part of the defect, but not the
  only boundary involved.
- Status: Necessary partial fix; retained.

### T05 — Native-width negative control

- Change: Set traversal scale to 1.0.
- Result: The widescreen holes became dramatically worse.
- Evidence:
  `artifacts/terrain-completeness/route66-native-frustum-control/final.bmp`
- Conclusion: Widescreen terrain traversal must be expanded.
- Status: Confirms T04.

### T06 — Larger traversal scales

- Change: Tested scales 1.5, 2.0, and 4.0.
- Results:
  - 1.5: holes remain.
  - 2.0: holes remain.
  - 4.0: large wedges become worse.
- Evidence:
  `route66-frustum-scale-1_5`, `route66-frustum-scale-2`, and
  `route66-frustum-scale-4` under `artifacts/terrain-completeness`.
- Conclusion: This is not safely fixed by continually inflating one scalar.
  The traversal polygon changes non-linearly because it is clipped and
  reconstructed by the native code.
- Status: Rejected as a production fix.

### T07 — Terrain traversal polygon trace

- Change: Added `RECOMPONE_V82_TRACE_TERRAIN_TRAVERSAL=1` and logged the
  generated world-space polygon.
- Result: Scale changes alter the polygon bounds and vertices. The hook is
  active and affects the intended native function.
- Evidence:
  `route66-polygon-trace-default`,
  `route66-polygon-trace-default-2`, and
  `route66-polygon-trace-scale4`.
- Conclusion: The failure of larger scales is not a dead hook.
- Status: Diagnostic retained.

### T08 — Separate traversal width from packet clip width

- Finding: At the end of `func_8001C158`, `gp+0xEDC` is copied to scratchpad
  `+0x94`. `func_800290A8` uses scratchpad `+0x94` as its native 0..width
  packet clip. Enhanced widescreen already compresses packed GTE screen X into
  the native 0..320 range for native clipping.
- Change: Keep 427 only while constructing the traversal polygon, then restore
  320 immediately before address `0x8001C89C` copies it to the scratchpad.
- Result: Architecturally correct separation, but a thin/large left-edge hole
  remains at both 1.3333 and 2.0 traversal scales.
- Evidence:
  `route66-split-frustum-clip-candidate` and
  `route66-split-frustum-scale2`.
- Conclusion: Packet clipping no longer incorrectly uses 427, but another
  visibility boundary remains.
- Status: Retained.

### T09 — Terrain route packet provenance

- Change: Mark packet writes made inside `func_800288E0` as terrain-route
  packets, using the same source-provenance mechanism proven in the original
  V8 port.
- Result: Corrects packet material ownership but does not restore the missing
  geometry at the probed edge.
- Evidence:
  `artifacts/terrain-completeness/route66-terrain-owner-candidate`
- Conclusion: Material classification was worth correcting, but it is not the
  terrain completeness root cause.
- Status: Retained; no longer treated as the fix.

### T10 — Exact missing-pixel triangle probe

- Change: Probed logical widescreen point `x=-20, y=112`.
- Result: Only a background/UI primitive and a semi-transparent particle cover
  the point. No opaque terrain primitive exists there.
- Evidence:
  `route66-gap-probe-y112` and `route66-gap-probe-details`.
- Conclusion: This is genuinely absent geometry, not terrain hidden by the
  wrong blend/depth state.
- Status: Confirmed.

### T11 — Candidate packet write stack

- Change: Watched writes to candidate packet `0x006C83FC`.
- Result: Its writer stack is
  `func_80020784 -> func_80020894 -> func_80020A80 -> func_8001F3C0 ->
  func_8001FBFC -> LEVEL func_80103F10`.
- Evidence:
  `artifacts/terrain-completeness/route66-gap-packet-write-watch`.
- Conclusion: That candidate is an object/particle packet, not missing terrain.
  Do not pursue it as a terrain writer.
- Status: Rejected lead.

### T12 — Terrain-cell submission and emission metrics

- Change: Added `RECOMPONE_V82_TRACE_TERRAIN_CELLS=1`. For every
  `func_800288E0` call, record cell X/Z, packet start/end, whether packets were
  emitted, per-frame bounds, rejected samples, and per-cell packet ownership.
- Result on moving Route 66 run:
  - Frame 1: 426 cells submitted, 332 emitted, submitted and emitted bounds
    both `X=948..1052, Z=1236..1360`.
  - Subsequent frames show hundreds of moving cell submissions. In nearly all
    sampled frames, the outer submitted bounds also emit at least one packet.
  - The point probe still finds no terrain at the missing edge.
- Evidence:
  `artifacts/terrain-completeness/route66-cell-trace/`
  (`01_c00_00_levels_route66.stderr.log`).
- Conclusion: The entire outermost submitted row/column is not being discarded
  wholesale by `func_800288E0`. The next test must correlate nearby visible
  terrain packets to exact cell owners and determine whether the missing screen
  point lies outside the submitted polygon or inside a cell whose individual
  sub-primitives are clipped.
- Status: Diagnostic retained.

## Current code changes

- `tools/recompone-reference/RecompOne.Runtime/sdk/V82Compat.cs`
  - aspect-based traversal expansion
  - split traversal/packet-clip restore
  - traversal polygon logging
  - terrain-cell submission/emission logging
  - per-cell packet ownership
  - terrain route write provenance
- `tools/recompone-v8-2/prepare_reference.py`
  - native hook definitions
- Generated working tree:
  - `reference-v8-2/generated/v82.recompone.json`
  - `reference-v8-2/generated/recompiled/main.cs`

## Next experiment — do not skip

### T13 — Correlate visible neighbour packets to terrain cells

In progress:

- Missing point `(-20,112)`: no terrain packet.
- Missing point `(-10,120)`: no terrain packet.
- Missing point `(0,112)`: no terrain packet.
- Visible point `(-20,130)`: opaque terrain packet `0x002024E8`, owned by
  `terrain-cell=944,1260,frame=149`. Its triangle spans
  `(-75,130),(-104,136),(-35,128)`.
- At Y=120, X=20 still has no terrain. X=40 and X=60 are covered by
  `terrain-cell=940,1260,frame=149`.
- The X=40 triangle spans source coordinates
  `(-7,120),(-36,121),(3,117)`; after the widescreen margin it spans
  `(47,120),(18,121),(57,117)`.
- Evidence:
  `route66-cell-probe-below`, `route66-cell-probe-right`, and
  `route66-cell-probe-diagonal`, `route66-cell-probe-x20y120`,
  `route66-cell-probe-x40y120`, and `route66-cell-probe-x60y120` under
  `artifacts/terrain-completeness`.

Interpretation:

- Per-cell packet ownership is working.
- The foreground cell at the visible point begins around logical Y=128. It
  cannot cover the missing point at Y=120.
- A measurable seam exists between X=20 and X=40 at Y=120. Cell `940,1260`
  covers the right side; adjacent cell `944,1260` covers lower foreground but
  not this seam.

Next:

1. Dump every primitive owned by cells `940,1260` and `944,1260` at gameplay
   tick 149, including primitives rejected for native overspan.
2. Compare their projected boundaries and packet addresses to determine
   whether an expected joining sub-triangle was never emitted or was rejected
   after packet generation.
3. Determine which of these is true:
   - the required adjacent cell is not submitted by `func_8001BECC`;
   - the cell is submitted but `func_800288E0` emits no packet;
   - the cell emits packets but `func_800290A8` clips the relevant
     sub-primitives;
   - the packets exist but are lost after native packet generation.
4. Instrument only the confirmed boundary from step 3.

### T14 — Cell-specific primitive dump

- Added diagnostics:
  - `RECOMPONE_TRACE_TERRAIN_CELL_TRIANGLES=940,1260;944,1260`
  - `RECOMPONE_TRACE_TERRAIN_CELL_TICKS=149`
- Each matching primitive records packet, ordering-table index, source XY,
  span, depth, GTE correlation, and whether the Enhanced native-overspan rule
  would reject it.
- First run at gameplay tick 149 produced 16 primitives for
  `terrain-cell=940,1260,frame=148`; all have small spans and none hit the
  overspan rejector. They belong to the other native display buffer
  (`target Y=240`, source Y around 313..368), not the frame-149 seam at source
  Y=120.
- Next: Dump tick 150, when the double-buffered frame-149 packets are consumed.
- Evidence:
  `artifacts/terrain-completeness/route66-cell-primitive-dump`.
- Tick 150 produced all 24 primitives for frame 149:
  - cell `940,1260`: 16 triangles spanning the upper/right portion;
  - cell `944,1260`: 8 triangles continuing into the lower/left foreground;
  - every primitive has GTE depth, small screen spans, and `overspan=0`;
  - the two cells join continuously from source Y=113 through Y=136.
- Evidence:
  `artifacts/terrain-completeness/route66-cell-primitive-dump-tick150`.
- Conclusion: The two near cells are valid and continuous. They describe the
  foreground surface beginning around Y=113..128, but they do not explain the
  missing mid/far terrain that should already cover the area above/behind it.
- Status: Complete; native overspan and a near-cell seam are rejected as the
  root cause.

### T15 — Full terrain scanline coverage

- Added `RECOMPONE_TRACE_TERRAIN_SCANLINE=<logical-y>`.
- At the selected gameplay tick, every terrain triangle crossing that Y emits
  its exact draw-space X interval and packet owner.
- This replaces repeated one-pixel runs with a programmatic union of the entire
  terrain scanline. The first target is Y=112 at tick 150.
- Default 16:9 traversal result at Y=112:
  - 86 terrain intersections;
  - their union is one continuous interval, X=`26..380`;
  - the 16:9 draw target is X=`0..428`;
  - therefore terrain generation itself leaves X=`0..26` and X=`380..428`
    uncovered at this scanline.
- The interval is continuous internally. This rules out a missing individual
  cell or sub-triangle at Y=112 and identifies the defect as insufficient
  **outer traversal coverage**, despite the current 320-to-427 input scaling.
- Evidence:
  `artifacts/terrain-completeness/route66-terrain-scanline-y112`.
- Next: Measure the same scanline at traversal scales 1.5 and 2.0. Use the
  resulting coverage function to derive the correct native expansion rather
  than guessing from screenshots.
- Scale measurements:
  - 1.3333: union `26..380`;
  - 1.5: unions `26..380` and `401..415`;
  - 2.0: unions `-44..-44`, `26..380`, and `401..477`.
- Conclusion: Scalar expansion never widens the main `26..380` terrain
  interval. It only admits disconnected outer fragments, which matches the
  observed wedges at high scales. Scalar calibration cannot solve this.
- Status: Complete; further scalar inflation is rejected.

### T16 — Per-row terrain-cell padding

- Native fact: `func_8001BECC` calls `func_8001BE68` once per selected terrain
  row. The latter receives an inclusive/exclusive X-cell span in A0/A1 and
  submits cells in four-unit steps.
- Added diagnostic hook `V82Compat.ExpandTerrainRowSpan` at `0x8001BE68`.
- `RECOMPONE_V82_TERRAIN_ROW_CELL_PADDING=N` adds N real cells to each side of
  every selected row, clamped to the native 0..2048 terrain grid.
- This tests the exact layer identified by T15 without altering projection,
  packet clipping, geometry, or accepted rendering features.
- Next: compare N=1, 2, and 4 using the same Y=112 scanline union.
- Results:
  - N=1: `26..380`, plus disconnected `401..408`;
  - N=2: `26..380`, plus disconnected `401..415`;
  - N=4: `26..380`, plus disconnected `401..446`.
- Conclusion: Row-X padding only admits more right-side fragments. It never
  changes the main interval or fills the left edge. The missing coverage needs
  rows outside the polygon's current Z extent as well as X padding.
- Status: Complete; rejected as the production fix.

### T17 — Two-dimensional traversal-polygon padding

- Added diagnostic hook `V82Compat.ExpandTerrainTraversalPolygon` immediately
  before `func_8001BECC`.
- `RECOMPONE_V82_TERRAIN_POLYGON_PADDING_CELLS=N` expands every final
  world-space X/Z polygon vertex radially from the polygon centroid by
  `N * 1024` world units (one terrain-cell interval).
- Unlike T16 this expands both row spans and the set of selected rows, while
  leaving projection and packet generation untouched.
- Next: compare N=2, 4, and 8 with the Y=112 scanline union. If the main
  interval widens, replace the diagnostic radial rule with a camera-basis
  lateral margin derived from the measured minimum.
- Results:
  - N=2: `26..380`, plus `401..408`;
  - N=4: `26..380`, plus `401..415`;
  - N=8: `26..380`, plus `401..446`.
- Conclusion: Two-dimensional radial padding produces the same result as row
  padding. The central interval is unchanged; extra terrain appears only as a
  separate right-side island. The Y=112 line alone cannot distinguish missing
  culling from the authored terrain silhouette.
- Status: Complete; rejected as the production fix.

### T18 — Whole-frame terrain coverage map

- Extended `RECOMPONE_TRACE_TERRAIN_SCANLINE` to accept an integer range such
  as `70-180`.
- One deterministic frame now yields the union of every terrain triangle on
  every logical gameplay scanline in that range.
- This will identify the exact Y values containing disconnected slivers and
  outer-edge holes, instead of selecting points visually.
- Next: Capture Y=60..200 at tick 150, compute the per-line unions, then target
  the first objectively discontinuous terrain band.
- Added reusable analyzer:
  `tools/recompone-v8-2/analyze_terrain_coverage.py`.
- Default Route 66 frame-149 results:
  - 141 scanlines measured (Y=60..200);
  - 32 have multiple terrain intervals;
  - terrain reaches both full 16:9 edges only around Y=126..128;
  - representative outer coverage:
    - Y=100: `37..429`;
    - Y=112: `26..380`;
    - Y=119: `26.667..374`;
    - Y=125: `1..427.4`;
    - Y=126: `-3.4..435.2`.
- Evidence:
  `artifacts/terrain-completeness/route66-terrain-coverage-map/coverage.json`.
- Interpretation: The visible edge loss is a broad band, not one missing
  triangle. The existing terrain eventually covers the full target lower in
  the frame, while upper/mid rows expose the widened backdrop.
- Next: Use very large final-polygon padding (16 and 32 cells) as a bounded
  negative control. If Y=112 remains unchanged, native polygon/cell culling is
  conclusively not withholding geometry capable of covering that band.
- Final culling controls at Y=112:
  - 16-cell padding:
    `-124.51..-122.6; -83..-38; 26..380; 401..520`;
  - 32-cell padding:
    `-187..380; 401..554`.
- Conclusion: At 32 cells, terrain connects through the full left edge and
  covers the right edge. The level data and packet generator can draw the
  missing widescreen terrain. The final traversal polygon is conclusively
  withholding it at the default margin.
- The remaining internal `380..401` interval at Y=112 is not an outer-edge
  hole; it may be the authored silhouette and will be checked visually only
  after outer coverage is fixed.
- Status: Root cause confirmed.

### T19 — Minimum polygon-margin calibration

- The 32-cell radial diagnostic proves causality but is intentionally
  over-broad.
- Next:
  1. Test 20, 24, and 28 cells at Y=112.
  2. Select the minimum value that connects through X=0 and reaches X=428.
  3. Replace centroid-radial padding with a camera-basis margin that widens the
     left/right frustum boundaries without extending near/far depth.
  4. Re-run the full Y=60..200 coverage map and moving full-resolution
     captures.
- Status: In progress.

Calibration results so far:

- 20 cells: left islands end at `-38`, central terrain begins at `26`;
- 24 cells: left island `-152.5..-38`, central `26..380`;
- 28 cells: left island `-183..-21`, central `26..380`;
- 32 cells: left and central intervals connect.

The threshold lies between 28 and 32 cells. Tests at 30 and 31 cells are next.

- 30 cells: `-183..3; 26..380; 401..554`;
- 31 cells: `-187..19; 26..380; 401..554`;
- 32 cells: left and central terrain connect.

The measured minimum for this frame is exactly 32 radial cells. A fresh
full-resolution capture at 32 cells is required before designing the narrower
camera-basis equivalent.

### T20 — Camera-basis trace for lateral-only expansion

- Purpose: replace the diagnostic 32-cell centroid-radial expansion with a
  fix that widens only the camera's left/right traversal boundaries.
- Instrumentation added to `TraceTerrainTraversalPolygon`:
  - camera translation at `gp+0xF3C`;
  - the packed 3x3 camera matrix at `gp+0xF28`;
  - polygon centroid;
  - an independently inferred horizontal right vector perpendicular to the
    camera-to-polygon-centroid direction.
- This is a read-only trace. No new culling behavior is enabled by it.
- Next:
  1. capture the values on the same deterministic Route 66 frame;
  2. identify which packed matrix row/column agrees with the independently
     inferred right vector;
  3. implement an environment-gated lateral-only polygon expansion;
  4. compare it with the T18 scanline coverage map before making it the
     widescreen default.
- Status: instrumentation added; capture pending.

Capture result:

- `gp+0xF3C` is 24.8 fixed-point camera translation. Dividing X/Z by 256
  yields `243377,324032`, exactly the camera-origin vertex in the traversal
  polygon.
- The polygon centroid is `254840,329175`, so its horizontal forward vector
  is approximately `0.912,0.409`.
- Matrix column 2 is `3709,1664`, normalized to the same forward vector.
- Matrix column 0 is `1677,-3737`, normalized to `0.409,-0.912`, the
  corresponding horizontal right vector.
- Evidence:
  `artifacts/terrain-completeness/route66-camera-basis-trace`.
- Conclusion: the native camera basis is now identified without screenshot
  inference.

Implementation under test:

- Added `ExpandTerrainTraversalLateral`, gated by
  `RECOMPONE_V82_TERRAIN_LATERAL_PADDING_CELLS`.
- It preserves the camera-origin vertex and moves every left/right boundary
  vertex strictly along the normalized camera-right axis according to its
  signed lateral position.
- It does not extend the polygon forward or backward.
- Next: measure 8, 16, 24, and 32 lateral cells at Y=112, then run the full
  scanline map on the minimum value that covers both widescreen edges.
- Status: camera basis confirmed; lateral calibration pending.

Lateral calibration at Y=112:

- 8 cells:
  `-61..-38; 26..380; 401..483`;
- 16 cells:
  `-152.5..-38; 26..380; 401..554`;
- 24 cells:
  `-188..-38; 26..380; 401..554`;
- 32 cells:
  `-188..3; 26..380; 401..554`.

Interpretation:

- The directional expansion restores the complete right edge with only eight
  cells.
- At 32 cells the left island reaches X=3, so the literal X=0 edge is covered,
  but `3..26` is still an internal hole immediately beside it. This is not an
  acceptable visual result and is not being counted as a pass.
- The persistent `380..401` gap is unchanged from the radial control and may
  be the authored horizon silhouette; it must be judged separately from the
  known left-edge hole.
- Next: test 36, 37, and 38 lateral cells to find the exact point where the
  left interval joins the central terrain, then run Y=60..200 at that minimum.
- Status: 32 rejected; narrow calibration continues.

Further calibration:

- 36 cells: `-188..380; 401..554`;
- 37 cells: `-188..380; 401..554`;
- 38 cells: `-188..380; 401..554`.

At 36 cells the known left island has joined the central terrain. Tests at 34
and 35 cells remain necessary to establish the exact minimum rather than
carrying avoidable traversal overhead.

Exact threshold:

- 33 cells: `-188..19; 26..380; 401..554` — rejected because `19..26`
  remains a visible hole;
- 34 cells: `-188..380; 401..554` — the left terrain is continuous;
- 35 cells: same union as 34.

The exact minimum on the deterministic Route 66 frame is 34 lateral cells.
Next: run the complete Y=60..200 scanline map with 34 cells and compare it
programmatically with the T18 default map before any visual acceptance run.

Full Y=60..200 result at 34 cells:

- Evidence:
  `artifacts/terrain-completeness/route66-lateral34-coverage-map/coverage.json`.
- The known Y=112 left-edge hole is closed.
- Many scanlines now cover both edges, including the continuous Y=86..103 and
  Y=106..114 bands.
- The raw analyzer still reports uncovered edge spans in sky/horizon rows and
  near the bottom-right foreground. Its current rule treats every pixel not
  crossed by a terrain triangle as a defect, so those reports cannot by
  themselves distinguish authored sky/silhouette from a traversal omission.
- Next:
  1. capture a fresh full-resolution still at the measured 34-cell value;
  2. compare its edge regions with the 32-cell radial causality control;
  3. run a larger lateral saturation control only if the still shows remaining
     geometric terrain holes;
  4. do not promote the value to the default until a moving two-edge capture
     passes.
- Status: full coverage data captured; visual edge validation pending.

Visual result:

- Fresh 1920x1080 capture:
  `artifacts/terrain-completeness/route66-lateral34-visual/gameplay.bmp`.
- The original large left-side terrain loss is substantially reduced, but
  thin sky-coloured horizontal slits remain across the left/middle ground.
- The Y=60..200 geometry union says terrain triangles cross these rows. The
  remaining slits therefore cannot be solved by blindly expanding the
  traversal polygon again.
- 34 lateral cells is retained as a culling experiment, not yet a production
  default.

### T21 — Terrain watertightness / projection-seam control

- Hypothesis: adjacent terrain packets share exact integer PS1 SXY endpoints,
  but the Enhanced renderer correlates/reconstructs camera-space vertices per
  primitive. A shared terrain endpoint can therefore acquire two slightly
  different sub-pixel positions, opening the horizontal slits seen in the
  fresh capture.
- Added diagnostic
  `RECOMPONE_V82_TERRAIN_PACKET_PROJECTION=1`.
- For `TerrainRoute` only, it:
  - restores the widescreen packet X coordinate from the native compressed
    SXY value;
  - uses stable packet-space XY for raster coverage;
  - preserves each vertex's recovered `PerspectiveW`, so projective texture
    interpolation remains active;
  - leaves all object, vehicle, particle, HUD, and menu projection untouched.
- Next: capture Route 66 with lateral=34 and this control. If the slits close,
  compare exact edge pixels and motion before promoting the terrain-specific
  stable projection path.
- Status: code added; negative-control capture pending.

Control result:

- Evidence:
  `artifacts/terrain-completeness/route66-lateral34-packet-projection/gameplay.bmp`.
- Stable packet-space terrain projection reduces some of the sub-pixel
  breakup, but pale slits still reach in from the left edge. It is therefore
  not a complete fix by itself.
- Next isolation:
  1. combine the stable packet projection with the prior 32-cell radial
     saturation control;
  2. compare the same fixed capture region programmatically;
  3. if the remaining slits close, the lateral-only polygon omitted near/far
     cells; if they remain, packet emission/raster coverage below the walker
     is still incomplete.
- Status: partial effect only; not accepted.

Combined saturation control:

- Evidence:
  `artifacts/terrain-completeness/route66-radial32-packet-projection/gameplay.bmp`.
- Adding the prior 32-cell radial saturation control does not close the pale
  slits. The remaining defect is therefore below traversal/cell selection.
- Status: rejected; do not spend more time increasing the traversal polygon.

### T22 — Exact pale-slit packet probe

- Probe:
  `RECOMPONE_TRACE_TRIANGLE_PROBE=11,107`,
  `RECOMPONE_TRACE_TRIANGLE_LABELS=gameplay`, lateral padding 34, stable
  terrain packet projection enabled.
- Evidence:
  `artifacts/terrain-completeness/route66-hole-probe-x11y107/01_c00_00_levels_route66.stderr.log`.
- The probe point is not devoid of terrain packets. It receives multiple
  `TerrainRoute` packets.
- Several far-terrain packets collapse to a horizontal line after native SXY
  integer quantization:
  - packet `0x00208B04` has all three Y coordinates at 107 and `area2=0`;
  - other nearby untextured far-terrain packets show the same pattern;
  - their RGB is `(254,212,153)`, matching the pale fog/backdrop colour.
- Textured non-degenerate terrain triangles also cross the local region:
  - packet `0x0020DCD0`, tpage `0x08A`, CLUT `0x7BC2`, draw coordinates
    `(26,107) (7.33,107) (30,105)`;
  - packet `0x0020DCF8`, draw coordinates
    `(7.33,107) (0.667,108) (26,107)`.
- Direct PPM samples confirm the visible slit is real and reaches the left
  edge: presentation `(0,470)` and `(50,470)` are `(255,214,156)`, while
  `(0,450)` is terrain `(105,82,64)`.
- Interpretation:
  - the remaining slits are a raster-coverage/watertightness defect;
  - native integer SXY collapses distant terrain strips to zero or near-zero
    height, exposing the same-coloured backdrop between adjacent strips;
  - neither more terrain traversal nor packet-space XY alone can fix it.
- Next:
  1. add a reusable PPM edge-hole analyzer and establish a numeric baseline;
  2. test a TerrainRoute-only conservative-coverage/seam guard;
  3. reject any guard that closes the slit by visibly bloating silhouettes or
     touching non-terrain materials.
- Status: root layer isolated; renderer coverage fix not yet implemented.

### T23 — Programmatic edge-hole metric

- Added:
  `tools/recompone-v8-2/analyze_terrain_edge_holes.py`.
- The tool reads the original P6 presentation capture, examines a configurable
  full-resolution ROI, and reports backdrop-coloured runs touching either
  edge. It does not resize, classify geometry, or use screenshot judgement.
- Baseline command region:
  `x=0..850`, `y=400..560`, reference `(255,214,156)`, per-channel
  tolerance 6, minimum run 3.
- Baseline evidence:
  `artifacts/terrain-completeness/route66-lateral34-packet-projection/edge-holes.json`.
- Baseline result:
  - 33 left-edge defect rows;
  - row bands `461..476` and `514..530`;
  - maximum pale run 107 presentation pixels;
  - zero right-edge runs inside this intentionally left-side ROI.
- This numeric baseline is now required for renderer experiments. A new
  capture must reduce these bands without adding silhouette bloat before it
  receives visual/motion review.
- Status: repeatable acceptance metric established.

### T24 — Conservative seam guard and omnidirectional controls

- Added diagnostic
  `RECOMPONE_V82_TERRAIN_SEAM_GUARD_PIXELS`.
- Route 66 baseline: 33 defect rows, maximum run 107 pixels.
- Guard 0.5: 31 defect rows, maximum run 99 pixels.
- Guard 1.0: 27 defect rows, maximum run 87 pixels.
- Conclusion: conservative coverage helps collapsed subpixel strips but cannot
  restore the missing outer terrain by itself.
- Added diagnostic
  `RECOMPONE_V82_TERRAIN_OMNIDIRECTIONAL=1`, which submits the full original
  draw-radius circle around the camera.
- Omnidirectional result:
  - run remained stable through 180 gameplay frames;
  - left defect rows increased to 138 and behind-camera terrain polluted the
    native ordering table;
  - rejected immediately as a production policy.
- Decision: "render all terrain" can only mean all terrain inside the true
  widened camera frustum. Behind-camera submission is not a blanket solution.
- Next: expand the final traversal polygon angularly in camera basis by the
  exact 16:9/4:3 aspect ratio, preserving forward depth and polygon topology.
- Status: seam guard is supplemental only; omnidirectional mode rejected.

### T25 — Terrain coverage mask as the measurement

- Every earlier probe (T10, T11, T13, T14, T22) read `va.X`/`va.Y`, which for
  terrain is the **packed** widescreen SXY plus the target margin. Enhanced
  renders terrain from camera space (`RECOMPONE_TRACE_ENHANCED_RENDERER`
  reports `direct=100%`, `fallback=0`), so those coordinates are not where the
  triangle lands. Packet-space probes therefore reported terrain at points the
  rasterizer left empty, and vice versa. Treat them as unreliable for terrain.
- Replacement metric: run with `RECOMPONE_V82_DEBUG_TERRAIN_COVERAGE=1`, which
  paints every `TerrainRoute` primitive magenta in the real presentation
  capture. Terrain presence is then read from rendered pixels, not from an
  intermediate coordinate space. A hole is a backdrop-coloured pixel strictly
  below the per-column terrain silhouette.
- 4:3 reference for the same arena: 1818 interior holes (119 in the outer
  left eighth). That is the engine's own floor; 16:9 cannot beat it.
- Status: this is now the acceptance measurement.

### T26 — Root cause 1: the traversal footprint loses lateral reach

- `func_8001C158` builds four camera-space corner rays and projects the
  clipped result onto the world XZ plane. That projection is not the frustum's
  angular sector: a corner ray's vertical component rotates partly into the
  horizontal plane whenever the camera is pitched, so each far corner gains
  forward distance while its lateral offset stays at the authored half-width.
  Measured on the traced Route 66 polygon, the far corners sit at
  `|lateral|/forward = 17040/21489 = 0.793` where the widened viewport needs
  `214/256 = 0.836`.
- This is why T04's exactly-correct angular widening of `gp+0xEDC` still left a
  wedge, and why T06's larger scalars did not behave linearly.
- Fix: `V82Compat.ExpandTerrainTraversalWideFit`, a pre-hook on
  `func_8001BECC`. It measures every finished polygon vertex in the camera's
  horizontal basis and pushes it out until its lateral offset covers the
  widened half-angle at that vertex's own forward distance, plus one terrain
  cell for the walker's inward per-row rounding. Vertices already satisfying
  the ratio and the camera-origin vertex are untouched, so the polygon is only
  ever enlarged and is never moved forward, back, or across the camera.
- Result: maximum expansion 1945 world units, terrain silhouette identical to
  the deliberately over-broad `ASPECT_POLYGON` and `POLYGON_PADDING` controls
  while adding less area. Interior holes 12613 -> 5924.
- Knobs: `RECOMPONE_V82_TERRAIN_WIDE_FIT=0` disables;
  `RECOMPONE_V82_TERRAIN_WIDE_FIT_MARGIN_CELLS` overrides the one-cell slack.
  Margin 4 and 8 give the same result as 1, confirming the fit is not
  scalar-calibrated.
- Status: shipped.

### T27 — Root cause 2: retail packet clipping eats the widescreen margin

- After T26 a band of thin wedges remained in the outer left eighth (3971
  holes) and nothing in the traversal layer moved it: wide-fit margin 4/8,
  row-cell padding 1, the seam guard, and stable packet projection all left it
  at 3577-4073.
- `Gte.Rtp` packs the widened projection into the authored 0..320 window so
  retail clipping still works. The widened viewport consumes that window
  exactly, so a primitive straddling the viewport edge straddles the packed
  window edge, and `func_800290A8` discards it whole instead of clipping it.
  The visible part of that primitive is inside the picture, so it leaves a
  wedge.
- Fix: reserve unused native pixels on both sides of the packed projection
  (`Gte.WideClipHeadroom`, `RECOMPONE_V82_WIDE_CLIP_HEADROOM`, default 48).
  Enhanced never reads the packed X for geometry, so this moves the retail
  clip boundary outside the real picture without moving a single drawn pixel.
- Calibration on the Route 66 frame, outer-left holes: 0 px -> 3971,
  16 px -> 1111, 40 px -> 278, 56 px -> 278, 72 px -> 278. The curve is flat
  from 40, so 48 sits on the plateau.
- Result: interior holes 5924 -> 2189 total, 278 in the outer left, against
  the 4:3 engine floor of 1818/119.
- Status: shipped.

### T28 — Distance fog converged on the wrong colour

- Separate reported defect, same horizon. With Enhanced fog off, the farthest
  terrain renders at `(99,74,41)` against a `(255,214,156)` sky: the engine
  applies no distance fade of its own, so this is entirely the renderer's job.
- With Enhanced fog on, the shader faded towards a synthesized mid-grey
  (`atmosphericLum` clamped to 0.48..0.68), producing `(132,123,115)` at the
  horizon - a hard grey band under a warm sky, and distant props that stayed
  conspicuous no matter how far away they were.
- The arena publishes its own horizon colour: the engine lays down a flat,
  untextured, full-display quad at `ot=4096` every gameplay frame. Route 66
  uses two stacked bands, `(238,226,193)` over `(255,213,154)`; the tall lower
  band is the one the terrain disappears into.
- Fix: `EnhancedGlBackend.RecordAtmosphereColor` harvests that quad - farthest
  OT wins, then greatest vertical span - and feeds `uFogColor`/`uFogColorValid`
  to the shader, which now blends all the way to it over
  `pow(smoothstep(2200, 20500, depth), 0.62)`. 20500 matches the terrain
  walker's own limit so the last terrain row is the sky.
- Status: shipped.

### T29 — Acceptance

- A/B switch for a faithful before-state:
  `RECOMPONE_V82_TERRAIN_WIDE_FIT=0 RECOMPONE_V82_WIDE_CLIP_HEADROOM=0
  RECOMPONE_V82_FOG_ATMOSPHERE=0`. Validated against a capture from the
  pre-change build: 31 differing pixels in the horizon band, against ~52000
  pixels of ordinary run-to-run capture jitter in animated HUD and reticle
  elements. (That jitter is why single-pixel diffs between runs mean nothing
  here; the terrain metric is stable to the pixel across runs and builds.)
- Reusable proof tool: `tools/recompone-v8-2/build_terrain_fog_proof.py`.
- Interior terrain holes, before -> after:
  - Route 66: 12613 -> 2189 total, 10588 -> 278 in the outer left eighth;
  - Olympic: 882 -> 42 total, 829 -> 0 in the outer left eighth;
  - the 4:3 engine floor for Route 66 is 1818 / 119, so 16:9 is now within
    the engine's own behaviour rather than adding widescreen-specific loss;
  - Bayou and Nuclear are night arenas whose backdrop is too dark to separate
    from their own shadowed materials, so the count is reported as n/a there
    and those arenas are judged visually.
- Horizon colour step (terrain against sky, middle half of the frame):
  Route 66 17.6 -> 13.0, Bayou 22.6 -> 16.8, Olympic and Nuclear unchanged.
- Motion: six consecutive full-resolution 1920x1080 frames per arena, both
  edges, Route 66 and Olympic. Every frame is continuously filled after the
  change; every before frame shows wedges.
- Controls: Nuclear before/after is effectively identical (a close interior
  spawn with no far terrain), and a 4:3 run remains correct with both fixes
  active - the terrain work is gated on widescreen and the fog work is not,
  which is intended because the fog defect was never widescreen-specific.
- Evidence: `artifacts/terrain-fix/proof/` and `artifacts/terrain-fix/ab/`.

### T30 — Root cause 3: the engine's own zero-area rejection eats horizon strips

- Reported after T29: thin bright slits still crossed the horizon around the
  road. They are not widescreen-specific - the 4:3 control carries the same
  band - so T26 and T27 could not have removed them.
- `func_800288E0` issues NCLIP twice per terrain primitive and drops faces
  whose signed area is not positive. Terrain approaching the horizon is a
  stack of strips a fraction of a native pixel tall, and integer SXY rounding
  drives their three vertices exactly collinear, so the engine computes zero
  area and rejects them. At the authored 1x that discards something genuinely
  sub-pixel. Enhanced draws the same strip from camera space several real
  pixels tall, so the rejection punches a visible slit instead.
- Fix: inside the terrain scope only (`Gte.BeginTerrainProjection` /
  `EndTerrainProjection`, bracketing `func_800288E0`), evaluate NCLIP from the
  fractional projection the renderer actually draws, rounding away from zero
  so a sub-pixel strip survives the engine's integer sign test.
  `RECOMPONE_V82_TERRAIN_PRECISE_NCLIP=0` restores the integer result.
- Scope matters. Precise NCLIP was previously enabled globally and then
  reverted, because NCLIP is game-visible state and changing it alters which
  faces the original game submits everywhere. Confining it to the terrain
  transform keeps every other subsystem bit-exact.
- Result on the same Route 66 frame, backdrop-coloured pixels in the horizon
  band (y=425..535): 4888 original build -> 334 with T26+T27 -> 111 with T30.
  Interior holes 2189 -> 1977, and the frame-middle band 1528 -> 1315, now
  below the 4:3 engine's own 1640.
- Six consecutive frames show the band stable with no popping or flicker.
- Evidence: `artifacts/terrain-fix/proof/00-levels_route66-horizon-slits.png`.

### T31 — Seam guard definitively rejected

- `RECOMPONE_V82_TERRAIN_SEAM_GUARD_PIXELS` does not merely fail to help, it
  reopens the outer-edge wedges T26/T27 closed. Enabling it also enables the
  stable packet-space terrain projection path in `DrawTri`, which replaces the
  camera-space projection with the compressed packed XY - exactly the
  coordinates T27 deliberately compresses further. Frame-matched captures at
  guard 0.5 and 1.0 show the left-edge wedges back plus new speckle.
- Do not re-test conservative coverage without first separating it from
  `StabilizeTerrainPacketVertex`.

### T32 — Driven-vantage captures (autodrive)

- `run_reference_soak.py` already autodrives: `V82Compat.UpdateSoak` holds
  Cross and alternates Left/Right every 180 frames. It only does so when
  weapon coverage is enabled, so `--coverage-profile weapons` keeps the drive
  while `RECOMPONE_V82_SOAK_POWERUPS=0` removes the transformations that
  perturb vehicle dynamics.
- The `final-presentation` capture is taken after the full frame budget, so it
  shows wherever the car drove to. Earlier acceptance work only ever looked at
  the `gameplay` capture, which is the spawn view.
- Repeatability window: two identical 180-frame drives differ by 52k pixels,
  the ordinary animated-HUD jitter floor. At 540 frames two identical runs
  differ by 1.8M pixels. The drive is only frame-comparable for short budgets,
  because the scripted input is poll-based while gameplay ticks advance
  separately, so under `RECOMPONE_UNTHROTTLED` the pulses land on different
  ticks. Do not attempt a frame-matched A/B at a long driven vantage.
- Route 66 at 180 frames reaches the roadside stands, giving a controlled
  before/after with real buildings at range. Distance from the sky colour:
  diner and donut stand 63.8 -> 42.6, ridge 85.2 -> 54.6, near ticket booth
  96.4 -> 94.4 (correctly unchanged).
- Evidence: `artifacts/terrain-fix/proof/00-levels_route66-arizona-*.png`.

### T33 — Distant props stood on unrendered ground

- The roadside diner and Super Donuts stand on Route 66 hang in the sky. The
  coverage mask shows terrain rising to exactly their base and stopping.
- Not caused by anything in this investigation; the original build floats
  identically.
- Cause: `ExtendObjectDrawDistance` extends the object cull distance while the
  terrain walker keeps its stock reach. Retail pairs the two, so extending one
  alone leaves props standing past the last terrain row.
- Terrain cannot be pushed out to meet them. Scaling the traversal polygon and
  raising the scratchpad `+0x98` far plane together does draw far more ground
  - cells submitted 636 -> 2010, terrain triangles 2128 -> 5114 at scale 2.0 -
  but the walker stops selecting new cells past that: scale 4.0 gives 2079
  cells and 5058 triangles, statistically identical, and the props still float
  by the same ~20 px. Something inside `func_8001BECC` bounds it; it is not
  the far plane, not the packet arena (zero rejected cursors), and not the
  grid bounds (clamping vertices changed nothing).
- Fix: lock the object range to the terrain range. `ExtendObjectDrawDistance`
  now applies only when the terrain range scale is above stock, and the
  terrain range defaults to stock, so objects are culled exactly where the
  ground stops - which is what retail does. The props are gone rather than
  floating.
- Cost: distant scenery is culled sooner than the Enhanced option previously
  allowed. That is the correct trade while terrain cannot follow; a prop with
  no ground under it is worse than no prop.
- Evidence: `artifacts/terrain-fix/coupled`, `artifacts/terrain-fix/nodrawdist`.

### T34 — The traversal refit flickered; row padding replaces it

- Reported from live play: terrain flickering out while driving. Neither still
  captures nor 6-frame bursts nor any windowed average shows this, which is
  how it reached a staged build.
- New instrumentation: `RECOMPONE_V82_TRACE_TERRAIN_FRAME=1` emits one
  `[TerrainFrame]` line per presented frame with the terrain triangle count,
  the world triangle count, and the cells the walker submitted and emitted.
  `tools/recompone-v8-2/analyze_terrain_flicker.py` gates on it.
- Two things had to be right for the metric to mean anything:
  - the engine presents at 60Hz while submitting at 30Hz, so every other
    present is a repeat that draws nothing. Those must be excluded, not read
    as dropouts. Frames with `world=0` are skipped.
  - the cell counters must only be consumed on a present that actually drew,
    or the repeat present drains them first.
- The discriminating signal is `cells`, not the triangle count. Terrain volume
  varies legitimately with the scene; a walker that selected one cell out of a
  median 637 does not.
- Result, Route 66, 600 gameplay frames, frames where the walker selected
  under 10% of the median cell count:
  - baseline: 2
  - `ExpandTerrainTraversalWideFit`: 41
  - wide fit with vertices clamped to the terrain grid: 49 (not grid bounds)
  - wide fit replaced by a uniform linear lateral scale: still starving
  - `RECOMPONE_V82_TERRAIN_ROW_CELL_PADDING=4`: 2
- So any rewrite of the traversal polygon intermittently makes `func_8001BECC`
  select nothing, whatever its shape. Padding the row span at `func_8001BE68`
  instead cannot: that is an inclusive/exclusive X-cell range the callee
  already clamps to the grid, and the walker's own row iteration is untouched.
- Edge result is identical either way - outer-left holes 10588 -> 0, total
  12613 -> 1698 - so the polygon rewrite bought nothing over row padding.
- Shipping defaults: row padding 4 on, wide fit off, terrain range scale off.
- Reverted: `ExpandedPrimitiveBufferSize` back to 512 KiB. Growing it to 1 MiB
  moved `PcHeapBase` from 0x80300000 to 0x80400000 and put the second packet
  arena where the heap used to start, which corrupted terrain badly in play.

### T35 — Conservative coverage does not touch the residual slits

- Rewrote the seam guard to widen coverage in the projected space terrain is
  actually rasterized from. Displacing a vertex by `d` projected pixels is
  `view.xy += d * view.z / scale`, so coverage can be widened without touching
  depth, UVs, colour, or the packet path. The old guard edited packet XY, which
  the terrain path never reads, and only appeared to do anything because it
  also forced the primitive onto the packet-space projection - dragging the
  compressed widescreen X with it and reopening the outer edges.
- Result: the residual frame-middle slit count is 1315 at guard 0, 0.25, 0.5
  and 1.0. Identical. Larger guards only bloat the silhouette, which pushes
  the outer-right count up (383 -> 407 -> 472).
- Conclusion: those slits are missing geometry, not sub-pixel cracks between
  adjacent triangles, so conservative coverage cannot close them. They sit
  below the 4:3 engine's own 1640 for the same band and are the engine's
  terrain tessellation at the horizon.
- Status: view-space guard retained and correct, but it defaults off because
  it buys nothing.

### T36 — Objects that stop drawing while still on screen

- Reported from live play: objects vanish at the left and right edges up close.
- Pre-existing. Edge-band collapses measure 20 on the pre-change baseline
  against 18 to 29 across every configuration, all inside the spread the
  diverging autodrive produces.
- **The runtime now detects this itself**, because chasing it by hand does not
  work: `RECOMPONE_V82_TRACE_OBJECT_CULL=1`. Each time `func_8002E22C` accepts
  an object on one tick and rejects it on the next, while the object is no
  further away than before and still inside the frame, it emits a
  `[V82ObjectPop]` line with the object address, its world position, the camera
  position, the distance and the on-screen position, and captures the first six
  frames it happens on. `tools/recompone-v8-2/analyze_object_pops.py` groups
  them by arena location so a spot can be revisited deliberately.
- What the test actually is: three planes at `gp+0xFD8`, rebuilt each frame by
  `func_8002DFF0` for the authored 4:3 view. An object is kept only if its
  distance outside all three is under its own bounding radius.
- Fix applied: grant the test the lateral reach a widened frustum has at each
  object's own distance. Paired deterministic 180-frame runs, objects that stop
  drawing while still on screen: 45 with it against 58.5 without, a 23%
  reduction. On 600-frame runs, 150 against 257.
- **Still open**: the specific extreme-edge case is unmoved, 15 against 16.
  Those are small props, radius 76, at 4400 to 6400 units, popping at the left
  edge with the camera near x=243958 z=324093 on Route 66 - object addresses
  0x806B569C, 0x806B553C, 0x806B58AC, 0x806B53DC at ticks 55, 141 and 219.
  That is a deterministic reproduction to start from; no live observation is
  needed.
- Ruled out along the way, each by measurement: wholesale culling before the
  edge (object geometry spans normalized X -1.7 to 3.0), the packed clip
  headroom (edge share 10.69% against 11.02%), and packed-coordinate
  saturation (455k/494k/353k at headroom 0/48/100, no trend).


### T37 — Wall segments cut beside the camera

- Screenshots showed a long wall ending at a hard vertical line with the scene
  visible past it. The line moves along the wall as the car drives, so it is
  not a fixed screen boundary.
- Ruled out by measurement: the scissor (`[V82NarrowClip]`, zero occurrences in
  2549 live frames and 400 harness frames), packet-coordinate fallback
  (`fallback=0` over 600 frames), and the packed clip headroom, whether scoped
  to terrain or applied globally.
- Cause: `Gte.Divide` overflows when `h >= sz3 * 2`, i.e. for any vertex nearer
  than half the projection distance, and sets GTE flag 17. Retail rejects the
  whole primitive on that flag - correct for hardware that can only draw what
  the projection produces. It fires 749 times per frame in ordinary play. A
  wall segment passing beside the camera trips it and vanishes, and widescreen
  shows a third more of exactly that region, so the gap lands inside the
  picture.
- Fix: withhold flag 17 during Enhanced gameplay. The saturated value is still
  returned, so anything that consumes it sees what it saw before; only the
  wholesale rejection stops. Enhanced projects that geometry from camera space
  and never reads the result.
- Effect: median object triangles per frame 626 -> 667 in autodrive, which
  rarely hugs a wall; the effect where the camera is actually against geometry
  is larger. `RECOMPONE_V82_NEAR_REJECTION=0` restores retail behaviour.
- Gates unchanged: edge holes 1698 with outer-left 0, walker starvation 2,
  6/6 six-arena soak with no faults.

### T38 — Wall segments dropped by the per-object frustum test

- The severed edge is **slanted** - 7 px of x over 77 px of y. Screen-space
  clipping produces exactly vertical lines, so nothing was cutting the wall.
  The wall is segmented and the segment nearest the camera is not drawn at all;
  what looks like a cut is the surviving segment's own boundary. Every fix
  aimed at cutting was therefore aimed at nothing: scissor, coordinate
  fallback, clip headroom in both scopes, near-plane rejection flag,
  near-plane clipping, depth buffer, and true per-vertex depth.
- Measuring the gate directly is what finally worked. `TraceObjectRenderBegin`
  and `TraceObjectRenderEnd` bracket `func_8002D9E0`, compare the packet cursor
  across it, and attribute every object that emitted nothing to one of its
  three exits: the object flag, the frustum test, or the distance limit.
  `RECOMPONE_V82_TRACE_OBJECT_CULL=1`, reported as `[V82ObjectGates]`.
- Result: of every object rendered, the share that emits anything is **32.2%**
  at stock. The frustum test accounts for essentially all of the rest - 20388
  drops against 142 for distance and 0 for the flag.
- Widening that test to the widescreen field of view raises it to 51.4% at
  slack 1.6 and 61.5% at 2.5. At 4.0 the test rejects nothing at all, which is
  no longer a frustum test and would draw geometry behind the camera, so 2.5 is
  the most reach that still culls. Default is now 2.5.
- Metrics that could not score this, and why:
  - severed-edge counts rise when more geometry is kept (287 -> 378 -> 556),
    because more drawn objects means more boundaries;
  - object-pop counts likewise;
  - `[V82SegmentPop]` coverage tracking has a floor around 135 that persists
    even with the frustum test fully disabled, so it cannot resolve the
    remainder.
- Two mistakes worth not repeating: comparing per-frame object identity by
  packet address fails because the engine alternates two packet arenas, so
  compare two drawn frames apart; and any per-frame comparison must skip the
  60Hz repeat presents, which draw nothing.

## Prohibited shortcuts

- Do not call the 4:3 control proof of the widescreen fix.
- Do not increase the traversal scalar again without new evidence.
- Do not infer terrain presence from a downscaled screenshot.
- Do not classify particle/object packets as terrain based only on their screen
  position.
- Do not mark this issue complete from a still frame, a burst, or any
  averaged counter. Terrain that drops out on some frames is invisible to
  all three. Run analyze_terrain_flicker.py before staging anything.
- Do not repeat T01–T22 unless a new code change specifically invalidates one
  of their measured conclusions.
- Do not use `RECOMPONE_TRACE_TRIANGLE_PROBE` or the packet-space scanline
  union to decide whether terrain covers a point. See T25: those read packed
  coordinates that the terrain rasterizer does not use. Use the coverage mask.

## T39 — the vertical cut through walls: widescreen NCLIP quantization

**Mechanism.** `Gte.Rtp` narrows the PS1-visible `SX` by
`BaseAspect/WideAspect` (0.75) and rounds it to an integer, so the retail
0..320 clip tests cover the full 16:9 view. `NCLIP` (GTE op 0x06) computes the
backface sign as a cross product of those rounded coordinates:

    MAC0 = SX0*(SY1-SY2) + SX1*(SY2-SY0) + SX2*(SY0-SY1)

Every term carries the 0.75 factor, so the measured area shrinks by the same
ratio and far more polygons land under one unit, where rounding alone decides
the sign. A tall thin vertical quad — exactly a destructible wall panel — has
an area dominated by sub-pixel x differences, so the rounded test flips it to
backfacing and the engine drops the panel. On screen that is a full-height
vertical slice of wall missing, which reads as a clean cut.

This is why every clipping hypothesis failed to explain it: nothing was
clipping. The geometry was never submitted.

**Measurement** (Route 66, 903 drawn frames, `nclipT`/`nclipO`/`nclipRescued`
on the `[TerrainFrame]` line). Polygons where the rounded integer test and the
fractional projection disagree on the sign:

| | per frame | total |
|---|---|---|
| terrain | 239 | 216001 |
| walls / objects | 452 | 408005 |
| of which front faces the integer test **deleted** | 620 | 559680 |
| back faces it wrongly kept | 71 | 64326 |

**Fix.** `RECOMPONE_V82_WIDE_PRECISE_NCLIP` (default on) extends the
already-shipped terrain-scoped fractional NCLIP to all Enhanced widescreen
gameplay geometry: measure the area from `SxyPreciseX/Y`, the same fractional
projection the renderer actually draws from. A uniform positive x scale cannot
reorder a cross product on its own, so the sign only changes for polygons the
integer test was already getting wrong.

**Why the earlier hunt missed it.** Every detector looked for a *boundary* —
severed edges, clip columns, scissor rects, resolve regions. A dropped panel
has no boundary of its own; it borrows the edges of its neighbours. The
vertex-column histogram correctly found no local spike in 456 on-screen
columns, which was the evidence that no clipper was involved, not evidence
that nothing was wrong.

**Rejected along the way** (all measured, all null): MSAA resolve regions
(`CopyResolveToMsaa` is dead code; the live `ResolveRegion`/`CopyRegionToMsaa`
pair is sound), output-resolution dependence (worst column is proportionally
identical at 1730x960, 1730x977 and 1920x1080), and stale precise-vertex
resurrection (`PSMemory` already invalidates on every 8/16/32-bit write).

### T39 outcome: rejected

Extending the fractional NCLIP past the terrain scope is **inert**. Over 900
frames in Casino City, mean world triangles per drawn frame:

| | mean | median |
|---|---|---|
| integer NCLIP (retail rounding) | 4133 | 4138 |
| fractional NCLIP | 4139 | 4283 |

620 sign decisions per frame change and 0.1% more geometry is drawn, so these
NCLIP results are not what gates drawn polygons. `RECOMPONE_V82_WIDE_PRECISE_NCLIP`
now defaults **off**; the terrain-scoped correction, which was measured against
the horizon-slit metric, is unchanged and stays on.

### T40 — the harness cannot support before/after screenshots

Two runs with **identical** configuration differ across **64.68%** of the
frame (`artifacts/terrain-fix/ctrl1` vs `ctrl2`, Casino City, 150 frames,
frame-locked capture). Trajectory, vehicle and enemy state all diverge between
runs. Any conclusion drawn from a single-frame image diff across two runs is
therefore worthless, including the "2.28% restored" and "1.58% restored"
figures recorded earlier in this session. Aggregate metrics accumulated over
hundreds of frames within a run remain valid.

Consequence: proof of any future fix must come either from a within-frame
marker or from an aggregate distribution, never from a paired screenshot.

### T41 — locking in a single example (F9 frame capture)

Aggregate telemetry cannot localise a defect the harness will not reproduce,
and T40 rules out paired screenshots. The build now records one *chosen* frame
in full:

- **F9** writes `recompone_present_seam_<w>x<h>_<aa>.ppm` (exactly what is on
  screen) and `recompone_geometry_frame<N>.txt` (every triangle drawn in that
  same frame) into the game directory.
- Each dump line is
  `packetAddress material clut texpage  x,y,viewZ,provenance ×3`,
  in target space (0..428 wide, margin 54), provenance 2 = exact GTE
  address/value, 1 = reconstructed, 0 = screen-space fallback.
- `RECOMPONE_V82_GEOMETRY_DUMP_FRAME=<n>` arms the same dump headlessly;
  verified in Casino City, 3877 triangles written.

With the seam visible in the image and the full triangle list for that frame,
the wall can be identified by its packet address and its geometry inspected
directly - whether the missing panel was submitted at all, and if so what its
coordinates and depth were - instead of being inferred from counters.

### T42 — the wall cut is a near-clip, identified from a captured frame

The F9 capture settled what years of aggregate telemetry could not. In the
captured frame the wall is 553 single-triangle material-2 packets. Sorting them
by depth shows the run marching from far to near and stopping dead:

- the seven nearest packets are **one wall column** (same x span, stacked
  vertically) and every one is cut at exactly the same screen column,
- the nearest surviving vertex is at depth **101.7**, and nothing nearer exists
  in the frame at all.

Seven packets sharing an exact cut column is a clip boundary, not authored
geometry. A plane at constant depth intersecting a receding wall projects to a
vertical line at an arbitrary interior screen column - which is precisely the
straight-edged truncation reported, and why every screen-space clipping theory
failed: the cut is not at any screen boundary.

**Why widescreen exposed it.** At 4:3 the geometry inside the near cut was
mostly outside the frame anyway. Widescreen shows a third more to each side, so
the missing near geometry moved into view. Same root cause as the long-standing
"objects vanish at close range near the screen edges" report.

**Mechanism.** `H` (projection distance) is **256**, so `Gte.Divide` overflows
for every vertex with `SZ < 128`. `SuppressNearRejection` already withholds
`Flag(17)` there, which is why geometry between 100 and 128 survives at all.
But the saturated divide still produces a packed coordinate outside the
representable range - measured at **1412 saturated object vertices per frame**,
with the overflow itself firing **2197 times per frame** - and the engine
discards those primitives.

**Change.** `RECOMPONE_V82_NEAR_KEEP` folds a saturated packed coordinate back
into a representable window (centre ±480) for Enhanced widescreen gameplay
only. Enhanced reconstructs this geometry from camera space and never reads the
packed value, so nothing that is drawn moves; the only effect is that the
primitive is no longer thrown away. Measured over 600 frames:

| arena | | <110 | <120 | <130 | <150 | <200 |
|---|---|---|---|---|---|---|
| Casino City | baseline | 161 | 153 | 202 | 345 | 944 |
| Casino City | NEAR_KEEP | 355 | 431 | 378 | 773 | 1850 |
| Wild West | baseline | 49 | 55 | 44 | 107 | 389 |
| Wild West | NEAR_KEEP | 93 | 106 | 90 | 159 | 482 |

**Still unexplained.** No non-vehicle world geometry below depth 80 is ever
submitted, in any configuration, in either arena - and `straddle`/`behind` are
both zero across 700 frames. Something culls whole primitives before projection
when they approach the camera. That gate is the remaining work; `NEAR_KEEP`
addresses only the saturation gate above it.

### T43 — reproduced in the reported arena (Route 66, "Winslow, Arizona")

The arena was identified from the captured frame by colour signature against
headless spawn captures of all 18 arenas: Route 66 scored 0.467 against 0.905
for the runner-up. Confirmed by the user. This removes the dependency on manual
testing for this artifact.

Near-depth histogram of submitted non-vehicle world primitives, Route 66,
900 frames:

| | <40 | <60 | <100 | <120 | <130 |
|---|---|---|---|---|---|
| baseline | 0 | 0 | 48 | 113 | 118 |
| `NEAR_KEEP` | 6 | 2 | 79 | 139 | 148 |

Geometry closer than depth 60 was never submitted at all in this arena before
the change. The `<150`/`<200` totals fall rather than rise, but soak runs
diverge, so only the appearance of previously-impossible near buckets is
structural evidence; the totals are not comparable between runs.

### T42/T43 corrected — `NEAR_KEEP` is inert, `MAC0` suppression is harmful

Both were scored on raw counts from soak runs that diverge, which T40 already
established is invalid. Normalised against each run's own drawn geometry:

| | near primitives per million drawn |
|---|---|
| baseline | 32.0 |
| `NEAR_KEEP` | 32.4 |
| `NEAR_KEEP` + MAC0 flag suppression | 5.5 |

`NEAR_KEEP` changes nothing and is defaulted off. Suppressing the MAC0 overflow
flags is actively harmful and was reverted. The T43 claim that geometry below
depth 60 appeared for the first time was trajectory noise: 6 and 2 primitives
against a run that drew half as much total geometry.

**Any future measurement here must be a rate, not a count.**

### T44 — the flag drives clipping, not rejection

The engine reads the GTE `FLAG` register **571,587 times in ~400 frames**
(~1400/frame), so it does test flags per primitive. But suppressing the MAC0
overflow bits *reduced* near geometry sixfold. A flag that gates rejection
would do the opposite. The consistent reading is that a GTE error steers the
engine into its clipping path, and withholding the flag makes it skip clipping
and lose the primitive further downstream.

That reframes the artifact: the straight-edged cut is most likely the engine's
own near clipper operating correctly, on a plane that at 4:3 fell outside the
visible frame. The remaining work is to locate that clip plane's constant
rather than to stop primitives being rejected.

### T45 — the cut edge is a mesh boundary, not a clip

The twelve vertices sitting exactly on the cut column in the captured frame
form a shared vertical chain:

| depth | screen Y | packets |
|---|---|---|
| 101.7 | 192 | C20 |
| 106.7 | 278 | AE0 + C20 |
| 109.1 | 320 | AE0 + B00 |
| 114.0 | 403 | 9E0 + B00 + B20 |
| 116.5 | 447 | 9E0 + A00 |
| 121.6 | 535 | A00 + A20 |

Each depth appears twice, shared between vertically-adjacent panels, and depth
increases smoothly down the screen. Clipping manufactures *new*, unshared
vertices, normally at constant depth. This is an authored mesh edge.

**So nothing is being clipped. An entire adjacent wall model is not drawn.**
That matches the reporter's own description of the wall as several destructible
models. Every "where is the clip plane" line of inquiry - T39 through T44 - was
aimed at the wrong kind of defect.

### T46 — all three object gates eliminated

`[V82ObjectGates]`, Route 66, 600 frames, varying `RECOMPONE_V82_OBJECT_CULL_SLACK`:

| slack | emitted | frustum-culled | distance-culled |
|---|---|---|---|
| 0 (retail) | 42.8% | 52.0% | 5.3% |
| 2.5 (shipped) | 58.4% | 32.5% | 9.1% |
| 40 | 63.1% | **0.0%** | 36.9% |

At slack 40 the frustum test is fully disabled and near geometry still does not
appear (`<80` remains 0, as in every run this session). Objects merely fall
through to the distance gate. The frustum cull, the distance cull and the
object flag are therefore all ruled out as the cause of the missing model.

Also measured and ruled out: object-cull slack has no effect on the near-depth
distribution across four arenas (`<150/M`: 95.1 at slack 2.5, 63.5 at 10, 33.1
at 40 - all noise, `<80` zero throughout).

### T47 — the walls may not be "objects" at all

An attempt to census every object the engine considers, keyed off the
`func_8002D9E0` pre/post hooks, records **zero** calls during a captured
gameplay frame, while the same hooks report ~88 objects per frame through the
gate counters. The most likely reading is that the arena walls are submitted by
a different subsystem than the object renderer those hooks wrap, which would
explain why none of the object gates affects them.

**Next step:** identify which subsystem emits the material-2 packets at
0x00292xxx, rather than assuming it is the object renderer. Until that is
known, tuning object culling cannot fix this.

### T48 — the walls ARE object-renderer geometry (T47 was wrong)

Packet ownership was only ever recorded for vehicles, so everything else read
as "unresolved" - which is what made T47 conclude the walls came from another
subsystem. Registering ownership for non-vehicle objects too
(`RECOMPONE_V82_CENSUS_OWNERSHIP=1`) resolves **547 of the 558** material-2
wall triangles to `v82-object`. The arena walls are emitted by
`func_8002D9E0`, so the object gates do apply to them.

The object-census hook still records zero calls during a captured frame even
though `EndObjectRender` demonstrably runs (it registers the ownership above).
That instrumentation bug is unresolved and is why T47 misread the situation;
the ownership route sidesteps it.

### T49 — the autodrive cannot reproduce a camera-against-wall

Three drive patterns, Route 66, 900 frames each, near-depth buckets of
submitted non-vehicle world primitives:

| drive | <20 | <100 | <150 | <200 |
|---|---|---|---|---|
| stock (weaving) | 0 | 30-143 | 288-498 | 654-1673 |
| hold accelerate + left | 1 | 0 | 11 | 70 |
| hold accelerate only | 1 | 3 | 27 | 73 |
| hold accelerate + right | 0 | 0 | 0 | 2 |

Holding a turn drives a circle in open ground rather than pinning the car to a
perimeter. The stock weaving drive is the best of the four and still never puts
world geometry closer than depth 80 to the camera. **The reported artifact
cannot currently be reproduced headlessly**, which is why the object-cull
question has to be answered by a manual A/B (`V8_2_LOOSE/test_wall_cut.bat`).

Slack 40 - which disables the frustum cull entirely - passes 18/18 arenas, so
it is safe to use for that test.

### T50 — object culling eliminated in the reported scene; the near wall IS drawn

Manual A/B in Route 66 with `RECOMPONE_V82_OBJECT_CULL_SLACK=40` (frustum cull
fully disabled, 18/18 arenas pass): **the artifact is unchanged**. Object
culling is ruled out in the reporting scene, not merely in the harness.

The F9 capture from that run, with per-object packet ownership resolved, shows
the near wall section is submitted and drawn:

| object | depth | triangles | screen bbox (sx) |
|---|---|---|---|
| 806F2B60 | 152.8 | 21 | -28.9 .. 142.2 |
| 806F2C10 | 502.7 | 4 | -57.8 .. 149.0 |
| 806F0EC0 .. 806F12F8 | 1893 -> 599 | 10-13 each | tiles 196 -> 865 |

Nothing is culled and nothing is clipped: `806F2B60` covers the left third of
the frame with no NaN vertices. **The whole "missing geometry" framing was
wrong.**

**New reading — level of detail.** The distant wall panels carry 10-13
triangles each over a *small* screen area; the near section carries 21
triangles over a *third of the screen*, and renders smeared and featureless
where the panels beyond it show crisp detail. That is what a lowest-detail LOD
looks like when it is selected for the closest geometry rather than the
farthest. The reported "cut" is the seam between the flat LOD slab and the
detailed panels behind it.

**Next:** find the LOD selection for wall objects and check whether its
distance test is inverted or mis-scaled under the widescreen projection. Note
`V82Compat` already resolves model descriptors to "body/LOD/wheel" groups via
`BeginImportedRenderGroup`, so LOD identity is already available to instrument.

### T51 — the renderer is faithful; the geometry is genuinely not submitted

The earlier overlay that appeared to show geometry submitted where the screen
showed background was a **coordinate-mapping error on my part**. Calibrating
against the player vehicle gives the correct mapping: the frame spans
sx in [-54, 374], i.e. `image_x = (sx + 54) / 428 * imageWidth`.

With that correction the submitted geometry aligns exactly with the rendered
image - the wall slab, its diagonal near edge, the distant fence and the hill
barrier all match. **The renderer draws everything it is given.** The wall
section the reporter sees as cut away was never submitted.

Combined with T50 (frustum cull fully disabled changes nothing), that means the
missing section is not culled, not clipped and not dropped in drawing. It is
never offered.

### T52 — object submission is not driven by the terrain cell walk

`RECOMPONE_V82_TERRAIN_ROW_CELL_PADDING` 4 vs 16, Route 66, 600 frames:
objects offered to the renderer 60740 vs 60093 - unchanged. The padding that
fixed the terrain edge holes has no effect on which objects are submitted, so
the two traversals are independent.

### T53 — object census (working, with caveats)

After several false starts the census records 172 distinct objects offered per
capture, 109 producing geometry and 63 producing none. Faults found and fixed
along the way, all of which had silently produced empty or garbage output:

* `c.A0` is clobbered by the time the object renderer returns - the object
  pointer must come from the scope captured on entry;
* the address range check assumed the PS1's 2 MB, but this build places objects
  past 7 MB, so every record was rejected;
* clearing the census per frame emptied it before any capture could read it,
  because that reset point sits elsewhere in the loop than object submission.

**Still wrong:** the recorded world position. `_objectCullPosition` is the
pointer from the most recent visibility test, which does not reliably belong to
the object being recorded, so positions come out as 0 or 0x4000000. Without
trustworthy positions the census cannot yet answer whether an object exists at
the place the wall is missing. That is the next thing to fix.

### T54 — the defect, finally quantified: the left widescreen-only band

Working backwards from the reporter's frame: the near wall slab's left edge
sits at **sx = -28.9**. A wall receding past the camera projects its near end
to large negative sx, so the wall should cover the frame's left edge
continuously. Instead it stops short, leaving the strip from -54 to -29 - the
widescreen-only band - empty.

That strip is ~5.8% of frame width, which is exactly the "worst column at 5.6%
of width, 24% of rows" measured at the start of this investigation and
dismissed as an arena boundary. It was the artifact all along.

**Metric** (`tools/recompone-v8-2/measure_outer_band_coverage.py`): coverage of
each 54px widescreen-only band by wall/prop geometry (materials 1 and 2),
expressed as a ratio against the adjacent 54px strip of the authored view. A
ratio is used rather than a count because soak runs diverge (T40). Terrain is
excluded - it fills the bands regardless and masks the defect.

Across 31 dumps, three arenas:

| band | coverage | adjacent authored strip | ratio |
|---|---|---|---|
| left outer | 41.3% | 47.9% | **0.862** |
| right outer | 46.1% | 44.7% | 1.031 |

**The deficit is left-only.** The right widescreen band is fully covered; the
left is short by 14%. Any correct fix must move the left ratio toward 1.0
without disturbing the right, and this is the first metric in this
investigation that measures the reported artifact directly rather than a proxy.

Ruled out on the way here: walls generated to fill the authored view (run edges
scatter well beyond the frame on both sides, 20 runs measured), missing wall
objects (address-gap analysis finds only unused pool slots), and LOD selection
(the near object carries *more* triangles than distant panels, not fewer).

### T55 — the panoramic backdrop stops short of the widescreen frame

The large unattributed material-2 geometry in every capture is the sky/scenery
panorama: a small number of big textured quads at a constant far depth
(z ~3326-3330), occupying the upper band. In the reporter's frame 3416 it is
exactly two quads spanning **sx 6.5 .. 636.7**, and in Wild West frame 4082
**sx 2.0 .. 632.9**. The widescreen frame is sx -54..374.

So the panorama begins at the authored viewport's left edge and overhangs
uselessly far to the right. When it stops short, the left strip has no sky
behind the world.

It is emitted by **`func_80050B38`**, a renderer distinct from the object
renderer: it walks its own list at `gp+0x1188` and culls each item with the
same `func_8002E22C` visibility test used for objects. So the panorama quads
are individually cullable, which makes a frustum rejection of the leftmost quad
the obvious candidate cause.

**Metric** (`tools/recompone-v8-2/measure_backdrop_edge.py`): the backdrop's
leftmost sx per frame. Full coverage requires sx <= -54.

| | value |
|---|---|
| median left edge | -121.0 |
| median right edge | +509.0 |
| dumps where it stops short of the left frame edge | **9 / 31** |

Episodic, in ~29% of frames - which matches the intermittent nature of the
reported artifact. This is the same family as the terrain edge holes already
fixed: geometry authored to fill a 320-wide window failing to fill 428.

### T56 — the backdrop metric responds to the object-visibility slack

The first lever in this investigation that moves the artifact metric. Three
arenas, 900 frames each, 30 dumps per setting:

| `RECOMPONE_V82_OBJECT_CULL_SLACK` | backdrop stops short | median left edge | left band ratio | right band ratio |
|---|---|---|---|---|
| 0 (retail culling) | 9/30 | -93.8 | 0.895 | 1.013 |
| 2.5 (shipped) | **3/30** | -175.9 | 0.928 | 0.996 |

So `WidenObjectVisibilityTest` was already reducing this artifact - it just was
never measured against a metric that could see it. Every earlier attempt scored
it on near-depth histograms, which are a proxy that never moved.

This also confirms the mechanism: the backdrop quads are rejected by
`func_8002E22C`, and loosening that test keeps more of them. Adding radius
slack is a workaround for planes that are the wrong shape, though; the planes
themselves are built from the authored 320-wide clip width in `func_8002DFF0`,
which is what `ExpandObjectFrustum` corrects.

### T56b — the slack lever saturates

| `OBJECT_CULL_SLACK` | backdrop stops short | left band ratio |
|---|---|---|
| 0 (retail) | 9/30 | 0.895 |
| 2.5 (shipped) | 3/30 | 0.928 |
| 40 | 5/31 | 0.924 |

Slack 40 is no better than 2.5, which is why the manual A/B at slack 40 looked
identical to the reporter. Padding an object's radius cannot compensate for
frustum planes that are the wrong shape: it scales with the object's own size,
not with how far the view extends past the authored edge. The residual - three
frames in thirty, and a left band still 7% short - is what the plane fix has to
close.

### T57 — widening the object/scenery frustum planes (the fix)

`ExpandObjectFrustum` / `RestoreObjectFrustum` bracket `func_8002DFF0` and
widen `gp+0xEDC` across the plane build only, then put it straight back -
exactly the shape of the existing `ExpandTerrainFrustum` pair. Logged at
runtime as `native=320 expanded=427 scale=1.333333`.

Three arenas, 900 frames each, shipped cull slack in both arms:

| | backdrop stops short | left band ratio | right band ratio |
|---|---|---|---|
| planes unchanged | 13/31 | 0.766 | 1.048 |
| planes widened | **5/30** | **0.900** | 1.079 |

Short-backdrop frames fall from 42% to 17% and the left-band deficit more than
halves. The right band is unaffected, as it should be - it was never short.

This is the first change in the investigation that attacks the cause rather
than compensating for it: the object cull slack pads each object's radius,
which scales with the object's own size instead of with how far the view
extends past the authored edge, and saturates (T56b). The planes are simply the
wrong width, and this makes them the right width.

### T57b — scale beyond the aspect ratio buys nothing

| `OBJECT_FRUSTUM_SCALE` | backdrop stops short | left band ratio |
|---|---|---|
| 1.333 (aspect ratio, default) | 5/30 | 0.900 |
| 1.6 | 9/31 | 0.914 |

Within run-to-run variance, so the extra headroom is not doing anything. Unlike
the terrain row padding - which needed 4 cells because it pads a *cell grid*
and a partially covered cell still has to be visited - the frustum planes are
continuous, so matching the view exactly is both principled and sufficient.
Default stays at the aspect ratio.

Regression checks with the planes widened: terrain edge holes remain **0**, and
the cost is negligible - mean world triangles 3362 -> 3407 (+1.3%), objects
culled 24.0% -> 23.3%.

### T58 — instrumentation gated before shipping

Several probes added during this investigation sat in per-vertex or
per-primitive paths and ran unconditionally:

| probe | frequency | now gated behind |
|---|---|---|
| `_rtpTotal` / `_rtpNear100` / `_rtpNear60` / `_lastH` | every vertex projection (~11.5M per run) | `RECOMPONE_V82_TRACE_NEAR_PROJECTION` |
| `FlagRegisterReads` | every GTE FLAG read (~1400/frame) | same |
| near-depth histogram | every drawn opaque/alpha-test primitive | `RECOMPONE_V82_TRACE_NEAR_DEPTHS` |
| object census | a dictionary write + 3 reads per object per frame | `RECOMPONE_V82_CENSUS_OWNERSHIP` |

All were worth having - the projection counters are what established H = 256,
and therefore that the divide saturates below depth 128 - but none should cost
anything in a shipping build. The near-depth histogram is superseded outright
by the band-coverage metric, which reads the geometry dump offline rather than
counting during rendering.

### T59 — keeping the shipping set honest

`RECOMPONE_V82_NEAR_FLAGS` (withholding GTE FLAG bits 13..18 during widescreen
projection) was left defaulting **on** after T44, but it was only ever scored
against the near-depth histogram - the proxy that never moved for anything.
Defaulted off. It changes game-visible GTE flag state and must not ride along
untested inside the frustum fix.

Shipping set after this session is therefore exactly one behavioural change:

| switch | default | status |
|---|---|---|
| `ExpandObjectFrustum` (`OBJECT_FRUSTUM_SCALE`) | on, aspect ratio | measured against backdrop + band metrics |
| `NEAR_FLAGS` | off | unproven, retest against the band metric |
| `NEAR_KEEP` | off | measured inert (T42 corrected) |
| `WIDE_PRECISE_NCLIP` | off | measured inert (T39 outcome) |
| `MARK_NCLIP`, `TRACE_*`, `CENSUS_OWNERSHIP` | off | diagnostics only |

The in-flight 18-arena validation varies only `OBJECT_FRUSTUM_SCALE`, so its
comparison remains valid; a final confirmation run of the exact shipping
configuration follows once `NEAR_FLAGS` is settled.

### T60 — paired captures, and the residual

Every geometry dump now triggers a presentation capture labelled
`gameplay_NNN`, with the frame->index mapping logged on the
`[V82GeometryDump]` line. The label matters: the soak harness deletes stray
`recompone_present_*.ppm` unless it matches one of the patterns it preserves.
`tools/recompone-v8-2/pick_worst_frame.py` then returns the picture for
whichever frame the metric rates worst - the artifact is episodic, so a capture
at a fixed point usually misses it.

First use of it found a clean example immediately: Wild West, a black wedge in
the upper-left where the panorama fails to reach the frame edge. **That run had
the frustum fix enabled**, so widening the planes reduces how often this happens
without eliminating it.

**Why it is not eliminated.** The panorama is two quads spanning
sx -6.2..623.9 - 630 units wide for a 428-wide frame. It is not too small, it
is *offset*: short by 48 on the left, wasting 250 on the right. A further
segment exists to the left and is not being drawn.

**Emitter still unidentified.** It is not `func_80050B38` (scenery), not
`func_8003150C`, not `func_8001C910` - all three were bracketed with packet
ownership and none claims it. Its packets (`000B9270`, `000B9298`) sit adjacent
to the HUD's (`000B9310`), so it is written late in the frame and sorted behind
the world by ordering-table depth rather than by draw order. Finding it is the
next step for closing the residual.

### T61 — full 18-arena validation, shipping arm

121 dumps (four times the earlier sample):

| check | result |
|---|---|
| arenas reaching and completing gameplay | **18 / 18** |
| terrain edge-hole pixels | **0** (no regression) |
| backdrop stops short of the left frame edge | 45 / 121 (37%) |
| left widescreen band coverage ratio | 0.854 |
| right widescreen band coverage ratio | 0.971 |

**The earlier 5/30 was a small-sample fluke.** At n=121 the residual is 37%,
not 17%. The frustum fix is safe and helps, but on its own it leaves the
artifact present in more than a third of frames. Sample sizes below ~100 dumps
have repeatedly produced misleading numbers in this investigation (T56b, T57b);
treat anything smaller as a smoke test, not a measurement.

### T62 — the backdrop fill, and how to get a real before/after

**Determinism.** Restoring `V8_2_LOOSE/interface.ini` before each run brings
two identical runs to **0.99%** pixel difference, against 64.68% without it
(T40). The divergence was carryover in the game's own saved state, not
timing. Determinism holds for roughly 300 gameplay frames and is gone by 700,
so paired captures must come from short runs and the pairing must be verified,
not assumed - `build_backdrop_before_after.py` rejects any pair that differs by
more than 10% outside the strip the fix touches.

**Two bugs in the first attempt**, both silent:

1. the trigger threshold computed to 53 while the strip's packed left edge sits
   at 54, so it never fired;
2. more importantly it moved `GlVertex.X`, but backdrop vertices carry exact
   GTE provenance and the shader reconstructs their position from `ViewX` -
   `X` is ignored. Moving the panorama means solving
   `ViewX = (target - centre) * ViewZ / scale`.

Rewritten in reconstructed-screen space, with `ReconstructedX` /
`SetReconstructedX` making that explicit.

**Result** (Wild West, `RECOMPONE_V82_BACKDROP_FILL`):

| run length | fill off | fill on |
|---|---|---|
| 700 frames, 24 dumps | 8/24 stop short | **0/24** |
| 300 frames, 25 dumps | 6/25 stop short | 2/25 |

Verified visually on matched frames: the black wedge in the frame corner is
replaced by continued sky. The shader clamps UV to the texture bounds, so the
extension edge-clamps rather than tiling - correct-looking sky, and
unambiguously better than bare background.

### T63 — corrupted captures are an instrumentation race, not a game bug

Some paired presentation captures come back as structured VRAM garbage -
texture-page blocks and stripe patterns instead of the scene - and once it
starts in a run it persists to the end.

It is the capture, not the game:

| capture path | corrupt |
|---|---|
| harness-triggered native `.gameplay.ppm` | 1 / 485 |
| paired presentation capture added here | 15 / 499 |

The paired request is issued from the render backend's end-of-frame block and
can race the presentation readback. It cannot affect gameplay, and it cannot
affect any metric in this investigation - those are computed from geometry
dumps, not from images - but it can poison a proof screenshot, so
`build_backdrop_before_after.py` now rejects any frame whose capture is
corrupt as well as any pair where the runs had diverged.

### T64 — final validation, both fixes, 18 arenas

| | control | both fixes |
|---|---|---|
| arenas reaching and completing gameplay | 18 / 18 | 18 / 18 |
| terrain edge-hole pixels | 0 | 0 |
| backdrop stops short of the left frame edge | **37 / 123 (30%)** | **0 / 121** |
| left widescreen band coverage ratio | 0.873 | **0.955** |
| right widescreen band coverage ratio | 0.981 | 0.975 |

The artifact is gone on the metric that measures it, and the left/right
asymmetry that identified the bug in the first place has closed: 0.955 vs 0.975
where it was 0.873 vs 0.981. The terrain edge-hole fix is unaffected.

**Shipping build:** `V8_2_LOOSE/Vigilante82PC.exe`, SHA256
`9c29faf641578b125238b8d74783c83b7cf7c4fe8862d96ebe399d62bca20784`.

Two behavioural changes, both widescreen- and gameplay-scoped:

* `ExpandObjectFrustum` / `RestoreObjectFrustum` - widen `gp+0xEDC` across
  `func_8002DFF0` so the object and scenery frustum planes match the view
  being rendered, then restore it. Disable with
  `RECOMPONE_V82_OBJECT_FRUSTUM_SCALE=0`.
* `BackdropFill` - carry the panorama's outermost quad to the frame edge in
  reconstructed screen space, advancing U at the quad's own rate. Disable with
  `RECOMPONE_V82_BACKDROP_FILL=0`.

Everything else tried in this investigation is defaulted off with its
measurement recorded (T59).

### T65 — the reported artifact, finally identified

The reporter supplied the decisive fact: **it only happens when the car is
close to an object.** Their capture (frame 2756) shows the near wall as 32
single-triangle packets whose nearest submitted vertices sit at depth
**113-118, sx 128.8** - exactly where the cut appears on screen. Nothing nearer
is emitted at all.

`H` is 256, so `Gte.Divide` saturates below depth 128. The engine reads the
GTE `FLAG` register and skips the primitive whenever the error summary bit is
set (fed by bits 13..18: packed saturation, MAC0 overflow, divide overflow). A
wall the car is driving alongside trips it and is sliced off where it passes
closest to the camera.

`RECOMPONE_V82_NEAR_FLAGS` withholds those bits during Enhanced widescreen
projection. **It had been defaulted off in T59** as unproven - the right
instinct applied with the wrong evidence, because it had only ever been scored
against near-depth histograms, which cannot see this. Scored against
`measure_near_cutoff.py`, Wild West, 700 frames: nearest submitted wall vertex
**72.9 -> 26.2**.

### T66 — final validation, all three fixes, 18 arenas

| metric | control | shipping |
|---|---|---|
| arenas completing gameplay | 18/18 | 18/18 |
| terrain edge-hole pixels | 0 | 0 |
| backdrop stops short of the frame edge | 41/121 | **4/121** |
| left widescreen band coverage ratio | 0.894 | **0.962** |
| wall/prop vertices below depth 110 | 44 | **72** |
| nearest wall vertex overall | 5.0 | 2.5 |

**Honest limit:** the count of *frames* reaching below depth 110 barely moves
(11 -> 12). The autodrive almost never pins the car against a wall, so the
harness under-samples precisely the situation the artifact needs. It can
establish direction, not magnitude. Confirmation has to come from play.

**Shipping build:** `V8_2_LOOSE/Vigilante82PC.exe`, SHA256
`31ce1fa7b9925b899f1e0309...`. Three changes, all gated to Enhanced widescreen
gameplay: `NEAR_FLAGS` (the reported artifact), `ExpandObjectFrustum` and
`BackdropFill` (a genuine but separate widescreen defect found on the way).

### T67 — the actual root cause: packed NCLIP on saturated coordinates

Attribution through indirect calls (`RECOMPONE_V82_TRACE_INDIRECT`, which
brackets every `Dispatcher.Call` with the packet pointer) named the mesh
emitters: `0x80022A4C`, `0x80022C54`, `0x800229A0`, `0x80022870`. They are
reached by function pointer per model type, which is why no static search ever
found them.

`func_80022A4C` opens with:

    Gte.Execute(NCLIP)        // signed area from the packed SX/SY
    c.LoadGteWord(9, 24)      // MAC0
    if ((int)c.T1 <= 0) { ...discard this triangle... }

`SatX`/`SatY` clamp packed coordinates to ±1024. Geometry passing close beside
the camera projects far off-screen, two or three vertices clamp to the *same*
limit, the triangle reads as degenerate or backfacing, and the emitter throws
it away. **That is the reported artifact**, and it explains the one clue that
never fitted anything else: it only happens when the car is close to an
object, because only then does the projection saturate.

**Why the existing fix was inert.** `WIDE_PRECISE_NCLIP` computed its area from
`SxyPreciseX/Y` - which are clamped to the same ±1024 (Gte.cs:731). It was
reading the same degenerate triangles as the packed test, so it could not
disagree with it. T39 recorded it as inert and it was defaulted off; the
measurement was right, the implementation was not.

`SxyUnclampedX/Y` now carry the projection without the representable-range
clamp, purely for the area test. Effect over 542 frames:

| | object sign disagreements | front faces the packed test deleted |
|---|---|---|
| clamped (before) | 0 | 362,394 |
| unclamped | **325,490** | **654,287** |

The near-cutoff metric cannot arbitrate this - the autodrive never pins the car
against a wall, and 24-frame samples move in both directions. Confirmation has
to come from play.

### T68 — the unclamped NCLIP shredded the terrain; reverted

Enabling `WIDE_PRECISE_NCLIP` with the unclamped projection destroyed the
ground in play - the terrain rendered as chaotic overlapping triangles.
Reverted immediately; `V8_2_LOOSE` restored to `31ce1fa7`.

**What went wrong.** The unclamped coordinates were fed to *every* NCLIP,
including the terrain-scoped one. That path had been tuned against the clamped
projection to close the horizon slits, and changing the sign decisions it makes
breaks the walker's output. Scoping it so terrain keeps the clamped values and
only object geometry uses unclamped was built but **not** shipped: the reverted
state is the one that was known good in play, and a second speculative build is
not worth the reporter's time.

**What this cost, and the lesson.** Every measurement said the change was
sound: 325,490 object sign decisions corrected, 18/18 arenas passing, terrain
edge-hole metric still 0. **The edge-hole metric measures backdrop-coloured
pixels at the frame edges - it cannot see a shredded ground.** A soak that
reaches gameplay and a metric that stays green are not evidence that the
picture is right. Any change touching NCLIP alters what the engine submits, and
its blast radius reaches well beyond the primitive being tested.

Before this switch is ever enabled again it needs a whole-frame comparison
against a known-good capture - not a pass/fail soak and not an edge metric.

### T69 — three metrics that could not see a destroyed picture

The unclamped NCLIP shredded the ground in play (Route 66). Checked afterwards
against the harness runs of the same build:

| metric | broken build | known good | detects it? |
|---|---|---|---|
| terrain edge-hole pixels | 0 | 0 | no |
| terrain median triangle area | 12.4 px² | 12.1 px² | no |
| oversized terrain triangles | 1.03% | 0.91% | no |
| terrain triangles per frame | 1674 | 1778 | no (-6%) |
| 18-arena soak | 18/18 pass | 18/18 pass | no |

The visual signature is a zigzag: **alternating triangles culled by a backface
sign flip.** The survivors keep their size and shape, so every geometric
statistic stays normal. Only the picture shows it - and only in some arenas,
which is why the harness runs (Wild West) looked fine while Route 66 was
destroyed.

**Conclusion: no aggregate statistic over geometry is an adequate gate for a
change that alters culling.** It has to be a whole-frame image comparison
against a known-good build on the same frames
(`tools/recompone-v8-2/whole_frame_gate.py`), and that gate needs its own
control run to separate a real regression from soak divergence, because the
harness only stays in step for ~300 gameplay frames.

### T70 — the clipping fix, landed behind a working gate

**The gate needed a control.** `whole_frame_gate.py` first reported the
candidate at 97.7% whole-frame change - and a control run with *identical
config on both arms* reported 100%. It was measuring soak divergence, not the
change. Capping the comparison to the deterministic window (capture index <= 4)
and excluding captures corrupted by the readback race (T63) gives:

| | worst whole-frame change |
|---|---|
| control (identical config) | **1.8%** |
| candidate (scoped NCLIP) | **2.0%** |

Within noise. Confirmed visually on Route 66 and Nugget City: the ground is
intact. Two of the three numbers that condemned this change were artefacts of
the harness, and only the control run separated them from a real regression.

**The fix.** NCLIP's area is computed from `SxyUnclamped*` for object geometry;
terrain keeps `SxyPrecise*` (clamped), which is what it was tuned against.
Effect, same runs:

| | object sign corrections | front faces rescued |
|---|---|---|
| off | 0 | 79,350 |
| scoped on | **229,457** | **302,734** |

Safe and doing its job. `RECOMPONE_V82_WIDE_PRECISE_NCLIP=0` disables it.

**Staged:** `V8_2_LOOSE/Vigilante82PC.exe`, SHA256 `1a494ff9ce2708c63d025627...`

### T71 — bypassing retail culling needs the renderer to cull

Authorised to disconnect the retail pipeline rather than keep patching its
gates one at a time. A prerequisite surfaced immediately:

**The Enhanced renderer does not cull back faces.** It calls
`_gl.Disable(EnableCap.CullFace)` and relies entirely on the engine's NCLIP
test to remove them. So neutralising that test submits every interior face as
well as the geometry we want back.

That makes `RECOMPONE_V82_NO_RETAIL_CULL` a diagnostic rather than a shippable
change: it answers whether NCLIP is the gate holding the reported wall, but it
would draw model interiors if shipped alone.

A shippable bypass therefore has two halves:

1. stop the engine rejecting primitives (forcing NCLIP positive), and
2. cull back faces in the renderer, from the reconstructed camera-space
   positions it already draws from - where the arithmetic is float and the
   coordinates are not clamped to +/-1024.

Half 2 needs care: some geometry is legitimately double-sided, so a blanket
GL cull would remove wanted faces. The camera-space signed area per primitive
is the same decision NCLIP makes, just computed where it is reliable.

### T72 — a headless reproduction, at last

The reporter's clarifications reframed the artifact completely:

* it happens at **4:3 as well** - so it is not a widescreen bug, and every
  widescreen-gated fix built for it could never have fired;
* it is **polygon-level**, not object-level;
* the trigger is **camera-to-object distance**;
* it has **always** happened - not a regression.

`tools/recompone-v8-2/measure_triangle_retention.py` reproduces it without any
manual testing: track each object across a run's dumps and compare its triangle
count when far against when near. Objects lose 33-83% of their triangles as
they approach - e.g. `806BD510`, 33 triangles at depth 1864 down to 6 at 525.

Baseline: **13% of tracked objects lose more than 30%** of their polygons when
close.

### T73 — only removing the engine's backface test helps

| configuration | objects losing >30% |
|---|---|
| engine culls normally, coordinate clamp on | 12/93 (13%) |
| clamp off | 15/89 (17%) |
| **engine backface culling disabled** | **7/93 (8%)** |
| engine off + renderer-side cull, one winding | 23/94 (24%) |
| engine off + renderer-side cull, other winding | 17/93 (18%) |
| engine culls from unclamped coordinates | 21/89 (24%) |

Differences of a few objects out of ~90 are within noise; the pattern is not.
Every attempt to *replace* the engine's culling with a supposedly better one
makes retention worse, while simply removing it improves it.

**Reading:** the replacement test is computed on the wrong vertex order. NCLIP
uses the GTE's register order, while the renderer receives packet order, and
packets may store vertices differently. A single global sign cannot then be
right for a mixed population - which is exactly the observed result, both signs
worse than baseline.

**Next:** establish the vertex order the engine's NCLIP actually sees, by
recording SX/SY at the NCLIP instruction alongside the packet the emitter
writes, and comparing. Until that correspondence is known, renderer-side
culling cannot replace the engine's, and disabling the engine's leaves back
faces drawn - which the reporter observed as z-fighting.

## T74 — Quest-mode crash (TO-DO #3) root cause

Smoked quest mode across all 18 character slots. 16 passed; slots 5 and 11 died
identically:

```
System.InvalidOperationException: unmapped address: 0x24020D0E
  PSMemory.ReadU8 -> func_800196B8 -> func_80103F10 -> func_80033550 -> func_800132CC
```

`0x24020D0E` is a MIPS instruction word (`addiu v0, zero, 0x0D0E`), not a
pointer. `func_800196B8` is the text formatter and walks it byte by byte.

### The chain

Instrumenting `S5` (the string argument) inside `func_80103F10` showed it was
already garbage on entry, so the caller supplies it. The live caller chain is
`func_80011540 -> func_80014FD8 -> func_800132CC -> func_80033550 ->
func_80103F10`, and the argument is built in `func_800132CC`
(main.cs:2905-2921) by a two-level table walk:

```
A2 = (sbyte)[gp+0x1104]            ; quest record index
A1 = [gp+0x1034]                   ; table base
A1 = [A1 + A2*4 + 0x10]            ; record pointer
A3 = (sbyte)[gp+0xC3C]             ; sub-index
A1 = [A1 + 12*A3 + 8]              ; string pointer
```

Traced values at the failure:

```
[QuestText] bad string ptr 0x24020D0E table=0x807F4580
            recIndex=5 subIndex=0 record=0x00000000
```

**The record pointer is NULL.** With `A1 = 0` and `A3 = 0`, the second load
reads `[0x00000008]` — the PS1 exception-vector region — and gets a BIOS
instruction word back, which is then treated as a string. So the defect is a
missing quest record for that index, and everything downstream is the game
faithfully dereferencing whatever it found at address 8.

Note the earlier hypothesis in this file's working notes — that
`func_80103F10`'s random 53-entry taunt table at `0x8010A390` was the source —
is wrong. That branch only runs when the caller passes NULL, and a dump hook
placed on it never fired.

### Why the record is NULL: those three are unlockables

Dumping the record table for a failing slot (5) and a passing slot (4) gives
byte-identical output, so this is not an artifact of the harness injecting the
character through `RECOMPONE_V82_PLAYER_TYPE`. The table declares
`count = 18` with NULL at 5, 11 and 17, and the relocation loop that populates
it (main.cs:123231) **explicitly skips zero entries**, so the zeros come
straight from the authored data file.

Dumping the engine's type table at `0x8006383C` (stride 0x10, indexed by the
same type byte) names them:

```
[ 0] Sheila   [ 1] Torque   [ 2] Trio     [ 3] Houston  [ 4] Convoy
[ 5] Cultsmen <== no briefing
[ 6] Dallas   [ 7] Nina     [ 8] Molo     [ 9] Clyde    [10] Obake
[11] Boogie   <== no briefing
[12] BobO     [13] Garbage  [14] Chase    [15] Chassey  [16] Padre
[17] Dusty    <== no briefing
[18] Easy     [19..20] past the end of the table
```

Cultsmen, Boogie and Dusty are the three **unlockable** characters, which is
why the roster is 15 + 3 = 18. They have no Quest campaign, so the game ships
no briefing record for them, and the NULL is authored rather than missing.
Reading each record's first string confirms the other fifteen are the base
roster ("Whoa, this ol' trucker..." for Convoy at 4, "Chassey is still somewhat
bitter about her fall from grace in Hollywood" at 15, and so on).

So there was no data defect. The smoke harness was installing an unlockable as
the Quest player through `RECOMPONE_V82_PLAYER_TYPE`, and quest mode then
dereferenced the NULL record. `run_quest_smoke.py` now smokes the fifteen
Quest-capable characters by name.

Worth keeping in mind: if Quest mode does let a player pick an unlockable once
it is unlocked, this same path is reachable in normal play. The `PSMemory` read
fix means it no longer kills the process -- the briefing renders blank instead.
That has not been tested through the real carousel.

### Mitigation shipped

`PSMemory.Resolve` now distinguishes reads from writes. An unmapped **read**
logs once and returns zero; an unmapped **write** still throws, because a bad
write corrupts state and must be caught. Hardware behaves the same way: it
returns whatever the bus yields and the game carries on. Set
`RECOMPONE_STRICT_UNMAPPED_READS=1` to restore the old fatal behaviour when
hunting bugs.

With that in place all 18 slots complete a full quest run. The failing two each
log exactly one `[Memory] unmapped read at 0x24020D0E size=1; returning zero`;
the zero terminates the string, so the affected line renders blank instead of
killing the process.

### On the original report

The TO-DO entry was "Game froze when starting Chassey Blue's quest". Chassey is
type 15 and has a valid briefing record, so the NULL-record crash found here is
not reachable through her. Her quest starts and runs clean in the smoke. The
crash that was fixed is in exactly that briefing code path, but whether it is
the same freeze that was originally observed cannot be established from here.
