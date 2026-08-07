#!/usr/bin/env python3
"""Answer one question from an F9 capture: why is the wall cut?

Distinguishes the two cases that need completely different fixes:

  offered and drew nothing - the engine considered the object and its mesh
      produced no packets, so the loss is inside the emitter;
  never offered           - the object never reached the renderer at all, so
      the loss is in whatever builds the frame's object list.

Everything else here exists to locate the wall in the first place: the cut is
where the nearest geometry stops, and the frame spans sx -54..374.
"""
from __future__ import annotations

import argparse
import collections
import sys
from pathlib import Path

WORLD = {1, 2}


def load(path: Path):
    census, drawn = {}, collections.defaultdict(
        lambda: [1e9, -1e9, 1e9, 0])
    for line in path.read_text(errors="replace").splitlines():
        if line.startswith("# object "):
            p = line.split()
            entry = {"outcome": p[3]}
            for token in p:
                if token.startswith("world="):
                    entry["world"] = token.split("=", 1)[1]
            census[int(p[2], 16)] = entry
            continue
        if line.startswith("#"):
            continue
        f = line.split()
        if len(f) < 8 or int(f[2]) not in WORLD:
            continue
        owner = f[1]
        e = drawn[owner]
        for token in f[5:8]:
            q = token.split(",")
            try:
                sx, z = float(q[2]), float(q[4])
            except (IndexError, ValueError):
                continue
            if sx == sx:
                e[0] = min(e[0], sx)
                e[1] = max(e[1], sx)
            if z > 1:
                e[2] = min(e[2], z)
        e[3] += 1
    return census, drawn


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("dump", type=Path)
    args = ap.parse_args()

    census, drawn = load(args.dump)
    print(f"objects the engine considered : {len(census)}")
    if census:
        by = collections.Counter(v["outcome"] for v in census.values())
        for outcome, n in by.most_common():
            print(f"    {outcome:9s} {n}")
    else:
        print("    (none recorded - build predates the always-on census)")

    near = sorted((e[2], k, e) for k, e in drawn.items() if e[3] >= 4)[:8]
    print(f"\nnearest wall/prop geometry drawn (frame spans sx -54..374):")
    for z, owner, e in near:
        print(f"    depth {z:8.1f}  sx {e[0]:8.1f}..{e[1]:7.1f}  "
              f"tris {e[3]:4d}  {owner[:44]}")

    if not near:
        return 0
    floor = near[0][0]
    print(f"\nthe cut sits at the near floor: depth {floor:.1f}")

    # Objects considered but producing nothing are the interesting ones: if the
    # missing wall section is among them the loss is inside the emitter.
    empty = [a for a, v in census.items() if v["outcome"] != "drawn"]
    print(f"objects offered that drew nothing: {len(empty)}")
    for a in empty[:12]:
        print(f"    {a:08X}  world={census[a].get('world', '?')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
