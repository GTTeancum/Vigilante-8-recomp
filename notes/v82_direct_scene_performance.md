# Four-player direct scene rendering — 2026-09-10

Accepted bounded runtime increment: approximately 15 to 25 FPS in the Bayou four-player harness, without changing graphics settings or authored geometry. Four synthetic players and two bots; Enhanced 1280x720. Enabled by default only during the four-player dev harness's render pass. Normal one/two-player rendering and menu availability remain unchanged.

## Final same-executable comparison

Staged `V8_2_LOOSE/Vigilante82PC.exe` SHA256: `38C2ABF9628139EAC17542206C8165D1C00E7628D9CF7229235A3FD5AB069405`.

Sequential off/on/on/off runs, 900 input polls each, captures disabled, no concurrent games or build activity. Off uses `RECOMPONE_DIRECT_TERRAIN=0`; on uses the new four-player default. All runs exited zero and verified exactly six actors.

| Run | Median FPS | Minimum sampled FPS | Matched mean frame time |
| --- | ---: | ---: | ---: |
| Off 1 | 14.66 | 13.06 | 68.712 ms |
| On 1 | 24.93 | 20.84 | 40.882 ms |
| On 2 | 25.89 | 21.23 | 39.532 ms |
| Off 2 | 15.09 | 12.94 | 67.488 ms |

FPS statistics use one-second host samples after excluding loading and five initial gameplay samples; minima are not individual-frame lows or 1% lows. Matched timing uses eleven complete 60-frame windows covering frames 121–780 in each run. Averaging those windows across repeats gives 68.100 to 40.207 ms/frame: **27.893 ms saved (41% less frame time), approximately 69% greater throughput**. The median FPS gain across paired runs is 10.27–10.80. This reaches the requested 10+ FPS improvement, but does not claim the aspirational 40–60 ms saving. These are PC results, not measured Xbox performance.

Evidence: `artifacts/split-terrain-direct-20260910/final-{off,on}-{1,2}/`, `final-comparison.json`, and `summarize.py`.

## What changed

The enhanced renderer was rebuilding authored terrain after the translated PS1 renderer had already produced coarse polygons it would discard. The new path replaces that redundant polygon production/storage/decoding with one ordering-table marker per patch, retaining native GTE depth-bucket calculation and the existing authored terrain rendering path.

Terrain heights, colors and materials are reused across cameras and frames. Each camera still gets its own exact transform. Byte snapshots detect changed terrain pages and texture descriptors on the next render frame; material-average generation, color ramp, mode and CLUT are also dependencies. Direct writes and bulk replacements invalidate the cache. Snapshot and patch counts are bounded, and cached entries do not retain old per-frame culled lists.

Conservative viewport bounds reject terrain before color/material preparation and tessellation, and reject offscreen exact object triangles before packet creation. Bounds include all authored patch positions, height morph extrema, signed-coordinate wraps and a four-native-pixel guard. There is no reduction in visible detail, resolution, antialiasing, textures or combatants. Backend readiness checks retain the original path when direct rendering is unavailable.

`RECOMPONE_DIRECT_TERRAIN=0` restores the control path. Diagnostic switches include `RECOMPONE_VERIFY_SHARED_TERRAIN=1`, `RECOMPONE_EARLY_TERRAIN_CULL=0` and `RECOMPONE_TERRAIN_PATCH_CULL=0`. Explicit direct-mode opt-in is available for developer investigation outside the default scope; broader acceptance is not implied.

## Verification and limits

- Release publish passed; final build and staged executable hashes match; temporary stage timers removed; changed-file whitespace check passed.
- Source-sharing oracle logged 333,962 comparisons with the uncached original path without a mismatch; native run completed successfully.
- Terrain contract checked 10,000 authored patches: 5,350 rejected patches, with 102,492 resulting morphed triangles independently clipped and no visible triangle incorrectly rejected; source snapshot invalidation checks passed.
- Existing mesh clipping contract passed 560 packet/provenance assertions.
- Final two-player smoke exited zero with two humans/four bots and default direct mode inactive.
- Paired native captures at 20, 120, 160 and the later scripted capture were inspected individually (`visual-off` versus `scene-visual`), including equipped HUDs; no new visual defect was identified. Some animation/effect pixels differ between runs, so pixel identity is not claimed.
- Exact final executable also completed a native visual run; inspected captures 120 and the later scripted capture in `final-visual`. PNGs are lossless conversions of native PPM captures. P4 dark water is the previously accepted scene appearance.

Coverage is bounded Bayou gameplay, numerical visibility cases and a short two-player smoke. This is not all-level, all-frame or long-session regression certification. Existing unrelated working-tree changes were preserved. No commit or push performed.

## Next opportunities, ranked

1. Reduce remaining per-camera scene preparation by sharing camera-independent object/model work; the latest instrumented whole-world preparation stage was about 14 ms/frame, including remaining terrain work, so that is an upper bound rather than all-object savings.
2. Remove or restructure rendered-pixel GPU-to-CPU feedback while preserving its CPU-visible effect state; the corresponding stage took about 10 ms elapsed, which includes waiting and is not a guaranteed independently recoverable cost.
3. Reduce remaining material/depth batch transitions while preserving transparency and draw order; this is a secondary opportunity until its residual cost is isolated.

The first two are the remaining candidates for larger architectural wins. HUD simplification and load-time work are not priorities. The retained direct-rendering and visibility principles can carry to native Xbox code, but transfer APIs, hardware costs and results need separate measurement there.

The three opportunities above were subsequently implemented and validated in [the scene efficiency increment](v82_scene_efficiency.md), reaching 42.80–44.74 median FPS in the same four-player/six-combatant Bayou workload. See that report for the newer staged executable and matched controls.
