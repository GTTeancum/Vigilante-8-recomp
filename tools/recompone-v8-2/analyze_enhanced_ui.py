#!/usr/bin/env python3
"""Headless composition checks for Enhanced menu/pause/objective captures."""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path


def read_ppm(path: Path) -> tuple[int, int, bytes]:
    with path.open("rb") as stream:
        if stream.readline().strip() != b"P6":
            raise ValueError(f"{path}: expected P6")
        dimensions = stream.readline()
        while dimensions.startswith(b"#"):
            dimensions = stream.readline()
        width, height = map(int, dimensions.split())
        if stream.readline().strip() != b"255":
            raise ValueError(f"{path}: expected 8-bit RGB")
        pixels = stream.read()
    if len(pixels) != width * height * 3:
        raise ValueError(f"{path}: truncated pixel payload")
    return width, height, pixels


def analyze(path: Path, label: str) -> dict[str, object]:
    width, height, pixels = read_ppm(path)
    yellow_x: dict[int, int] = {}
    yellow_y: dict[int, int] = {}
    gray_columns = [0] * width
    nonblack = 0
    for y in range(height):
        row = pixels[y * width * 3 : (y + 1) * width * 3]
        for x, (r, g, b) in enumerate(zip(row[0::3], row[1::3], row[2::3])):
            if max(r, g, b) >= 12:
                nonblack += 1
            if r >= 155 and g >= 115 and b <= 105 and r + g >= b * 3:
                yellow_x[x] = yellow_x.get(x, 0) + 1
                yellow_y[y] = yellow_y.get(y, 0) + 1
            if max(r, g, b) - min(r, g, b) <= 2 and 70 <= (r + g + b) / 3 <= 190:
                gray_columns[x] += 1

    dense_x = [x for x, count in yellow_x.items() if count >= max(6, height // 90)]
    dense_y = [y for y, count in yellow_y.items() if count >= max(8, width // 100)]
    yellow_box = None
    center_error_x = center_error_y = None
    if dense_x and dense_y:
        yellow_box = [min(dense_x), min(dense_y), max(dense_x), max(dense_y)]
        center_error_x = abs((yellow_box[0] + yellow_box[2]) / 2 - width / 2) / width
        center_error_y = abs((yellow_box[1] + yellow_box[3]) / 2 - height / 2) / height

    tall_gray_columns = sum(count >= height * 0.45 for count in gray_columns)
    failures: list[str] = []
    if nonblack / (width * height) < 0.15:
        failures.append("composition is mostly blank")
    if yellow_box is None:
        failures.append("authored yellow frame/title accents were not detected")
    else:
        if center_error_x is not None and center_error_x > 0.10:
            failures.append(f"overlay horizontal centering error {center_error_x:.3f}")
        if center_error_y is not None and center_error_y > 0.18:
            failures.append(f"overlay vertical centering error {center_error_y:.3f}")
    if tall_gray_columns > width * 0.08:
        failures.append(
            f"neutral-gray corruption spans {tall_gray_columns} full-height columns"
        )

    return {
        "label": label,
        "file": str(path),
        "width": width,
        "height": height,
        "nonblack_fraction": round(nonblack / (width * height), 6),
        "yellow_frame_bounds": yellow_box,
        "horizontal_center_error": (
            round(center_error_x, 6) if center_error_x is not None else None
        ),
        "vertical_center_error": (
            round(center_error_y, 6) if center_error_y is not None else None
        ),
        "tall_neutral_gray_columns": tall_gray_columns,
        "passed": not failures,
        "failures": failures,
    }


def write_bmp(path: Path, width: int, height: int, pixels: bytes) -> None:
    source_stride = width * 3
    output_stride = (source_stride + 3) & ~3
    pixel_bytes = output_stride * height
    header = bytearray(54)
    header[0:2] = b"BM"
    struct.pack_into("<I", header, 2, len(header) + pixel_bytes)
    struct.pack_into("<I", header, 10, len(header))
    struct.pack_into("<I", header, 14, 40)
    struct.pack_into("<i", header, 18, width)
    struct.pack_into("<i", header, 22, height)
    struct.pack_into("<H", header, 26, 1)
    struct.pack_into("<H", header, 28, 24)
    struct.pack_into("<I", header, 34, pixel_bytes)
    padding = b"\0" * (output_stride - source_stride)
    with path.open("wb") as stream:
        stream.write(header)
        for y in range(height - 1, -1, -1):
            source = pixels[y * source_stride : (y + 1) * source_stride]
            bgr = bytearray(source_stride)
            bgr[0::3] = source[2::3]
            bgr[1::3] = source[1::3]
            bgr[2::3] = source[0::3]
            stream.write(bgr)
            stream.write(padding)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--overlay",
        action="append",
        nargs=2,
        metavar=("LABEL", "PPM"),
        required=True,
    )
    parser.add_argument("--output", type=Path)
    parser.add_argument("--bmp-dir", type=Path)
    args = parser.parse_args()

    results = [
        analyze(Path(path).resolve(), label)
        for label, path in args.overlay
    ]
    if args.bmp_dir:
        args.bmp_dir.mkdir(parents=True, exist_ok=True)
        for label, path in args.overlay:
            width, height, pixels = read_ppm(Path(path).resolve())
            write_bmp(
                args.bmp_dir / f"{label}.bmp",
                width,
                height,
                pixels,
            )
    report = {
        "overlays": results,
        "passed": all(bool(result["passed"]) for result in results),
    }
    encoded = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded, encoding="utf-8")
    print(encoded, end="")
    raise SystemExit(0 if report["passed"] else 1)


if __name__ == "__main__":
    main()
