# Shared near-object viewport-edge clipping

User scope (2026-09-04): any nearby object at either view edge, including
other vehicles. The Ski Resort orange fence is a reproducible example, not
permission for a fence/map exception. The terrain/Sheila five-map goal is
paused at the user's request; retain its improvements and unresolved review.

## Isolations

- Baseline: `71618E6B89D311CC47522B0AAB1A8EAB873E5E054B6E3A5D426ECBBE4B0FA8DD`.
  Original V8 Ski Resort slot 17; hold player/vehicles at frame 30, native
  presentation capture at gameplay poll 601, 1920x1080.
- `artifacts/v82-side-clipping-20260904/cull-isolation`: disabling the final
  packet NCLIP facing cull did **not** remove the missing fence-edge wedges.
  Inspected the individual full-size PNG; do not solve this by disabling
  facing culling globally.
- `packet-geometry/runtime.log`: tick 300 retains exact address/value camera
  provenance for the near fence object's emitted triangles. A diagnostic
  owner address identifies packets for inspection only, never repair logic.
- Native mesh subdividers `80024E40`, `800259C8`, `80026694`, `8002737C`
  share scratch layout and screen rejection, subdivide for affine texturing,
  and drop leaves whose median depth is below 128. Enhanced clips at camera
  depth 1 and uses perspective-correct textures. Bypassing **only** the
  128-unit near region did not fix the screenshot; that experiment failed.
- `headroom-isolation`: compressing all native packed X coordinates further
  recovers the missing fence but shifts line/cable positions. Rejected as a
  shipping solution. This isolates the native screen-boundary rejection;
  the final repair must not modify projected positions.

## Shared mesh boundary repair

`V82MeshClipCompat.SubmitTriangle` submits the original exact triangle
at the four shared native mesh-subdivision entries, with all source color,
UV, CLUT/tpage, and native AVSZ3/OT ordering retained. The four packet forms
use the same hook. Missing/mismatched camera provenance falls back to retail.
Entirely behind-near geometry is rejected; intersecting triangles reach the
existing camera-space clipper. No content IDs select the repair.

The hardware clipper receives the unsaturated source geometry and owns the
viewport boundary. Camera/depth/UV information is not reconstructed from a
triangle already cut by the retail subdivider. Affine approximation leaves
are no longer needed on this exact, perspective-correct path. The existing
native path remains the fallback for incomplete or mixed camera provenance.
No terrain, vehicle-material, or global GTE projection changes are made.

`mesh-contract` executable:
`E5F7C112F2D4EBAECD4957DAD35BFF33C7D4F4D3DA446B8B600183E3F9A435E1`.
Full-size screenshot inspected: the fence extends through the left edge,
cables retain their original positions, and Sheila's roof remains opaque.
Capture: original V8 Ski Resort slot 17, held at vehicle frame 30, gameplay
poll 601, 1920x1080. Median/minimum: **60.04/50.20 FPS**. Technical aggregate
remains false for the intentionally stationary route; do not call it a
completed traversal or user visual pass.

Default-enabled test build:
`6BF6C96C44357BA305ECF67563807362CDE00B1CA499B834737271ED39F06674`.
The repair can be disabled diagnostically with
`RECOMPONE_V82_MODERN_MESH_CLIP=0`. The AI-edge fixture is strictly opt-in,
process-local, and moves existing native AI vehicles rather than injecting
synthetic draw calls. Offsets 140, 50 and 3 did not produce an in-frame
vehicle-edge proof; those captures are **not** accepted vehicle evidence.
Packet coordinates at offset 3 confirm the AI mesh is emitted but still
outside the visible left boundary (camera X approximately -3100, Z 2550).
Do not confuse a successful packet submission with a valid visual test.

Latest deployed executable:
`523CC0F9ECCF90B00A49136615E5A697B6A684DD755C86ED77CD4412A0493AEB`.
This adds render-time reapplication of the opt-in held AI test poses; normal
gameplay is unaffected. The `vehicle-render-pose` scenery image remains
correct (fence, cables, opaque Sheila roof), median/minimum **60.02/53.03 FPS**.
Capture point remains slot 17, held frame 30, gameplay poll 601, 1920x1080.

`dotnet run --project tools/recompone-v8-2/tests/MeshClipContract -c Release`
passes 159 assertions using real runtime memory/provenance and GTE: four
native packet layouts, OT linkage, exact vertex propagation, colors, UVs,
CLUT/tpage, stack/return preservation, near crossings, wholly behind-near
rejection, stale provenance fallback, and mixed-projection fallback.

## Vehicle-edge visual evidence

`vehicle-edges-unit` uses the same deployed `523CC0F9...493AEB` executable,
same slot 17/camera hold frame 30/gameplay poll 601/native 1920x1080 capture,
and opt-in AI lateral offset 1. Full-size image inspected individually:
`artifacts/v82-side-clipping-20260904/vehicle-edges-unit/recompone_present_gameplay_0601_1920x1080_fxaa.png`.
An existing AI bus intersects the left viewport edge and a second existing
AI vehicle intersects the right edge. Neither shows the fence-style missing
triangular wedges at the viewport boundary. Sheila's roof remains opaque.
An active weapon flash brightens the bus and fence; this is edge-geometry
evidence, not lighting/material approval. These are native game meshes,
not synthetic replacements or edited screenshots.

Median/minimum **59.94/51.04 FPS**, packet arena high-water **32.25%**,
maximum concurrent games **1**. All processes exited. Stationary route
checks intentionally fail; no claim of completed driving traversal.

Repeatable command: set `RECOMPONE_V82_PROOF_EDGE_VEHICLES=1` and
`RECOMPONE_V82_PROOF_EDGE_VEHICLE_OFFSET=1`, then run
`run_map_fidelity_gate.py --map-slots 17 --gameplay-frames 360
--capture-gameplay --hold-for-capture --presentation-resolution 1920x1080
--timeout 300 --output <new-artifact-directory>`.
Unset both variables after this process-local test. The lateral offset is
in native fixed-world units, not camera/model units; offsets 3 and above
are unsuitable for this camera. The fixture is disabled in normal runs.

Dedicated building-edge visual verification and user visual acceptance
remain pending. The paused five-map goal is not resumed; #10 is not closed.
