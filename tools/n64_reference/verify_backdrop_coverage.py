#!/usr/bin/env python3
"""Verify that the widescreen panorama reaches both edges without moving joins."""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


LINE = re.compile(
    r"\[BackdropCoverage\] frame=(?P<frame>\d+) "
    r"triangles=(?P<triangles>\d+) packets=(?P<packets>\S+) "
    r"span=(?P<left>-?[\d.]+)\.\.(?P<right>-?[\d.]+) "
    r"target=(?P<target_left>-?[\d.]+)\.\.(?P<target_right>-?[\d.]+) "
    r"moved-left=(?P<moved_left>\d+) "
    r"moved-right=(?P<moved_right>\d+) "
    r"wrong-side=(?P<wrong_side>\d+) covered=(?P<covered>[01])"
)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("log", type=Path)
    parser.add_argument("--minimum-frames", type=int, default=120)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    rows = []
    for line in args.log.read_text(errors="replace").splitlines():
        match = LINE.search(line)
        if match:
            row = match.groupdict()
            rows.append({
                "frame": int(row["frame"]),
                "triangles": int(row["triangles"]),
                "packets": row["packets"],
                "left": float(row["left"]),
                "right": float(row["right"]),
                "target_left": float(row["target_left"]),
                "target_right": float(row["target_right"]),
                "moved_left": int(row["moved_left"]),
                "moved_right": int(row["moved_right"]),
                "wrong_side": int(row["wrong_side"]),
                "covered": row["covered"] == "1",
            })

    uncovered = [row["frame"] for row in rows if not row["covered"]]
    wrong_side = [row["frame"] for row in rows if row["wrong_side"]]
    too_few_triangles = [row["frame"] for row in rows
                         if row["triangles"] < 4]
    passed = (
        len(rows) >= args.minimum_frames and
        not uncovered and
        not wrong_side and
        not too_few_triangles
    )
    report = {
        "passed": passed,
        "frames": len(rows),
        "minimum_frames": args.minimum_frames,
        "uncovered_frames": uncovered,
        "wrong_side_frames": wrong_side,
        "too_few_triangle_frames": too_few_triangles,
        "minimum_left_margin": min(
            (row["target_left"] - row["left"] for row in rows),
            default=None,
        ),
        "minimum_right_margin": min(
            (row["right"] - row["target_right"] for row in rows),
            default=None,
        ),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report, indent=2))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
