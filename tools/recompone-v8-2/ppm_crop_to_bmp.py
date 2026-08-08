#!/usr/bin/env python3
"""Crop a P6 PPM proof and enlarge it into a dependency-free 24-bit BMP."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

from ppm_to_bmp import token


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    parser.add_argument("--x", type=int, required=True)
    parser.add_argument("--y", type=int, required=True)
    parser.add_argument("--width", type=int, required=True)
    parser.add_argument("--height", type=int, required=True)
    parser.add_argument("--scale", type=int, default=1)
    args = parser.parse_args()
    data = args.source.read_bytes()
    cursor = 0
    values: list[bytes] = []
    for _ in range(4):
        value, cursor = token(data, cursor)
        values.append(value)
    if values[0] != b"P6" or values[3] != b"255":
        raise ValueError(f"{args.source}: unsupported PPM header")
    source_width, source_height = int(values[1]), int(values[2])
    cursor += 2 if data[cursor : cursor + 2] == b"\r\n" else 1
    pixels = memoryview(data)[cursor:]
    if (
        args.x < 0
        or args.y < 0
        or args.width <= 0
        or args.height <= 0
        or args.x + args.width > source_width
        or args.y + args.height > source_height
        or args.scale <= 0
    ):
        raise ValueError("crop rectangle or scale is outside the source")
    width, height = (
        args.width * args.scale,
        args.height * args.scale,
    )
    stride = (width * 3 + 3) & ~3
    output = bytearray(stride * height)
    for source_y in range(args.height):
        for scale_y in range(args.scale):
            destination_y = (
                height - 1 - (source_y * args.scale + scale_y)
            )
            row = destination_y * stride
            for source_x in range(args.width):
                source_offset = (
                    ((args.y + source_y) * source_width
                    + args.x + source_x)
                    * 3
                )
                red, green, blue = pixels[
                    source_offset : source_offset + 3
                ]
                for scale_x in range(args.scale):
                    destination_x = (
                        source_x * args.scale + scale_x
                    )
                    offset = row + destination_x * 3
                    output[offset : offset + 3] = bytes(
                        (blue, green, red)
                    )
    header_size = 54
    args.destination.parent.mkdir(parents=True, exist_ok=True)
    args.destination.write_bytes(
        b"BM"
        + struct.pack(
            "<IHHI",
            header_size + len(output),
            0,
            0,
            header_size,
        )
        + struct.pack(
            "<IIIHHIIIIII",
            40,
            width,
            height,
            1,
            24,
            0,
            len(output),
            2835,
            2835,
            0,
            0,
        )
        + output
    )
    print(
        f"[PpmCrop] source={args.source} "
        f"rect={args.x},{args.y},{args.width}x{args.height} "
        f"scale={args.scale} output={args.destination}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
