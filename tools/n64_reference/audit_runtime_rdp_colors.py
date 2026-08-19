#!/usr/bin/env python3
"""Find runtime RDP color commands in captured N64 RDRAM images.

The graphics proxy stores emulator-native RDRAM.  Depending on the core, each
32-bit word is either ordinary big-endian byte order or host-word-swapped.
Audit both interpretations and rank SetFogColor candidates by persistence at
the same address across several gameplay captures.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import struct


COLOR_COMMANDS = {
    0xF8: "fog",
    0xF9: "blend",
    0xFA: "primitive",
    0xFB: "environment",
}


def words(data: bytes, layout: str, offset: int) -> tuple[int, int]:
    return struct.unpack_from(">II" if layout == "big" else "<II", data, offset)


def command_density(data: bytes, layout: str, offset: int) -> int:
    score = 0
    for neighbor in range(max(0, offset - 32), min(len(data) - 7, offset + 40), 8):
        word0, _word1 = words(data, layout, neighbor)
        opcode = word0 >> 24
        if opcode >= 0xC0 or opcode in {
            0x01, 0x03, 0x04, 0x05, 0x06, 0xD7, 0xD9,
        }:
            score += 1
    return score


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dumps", nargs="+", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    per_layout: dict[str, dict[str, object]] = {}
    for layout in ("big", "host-word"):
        by_address: dict[tuple[int, str, tuple[int, int, int, int]], set[str]] = defaultdict(set)
        colors: Counter[tuple[str, tuple[int, int, int, int]]] = Counter()
        dense = 0
        total = 0
        fog_setups: Counter[
            tuple[tuple[int, int, int, int], int, int, float, float]
        ] = Counter()
        for path in args.dumps:
            data = path.read_bytes()
            for offset in range(0, len(data) - 7, 8):
                word0, word1 = words(data, layout, offset)
                kind = COLOR_COMMANDS.get(word0 >> 24)
                if kind is None:
                    continue
                rgba = (
                    (word1 >> 24) & 255,
                    (word1 >> 16) & 255,
                    (word1 >> 8) & 255,
                    word1 & 255,
                )
                total += 1
                colors[(kind, rgba)] += 1
                density = command_density(data, layout, offset)
                if density >= 5:
                    dense += 1
                    by_address[(offset, kind, rgba)].add(path.name)
                if kind == "fog":
                    for factor_offset in range(offset + 8, min(offset + 40, len(data) - 7), 8):
                        factor0, factor1 = words(data, layout, factor_offset)
                        if factor0 != 0xDB080000:
                            continue
                        multiplier = (factor1 >> 16) & 0xFFFF
                        displacement = factor1 & 0xFFFF
                        if displacement >= 0x8000:
                            displacement -= 0x10000
                        if multiplier:
                            span = 128000.0 / multiplier
                            minimum = 500.0 - displacement * span / 256.0
                            maximum = minimum + span
                            fog_setups[(
                                rgba,
                                multiplier,
                                displacement,
                                round(minimum, 4),
                                round(maximum, 4),
                            )] += 1
                        break

        persistent = [
            {
                "offset": f"0x{offset:08X}",
                "kind": kind,
                "rgba": list(rgba),
                "captures": len(files),
                "files": sorted(files),
            }
            for (offset, kind, rgba), files in by_address.items()
            if len(files) >= 2
        ]
        persistent.sort(
            key=lambda item: (-int(item["captures"]), item["kind"], item["offset"])
        )
        per_layout[layout] = {
            "color_opcode_hits": total,
            "dense_command_neighborhood_hits": dense,
            "persistent_candidates": persistent,
            "top_colors": [
                {
                    "kind": key[0],
                    "rgba": list(key[1]),
                    "occurrences": count,
                }
                for key, count in colors.most_common(32)
            ],
            "fog_setups": [
                {
                    "rgba": list(key[0]),
                    "multiplier": key[1],
                    "displacement": key[2],
                    "minimum_per_mille": key[3],
                    "maximum_per_mille": key[4],
                    "occurrences": count,
                }
                for key, count in fog_setups.most_common()
            ],
        }

    report = {
        "schema": "v8.n64-runtime-rdp-colors.v1",
        "dumps": [str(path.resolve()) for path in args.dumps],
        "layouts": per_layout,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(
        json.dumps(
            {
                layout: {
                    "color_opcode_hits": values["color_opcode_hits"],
                    "dense_command_neighborhood_hits": values[
                        "dense_command_neighborhood_hits"
                    ],
                    "fog_setups": values["fog_setups"],
                }
                for layout, values in per_layout.items()
            },
            indent=2,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
