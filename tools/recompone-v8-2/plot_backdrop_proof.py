#!/usr/bin/env python3
"""Distribution of the backdrop's left edge, with and without the fix.

The soak harness diverges between runs, so a paired before/after screenshot
proves nothing (two identical runs differ across most of the frame). What can
be compared is the distribution of a per-frame measurement across many frames.
Each sample is one captured frame; the dashed line is the frame edge, and a
sample to the right of it is a frame where the panorama stopped short and left
a strip of the widescreen view with nothing behind the world.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

sys.path.insert(0, str(Path(__file__).resolve().parent))
from measure_backdrop_edge import backdrop_edges, FRAME_LEFT


def collect(root: Path):
    out = []
    for dump in sorted(root.rglob("*recompone_geometry_frame*.txt")):
        left, _ = backdrop_edges(dump, 3000.0, 3600.0, 2000.0)
        if left is not None:
            out.append(left)
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--control", type=Path, required=True)
    ap.add_argument("--fixed", type=Path, required=True)
    ap.add_argument("--output", type=Path, required=True)
    args = ap.parse_args()

    control, fixed = collect(args.control), collect(args.fixed)
    if not control or not fixed:
        print("not enough samples")
        return 1

    fig, ax = plt.subplots(figsize=(9, 4.2))
    bins = 30
    ax.hist(control, bins=bins, alpha=0.65, label=f"planes built for 4:3 (n={len(control)})")
    ax.hist(fixed, bins=bins, alpha=0.65, label=f"planes widened (n={len(fixed)})")
    ax.axvline(FRAME_LEFT, linestyle="--", linewidth=2, color="k")
    ax.annotate("frame edge\nright of this = strip with no backdrop",
                xy=(FRAME_LEFT, ax.get_ylim()[1] * 0.82),
                xytext=(FRAME_LEFT + 40, ax.get_ylim()[1] * 0.82),
                arrowprops=dict(arrowstyle="->"), fontsize=9)
    ax.set_xlabel("backdrop left edge (screen x; frame spans -54 to 374)")
    ax.set_ylabel("captured frames")
    ax.set_title("Panorama coverage of the widescreen view")
    ax.legend(loc="upper left", fontsize=9)
    fig.tight_layout()
    fig.savefig(args.output, dpi=130)

    def short(v):
        return sum(1 for x in v if x > FRAME_LEFT)
    print(f"control: {short(control)}/{len(control)} frames stop short")
    print(f"fixed  : {short(fixed)}/{len(fixed)} frames stop short")
    print(f"wrote {args.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
