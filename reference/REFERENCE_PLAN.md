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
