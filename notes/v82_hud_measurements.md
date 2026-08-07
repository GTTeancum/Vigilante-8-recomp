# V8: 2nd Offense HUD measurements

These measurements come from the original Route 66 gameplay draw packets and
the unmodified Original-preset presentation. Coordinates and sizes are native
PS1 framebuffer pixels.

| Backing element | Screen rectangle | Atlas origin | Authored size |
| --- | --- | --- | --- |
| Radar and right connector | `(16, 16)` | `(0, 90)` | `64 x 55` |
| Vehicle/status, weapon, and ammo backing | `(80, 20)` | `(64, 90)` | `84 x 34` |
| Vertical health backing | `(24, 71)` | `(0, 145)` | `16 x 49` |

The status row is emitted at authored UV `(64, 90)`, tpage `0x005`, CLUT
`0x7804`, with texture-window masking disabled for this draw. Runtime
classification uses that complete native packet identity plus its exact
`84 x 34` dimensions; the former inferred `(104, 32)` identity was disproved
by the live packet trace.

The radar face occupies the left `55 x 55` pixels of its rectangle. Its center
is `(27.5, 27.5)` in local atlas coordinates. Inspection of the recovered
`hud1.bmp` source shows three interior range rings, with measured centers at
`10.5`, `18`, and `24` pixels. The SVG reconstruction uses those three circle
elements, a `27.5`-pixel framed outer extent, one-pixel axes and diagonals, and
a three-pixel center hub. Contacts remain separate source-driven game
primitives.

The backing colors are not flat presentation colors. Direct decoding of the
three gameplay CLUTs proves that every nontransparent backing texel has STP
set and each rectangle uses PS1 semitransparency mode 0. The correct result is
therefore half the palette-resolved source color plus half the existing
framebuffer. The source distributions are:

| Backing | Nontransparent / total | Native RGB5 material range |
| --- | ---: | --- |
| Radar and connector | `2589 / 3520` | 15 authored colors, principally dark green/grey `(1..14, 1..15, 1..14)` |
| Status, weapon, ammo | `2110 / 2856` | 13 authored green/black/purple colors `(1..13, 1..13, 1..13)` |
| Vertical health | `754 / 784` | 14 authored purple/brown colors `(1..14, 1..12, 1..13)` |

The clean radar face uses the measured non-grid upper RGB5 mean
`(2.355, 2.855, 2.120)` and lower mean `(5.830, 6.740, 4.985)` as a continuous
vertical material transition. This removes the source's concentric/radial
color noise without replacing its palette or blend response. Grid, hub,
borders, connectors, status/weapon gradients, ammo tail, and health housing
use gradient stops sampled from the exact palette-resolved source colors.
Continuous interpolation replaces native pixel steps between those authored
anchors, while the shader requantizes to RGB5 and retains mode-0 STP blending.

All three backing groups are classified by their exact authored rectangle.
Their silhouettes, interior gradients, frames, and radar grid are authored in
literal SVG documents: `v82_radar.svg`, `v82_status.svg`, and
`v82_health.svg`. A dependency-free in-tree SVG subset renderer rasterizes the
documents into a premultiplied 8x atlas and generates mipmaps, so downsampling
retains antialiased vector coverage rather than recreating native-resolution
steps or dark fringes. The documents are embedded as a fallback and exposed
under `SHARED/HUD` in the loose-files build. They are left-anchored as one
group. Colored status, weapon, ammunition, health, and live radar content
remains game-driven.

The Enhanced SVG design intentionally omits the retail connector ornaments
between radar and status, status and weapon, and radar and health. Each widget
retains its own closed frame with transparent negative space between groups.

## Enhanced texture projection

The rejected implementation associated primitive screen coordinates with
historic GTE output and could assign unrelated depths. That path was removed.
Enhanced textured quads now derive projective denominators only from their own
four screen/UV correspondences. Textured triangles remain affine because three
screen/UV correspondences do not contain enough information to infer a unique
projective transform. Stock/Original retains the authentic affine path.

Gameplay raster depth is independent of texture projection. It is seeded from
the original ordering-table bucket for every polygon and refined by a valid
quad's relative projective denominator. This preserves the game's painter
order while preventing later terrain polygons from covering nearer vehicle,
building, or prop geometry. UI remains outside depth testing. In Enhanced
rendering, stable depth remains active if projective texture correction is
turned off, so the affine option does not restore the clipping defect.
