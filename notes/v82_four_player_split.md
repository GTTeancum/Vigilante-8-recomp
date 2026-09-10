# Four-player split-screen implementation

Status: direct-map four-player harness implemented and baseline measured. This is not a menu/controller or release-readiness milestone.

## Requested acceptance

- One native V8:2 match, four independent player vehicles and two AI bots.
- Direct-map, process-local harness; no host input or desktop automation.
- Four cameras and HUDs with behavior equivalent to existing two-player split.
- Preserve rendering fidelity, deploy tested executable to V8_2_LOOSE, capture
  through the game's native presentation facility, report measured FPS.
- Multiplayer has six combatants maximum: 2P/4 bots, 3P/3 bots, 4P/2 bots.
  Single-player keeps its existing rules. Three-player menus are future scope.

## Source findings

Generated files live in reference-v8-2/generated/recompiled. Changes require
durable source patching/build integration rather than edits only to generated C#.

- main.func_800132CC owns match initialization and frame loop.
- GP+0x1104 contains six participant type bytes. The resource-mask loader
  already iterates all six. V82Compat.TraceCommonObjectLoadPre rebuilds masks.
- GP+0x31 is match mode. Modes 5..8 select ordinary two-player behavior;
  modes >=9 use four primary resource slots, but this is not evidence of four
  working local views: spawn and render paths still explicitly handle two.
- GP+0x2C is split layout: 0 full, 1 horizontal, 2 vertical; value 3 configures
  160x120 projection but does not provide four local render blocks.
- func_8002DF30 sets projection width/height and center. func_8002D278 installs
  each camera. func_80031678 traverses and renders the world.
- main L80013E80..L800143FC contains two explicit camera/render/HUD blocks.
  Full-screen path begins L800143FC. L8001473C returns to frame-loop control.
- func_80014B3C selects one of two primitive/ordering-table buffers; its A0
  is a buffer index, not a player index. V82Compat expands primitive capacity.
- Draw environments at 0x8006BEA8 have stride 0x5C, ten entries: two full-screen,
  four horizontal halves and four vertical halves. Divider packets at
  0x8006BB50. These cannot be blindly indexed beyond their native allocation.
- Player vehicle pointers 0x8006B7E8 and +4 are immediately followed by other
  globals. Likewise native input state 0x800B4A68 has only two 0x18-byte records,
  with a separate structure beginning at 0x800B4A98. Do not extend in place.
- main.func_80015540 converts two pad records. LibPad and Controller expose
  two ports. InputManager owns the existing process-local scripted pulses.
- Player identity is negative object+0x0A: -1 and -2, read with bitwise complement
  to index participant/input/team tables. AI IDs are nonnegative.
- main L8003A9D4 and L8003C1A4 index 0x800B4A68 via identity. Other player-input
  references must be inventoried before relocating/expanding the records.
- main.func_8003208C takes a negative player ID, resolves a map spawn record
  through func_80030250(GP+0x1078, ID), and calls func_80036C2C. The latter is the
  vehicle factory, with registry hooks; func_8003066C activates spawned objects.
- SHELL_LOAD L80104E84..L80104F88 explicitly creates only player -1 and -2.
  L80105040 calls func_80103D94 for each and sets initial cross-target pointers.
  Extra player spawns need valid map records and the same initialization.
- main.func_80036E78 loads HUD sprite resources (not vehicle spawning).

## Harness and performance

Existing scripts: tools/recompone-v8-2/input-scripts/native_2p_versus_soak.txt
and native_2p_cooperative_soak.txt. These navigate retail menus, so a direct-map
entry still needs implementation.

Use RECOMPONE_WINDOW_VISIBLE=0, RECOMPONE_DISABLE_LIVE_INPUT=1,
RECOMPONE_SUPPRESS_RUMBLE=1 and subprocess CREATE_NO_WINDOW. Native capture via
RECOMPONE_PRESENTATION_CAPTURE and RECOMPONE_CAPTURE_DIR is allowed.
Measure without verbose rendering/input diagnostics. Record resolution, arena,
duration, FPS statistics and confirmed six-object roster. Do not infer FPS from
earlier single-player ~50 reports.

Prior staged executable SHA256 (before any 4P edits):
A6A307099366ED44E4EC05B3C3926624EB0205E75C84EC40A9136BA1C241984E.
Preserve existing uncommitted Houston/Y source-port and rendering work.

## Implementation milestone (2026-09-10)

