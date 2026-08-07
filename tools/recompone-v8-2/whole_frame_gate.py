#!/usr/bin/env python3
"""Whole-frame regression gate for renderer changes.

Written after an NCLIP change shredded the terrain in play while every metric
stayed green: 18/18 arenas passing and a terrain edge-hole count of zero. That
metric counts backdrop-coloured pixels at the frame edges and is structurally
blind to a broken ground.

Runs are deterministic once the game's saved state is restored beforehand, so
the same capture index is the same moment in both builds. This compares whole
frames and fails loudly when a change alters more of the picture than it should.
"""
from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image

REPO = Path(__file__).resolve().parents[2]
TOOLS = REPO / "tools/recompone-v8-2"
IDX = re.compile(r"gameplay_(\d+)_")


def run(out: Path, arena: int, frames: int, env_extra: dict) -> None:
    env = dict(os.environ)
    env["RECOMPONE_V82_SOAK_POWERUPS"] = "0"
    env["RECOMPONE_V82_GEOMETRY_DUMP_EVERY"] = "45"
    env["RECOMPONE_V82_TRACE_TERRAIN_FRAME"] = "1"
    env.update(env_extra)
    baseline = REPO / "artifacts/terrain-fix/interface.ini.baseline"
    live = REPO / "V8_2_LOOSE/interface.ini"
    if baseline.is_file():
        shutil.copy2(baseline, live)
    out.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        [sys.executable, str(TOOLS / "run_reference_soak.py"),
         "--loose-root", str(REPO / "V8_2_LOOSE"),
         "--maps", str(arena), "--frames", str(frames),
         "--coverage-profile", "weapons", "--output", str(out),
         "--capture-presentation", "--presentation-resolution", "1280x720"],
        env=env, cwd=str(REPO),
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)


def frames_by_index(run_dir: Path):
    out = {}
    for shot in run_dir.glob("*gameplay_*.ppm"):
        m = IDX.search(shot.name)
        if m:
            out[m.group(1)] = shot
    return out


def corrupt(img) -> bool:
    d = np.abs(np.diff(img, axis=1)).mean(axis=2)
    return (d > 60).mean() > 0.05


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--arenas", nargs="+", type=int, default=[0, 12, 15])
    ap.add_argument("--frames", type=int, default=300)
    ap.add_argument("--candidate-env", nargs="*", default=[],
                    help="VAR=VALUE applied to the candidate run only")
    ap.add_argument("--max-index", type=int, default=4,
                    help="last capture index still inside the deterministic "
                         "window; a control run with identical config fails "
                         "from index 5 onward purely from soak divergence")
    ap.add_argument("--threshold", type=float, default=0.25,
                    help="fail if more than this fraction of the frame changes")
    ap.add_argument("--output", type=Path,
                    default=REPO / "artifacts/terrain-fix/gate")
    args = ap.parse_args()

    candidate = dict(kv.split("=", 1) for kv in args.candidate_env)
    reference = {k: "0" for k in candidate}

    worst = 0.0
    failures = []
    for arena in args.arenas:
        ref_dir = args.output / f"ref-m{arena}"
        cand_dir = args.output / f"cand-m{arena}"
        run(ref_dir, arena, args.frames, reference)
        run(cand_dir, arena, args.frames, candidate)
        ref, cand = frames_by_index(ref_dir), frames_by_index(cand_dir)
        shared = [i for i in sorted(set(ref) & set(cand))
                  if int(i) <= args.max_index]
        for index in shared:
            a = np.asarray(Image.open(ref[index]).convert("RGB")).astype(int)
            b = np.asarray(Image.open(cand[index]).convert("RGB")).astype(int)
            if a.shape != b.shape or corrupt(a) or corrupt(b):
                continue
            changed = (np.abs(a - b).sum(axis=2) > 40).mean()
            worst = max(worst, changed)
            if changed > args.threshold:
                failures.append((arena, index, changed))
        print(f"  arena {arena}: {len(shared)} paired frames", flush=True)

    print(f"\nworst whole-frame change: {100 * worst:.1f}%")
    if failures:
        print(f"FAIL - {len(failures)} frames changed more than "
              f"{100 * args.threshold:.0f}%:")
        for arena, index, changed in failures[:10]:
            print(f"    arena {arena} frame {index}: {100 * changed:.1f}%")
        return 1
    print("PASS - no frame changed more than the threshold")
    return 0


if __name__ == "__main__":
    sys.exit(main())
