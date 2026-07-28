# Interstate '76 VQM/CBK textures

The retail VQM texture format is a fixed 4x4 vector-quantized image using a
primary codebook and the active 256-color game palette.

## VQM header

All integer fields are little-endian.

| Offset | Type | Meaning |
|---:|---|---|
| 0x00 | `uint32_t` | Width in pixels |
| 0x04 | `uint32_t` | Height in pixels |
| 0x08 | `char[16]` | NUL-terminated primary `.CBK` filename |
| 0x18 | `uint16_t[]` | Block tokens in top-down row-major order |

The token count is exactly `ceil(width / 4) * ceil(height / 4)`.

## Block token

| Bits | Meaning |
|---:|---|
| 0-11 | Codebook entry index when bit 15 is clear |
| 12-14 | Zero in the verified retail assets |
| 15 | Solid-color block; fill all output pixels with palette index `token & 0xFF` |

Bit 15 does not select another codebook and is not a block flip. The retail
decoder at `I76.EXE` address `0x00483110` tests `0x8000`; when set it copies
the token's low byte to every destination pixel. Otherwise it reads sixteen
bytes from `primaryCodebook + token * 16`.

## CBK

A verified codebook is 65,540 bytes:

- `uint32_t entryCount`, always 4096
- 4096 entries of 16 bytes
- Each entry contains sixteen palette indices for a row-major 4x4 pixel block

Palette indices address the active 768-byte `.ACT` file as 256 RGB triples.
The Piranha Blender proof uses the retail `p02.act` palette.

## GEO cutout use

VQM itself stores palette indices, not a universal alpha channel. GEO faces
whose three-byte render flags have byte 1 equal to `5` use palette index 255
as transparent. This mode is present on the Piranha's `HL1` headlight cards,
`PP11BKX1` tail-light cards, `WA11LS01` wheel sidewalls, and weapon muzzle
disc materials. Opaque faces keep index 255 as an ordinary palette color.

The Blender importer creates a separate RGBA image/material variant only for
render-mode-5 faces, assigns alpha zero to index 255, and mixes the unlit
texture with a transparent shader.

## Verified example

`pp11ftf1.vqm` is 180x71, names `ORNGEVCL.CBK` as its primary codebook, and
contains 810 tokens. Of those, 398 are solid-color tokens. Expanding the file
with the retail rule above produces RGBA SHA-256
`317DCB8D0B825300B3219A5FCE20A147785D147D0BBEDC98132AF135C4F030A7`.
