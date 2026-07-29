#!/usr/bin/env python3
"""Report abrupt height-sample edges in a V8 N64 terrain EXP."""

from __future__ import annotations

import argparse
from pathlib import Path

import v8_n64_level as n64


def decode_samples(exp: bytes) -> dict[tuple[int, int], tuple[int, int, bytes]]:
    children = n64.root_children(exp)
    zmaps = [child.payload for child in children if child.tag == b"ZMAP"]
    zones = [child.payload for child in children if child.tag == b"ZONE"]
    if len(zmaps) != 1 or not zones:
        raise ValueError("terrain needs exactly one ZMAP and at least one ZONE")

    samples: dict[tuple[int, int], tuple[int, int, bytes]] = {}
    zmap = zmaps[0]
    for map_offset in range(0, len(zmap), 2):
        zone_index = n64.be16(zmap, map_offset)
        if zone_index == 0:
            continue
        if zone_index > len(zones):
            raise ValueError(f"ZMAP references missing ZONE {zone_index}")
        zone = zones[zone_index - 1]
        chunk_index = map_offset // 2
        chunk_x = chunk_index & 31
        chunk_z = chunk_index >> 5
        for local_x in range(64):
            for local_z in range(64):
                offset = (local_x * 64 + local_z) * 4
                encoded = zone[offset : offset + 4]
                if len(encoded) != 4:
                    raise ValueError(f"ZONE {zone_index} is truncated")
                height = (
                    (n64.be16(encoded, 0) - 0x0200)
                    | ((encoded[2] >> 3) << 11)
                ) & 0x07FF
                samples[
                    (chunk_x * 64 + local_x, chunk_z * 64 + local_z)
                ] = (height, encoded[3], encoded)
    return samples


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("exp", type=Path)
    parser.add_argument("--center", help="optional cell X,Z")
    parser.add_argument("--radius", type=int, default=16)
    parser.add_argument("--limit", type=int, default=100)
    args = parser.parse_args()

    samples = decode_samples(args.exp.read_bytes())
    edges: list[
        tuple[int, tuple[int, int], tuple[int, int], int, int, int, int]
    ] = []
    for (x, z), (height, material, _raw) in samples.items():
        for neighbor in ((x + 1, z), (x, z + 1)):
            if neighbor not in samples:
                continue
            other_height, other_material, _other_raw = samples[neighbor]
            edges.append(
                (
                    abs(other_height - height),
                    (x, z),
                    neighbor,
                    height,
                    other_height,
                    material,
                    other_material,
                )
            )
    edges.sort(reverse=True)

    selected = edges
    if args.center:
        center_x, center_z = (int(value) for value in args.center.split(",", 1))
        radius = max(0, args.radius)
        selected = [
            edge
            for edge in edges
            if any(
                abs(point[0] - center_x) <= radius
                and abs(point[1] - center_z) <= radius
                for point in (edge[1], edge[2])
            )
        ]

    print(f"samples={len(samples)} edges={len(edges)} selected={len(selected)}")
    for delta, first, second, h0, h1, m0, m1 in selected[: args.limit]:
        raw0 = samples[first][2].hex()
        raw1 = samples[second][2].hex()
        print(
            f"delta={delta:4d} {first} h={h0:4d} mat={m0:3d} raw={raw0} -> "
            f"{second} h={h1:4d} mat={m1:3d} raw={raw1}"
        )


if __name__ == "__main__":
    main()
