#!/usr/bin/env python3
"""Where does the panoramic backdrop stop, relative to the frame edge?

The backdrop is emitted by func_80050B38 as a small number of large textured
quads at a constant far depth. They are positioned for the authored 320-wide
viewport, so in widescreen the leftmost quad can stop short of the frame edge
and leave a strip with no sky behind the world.

Frame spans sx -54..374. A backdrop that covers the frame reaches sx <= -54.
"""
from __future__ import annotations

import argparse
import statistics
import sys
from pathlib import Path

FRAME_LEFT, FRAME_RIGHT = -54.0, 374.0


def backdrop_edges(path: Path, zlo: float, zhi: float, min_area: float):
    left, right = None, None
    for line in path.read_text(errors="replace").splitlines():
        if line.startswith("#"):
            continue
        f = line.split()
        offset = 1 if f and f[0].startswith("nclip=") else 0
        if len(f) < offset + 8 or int(f[offset + 2]) != 2:
            continue
        pts = []
        for token in f[offset + 5:offset + 8]:
            q = token.split(",")
            try:
                sx, sy, z = float(q[2]), float(q[3]), float(q[4])
            except (IndexError, ValueError):
                pts = []
                break
            if sx != sx or sy != sy or not (zlo <= z <= zhi):
                pts = []
                break
            pts.append((sx, sy))
        if len(pts) != 3:
            continue
        (x0, y0), (x1, y1), (x2, y2) = pts
        area = abs((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)) / 2
        if area < min_area:
            continue
        lo, hi = min(x0, x1, x2), max(x0, x1, x2)
        left = lo if left is None else min(left, lo)
        right = hi if right is None else max(right, hi)
    return left, right


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("dumps", nargs="+", type=Path)
    ap.add_argument("--zlo", type=float, default=3000.0)
    ap.add_argument("--zhi", type=float, default=3600.0)
    ap.add_argument("--min-area", type=float, default=2000.0)
    args = ap.parse_args()

    lefts, rights, short = [], [], 0
    for path in args.dumps:
        left, right = backdrop_edges(path, args.zlo, args.zhi, args.min_area)
        if left is None:
            continue
        lefts.append(left)
        rights.append(right)
        if left > FRAME_LEFT:
            short += 1
    if not lefts:
        print("no backdrop quads matched")
        return 1
    print(f"{len(lefts)} dumps with a backdrop")
    print(f"  left edge  : median {statistics.median(lefts):7.1f} "
          f"(frame edge {FRAME_LEFT})")
    print(f"  right edge : median {statistics.median(rights):7.1f} "
          f"(frame edge {FRAME_RIGHT})")
    print(f"  dumps where the backdrop stops short of the left edge: "
          f"{short}/{len(lefts)}")
    gap = statistics.median(lefts) - FRAME_LEFT
    print(f"  median uncovered strip: {gap:.1f} sx "
          f"({100 * gap / (FRAME_RIGHT - FRAME_LEFT):.1f}% of frame width)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
