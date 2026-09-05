# V8:2 Dreamcast terrain-distance reverse engineering

## Current translation — 2026-09-04, pending visual approval

Candidate `71618E6B89D311CC47522B0AAB1A8EAB873E5E054B6E3A5D426ECBBE4B0FA8DD`
replaces the withdrawn forced-four-unit implementation with the shared
`DreamcastTerrainGeometry` builder. It consumes 25 authored samples per outer
patch, recursively emits 4/2/1 leaves, preserves parent decision depth, and
applies the recovered height/color morph before projection. Each textured
one-unit leaf selects its own material and authored diagonal. The common
loader snapshots authored XTIN averages before native scratch mutation.

The camera snapshot uses native terrain origins, GTE rotation/translation,
projection center and scale; it does not contain map or vehicle identities.
Maximum-depth rejection applies at root and textured leaf submission, not
prematurely at two-unit subdivision nodes.

Binary checks in `tools/dreamcast_terrain_contract.py` pass. Focused tests in
`artifacts/v82-dreamcast-subdivision-tests` pass for both tuple modes, mixed
depths, the height-morph tail, root rejection, and visible interior geometry
inside a two-unit node whose parent corners are behind the camera.
The ordinary Enhanced Hoover traversal gate passed on this exact executable
(59.96 median / 55.70 minimum FPS). Final five-map evidence is recorded in
`notes/v82_map_fidelity_visual_proofs.md`; no visual acceptance is implied.

This translates the recovered distance/subdivision/material contract; it does
not establish bit-exact Dreamcast PVR rasterization or near-plane equivalence.
The historical omission below explains why the preceding candidate was withdrawn.

## Correction: subdivision was omitted from the previous translation

The September 4 continuation followed `0x8C10238C` through its child-work
stack and back to `0x8C1020A6`. This invalidates the earlier claim that the
four-unit outer traversal implies a uniform four-unit rendered mesh. Candidate
`13316E77...D29C4` is **not qualified for Dreamcast terrain parity**. Its five
screenshots remain evidence of that candidate, not a completed translation.

Binary-anchored decision reference: `tools/dreamcast_terrain_contract.py`.
It verifies the executable hash and 13 relevant SH-4 instructions, then reads
the parameter tuples directly. Running it against `1ST_READ.BIN` passes.
The file named `1ST_READ.unscrambled.bin` is a different, incorrectly
descrambled artifact and must not be used for this analysis.

Recovered control flow (HIGH, instruction-traced):

- `0x8C101FC4`: initial subdivision state 2; `0x8C1020A2`: width 4.
- `0x8C1020A6..0x8C1020B4`: tuple address is `GBR+20+32*state`;
  state zero enters the textured material path.
- `0x8C1020B6..0x8C1020DE`: obtain the minimum of four corner depths;
  if it exceeds tuple `+8`, submit an untextured patch at `0x8C10227A`;
  otherwise subdivide at `0x8C10238C`.
- `0x8C10238C..0x8C102446`: halve the patch width, fetch five intermediate
  authored height records, construct their parent-surface midpoint heights.
- `0x8C10244A..0x8C1026B4`: transform/morph those vertices and construct
  their colors using the distance tuples and color interpolation helpers.
- `0x8C1026C6..0x8C10270E`: decrement state, push three child patches,
  and evaluate the first child. `0x8C102308..0x8C102342` drains that stack.

| Mode | State 1 / width 2 cutoff | State 2 / width 4 cutoff |
| --- | ---: | ---: |
| Normal | 25.2 | 34.3 |
| Alternate | 19.6 | 34.3 |

These compare the minimum depth, not center depth or radial distance. Near
patches reach width 1 and select their own top-left material. A uniformly
depth-10 four-unit patch produces sixteen textured one-unit leaves; depth 26
produces four untextured two-unit leaves; depth 35 produces one untextured
four-unit leaf. Mixed-depth patches must follow the recursive decisions.

