#!/usr/bin/env python3
"""Find cracks between adjacent polygons in a one-frame geometry dump.

A crack is a thin run of uncovered pixels with drawn geometry on both sides.
It is what the eye sees as a bright line through a wall, and unlike a missing
panel it cannot be found by counting triangles: every triangle is present, the
edges simply do not meet.

The dump records both the packed screen position the engine produced and the
position the shader reconstructs from view space. Rasterising the
reconstructed positions reproduces exactly what reaches the screen.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

import numpy as np

# HleMaterialKind. Ui and ScreenEffect are overlays, not world geometry.
OVERLAY = {8, 9}


def load(path: Path):
    tris = []
    for line in path.read_text().splitlines():
        f = line.split()
        if not f or f[0].startswith("#"):
            continue
        offset = 1 if f[0].startswith("nclip=") else 0
        if len(f) < offset + 8:
            continue
        material = int(f[offset + 2])
        verts = []
        for token in f[offset + 5:offset + 8]:
            p = token.split(",")
            if len(p) < 8:
                return None
            verts.append({
                "packed": (float(p[0]), float(p[1])),
                "screen": (float(p[2]), float(p[3])),
                "z": float(p[4]),
                "centre": float(p[5]),
                "scale": float(p[6]),
                "provenance": float(p[7]),
            })
        tris.append((f[offset], material, verts))
    return tris


def rasterise(tris, width, height, scale):
    w, h = int(width * scale), int(height * scale)
    cover = np.zeros((h, w), dtype=bool)
    yy, xx = np.mgrid[0:h, 0:w]
    for _, material, verts in tris:
        if material in OVERLAY:
            continue
        pts = [(v["screen"][0] * scale, v["screen"][1] * scale)
               for v in verts]
        if any(not np.isfinite(c) for p in pts for c in p):
            continue
        (x0, y0), (x1, y1), (x2, y2) = pts
        lo_x, hi_x = int(max(0, min(x0, x1, x2))), int(min(w, max(x0, x1, x2)) + 1)
        lo_y, hi_y = int(max(0, min(y0, y1, y2))), int(min(h, max(y0, y1, y2)) + 1)
        if lo_x >= hi_x or lo_y >= hi_y:
            continue
        sx, sy = xx[lo_y:hi_y, lo_x:hi_x] + 0.5, yy[lo_y:hi_y, lo_x:hi_x] + 0.5
        d0 = (x1 - x0) * (sy - y0) - (y1 - y0) * (sx - x0)
        d1 = (x2 - x1) * (sy - y1) - (y2 - y1) * (sx - x1)
        d2 = (x0 - x2) * (sy - y2) - (y0 - y2) * (sx - x2)
        inside = ((d0 >= 0) & (d1 >= 0) & (d2 >= 0)) | \
                 ((d0 <= 0) & (d1 <= 0) & (d2 <= 0))
        cover[lo_y:hi_y, lo_x:hi_x] |= inside
    return cover


def cracks(cover, max_width):
    """Uncovered vertical runs no wider than max_width with cover on both sides."""
    h, w = cover.shape
    found = np.zeros(w, dtype=int)
    for y in range(h):
        row = cover[y]
        x = 1
        while x < w - 1:
            if row[x]:
                x += 1
                continue
            start = x
            while x < w and not row[x]:
                x += 1
            run = x - start
            if run <= max_width and start > 0 and x < w and \
                    row[start - 1] and row[x]:
                found[start:x] += 1
    return found


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("dumps", nargs="+", type=Path)
    ap.add_argument("--width", type=float, default=428.0)
    ap.add_argument("--height", type=float, default=240.0)
    ap.add_argument("--scale", type=float, default=4.0,
                    help="supersampling; the artifact is sub-pixel at 1x")
    ap.add_argument("--max-width", type=int, default=6,
                    help="widest gap still counted as a crack, in samples")
    args = ap.parse_args()

    for path in args.dumps:
        tris = load(path)
        if tris is None:
            print(f"{path.name}: dump predates the projection-term fields")
            continue
        cover = rasterise(tris, args.width, args.height, args.scale)
        found = cracks(cover, args.max_width)
        total = int(found.sum())
        print(f"{path.name}: {len(tris)} triangles, "
              f"{cover.sum()} covered samples, crack samples {total}")
        if total == 0:
            continue
        order = np.argsort(-found)[:6]
        for col in sorted(order):
            if found[col] == 0:
                continue
            print(f"    column x={col / args.scale:7.2f} "
                  f"(target space) crack in {found[col]} rows "
                  f"of {int(args.height * args.scale)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
