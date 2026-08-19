#!/usr/bin/env python3
"""Verify repeatability of two corrected Super Dreamland port runs."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re

from PIL import Image, ImageChops, ImageStat


ROUTE_FIELDS = (
    "terrain-route",
    "route-opaque",
    "route-transparent",
    "route-depth-write",
    "route-depth-test",
    "route-depth-compare-write",
)


def key_values(line: str) -> dict[str, int]:
    return {
        key: int(value)
        for key, value in re.findall(r"([a-z][a-z-]+)=(\d+)", line)
    }


def trace_contract(
    path: Path,
) -> tuple[list[dict[str, int]], list[dict[str, int]], list[int]]:
    route_intervals: list[dict[str, int]] = []
    color_summaries: list[dict[str, int]] = []
    deterministic_rng_seeds: list[int] = []
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if "[EnhancedRenderer]" in line:
            values = key_values(line)
            if values.get("terrain-route", 0) > 0:
                route_intervals.append(
                    {field: values.get(field, 0) for field in ROUTE_FIELDS}
                )
        elif "[V8N64RouteColorSummary]" in line:
            values = key_values(line)
            # Tick zero belongs to asynchronous shell teardown and can include
            # a different number of post-gameplay ordering-table submissions.
            # Positive gameplay ticks are the deterministic route contract.
            if values.get("tick", 0) > 0:
                color_summaries.append(
                    {
                        "tick": values.get("tick", 0),
                        "mapped": values.get("mapped", 0),
                        "rejected": values.get("rejected", 0),
                    }
                )
        elif "[V8DeterministicRng]" in line:
            values = key_values(line)
            deterministic_rng_seeds.append(values.get("effective", -1))
    return route_intervals, color_summaries, deterministic_rng_seeds


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def mean_abs_rgb(left: Image.Image, right: Image.Image) -> float:
    difference = ImageChops.difference(left.convert("RGB"), right.convert("RGB"))
    return sum(ImageStat.Stat(difference).mean) / 3.0


def road_crop(image: Image.Image) -> Image.Image:
    width, height = image.size
    return image.crop(
        (
            int(width * 0.30),
            int(height * 0.62),
            int(width * 0.70),
            int(height * 0.94),
        )
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("primary_stderr", type=Path)
    parser.add_argument("primary_capture_dir", type=Path)
    parser.add_argument("repeat_stderr", type=Path)
    parser.add_argument("repeat_capture_dir", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument(
        "--capture-glob",
        default="recompone_present_gameplay_*.ppm",
    )
    parser.add_argument(
        "--primary-capture-glob",
        help="override --capture-glob for the primary run",
    )
    parser.add_argument(
        "--repeat-capture-glob",
        help="override --capture-glob for the repeat run",
    )
    parser.add_argument("--expected-frames", type=int, default=20)
    parser.add_argument(
        "--max-road-mean-abs-rgb",
        type=float,
        default=0.5,
        help="maximum repeat-run road-crop mean absolute RGB difference",
    )
    args = parser.parse_args()

    failures: list[str] = []
    primary_route, primary_colors, primary_rng = trace_contract(
        args.primary_stderr
    )
    repeat_route, repeat_colors, repeat_rng = trace_contract(args.repeat_stderr)
    if not primary_rng or primary_rng != repeat_rng:
        failures.append("deterministic game RNG seed evidence differs")
    if not primary_route or primary_route != repeat_route:
        failures.append("repeat route render intervals differ")
    if not primary_colors or primary_colors != repeat_colors:
        failures.append("repeat positive-tick route-color summaries differ")
    for index, interval in enumerate(primary_route):
        route = interval["terrain-route"]
        if not (
            interval["route-opaque"] == route
            and interval["route-transparent"] == 0
            and interval["route-depth-write"] == route
            and interval["route-depth-test"] == route
            and interval["route-depth-compare-write"] == route
        ):
            failures.append(f"primary route interval {index} violates depth parity")

    primary_paths = sorted(
        args.primary_capture_dir.glob(
            args.primary_capture_glob or args.capture_glob
        )
    )
    repeat_paths = sorted(
        args.repeat_capture_dir.glob(
            args.repeat_capture_glob or args.capture_glob
        )
    )
    if len(primary_paths) != args.expected_frames:
        failures.append(
            f"primary capture has {len(primary_paths)} frames, "
            f"expected {args.expected_frames}"
        )
    if len(repeat_paths) != args.expected_frames:
        failures.append(
            f"repeat capture has {len(repeat_paths)} frames, "
            f"expected {args.expected_frames}"
        )
    if len(primary_paths) != len(repeat_paths):
        failures.append("repeat capture frame counts differ")

    frame_reports: list[dict[str, object]] = []
    for index, (primary_path, repeat_path) in enumerate(
        zip(primary_paths, repeat_paths)
    ):
        primary = Image.open(primary_path).convert("RGB")
        repeat = Image.open(repeat_path).convert("RGB")
        if primary.size != repeat.size:
            failures.append(
                f"repeat frame {index} dimensions differ: "
                f"{primary.size} != {repeat.size}"
            )
            continue
        whole_difference = mean_abs_rgb(primary, repeat)
        road_difference = mean_abs_rgb(road_crop(primary), road_crop(repeat))
        if road_difference > args.max_road_mean_abs_rgb:
            failures.append(
                f"repeat frame {index} road mean absolute RGB difference "
                f"{road_difference:.3f} exceeds {args.max_road_mean_abs_rgb:.3f}"
            )
        frame_reports.append(
            {
                "frame": index,
                "size": list(primary.size),
                "primary_sha256": sha256(primary_path),
                "repeat_sha256": sha256(repeat_path),
                "exact": sha256(primary_path) == sha256(repeat_path),
                "whole_mean_abs_rgb": round(whole_difference, 6),
                "road_mean_abs_rgb": round(road_difference, 6),
            }
        )

    report = {
        "schema": "v8.dreamland-port-repeatability.v1",
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "route_intervals_identical": primary_route == repeat_route,
        "route_intervals": primary_route,
        "positive_tick_color_summaries_identical": (
            primary_colors == repeat_colors
        ),
        "positive_tick_color_summaries": primary_colors,
        "deterministic_rng_seeds": primary_rng,
        "expected_capture_frames": args.expected_frames,
        "compared_capture_frames": len(frame_reports),
        "exact_capture_frames": sum(bool(item["exact"]) for item in frame_reports),
        "max_whole_mean_abs_rgb": max(
            (float(item["whole_mean_abs_rgb"]) for item in frame_reports),
            default=None,
        ),
        "max_road_mean_abs_rgb": max(
            (float(item["road_mean_abs_rgb"]) for item in frame_reports),
            default=None,
        ),
        "road_mean_abs_rgb_limit": args.max_road_mean_abs_rgb,
        "frames": frame_reports,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
