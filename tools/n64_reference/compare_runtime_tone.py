#!/usr/bin/env python3
"""Compare N64 final RDP shade modulation with PS1 GTE world shading."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


def weighted_mean(records: list[dict[str, object]]) -> tuple[list[float], int]:
    total = sum(int(record["shaded_triangles"]) for record in records)
    mean = [
        sum(
            float(record["shade_start"]["mean_rgb"][channel])
            * int(record["shaded_triangles"])
            for record in records
        ) / total
        for channel in range(3)
    ]
    return mean, total


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("n64", type=Path)
    parser.add_argument("ps1", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument(
        "--minimum-n64-share",
        type=float,
        default=0.10,
        help="Select dominant shaded combiner passes at or above this share",
    )
    args = parser.parse_args()

    n64 = json.loads(args.n64.read_text(encoding="utf-8"))
    ps1 = json.loads(args.ps1.read_text(encoding="utf-8"))
    n64_total = int(n64["shaded_triangles"])
    dominant = [
        record for record in n64["combiners"]
        if int(record["shaded_triangles"]) / n64_total >= args.minimum_n64_share
    ]
    n64_rgb, selected = weighted_mean(dominant)
    n64_factor = [value / 255.0 for value in n64_rgb]
    ps1_output = ps1["unscoped_normal_colour_output"]
    ps1_rgb = [float(value) for value in ps1_output["mean_rgb"]]
    ps1_factor = [value / 128.0 for value in ps1_rgb]
    delta = [ps1_factor[c] - n64_factor[c] for c in range(3)]

    report = {
        "schema": "v8.runtime-tone-parity.v1",
        "n64_proof": str(args.n64.resolve()),
        "ps1_proof": str(args.ps1.resolve()),
        "comparison_contract": {
            "n64": "RDP 16.16 shade start divided by 255",
            "ps1": "GTE normal-colour output divided by 128, matching the port's native 7-bit texture modulation",
            "scope": "two dominant N64 shaded combiner passes versus unscoped PS1 normal-colour world output",
            "limitation": "aggregate parity does not prove identical per-triangle pairing or framebuffer presentation",
        },
        "n64": {
            "selected_shaded_triangles": selected,
            "total_shaded_triangles": n64_total,
            "selected_share": round(selected / n64_total, 8),
            "selected_combiners": [
                {
                    "word0": record["word0"],
                    "word1": record["word1"],
                    "triangles": record["shaded_triangles"],
                }
                for record in dominant
            ],
            "weighted_mean_rgb": [round(value, 6) for value in n64_rgb],
            "mean_modulation_factor": [round(value, 8) for value in n64_factor],
        },
        "ps1": {
            "vertices": ps1_output["samples"],
            "mean_rgb_7bit": ps1_rgb,
            "mean_modulation_factor": [round(value, 8) for value in ps1_factor],
        },
        "ps1_minus_n64_modulation": [round(value, 8) for value in delta],
        "absolute_delta": [round(abs(value), 8) for value in delta],
        "maximum_absolute_delta": round(max(abs(value) for value in delta), 8),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
