#!/usr/bin/env python3
"""Deterministically sweep the camera through close static scenery.

This is a regression fixture for Enhanced-renderer near clipping.  The stock
soak zig-zags across open terrain; this fixture instead makes one sustained
turn, brakes, then counter-steers so Route 66's roadside buildings pass both
sides of the camera at close range.  Set V82_CLOSE_STEER=LEFT to mirror it.
"""
from __future__ import annotations

import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import run_reference_soak as soak


_original = soak.build_input_script


def build_input_script(slot: int, character_slot: int = 0) -> str:
    script = _original(slot, character_slot)
    first = os.environ.get("V82_CLOSE_STEER", "RIGHT").strip().upper()
    if first not in ("LEFT", "RIGHT"):
        raise SystemExit("V82_CLOSE_STEER must be LEFT or RIGHT")
    second = "LEFT" if first == "RIGHT" else "RIGHT"
    fixture = (
        "[gameplay]\n"
        f"0+150=CROSS\n0+150={first}\n"
        "150+45=SQUARE\n"
        f"195+165=CROSS\n195+165={second}\n"
        "360+45=SQUARE\n"
        f"405+195=CROSS\n405+195={first}\n"
    )
    return script.replace("[gameplay]\n", fixture)


soak.build_input_script = build_input_script


if __name__ == "__main__":
    sys.exit(soak.main())
