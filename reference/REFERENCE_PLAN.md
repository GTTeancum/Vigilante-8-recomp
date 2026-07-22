# Vigilante 8 reference execution plan

## Architecture

| Lane | Responsibility |
|---|---|
| Native C runtime | Shipping implementation and existing asset/runtime work |
| RecompOne reference | Instruction-faithful execution, hooks, RAM/register traces |
| Original disc | Behavioral and asset source of truth |
| Comparison tools | First-divergence detection under identical recorded inputs |

RecompOne output is not treated as readable reconstructed source and is not
linked into the shipping C executable. It is an oracle used to prove behavior.

## Current phase: boot-to-game-over vertical slice

The reference lane now has an explicit end-to-end fidelity target. It must run
the original boot and intro sequence, expose the complete one-player menu path,
load one deterministic match, exercise vehicle physics and weapons, and reach
the original game-over/results flow. Graphics, music, sound effects, voices,
and their transition rules are part of this reference target.

This is a reference-lane expansion, not a change to the shipping architecture.
The native Xbox game remains a separate 32-bit x86 implementation. Its renderer,
controls, and playback engine may be rewritten, but the reference is used to
recover the original state rules, asset interpretation, event timing, and
observable behavior for those systems.

Detailed gates and evidence requirements are tracked in
`VERTICAL_SLICE.md`.

## Bring-up sequence

1. Generate function maps from the existing main-EXE and 12-overlay Ghidra
   inventories.
2. Confirm the disc paths and bring up the main executable in RecompOne.
3. Confirm `SHELL`, `LOAD`, and one terrain overlay load and dispatch correctly.
4. Stub or patch presentation-only blockers only where necessary to reach a
   deterministic gameplay tick.
5. Add a trace schema shared conceptually by both runtimes: frame, function,
   object address, event, RNG state, selected RAM ranges, and changed words.
6. Replay a short fixed input sequence and locate the first divergent frame.
7. Correct the native C implementation from original instruction/state
   evidence, then keep the trace as a regression fixture.

## First gameplay targets

1. Pickup population and type selection.
2. Weapon attachment, detachment, ammo, and callback lifecycle.
3. Seeker targeting and projectile retirement.
4. Destructible-prop collision class, damage, and replacement state.
5. AI spawn and vehicle-control dispatch.
6. Vehicle physics fields that remain semantically uncertain.

## HUD and UI targets

The clean renderer should reproduce behavior, not the PS1 GPU implementation.
Trace these source rules before rewriting presentation:

- Health, armor, ammo, selected weapon, targeting, radar, score, timer, warning,
  and split-screen visibility state.
- Flash cadence, animation counters, colors, primitive ordering, and transition
  timing.
- Menu graph, focus movement, disabled/hidden entries, confirm/cancel behavior,
  option persistence, character/vehicle selection, pause, and results flow.
- Text identifiers, font metrics, texture ownership, coordinates, palettes, and
  sound-event triggers.

Useful capture points are function pre/post hooks, GPU primitive submission,
font draw calls, controller-state reads, overlay load events, and RAM writes to
known HUD/menu globals.

## Acceptance rule

RecompOne earns a permanent place in the workflow once one meaningful original
gameplay path can be replayed and compared at the state level. It remains a
reference tool; the existing C project remains authoritative for the port.

The executable-gameplay portion of this rule was reached on 2026-07-21:
Oilfield ran under scripted steering, acceleration, and weapon input for 272.7
seconds without a runtime error, and the deployed copy passed a second active
smoke test. Shared state-trace capture and first-divergence comparison remain
the next reference-lane work rather than prerequisites for running the game.

The current vertical slice is complete only when a clean launch reaches
game-over/results through a reproducible input fixture and every gate in
`VERTICAL_SLICE.md` has current runtime evidence. Process responsiveness or a
single gameplay screenshot is not sufficient.

## First post-goal presentation phase — complete

The reference host now offers 720p, 1080p, 1440p, and 4K output canvases,
desktop-resolution fullscreen, 4x internal 3D rasterization, and optional FXAA.
The PS1 primitive stream is rasterized at four times the native dimensions
before FXAA and final 4:3 presentation. UI coordinates, VRAM data, fixed-point
gameplay state, and timing remain unchanged.

The original-framebuffer presentation pass covered the complete boot-to-result
route. The corrected 4x 3D path separately completed the title/menu route,
active Oil Fields physics and weapon rendering, and the complete pause and
confirmation flow. Its exact 1080p captures show that vehicle silhouettes and
other polygon edges are generated at higher resolution before FXAA, rather than
merely filtering enlarged native-resolution stair steps.

This work does not relax the boot-to-results, audio, graphics, stability, or
state-evidence gates and does not change the separate native 32-bit x86 Xbox
architecture.
