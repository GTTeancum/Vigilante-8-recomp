#!/usr/bin/env python3
"""One quest run, in its own process so a hang can be timed out and killed."""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_reference_soak as soak

soak._original_build_input_script = soak.build_input_script


def build_input_script(slot: int, character_slot: int = 0) -> str:
    script = soak._original_build_input_script(slot, character_slot)
    # Quest sits one entry below Arcade on the mode menu.
    return script.replace("1200+2=SELECT", "1200+3=DOWN", 1)


soak.build_input_script = build_input_script

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--map", type=int, required=True)
    ap.add_argument("--character", type=int, required=True)
    ap.add_argument("--frames", type=int, required=True)
    ap.add_argument("--output", required=True)
    args, _ = ap.parse_known_args()
    sys.argv = [
        "run_reference_soak.py",
        "--loose-root", str(Path(__file__).resolve().parents[2] / "V8_2_LOOSE"),
        "--maps", str(args.map),
        "--characters", str(args.character),
        "--frames", str(args.frames),
        "--coverage-profile", "weapons",
        "--output", args.output,
    ]
    sys.exit(soak.main())
