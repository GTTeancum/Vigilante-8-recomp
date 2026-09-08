# PS1 panorama ownership and the horizontal scenery smear

The runtime uses PS1 assets. Dreamcast binaries are reverse-engineering
references for rendering behavior; they do not supply this scene's assets.

## Cause and evidence

The Louisiana poll-300 presentation exposed an off-screen scenery triangle
stretched across the midground by panorama edge continuation. The source
was PS1 packet `0x00287C7C`, incorrectly labelled `v82-sky=80050B38`.
At draw frame 3918 / gameplay tick 149 its projected vertices were
`(679.14734,377.46094)`, `(645.0221,382.24478)`, and
`(584.11145,373.3255)` before continuation. Their camera depths were
1210.2595, 1123.9773, and 1364. The continuation moved the third vertex to
x=-54 and extrapolated its U from 73 to 193.85968. Its PS1 texture page was
0x88A, CLUT 0x7F42; the original UVs were (55,160), (93,163), (73,186).

The native-framebuffer probe at (213,135), framebuffer pixel (801,405),
first saw terrain cell (892,1256), leaf (1,2), material 197, true camera
depth 1877.7454. Its resolved RGB was (25,25,8), depth 0.999481380.
Subsequent alpha-tested scenery writes changed it to (25,16,8), then
(0,0,0), depth 0.999279916. The final writer was the stretched triangle
above. This excludes terrain atlas padding, terrain material selection,
and water as the cause of this particular band.

The probe uses the render target's shader position bias, display-buffer
origin, widescreen margin, and resolution scale. Poll 300 is a capture
request, not gameplay tick 300. The originally suggested native point
(333,155) instead maps to framebuffer (1161,465), outside this band.

Raw diagnostic evidence is retained in
`artifacts/dreamcast-render-full-re/louisiana-sky-source-before/stderr.log`
under `SkyExtensionSource`, `FinalOwnerBatchTriangle`, and `FinalOwnerPixel`.
The complete before image is in `louisiana-final-owner-capture-aligned`.

## Recovered PS1 contract

Generated retail code in `reference-v8-2/generated/recompiled/main.cs`
shows that `func_80050B38` renders scenery from the list at gp+0x1188 and
the array at gp+0x1198. Its dynamic packet-cursor interval is not sky.

`func_8001C910` is the panorama emitter. It constructs two static FT4
packets at `0x800B9270 + bufferIndex*0x50` and that address plus 0x28,
with a flat backdrop at `0x800B9310 + bufferIndex*0x30`. Buffer selection
comes from gp+0x20. It prepends these packets to the ordering-table bucket
at `ReadU32(gp+0xCE0) + 0x3FFC`; it does not allocate them by advancing
the dynamic scenery cursor at gp+0x610.

The compatibility hook records that bucket's old head before the emitter
and follows only its newly linked packets afterward, stopping at the old
head. The renderer applies panorama continuation only to textured packets
with this provenance. It combines adjacent panorama batches across
texture/state changes, then submits them before subsequent world
triangles, rectangles, or lines. Ordinary scenery retains its original
geometry and ordering. No map, texture, coordinate, or packet-address
whitelist is used.

The existing Dreamcast reference guard `tools/dreamcast_sky_contract.py`
also verifies a separate panorama renderer at 0x8C095C40: two
camera-relative four-vertex strips and six horizon/wrap vertices. The
verified reference executable SHA-256 is
`06F5A6FCDC9F8E0EF486A1CE1DF7FCF61F5A756DBEF7248139291AE520E3D3E7`.
Its contract output is retained as
`artifacts/dreamcast-render-full-re/sky-reference-contract.json`.

## Verification

The MeshClipContract regression failed before the fix because scenery
packets inherited sky ownership. Afterward all 182 assertions passed,
including both static panorama packets, exclusion of prior OT contents,
and exclusion of the dynamic scenery arena.

The first corrected 1280x720 composed capture at poll 300, retained as
`artifacts/dreamcast-render-full-re/louisiana-sky-provenance-fixed/candidate.png`,
was inspected at full size: the horizontal midground smear is absent,
and tree silhouettes, foreground foliage, and full-width sky remain.
This diagnostic candidate passed gameplay checks but failed the sustained
performance gate (33.17 median FPS, 20.13 minimum window FPS). That run
is visual evidence, not a passing release-performance claim.

The final packaged executable is retained in
`artifacts/dreamcast-render-full-re/ps1-sky-provenance-release/`.
Its SHA-256 is
`D2B5BC267012975DF4C09AABC697FC980E07ACE077A4B08BF92A07EE7682827A`;
the bundled runtime DLL SHA-256 is
`8679A1E309DCA1B5FAE1A20290E125203EE703CE84379FB5017E2656E778B615`.
`sha256.json` also records the sources, PS1 BAYOU.EXP, and previous EXE.

Packaged runs used Enhanced rendering, the original loose PS1 data root,
normal frame pacing, one hidden/silent game process, no live input, and
process-local harness input. Both captured one composed 1280x720 FXAA
frame at gameplay poll 300; both full images were visually inspected.

| Run directory under `artifacts/dreamcast-render-full-re` | Visual result | Median FPS | Minimum window FPS |
| --- | --- | ---: | ---: |
| `louisiana-sky-release-proof` | Midground strip removed; foreground trees and sky preserved | 40.91 | 9.77 |
| `florida-sky-release-proof` | Sky reaches both widescreen edges without a new join seam | 42.99 | 9.70 |

Both packaged runs pass the gameplay/clean-exit/single-process checks,
but fail the sustained-performance gate. Minimum FPS refers to a
60-frame measurement window, not an individual-frame reciprocal. The
Florida tail also fails the sustained-decline limit. Do not describe
these results as full release-gate passes.

Subsequent sequential, capture-disabled performance runs used the same
normal pacing and gameplay route. The existing deployed EXE passed at
59.95 median / 54.61 minimum FPS
(`louisiana-deployed-performance-baseline`). The unchanged packaged
candidate passed every gate at 59.99 median / 53.59 minimum FPS with no
tail decline (`louisiana-sky-release-performance-repeat`). Capture-mode
performance and capture-disabled performance are reported separately;
the earlier slow runs have not been discarded or relabelled as passes.
