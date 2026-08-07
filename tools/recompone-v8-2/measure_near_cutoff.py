#!/usr/bin/env python3
"""How close to the camera does submitted world geometry actually get?

The reported artifact is a wall being sliced off where it passes closest to the
camera. That shows up here as a floor on the depth of submitted wall/prop
geometry: nothing nearer than the floor is ever drawn, so the wall ends in
mid-air at whatever screen position that depth falls on.
"""
from __future__ import annotations

import argparse
import statistics
import sys
from pathlib import Path

WORLD = {1, 2}


def near_depths(path: Path):
    out = []
    for line in path.read_text(errors="replace").splitlines():
        if line.startswith("#"):
            continue
        f = line.split()
        if len(f) < 8 or int(f[2]) not in WORLD:
            continue
        for token in f[5:8]:
            q = token.split(",")
            try:
                z = float(q[4])
            except (IndexError, ValueError):
                continue
            if z > 1.0:
                out.append(z)
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("dumps", nargs="+", type=Path)
    args = ap.parse_args()

    mins, all_near = [], 0
    for path in args.dumps:
        depths = near_depths(path)
        if not depths:
            continue
        mins.append(min(depths))
        all_near += sum(1 for z in depths if z < 110)
    if not mins:
        print("no wall/prop geometry found")
        return 1
    mins.sort()
    print(f"{len(mins)} frames with wall/prop geometry")
    print(f"  nearest vertex overall      : {mins[0]:7.1f}")
    print(f"  median frame's nearest vertex: {statistics.median(mins):7.1f}")
    print(f"  frames reaching below 110    : "
          f"{sum(1 for m in mins if m < 110)}/{len(mins)}")
    print(f"  vertices below depth 110     : {all_near}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
