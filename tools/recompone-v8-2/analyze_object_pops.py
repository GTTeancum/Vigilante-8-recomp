#!/usr/bin/env python3
"""Locate objects that stop being drawn while still on screen.

Run the game with RECOMPONE_V82_TRACE_OBJECT_CULL=1 and play normally. Each
time the engine draws an object on one tick and rejects it on the next, while
that object is still inside the frame and no further away than it was, the
runtime emits a [V82ObjectPop] line carrying the object address, its world
position, the camera position, and where on screen it was.

Nothing has to be paused or timed: the runtime notices the event itself. This
groups what it found so a spot can be revisited deliberately instead of hunted
for.
"""
from __future__ import annotations

import argparse
import collections
import re
import statistics
import sys
from pathlib import Path

LINE = re.compile(
    r"\[V82ObjectPop\] tick=(\d+) object=0x([0-9A-F]+) "
    r"(?:planes=\([^)]*\) vs=\d+ )?"
    r"world=\((-?\d+),(-?\d+),(-?\d+)\) "
    r"camera=\((-?\d+),(-?\d+),(-?\d+)\) "
    r"distance=(\d+) previous=(\d+) "
    r"screen=\((-?[\d.]+),(-?[\d.]+)\) "
    r"halfWidth=(\d+) halfHeight=(\d+) radius=(\d+)")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("logs", nargs="+", type=Path)
    parser.add_argument(
        "--edge", type=float, default=0.60,
        help="fraction of the half-width beyond which a pop counts as an "
             "edge pop")
    args = parser.parse_args()

    pops = []
    for path in args.logs:
        text = path.read_text(encoding="utf-8", errors="replace")
        for line in text.splitlines():
            match = LINE.search(line)
            if match:
                g = match.groups()
                pops.append({
                    "tick": int(g[0]),
                    "object": g[1],
                    "world": tuple(int(v) for v in g[2:5]),
                    "camera": tuple(int(v) for v in g[5:8]),
                    "distance": int(g[8]),
                    "screen_x": float(g[10]),
                    "screen_y": float(g[11]),
                    "half_width": int(g[12]),
                    "radius": int(g[14]),
                })

    if not pops:
        print("no [V82ObjectPop] lines found. Was "
              "RECOMPONE_V82_TRACE_OBJECT_CULL=1 set for the run?")
        return 2

    fractions = [abs(p["screen_x"]) / p["half_width"] for p in pops]
    edge = [p for p, f in zip(pops, fractions) if f >= args.edge]
    print(f"objects that stopped drawing while still on screen : {len(pops)}")
    print(f"  median position across the frame                 : "
          f"{statistics.median(fractions):.2f} of the half-width")
    print(f"  at the outer edges (beyond {args.edge:.2f})              : "
          f"{len(edge)} ({100 * len(edge) / len(pops):.0f}%)")

    if edge:
        print("\nedge pops, grouped by where in the arena they happened:")
        buckets = collections.Counter()
        example = {}
        for p in edge:
            key = tuple(v // 4096 for v in (p["camera"][0], p["camera"][2]))
            buckets[key] += 1
            example.setdefault(key, p)
        for key, count in buckets.most_common(10):
            p = example[key]
            side = "left" if p["screen_x"] < 0 else "right"
            print(f"  camera near x={key[0] * 4096}, z={key[1] * 4096}: "
                  f"{count} pops")
            print(f"      e.g. tick {p['tick']} object 0x{p['object']} "
                  f"{side} edge, distance {p['distance']}, "
                  f"radius {p['radius']}, "
                  f"world {p['world']}")

    close = [p for p in edge if p["distance"] < 8000]
    print(f"\nedge pops within 8000 units (the reported case)     : "
          f"{len(close)}")
    for p in close[:8]:
        side = "left" if p["screen_x"] < 0 else "right"
        print(f"  tick {p['tick']:5d} object 0x{p['object']} {side:5s} "
              f"distance {p['distance']:6d} radius {p['radius']:5d} "
              f"camera {p['camera']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
