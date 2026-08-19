#!/usr/bin/env python3
"""Audit source PS1 GTE light state and normal-colour output from a runtime log."""

from __future__ import annotations

import argparse
from collections import Counter
import json
from pathlib import Path
import re


LIGHT_RE = re.compile(
    r"\[GteLighting\] tick=(?P<tick>\d+) reg=(?P<reg>\d+) "
    r"llm=(?P<llm>[-\d,]+) back=(?P<back>[-\d,]+) color=(?P<color>[-\d,]+)"
)
SHADE_RE = re.compile(
    r"\[GteShade\] tick=(?P<tick>\d+) op=0x(?P<op>[0-9A-Fa-f]+) "
    r"normal=(?P<normal>[-\d,]+) rgb=(?P<rgb>\d+,\d+,\d+) owner=(?P<owner>.*)"
)
NORMAL_COLOUR_OPS = {0x13, 0x16, 0x1B, 0x1E, 0x20, 0x3F}


def integers(value: str) -> tuple[int, ...]:
    return tuple(int(item) for item in value.split(","))


def shade_summary(samples: list[tuple[int, int, int]]) -> dict[str, object]:
    if not samples:
        return {
            "samples": 0,
            "unique_rgb": 0,
            "mean_rgb": None,
            "mean_modulation_factor": None,
            "top_rgb": [],
        }
    counter = Counter(samples)
    mean = [sum(rgb[c] for rgb in samples) / len(samples) for c in range(3)]
    return {
        "samples": len(samples),
        "unique_rgb": len(counter),
        "mean_rgb": [round(value, 6) for value in mean],
        # The recompiled PS1 shader reproduces the original 7-bit colour
        # modulation: (texel * vertexColour) >> 7.
        "mean_modulation_factor": [round(value / 128.0, 8) for value in mean],
        "top_rgb": [
            {"rgb": list(rgb), "vertices": count}
            for rgb, count in counter.most_common(32)
        ],
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    lighting: Counter[tuple[tuple[int, ...], tuple[int, ...], tuple[int, ...]]] = Counter()
    lighting_registers: Counter[int] = Counter()
    all_samples: list[tuple[int, int, int]] = []
    normal_samples: list[tuple[int, int, int]] = []
    unscoped_samples: list[tuple[int, int, int]] = []
    by_op: dict[int, list[tuple[int, int, int]]] = {}
    by_owner: dict[str, list[tuple[int, int, int]]] = {}
    malformed = 0

    for line in args.log.read_text(encoding="utf-8", errors="replace").splitlines():
        if "[GteLighting]" in line:
            match = LIGHT_RE.search(line)
            if not match:
                malformed += 1
                continue
            state = (
                integers(match.group("llm")),
                integers(match.group("back")),
                integers(match.group("color")),
            )
            lighting[state] += 1
            lighting_registers[int(match.group("reg"))] += 1
            continue
        if "[GteShade]" not in line:
            continue
        match = SHADE_RE.search(line)
        if not match:
            malformed += 1
            continue
        op = int(match.group("op"), 16)
        rgb = integers(match.group("rgb"))
        owner = match.group("owner")
        all_samples.append(rgb)
        by_op.setdefault(op, []).append(rgb)
        by_owner.setdefault(owner, []).append(rgb)
        if op in NORMAL_COLOUR_OPS:
            normal_samples.append(rgb)
            if owner == "unscoped":
                unscoped_samples.append(rgb)

    states = []
    for (llm, back, color), uses in lighting.most_common():
        states.append({
            "uses": uses,
            "light_matrix_raw_12bit": list(llm),
            "back_colour_raw_12bit": list(back),
            "back_colour_8bit_equivalent": [round(value / 16.0, 6) for value in back],
            "colour_matrix_raw_12bit": list(color),
            "directional_colours_8bit_equivalent": [
                [round(color[row * 3 + column] / 16.0, 6) for row in range(3)]
                for column in range(3)
            ],
        })

    report = {
        "schema": "v8.ps1-runtime-gte-shade.v1",
        "log": str(args.log.resolve()),
        "normal_colour_ops": [f"0x{op:02X}" for op in sorted(NORMAL_COLOUR_OPS)],
        "lighting_updates": sum(lighting.values()),
        "lighting_register_updates": {str(key): value for key, value in sorted(lighting_registers.items())},
        "lighting_states": states,
        "all_shade_output": shade_summary(all_samples),
        "normal_colour_output": shade_summary(normal_samples),
        "unscoped_normal_colour_output": shade_summary(unscoped_samples),
        "shade_output_by_op": {
            f"0x{op:02X}": shade_summary(samples)
            for op, samples in sorted(by_op.items())
        },
        "shade_output_by_owner": {
            owner: shade_summary(samples)
            for owner, samples in sorted(by_owner.items())
        },
        "malformed_trace_lines": malformed,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({
        "lighting_updates": report["lighting_updates"],
        "lighting_states": len(states),
        "all_shade_output": report["all_shade_output"],
        "normal_colour_output": report["normal_colour_output"],
        "unscoped_normal_colour_output": report["unscoped_normal_colour_output"],
        "malformed_trace_lines": malformed,
    }, indent=2))
    return 0 if malformed == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
