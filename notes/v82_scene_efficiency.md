# Four-player scene efficiency — 2026-09-10

All three approved runtime opportunities are implemented. The Bayou dev harness now measures **42.80–44.74 median FPS**, up from **25.03–25.20** with these additions disabled. Four synthetic players, two bots, Enhanced 1280x720, unchanged graphics settings. Default activation remains confined to the four-player developer render path; no menu entry was added.

## Reproducible comparison

Staged executable: `V8_2_LOOSE/Vigilante82PC.exe`.
SHA256: `ABE515083AEFF2571E0D3B519DCB35E1F41B325CE555A2CD24DFBE38608C04D3`.
Build source: `artifacts/split-scene-efficiency-20260910/build-water-material/`.

Sequential on/off/off/on runs used the **same executable**, 900 input polls, captures disabled, and no concurrent game, build, or test workload. Both paths include the earlier terrain optimization and the water material correction described below. The control disables only these new switches:

```
RECOMPONE_SHARED_MODEL_BOUNDS=0
RECOMPONE_PIPELINED_FEEDBACK=0
RECOMPONE_STREAM_BATCHES=0
RECOMPONE_CACHE_DRAW_STATE=0
```

Leave `RECOMPONE_DIRECT_TERRAIN` unset for this comparison. Disabling it also disables the previously accepted terrain work and produces a different baseline.

| Run | Median FPS | Minimum sampled FPS | Matched mean frame time |
| --- | ---: | ---: | ---: |
| On A | 42.80 | 35.17 | 23.765 ms |
| Off A | 25.03 | 20.44 | 40.725 ms |
| Off B | 25.20 | 20.31 | 40.765 ms |
| On B | 44.74 | 33.46 | 23.764 ms |

FPS uses one-second host samples after loading and five initial gameplay samples are excluded. The minimum is a minimum sampled FPS, not a single-frame low or a 1% low. Matched timings use eleven complete 60-frame windows, frames 121–780. Their repeat averages are **40.745 → 23.7645 ms/frame**: **16.9805 ms saved**, **41.7% less frame time**, **71.5% higher throughput**. Every run exited zero and verified six combatants. These results measure the combined increment; they do not assign independent gains to each component.

Evidence: `artifacts/split-scene-efficiency-20260910/accepted-{on,off}-{a,b}/`, `accepted-comparison.json`, and `summarize.py`.

## Implementation

1. **Shared model preparation and visibility.** Cache object-space bounds from authored vertex bytes, reuse across cameras, and skip the native face traversal when the complete transformed model cannot enter the viewport. The native common matrix/light setup and epilogue remain. All near/behind exceptional projections retain the original traversal; backdrops and unscoped draws retain it as well. Byte comparisons detect deformation, heap reuse, and bulk replacement; memory ownership and storage caps prevent stale/unbounded caches. Camera transforms remain independent. No visible model detail, mesh resolution, or draw distance is reduced.
2. **Pipelined rendered-pixel feedback.** Snapshot the original sampled pixels into a bounded GPU staging buffer after each view, then download them after CPU preparation of the next view. Original world/effect/HUD GPU order, native pixel conversion, VRAM writeback region, and native effect-packet processing are preserved. It does not delay effects to a later frame. The bounded OpenGL staging transfer is 16 KiB; the portable improvement is overlapping CPU preparation with readback rather than the specific API.
3. **Batch upload and state efficiency.** Stream vertex batches through the existing bounded VBO instead of overwriting offset zero at every draw, cache unchanged primitive uniforms/depth state, and avoid texture barriers for render-target draws that do not read that attachment. Required feedback/check-mask/subtractive barriers remain. Draw order, blend equations, depth comparisons, and graphics options are retained.

The shared model seam is installed idempotently by `apply_vehicle_engine_patches.py`, immediately before the native mesh face loop. Running the applier against the final generated source reports zero changes.

## Visual regression discovered and repaired

Removing offscreen model projections exposed broad white horizontal bands in distant water. Native water polygons already carried authoritative WaterBase/WaterSurface provenance, but a generic high-ordering-table/no-projection heuristic could overwrite it with UI material. Whether that happened depended on incidental screen-coordinate matches against earlier model projections. The correction preserves the water material ahead of generic UI inference. It is based on native render-pass ownership, with no map, vehicle, coordinate, or address whitelist.

A native GP0 scanline replay test now verifies water provenance with no preceding model projection, for opaque and semitransparent packets, and verifies that unrelated UI still takes its original path. The late captured bands are removed. Both final performance paths include this correction so its benefit is not attributed to the three optimizations.

## Verification

- Release publish passed; staged executable and build executable hashes match.
- Model contract: byte invalidation, memory ownership, deformation and heap replacement; 10,000 random bounds, 8,609 rejected bounds, and 68,872 contained triangles independently clipped without a visible triangle incorrectly rejected.
- Native model oracle rendered proposed rejections through the original traversal; no `ModelBoundsMismatch` diagnostics in the final 400-poll run. This supplements, rather than replaces, the numerical and visual checks.
- Native pipelined-feedback oracle: **9,733 exact pixel comparisons** reported, no mismatch, 700-poll HUD-fixture run exited zero with six actors. Diagnostic timing is excluded from performance claims.
- Existing mesh contract: **560** packet/provenance assertions passed.
- Existing terrain contract: **10,000** patches, **102,492** morphed triangles independently clipped, and source invalidation checks passed.
- Four paired captures (20, 120, 160, and the later scripted capture) were inspected individually: `accepted-control-visual/` against `water-material-visual/`. Scene, player/HUD coverage, and water remain consistent; broad white scanline bands are absent. Animation/effect pixels can differ, so pixel identity is not claimed. P4 dark water remains the previously accepted appearance.
- Two-player smoke: 300 polls, two humans/four bots, exit zero; no model-bounds or pipelined-feedback activation. The run was too short for a steady FPS statistic and is used only for compatibility.
- PNGs are lossless conversions of the game's native PPM captures. No desktop capture or host input was used.

Coverage is bounded Bayou gameplay, synthetic/numerical contracts, and compatibility smoke testing; it is not all-level or long-session certification. Xbox performance remains unmeasured. Existing unrelated working-tree changes were preserved. No commit or push was performed for this increment.
