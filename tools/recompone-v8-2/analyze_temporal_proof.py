#!/usr/bin/env python3
"""Headless temporal acceptance checks for Enhanced-renderer proof frames.

The host writes lossless P6 frames before encoding.  This checker samples a
regular grid from those frames and rejects blank/stalled sequences plus the
one-frame A/B/A reversions characteristic of geometry popping or flicker.
Scene cuts are intentionally not treated as flicker because the third frame
after a cut does not return to the pre-cut image.
"""

from __future__ import annotations

import argparse
import glob
import hashlib
import json
import math
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input_glob")
    parser.add_argument("--kind", choices=("media", "gameplay"), required=True)
    parser.add_argument("--minimum-frames", type=int, default=120)
    parser.add_argument("--output", type=Path, required=True)
    return parser.parse_args()


def read_ppm(path: Path) -> tuple[int, int, bytes]:
    with path.open("rb") as stream:
        if stream.readline().strip() != b"P6":
            raise ValueError(f"{path}: expected P6")
        dimensions = stream.readline()
        while dimensions.startswith(b"#"):
            dimensions = stream.readline()
        width, height = map(int, dimensions.split())
        if stream.readline().strip() != b"255":
            raise ValueError(f"{path}: expected 8-bit RGB")
        pixels = stream.read()
    if len(pixels) != width * height * 3:
        raise ValueError(f"{path}: truncated P6 payload")
    return width, height, pixels


def thumbnail(
    pixels: bytes, width: int, height: int, columns: int = 32, rows: int = 18
) -> tuple[int, ...]:
    values: list[int] = []
    for row in range(rows):
        y = min(height - 1, round((row + 0.5) * height / rows))
        for column in range(columns):
            x = min(width - 1, round((column + 0.5) * width / columns))
            offset = (y * width + x) * 3
            values.extend(pixels[offset : offset + 3])
    return tuple(values)


def mean_absolute_difference(
    left: tuple[int, ...], right: tuple[int, ...]
) -> float:
    return sum(abs(a - b) for a, b in zip(left, right)) / len(left)


def percentile(values: list[float], fraction: float) -> float:
    if not values:
        return 0.0
    ordered = sorted(values)
    index = min(len(ordered) - 1, math.floor(fraction * len(ordered)))
    return ordered[index]


def main() -> int:
    args = parse_args()
    paths = [Path(path) for path in sorted(glob.glob(args.input_glob))]
    failures: list[str] = []
    if len(paths) < args.minimum_frames:
        failures.append(
            f"only {len(paths)} frames; expected at least {args.minimum_frames}"
        )

    dimensions: tuple[int, int] | None = None
    thumbnails: list[tuple[int, ...]] = []
    hashes: list[str] = []
    blank_frames = 0
    for path in paths:
        width, height, pixels = read_ppm(path)
        if dimensions is None:
            dimensions = (width, height)
        elif dimensions != (width, height):
            failures.append(
                f"frame size changed at {path.name}: "
                f"{width}x{height} != {dimensions[0]}x{dimensions[1]}"
            )
        thumb = thumbnail(pixels, width, height)
        thumbnails.append(thumb)
        hashes.append(hashlib.sha256(pixels).hexdigest())
        if max(thumb) < 8:
            blank_frames += 1

    changes = [
        mean_absolute_difference(thumbnails[index - 1], thumbnails[index])
        for index in range(1, len(thumbnails))
    ]
    # A/B/A is a stronger popping signal than a large A/B transition: the
    # latter is ordinary camera motion or an authored movie cut.
    reversions: list[dict[str, float | int]] = []
    for index in range(2, len(thumbnails)):
        previous = changes[index - 2]
        current = changes[index - 1]
        two_back = mean_absolute_difference(
            thumbnails[index - 2], thumbnails[index]
        )
        if previous >= 7.5 and current >= 7.5 and two_back <= 1.5:
            reversions.append(
                {
                    "frame": index,
                    "into_spike": round(previous, 4),
                    "out_of_spike": round(current, 4),
                    "two_frame_difference": round(two_back, 4),
                }
            )

    unique_frames = len(set(hashes))
    unique_fraction = unique_frames / len(paths) if paths else 0.0
    blank_fraction = blank_frames / len(paths) if paths else 1.0
    if args.kind == "media":
        if unique_frames < 24 or unique_fraction < 0.12:
            failures.append(
                f"media stalled ({unique_frames}/{len(paths)} unique frames)"
            )
        if percentile(changes, 0.90) < 0.75:
            failures.append("media sequence lacks visible motion")
        # Short black cuts between authored logo streams are valid.
        if blank_fraction > 0.45:
            failures.append(
                f"media mostly blank ({blank_fraction:.3f} blank frames)"
            )
    else:
        if unique_frames < max(24, len(paths) // 5):
            failures.append(
                f"gameplay stalled ({unique_frames}/{len(paths)} unique frames)"
            )
        if blank_fraction > 0.05:
            failures.append(
                f"gameplay contains {blank_fraction:.3f} blank frames"
            )
        if reversions:
            failures.append(
                f"{len(reversions)} one-frame A/B/A flicker reversions"
            )

    report = {
        "kind": args.kind,
        "input_glob": args.input_glob,
        "frames": len(paths),
        "dimensions": list(dimensions) if dimensions else None,
        "unique_frames": unique_frames,
        "unique_fraction": round(unique_fraction, 6),
        "blank_frames": blank_frames,
        "blank_fraction": round(blank_fraction, 6),
        "median_frame_change": round(percentile(changes, 0.50), 6),
        "p90_frame_change": round(percentile(changes, 0.90), 6),
        "p99_frame_change": round(percentile(changes, 0.99), 6),
        "one_frame_reversions": reversions,
        "passed": not failures,
        "failures": failures,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
