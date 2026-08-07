#!/usr/bin/env python3
"""Score object-cull slack settings by near-geometry rate.

Two rules learned the hard way in this investigation are baked in here:

* Soak runs diverge, so a raw primitive count says more about where the car
  drove than about the setting. Everything is reported as a rate against the
  geometry that run actually drew.
* Two game instances sharing the loose root corrupt each other's results, so
  runs are strictly serial.
"""
from __future__ import annotations

import argparse
import collections
import os
import re
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
DEPTHS = re.compile(r"depths\[([^\]]+)\]")
WORLD = re.compile(r"world=(\d+)")
BUCKETS = ["<20", "<40", "<60", "<80", "<100", "<110", "<120", "<130",
           "<150", "<200"]


def run(slack: str, arena: int, frames: int, out: Path) -> None:
    env = dict(os.environ)
    env["RECOMPONE_V82_SOAK_POWERUPS"] = "0"
    env["RECOMPONE_V82_TRACE_TERRAIN_FRAME"] = "1"
    env["RECOMPONE_V82_OBJECT_CULL_SLACK"] = slack
    subprocess.run(
        [sys.executable,
         str(REPO / "tools/recompone-v8-2/run_reference_soak.py"),
         "--loose-root", str(REPO / "V8_2_LOOSE"),
         "--maps", str(arena),
         "--frames", str(frames),
         "--coverage-profile", "weapons",
         "--output", str(out)],
        env=env, cwd=str(REPO),
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)


def tally(out: Path) -> tuple[collections.Counter, int]:
    agg, world = collections.Counter(), 0
    for log in out.glob("*.stderr.log"):
        for line in log.read_text(errors="replace").splitlines():
            found = DEPTHS.search(line)
            if found:
                for part in found.group(1).split():
                    key, value = part.split("=")
                    agg[key] += int(value)
            w = WORLD.search(line)
            if w:
                world += int(w.group(1))
    return agg, world


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--slacks", nargs="+", default=["2.5", "10", "40"])
    ap.add_argument("--arenas", nargs="+", type=int, default=[0, 12, 15, 16])
    ap.add_argument("--frames", type=int, default=600)
    ap.add_argument("--output", type=Path,
                    default=REPO / "artifacts/terrain-fix/slacksweep")
    args = ap.parse_args()

    results = {}
    for slack in args.slacks:
        agg, world = collections.Counter(), 0
        for arena in args.arenas:
            out = args.output / f"s{slack}-m{arena}"
            out.mkdir(parents=True, exist_ok=True)
            run(slack, arena, args.frames, out)
            a, w = tally(out)
            agg += a
            world += w
            print(f"  slack {slack} arena {arena}: world={w}", flush=True)
        results[slack] = (agg, world)

    print(f"\n{'slack':>6}  {'world drawn':>12}  {'<150/M':>8}  {'<100/M':>8}"
          f"  {'<80':>5}  {'<60':>5}")
    for slack, (agg, world) in results.items():
        if not world:
            continue
        n150 = sum(agg[b] for b in BUCKETS[:9])
        n100 = sum(agg[b] for b in BUCKETS[:5])
        print(f"{slack:>6}  {world:>12}  {1e6 * n150 / world:>8.1f}"
              f"  {1e6 * n100 / world:>8.1f}  {agg['<80']:>5}  {agg['<60']:>5}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
