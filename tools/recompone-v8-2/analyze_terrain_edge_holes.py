#!/usr/bin/env python3
"""Measure backdrop-coloured terrain holes in a binary PPM capture.

The tool deliberately reports pixels rather than interpreting game geometry.
For each row in a configurable region it measures backdrop-coloured runs that
touch the left or right edge.  This makes widescreen regressions comparable
between renderer experiments without relying on resized screenshots.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path


def parse_rgb(value: str) -> tuple[int, int, int]:
    parts = value.split(",")
    if len(parts) != 3:
        raise argparse.ArgumentTypeError("RGB must be R,G,B")
    try:
        rgb = tuple(int(part) for part in parts)
    except ValueError as error:
        raise argparse.ArgumentTypeError("RGB values must be integers") from error
    if any(channel < 0 or channel > 255 for channel in rgb):
        raise argparse.ArgumentTypeError("RGB values must be in 0..255")
    return rgb  # type: ignore[return-value]


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("image", type=Path)
    parser.add_argument("--reference", type=parse_rgb, default=(255, 214, 156))
    parser.add_argument(
        "--tolerance",
        type=int,
        default=6,
        help="maximum absolute difference allowed in every RGB channel",
    )
    parser.add_argument("--x0", type=int, default=0)
    parser.add_argument("--x1", type=int)
    parser.add_argument("--y0", type=int, default=0)
    parser.add_argument("--y1", type=int)
    parser.add_argument(
        "--min-run",
        type=int,
        default=1,
        help="minimum matching edge run to include in defect row lists",
    )
    parser.add_argument("--output", type=Path)
    return parser.parse_args()


def read_token(data: bytes, offset: int) -> tuple[bytes, int]:
    while offset < len(data):
        if data[offset] == ord("#"):
            newline = data.find(b"\n", offset)
            if newline < 0:
                raise ValueError("unterminated PPM comment")
            offset = newline + 1
        elif chr(data[offset]).isspace():
            offset += 1
        else:
            break
    end = offset
    while end < len(data) and not chr(data[end]).isspace():
        end += 1
    if end == offset:
        raise ValueError("truncated PPM header")
    return data[offset:end], end


def read_ppm(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    magic, offset = read_token(data, 0)
    width_text, offset = read_token(data, offset)
    height_text, offset = read_token(data, offset)
    maximum_text, offset = read_token(data, offset)
    if magic != b"P6":
        raise ValueError(f"{path}: expected binary P6 PPM")
    width = int(width_text)
    height = int(height_text)
    if int(maximum_text) != 255:
        raise ValueError(f"{path}: only 8-bit PPM data is supported")
    while offset < len(data) and chr(data[offset]).isspace():
        offset += 1
    expected = width * height * 3
    pixels = data[offset:]
    if len(pixels) != expected:
        raise ValueError(
            f"{path}: pixel payload is {len(pixels)} bytes; expected {expected}"
        )
    return width, height, pixels


def merge_rows(rows: list[int]) -> list[list[int]]:
    intervals: list[list[int]] = []
    for row in rows:
        if not intervals or row != intervals[-1][1] + 1:
            intervals.append([row, row])
        else:
            intervals[-1][1] = row
    return intervals


def main() -> int:
    args = arguments()
    width, height, pixels = read_ppm(args.image)
    x0 = max(0, args.x0)
    x1 = min(width, width if args.x1 is None else args.x1)
    y0 = max(0, args.y0)
    y1 = min(height, height if args.y1 is None else args.y1)
    if x0 >= x1 or y0 >= y1:
        raise ValueError("analysis region is empty")
    if args.tolerance < 0 or args.min_run < 1:
        raise ValueError("tolerance must be >= 0 and min-run must be >= 1")

    reference = args.reference

    def matches(x: int, y: int) -> bool:
        offset = (y * width + x) * 3
        return all(
            abs(pixels[offset + channel] - reference[channel]) <= args.tolerance
            for channel in range(3)
        )

    rows = []
    total_matching = 0
    left_defect_rows: list[int] = []
    right_defect_rows: list[int] = []
    for y in range(y0, y1):
        matching = sum(1 for x in range(x0, x1) if matches(x, y))
        total_matching += matching

        left_run = 0
        for x in range(x0, x1):
            if not matches(x, y):
                break
            left_run += 1

        right_run = 0
        for x in range(x1 - 1, x0 - 1, -1):
            if not matches(x, y):
                break
            right_run += 1

        if left_run >= args.min_run:
            left_defect_rows.append(y)
        if right_run >= args.min_run:
            right_defect_rows.append(y)
        if matching or left_run or right_run:
            rows.append(
                {
                    "y": y,
                    "matching_pixels": matching,
                    "left_edge_run": left_run,
                    "right_edge_run": right_run,
                }
            )

    summary = {
        "schema": "v82-terrain-edge-holes-v1",
        "source": str(args.image.resolve()),
        "image_size": [width, height],
        "region": [x0, y0, x1, y1],
        "reference_rgb": list(reference),
        "per_channel_tolerance": args.tolerance,
        "minimum_reported_run": args.min_run,
        "matching_pixels": total_matching,
        "left_edge": {
            "row_count": len(left_defect_rows),
            "row_intervals": merge_rows(left_defect_rows),
            "maximum_run": max(
                (row["left_edge_run"] for row in rows),
                default=0,
            ),
        },
        "right_edge": {
            "row_count": len(right_defect_rows),
            "row_intervals": merge_rows(right_defect_rows),
            "maximum_run": max(
                (row["right_edge_run"] for row in rows),
                default=0,
            ),
        },
        "rows": rows,
    }
    text = json.dumps(summary, indent=2) + "\n"
    if args.output is not None:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text, encoding="utf-8")
    print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
