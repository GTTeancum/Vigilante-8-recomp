# PS1 shoreline rendering coordinates

The deployed sky-strip fix (EXE SHA-256
`D2B5BC267012975DF4C09AABC697FC980E07ACE077A4B08BF92A07EE7682827A`)
still showed rectangular water patches and angular water/land cutoffs.
The two composed 1920x1080 baseline views are retained in
`artifacts/louisiana-shoreline-native-camera-check`, at native gameplay
frames 270 and 540. Both were inspected individually at original size.

PS1 assets remain the runtime inputs. Dreamcast code is used only as a
reference for the shared rendering behavior.

## Confirmed coordinate and ordering faults

1. Both water replacement paths produced display-local XY but passed it to
   a shader that subtracts the target's framebuffer origin. The terrain
   reconstruction already restores this origin; the water paths did not.
   In the vertically double-buffered PS1 display, target Y=240 therefore
   moved water up a full screen relative to the terrain. The bounded probe
   at ticks 538–540 saw water depth about 0.9973 on that target, versus
   about 0.99956 at the same local points on target Y=0. It was sampling a
   different part of the water mesh. Both base and surface conversions now
   restore target X/Y while retaining the existing widescreen margin rule.

2. The untextured PS1 sky backdrop has UI material classification. `Begin`
   interpreted it as a HUD boundary and replayed buffered water before the
   land was complete. The actual panorama packet provenance now excludes
   this background from the HUD boundary, keeping water composition after
   the scene and before actual HUD primitives.

3. The distant base-water strip combined Dreamcast's fixed 512-projection
   screen slopes with PS1's 256-projection coordinates and unchanged world
   depths. Unprojecting those vertices placed them at twice the authored
   plane distance. Multiplying the slopes by `projection/512` restores the
   common plane; reciprocal-depth planes retain their recovered values.

The framebuffer/triangle evidence is in
`artifacts/louisiana-shoreline-owner-trace/runtime.log` under `FinalOwner`,
`FinalOwnerPixel`, and `FinalOwnerBatchTriangle`. Diagnostic probe points
were native (340,155), (350,155), and (350,172). These probes are disabled
in normal runs.

## Regression and visual verification

The new regression unprojects all four base vertices and checks the
authored plane equation, using both orientation branches and three
projection scales. It fails on the old implementation and passes with the
fix. The complete MeshClipContract suite now passes 206 assertions.

The candidate visual run uses the same native water-entry fixture and
original PS1 assets as the baseline. Candidate images and results are in
`artifacts/louisiana-shoreline-coordinate-candidate`; visual acceptance
must be recorded after inspecting its composed images.

The initial candidate loaded its own saved settings and is rejected as an
A/B comparison. The harness now seeds candidate settings from the deployed
PS1 root and restores the candidate's prior settings afterward. Matched
settings captures are in `louisiana-shoreline-matched-settings-candidate`.

## Final packaged verification and deployment

The final executable SHA-256 is
`97611011CABEAAF2510B1FFD21C69E658AF75AE3C95CA3D98CF36CB45B39522D`.
Its bundled runtime DLL SHA-256 is
`F7548B2D8F479970F9240493A2FA92B70CF025BAAAEDC56BEB8466ECE875FFF8`.
The bundle and previous executable are retained in
`artifacts/ps1-shoreline-release`; the identical verified bundle was copied
to `V8_2_LOOSE/Vigilante82PC.exe` after the test process exited.

Both final composed 1920x1080 FXAA views (native gameplay frames 270 and
540) in `artifacts/louisiana-shoreline-release-proof` were inspected
individually at original resolution. The misplaced rectangular patches
from the baseline are absent, and the water follows the terrain boundary.
These are native fixture approaches, not identical frozen camera poses:
AI collisions and vehicle motion vary between runs.

The final pixel probe at native (350,155) records depth 0.999286950 on
target Y=0 and 0.999286413 on target Y=240 in adjacent gameplay ticks.
The prior full-screen displacement and approximately 0.9973 sample on
the second buffer are gone. The trace also retains completed land before
water/HUD composition. Both coordinate regressions and the earlier sky
packet provenance assertions remain in the 206 passing unit checks.

The final hidden/silent, single-game-process fixture exited with code 0,
restored settings, and reported 59.51 median / 30.83 minimum window FPS
with no sustained tail decline. This is a Louisiana verification only.
