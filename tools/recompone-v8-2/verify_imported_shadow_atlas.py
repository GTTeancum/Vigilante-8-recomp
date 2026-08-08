#!/usr/bin/env python3
"""Decode imported-shadow VRAM traces and reject opaque card coverage."""

from __future__ import annotations

import argparse
import base64
import json
import re
import struct
from pathlib import Path


TRACE = re.compile(
    r"\[ImportedShadowAtlas\] "
    r"(?:source=(?P<source>\w+) )?"
    r"tpage=0x(?P<tpage>[0-9A-Fa-f]+) "
    r"clut=0x(?P<clut>[0-9A-Fa-f]+) "
    r"depth=(?P<depth>\d+) "
    r"page=(?P<page_x>\d+),(?P<page_y>\d+) "
    r"uv=(?P<min_u>\d+),(?P<min_v>\d+),"
    r"(?P<max_u>\d+),(?P<max_v>\d+) "
    r"word-u=(?P<word_u>\d+) "
    r"word-size=(?P<word_w>\d+),(?P<word_h>\d+) "
    r"vertex-rgb=(?P<vertex_rgb>[0-9,;]+) "
    r"palette=(?P<palette>[A-Za-z0-9+/=]*) "
    r"texels=(?P<texels>[A-Za-z0-9+/=]+)"
)


def words(payload: str) -> tuple[int, ...]:
    data = base64.b64decode(payload)
    if len(data) % 2:
        raise ValueError("16-bit VRAM payload has odd length")
    return struct.unpack(f"<{len(data) // 2}H", data)


def rgb5(value: int) -> tuple[float, float, float]:
    return (
        (value & 31) / 31.0,
        ((value >> 5) & 31) / 31.0,
        ((value >> 10) & 31) / 31.0,
    )


def luminance(rgb: tuple[float, float, float]) -> float:
    return rgb[0] * 0.299 + rgb[1] * 0.587 + rgb[2] * 0.114


def indices(
    packed: tuple[int, ...],
    *,
    depth: int,
    word_w: int,
    word_h: int,
    word_u: int,
    min_u: int,
    max_u: int,
) -> list[int]:
    texels_per_word = {0: 4, 1: 2, 2: 1, 3: 1}[depth]
    result: list[int] = []
    for y in range(word_h):
        row = packed[y * word_w : (y + 1) * word_w]
        for u in range(min_u, max_u + 1):
            relative_word = u // texels_per_word - word_u
            value = row[relative_word]
            if depth == 0:
                result.append((value >> ((u & 3) * 4)) & 0xF)
            elif depth == 1:
                result.append((value >> ((u & 1) * 8)) & 0xFF)
            else:
                result.append(value)
    return result


def analyze(match: re.Match[str]) -> dict[str, object]:
    values = match.groupdict()
    depth = int(values["depth"])
    palette = words(values["palette"]) if values["palette"] else ()
    packed = words(values["texels"])
    min_u, max_u = int(values["min_u"]), int(values["max_u"])
    word_u = int(values["word_u"])
    word_w, word_h = int(values["word_w"]), int(values["word_h"])
    samples = indices(
        packed,
        depth=depth,
        word_w=word_w,
        word_h=word_h,
        word_u=word_u,
        min_u=min_u,
        max_u=max_u,
    )
    vertex_values = [
        tuple(int(channel) / 255.0 for channel in vertex.split(","))
        for vertex in values["vertex_rgb"].split(";")
    ]
    maximum_vertex_intensity = max(map(luminance, vertex_values))
    coverages: list[float] = []
    black_coverages: list[float] = []
    colors: list[int] = []
    for sample in samples:
        color = palette[sample] if palette else sample
        colors.append(color)
        texture_intensity = luminance(rgb5(color))
        coverage = min(
            0.20,
            texture_intensity * maximum_vertex_intensity * 0.72,
        )
        coverages.append(coverage)
        if (color & 0x7FFF) == 0:
            black_coverages.append(coverage)
    nonzero = [coverage for coverage in coverages if coverage > 0.001]
    return {
        "source": values["source"] or "enhanced",
        "tpage": f"0x{int(values['tpage'], 16):03X}",
        "clut": f"0x{int(values['clut'], 16):04X}",
        "depth": depth,
        "uv": [
            min_u,
            int(values["min_v"]),
            max_u,
            int(values["max_v"]),
        ],
        "sample_count": len(samples),
        "unique_palette_indices": len(set(samples)),
        "unique_decoded_colors": len(set(colors)),
        "black_sample_count": len(black_coverages),
        "black_maximum_coverage":
            max(black_coverages, default=0.0),
        "nonzero_coverage_count": len(nonzero),
        "maximum_coverage": max(coverages, default=0.0),
        "mean_nonzero_coverage":
            sum(nonzero) / len(nonzero) if nonzero else 0.0,
        "card_coverage_fraction":
            sum(coverage >= 0.19 for coverage in coverages)
            / max(1, len(coverages)),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    text = args.log.read_text(encoding="utf-8", errors="replace")
    records = [analyze(match) for match in TRACE.finditer(text)]
    checks = {
        "shadow_regions_traced": bool(records),
        "black_subtractive_texels_are_no_op": bool(records)
        and all(
            float(record["black_maximum_coverage"]) == 0.0
            for record in records
        ),
        "authored_shadow_intensity_survives": bool(records)
        and any(
            int(record["nonzero_coverage_count"]) > 0
            for record in records
        ),
        "coverage_is_bounded": bool(records)
        and all(
            float(record["maximum_coverage"]) <= 0.200001
            for record in records
        ),
        "binary_opaque_card_rejected": bool(records)
        and all(
            float(record["card_coverage_fraction"]) < 0.95
            for record in records
        ),
    }
    report = {
        "schema": 1,
        "acceptance":
            "decoded imported shadow atlas produces authored, bounded "
            "coverage without an opaque projected card",
        "passed": all(checks.values()),
        "checks": checks,
        "records": records,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2) + "\n",
        encoding="utf-8",
    )
    print(
        f"[ImportedShadowAtlas] "
        f"{'PASS' if report['passed'] else 'FAIL'} "
        f"regions={len(records)}"
    )
    for name, passed in checks.items():
        print(f"  {'PASS' if passed else 'FAIL'} {name}")
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
