#!/usr/bin/env python3
"""End-to-end validation for the widescreen object/scenery frustum fix.

Runs every arena, then checks the three things that must hold together:

* every arena still reaches and completes gameplay;
* the terrain edge-hole fix has not regressed (must stay at zero);
* the artifact metrics improve - the panorama stops short of the frame edge in
  fewer frames, and the left widescreen band is better covered.

Runs are strictly serial: two game instances sharing the loose root corrupt
each other's results.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
TOOLS = REPO / "tools/recompone-v8-2"
PASS = re.compile(r"^\[soak\] (PASS|FAIL)", re.M)


def soak(out: Path, arenas, frames: int, env_extra: dict) -> tuple[int, int]:
    env = dict(os.environ)
    env["RECOMPONE_V82_SOAK_POWERUPS"] = "0"
    env["RECOMPONE_V82_GEOMETRY_DUMP_EVERY"] = "60"
    env["RECOMPONE_V82_TRACE_TERRAIN_FRAME"] = "1"
    env["RECOMPONE_V82_CENSUS_OWNERSHIP"] = "1"
    env.update(env_extra)
    passes = fails = 0
    for arena in arenas:
        target = out / f"m{arena}"
        target.mkdir(parents=True, exist_ok=True)
        snapshot = REPO / "artifacts/terrain-fix/interface.ini.baseline"
        live = REPO / "V8_2_LOOSE/interface.ini"
        if snapshot.is_file():
            shutil.copy2(snapshot, live)
        proc = subprocess.run(
            [sys.executable, str(TOOLS / "run_reference_soak.py"),
             "--loose-root", str(REPO / "V8_2_LOOSE"),
             "--maps", str(arena), "--frames", str(frames),
             "--coverage-profile", "weapons", "--output", str(target),
             "--capture-presentation",
             "--presentation-resolution", "1280x720"],
            env=env, cwd=str(REPO), capture_output=True, text=True)
        for kind in PASS.findall(proc.stdout or ""):
            if kind == "PASS":
                passes += 1
            else:
                fails += 1
    return passes, fails


def edge_holes(out: Path) -> int:
    total = 0
    for shot in out.rglob("*.gameplay.ppm"):
        proc = subprocess.run(
            [sys.executable, str(TOOLS / "analyze_terrain_edge_holes.py"),
             str(shot)], capture_output=True, text=True)
        try:
            data = json.loads(proc.stdout)
        except json.JSONDecodeError:
            continue
        # The analyzer reports backdrop-coloured pixels as matching_pixels,
        # plus per-edge row counts. Sum all three so a regression on either
        # edge is caught.
        total += int(data.get("matching_pixels", 0) or 0)
        for edge in ("left_edge", "right_edge"):
            total += int((data.get(edge) or {}).get("row_count", 0) or 0)
    return total


def metric(script: str, out: Path) -> str:
    dumps = sorted(str(p) for p in out.rglob("recompone_geometry_frame*.txt"))
    if not dumps:
        return "    (no dumps)"
    proc = subprocess.run([sys.executable, str(TOOLS / script), *dumps],
                          capture_output=True, text=True)
    return "\n".join("    " + line for line in proc.stdout.strip().splitlines())


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--arenas", nargs="+", type=int,
                    default=list(range(18)))
    ap.add_argument("--frames", type=int, default=700)
    ap.add_argument("--output", type=Path,
                    default=REPO / "artifacts/terrain-fix/validate")
    args = ap.parse_args()

    # Restoring the game's saved state before each run keeps runs comparable;
    # without it the carryover alone diverges them (T62).
    for label, extra in (("both fixes (shipping)", {}),
                         ("neither fix (control)",
                          {"RECOMPONE_V82_OBJECT_FRUSTUM_SCALE": "0",
                           "RECOMPONE_V82_BACKDROP_FILL": "0",
                           "RECOMPONE_V82_NEAR_FLAGS": "0"})):
        out = args.output / label.split()[1]
        print(f"\n=== {label}", flush=True)
        passes, fails = soak(out, args.arenas, args.frames, extra)
        print(f"  arenas: {passes} pass, {fails} fail", flush=True)
        print(f"  terrain edge-hole pixels: {edge_holes(out)}", flush=True)
        print("  backdrop edge:")
        print(metric("measure_backdrop_edge.py", out), flush=True)
        print("  widescreen band coverage:")
        print(metric("measure_outer_band_coverage.py", out), flush=True)
        print("  near cutoff (the reported artifact):")
        print(metric("measure_near_cutoff.py", out), flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
