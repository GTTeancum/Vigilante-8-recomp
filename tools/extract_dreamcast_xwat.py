#!/usr/bin/env python3
"""Extract a Dreamcast V8:2 XWAT PAL4_TWIDDLED texture to PNG.

The level EXP is EA-IFF (big-endian outer chunk sizes), while the XWAT
payload contains little-endian CL32/PVRT subchunks.  This reads the asset
directly; no emulator or VRAM capture participates.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import struct

from PIL import Image


def morton2(x: int, y: int) -> int:
    result = 0
    bit = 0
    while (1 << bit) <= max(x, y):
        result |= ((y >> bit) & 1) << (2 * bit)
        result |= ((x >> bit) & 1) << (2 * bit + 1)
        bit += 1
    return result


def extract_xwat(data: bytes) -> tuple[list[tuple[int, int, int, int]], int, int, bytes]:
    marker = data.find(b"XWAT")
    if marker < 0 or marker + 8 > len(data):
        raise ValueError("EXP contains no XWAT chunk")
    size = struct.unpack_from(">I", data, marker + 4)[0]
    body = memoryview(data)[marker + 8:marker + 8 + size]

    if body[:4] != b"CL32":
        raise ValueError("XWAT does not begin with CL32")
    cl32_size = struct.unpack_from("<I", body, 4)[0]
    if cl32_size < 8 + 16 * 4:
        raise ValueError(f"short CL32 payload: {cl32_size:#x}")
    color_count = struct.unpack_from("<H", body, 12)[0]
    if color_count != 16:
        raise ValueError(f"expected 16 XWAT colours, got {color_count}")
    palette = [tuple(body[16 + index * 4:20 + index * 4]) for index in range(16)]

    pvrt = 8 + cl32_size
    if body[pvrt:pvrt + 4] != b"PVRT":
        raise ValueError("CL32 is not followed by PVRT")
    pvrt_size = struct.unpack_from("<I", body, pvrt + 4)[0]
    pixel_format = body[pvrt + 8]
    data_format = body[pvrt + 9]
    width, height = struct.unpack_from("<HH", body, pvrt + 12)
    if pixel_format != 0 or data_format != 5:
        raise ValueError(
            f"expected PAL4_TWIDDLED 00/05, got {pixel_format:02x}/{data_format:02x}"
        )
    pixels = bytes(body[pvrt + 16:pvrt + 8 + pvrt_size])
    expected = width * height // 2
    if len(pixels) != expected:
        raise ValueError(f"expected {expected} pixel bytes, got {len(pixels)}")
    return palette, width, height, pixels


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    palette, width, height, packed = extract_xwat(Path(args.input).read_bytes())
    rgba: list[tuple[int, int, int, int]] = []
    for y in range(height):
        for x in range(width):
            index = morton2(x, y)
            value = packed[index >> 1]
            palette_index = (value >> (4 * (index & 1))) & 0xF
            rgba.append(palette[palette_index])

    args.output.parent.mkdir(parents=True, exist_ok=True)
    image = Image.new("RGBA", (width, height))
    image.putdata(rgba)
    image.save(args.output)
    print(f"XWAT {width}x{height} PAL4_TWIDDLED -> {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
