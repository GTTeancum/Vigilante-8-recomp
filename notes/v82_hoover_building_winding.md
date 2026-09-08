# Hoover building face orientation

The default-on `DreamcastWorldObjectCull` rejected positive projected area for
ordinary PS1 scenery. This is present at commit e0b5660 and predates the planar
quad UV correction. The first introduction has not been established.

A native hidden/silent Hoover capture with that rule disabled logged all ten
triangles of the imported PS1 `HOOVRDAM.EXP` bank 0 / group 13 / rib_1 tower at
gameplay tick 301. Source packets 0/1 form the X=40 exterior wall, at view
Z=958.9451..981.5439. These have positive projected area. Source packets 2/3
form the X=-39 far wall, at Z=1037.3665..1059.9653, and have negative area.
The old rule discarded the nearer wall and retained the back of the far wall.
This is actual reversed face rejection, independent of UV interpolation.

`Ps1CullBackFace` now rejects negative area after the existing near-plane
clipping. The same per-triangle test avoids the stale/shared GTE association
problem documented in earlier renderer research. Runtime configuration uses
`RECOMPONE_V82_PS1_WORLD_OBJECT_CULL` (default on). The obsolete Dreamcast
world-object switch and helper are removed. Existing water, terrain, vehicle,
and material rules are unchanged. No asset or fidelity reduction is involved.

The contract fixture passes 482 assertions, including both halves of the real
tower near and far walls at three camera distances. Build/staged SHA256:
B14CFCCC39684ECA96D5C6C0A9BEF3FE84F7A34D2EF50E6281328B78E178866E.

Evidence:
- `artifacts/hoover-building-winding-before`: original face rule, held capture.
- `artifacts/hoover-building-winding-no-cull`: disabled-rule diagnostic and
  `tower-winding.json` with the ten exact emitted triangles and both decisions.
- `artifacts/hoover-building-winding-fixed-proof`: final staged build, native
  captures at polls 601 and 801 with the existing process-local approach fixture.

Held visual fixtures deliberately do not traverse the arena, so their general
map fidelity gate reports incomplete routes. The two-capture run also exceeds
that gate's one-capture expectation. These are visual diagnostics, not claims
of map acceptance or a new performance benchmark.

Final review: both native PNGs were inspected individually at full resolution.
Near exterior walls remain visible in both views; the tower bands are continuous.
The closer frame has a combat projectile partly covering the central building,
while the left tower remains unobstructed. The final gate reports ONLY
`exactly_one_gameplay_image` failure (two intentional captures); all per-map
contracts passed. No new performance result is claimed. See the final proof
directory visual-verification.json.
