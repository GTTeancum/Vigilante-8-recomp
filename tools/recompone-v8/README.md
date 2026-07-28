# V8 RecompOne preparation tools

`prepare_reference.py` converts the repository's checked-in Ghidra inventories
into the exact JSON schema accepted by RecompOne's `funcMap` option. It filters
out synthetic SDK/GTE symbols outside the loaded PS1 executable ranges and
preserves function names, addresses, and byte sizes.

The generated files are intentionally ignored. Regenerate them whenever the
analysis inventories or overlay manifest changes:

```powershell
python tools/recompone-v8/prepare_reference.py
```

Options:

```text
--cue PATH      repository-relative BIN/CUE entry point
--output PATH   repository-relative generated directory
```

`overlay_paths.json` separates source-proven disc paths from layout assumptions.
Validate its terrain paths against the actual disc before the first execution.

`prepare_loose_media.py` is the one-time bridge from a legally obtained disc
to the standalone/moddable asset tree. It writes the metadata-only retail LBA
manifest and expands `.STR`/`.XA` files to complete 2336-byte Mode 2 sectors.
The resulting `PS1 game` deployment does not need or search for BIN/CUE files.
That exact-path tree is also the supported asset-mod surface. C# runtime hooks
are confined to framework-dependent development builds until their detour
dependency supports the deployed single-file layout.

## State tracing

Set `RECOMPONE_STATE_TRACE_PATH` to write one JSON Lines state record at the
entry to every player-vehicle physics tick. Schema `v8-reference-state-v1`
contains the deterministic physics tick, original active-low pad words, exact
PRNG seed and one-byte carry, match mode, split-screen mode, and pointer-neutral
vehicle transforms, velocities, damage zones, and weapon inventories. It does
not write to emulated game state.

Compare two captures with:

```powershell
python tools/recompone-v8/compare_state_traces.py expected.jsonl actual.jsonl
```

The comparator reports the first divergent tick and field path. `--ignore`
can omit a deliberately uncontrolled field; `--allow-prefix`
validates a clean common prefix when a capture is interrupted. The later native
port should emit the same pointer-neutral schema when its gameplay loop is
ready. External PS1/emulator comparison is not part of the reference goal.

## Independent native vehicle runtime

V8 is fully supported by the same donor-free package and stable-ID roster used
by the V8:2 lead implementation, with sequel-only transformation and powerup
fields rejected. Select an append-only guest entry with:

```powershell
Vigilante8PC.exe --guest-vehicle author.vehicle_id retail-disc.cue
```

Set `RECOMPONE_V8_VEHICLE_PACKAGE` to the directory containing
`VEHICLES.V8R` and `CUSTOM.EXP`. The engine streams only the selected native
body bank, allocates independent stats and identity state, resets the original
match VRAM allocator, and reserves the bank's exact CLUT/image rectangles
before COMMON loads. The original texture loader claims those reservations at
construction; no retail character, donor bank, or opaque payload is used.