Opt-in V82SplitScreenHarness.cs now launches a six-actor match, renders four
160x120 native quadrants, and drives four vehicles with process-local input.
Tests exposed two-player bounds in processed input, camera pointers, target
matrices, upgrades, rumble, and per-player model overrides. These accesses now
use separately allocated state for players three and four. Model override
slots 42/43 are level objects, so extra players require their own empty override
cells and the original vehicle-bank fallback. Native digital controller type
is 2, with mapping row at 800637EC.

World and HUD submissions finish before recycling either native primitive
arena. The current edge-repair queue is consumed for each view. Camera focal
length is halved for the half-height viewport to preserve vertical framing.
No geometry, texture, effects, AI, or draw-distance fidelity reductions.

Evidence: artifacts/split-screen-20260910/digital-4p contains correct models
and independently changing positions for all four humans and both bots.
The dark area in the Bayou fourth viewport is water, confirmed by the user.
The earlier camera-obstruction interpretation was incorrect. This is a harness only; native menus, general
three/four-player match-end flows, and hardware controllers are not enabled.

## Harness acceptance and measured baseline

Final staged executable SHA256:
BEA79306AA5D8AB86286D29CD540FAEBDE14BEC7A64FE133992C2107B3750F70

Bayou, Enhanced preset, 1280x720 presentation with native 4:3 content,
unthrottled, hidden native window: four local human vehicles and two bots.
The native active-object list contains exactly six distinct combatants.
All six change horizontal position; P3/P4 mapped input matches the native
digital controller mapping. No external input or desktop capture was used.
No geometry, textures, effect quality, or draw distance was reduced.

Capture-free baseline: 61.02 seconds total process time, 800 input polls,
48 one-second FPS samples after loading and five gameplay warmup samples.
Median 13.84 FPS; minimum 12.30; maximum 15.32. Minimum is the minimum
one-second sample, not an instantaneous worst frame. No PPM captures were
generated during this run, and no edge-repair queue exhaustion occurred.
Evidence: artifacts/split-screen-20260910/baseline-clean-4p/result.json,
runtime.log and movement.json. Earlier baseline-4p is superseded because
a native stage capture still occurred in that run.

Visual evidence: artifacts/split-screen-20260910/driving-4p/
recompone_present_gameplay_0350_1280x720_fxaa.png. Four independent viewports
and the four correct vehicle models are present. Native HUD uses the target
vehicle silhouette, so differing silhouettes are not a player-model mismatch.

Limits: Route66 camera/geometry distortion was observed. The dark Bayou P4
area is water, not evidence of camera obstruction or an out-of-bounds spawn. Route66 distortion reproduces with the
native two-view renderer in route66-2p; it is not established as caused by
the four-view renderer. These are not declared fixed or a whole-map visual
regression pass. Existing two-player match-end/pause logic is still native;
this pass is direct harness gameplay, not menu integration or release-ready
three/four-player match lifecycle support. Future roster policy is 2+4, 3+3,
4+2, while normal single-player remains outside the opt-in path.

Validation: release publish passed; combined vehicle-engine and split source
patches are idempotent; final six-actor native run exited zero; saved cheat
preferences remain 1048640. All harness-owned player tables are separate
allocations instead of overwriting adjacent native globals.

Run from repository root:

    python tools/recompone-v8-2/run_split_screen_harness.py --players 4 --output artifacts/my-4p-proof --polls 500

For measurement, add --baseline (disables both presentation and stage captures).

## Simplified 4P HUD
Four-player views show the retail segmented health pill at bottom-left and the equipped secondary weapon sprite with bare ammunition at bottom-right. There are no HP/W/AM labels, status panels, or bottom strip. Empty selection leaves the weapon corner empty. Health retains the native paired-part calculation and low-health blink. Weapon sprites use the native per-weapon descriptor or generic icon atlas; ammunition uses the retail HUD font. Per-player sprite copies prevent one viewport from overwriting another's HUD.

Full-display margin anchoring excludes smaller split viewports. An explicit viewport-HUD submission scope supports health artwork recognition away from the top of the full display. Normal 1P/2P HUD submission remains unchanged.

Staged build: 5F2AC88A778CFA909AF67120403C9FA447DDDE80F72EDF9BB56050CE1A2A1766. Release publish passed. Native unarmed and equipped visual runs exited zero and verified four humans plus two AI. Equipped fixture uses the retail pickup path, with weapon kinds 1/2/3/4 and ammo 7/12/99/100; enable explicitly with --hud-fixture. It is disabled by default. Native captures checked for bottom-corner placement, individual weapon/ammo state, and absence of panels/labels. Proof: artifacts/split-screen-20260910/native-hud-equipped-4p/proof.png. These were visual checks, not a new performance baseline.
