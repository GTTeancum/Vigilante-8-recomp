#!/usr/bin/env python3
"""Dependency-free pixel audit for widescreen and 4:3-menu return captures."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path


def ppm(source: Path) -> tuple[int, int, memoryview]:
    data = source.read_bytes()
    cursor = 0
    values: list[bytes] = []
    for _ in range(4):
        while cursor < len(data):
            if data[cursor] == 35:
                cursor = data.find(b"\n", cursor)
                if cursor < 0:
                    raise ValueError(f"{source}: unterminated comment")
            elif data[cursor] <= 32:
                cursor += 1
            else:
                break
        end = cursor
        while end < len(data) and data[end] > 32:
            end += 1
        values.append(data[cursor:end])
        cursor = end
    if values[0] != b"P6" or values[3] != b"255":
        raise ValueError(f"{source}: unsupported PPM header {values}")
    width, height = int(values[1]), int(values[2])
    cursor += 2 if data[cursor : cursor + 2] == b"\r\n" else 1
    pixels = memoryview(data)[cursor:]
    if len(pixels) != width * height * 3:
        raise ValueError(
            f"{source}: {len(pixels)} bytes, expected {width * height * 3}"
        )
    return width, height, pixels


def is_dark(pixels: memoryview, offset: int, limit: int = 10) -> bool:
    return (
        pixels[offset] <= limit
        and pixels[offset + 1] <= limit
        and pixels[offset + 2] <= limit
    )


def gameplay_metrics(source: Path) -> dict[str, object]:
    width, height, pixels = ppm(source)
    runs: list[int] = []
    deep_rows = 0
    threshold = max(8, width // 20)
    start_row = height // 20
    end_row = height - start_row
    for y in range(start_row, end_row):
        left = 0
        while left < width and is_dark(pixels, (y * width + left) * 3):
            left += 1
        right = 0
        while right < width and is_dark(
            pixels, (y * width + width - 1 - right) * 3
        ):
            right += 1
        edge_run = max(left, right)
        runs.append(edge_run)
        if edge_run >= threshold:
            deep_rows += 1
    ordered = sorted(runs)
    p95 = ordered[min(len(ordered) - 1, int(len(ordered) * 0.95))]
    return {
        "file": str(source),
        "width": width,
        "height": height,
        "sha256": hashlib.sha256(pixels).hexdigest().upper(),
        "maximum_dark_edge_run_pixels": max(runs),
        "p95_dark_edge_run_pixels": p95,
        "deep_edge_rows": deep_rows,
        "deep_edge_row_fraction": deep_rows / len(runs),
        "passed": (
            p95 < max(8, width // 50)
            and deep_rows / len(runs) < 0.02
        ),
    }


def shell_metrics(source: Path) -> dict[str, object]:
    width, height, pixels = ppm(source)
    dark = 0
    total = width * height
    maximum = 0
    for offset in range(0, len(pixels), 3):
        maximum = max(
            maximum,
            pixels[offset],
            pixels[offset + 1],
            pixels[offset + 2],
        )
        if is_dark(pixels, offset):
            dark += 1
    return {
        "file": str(source),
        "width": width,
        "height": height,
        "sha256": hashlib.sha256(pixels).hexdigest().upper(),
        "dark_fraction": dark / total,
        "maximum_channel": maximum,
    }


COMPOSITION = re.compile(
    r"\[HostComposition\] label=(?P<label>\S+) "
    r"framebuffer=(?P<fw>\d+),(?P<fh>\d+) "
    r"available=(?P<ax>-?\d+),(?P<ay>-?\d+),"
    r"(?P<aw>\d+),(?P<ah>\d+) "
    r"image=(?P<ix>-?\d+),(?P<iy>-?\d+),"
    r"(?P<iw>\d+),(?P<ih>\d+)"
)
COMPOSITION_METRICS = re.compile(
    r"\[HostCompositionMetrics\] label=(?P<label>\S+) "
    r"left-max=(?P<left_max>\d+) right-max=(?P<right_max>\d+) "
    r"left-width=(?P<left_width>\d+) "
    r"right-width=(?P<right_width>\d+) "
    r"image-nonblack=(?P<image_nonblack>\d+)/(?P<image_pixels>\d+)"
)


def region_metrics(
    pixels: memoryview,
    width: int,
    height: int,
    x: int,
    y: int,
    w: int,
    h: int,
) -> dict[str, object]:
    x0, y0 = max(0, x), max(0, y)
    x1, y1 = min(width, x + w), min(height, y + h)
    maximum = 0
    nonblack = 0
    count = max(0, x1 - x0) * max(0, y1 - y0)
    for py in range(y0, y1):
        for px in range(x0, x1):
            offset = (py * width + px) * 3
            value = max(
                pixels[offset],
                pixels[offset + 1],
                pixels[offset + 2],
            )
            maximum = max(maximum, value)
            if value > 2:
                nonblack += 1
    return {
        "x": x0,
        "y": y0,
        "width": max(0, x1 - x0),
        "height": max(0, y1 - y0),
        "maximum_channel": maximum,
        "nonblack_fraction": nonblack / count if count else 0.0,
    }


def shell_menu_metrics(
    source: Path,
    layout: dict[str, int],
) -> dict[str, object]:
    width, height, pixels = ppm(source)
    if width != layout["fw"] or height != layout["fh"]:
        raise ValueError(
            f"{source}: capture {width}x{height}, "
            f"layout {layout['fw']}x{layout['fh']}"
        )
    ax, ay, aw, ah = (
        layout["ax"], layout["ay"], layout["aw"], layout["ah"]
    )
    ix, iy, iw, ih = (
        layout["ix"], layout["iy"], layout["iw"], layout["ih"]
    )
    outer_chrome_inset = 20
    right_x = ix + iw
    image_edge_inset = 2
    left = region_metrics(
        pixels,
        width,
        height,
        ax + outer_chrome_inset,
        ay,
        ix - ax - outer_chrome_inset - image_edge_inset,
        ah,
    )
    right = region_metrics(
        pixels,
        width,
        height,
        right_x + image_edge_inset,
        ay,
        ax + aw - right_x - outer_chrome_inset - image_edge_inset,
        ah,
    )
    image = region_metrics(pixels, width, height, ix, iy, iw, ih)
    bars_wide_enough = (
        int(left["width"]) >= max(32, width // 20)
        and int(right["width"]) >= max(32, width // 20)
    )
    bars_black = (
        int(left["maximum_channel"]) <= 2
        and int(right["maximum_channel"]) <= 2
    )
    return {
        "file": str(source),
        "width": width,
        "height": height,
        "sha256": hashlib.sha256(pixels).hexdigest().upper(),
        "left_bar": left,
        "right_bar": right,
        "image": image,
        "bars_wide_enough": bars_wide_enough,
        "bars_black": bars_black,
        "menu_content_visible":
            float(image["nonblack_fraction"]) >= 0.02,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--mode",
        choices=("gameplay", "shell-transition", "shell-menu"),
        required=True,
    )
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--layout-log", type=Path)
    parser.add_argument("inputs", type=Path, nargs="+")
    args = parser.parse_args()
    inputs = [path.resolve() for path in args.inputs]
    if args.mode == "gameplay":
        captures = [gameplay_metrics(path) for path in inputs]
        checks = {
            "multiple_frames": len(captures) >= 2,
            "all_edges_clean": all(
                bool(capture["passed"]) for capture in captures
            ),
        }
    elif args.mode == "shell-transition":
        captures = [shell_metrics(path) for path in inputs]
        dimensions: dict[tuple[int, int], list[dict[str, object]]] = {}
        for capture in captures:
            key = (int(capture["width"]), int(capture["height"]))
            dimensions.setdefault(key, []).append(capture)
        transition = max(dimensions.values(), key=len)
        first = transition[:3]
        checks = {
            "transition_sequence": len(transition) >= 60,
            "first_frames_cleared": all(
                float(capture["dark_fraction"]) >= 0.999
                for capture in first
            ),
            "no_stale_gameplay_fragments": all(
                float(capture["dark_fraction"]) >= 0.995
                for capture in transition
            ),
        }
    else:
        if args.layout_log is None:
            parser.error("--layout-log is required for shell-menu mode")
        layouts: dict[str, dict[str, int]] = {}
        for match in COMPOSITION.finditer(
            args.layout_log.read_text(
                encoding="utf-8", errors="replace"
            )
        ):
            values = match.groupdict()
            layouts[values["label"]] = {
                key: int(value)
                for key, value in values.items()
                if key != "label"
            }
        captures = []
        for path in inputs:
            match = re.search(
                r"recompone_composed_(.+)_\d+x\d+\.ppm$",
                path.name,
            )
            if match is None or match.group(1) not in layouts:
                raise ValueError(f"{path}: no composition layout")
            captures.append(
                shell_menu_metrics(
                    path,
                    layouts[match.group(1)],
                )
            )
        all_frames = [
            {
                "label": match.group("label"),
                "left_bar_maximum": int(match.group("left_max")),
                "right_bar_maximum": int(match.group("right_max")),
                "left_bar_width": int(match.group("left_width")),
                "right_bar_width": int(match.group("right_width")),
                "image_nonblack_fraction":
                    int(match.group("image_nonblack"))
                    / max(1, int(match.group("image_pixels"))),
            }
            for match in COMPOSITION_METRICS.finditer(
                args.layout_log.read_text(
                    encoding="utf-8", errors="replace"
                )
            )
        ]
        generations: list[list[dict[str, object]]] = []
        for frame in all_frames:
            number_match = re.search(
                r"_(\d+)$", str(frame["label"])
            )
            number = (
                int(number_match.group(1))
                if number_match is not None else 0
            )
            if (
                not generations
                or (
                    generations[-1]
                    and number <= int(
                        re.search(
                            r"_(\d+)$",
                            str(generations[-1][-1]["label"]),
                        ).group(1)
                    )
                )
            ):
                generations.append([])
            generations[-1].append(frame)
        frames = generations[-1] if generations else []
        first_pillarbox = next(
            (
                index
                for index, frame in enumerate(frames)
                if int(frame["left_bar_width"]) >= 32
                and int(frame["right_bar_width"]) >= 32
            ),
            len(frames),
        )
        widescreen_frames = frames[:first_pillarbox]
        returned_frames = frames[first_pillarbox:]
        menu_frames = [
            frame for frame in returned_frames
            if float(frame["image_nonblack_fraction"]) >= 0.02
        ]
        checks = {
            "widescreen_composition_observed_before_return":
                len(widescreen_frames) >= 1,
            "continuous_return_sequence": len(returned_frames) >= 240,
            "four_three_pillarbox_observed": all(
                int(frame["left_bar_width"]) >= 32
                and int(frame["right_bar_width"]) >= 32
                for frame in returned_frames
            ),
            "all_returned_menu_bars_black": all(
                int(frame["left_bar_maximum"]) <= 2
                and int(frame["right_bar_maximum"]) <= 2
                for frame in returned_frames
            ),
            "stable_menu_frames_observed": len(menu_frames) >= 10,
            "final_frame_is_menu":
                bool(returned_frames)
                and float(
                    returned_frames[-1]["image_nonblack_fraction"]
                ) >= 0.02,
            "visual_samples_match_metrics": all(
                bool(capture["bars_black"])
                for capture in captures
            ),
        }
    report = {
        "schema": 2,
        "mode": args.mode,
        "passed": all(checks.values()),
        "checks": checks,
        "captures": captures,
    }
    if args.mode == "shell-menu":
        report["frames"] = frames
        report["widescreen_frame_count"] = len(widescreen_frames)
        report["returned_four_three_frame_count"] = len(returned_frames)
        report["capture_generations"] = [
            len(generation) for generation in generations
        ]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    print(
        f"[CaptureEdges] {'PASS' if report['passed'] else 'FAIL'} "
        f"mode={args.mode} captures={len(captures)}"
    )
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