`0x8C101EC0` is a subdivision vertex height-morph/transform helper, not an
edge-intersection clipping helper. It consumes one vertex plus distance and
projection parameters, updates height, transforms again, and returns a blend
factor. The literals at `0x8C101E0C` and `0x8C101F74` are 0.01; the separate
quad-rejection literal at `0x8C102380` is 0.015. Exact near-plane equivalence
still requires verification beyond this host renderer's clipping tests.

The current candidate implements the shared recursive 4/2/1 geometry,
authored intermediate heights, distance morph, and per-leaf material/color
contracts. User visual qualification remains required; no map- or
content-specific corrections are permitted.

### Recovered morph operation

`0x8C101EC0..0x8C101F2A` first transforms the parent-interpolated vertex.
With `remaining = farEnd - parentDepth` and
`f = (span - remaining) * inverseSpan`, its in-range height operation is:

```
weight = 0                         when span - remaining <= morphTail
weight = (4*f - 3)*f               otherwise
height = authoredHeight + (parentHeight - authoredHeight)*weight
```

Vertices beyond `farEnd` keep their parent height. Vertices before the
transition keep authored height. The helper transforms the changed height
again before projection. This is a geometry change and cannot be reproduced
by the current shader's texture/color fade. Normal state-2 height morph spans
33.55..34.3 units; alternate state-2 spans 32.8..34.3. State-1 height morph
spans 23.95..25.2 or 18.35..19.6 respectively. These are parent-depth-derived
ranges; a whole patch is not selected from a single representative depth.

Fresh full disassembly of `0x8C1017C0..0x8C101878` confirms the color helper
computes `authoredColor*(1-f) + (parentAColor+parentBColor)*(f/2)`, then
truncates components when packing. `0x8C101700` averages the two parents;
`0x8C1016A0` loads the authored color. The subdivision caller selects these
helpers separately for states 1 and 2, and the texture-fade factor at vertex
offset 56 must be preserved alongside the locally interpolated flat color.

## Scope and inputs

This note covers the complete Dreamcast terrain path that changes with camera
distance: visible-footprint construction, clipping, mesh/material cadence,
per-vertex terrain color construction, distance-factor selection, PVR
base/offset color submission, and distance-dependent texture state. It does
not claim to reproduce unrelated Dreamcast object rendering or the entire PVR
hardware rasterizer.

- Archive: `Vigilante 8 - 2nd Offense v1.000 (1999)(Activision)(US)[!] - Dreamcast.zip`
- Executable: `1ST_READ.BIN`
- Executable SHA-256: `06F5A6FCDC9F8E0EF486A1CE1DF7FCF61F5A756DBEF7248139291AE520E3D3E7`
- Dreamcast Hoover Dam asset SHA-256: `17B888B58AA7CFBB8CD4AC48123A60D1F24ECB6E9C1B304567170ED68B84C25B`
- Analysis image base: `0x8C010000`
- Processor: little-endian SH-4

The extracted executable, GDI index, Ghidra project, decompilations, literal
dumps, and Capstone disassembly are retained under
`artifacts/dreamcast-render-distance-reference/`.

## Visible footprint and clipping

The frame scene function at `0x8C0E27E0` calls the terrain renderer at
`0x8C0955A0`. The renderer constructs the camera-ground footprint, clips it
through the edge table at `0x8C113EA8`, and passes the resulting polygon to
`0x8C095140`.

Recovered distance-related constants and behavior:

- `0x8C0957CC`: fixed `80.0f` terrain-view extent.
- `0x8C0957D0`: `47.96875f` clip constant.
- `0x8C0957D4`: `-0.03125f` clip constant.
- `0x8C0957D8`: `0.03125f` matrix scale.
- `0x8C101F74`: `0.01f` projection depth check in the morph helper;
  `0x8C102380`: separate `0.015f` quad-rejection threshold.
- `0x8C101EC0`: subdivision vertex height-morph/transform helper; the earlier
  edge-intersection classification was incorrect.

The 80-unit footprint is fixed. The renderer does not extend detailed terrain
indefinitely and it does not use an authored per-map distance.

## Mesh and material cadence

`0x8C095140` aligns terrain coordinates to multiples of four, walks both axes
in four-unit increments, resolves 20-byte terrain records through the 64-by-64
zone pointer grid at `0x8C28D160`, and submits each four-unit quad through
`0x8C101FA0`.

