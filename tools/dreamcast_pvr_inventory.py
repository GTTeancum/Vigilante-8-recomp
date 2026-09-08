#!/usr/bin/env python3
"""Inventory embedded Dreamcast PVRT headers without decoding VRAM state.

The V8:2 Dreamcast level containers keep ordinary textures as standard PVRT
chunks.  This scanner reads those file headers directly so renderer-parity
work can distinguish authored mip chains from filtering invented at runtime.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import pathlib
import struct


COLOR_FORMATS = {
    0x00: "ARGB1555",
    0x01: "RGB565",
    0x02: "ARGB4444",
    0x03: "YUV422",
    0x04: "BUMP",
    0x05: "PAL4",
    0x06: "PAL8",
}

DATA_FORMATS = {
    0x01: "TWIDDLED",
    0x02: "TWIDDLED_MIPMAP",
    0x03: "VQ",
    0x04: "VQ_MIPMAP",
    0x05: "PAL4_TWIDDLED",
    0x06: "PAL4_TWIDDLED_MIPMAP",
    0x07: "PAL8_TWIDDLED",
    0x08: "PAL8_TWIDDLED_MIPMAP",
    0x09: "RECTANGLE",
    0x0B: "STRIDE",
    0x0D: "RECTANGLE_TWIDDLED",
    0x10: "SMALL_VQ",
    0x11: "SMALL_VQ_MIPMAP",
}

MIPMAPPED_DATA_FORMATS = {0x02, 0x04, 0x06, 0x08, 0x11}


def scan(path: pathlib.Path):
    data = path.read_bytes()
    start = 0
    while True:
        marker = data.find(b"PVRT", start)
        if marker < 0:
            return
        start = marker + 4
        if marker + 16 > len(data):
            continue
        payload_size = struct.unpack_from("<I", data, marker + 4)[0]
        color_format = data[marker + 8]
        data_format = data[marker + 9]
        width, height = struct.unpack_from("<HH", data, marker + 12)
        if payload_size < 8 or marker + 8 + payload_size > len(data):
            continue
        if width == 0 or height == 0:
            continue
        yield marker, payload_size, color_format, data_format, width, height


def top_level_chunks(data: bytes):
    """Yield the direct IFF children of a FORM container."""
    if len(data) < 12 or data[:4] != b"FORM":
        return
    form_end = min(len(data), 8 + struct.unpack_from(">I", data, 4)[0])
    offset = 12
    while offset + 8 <= form_end:
        tag = data[offset : offset + 4].decode("ascii", errors="replace")
        size = struct.unpack_from(">I", data, offset + 4)[0]
        data_start = offset + 8
        data_end = data_start + size
        if data_end > form_end:
            return
        yield tag, data_start, data_end
        offset = data_end + (size & 1)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("paths", nargs="+", type=pathlib.Path)
    parser.add_argument(
        "--parents",
        action="store_true",
        help="also group PVRT headers by enclosing top-level IFF chunk",
    )
    args = parser.parse_args()

    total = 0
    mipmapped = 0
    for path in args.paths:
        data = path.read_bytes()
        parents = list(top_level_chunks(data) or ())
        counts: collections.Counter[tuple[int, int, int, int]] = collections.Counter()
        parent_counts: collections.Counter[tuple[str, int, int]] = collections.Counter()
        file_total = 0
        file_mipmapped = 0
        for offset, _, color, layout, width, height in scan(path):
            counts[(color, layout, width, height)] += 1
            parent = next(
                (tag for tag, begin, end in parents if begin <= offset < end), "(none)"
            )
            parent_counts[(parent, layout, color)] += 1
            file_total += 1
            file_mipmapped += layout in MIPMAPPED_DATA_FORMATS
        total += file_total
        mipmapped += file_mipmapped
        digest = hashlib.sha256(data).hexdigest().upper()
        print(f"FILE {path} sha256={digest}")
        print(f"  textures={file_total} mipmapped={file_mipmapped}")
        for (color, layout, width, height), count in sorted(counts.items()):
            color_name = COLOR_FORMATS.get(color, f"UNKNOWN_{color:02X}")
            layout_name = DATA_FORMATS.get(layout, f"UNKNOWN_{layout:02X}")
            print(f"  {count:4d} {width:4d}x{height:<4d} {color_name:<10s} {layout_name}")
        if args.parents:
            print("  parent chunks:")
            for (parent, layout, color), count in sorted(parent_counts.items()):
                color_name = COLOR_FORMATS.get(color, f"UNKNOWN_{color:02X}")
                layout_name = DATA_FORMATS.get(layout, f"UNKNOWN_{layout:02X}")
                print(f"    {count:4d} {parent:<4s} {color_name:<10s} {layout_name}")
    print(f"TOTAL textures={total} mipmapped={mipmapped}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
