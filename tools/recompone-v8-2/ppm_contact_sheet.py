#!/usr/bin/env python3
"""Dependency-free P6 PPM contact sheets for full visual regression matrices."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

from ppm_to_bmp import token


def read_ppm(path: Path) -> tuple[int, int, memoryview]:
    data = path.read_bytes()
    cursor = 0
    values: list[bytes] = []
    for _ in range(4):
        value, cursor = token(data, cursor)
        values.append(value)
    if values[0] != b"P6" or values[3] != b"255":
        raise ValueError(f"{path}: unsupported PPM header {values}")
    width, height = int(values[1]), int(values[2])
    if cursor >= len(data) or data[cursor] > 32:
        raise ValueError(f"{path}: missing PPM header separator")
    cursor += (
        2
        if data[cursor : cursor + 2] == b"\r\n"
        else 1
    )
    pixels = memoryview(data)[cursor:]
    if len(pixels) != width * height * 3:
        raise ValueError(f"{path}: incomplete RGB payload")
    return width, height, pixels


def write_bmp(path: Path, width: int, height: int, rgb: bytearray) -> None:
    stride = (width * 3 + 3) & ~3
    pixels = bytearray(stride * height)
    for output_row, source_row in enumerate(range(height - 1, -1, -1)):
        for column in range(width):
            source = (source_row * width + column) * 3
            destination = output_row * stride + column * 3
            pixels[destination : destination + 3] = bytes(
                (rgb[source + 2], rgb[source + 1], rgb[source])
            )
    header_size = 54
    path.write_bytes(
        b"BM"
        + struct.pack("<IHHI", header_size + len(pixels), 0, 0, header_size)
        + struct.pack(
            "<IIIHHIIIIII",
            40, width, height, 1, 24, 0, len(pixels),
            2835, 2835, 0, 0,
        )
        + pixels
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    parser.add_argument("inputs", type=Path, nargs="+")
    parser.add_argument("--columns", type=int, default=4)
    parser.add_argument("--cell-width", type=int, default=480)
    args = parser.parse_args()
    sources = [read_ppm(path) for path in args.inputs]
    aspect = sources[0][1] / sources[0][0]
    cell_width = args.cell_width
    cell_height = round(cell_width * aspect)
    columns = max(1, args.columns)
    rows = (len(sources) + columns - 1) // columns
    width, height = columns * cell_width, rows * cell_height
    output = bytearray(width * height * 3)
    for index, (source_width, source_height, pixels) in enumerate(sources):
        cell_x = index % columns * cell_width
        cell_y = index // columns * cell_height
        for y in range(cell_height):
            source_y = min(source_height - 1, y * source_height // cell_height)
            for x in range(cell_width):
                source_x = min(source_width - 1, x * source_width // cell_width)
                source = (source_y * source_width + source_x) * 3
                destination = (
                    (cell_y + y) * width + cell_x + x
                ) * 3
                output[destination : destination + 3] = pixels[
                    source : source + 3
                ]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    write_bmp(args.output, width, height, output)
    print(
        f"[PpmContactSheet] inputs={len(sources)} "
        f"size={width}x{height} output={args.output}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
