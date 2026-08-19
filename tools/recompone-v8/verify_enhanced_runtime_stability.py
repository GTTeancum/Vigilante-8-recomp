#!/usr/bin/env python3
"""Verify enhanced-renderer source stability and a captured gameplay burst."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

from PIL import Image, ImageDraw, ImageStat


PERFORMANCE_RE = re.compile(
    r"\[EnhancedPerformance\].*?"
    r"present-reallocations=(?P<reallocations>\d+).*?"
    r"vram-fallback-presents=(?P<fallbacks>\d+)"
)
SOURCE_RE = re.compile(
    r"\[EnhancedPresentFrame\].*?source=(?P<source>\S+)"
)
ROUTE_RENDER_RE = re.compile(
    r"\[EnhancedRenderer\].*?"
    r"terrain-route=(?P<triangles>\d+)"
)
ROUTE_PACKET_RE = re.compile(
    r"\[EnhancedPacketOwner\].*?"
    r"words=(?P<words>\d+).*?"
    r"terrain-route=1"
)


def make_contact_sheet(
    frames: list[tuple[Path, Image.Image]],
    output: Path,
) -> None:
    thumb_w, thumb_h = 456, 257
    columns = 5
    rows = (len(frames) + columns - 1) // columns
    label_h = 18
    sheet = Image.new(
        "RGB",
        (columns * thumb_w, rows * (thumb_h + label_h)),
        "black",
    )
    draw = ImageDraw.Draw(sheet)
    for index, (path, image) in enumerate(frames):
        x = index % columns * thumb_w
        y = index // columns * (thumb_h + label_h)
        thumb = image.copy()
        thumb.thumbnail((thumb_w, thumb_h))
        sheet.paste(thumb, (x, y + label_h))
        match = re.search(r"gameplay_(\d+)_", path.name)
        label = match.group(1) if match else f"{index:03d}"
        draw.text((x + 3, y + 2), label, fill="white")
    output.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(output)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("stderr_log", type=Path)
    parser.add_argument("capture_dir", type=Path)
    parser.add_argument(
        "--capture-glob",
        default="recompone_present_gameplay_*.ppm",
    )
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--contact-sheet", type=Path)
    parser.add_argument(
        "--expected-frames",
        type=int,
        default=20,
        help="required presentation-burst frame count (default: 20)",
    )
    args = parser.parse_args()

    log_lines = args.stderr_log.read_text(
        encoding="utf-8",
        errors="replace",
    ).splitlines()
    gameplay_seen = False
    performance: list[dict[str, int]] = []
    gameplay_sources: list[str] = []
    route_render_intervals: list[int] = []
    route_packet_words: list[int] = []
    for line in log_lines:
        if "stage 'gameplay'" in line:
            gameplay_seen = True
        # The scripted pause/quit transition intentionally presents one native
        # VRAM frame while the shell takes ownership back. It is not gameplay
        # renderer fallback and must not contaminate the steady-play contract.
        if "stage 'soak_teardown'" in line:
            gameplay_seen = False
        if not gameplay_seen:
            continue
        if match := PERFORMANCE_RE.search(line):
            performance.append(
                {
                    "present_reallocations": int(
                        match.group("reallocations")
                    ),
                    "vram_fallback_presents": int(
                        match.group("fallbacks")
                    ),
                }
            )
        if match := SOURCE_RE.search(line):
            gameplay_sources.append(match.group("source"))
        if match := ROUTE_RENDER_RE.search(line):
            route_render_intervals.append(int(match.group("triangles")))
        if match := ROUTE_PACKET_RE.search(line):
            route_packet_words.append(int(match.group("words")))

    capture_paths = sorted(args.capture_dir.glob(args.capture_glob))
    frames: list[tuple[Path, Image.Image]] = [
        (path, Image.open(path).convert("RGB"))
        for path in capture_paths
    ]
    sizes = sorted({image.size for _, image in frames})

    # The lower-centre crop includes the road directly beneath and behind the
    # vehicle. A disappearing render source produces a large whole-region
    # luminance discontinuity; ordinary camera settling only changes it
    # incrementally.
    road_means: list[float] = []
    for _, image in frames:
        width, height = image.size
        crop = image.crop(
            (
                int(width * 0.30),
                int(height * 0.62),
                int(width * 0.70),
                int(height * 0.94),
            )
        ).convert("L")
        road_means.append(ImageStat.Stat(crop).mean[0])
    adjacent_luma_jumps = [
        abs(current - previous)
        for previous, current in zip(road_means, road_means[1:])
    ]
    # A moving camera, an explosion, fog, or ordinary lighting can change the
    # road crop smoothly.  The regression being guarded here was temporal
    # alternation: the coplanar route strip dropped in and out against the
    # terrain, producing a sample that departed sharply from both neighbours.
    # Compare each frame with the linear interpolation of its neighbours so
    # legitimate monotonic motion does not masquerade as z-fighting.
    temporal_residuals = [
        abs(
            road_means[index] -
            (road_means[index - 1] + road_means[index + 1]) / 2.0
        )
        for index in range(1, len(road_means) - 1)
    ]

    failures: list[str] = []
    if len(frames) != args.expected_frames:
        failures.append(
            f"expected {args.expected_frames} gameplay burst frames, "
            f"found {len(frames)}"
        )
    if len(sizes) != 1:
        failures.append(f"capture dimensions changed: {sizes}")
    # The interval containing the native loading-to-gameplay display change
    # legitimately includes the old 4:3 loading surface and its resize. The
    # regression was repeated fallback after the first enhanced gameplay RT
    # existed, so acceptance starts with the following complete interval.
    steady_performance = performance[1:]
    def is_enhanced_rt(source: str) -> bool:
        return source == "rt" or source.startswith("rt:")

    first_rt = next(
        (
            index
            for index, source in enumerate(gameplay_sources)
            if is_enhanced_rt(source)
        ),
        len(gameplay_sources),
    )
    steady_sources = gameplay_sources[first_rt:]

    if not steady_performance:
        failures.append("no gameplay performance intervals found")
    if any(
        item["vram_fallback_presents"]
        for item in steady_performance
    ):
        failures.append("gameplay fell back to the native VRAM source")
    if any(item["present_reallocations"] for item in steady_performance):
        failures.append("presentation target was reallocated during gameplay")
    if not gameplay_sources:
        failures.append("no per-frame gameplay presentation-source trace found")
    if any(not is_enhanced_rt(source) for source in steady_sources):
        failures.append("a gameplay presentation source was not an enhanced RT")
    max_jump = max(adjacent_luma_jumps, default=0.0)
    max_temporal_residual = max(temporal_residuals, default=0.0)
    if max_temporal_residual > 1.5:
        failures.append(
            "road-region temporal residual "
            f"{max_temporal_residual:.3f} exceeds 1.5"
        )
    if not any(route_render_intervals):
        failures.append("no TerrainRoute triangles were rendered during gameplay")
    if not route_packet_words:
        failures.append("no source-tagged terrain-route packets were observed")
    if any(words != 12 for words in route_packet_words):
        failures.append(
            "a source-tagged terrain-route packet was not a native 12-word GT4"
        )

    if args.contact_sheet and frames:
        make_contact_sheet(frames, args.contact_sheet)
        representative_indexes = {
            0,
            3,
            4,
            len(frames) // 2,
            14,
            15,
            len(frames) - 1,
        }
        for index in sorted(
            index
            for index in representative_indexes
            if index < len(frames)
        ):
            frames[index][1].save(
                args.contact_sheet.parent /
                f"flicker-frame-{index:03d}.png"
            )

    report = {
        "schema": "v8.enhanced-runtime-stability.v2",
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "gameplay_performance_intervals": len(performance),
        "steady_gameplay_performance_intervals": len(
            steady_performance
        ),
        "vram_fallback_presents": sum(
            item["vram_fallback_presents"]
            for item in steady_performance
        ),
        "present_reallocations": sum(
            item["present_reallocations"]
            for item in steady_performance
        ),
        "gameplay_source_events": len(gameplay_sources),
        "steady_gameplay_source_events": len(steady_sources),
        "non_rt_gameplay_source_events": sum(
            not is_enhanced_rt(source) for source in steady_sources
        ),
        "expected_capture_frame_count": args.expected_frames,
        "capture_frame_count": len(frames),
        "capture_sizes": [list(size) for size in sizes],
        "road_region_luma": [round(value, 3) for value in road_means],
        "max_adjacent_road_luma_jump": round(max_jump, 3),
        "road_region_temporal_residual": [
            round(value, 3) for value in temporal_residuals
        ],
        "max_road_temporal_residual": round(
            max_temporal_residual,
            3,
        ),
        "terrain_route_render_intervals": route_render_intervals,
        "terrain_route_triangles": sum(route_render_intervals),
        "source_tagged_terrain_route_packets": len(route_packet_words),
        "terrain_route_packet_word_counts": sorted(set(route_packet_words)),
        "stderr_log": str(args.stderr_log.resolve()),
        "capture_dir": str(args.capture_dir.resolve()),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2) + "\n",
        encoding="utf-8",
    )
    print(json.dumps(report, indent=2))
    raise SystemExit(0 if not failures else 1)


if __name__ == "__main__":
    main()
