#!/usr/bin/env python3
"""Verify every traced runtime terrain-atlas hit against source indices."""
from __future__ import annotations

import argparse
import base64
import json
import re
from pathlib import Path


HIT = re.compile(
    r"\[TerrainAtlasFragment\] hit mode=(catalog|anchor) atlas=(\S+) "
    r"depth=(\d+) live=(\d+),(\d+) source=(\d+),(\d+) "
    r"size=(\d+)x(\d+) indexHash=([0-9a-f]{16}) anchors=(\d+)"
)
MISS = re.compile(r"\[TerrainAtlasFragment\] miss ")


def fnv_region(
    indices: bytes,
    source_width: int,
    depth: int,
    x0: int,
    y0: int,
    width: int,
    height: int,
) -> int:
    value = 14695981039346656037

    def add(byte: int) -> None:
        nonlocal value
        value ^= byte
        value = (value * 1099511628211) & 0xFFFFFFFFFFFFFFFF

    add(width & 0xFF)
    add((width >> 8) & 0xFF)
    add(height & 0xFF)
    add((height >> 8) & 0xFF)
    bytes_per_pixel = 2 if depth == 2 else 1
    for y in range(height):
        start = ((y0 + y) * source_width + x0) * bytes_per_pixel
        for byte in indices[start : start + width * bytes_per_pixel]:
            add(byte)
    return value


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("manifest", type=Path)
    parser.add_argument("log", type=Path)
    parser.add_argument("--atlas", required=True)
    args = parser.parse_args()

    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    entry = next(
        item
        for item in manifest.get("terrainAtlases", [])
        if item.get("name") == args.atlas
    )
    log = args.log.read_text(encoding="utf-8", errors="replace")
    indices = base64.b64decode(entry["indices"])
    source_width = int(entry["width"])
    source_height = int(entry["height"])
    source_depth = int(entry["depth"])
    hits = HIT.findall(log)
    mismatches: list[dict[str, object]] = []
    modes = {"catalog": 0, "anchor": 0}
    for mode, atlas, depth, _live_x, _live_y, source_x, source_y, width, height, runtime_hash, _anchors in hits:
        modes[mode] += 1
        x = int(source_x)
        y = int(source_y)
        w = int(width)
        h = int(height)
        actual = fnv_region(indices, source_width, source_depth, x, y, w, h)
        valid = (
            atlas == args.atlas
            and int(depth) == source_depth
            and x >= 0
            and y >= 0
            and x + w <= source_width
            and y + h <= source_height
            and actual == int(runtime_hash, 16)
        )
        if not valid:
            mismatches.append({
                "mode": mode,
                "atlas": atlas,
                "source": [x, y],
                "size": [w, h],
                "runtimeHash": runtime_hash,
                "sourceHash": f"{actual:016x}",
            })

    report = {
        "schema": "v82-terrain-atlas-fragment-audit-v2",
        "atlas": args.atlas,
        "tracedHits": len(hits),
        "catalogHits": modes["catalog"],
        "anchorHits": modes["anchor"],
        "tracedMisses": len(MISS.findall(log)),
        "hashMismatches": len(mismatches),
        "mismatchSample": mismatches[:32],
    }
    print(json.dumps(report, indent=2))
    return 0 if hits and not mismatches and not MISS.findall(log) else 1


if __name__ == "__main__":
    raise SystemExit(main())
