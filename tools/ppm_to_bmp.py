#!/usr/bin/env python3
"""Convert a binary P6 PPM capture to a dependency-free 24-bit BMP."""

from __future__ import annotations

import argparse
from pathlib import Path
import struct


def token(stream) -> bytes:
    while True:
        value = stream.readline()
        if not value:
            raise ValueError("truncated PPM header")
        value = value.split(b"#", 1)[0].strip()
        if value:
            return value


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("target", type=Path)
    args = parser.parse_args()

    with args.source.open("rb") as source:
        if token(source) != b"P6":
            raise ValueError("only binary P6 PPM is supported")
        width, height = map(int, token(source).split())
        if token(source) != b"255":
            raise ValueError("only 8-bit PPM is supported")
        rgb = source.read()
    stride = width * 3
    if len(rgb) != stride * height:
        raise ValueError("PPM pixel payload length does not match dimensions")

    bmp_stride = (stride + 3) & ~3
    image_size = bmp_stride * height
    with args.target.open("wb") as target:
        target.write(
            struct.pack(
                "<2sIHHI",
                b"BM",
                54 + image_size,
                0,
                0,
                54,
            )
        )
        target.write(
            struct.pack(
                "<IiiHHIIiiII",
                40,
                width,
                -height,
                1,
                24,
                0,
                image_size,
                2835,
                2835,
                0,
                0,
            )
        )
        padding = b"\0" * (bmp_stride - stride)
        for y in range(height):
            row = bytearray(rgb[y * stride : (y + 1) * stride])
            for x in range(0, stride, 3):
                row[x], row[x + 2] = row[x + 2], row[x]
            target.write(row)
            target.write(padding)


if __name__ == "__main__":
    main()
