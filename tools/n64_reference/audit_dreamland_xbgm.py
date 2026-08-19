#!/usr/bin/env python3
"""Audit Dreamland's converted panorama texture and transparency contract."""
from __future__ import annotations

import argparse
import json
import struct
from collections import Counter
from pathlib import Path

from terrain_runtime_decode import collect


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def s16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<h", data, offset)[0]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("exp", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    chunks = collect(args.exp.read_bytes(), b"XBGM", b"TERR")
    if len(chunks) != 1:
        raise SystemExit(f"expected one XBGM, found {len(chunks)}")
    payload = chunks[0][4]
    image_offset = u32(payload, 8)
    width = s16(payload, image_offset + 0x10) * 2
    height = s16(payload, image_offset + 0x12)
    pixels = payload[image_offset + 0x14:image_offset + 0x14 + width * height]
    palette = [struct.unpack_from("<H", payload, 0x14 + i * 2)[0]
               for i in range(256)]
    counts = Counter(pixels)
    transparent_indices = [i for i, color in enumerate(palette)
                           if (color & 0x7fff) == 0 and not (color & 0x8000)]
    transparent_texels = sum(counts[i] for i in transparent_indices)
    rows = []
    for y in range(height):
        row = pixels[y * width:(y + 1) * width]
        count = sum(value in transparent_indices for value in row)
        if count:
            runs = []
            start = None
            for x, value in enumerate(row + b"\xff"):
                clear = x < width and value in transparent_indices
                if clear and start is None:
                    start = x
                elif not clear and start is not None:
                    runs.append([start, x - 1])
                    start = None
            rows.append({"y": y, "transparent_texels": count, "runs": runs})
    report = {
        "asset": str(args.exp.resolve()),
        "width": width,
        "height": height,
        "transparent_palette_indices": transparent_indices,
        "transparent_texels": transparent_texels,
        "transparent_fraction": transparent_texels / max(1, len(pixels)),
        "transparent_rows": rows,
        "used_palette_entries": len(counts),
        "most_common_indices": counts.most_common(16),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report, indent=2))
    return 1 if transparent_texels else 0


if __name__ == "__main__":
    raise SystemExit(main())
