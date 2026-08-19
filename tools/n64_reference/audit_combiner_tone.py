#!/usr/bin/env python3
"""Audit the authored N64 combiner and vertex-colour state per triangle."""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import sys


TOOLS = Path(__file__).resolve().parents[1]
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import v8_n64_level as n64  # noqa: E402
from n64_reference.audit_rdp_state import (  # noqa: E402
    G_LIGHTING,
    object_heads,
    source_group_owners,
    xobf_bins,
)


def looks_like_normal(rgb: tuple[int, int, int]) -> bool:
    components = tuple(value - 256 if value >= 128 else value for value in rgb)
    magnitude_squared = sum(value * value for value in components)
    return 14000 <= magnitude_squared <= 18000


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exp", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    exp = args.exp.read_bytes()
    bins = xobf_bins(exp)
    owners = source_group_owners(exp, bins)
    combinations: Counter[tuple[int, int]] = Counter()
    combination_groups: dict[tuple[int, int], set[tuple[int, int]]] = defaultdict(set)
    combination_owners: dict[tuple[int, int], set[str]] = defaultdict(set)
    combination_vertex_shade: dict[tuple[int, int], Counter[tuple[int, int, int]]] = defaultdict(Counter)
    combination_vertex_normals: dict[tuple[int, int], Counter[tuple[int, int, int]]] = defaultdict(Counter)
    combination_attribute_roles: dict[tuple[int, int], Counter[str]] = defaultdict(Counter)
    combination_primitive_rgb: dict[tuple[int, int], Counter[tuple[int, int, int]]] = defaultdict(Counter)
    warnings: list[str] = []

    for bank, data in enumerate(bins):
        group_count = n64.be32(data, 0)
        group_table = n64.be32(data, 4)
        group_targets = [
            group_table + n64.be32(data, group_table + index * 4)
            for index in range(group_count)
        ]
        for group, group_offset in enumerate(group_targets):
            vertex_relative = n64.be32(data, group_offset + 4)
            vertex_base = group_offset + vertex_relative
            for display_relative in (
                n64.be32(data, group_offset + 8),
                n64.be32(data, group_offset + 12),
            ):
                if display_relative == 0 or vertex_relative == 0:
                    continue
                cursor = group_offset + display_relative
                cache: dict[int, tuple[int, int, int]] = {}
                combine = (0, 0)
                primitive = (127, 127, 127)
                geometry = 0
                geometry_known = 0
                for _ in range(0x10000):
                    if cursor + 8 > len(data):
                        raise n64.FormatError(
                            f"bank {bank} group {group} display is truncated"
                        )
                    command = data[cursor:cursor + 8]
                    cursor += 8
                    opcode = command[0]
                    word0 = n64.be32(command, 0)
                    word1 = n64.be32(command, 4)
                    if opcode == 0xDF:
                        break
                    if opcode == 0xFC:
                        combine = (word0 & 0x00FFFFFF, word1)
                        continue
                    if opcode == 0xFA:
                        primitive = (command[4], command[5], command[6])
                        continue
                    if opcode == 0xD9:
                        keep = word0 & 0x00FFFFFF
                        geometry = (geometry & keep) | word1
                        geometry_known = (
                            (geometry_known & keep)
                            | ((~keep) & 0x00FFFFFF)
                            | word1
                        )
                        continue
                    if opcode == 0x01:
                        count = (word0 >> 12) & 0xFF
                        end_index = (word0 >> 1) & 0x7F
                        first_index = end_index - count
                        source = vertex_base + (word1 & 0xFFFFFF)
                        for item in range(count):
                            offset = source + item * 16
                            cache[first_index + item] = tuple(
                                data[offset + 12:offset + 15]
                            )
                        continue
                    triangles: tuple[tuple[int, int, int], ...] = ()
                    if opcode == 0x05:
                        triangles = ((
                            command[1] >> 1,
                            command[2] >> 1,
                            command[3] >> 1,
                        ),)
                    elif opcode == 0x06:
                        triangles = (
                            (command[1] >> 1, command[2] >> 1, command[3] >> 1),
                            (command[5] >> 1, command[6] >> 1, command[7] >> 1),
                        )
                    for indices in triangles:
                        if any(index not in cache for index in indices):
                            warnings.append(
                                f"bank={bank} group={group} missing vertex cache"
                            )
                            continue
                        combinations[combine] += 1
                        combination_groups[combine].add((bank, group))
                        combination_owners[combine].update(
                            owners.get((bank, group), ())
                        )
                        combination_primitive_rgb[combine][primitive] += 1
                        attributes = tuple(cache[index] for index in indices)
                        if geometry_known & G_LIGHTING:
                            role = (
                                "normal_explicit"
                                if geometry & G_LIGHTING
                                else "shade_explicit"
                            )
                        elif all(looks_like_normal(value) for value in attributes):
                            role = "normal_inherited"
                        else:
                            role = "shade_inherited"
                        combination_attribute_roles[combine][role] += 1
                        for index in indices:
                            value = cache[index]
                            if role.startswith("normal"):
                                signed = tuple(
                                    channel - 256 if channel >= 128 else channel
                                    for channel in value
                                )
                                combination_vertex_normals[combine][signed] += 1
                            else:
                                combination_vertex_shade[combine][value] += 1
                else:
                    raise n64.FormatError(
                        f"bank {bank} group {group} has no display-list end"
                    )

    records = []
    for combine, triangles in combinations.most_common():
        shade_counter = combination_vertex_shade[combine]
        shade_samples = sum(shade_counter.values())
        mean_shade = [
            round(
                sum(rgb[channel] * count for rgb, count in shade_counter.items())
                / max(1, shade_samples),
                4,
            )
            for channel in range(3)
        ] if shade_samples else None
        records.append({
            "combine_word0_low24": f"0x{combine[0]:06X}",
            "combine_word1": f"0x{combine[1]:08X}",
            "triangles": triangles,
            "groups": [list(item) for item in sorted(combination_groups[combine])],
            "owners": sorted(combination_owners[combine]),
            "attribute_roles": dict(combination_attribute_roles[combine]),
            "mean_authored_shade_rgb": mean_shade,
            "top_authored_shade_rgb": [
                {"rgb": list(rgb), "uses": count}
                for rgb, count in shade_counter.most_common(16)
            ],
            "top_authored_normals": [
                {"xyz": list(xyz), "uses": count}
                for xyz, count in combination_vertex_normals[combine].most_common(16)
            ],
            "primitive_rgb": [
                {"rgb": list(rgb), "triangles": count}
                for rgb, count in combination_primitive_rgb[combine].most_common()
            ],
        })

    report = {
        "schema": "v8.n64-combiner-tone.v1",
        "source": str(args.exp.resolve()),
        "object_heads": len(object_heads(exp)),
        "triangles": sum(combinations.values()),
        "combiners": records,
        "warnings": warnings,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({
        "triangles": report["triangles"],
        "combiners": [
            {
                "word0": item["combine_word0_low24"],
                "word1": item["combine_word1"],
                "triangles": item["triangles"],
                "attribute_roles": item["attribute_roles"],
                "mean_authored_shade_rgb": item["mean_authored_shade_rgb"],
            }
            for item in records
        ],
        "warnings": len(warnings),
    }, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
