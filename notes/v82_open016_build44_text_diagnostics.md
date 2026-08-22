# V82-OPEN-016 Build 44 text-diagnostic record

Timestamp: 2026-08-20T08:16:29.9766082-04:00

## Bad-request diagnosis

The failed Codex tasks were inspected before continuing game work. Both failures
occurred at the task/request layer, not in the repository or game runtime:

- `Continue V82 chat 3 review` ended with an immediate HTTP 400 response after
  its local session JSONL reached 2,334,987,663 bytes (2,226.82 MiB).
- `Review recent V82 chat work` ended with the same response after its local
  session JSONL reached 422,635,324 bytes (403.06 MiB).
- The predecessor session contained 705 `view_image` operations, 1,268 embedded
  `data:image` payloads, 1,706 `image_url` records, 808 screenshot references,
  and 69 context compactions.
- The takeover session added 64 `view_image` operations, 157 embedded image
  payloads, 221 `image_url` records, 55 screenshot references, and seven more
  compactions.

Conclusion: the repeated 400 responses were caused by oversized, image-heavy
request context. Continuing from a fresh task and retaining textual evidence
instead of image payloads removes the triggering condition.

## Diagnostic implementation

Build 44 adds the following opt-in diagnostics and artifact controls:

- `run_reference_soak.py --text-only` retains JSON summaries, stdout/stderr,
  input fixtures, framebuffer-change hashes, heartbeats, callbacks, collision
  checks, weapon coverage, special commands, and power-state transitions while
  disabling PPM/stage/presentation capture requests.
- `RECOMPONE_DISPLAY_PROBE_IMAGES=0` prevents both latest-frame PPM output and
  scripted stage-image retention inside `HostWindow`; stage observations remain
  in the textual trace.
- `run_reference_soak.py --verbose-log` enables detailed selector, selector
  physics, vehicle material, ownership, input, presentation, and enhanced-render
  traces.
- `RECOMPONE_TRACE_V82_REFLECTION_SUMMARY=1` emits one aggregate line per
  vehicle/frame/object with `opaque-glass` and `gloss` primitive counts. This
  replaces the capped packet-by-packet trace for long angle sweeps.
- The packet-by-packet trace remains independently available through
  `RECOMPONE_TRACE_V82_REFLECTION_PACKETS=1` for narrow investigations.
- `tools/cleanup_image_artifacts.ps1` removes generated raster/video artifacts
  without following workspace junctions and preserves the active converter
  reference root.
- `tools/clean_v82_run_logs.ps1` removes stale top-level V8:2 run logs before a
  test handoff.

## Build identity and static checks

- Release build: 0 errors; 3,504 existing generated/nullability warnings.
- Publish: completed with trim-analysis warnings only.
- Converter regression suite: 10/10 tests passed.
- Python soak runner compilation: passed.
- `git diff --check`: passed (line-ending notices only).
- Staged executable: `V8_2_LOOSE/Vigilante82PC.exe`
- Size: 17,056,103 bytes.
- SHA-256: `86B91A9EB234A6DCFEABE97BEEDA5BFE2F4884D9CA6A611272B107F6A5EBA5FE`
- `PS1 game/Vigilante82PC.exe`: absent.
- Restored original V8 asset: `PS1 game/SHELL/VEHICLES.EXP`, 320,346 bytes,
  SHA-256 `E3AA49EE8AA198D9040E201A4FA0742398D8223DAD90B8C9FC2F78753D900307`.

## Native selector full-turn material proof

Artifact root:
`artifacts/v82-build44-textdiag-molo-selector-20260820`

The fixture intentionally remains in the native selector, so the soak harness
reports `gameplay entry timeout` after 105 seconds. That timeout is expected and
is not a runtime failure.

- Molo aggregate records: 2,235 consecutive selector frames.
- Requested audit window: frames 80 through 320 inclusive.
- Window coverage: 241/241 distinct frames; zero missing.
- Opaque pane primitives in the window: minimum 9, maximum 113, zero frames
  without opaque pane geometry.
- Gloss primitives in the window: minimum 0, maximum 11. Gloss is edge-on at
  frames 125-129, but opaque pane geometry remains 9-14 primitives there.
- Entire 2,235-frame Molo trace: minimum 8 opaque pane primitives and zero
  frames without opaque pane geometry.
- Fatal/runtime markers: 0.
- Retained image/video files: 0.
- Stderr trace size: 63,131,769 bytes.

This is the structural check for the user-reported angle-dependent see-through
failure: the depth-writing opaque pane layer is submitted at every recorded
angle, including the angles where the separate gloss overlay becomes edge-on.

## Super Dreamland 900-frame smoke

Artifact root:
`artifacts/v82-build44-textdiag-molo-dreamland-20260820`

- Result: passed 900/900 gameplay frames.
- Loaded overlay: `LEVELS_N64_DREAMLND`.
- Framebuffer changes: 45.
- Heartbeats: 6.
- Runtime callbacks: 17.
- Collision-stream rejections: 0.
- Weapons armed/fired: 1, 2, 3.
- Special commands: eight across weapon groups 1-3.
- Powerups: radar jammer, repair wrench, shield, weapon upgrade.
- Power-state changes: 6.
- Fatal/runtime markers: 0.
- Retained image/video files: 0.
- Molo reflection records while present: 312, gameplay frames 1-402,
  minimum 8 opaque pane primitives, zero records without opaque pane geometry.
- Stderr trace size: 57,591,489 bytes.

## Super Dreamland 2,700-frame weapon sweep

Artifact root:
`artifacts/v82-build44-textdiag-molo-dreamland-weapons-20260820`

