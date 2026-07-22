# Verification Scaffolding

Implements the three deliverables from `PROJECT_SCOPE.md > Verification`:

1. **Native-port snapshot serializer** — `state_snapshot.{h,c}` dumps the
   currently recovered native state (RNG, frame counter, match mode, and the
   two player vehicles) to a binary blob. Comparator does a byte-level diff.

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

2. **Replay through the port.** Drive the abstract input seam from the recorded
   log and call `V8_SnapshotCapture` at the matching gameplay tick. The native
   game-loop integration remains to be added when that loop is linkable.

3. **Diff.** Pass both snapshots to `V8_SnapshotCompare`. Any difference
   means the port is not 1:1; the byte offset points at the diverged
   field. The diff procedure is documented; running it requires the
   instrumented emulator which is outside the decomp's scope.

## Status

- The native snapshot remains partial: its `Vehicle` layout contains host
  pointers and unresolved padding, AI vehicles and projectiles are absent, and
  there is not yet a linkable replay driver. It is scaffolding, not evidence of
  full-state fidelity.
- The running RecompOne reference now supplies immediately usable,
  pointer-neutral per-tick JSONL traces through `RECOMPONE_STATE_TRACE_PATH`.
  `tools/recompone-v8/compare_state_traces.py` reports the first divergent tick
  and field. Cross-runtime validation still requires an external emulator
  capture or the native gameplay loop; neither is available locally.
- The PRNG carry at `0x800568d8` is one byte (`LBU`/`SB`), not an `int32_t`.
  Binary snapshot schema version 3 records the corrected type with explicit
  padding.
