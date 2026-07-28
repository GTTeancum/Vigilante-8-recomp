# V8: 2nd Offense HUD measurements

These measurements come from the original Route 66 gameplay draw packets and
the unmodified Original-preset presentation. Coordinates and sizes are native
PS1 framebuffer pixels.

| Backing element | Screen rectangle | Atlas origin | Authored size |
| --- | --- | --- | --- |
| Radar and right connector | `(16, 16)` | `(0, 90)` | `64 x 55` |
| Vehicle/status, weapon, and ammo backing | `(80, 20)` | `(64, 90)` | `84 x 34` |
| Vertical health backing | `(24, 71)` | `(0, 145)` | `16 x 49` |

The radar face occupies the left `55 x 55` pixels of its rectangle. Its center
is `(27.5, 27.5)` in local atlas coordinates. The clean reconstruction uses the
measured `27.5`-pixel outer radius, one-pixel axes and diagonals, a
`13.5`-pixel ring, and a three-pixel center hub. Contacts remain separate
source-driven game primitives.

The shader-stage RGB5 values and resulting presentation colors are:

| Surface | RGB5 | Presented RGB8 |
| --- | --- | --- |
| Radar and main HUD backing | `(16, 15, 12)` | `(132, 123, 99)` |
| Radar grid | `(19, 17, 14)` | `(156, 140, 115)` |
| Radar hub | `(21, 19, 17)` | `(173, 156, 140)` |
| Vertical health backing | `(9, 6, 5)` | `(74, 49, 41)` |

All three backing groups are classified by their exact authored rectangle.
Their complete row-measured silhouettes are continuous analytic paths: the
radar disk and connector, both status/weapon lobes and ammo tail, and the
vertical health housing. They are left-anchored as one group and rendered in
an opaque batch, so the PS1 STP stipple/grain cannot reappear. Colored status,
weapon, ammunition, health, and live radar content remains game-driven.

## Enhanced texture projection

The earlier Original/Enhanced camera comparison was inconclusive: both modes
were still taking the affine fallback for the reported depthless packets.
Enhanced mode no longer relies on that path for ordinary textured quads.

GTE screen depth is now retained in O(1) current/previous generation maps to
match the game's one-frame ordering-table latency. A one-pixel bounded lookup
recovers fixed-point/draw-offset rounding differences. Four-depth quads use the
original depth, three-depth quads reconstruct the fourth using a
reciprocal-depth screen-space plane, and otherwise valid quads derive
projective denominators directly from their screen homography. Stock/Original
keeps the authentic affine path.

The final Route 66 trace corrected 10,046 of 11,362 textured quads (88.4%) and
44,664 of 161,465 textured triangles (27.7%) with correlated GTE depth across
six 60-present intervals. The remaining triangle fallback cannot be uniquely
made projective from only three screen/UV correspondences and no depth; keeping
it affine avoids inventing unstable camera-dependent depth.
