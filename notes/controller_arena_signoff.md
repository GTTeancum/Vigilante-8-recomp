# Controller and arena signoff — September 14, 2026

Status: **CLOSED BY USER — September 14, 2026.** All 19 arenas
completed normal-menu 1P and direct-harness 2P/4P checks on the same staged
build: 57 clean runs with the texture pack active, 95 captures opened and
reviewed individually. The identified initialization, split terrain, divider,
shadow and controller-reset faults are corrected. The user closed TO-DO #2 after this testing. Physical-device behavior was
not independently certified by the synthetic checks.

## Corrections after the initial sweep

- Direct entry skipped native `L800135C8` setup, including `func_8001B608`'s
  default terrain page/table initialization. The harness now executes the same
  native setup sequence as the menu route. This resolved the invalid 2P spawn
  views and Casino City load timeout.
- Reconstructed terrain used the framebuffer origin in place of the native
  GPU drawing offset. Those differ for split viewports, especially P3/P4.
  Terrain vertices and both culling paths now use GP0(E5h) coordinates. The
  corrected 4P Route 66 and Olympic captures were opened individually at frames
  120 and 350; all four views show coherent terrain and complete vehicles.
- The full-height native 2P divider was treated as a top HUD element and moved
  inward by widescreen HUD anchoring. Full-height strips now retain display
  placement. A normal-menu 2P Route 66 run confirms the duplicate divider is gone
  at both 120 and 350 (`menu-2p-route66-terrain-origin`).
- Lowercase direct-launch paths loaded the files but failed the native loader's
  case-sensitive arena callback lookup. In Dreamland this left the water plane
  at zero and killed P2 on the first update. The correct native export spelling
  initializes water to `0x002D5400`; 2P and 4P now pass the weapon fixture and
  startup checks. The harness obtains the matching name from the DLL's export
  table; retail Olympic demonstrates why the arena need not be its first export.

The subsequent visual sweep exposed a vehicle shadow defect on Valley Farm
and Dreamland roads. At native tick 60, the final-pixel trace identifies packet
`0x002077BC`, tpage `0x058` (subtractive blend 2), CLUT `0x7F60`. The palette-row
glass override classified it as `Glass`, enabled depth writing, and blocked the
deferred road inside the shadow rectangle. `PrimOf` now respects the native
blend mode even when a shadow shares a palette row with panes. The existing
subtractive replay path then shades the completed road without occluding it.
Opaque and ordinary glass behavior are preserved; no atlas/map/car exception
was introduced.

The real `PrimOf` regression test failed before correction and passes afterward
for both palettes and all four blend modes, including opaque controls. The
corrected Valley Farm and Dreamland captures were opened individually at frames
120 and 350 in both 2P and 4P; the rectangular cutouts are absent.

Current staged SHA256:
`C72C8FD10A82C0A62A3362429F3D8692A7ABFAD11B82685393E1DDDB17379542`.
The completed sweep is under `artifacts/input-arena-review-20260914/*-shadow-fixed`.
Its individual review manifest is `shadow-fixed-visual-review.json`;
`completed-summary.json` joins every run result to its visual review.
The earlier `*-final` sweep was stopped for this correction and is superseded.

Contract checks: 625 mesh/projection/material assertions; 35 controller assertions plus
18 real retail DLL export lookups. Native captures remain bounded checks, not
exhaustive traversal or physical-device tests. The texture pack stays active.
Release publish passed (`shadow-material-build.log`); final contracts passed
(`shadow-material-after.log`, `controls-shadow-final.log`). Published and staged
executable hashes match. No content-specific runtime exception or fidelity
reduction was added.

## Controller correction

Closing/reopening SDL controllers preserved the previous controller's analog
hysteresis latch and stick positions. A replacement trigger at 7000 could
inherit a held state without crossing its 8000 press threshold.
`InputManager.CloseControllers` now clears the latches and returns both ports'
four native axes to 128. The real private input functions were exercised
without SDL handles; this does not certify physical devices or rumble.

