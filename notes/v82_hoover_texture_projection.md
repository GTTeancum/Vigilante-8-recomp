# Hoover Dam wall texture projection diagnosis

The user identified long diagonal dark marks on the dam walls in the matched
Hoover readback comparison and asked whether they were affine warping.
Diagnosis is limited to those sampled wall surfaces, not every renderer path.

## Result

The sampled wall is using perspective-correct interpolation. The elongated
marks are consistent with the authored textured slope seen obliquely; this
trace does not establish an affine interpolation defect to fix. No renderer,
asset, graphics-setting, or executable change was made for this diagnosis.

Evidence is retained in `artifacts/hoover-warp-owner`: native composed capture,
bounded frame/tick packet diagnostics, losslessly decoded installed terrain
atlas, and `projection-analysis.json`.

- Installed interface.ini explicitly sets PerspectiveCorrectTextures=True.
- PrimVs uses camera-space ViewZ as gl_Position.w for modern geometry.
- PrimFs selects the ordinary perspective interpolant vUVPerspective.
- Sampled terrain triangles have modern=2 and varying camera depths, rather
  than the constant clip W that would yield affine texture interpolation.
- At tick303, packet00282370, cell944,1348, leaf1,0, material9, texture211,
  UVs are (16,143), (16,96), (63,143). The texture descriptor and UV mapping
  come from the active PS1 terrain data. The active replacement is the
  installed levels/v8/hoovrdam atlas, not a Dreamcast asset.
- The complete sampled tile's camera-space corners are:
  (512,-53.44507,2168.4446), (768,-259.92163,2143.3079),
  (512,-22.507568,1914.3196), (768,-228.984128,1889.1829).
- Its two physical edges measure approximately256 and329.85 world units.
  Applying the actual256 projection scale to these points gives a local
  texture-circle major/minor axis ratio of5.75097 at the tile center.
  This is a local differential measurement, not a claim that an entire
  large circle has a constant scale over the perspective transform.
- The installed source atlas contains the openings/dark marks and horizontal
  bands. Their projected elongation does not alone imply bad interpolation.

The existing native owner probe samples native(63,110)/(279,110), corresponding
to framebuffer(351,330)/(999,330) for margin54 and scale3. TriangleProbe used
its separate draw-coordinate convention at(116,110); do not equate those
coordinate conventions when reading the logs. The held-scene run is a visual
diagnostic, not a traversal/performance acceptance run; its expected red
traversal gate does not invalidate the geometry/texture evidence.

The assistant's earlier language called the visible feature distortion before
establishing its cause. This diagnosis corrects that characterization for the
sampled dam wall. It does not claim the entire renderer is defect-free.

## Tower correction and approach diagnostic

The red arrow in the subsequent user screenshot identifies the left tower,
not the sloped dam wall measured above. The wall analysis does not establish
that the tower is correct. Native owner probes identify object 0x806D34B0,
packets 0x0029FEC8 and 0x0029FEE8 at tick 301. Their camera vertices match
PS1-imported HOOVRDAM.EXP XOBF bank 0, group 13, rib_1, packets 2/3.
The planar face has uneven lower corners but rectangular UV corners.
Source coordinates are retained in artifacts/hoover-tower-projection-owner/
tower-source-face.json. A continuous quad-mapping experiment is unbuilt and
not deployed. At the user's request, the current deployed executable is
being captured while driving closer before accepting a rendering change.

### Closer-camera result

The accepted same-tower comparison is artifacts/hoover-tower-forward-proof
polls 601 and 801. The tower becomes larger/closer and the upper-band kink
persists on its face. All five captured frames were inspected individually;
921, 1041, and 1161 are excluded because the fixture enters the dam and/or
the tower is obscured/clipped. Earlier routes approached a different rib or
overshot the scenery and have explicit diagnostic-rejection.json records.
The two usable stills do not independently prove temporal affine swimming.
The renderer uses exact perspective interpolation, while source bank 0 group
13 packets 2/3 map rectangular UVs across an irregular planar face.
A continuous quad-mapping experiment was removed from runtime source before
building this diagnostic; it is saved only as text in the owner-probe folder.
Only the opt-in held-player translation fixture was added for the captures.
The diagnostic build passed the 460 existing mesh/readback assertions.
Original deployed executable 5D11D075...D6EF was restored afterward.
The tower rendering defect remains unresolved; no fix is claimed.

## Continuous planar mapping fix

The Enhanced backend pairs complementary rectangular-UV triangle halves
within a draw batch using their shared camera-space diagonal, UVs, texture
page, palette and colour. It rejects mismatched vertices/materials and
nonplanar, concave or degenerate faces. A planar face receives homogeneous
UV weights derived from its diagonal intersection. The shader interpolates
(U*Q,V*Q,Q) and divides by Q; camera projection/clipping still use exact ViewZ.
Geometry, UV corner values, source textures, colours and depth are preserved.
Parallelograms retain the existing interpolation exactly. No arena-name
recognition or map-specific render branch is used. Only Hoover is tested.

The new tests use the real rib_1 vertices, prove both diagonals map to the
same UV centre, check invariance under camera rotation/translation, exercise
the backend's actual pairing/writeback, and reject nonplanar/concave/degenerate
faces. The total mesh/readback contract count is now 476 passing assertions.

Initial native proof at artifacts/hoover-tower-mapping-proof was inspected
individually at polls 601 and 801. The upper-band kink is removed in both.
At each viewpoint, 60,200 sampled sky pixels and 73,500 foreground road
pixels exactly match the old rendering. The tower mapping is intentionally
different. Animated vehicles/effects/HUD prevent full-frame identity.
Final build 2DEE9DE9C171D63FA7D3609F012A0EDAE8F70D69AE1CA3704EC095715CF576DE
is deployed for final visual and performance verification.

### Final verification and deployment

Both final native 1280x720 captures (polls 601/801) in
artifacts/hoover-tower-fixed-proof were individually inspected. The band kink
is removed at both distances. At each distance 133,700 sampled sky/road pixels
remain identical to the old rendering. Deployed build, visual proof, and final
performance run share SHA-256 2DEE9DE9C171D63FA7D3609F012A0EDAE8F70D69AE1CA3704EC095715CF576DE.
The 476 assertions and git diff --check pass.

The first performance run had a 24.40 FPS window. A profiled control with
planar mapping disabled also had a 28.36 FPS window with one 548 ms frame.
The final profiled enabled run passed both Hoover visits. All results,
including failures, are retained in artifacts/hoover-tower-fix-result.json.
These varying native combat trajectories are not identical-workload timing
comparisons and do not establish a universal 40 FPS floor. No extra runs
were started after the user's performance-run question/disregard messages.
The requested tower correction is fixed, visually verified, and deployed.
