#!/usr/bin/env python3
"""Extract paletted terrain presentation images from TERRAIN/*.EXP.

This is a headless reverse-engineering aid, not a renderer screenshot tool.
It decodes the 8bpp image containers used by LOAD.DLL's XBMP/XBGM handlers:

  +0x04 u32 flags        (low two bits are PSX texture depth, 1 == 8bpp)
  +0x08 u32 image_off    (second descriptor base)
  +0x0c RECT clut_rect   (x, y, w, h)
  +0x14 u16 clut[256]
  +image_off+0x0c RECT image_rect (PSX width in 16-bit VRAM words)
  +image_off+0x14 u8 pixels[pixel_w*h] for 8bpp
"""
from __future__ import annotations

from pathlib import Path
import argparse
import struct

from PIL import Image

from terrain_runtime_decode import collect


ROOT = Path(__file__).resolve().parents[1]


def ule32(b: bytes, o: int) -> int:
    return struct.unpack_from("<I", b, o)[0]


def sle16(b: bytes, o: int) -> int:
    return struct.unpack_from("<h", b, o)[0]


def decode_psx555(c: int) -> tuple[int, int, int, int]:
    r = c & 0x1f
    g = (c >> 5) & 0x1f
    b = (c >> 10) & 0x1f
    return ((r << 3) | (r >> 2),
            (g << 3) | (g >> 2),
            (b << 3) | (b >> 2),
            255)


def rect(b: bytes, o: int) -> tuple[int, int, int, int]:
    return sle16(b, o), sle16(b, o + 2), sle16(b, o + 4), sle16(b, o + 6)


def pixel_width(rect_w_words: int, tex_depth: int) -> int:
    if tex_depth == 0:
        return rect_w_words * 4
    if tex_depth == 1:
        return rect_w_words * 2
    return rect_w_words


def decode_image_chunk(payload: bytes) -> tuple[Image.Image, dict]:
    if len(payload) < 0x220:
        raise ValueError("image chunk too small")
    flags = ule32(payload, 4)
    image_off = ule32(payload, 8)
    if (flags & 3) != 1:
        raise ValueError(f"unsupported PSX texture depth flags=0x{flags:x}")

    clut_rect = rect(payload, 0x0c)
    image_rect = rect(payload, image_off + 0x0c)
    depth = flags & 3
    w, h = pixel_width(image_rect[2], depth), image_rect[3]
    clut_data = payload[0x14:0x14 + 512]
    pix_off = image_off + 0x14
    pix = payload[pix_off:pix_off + w * h]
    if len(clut_data) != 512 or len(pix) != w * h:
        raise ValueError("truncated CLUT or pixel data")

    palette = [decode_psx555(struct.unpack_from("<H", clut_data, i * 2)[0])
               for i in range(256)]
    out = Image.new("RGBA", (w, h))
    out.putdata([palette[p] for p in pix])
    meta = {
        "flags": flags,
        "depth": depth,
        "clut_rect": clut_rect,
        "image_rect": image_rect,
        "pixel_size": (w, h),
        "image_off": image_off,
    }
    return out, meta


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("exp", nargs="?", default=str(ROOT / "input" / "TERRAIN" / "OILFIELD.EXP"))
    ap.add_argument("--outdir", default=str(ROOT / "analysis" / "terrain_images"))
    args = ap.parse_args()

    exp = Path(args.exp)
    data = exp.read_bytes()
    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)

    for tag in (b"XBMP", b"XBGM"):
        chunks = collect(data, tag, b"TERR")
        for idx, (_off, _size, _body, _parent, payload) in enumerate(chunks):
            img, meta = decode_image_chunk(payload)
            suffix = "" if len(chunks) == 1 else f"_{idx}"
            out = outdir / f"{exp.stem}_{tag.decode('ascii')}{suffix}.png"
            img.save(out)
            print(f"{out} {img.width}x{img.height} flags=0x{meta['flags']:x} "
                  f"depth={meta['depth']} clut={meta['clut_rect']} "
                  f"rect_words={meta['image_rect']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
