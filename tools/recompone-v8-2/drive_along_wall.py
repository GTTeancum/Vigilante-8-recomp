#!/usr/bin/env python3
"""Autodrive that pins the car against an arena wall.

The stock soak autodrive alternates its turn every 180 frames, which keeps the
car away from sustained wall contact - so it never reproduces the case where a
wall passes close beside the camera, which is exactly the case under
investigation. Holding accelerate and one direction drives a constant circle
until the car reaches the arena perimeter and then slides along it.
"""
from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import run_reference_soak as soak

_original = soak.build_input_script


def build_input_script(slot: int, character_slot: int = 0) -> str:
    script = _original(slot, character_slot)
    # Everything after [gameplay] is stage-relative, so these hold for the
    # whole gameplay portion however long the run is.
    return script.replace(
        "[gameplay]\n",
        "[gameplay]\n0+100000=CROSS\n60+100000=LEFT\n")


soak.build_input_script = build_input_script

if __name__ == "__main__":
    sys.exit(soak.main())
