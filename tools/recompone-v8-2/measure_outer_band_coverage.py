#!/usr/bin/env python3
"""Measure how well world geometry covers the widescreen-only bands.

The authored viewport is sx 0..320; widescreen renders sx -54..374. Geometry
that passes close beside the camera projects far off-axis, and that is exactly
what fills the outer bands. If the engine drops those primitives, the bands
stay empty and the scene appears to be sliced off short of the frame edge.

Reported as coverage relative to the adjacent authored-view band, so it is a
ratio rather than a count and does not depend on where the car drove.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

import numpy as np

MARGIN, AUTHORED = 54.0, 320.0
OVERLAY = {8, 9}
# Walls and props. Terrain (10) fills the bands regardless and masks the
# defect, so it is excluded by default.
WORLD = {1, 2}


def coverage(path: Path, height: int, samples: int, materials):
    width = AUTHORED + 2 * MARGIN
    grid = np.zeros((height, int(width) * samples), bool)
    yy, xx = np.mgrid[0:height, 0:grid.shape[1]]
    for line in path.read_text(errors="replace").splitlines():
        if line.startswith("#"):
            continue
        f = line.split()
        offset = 1 if f and f[0].startswith("nclip=") else 0
        if len(f) < offset + 8 or int(f[offset + 2]) in OVERLAY:
            continue
        if materials and int(f[offset + 2]) not in materials:
            continue
        pts = []
        for token in f[offset + 5:offset + 8]:
            q = token.split(",")
            try:
                sx, sy = float(q[2]), float(q[3])
            except (IndexError, ValueError):
                pts = []
                break
            if sx != sx or sy != sy:
                pts = []
                break
            pts.append(((sx + MARGIN) * samples, sy * height / 240.0))
        if len(pts) != 3:
            continue
        (x0, y0), (x1, y1), (x2, y2) = pts
        lox, hix = int(max(0, min(x0, x1, x2))), int(min(grid.shape[1], max(x0, x1, x2)) + 1)
        loy, hiy = int(max(0, min(y0, y1, y2))), int(min(height, max(y0, y1, y2)) + 1)
        if lox >= hix or loy >= hiy:
            continue
        sx_, sy_ = xx[loy:hiy, lox:hix] + .5, yy[loy:hiy, lox:hix] + .5
        d0 = (x1 - x0) * (sy_ - y0) - (y1 - y0) * (sx_ - x0)
        d1 = (x2 - x1) * (sy_ - y1) - (y2 - y1) * (sx_ - x1)
        d2 = (x0 - x2) * (sy_ - y2) - (y0 - y2) * (sx_ - x2)
        grid[loy:hiy, lox:hix] |= (((d0 >= 0) & (d1 >= 0) & (d2 >= 0)) |
                                   ((d0 <= 0) & (d1 <= 0) & (d2 <= 0)))
    return grid


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("dumps", nargs="+", type=Path)
    ap.add_argument("--height", type=int, default=240)
    ap.add_argument("--samples", type=int, default=2)
    ap.add_argument("--all-materials", action="store_true")
    args = ap.parse_args()

    band = int(MARGIN) * args.samples
    outer_l = outer_r = inner_l = inner_r = 0
    used = 0
    for path in args.dumps:
        grid = coverage(path, args.height, args.samples,
                        None if args.all_materials else WORLD)
        if not grid.any():
            continue
        used += 1
        outer_l += grid[:, :band].mean()
        inner_l += grid[:, band:band * 2].mean()
        outer_r += grid[:, -band:].mean()
        inner_r += grid[:, -band * 2:-band].mean()
    if used == 0:
        print("no usable dumps")
        return 1
    print(f"{used} dumps")
    print(f"  left  outer band {100 * outer_l / used:5.1f}%  "
          f"adjacent authored band {100 * inner_l / used:5.1f}%  "
          f"ratio {outer_l / max(inner_l, 1e-9):.3f}")
    print(f"  right outer band {100 * outer_r / used:5.1f}%  "
          f"adjacent authored band {100 * inner_r / used:5.1f}%  "
          f"ratio {outer_r / max(inner_r, 1e-9):.3f}")
    print("  ratio 1.0 = outer band as well covered as the authored view")
    return 0


if __name__ == "__main__":
    sys.exit(main())
