# Interstate '76 ZFSF archive

## Provenance

The retail Interstate '76 disc is mounted read-only at `D:\`. Its main
`I76.ZFS` archive is extracted locally to the ignored top-level `I76_ZFS/`
directory by `tools/i76_zfs_extract.py`. The source archive SHA-256 is:

`22AC82D6FE85293EC1E9BE01CD59F0626E34DEEA882FF7755EE24A69BA0A8E59`

The local extraction contains a generated `_i76_zfs_manifest.json` with every
entry's archive offset, size, timestamp, unknown header word, and SHA-256.
Original Interstate '76 data is intentionally excluded from Git.

## Container layout

All integers are little-endian.

```text
ZFS header (28 bytes)
  char[4] magic             "ZFSF"
  uint32  version           1 in this archive
  uint32  name_length       16
  uint32  files_per_page    100
  uint32  file_count        3847
  uint8[4] xor_key          all zero in this archive
  uint32  first_page        0x1c

Page
  uint32  next_page         absolute archive offset; zero on final page
  Entry entries[min(100, remaining file count)]
  byte data[]               entry offsets point into the archive

Entry (36 bytes here)
  char[16] name             NUL-padded ASCII leaf filename
  uint32  offset            absolute archive offset
  uint32  id                contiguous 0..3846
  uint32  size
  uint32  mtime             Unix timestamp
  uint32  unknown
```

The retail archive has 39 pages, 3,847 unique case-insensitive filenames, a
zero XOR key, contiguous entry IDs, and no out-of-bounds payload ranges.

## Vehicle conversion staging

There are 34 `.vdf` vehicle-model candidates. They remain untouched until the
planned vehicle conversion work. The extraction manifest is the authoritative
inventory; likely player vehicles use names such as `vazamz.vdf`,
`vcgcourc.vdf`, `vcmanta.vdf`, `vcroyale.vdf`, `vfpalmno.vdf`,
`vfrattlr.vdf`, `vjsovern.vdf`, `vmxmarx.vdf`, `vpjrabit.vdf`, and
`vppirnha.vdf`. Associated textures and metadata commonly use `.pak`, `.pix`,
`.vtf`, and `.vcf`.
