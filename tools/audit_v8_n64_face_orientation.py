#!/usr/bin/env python3
"""Report source-space orientation for selected N64 XOBF faces."""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))

import audit_v8_n64_model_conversion as audit
import v8_n64_level as n64


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("exp", type=Path)
    parser.add_argument("--groups", default="103,105")
    parser.add_argument("--texture", type=int, default=61)
    args = parser.parse_args()

    exp = args.exp.read_bytes()
    bank = audit.xobf_bins(exp)[0]
    group_table = n64.be32(bank, 4)
    texture_count = n64.be32(bank, 16)
    texture_table = n64.be32(bank, 20)
    texture_offsets = [
        texture_table + n64.be32(bank, texture_table + index * 4)
        for index in range(texture_count)
    ]
    textures = [
        n64.parse_n64_texture(
            bank,
            offset,
            index,
            texture_offsets[index + 1]
            if index + 1 < texture_count
            else len(bank),
        )
        for index, offset in enumerate(texture_offsets)
    ]
    texture_base = texture_offsets[0] if texture_offsets else 0

    for group in (int(value) for value in args.groups.split(",")):
        group_offset = group_table + n64.be32(
            bank, group_table + group * 4
        )
        vertices, faces, *_ = n64.parse_group_faces(
            bank, group_offset, textures, texture_base
        )
        center = tuple(
            sum(getattr(vertex, axis) for vertex in vertices) / len(vertices)
            for axis in ("x", "y", "z")
        )
        print(f"group={group} center={center} faces={len(faces)}")
        for index, face in enumerate(faces):
            if face.texture != args.texture:
                continue
            points = [
                (vertex.x, vertex.y, vertex.z) for vertex in face.vertices
            ]
            edge_a = tuple(
                points[1][axis] - points[0][axis] for axis in range(3)
            )
            edge_b = tuple(
                points[2][axis] - points[0][axis] for axis in range(3)
            )
            normal = (
                edge_a[1] * edge_b[2] - edge_a[2] * edge_b[1],
                edge_a[2] * edge_b[0] - edge_a[0] * edge_b[2],
                edge_a[0] * edge_b[1] - edge_a[1] * edge_b[0],
            )
            face_center = tuple(
                sum(point[axis] for point in points) / 3.0
                for axis in range(3)
            )
            radial = tuple(
                face_center[axis] - center[axis] for axis in range(3)
            )
            normal_dot_radial = sum(
                normal[axis] * radial[axis] for axis in range(3)
            )
            print(
                f"  face={index:2d} uv={face.uv} vertices={points} "
                f"normal_dot_radial={normal_dot_radial:.0f}"
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
