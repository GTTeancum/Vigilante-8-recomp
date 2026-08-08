#!/usr/bin/env python3
"""Summarize Enhanced terrain scanline traces without visual inspection."""

from __future__ import annotations

import argparse
import json
import re
from collections import defaultdict
from pathlib import Path


TRACE = re.compile(
    r"^\[TerrainScanline\].*?\by=(-?\d+)\s+"
    r"x=(-?\d+(?:\.\d+)?)\.\.(-?\d+(?:\.\d+)?)"
)


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("log", type=Path)
    parser.add_argument("--expected-min", type=float, default=0.0)
    parser.add_argument("--expected-max", type=float, default=428.0)
    parser.add_argument("--merge-tolerance", type=float, default=0.01)
    parser.add_argument("--output", type=Path)
    return parser.parse_args()


def merge(
    intervals: list[tuple[float, float]], tolerance: float
) -> list[list[float]]:
    result: list[list[float]] = []
    for minimum, maximum in sorted(intervals):
        if maximum < minimum:
            minimum, maximum = maximum, minimum
        if not result or minimum > result[-1][1] + tolerance:
            result.append([minimum, maximum])
        elif maximum > result[-1][1]:
            result[-1][1] = maximum
    return result


def main() -> int:
    args = arguments()
    by_y: dict[int, list[tuple[float, float]]] = defaultdict(list)
    for line in args.log.read_text(encoding="utf-8", errors="replace").splitlines():
        match = TRACE.match(line)
        if match is None:
            continue
        y = int(match.group(1))
        by_y[y].append((float(match.group(2)), float(match.group(3))))

    lines = []
    for y in sorted(by_y):
        unions = merge(by_y[y], args.merge_tolerance)
        clipped = [
            [max(args.expected_min, start), min(args.expected_max, end)]
            for start, end in unions
            if end >= args.expected_min and start <= args.expected_max
        ]
        clipped = [interval for interval in clipped if interval[1] >= interval[0]]
        left_missing = (
            max(0.0, clipped[0][0] - args.expected_min)
            if clipped
            else args.expected_max - args.expected_min
        )
        right_missing = (
            max(0.0, args.expected_max - clipped[-1][1])
            if clipped
            else args.expected_max - args.expected_min
        )
        internal_gaps = [
            [clipped[index][1], clipped[index + 1][0]]
            for index in range(len(clipped) - 1)
            if clipped[index + 1][0] >
            clipped[index][1] + args.merge_tolerance
        ]
        lines.append(
            {
                "y": y,
                "triangle_intersections": len(by_y[y]),
                "unions": unions,
                "left_missing": left_missing,
                "right_missing": right_missing,
                "internal_gaps": internal_gaps,
            }
        )

    summary = {
        "schema": "v82-terrain-coverage-v1",
        "source": str(args.log.resolve()),
        "expected": [args.expected_min, args.expected_max],
        "scanline_count": len(lines),
        "scanlines": lines,
        "disconnected_scanlines": [
            line["y"] for line in lines if line["internal_gaps"]
        ],
        "left_edge_missing_scanlines": [
            line["y"] for line in lines if line["left_missing"] > 0.0
        ],
        "right_edge_missing_scanlines": [
            line["y"] for line in lines if line["right_missing"] > 0.0
        ],
    }
    text = json.dumps(summary, indent=2) + "\n"
    if args.output is not None:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text, encoding="utf-8")
    print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