- Result: passed 2,700/2,700 gameplay frames.
- Loaded overlay: `LEVELS_N64_DREAMLND`.
- Framebuffer changes: 51.
- Heartbeats: 16.
- Runtime callbacks: 29.
- Collision-stream rejections: 0.
- Weapons armed/fired: all seven kinds, 1 through 7.
- Special commands: 18 across weapon groups 1-6.
- Powerups/states: radar jammer, repair wrench, shield, transforms 1-3,
  weapon upgrade.
- Power-state changes: 14.
- Fatal/runtime markers: 0.
- Retained image/video files: 0.
- Molo reflection records while present: 312, gameplay frames 1-402,
  minimum 8 opaque pane primitives, zero records without opaque pane geometry.
- Stderr trace size: 58,767,990 bytes.

## Artifact cleanup

- Initial generated-image inventory: 19,096 files, 83,136,269,765 bytes
  (77.427 GiB).
- Removed: 19,084 generated image/video files, approximately 77.423 GiB.
- Retained: 12 active converter/reference images totaling 3,950,100 bytes under
  `artifacts/v8_to_v82_guest_roster`.
- Apparent images reached through the old Build 39 `mods` junction are live
  `V8_2_LOOSE/mods` runtime assets, not duplicate artifact files; they are
  required and were not deleted.
- Top-level V8:2 stale run logs after final staging: 0.

## Review state

Mechanical and structural verification passes. V82-OPEN-016 remains
`PASS_PENDING_USER_REVIEW` because explicit user review is the closure gate.

## Exact source-goal transplant and front-end completion

Timestamp: 2026-08-20T08:30:24.3293146-04:00

The source task's active goal was recovered verbatim from its persisted
`create_goal` record and transplanted into the current task. The previous
handoff had incorrectly narrowed it to glass/text-diagnostic validation. The
actual V82-OPEN-016 goal also requires loading transition, preview/aspect,
distance-lighting, weapon-effect, and high-resolution UI coverage.

The source task stopped after opening frame 2582 of the Build 41 front-end
proof. It had inspected the requested images but never recorded the verdict,
advanced the goal, or cleaned the proof because the next request returned HTTP
400. Build 44 therefore repeated only that interrupted acceptance surface.

Artifact root before post-inspection image cleanup:
`artifacts/v82-open016-build44-selector-loading-minimal-20260820`

- Executable: exact staged Build 44, SHA-256
  `86B91A9EB234A6DCFEABE97BEEDA5BFE2F4884D9CA6A611272B107F6A5EBA5FE`.
- Incidental framebuffer/stage images were disabled with
  `RECOMPONE_DISPLAY_PROBE_IMAGES=0`; stage observations remained textual.
- Verbose selector, material, input, presentation, and reflection diagnostics
  remained enabled.
- Run result: passed 180/180 Dreamland gameplay frames, loaded
  `LEVELS_N64_DREAMLND`, 10 callbacks, weapon 1 armed/fired, two special
  commands, zero collision-stream rejections, and zero fatal/runtime markers.
- Required visual frames: exactly 19 at 3840x2160.
- Selector frames 1900, 1960, 1990, and 2010 were inspected sequentially.
  Frame 1900 shows Canyonlands before navigation. Frames 1960-2010 retain the
  Super Dreamland 64 title, Dreamland preview, location marker, and USA map in
  stable layers with no stale-map preview or title contamination.
- Transition frames 2568-2582 were inspected sequentially. Frames 2568-2571
  are a monotonic fade from the enemy selector; frame 2572 is clean black;
  frames 2573-2582 show only the Super Dreamland 64 loading card, native tip,
  and animated tread. No frame shows another map, a mixed card, or an
  intermediate aspect/layout jump.
- The runtime selected the 1280x448
  `n64_dreamlnd_loading_card_4x.ppm` asset and the 440x115
  `n64_dreamlnd_selector_preview.ppm` asset with 2x replacements enabled.
- Direct 4K loading-card pixel verification: correlation
  `0.9996493324936494`, mean absolute error `0.6262165122615173`.
- Recorded output aspect: `1.333333` while gameplay=0; after the Dreamland
  overlay loaded and gameplay became active, `1.777778` while gameplay=1.

After the literal frame-by-frame review, all generated PPM/PNG proof files are
deleted again. The JSON summary, input fixture, stdout, stderr, hashes, metrics,
and this audit remain as the durable evidence.

## Representative native-vehicle matrix

Timestamp: 2026-08-20T08:45:42.3649841-04:00

Artifact root:
`artifacts/v82-build44-textdiag-native-vehicle-matrix-20260820`

The full-objective audit found that the exact staged Build 44 had exhaustive
imported-Molo material coverage but no current-build multi-vehicle gameplay
matrix. A new text-only run therefore exercised widely separated retail
character slots 0, 8, and 17 on Super Dreamland 64 for 900 gameplay frames
each.

- Result: 3/3 runs passed, 2,700 aggregate gameplay frames.
- Every run loaded `LEVELS_N64_DREAMLND` and reached frame 900.
- Runtime callbacks: 20, 19, and 19 respectively.
- Every run armed/fired weapons 1, 2, and 3.
- Every run exercised eight special commands across weapon groups 1-3.
- Every run exercised radar jammer, repair wrench, shield, and weapon upgrade.
- Collision-stream rejections: 0 across the matrix.
- Fatal/runtime markers: 0 across all six stdout/stderr logs.
- Retained image/video files: 0.

The acceptance verifier now treats this matrix as an independent required
check. Its superseding report passes 12/12 checks while retaining
`goal_complete=false` until explicit user review.
