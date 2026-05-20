# EXP File Format (Vigilante 8)

Observed from headers of COMMON.EXP, HUD/ARMS.EXP, TERRAIN/*.EXP.

## Container

All EXP files are EA / IFF-style FORM containers, big-endian chunk sizes,
4-char-code tag identifiers. This matches the EA-IFF-85 convention.

```
offset  size  field
0x00    4     "FORM"           (magic, ASCII)
0x04    4     formSize         (big-endian; bytes after this field)
0x08    4     formType         (4cc: "TERR" common to all observed EXPs)
0x0C    ...   sub-chunks       (header chunk + nested FORMs)
```

## TERR formType sub-chunks observed

- `HEAD` -- file header / metadata (offset 0x0C). 26 bytes. Contains version / sizes / counts.
  Example bytes from AIRGRAVE.EXP HEAD:
  `0000 0040 0000 0000 0010 0000 000c 0005 0002 0005 0000 0000 0000`
  Likely: count[4] / size[4] / type[4] / counts...

- Nested `FORM`s with types observed:
  - `XOBF` (eXternal OBject Format?) -- mesh/model data
  - `XLSC` (eXternal Level Sub-Chunk?) -- AIRGRAVE.EXP begins with this
  - `BIN ` -- raw binary blob (with trailing space)

## Chunk reader (canonical)

```c
typedef struct {
    char     id[4];        /* 4cc */
    uint32_t size;         /* big-endian */
} IffChunkHeader;
```

Reader walks: read tag+size (size is BE), if tag=="FORM" read 4cc formType
then recurse into payload of (size-4) bytes. Otherwise skip `size` bytes
(payload often padded to 2-byte boundary in EA-IFF; verify per file).

## Resolved in DECOMP_PASS_2

- **Endianness of chunk size:** confirmed big-endian on PSX, matching
  EA-IFF-85 convention. The walker in `src/assets/iff_chunk.c` reads
  size as `(b[0]<<24)|(b[1]<<16)|(b[2]<<8)|b[3]`.
- **4cc inventory:** TERR (root formType), XOBF (model container,
  parsed by `src/assets/xobf_parse.c`), XLSC (level sub-chunk;
  contains terrain bind data), BIN<space> (raw payload, used by XOBF
  inner chunks and by raw mesh data), HEAD (26-byte header at start
  of every FORM), ANM (animation tracks, inside XOBF), SND (sound
  table, inside XOBF -- parsed by `src/assets/snd_parse.c`).
- **EXP loader:** identified -- `vehicle_exp.c` (Vehicles.EXP),
  `iff_chunk.c` (generic FORM walker), with per-formType handlers
  in xobf_parse / snd_parse. The `Asset_LoadEXP` hypothesis was a
  red herring -- the FORM walker is the entry point, not a separate
  per-format function.
- **`tools/exp_dump.py`:** not built; per-format loaders provide the
  introspection needed for the decomp goal. A dumper would help port
  development but is not required by CLAUDE.md.
