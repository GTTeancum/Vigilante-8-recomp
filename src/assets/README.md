# src/assets

Asset loaders -- everything that turns binary game data on disc into
in-memory structs.

## Status

DECOMP_PASS_2: **complete.** All in-scope asset loaders are
hand-cleaned at HIGH confidence (foundation primitives) or MED
(per-format parsers).

## Foundation primitives (HIGH)

- `heap.c` -- K&R first-fit free-list heap.
- `mem.c`  -- V8_MemSet/MemCopy/MemMove (libc-style with 16-byte unroll).
- `cd_io.c` -- low-level CD sector reader.
- `iso9660.c` -- ISO9660 directory cache + PVD reader.

## Format parsers (HIGH/MED)

- `overlay_loader.c` -- DLL overlay load+relocate (tag 0/1/2/3 =
  abs32/HI16/abs16/J26). Cleaned and bit-exact.
- `iff_chunk.c` -- IFF FORM 4cc + BE-size chunk walker.
- `xobf_parse.c` -- XOBF inner container with BIN/ANM/SND dispatch.
- `xobf_health.c` -- per-object health-table extractor.
- `vehicle_exp.c` -- Vehicles.exp 14-entry roster loader.
- `snd_loader.c`, `snd_parse.c` -- SND sound bank loader + parser.
- `cd_audio.c` -- CD audio TOC + XA streamer.
- `font_load.c`, `tbl_loader.c`, etc.

## File layout

```
heap.c             -- the heap
mem.c              -- memcpy/memset/memmove
cd_io.c            -- low-level CD reads
iso9660.c          -- directory iteration
overlay_loader.c   -- DLL load + relocate
iff_chunk.c        -- FORM-IFF chunk walker
xobf_*.c           -- XOBF parser family
vehicle_exp.c      -- Vehicles.exp loader
snd_*.c            -- sound bank loader/parser
cd_audio.c         -- CD-DA + XA streamer
font_load.c        -- .FNT loader
str_streamer.c     -- video .STR streamer (the seam to the out-of-scope
                      MDEC video player documented in src/skipped/load_renderer.c)
```

## Format notes

See `notes/formats/exp_format.md` and `notes/formats/dll_format.md`
for the on-disc layouts. TBL / BIN / FNT / SND headers follow a
shared `u32 count + u32 offset[count]` table-of-contents pattern.

## What's still UNKNOWN

- `QUEST.BIN`, `LOCATNS.BIN`, `OPTIONS.BIN`, `CURSOR.BIN`: TBL
  header decoded, per-entry payload schemas still UNKNOWN.
  Deferred to pass 3 / Ghidra interactive session.
