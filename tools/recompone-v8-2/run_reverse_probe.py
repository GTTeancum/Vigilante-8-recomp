#!/usr/bin/env python3
"""Probe whether the digital Down bit alone makes a vehicle reverse.

TO-DO item 1: "Vehicles will not reverse. Brakes allow stopping, but not
reversing." The reported shape is that the control stick reverses but a face
button or trigger bound to the same native Down command does not.

Scripted input can only set digital direction bits, never the analog bytes, so
a run that holds DOWN reproduces the face-button/trigger case exactly: the bit
is set and the stick stays centred. RECOMPONE_V82_REVERSE_TRACE=1 logs the pad
buffer and camera position every ten polls, which is enough to see whether the
car moves backwards during the hold.

Run with --analog to have the runtime drive the analog axis from the digital
bit as well; comparing the two runs isolates the axis as the cause.
"""
from __future__ import annotations

import argparse
import os
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_reference_soak as soak

# Coast on the gas first so the hold starts from a moving car, then release and
# hold Down long enough to brake to a stop and, if reverse works at all, back
# up a visible distance.
COAST_FRAMES = int(os.environ.get("REVERSE_PROBE_COAST", "240"))
HOLD_FRAMES = 420

_HOLD_BUTTON = os.environ.get("REVERSE_PROBE_BUTTON", "DOWN")

_original = soak.build_input_script


def build_input_script(slot: int, character_slot: int = 0) -> str:
    script = _original(slot, character_slot)
    return (
        f"{script}\n"
        f"60+{COAST_FRAMES}=CROSS\n"
        f"{60 + COAST_FRAMES + 30}+{HOLD_FRAMES}={_HOLD_BUTTON}\n"
    )


soak.build_input_script = build_input_script

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--character", type=int, default=4)
    ap.add_argument("--map", type=int, default=0)
    ap.add_argument("--frames", type=int, default=900)
    ap.add_argument("--analog", action="store_true",
                    help="drive the analog axis from the digital bit too")
    ap.add_argument("--output", required=True)
    ap.add_argument("--burst", type=int, default=0,
                    help="retain this many consecutive gameplay frames")
    args, _ = ap.parse_known_args()

    os.environ["RECOMPONE_V82_REVERSE_TRACE"] = "1"
    # REVERSE_PROBE_BUTTON may name a native command (DOWN) or, with a
    # "PHYS:" prefix, a physical controller input (PHYS:LT) that is routed
    # through profile resolution exactly as a real pad would be.
    os.environ["RECOMPONE_V82_SOAK_NO_AUTOINPUT"] = "1"
    if args.analog:
        os.environ["RECOMPONE_V82_DIGITAL_ANALOG"] = "1"

    sys.argv = [
        "run_reference_soak.py",
        "--loose-root", str(REPO / "V8_2_LOOSE"),
        "--maps", str(args.map),
        "--characters", str(args.character),
        "--frames", str(args.frames),
        "--coverage-profile", "weapons",
        "--output", args.output,
    ]
    if args.burst:
        sys.argv += ["--presentation-burst-frames", str(args.burst)]
    sys.exit(soak.main())
