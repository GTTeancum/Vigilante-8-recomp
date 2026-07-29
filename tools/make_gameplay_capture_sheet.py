#!/usr/bin/env python3
"""Convert scripted gameplay PPM captures to PNG and a labeled contact sheet."""

from __future__ import annotations

import argparse
import math
import re
from pathlib import Path

from PIL import Image, ImageDraw


CAPTURE_PATTERN = re.compile(r"\.gameplay_(\d+)\.ppm$", re.IGNORECASE)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("capture_dir", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--columns", type=int, default=4)
    args = parser.parse_args()

    captures = sorted(
        (
            (int(match.group(1)), path)
            for path in args.capture_dir.glob("*.gameplay_*.ppm")
            if (match := CAPTURE_PATTERN.search(path.name))
        ),
        key=lambda item: item[0],
    )
    if not captures:
        raise SystemExit(f"no scripted gameplay captures in {args.capture_dir}")

    frames: list[tuple[int, Image.Image]] = []
    for poll, ppm_path in captures:
        frame = Image.open(ppm_path).convert("RGB")
        frame.save(ppm_path.with_suffix(".png"))
        frames.append((poll, frame))

    columns = max(1, args.columns)
    rows = math.ceil(len(frames) / columns)
    frame_width, frame_height = frames[0][1].size
    label_height = 16
    sheet = Image.new(
        "RGB",
        (columns * frame_width, rows * (frame_height + label_height)),
        "black",
    )
    draw = ImageDraw.Draw(sheet)
    for index, (poll, frame) in enumerate(frames):
        x = (index % columns) * frame_width
        y = (index // columns) * (frame_height + label_height)
        draw.text((x + 3, y + 2), f"{poll:04d}", fill="white")
        sheet.paste(frame, (x, y + label_height))

    output = args.output or args.capture_dir / "gameplay_sequence.png"
    sheet.save(output)
    print(output.resolve())


if __name__ == "__main__":
    main()
