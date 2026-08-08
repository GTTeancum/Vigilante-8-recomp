#!/usr/bin/env python3
"""Locate hard near-black regions in a presentation capture.

This is a diagnostic aid for imported-vehicle material regressions.  It reads
the dependency-free P6 captures produced by the runtime and reports connected
components rather than relying on a hand-picked screenshot coordinate.
"""

from __future__ import annotations

import argparse
import json
from collections import deque
from pathlib import Path

from ppm_to_bmp import token


def read_ppm(path: Path) -> tuple[int, int, memoryview]:
    data = path.read_bytes()
    cursor = 0
    values: list[bytes] = []
    for _ in range(4):
        value, cursor = token(data, cursor)
        values.append(value)
    if values[0] != b"P6" or values[3] != b"255":
        raise ValueError(f"{path}: unsupported PPM header")
    width, height = int(values[1]), int(values[2])
    cursor += 2 if data[cursor : cursor + 2] == b"\r\n" else 1
    pixels = memoryview(data)[cursor:]
    if len(pixels) != width * height * 3:
        raise ValueError(f"{path}: incomplete PPM payload")
    return width, height, pixels


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--x", type=int, default=0)
    parser.add_argument("--y", type=int, default=0)
    parser.add_argument("--width", type=int)
    parser.add_argument("--height", type=int)
    parser.add_argument("--threshold", type=int, default=18)
    parser.add_argument("--minimum-pixels", type=int, default=24)
    args = parser.parse_args()

    source_width, source_height, pixels = read_ppm(args.capture)
    x0 = max(0, args.x)
    y0 = max(0, args.y)
    x1 = min(source_width, x0 + (args.width or source_width - x0))
    y1 = min(source_height, y0 + (args.height or source_height - y0))
    if x0 >= x1 or y0 >= y1:
        raise ValueError("empty analysis rectangle")

    region_width = x1 - x0
    region_height = y1 - y0
    dark = bytearray(region_width * region_height)
    luminance = bytearray(region_width * region_height)
    for local_y in range(region_height):
        for local_x in range(region_width):
            source = (
                ((y0 + local_y) * source_width + x0 + local_x) * 3
            )
            red, green, blue = pixels[source : source + 3]
            value = (54 * red + 183 * green + 19 * blue) >> 8
            index = local_y * region_width + local_x
            luminance[index] = value
            dark[index] = value <= args.threshold

    visited = bytearray(len(dark))
    components: list[dict[str, object]] = []
    for seed in range(len(dark)):
        if not dark[seed] or visited[seed]:
            continue
        queue = deque([seed])
        visited[seed] = 1
        count = 0
        sum_luminance = 0
        min_x = max_x = seed % region_width
        min_y = max_y = seed // region_width
        while queue:
            index = queue.popleft()
            local_x = index % region_width
            local_y = index // region_width
            count += 1
            sum_luminance += luminance[index]
            min_x = min(min_x, local_x)
            max_x = max(max_x, local_x)
            min_y = min(min_y, local_y)
            max_y = max(max_y, local_y)
            for next_x, next_y in (
                (local_x - 1, local_y),
                (local_x + 1, local_y),
                (local_x, local_y - 1),
                (local_x, local_y + 1),
            ):
                if (
                    next_x < 0
                    or next_x >= region_width
                    or next_y < 0
                    or next_y >= region_height
                ):
                    continue
                next_index = next_y * region_width + next_x
                if dark[next_index] and not visited[next_index]:
                    visited[next_index] = 1
                    queue.append(next_index)
        if count < args.minimum_pixels:
            continue
        components.append(
            {
                "pixels": count,
                "bounds": {
                    "x": x0 + min_x,
                    "y": y0 + min_y,
                    "width": max_x - min_x + 1,
                    "height": max_y - min_y + 1,
                },
                "mean_luminance": sum_luminance / count,
                "probe": {
                    "presentation_x": x0 + (min_x + max_x) // 2,
                    "presentation_y": y0 + (min_y + max_y) // 2,
                    "native_x": (
                        (x0 + (min_x + max_x) / 2)
                        * 240
                        / source_height
                    ),
                    "native_y": (
                        (y0 + (min_y + max_y) / 2)
                        * 240
                        / source_height
                    ),
                },
            }
        )

    components.sort(key=lambda item: int(item["pixels"]), reverse=True)
    report = {
        "schema": 1,
        "capture": str(args.capture.resolve()),
        "source_size": [source_width, source_height],
        "analysis_rect": [x0, y0, x1 - x0, y1 - y0],
        "threshold": args.threshold,
        "components": components,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2) + "\n",
        encoding="utf-8",
    )
    print(
        f"[DarkRegions] components={len(components)} "
        f"largest={components[0]['pixels'] if components else 0}"
    )
    for component in components[:12]:
        print(
            f"  pixels={component['pixels']} "
            f"bounds={component['bounds']} "
            f"probe={component['probe']}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
