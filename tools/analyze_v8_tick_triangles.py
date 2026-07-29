#!/usr/bin/env python3
"""Rank traced V8 GPU triangles by screen extent and thinness."""

from __future__ import annotations

import argparse
from pathlib import Path
import re


TRIANGLE = re.compile(
    r"\[V8TickTriangle\] tick=(?P<tick>-?\d+) "
    r"packet=(?P<packet>0x[0-9A-Fa-f]+).*?"
    r"xy=\((?P<x0>-?\d+),(?P<y0>-?\d+)\),"
    r"\((?P<x1>-?\d+),(?P<y1>-?\d+)\),"
    r"\((?P<x2>-?\d+),(?P<y2>-?\d+)\).*?"
    r"uv=\((?P<u0>-?\d+),(?P<v0>-?\d+)\),"
    r"\((?P<u1>-?\d+),(?P<v1>-?\d+)\),"
    r"\((?P<u2>-?\d+),(?P<v2>-?\d+)\).*?"
    r"owner=(?P<owner>.*)$"
)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("log", type=Path)
    parser.add_argument("--tick", type=int, required=True)
    parser.add_argument("--limit", type=int, default=40)
    parser.add_argument(
        "--thin",
        action="store_true",
        help="show only triangles at least 80 px long and at most 20 px wide",
    )
    parser.add_argument(
        "--contains-x",
        type=int,
        help="show only triangles whose screen-space X range contains this value",
    )
    parser.add_argument("--min-span", type=int, default=0)
    args = parser.parse_args()

    records = []
    for line in args.log.read_text(errors="replace").splitlines():
        match = TRIANGLE.search(line)
        if match is None or int(match["tick"]) != args.tick:
            continue
        points = [
            (int(match[f"x{index}"]), int(match[f"y{index}"]))
            for index in range(3)
        ]
        uvs = [
            (int(match[f"u{index}"]), int(match[f"v{index}"]))
            for index in range(3)
        ]
        span_x = max(point[0] for point in points) - min(
            point[0] for point in points
        )
        span_y = max(point[1] for point in points) - min(
            point[1] for point in points
        )
        area2 = abs(
            (points[1][0] - points[0][0])
            * (points[2][1] - points[0][1])
            - (points[1][1] - points[0][1])
            * (points[2][0] - points[0][0])
        )
        if args.thin and not (
            (span_x >= 80 and span_y <= 20)
            or (span_y >= 80 and span_x <= 20)
        ):
            continue
        if args.contains_x is not None and not (
            min(point[0] for point in points)
            <= args.contains_x
            <= max(point[0] for point in points)
        ):
            continue
        if max(span_x, span_y) < args.min_span:
            continue
        records.append(
            (
                max(span_x, span_y),
                span_x + span_y,
                area2,
                match["packet"],
                points,
                uvs,
                match["owner"],
            )
        )
    records.sort(reverse=True)
    for extent, _span_sum, area2, packet, points, uvs, owner in records[
        : args.limit
    ]:
        print(
            f"extent={extent:4d} area2={area2:7d} packet={packet} "
            f"xy={points} uv={uvs} owner={owner}"
        )
    print(f"triangles={len(records)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