- `0x8C0951E0` / `0x8C095580`: `4.0f` scan step.
- `0x8C095484` / `0x8C095574`: `0x0500` zone-record stride component.
- `0x8C095486` / `0x8C095576`: `0x0050` four-record row increment.
- `0x8C09548C` / `0x8C09557C`: `0x8C101FA0` quad submission.

This is the outer traversal cadence only. The submitter recursively selects
four-, two-, or one-unit leaves using the distance conditions recorded above.

At `0x8C1020E2`, `0x8C101FA0` reads material byte `+6` from the quad's
top-left terrain record, indexes the 48-byte material table at `0x8C289958`,
and submits one texture across the quad. Descriptor flags at `+30` select the
direct/clipped path and diagonal; neither flag changes at a distance. The
Dreamcast does not reconstruct the sixteen one-unit PS1 `XTIN` cells inside
that quad.

## Terrain color data

Dreamcast `0x8C084700` consumes the level's 32-byte `COLS` chunk and builds a
32-entry ramp at `0x8C2901D0`:

```
ramp[channel, shade] = low[channel]
                     + (high[channel] - low[channel]) * shade / 31
```

The division is signed integer truncation. `low` is `COLS` bytes 12..14 and
`high` is bytes 16..18. The PS1 loader keeps those same byte-identical words at
`gp+0xE04` and `gp+0xDAC`, respectively. The shade index is the upper five bits
of the terrain height word.

`0x8C085100` stores each `XTIN` descriptor's average RGB. `0x8C085420` builds
the terrain record and calls `0x8C084540`, which constructs the local flat
color at each terrain vertex from the four material cells meeting there:

```
localFlat[channel] = trunc(
    ramp[channel] *
    (average00[channel] + average01[channel]
     + average10[channel] + average11[channel]) / 1024.0)
```

The literal at `0x8C0845C8` is exactly `1/1024`. Missing page-edge neighbors
use material zero, matching the native lookup fallback. This is a local,
level-data-driven color; it is not a fixed fog color and does not require a
map exception.

## Exact distance transition

`0x8C0955A0` copies one of two 96-byte distance parameter blocks through
`0x8C100392`. Selector `0x8C1133F0 == 0` uses the normal block at
`0x8C113DE8`; nonzero uses the alternate block at `0x8C113E48`. The PS1 port's
equivalent signed view-mode byte at `gp+0x31` selects the corresponding
80/40/20 or 48/24/12 setup, so the translation uses that direct mode state and
does not infer it from a map or projected polygon.

For each transformed vertex, `0x8C102780` computes:

```
remaining = farEnd - depth
factor = 0                                  when remaining >= span
factor = 1 - remaining * inverseSpan        when 0 < remaining < span
factor = 1                                  when remaining <= 0
```

The relevant first-tuple parameters are:

| Mode | farEnd | span | inverseSpan | factor begins |
| --- | ---: | ---: | ---: | ---: |
| Normal | 25.2 | 5.0 | 0.2 | 20.2 |
| Alternate | 19.6 | 5.0 | 0.2 | 14.6 |

One Dreamcast terrain unit equals 256 PS1 world/view units in these
byte-identical converted levels.

`0x8C101E20` converts the factor into the PVR base and offset colors. Its
literals are exactly `0.75f` and `1.3333334f`:

```
factor <= 0:
    base = ramp
    offset = 0

0 < factor < 0.75:
    base = trunc(ramp * (0.75 - factor) / 0.75)
    offset = trunc(localFlat * factor / 0.75)

factor >= 0.75:
    base = 0
    offset = localFlat
```

The PVR combines these as `texture * base + offset`. Consequently the visible
normal transition is 20.2..23.95 units and the alternate transition is
14.6..18.35 units. Past that transition, terrain remains as the locally
computed flat color to the 80-unit footprint clip.

## Texture state

