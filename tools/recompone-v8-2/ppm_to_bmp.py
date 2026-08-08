#!/usr/bin/env python3
"""Dependency-free P6 PPM to 24-bit BMP conversion for capture inspection."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


def token(data: bytes, cursor: int) -> tuple[bytes, int]:
    while cursor < len(data):
        if data[cursor] == 35:
            cursor = data.find(b"\n", cursor)
            if cursor < 0:
                raise ValueError("unterminated PPM comment")
        elif data[cursor] <= 32:
            cursor += 1
        else:
            break
    end = cursor
    while end < len(data) and data[end] > 32:
        end += 1
    return data[cursor:end], end


def convert(source: Path, destination: Path) -> None:
    data = source.read_bytes()
    cursor = 0
    values: list[bytes] = []
    for _ in range(4):
        value, cursor = token(data, cursor)
        values.append(value)
    magic, width_raw, height_raw, maximum_raw = values
    if magic != b"P6" or maximum_raw != b"255":
        raise ValueError(f"{source}: unsupported PPM header {values}")
    width, height = int(width_raw), int(height_raw)
    if cursor >= len(data) or data[cursor] > 32:
        raise ValueError(f"{source}: missing PPM header separator")
    cursor += (
        2
        if data[cursor : cursor + 2] == b"\r\n"
        else 1
    )
    rgb = memoryview(data)[cursor:]
    if len(rgb) != width * height * 3:
        raise ValueError(
            f"{source}: {len(rgb)} RGB bytes, expected {width * height * 3}"
        )
    stride = (width * 3 + 3) & ~3
    pixels = bytearray(stride * height)
    for output_row, source_row in enumerate(range(height - 1, -1, -1)):
        source_start = source_row * width * 3
        destination_start = output_row * stride
        for column in range(width):
            red, green, blue = rgb[
                source_start + column * 3 :
                source_start + column * 3 + 3
            ]
            offset = destination_start + column * 3
            pixels[offset : offset + 3] = bytes((blue, green, red))
    header_size = 14 + 40
    file_size = header_size + len(pixels)
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(
        b"BM"
        + struct.pack("<IHHI", file_size, 0, 0, header_size)
        + struct.pack(
            "<IIIHHIIIIII",
            40, width, height, 1, 24, 0, len(pixels),
            2835, 2835, 0, 0,
        )
        + pixels
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    args = parser.parse_args()
    convert(args.source, args.destination)
    print(f"[PpmToBmp] {args.source} -> {args.destination}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
