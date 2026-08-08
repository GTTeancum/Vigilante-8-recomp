#!/usr/bin/env python3
"""Capture the retail PAUSED screen so its widescreen layout can be compared.

TO-DO item 9: in widescreen the pause menu's selection highlight and its
Track / Quit / Resume entries are offset from one another and overlap.

Presses START a little way into gameplay and retains a burst of presentation
frames from that point, so the paused screen can be captured at any aspect
ratio and the two compared element by element.
"""
from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_reference_soak as soak

PAUSE_AT = int(os.environ.get("PAUSE_PROBE_FRAME", "150"))

_original = soak.build_input_script


def build_input_script(slot: int, character_slot: int = 0) -> str:
    script = _original(slot, character_slot)
    # A short tap; holding START would toggle the menu open and shut.
    return f"{script}\n{PAUSE_AT}+3=START\n"


soak.build_input_script = build_input_script

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--character", type=int, default=4)
    ap.add_argument("--map", type=int, default=0)
    ap.add_argument("--frames", type=int, default=600)
    ap.add_argument("--burst", type=int, default=400)
    ap.add_argument("--output", required=True)
    args, _ = ap.parse_known_args()

    os.environ["RECOMPONE_V82_SOAK_NO_AUTOINPUT"] = "1"

    sys.argv = [
        "run_reference_soak.py",
        "--loose-root", str(REPO / "V8_2_LOOSE"),
        "--maps", str(args.map),
        "--characters", str(args.character),
        "--frames", str(args.frames),
        "--coverage-profile", "weapons",
        "--presentation-burst-frames", str(args.burst),
        "--output", args.output,
    ]
    sys.exit(soak.main())
