#!/usr/bin/env python3
"""Detect wall panels the engine never offered to the renderer.

Arena walls are arrays of objects at a fixed address stride that tile across
the screen. A missing section shows up as an address inside such a run that
produced no geometry AND never appears in the object census - meaning it was
never submitted, as opposed to submitted and culled.

Constraining the search to runs that actually tile on screen matters: raw
address gaps also catch unused pool slots, which are not walls at all.
"""
from __future__ import annotations

import argparse
import collections
import sys
from pathlib import Path


def load(path: Path):
    census, drawn = {}, collections.defaultdict(
        lambda: [1e9, -1e9, 1e9, -1e9, 1e9, 0])
    for line in path.read_text(errors="replace").splitlines():
        if line.startswith("# object "):
            parts = line.split()
            frame = 0
            for token in parts:
                if token.startswith("lastFrame="):
                    frame = int(token.split("=")[1])
            census[int(parts[2], 16)] = (parts[3], frame)
            continue
        if line.startswith("#"):
            continue
        f = line.split()
        if len(f) < 8 or not f[1].startswith("v82-object=0x"):
            continue
        address = int(f[1].split("0x")[1], 16)
        entry = drawn[address]
        for token in f[5:8]:
            p = token.split(",")
            try:
                sx, sy, z = float(p[2]), float(p[3]), float(p[4])
            except (IndexError, ValueError):
                continue
            if sx == sx:
                entry[0] = min(entry[0], sx)
                entry[1] = max(entry[1], sx)
            if sy == sy:
                entry[2] = min(entry[2], sy)
                entry[3] = max(entry[3], sy)
            if z > 0:
                entry[4] = min(entry[4], z)
        entry[5] += 1
    return census, drawn


def runs(drawn, stride):
    """Consecutive stride-spaced addresses that also tile horizontally."""
    out, current = [], []
    for address in sorted(drawn):
        if current and address - current[-1] == stride:
            current.append(address)
        else:
            if len(current) >= 3:
                out.append(current)
            current = [address]
    if len(current) >= 3:
        out.append(current)
    tiling = []
    for run in out:
        xs = [drawn[a][0] for a in run]
        # a wall marches across the screen rather than sitting in one place
        if max(xs) - min(xs) > 40:
            tiling.append(run)
    return tiling


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("dumps", nargs="+", type=Path)
    ap.add_argument("--stride", type=lambda v: int(v, 0), default=0xD8)
    args = ap.parse_args()

    hits = 0
    for path in args.dumps:
        census, drawn = load(path)
        if not drawn:
            continue
        digits = "".join(ch for ch in path.stem if ch.isdigit())
        dump_frame = int(digits) if digits else 0
        for run in runs(drawn, args.stride):
            lo, hi = run[0], run[-1]
            # look one stride beyond each end: a truncated wall stops early
            for probe, side in ((lo - args.stride, "before"),
                                (hi + args.stride, "after")):
                if probe in drawn:
                    continue
                entry = census.get(probe)
                if entry is None:
                    # Never offered at all: this is the end of the wall array,
                    # not a panel being dropped.
                    continue
                _, last = entry
                age = dump_frame - last
                # Offered recently but not now is the defect: the panel was
                # being submitted and then stopped as the camera closed in.
                if age <= 2 or age > 600:
                    continue
                # Extrapolate where the absent panel would have landed. A
                # panel that left the view is expected to stop being offered;
                # one whose place is still inside the frame is the defect.
                end = drawn[lo if side == "before" else hi]
                nxt = drawn[run[1] if side == "before" else run[-2]]
                step = (end[0] - nxt[0])
                expect0, expect1 = end[0] + step, end[1] + step
                if max(expect0, expect1) < -54 or min(expect0, expect1) > 374:
                    continue
                near = end
                hits += 1
                print(f"{path.name}: wall run {lo:08X}..{hi:08X} "
                      f"({len(run)} panels) stops {side} {probe:08X}, "
                      f"last offered {age} frames ago")
                print(f"    last drawn panel x {near[0]:.0f}..{near[1]:.0f} "
                      f"depth {near[4]:.0f}; missing panel would sit at "
                      f"x {min(expect0, expect1):.0f}..{max(expect0, expect1):.0f}")
    if hits == 0:
        print("no wall panels found that stopped being offered")
    return 0


if __name__ == "__main__":
    sys.exit(main())
