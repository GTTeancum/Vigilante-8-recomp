#!/usr/bin/env python3
"""Convert captured PPM frames to compact PNG files."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("--pattern", default="*.ppm")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    source = args.source.resolve()
    output = (args.output or source / "png").resolve()
    output.mkdir(parents=True, exist_ok=True)
    frames = sorted(source.glob(args.pattern))
    for frame in frames:
        with Image.open(frame) as image:
            image.save(output / f"{frame.stem}.png", optimize=True)
    print(f"converted {len(frames)} frames to {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