## Capture method and limits

All game processes were hidden, with process-local scripted input, live input
and rumble suppressed, and the game's native presentation capture enabled.
Enhanced rendering and the staged texture pack were active; every successful
scenario recorded replacement-texture hits. No desktop automation, performance
benchmark, or five-hour soak was used.

Two-player tests use two humans and four bots; four-player tests use four humans
and two bots. Each split run captures frames 120 and 350. P1/P2 throttle starts
after the first capture; P3/P4 built-in input starts at native tick 20.
Single-player tests follow normal menus and capture frame 601 after scripted
driving. Each 1P process runs approximately one minute.

The runner's `passed` means clean exit, expected split roster, texture hits,
capture files and no fatal log entries. Visual results are separately recorded
in `shadow-fixed-visual-review.json`; each image is opened individually.
These are bounded still-image checks, not continuous video inspection or
exhaustive arena traversal. Scripted routes sometimes reach walls, banks or
other cars; the review manifest records those limitations.

Historical failed sweeps and superseded build hashes are retained in
`artifacts/input-arena-review-20260914/signoff-before-final.md` and `progress.log`.
They are not outstanding defects on the current build.

## Final arena coverage

Each cell means runtime gates passed and the listed native captures were
individually reviewed for the identified defects. It does not certify every
route or animation. Exact per-scenario observations, including collisions,
close walls and bank-only views, are in the review manifest.

| Arena | 1P (601) | 2P (120, 350) | 4P (120, 350) |
|---|---|---|---|
| Route 66 | Pass | Pass | Pass |
| Olympic | Pass | Pass | Pass |
| Bayou | Pass | Pass | Pass |
| Launch | Pass | Pass | Pass |
| Steel Mill | Pass | Pass | Pass |
| Nuclear | Pass | Pass | Pass |
| Oilfield | Pass | Pass | Pass |
| Harbor | Pass | Pass | Pass |
| Secret Base | Pass | Pass | Pass |
| Sand Factory | Pass | Pass | Pass |
| V8 Oilfield | Pass | Pass | Pass |
| Air Graveyard | Pass | Pass | Pass |
| Wild West | Pass | Pass | Pass |
| Hoover Dam | Pass | Pass | Pass |
| Valley Farms | Pass | Pass | Pass |
| Casino City | Pass | Pass | Pass |
| Canyonlands | Pass | Pass | Pass |
| Ski Resort | Pass | Pass | Pass |
| Super Dreamland 64 | Pass | Pass | Pass |

No failed runtime gate or identified initialization/viewport/shadow defect
remains in this finite sweep. The 1P Canyonlands capture ends against a cliff,
so it is not fresh road/bridge traversal evidence; the 2P/4P road captures and
previously accepted corrective work remain separate evidence.

Representative current-build captures:

- [2P Canyonlands road](../artifacts/input-arena-review-20260914/2p-16-levels_v8_canynlnd-shadow-fixed/recompone_present_gameplay_0120_1280x720_fxaa.png)
- [4P Route 66 terrain](../artifacts/input-arena-review-20260914/4p-00-levels_route66-shadow-fixed/recompone_present_gameplay_0120_1280x720_fxaa.png)
- [2P Valley Farms shadows](../artifacts/input-arena-review-20260914/2p-14-levels_v8_vallyfrm-shadow-fixed/recompone_present_gameplay_0120_1280x720_fxaa.png)
- [4P Dreamland startup](../artifacts/input-arena-review-20260914/4p-18-levels_n64_dreamlnd-shadow-fixed/recompone_present_gameplay_0120_1280x720_fxaa.png)

## Closure and separate work

- The user closed arena/controller signoff; no further checks are required
  under this item. Recorded test limitations remain part of the evidence.
- Physical P3/P4 ownership and normal menu integration remain the separate
  four-player menu item. The user closed the separate resource-cleanup/soak
  item on September 14 without requiring the five-hour run; it was not performed.
