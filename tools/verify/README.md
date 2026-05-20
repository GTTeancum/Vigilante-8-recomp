# Verification Scaffolding

Implements the three deliverables from `PROJECT_SCOPE.md > Verification`:

1. **State snapshot serializer** — `state_snapshot.{h,c}` dumps the full
   game state struct (RNG, frame counter, match mode, vehicles[]) to a
   binary blob. Comparator does a byte-level diff.

2. **Input recorder / replayer** — `input_recorder.c` writes a per-frame
   stream of `V8InputCommand` and replays it.

3. **State comparator** — `V8_SnapshotCompare()` walks two snapshots and
   reports byte-level offsets that differ. Per-field labels come once
   the structs are finalized in pass 2.

## How to run a verification cycle

1. **Capture original.** Boot the original game in an instrumented PSX
   emulator (e.g. PCSX-Redux with a Lua hook). At each game-tick (60Hz),
   read the same global addresses listed in `include/globals.h` and emit
   a snapshot via the same blob layout. Capture an input log alongside.

2. **Replay through the port.** `main_test.c` (TBD) drives the port from
   the recorded input log, capturing a snapshot at the matching tick.

3. **Diff.** Pass both snapshots to `V8_SnapshotCompare`. Any difference
   means the port is not 1:1; the byte offset points at the diverged
   field. The diff procedure is documented; running it requires the
   instrumented emulator which is outside the decomp's scope.

## Status

- Snapshot header is partial: vehicles[] layout is LOW confidence and
  projectiles/match aren't included yet. The framework exists; field
  coverage grows as `include/structs.h` is filled in.