`0x8C095040` builds the terrain PVR material headers once before the walker and
reuses them for every quad. `0x8C101FA0`, `0x8C102780`, and `0x8C101E20` do not
alter filtering, texture format, mip selection, or material header bits based
on depth. The native distance behavior is therefore the mesh/color transition
above, not a hidden distance-dependent mipmap switch. Enhanced may use its
global bounded texture-filter option, but that state is uniform and does not
drive the terrain transition.

## Earlier Enhanced-renderer translation (incomplete; superseded above)

The global translation is:

1. Keep the PS1 port's native clipped traversal polygon and 80-unit normal
   extent; preserve widescreen lateral coverage separately.
2. Zero the PS1 `+0x98` and `+0x9A` representation thresholds at Maximum LOD,
   making its existing four-unit coarse emitter cover the visible polygon.
3. Select the top-left material once per four-unit quad.
4. Rebuild each vertex's `COLS` ramp and four-neighbor `XTIN` local-flat
   color from the authored level data. Preserve all 256 material average RGB
   entries at both common terrain-loader completion boundaries before arena
   animation can reuse the live PS1 descriptor bytes.
5. Compute the normal or alternate Dreamcast factor from view-space depth.
6. Send the exact base and offset channels to the Enhanced shader and evaluate
   `texture * base + offset` before the common framebuffer output stage.

If Maximum LOD / high-resolution 3D is disabled, the new Dreamcast color path
is not attached and the original packet colors remain intact. The shipping
path contains no arena, cell, texture, vehicle, object, or packet-address
exception.

## Historical validation (rejected candidate)

Shipping Release executable under test:

`5348CEE15D38E40A5A16AD84BAB7CA5D17A5B034769B2D268EBC09F60FD84A0B`

The single-process silent Enhanced gate ran Louisiana, Hoover Dam, and Utah
back-to-back from `V8_2_LOOSE` for 360 gameplay frames each. All functional,
HD-texture, traversal, packet-budget, renderer-purity, and performance checks
passed:

| Arena | Median FPS | Minimum FPS | Result |
| --- | ---: | ---: | --- |
| Louisiana | 59.91 | 53.94 | pass |
| Hoover Dam | 59.98 | 58.84 | pass |
| Utah | 60.09 | 57.67 | pass |

Packet-arena high water was 42.03%, with zero edge-pool drops and one game
process. The detailed acceptance report is
`artifacts/v82-dreamcast-terrain-full-re/three-map-release/acceptance.json`.
The retained Hoover visual proof is
`artifacts/v82-dreamcast-terrain-full-re/hoover-runtime/recompone_present_gameplay_0601_1280x720_fxaa.png`.

The user rejected this candidate's visual result. The historical technical
results above do not establish visual qualification.

## Current candidate and qualification limits (2026-09-04)

Deployed executable SHA-256, rechecked against all six current reports:
`13316E77BFFD4D32E73CC5E1F8820DA9FDF3164723F87FBE40FF698DF77D29C4`.

The current shared implementation also rejoins complete native coarse-terrain
triangle pairs before selecting the authored XTIN diagonal. The native PS1
emitter otherwise fixes that diagonal independently of Dreamcast metadata.
Genuinely clipped single halves retain the native triangle path; therefore
complete-quad diagonal coverage must not be described as a proof of exact
Dreamcast near-plane clipping equivalence. The translation retains the native
PS1 footprint traversal and widescreen coverage described above.

Evidence root: `artifacts/v82-shared-sheila-distance-goal/final-13316e77/`.
The ordinary `normal-gate-hoover/acceptance.json` passes on this exact hash.
The five 1920x1080 captures in `louisiana`, `hoover-dam`, `utah`, `florida`, and
`minnesota` are deliberately held for framing. Their reports correctly remain
overall false: `all_representative_routes` and `all_map_contracts` are false
because those runs do not traverse the routes. All other reported checks are
true; each report records one game process and zero renderer content-branch
hits. These captures are visual evidence, not five successful traversal gates.

The Minnesota frame still visibly contains the separately logged building
polygon defect. The user's visual approval of Sheila and terrain distance is
outstanding. Neither the binary analysis nor these technical checks closes the
goal or qualifies the rendered result on the user's behalf.
