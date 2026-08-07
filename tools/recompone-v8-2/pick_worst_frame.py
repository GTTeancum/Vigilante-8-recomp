#!/usr/bin/env python3
"""Pull the captured image for the frame with the worst backdrop coverage.

Every geometry dump is paired with a presentation capture; the run log records
which index goes with which frame. This finds the frame the metric rates worst
and returns its picture, so the artifact can be looked at rather than inferred.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from measure_backdrop_edge import backdrop_edges, FRAME_LEFT

PAIR = re.compile(r"V82GeometryDump\] frame=(\d+) index=(\d+)")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("run", type=Path)
    ap.add_argument("--count", type=int, default=3)
    args = ap.parse_args()

    pairing = {}
    for log in args.run.glob("*.stdout.log"):
        for frame, index in PAIR.findall(log.read_text(errors="replace")):
            pairing[int(frame)] = index

    scored = []
    for dump in args.run.glob("recompone_geometry_frame*.txt"):
        digits = "".join(ch for ch in dump.stem if ch.isdigit())
        if not digits:
            continue
        left, _ = backdrop_edges(dump, 3000.0, 3600.0, 2000.0)
        if left is None:
            continue
        scored.append((left, int(digits)))
    if not scored:
        print("no scored frames")
        return 1

    scored.sort(reverse=True)          # largest left edge = worst coverage
    for left, frame in scored[:args.count]:
        index = pairing.get(frame)
        shots = list(args.run.glob(f"*gameplay_{index}_*.ppm")) if index else []
        state = "STOPS SHORT" if left > FRAME_LEFT else "covers frame"
        print(f"frame {frame} index {index}: backdrop left edge {left:7.1f} "
              f"({state})")
        for shot in shots:
            print(f"    {shot}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
