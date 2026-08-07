#!/usr/bin/env python3
"""Do objects keep their polygons as they approach the camera?

The reported artifact is polygons vanishing from an object when you get close
to it. Tracking each object across a run's dumps and comparing its triangle
count when far against when near measures exactly that, with no dependence on
where the car happened to drive: it is a per-object ratio, not a scene total.

Some loss is legitimate - a wall turned edge-on genuinely shows fewer faces -
so what matters is the change between runs, not the absolute number.
"""
from __future__ import annotations

import argparse
import collections
import glob
import re
import statistics
import sys
from pathlib import Path

WORLD = {1, 2}


def per_object(dump: Path):
    out = collections.defaultdict(lambda: [1e9, 0])
    for line in dump.read_text(errors="replace").splitlines():
        if line.startswith("#"):
            continue
        f = line.split()
        if len(f) < 8 or int(f[2]) not in WORLD:
            continue
        if not f[1].startswith("v82-object="):
            continue
        e = out[f[1]]
        for token in f[5:8]:
            q = token.split(",")
            try:
                z = float(q[4])
            except (IndexError, ValueError):
                continue
            if z > 1:
                e[0] = min(e[0], z)
        e[1] += 1
    return {k: tuple(v) for k, v in out.items() if v[0] < 1e8}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("dumps", nargs="+")
    ap.add_argument("--min-range", type=float, default=2.0,
                    help="an object must be seen at least this many times "
                         "nearer than its farthest sighting to count")
    args = ap.parse_args()

    seen = collections.defaultdict(list)
    files = []
    for pattern in args.dumps:
        files.extend(glob.glob(pattern))
    for path in files:
        for k, (z, c) in per_object(Path(path)).items():
            seen[k].append((z, c))

    ratios, losses = [], 0
    for k, v in seen.items():
        if len(v) < 3:
            continue
        v.sort()
        (near_z, near_c), (far_z, far_c) = v[0], v[-1]
        if far_z < near_z * args.min_range or far_c == 0:
            continue
        ratio = near_c / far_c
        ratios.append(ratio)
        if ratio < 0.7:
            losses += 1
    if not ratios:
        print("no objects observed across a wide enough depth range")
        return 1
    print(f"{len(ratios)} objects tracked from far to near")
    print(f"  median triangles kept when close : "
          f"{100 * statistics.median(ratios):5.1f}%")
    print(f"  mean                             : "
          f"{100 * statistics.mean(ratios):5.1f}%")
    print(f"  objects losing more than 30%     : {losses} "
          f"({100 * losses / len(ratios):.0f}%)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
