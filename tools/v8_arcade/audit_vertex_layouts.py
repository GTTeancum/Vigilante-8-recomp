#!/usr/bin/env python3
"""Audit candidate UV encodings in V8 Arcade vertex streams.

Run with Blender's Python when a system Python is unavailable:

    blender --background --factory-startup --python audit_vertex_layouts.py \
      -- Piranha.ib --model 0xA1DCC
"""

from __future__ import annotations

import argparse
import json
import math
import statistics
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from extract_vehicle_blends import semantic_layout, shader_semantics
from extract_vehicle_lods import decode_bundle, find_models


def quantile(values: list[float], fraction: float) -> float:
    if not values:
        return math.inf
    ordered = sorted(values)
    return ordered[min(len(ordered) - 1, int((len(ordered) - 1) * fraction))]


def decode_pair(data: bytes, offset: int, encoding: str) -> tuple[float, float]:
    if encoding == "float2":
        return struct.unpack_from(">2f", data, offset)
    if encoding == "half2":
        return struct.unpack_from(">2e", data, offset)
    if encoding == "snorm16x2":
        x, y = struct.unpack_from(">2h", data, offset)
        return max(-1.0, x / 32767.0), max(-1.0, y / 32767.0)
    if encoding == "unorm16x2":
        x, y = struct.unpack_from(">2H", data, offset)
        return x / 65535.0, y / 65535.0
    raise ValueError(encoding)


def candidate_record(
    data: bytes,
    vertex_data: int,
    vertex_count: int,
    stride: int,
    offset: int,
    encoding: str,
) -> dict[str, object]:
    values: list[tuple[float, float]] = []
    invalid = 0
    for index in range(vertex_count):
        try:
            value = decode_pair(data, vertex_data + index * stride + offset, encoding)
        except (OverflowError, struct.error):
            invalid += 1
            continue
        if not all(math.isfinite(component) for component in value):
            invalid += 1
            continue
        values.append(value)
    magnitudes = [max(abs(value[0]), abs(value[1])) for value in values]
    in_unit = sum(
        -0.125 <= value[0] <= 1.125 and -0.125 <= value[1] <= 1.125
        for value in values
    )
    distinct = len({(round(value[0], 6), round(value[1], 6)) for value in values})
    return {
        "offset": offset,
        "encoding": encoding,
        "valid_count": len(values),
        "invalid_count": invalid,
        "distinct_count": distinct,
        "unit_fraction": in_unit / len(values) if values else 0.0,
        "abs_q95": quantile(magnitudes, 0.95),
        "abs_q99": quantile(magnitudes, 0.99),
        "u_min": min((value[0] for value in values), default=None),
        "u_max": max((value[0] for value in values), default=None),
        "v_min": min((value[1] for value in values), default=None),
        "v_max": max((value[1] for value in values), default=None),
        "samples": values[:8],
    }


def audit(path: Path, model_filter: int | None) -> dict[str, object]:
    data, container = decode_bundle(path)
    records = []
    for model in find_models(data):
        if model_filter is not None and model.offset != model_filter:
            continue
        for submesh_index, submesh in enumerate(model.submeshes):
            semantics = shader_semantics(data, submesh.technique_offset)
            candidates = []
            for encoding, size in (
                ("float2", 8),
                ("half2", 4),
                ("snorm16x2", 4),
                ("unorm16x2", 4),
            ):
                for offset in range(12, submesh.vertex_stride - size + 1, 4):
                    candidates.append(
                        candidate_record(
                            data,
                            submesh.vertex_data_offset,
                            submesh.vertex_count,
                            submesh.vertex_stride,
                            offset,
                            encoding,
                        )
                    )
            candidates.sort(
                key=lambda item: (
                    int(item["invalid_count"]) != 0,
                    -float(item["unit_fraction"]),
                    float(item["abs_q99"]),
                    -int(item["distinct_count"]),
                )
            )
            records.append(
                {
                    "model_offset": model.offset,
                    "submesh_index": submesh_index,
                    "technique_offset": submesh.technique_offset,
                    "parameter_offset": submesh.parameters_offset,
                    "vertex_data_offset": submesh.vertex_data_offset,
                    "vertex_count": submesh.vertex_count,
                    "stride": submesh.vertex_stride,
                    "semantics": semantics,
                    "current_layout": semantic_layout(
                        semantics, submesh.vertex_stride
                    ),
                    "best_uv_candidates": candidates[:12],
                }
            )
    return {
        "source": str(path.resolve()),
        "container": container,
        "model_filter": model_filter,
        "submeshes": records,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("--model", type=lambda value: int(value, 0))
    parser.add_argument("--out", type=Path)
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None
    args = parser.parse_args(argv)
    result = audit(args.input, args.model)
    text = json.dumps(result, indent=2) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
