# Hoover Dam efficiency run

Scope: Hoover Dam only, including consecutive visits in one game process.
Target: at least 40 FPS median and minimum measured 60-frame window, with
no reduction in fidelity. PS1 assets remain the runtime inputs.

## Baseline and measured cost

The user's earlier two-map run recorded Hoover at 25.14 median / 16.64
minimum FPS on executable `97611011...B39522D`. The current Hoover-only
reproduction did not reproduce that severity:

| Run | Median FPS | Minimum window FPS |
| --- | ---: | ---: |
| Single Hoover, detailed profiling attached | 58.90 | 36.66 |
| Consecutive Hoover baseline, first visit | 59.53 | 44.94 |
| Consecutive Hoover baseline, second visit | 60.02 | 46.89 |

Do not attribute the difference from the historical 25.14 measurement to
the optimization: these three measurements used the unchanged executable.
The detailed profile includes profiling overhead and is diagnostic only.
Timing and routes are retained in `artifacts/hoover-efficiency-baseline-profile`
and `artifacts/hoover-efficiency-repeat-baseline`.

The 15-second managed thread sample located approximately 4.17 seconds
inside `GlVram.ReadRect`, including GPU waits. Small native VRAM reads used
to download and convert all 524,288 pixels whenever the cache was invalid.
This is a synchronization/transfer cost, not evidence that all of that
time is CPU computation.

## Implementation and fidelity boundaries

Small reads now populate a 64-aligned native pixel window. A usual 2-pixel
request downloads 4,096 pixels instead of 524,288. Requests at VRAM wrap
seams retain the full-window path; large uncached requests retain direct
readback. Cache reuse requires containment, and every existing mutation
invalidation remains in place.

The original full framebuffer-to-VRAM writeback boundary is unchanged.
This preserves GPU texture feedback as well as CPU-visible pixels. No
geometry, draw order, culling, texture, filtering, shading, resolution,
MSAA, simulation cadence, or presentation cadence was changed.

`RECOMPONE_FULL_VRAM_READBACK=1` selects the previous full-window behavior
for controlled comparisons. `RECOMPONE_VERIFY_VRAM_READBACK=1` downloads
the full VRAM as an oracle and compares every returned small-read word,
throwing immediately on any difference. These controls are off in normal
play. Performance logs now count readbacks, downloaded pixels, and cache hits.

The mesh/provenance suite passes 460 assertions: the existing 206 plus 254
read-window checks covering native wrap, aliased coordinates, row copies,
cache containment, and the reduced window size.

Candidate executable: `5D11D075D080C4CCD2833EE16E6341D711D261FA6655FC5C27B506F816B5D6EF`.
It was published to `artifacts/hoover-efficiency-release`, staged in
`V8_2_LOOSE`, and the previous executable retained alongside the bundle.
## Candidate performance

The identical packaged/deployed executable completed two Hoover visits at
1280x720, normal pacing, capture disabled, original PS1 assets, and the same
360-poll route configuration as the baseline. Every functional/HD/route and
standard performance gate passed; all windows additionally exceed this
task's stricter 40 FPS minimum.

| Visit | Median FPS | Minimum window FPS | Head median | Tail median |
| --- | ---: | ---: | ---: | ---: |
| First | 57.87 | 40.81 | 57.65 | 47.12 |
| Second | 59.94 | 47.77 | 59.98 | 60.14 |

Evidence: `artifacts/hoover-efficiency-repeat-candidate/acceptance.json`.
The first run's scenery/vehicle workload differs from baseline (140.79
versus 118.01 flushes per frame). Native AI collisions and vehicle movement
can vary; this is not an identical frozen workload. These results establish
the requested FPS floor in the tested runs, not a measured 25-to-58 FPS
improvement. The proven optimization is reduced readback work, with the
original full-window behavior available for diagnosis.

Typical gameplay counters now report about 31 readbacks and 199,680 pixels
downloaded per 60-frame window, versus about 16 million pixels if those
reads all filled the former full-window cache. Native oracle verification
and the composed capture are recorded separately to avoid mixing their
readback/capture overhead into these performance measurements.

## Final validation

The separate native oracle run passed every functional/HD/route check and
exited cleanly. At least 840 small reads matched a fresh full-VRAM readback
word-for-word, with zero mismatches. The native composed 1280x720 FXAA
capture at gameplay poll 300 was inspected individually at original size;
terrain, scenery, vehicle surfaces, and HUD are present. The car's pose is
from native fixture movement/collisions, not a camera-matched baseline.

Proof: `artifacts/hoover-efficiency-pixel-proof/acceptance.json` and
`recompone_present_gameplay_0300_1280x720_fxaa.png` in the same directory.
The aggregate `artifacts/hoover-efficiency-result.json` verifies both the
40 FPS median/minimum requirement and the deployed executable hash against
both runs. No game process or stale runnable-root run log remains. The
original executable is retained as `Vigilante82PC.previous.exe` in the
release artifact folder. No other maps were run.

## Matched visual comparison requested after deployment

Captured the old full-readback behavior and optimized regional behavior from
this same deployed executable, using Hoover only, 1280x720, held native
vehicles, and capture poll 601. The two original composed images were each
inspected at full resolution. Evidence is in
`artifacts/hoover-efficiency-visual-ab/before` and `after`; `comparison.json`
records the controls and pixel comparison.

The compared static sky, left/right dam walls, left tower, and foreground
road regions contain 400,100 pixels total and are all pixel-identical.
The full images differ at 30,742 of 921,600 pixels, with differences in
animated vehicles/effects and the HUD icon. They are not wholly identical
snapshots. The distortion the user identified is visible in both images;
this optimization did not remove it and it is not a new scenery regression
in this matched view. No claim is made that the renderer is free of defects.

The held-scene diagnostic intentionally fails traversal acceptance; the
full-readback capture also fails the separate performance check. Both game
processes exited cleanly, and these capture runs do not replace the earlier
moving-gameplay performance proof. Settings SHA-256 remained
`14902F67A2CE3D285FE6C30A3402564ABBE80E6DB585B053047C74616AD2886F`.
