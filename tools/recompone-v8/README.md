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
