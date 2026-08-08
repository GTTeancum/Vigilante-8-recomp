#!/usr/bin/env python3
"""Gate on per-frame terrain volume, to catch terrain that drops out on some
frames and comes back on others.

A flicker is invisible to a still capture and to any counter averaged over a
window: the mean is fine, every individual frame is fine, and the defect lives
entirely in the variation between consecutive frames. That is how a packet
arena collision reached a staged build.

Run the game with RECOMPONE_V82_TRACE_TERRAIN_FRAME=1 and point this at the
stderr log. Each presented gameplay frame emits one [TerrainFrame] line.

The engine presents at 60Hz while submitting geometry at 30Hz, so every other
present is a repeat of the previous image and submits nothing at all. Those
frames carry zero of everything and must not be read as terrain dropouts, so
only frames that actually drew world geometry are considered. Within those, a
frame carrying far less terrain than the surrounding stretch is the defect:
the frame drew the world but left the ground out of it.
"""
from __future__ import annotations

import argparse
import re
import statistics
import sys
from pathlib import Path

LINE = re.compile(
    r"\[TerrainFrame\] frame=(\d+) tick=(-?\d+) terrain=(\d+) world=(\d+)")


def parse(paths: list[Path]) -> list[tuple[int, int, int, int]]:
    frames = []
    for path in paths:
        for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
            match = LINE.search(line)
            if match:
                frames.append(tuple(int(g) for g in match.groups()))
    frames.sort(key=lambda row: row[0])
    return frames


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("logs", nargs="+", type=Path)
    parser.add_argument("--window", type=int, default=15,
                        help="rolling median width, in presented frames")
    parser.add_argument("--drop", type=float, default=0.55,
                        help="fraction of the local median below which a frame "
                             "counts as a dropout")
    parser.add_argument("--skip", type=int, default=30,
                        help="ignore this many leading frames while the arena "
                             "is still filling")
    parser.add_argument("--max-dropouts", type=int, default=0,
                        help="fail if more than this many frames drop out")
    args = parser.parse_args()

    frames = parse(args.logs)
    if not frames:
        print("no [TerrainFrame] lines found; "
              "was RECOMPONE_V82_TRACE_TERRAIN_FRAME=1 set?")
        return 2

    # A present that submitted no world geometry is a repeat of the previous
    # image, not a frame that lost its terrain.
    drawn = [row for row in frames if row[3] > 0]
    repeats = len(frames) - len(drawn)
    counts = [row[2] for row in drawn]
    body = counts[args.skip:]
    kept = drawn[args.skip:]
    if len(body) < args.window * 2:
        print(f"only {len(body)} usable frames; need a longer run")
        return 2

    dropouts = []
    half = args.window // 2
    for i in range(half, len(body) - half):
        window = body[i - half:i + half + 1]
        local = statistics.median(window)
        if local <= 0:
            continue
        if body[i] < local * args.drop:
            dropouts.append((kept[i][0], body[i], local))

    total = len(body)
    print(f"presents seen                      : {len(frames)} "
          f"({repeats} were repeats with no draws)")
    print(f"frames that drew world geometry    : {total}")
    print(f"terrain triangles  min/median/max  : "
          f"{min(body)} / {int(statistics.median(body))} / {max(body)}")
    zero = sum(1 for v in body if v == 0)
    print(f"frames with no terrain at all      : {zero}")
    print(f"dropout frames (< {args.drop:.0%} of local median): {len(dropouts)}")
    for frame, value, local in dropouts[:15]:
        print(f"    frame {frame}: {value} triangles against local median {int(local)}")
    if len(dropouts) > 15:
        print(f"    ... and {len(dropouts) - 15} more")

    failed = len(dropouts) > args.max_dropouts or zero > 0
    print("RESULT:", "FAIL" if failed else "PASS")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
