#!/usr/bin/env python3
"""Detect shredded terrain from a geometry dump.

The edge-hole metric counts backdrop-coloured pixels at the frame edges and
stayed at zero while the ground was rendering as chaotic overlapping triangles.
This looks at the terrain geometry itself instead of the picture's edges.

Healthy terrain is a walked grid: strips of similar-sized triangles that tile
without heavy overlap. Shredded terrain shows up as a blown-out triangle size
distribution - a few enormous triangles spanning the frame - and as coverage
far exceeding the area actually drawn.
"""
from __future__ import annotations

import argparse
import statistics
import sys
from pathlib import Path

TERRAIN = 10


def stats(path: Path):
    areas = []
    for line in path.read_text(errors="replace").splitlines():
        if line.startswith("#"):
            continue
        f = line.split()
        if len(f) < 8 or int(f[2]) != TERRAIN:
            continue
        pts = []
        for token in f[5:8]:
            q = token.split(",")
            try:
                sx, sy = float(q[2]), float(q[3])
            except (IndexError, ValueError):
                pts = []
                break
            if sx != sx or sy != sy:
                pts = []
                break
            pts.append((sx, sy))
        if len(pts) != 3:
            continue
        (x0, y0), (x1, y1), (x2, y2) = pts
        areas.append(abs((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)) / 2)
    return areas


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("dumps", nargs="+", type=Path)
    args = ap.parse_args()

    med, big, total, counts = [], 0, 0, []
    for path in args.dumps:
        areas = stats(path)
        if len(areas) < 20:
            continue
        areas.sort()
        med.append(statistics.median(areas))
        counts.append(len(areas))
        # A terrain triangle covering more than a tenth of the frame is not a
        # terrain triangle any more.
        big += sum(1 for a in areas if a > 428 * 240 * 0.1)
        total += len(areas)
    if not med:
        print("no terrain geometry found")
        return 1
    print(f"{len(med)} frames, {total} terrain triangles")
    print(f"  median triangle area : {statistics.median(med):8.1f} px^2")
    print(f"  oversized triangles  : {big} "
          f"({100 * big / max(total, 1):.3f}% - healthy is ~0)")
    # A backface sign flip culls alternating triangles: the survivors keep
    # their size and shape, so only the count moves. That is what the area
    # statistics above are blind to, and it is what shredded the ground.
    print(f"  triangles per frame  : median {statistics.median(counts):.0f}"
          f"  min {min(counts)}  max {max(counts)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
