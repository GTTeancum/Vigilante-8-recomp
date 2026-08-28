#!/usr/bin/env python3
"""Summarize verbose V8:2 renderer fidelity traces without image capture."""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import re


TERRAIN_PATH = re.compile(
    r"\[V82TerrainPath\].*?frame=(\d+).*?packet=(0x[0-9A-F]+) "
    r"source=(detail|coarse|transition|unresolved).*?"
    r"rgb=(\d+),(\d+),(\d+);(\d+),(\d+),(\d+);(\d+),(\d+),(\d+)"
)
FONT_RESOLVE = re.compile(
    r"\[V82LoadingFontResolve\].*?key=([0-9a-f]{16}) hit=([01]) "
    r"size=(\d+)x(\d+).*?kind=([^ ]+)(?: .*?source-label=\"([^\"]*)\")?"
)


def color_summary(samples: list[tuple[int, ...]]) -> dict[str, object]:
    if not samples:
        return {"samples": 0}
    flattened = [value for sample in samples for value in sample]
    per_triangle = [sum(sample) / len(sample) for sample in samples]
    return {
        "samples": len(samples),
        "channel_min": min(flattened),
        "channel_max": max(flattened),
        "channel_mean": round(sum(flattened) / len(flattened), 3),
        "triangle_mean_min": round(min(per_triangle), 3),
        "triangle_mean_max": round(max(per_triangle), 3),
    }


def analyze(path: Path) -> dict[str, object]:
    packet_sources: dict[tuple[int, str], set[str]] = defaultdict(set)
    terrain_rows: list[tuple[tuple[int, str], str, tuple[int, ...]]] = []
    font_hits = Counter()
    font_misses = Counter()
    font_sources: dict[str, str] = {}

    with path.open("r", encoding="utf-8", errors="ignore") as stream:
        for line in stream:
            terrain = TERRAIN_PATH.search(line)
            if terrain is not None:
                frame, packet, source, *channels = terrain.groups()
                key = (int(frame), packet)
                packet_sources[key].add(source)
                terrain_rows.append((key, source, tuple(map(int, channels))))
                continue
            font = FONT_RESOLVE.search(line)
            if font is not None:
                key, hit, width, height, kind, source = font.groups()
                label = f"{width}x{height}:{kind}"
                (font_hits if hit == "1" else font_misses)[label] += 1
                if source:
                    font_sources[key] = source

    terrain_colors: dict[str, list[tuple[int, ...]]] = defaultdict(list)
    for key, source, colors in terrain_rows:
        sources = packet_sources[key]
        if source == "detail" and "coarse" in sources:
            group = "coarse_reconstructed_detail"
        elif source == "detail" and "transition" in sources:
            group = "transition_reconstructed_detail"
        elif source == "detail":
            group = "native_detail"
        else:
            group = source
        terrain_colors[group].append(colors)

    return {
        "trace": str(path.resolve()),
        "terrain": {
            group: color_summary(samples)
            for group, samples in sorted(terrain_colors.items())
        },
        "font": {
            "hits": dict(sorted(font_hits.items())),
            "misses": dict(sorted(font_misses.items())),
            "resolved_keys": len(font_sources),
            "resolved_sources": font_sources,
        },
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("trace", type=Path)
    args = parser.parse_args()
    print(json.dumps(analyze(args.trace), indent=2))


if __name__ == "__main__":
    main()
