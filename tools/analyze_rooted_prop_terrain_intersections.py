#!/usr/bin/env python3
"""Measure placed XOBF triangles against an arena's authored terrain.

The report follows the same OBJ/HEAD root replacement and YXZ slot hierarchy
used by the retail object constructor, transforms every referenced triangle to
world 16.16 coordinates, and samples the unchanged ZMAP/ZONE height field at
each vertex.  It distinguishes meshes which merely span local Y=0 from faces
which actually cross or sit below the placed terrain surface.

This is deliberately identity-neutral: names are optional report filters and
never participate in classification.
"""

from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import dataclass
import fnmatch
import math
from pathlib import Path
import struct
import sys


REPO = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO / "tools"))
sys.path.insert(0, str(REPO / "tools" / "blender_addons"))

import v8_n64_level as terrain
from vigilante8_vehicle_tools import iff, xobf


FIXED = 65536.0
HEAD_Y_BIAS = 0x100000


@dataclass(frozen=True)
class Transform:
    rotation: tuple[tuple[float, float, float], ...]
    position: tuple[float, float, float]


@dataclass(frozen=True)
class Head:
    name: str
    kind: int
    flags: int
    position: tuple[int, int, int]
    rotation_yxz: tuple[int, int, int]
    bank: int
    slot: int


def _matrix_yxz(rotation: tuple[int, int, int]) -> tuple[tuple[float, float, float], ...]:
    """Float expression of the retail RotMatrixYXZ_gte coefficient order."""

    x, y, z = (value * math.tau / 4096.0 for value in rotation)
    sx, cx = math.sin(x), math.cos(x)
    sy, cy = math.sin(y), math.cos(y)
    sz, cz = math.sin(z), math.cos(z)
    return (
        (cy * cz + sz * sy * sx, cz * sy * sx - cy * sz, cx * sy),
        (sz * cx, cz * cx, -sx),
        (sz * cy * sx - sy * cz, cz * cy * sx + sy * sz, cx * cy),
    )


def _mul_matrix(
    left: tuple[tuple[float, float, float], ...],
    right: tuple[tuple[float, float, float], ...],
) -> tuple[tuple[float, float, float], ...]:
    return tuple(
        tuple(sum(left[row][item] * right[item][column] for item in range(3))
              for column in range(3))
        for row in range(3)
    )


def _rotate(
    matrix: tuple[tuple[float, float, float], ...],
    vector: tuple[float, float, float],
) -> tuple[float, float, float]:
    return tuple(
        sum(matrix[row][column] * vector[column] for column in range(3))
        for row in range(3)
    )  # type: ignore[return-value]


def _compose(parent: Transform, local: Transform) -> Transform:
    offset = _rotate(parent.rotation, local.position)
    return Transform(
        rotation=_mul_matrix(parent.rotation, local.rotation),
        position=tuple(parent.position[index] + offset[index] for index in range(3)),
    )


def _heads(document: iff.IffDocument) -> tuple[Head, ...]:
    result = []
    for form in document.forms(b"OBJ "):
        payload = next(
            (child.payload for child in form.children if child.tag == b"HEAD"),
            None,
        )
        if payload is None or len(payload) < 34:
            continue
        result.append(
            Head(
                name=payload[34:].split(b"\0", 1)[0].decode("ascii", "replace"),
                kind=payload[1],
                flags=struct.unpack_from(">I", payload, 4)[0],
                position=struct.unpack_from(">iii", payload, 8),
                rotation_yxz=struct.unpack_from(">hhh", payload, 20),
                bank=struct.unpack_from(">h", payload, 26)[0],
                slot=struct.unpack_from(">h", payload, 28)[0],
            )
        )
    return tuple(result)


def _models(document: iff.IffDocument, dialect: str) -> tuple[xobf.Model, ...]:
    models = []
    for form in document.forms(b"XOBF"):
        bins = [child.payload for child in form.children if child.tag == b"BIN "]
        if len(bins) != 1:
            raise ValueError("XOBF does not contain exactly one BIN chunk")
        models.append(xobf.Model(bins[0], dialect=dialect))
    return tuple(models)


def _terrain_payloads(document: iff.IffDocument) -> tuple[bytes, list[bytes]]:
    root = next(document.forms(b"TERR"), None)
    if root is None:
        raise ValueError("EXP does not contain FORM TERR")
    zmap = next((child.payload for child in root.children if child.tag == b"ZMAP"), None)
    zones = [child.payload for child in root.children if child.tag == b"ZONE"]
    if zmap is None or not zones:
        raise ValueError("arena does not contain ZMAP/ZONE terrain")
    return zmap, zones


def _slot_nodes(model: xobf.Model, head: Head) -> tuple[tuple[xobf.Slot, Transform], ...]:
    slots = model.slots()
    if head.slot < 0 or head.slot >= len(slots):
        return ()
    root = Transform(
        rotation=_matrix_yxz(head.rotation_yxz),
        position=(
            float(head.position[0]),
            float(head.position[1] - HEAD_Y_BIAS),
            float(head.position[2]),
        ),
    )
    result: list[tuple[xobf.Slot, Transform]] = []
    seen: set[int] = set()

    def visit(index: int, parent: Transform, include_sibling: bool) -> None:
        if index == 0xFFFF or index >= len(slots) or index in seen:
            return
        seen.add(index)
        slot = slots[index]
        transform = parent if index == head.slot else _compose(
            parent,
            Transform(
                rotation=_matrix_yxz((slot.rot_y, slot.rot_x, slot.rot_z)),
                position=(float(slot.x), float(slot.y), float(slot.z)),
            ),
        )
        result.append((slot, transform))
        if slot.first_child != 0xFFFF:
            visit(slot.first_child, transform, True)
        if include_sibling and slot.next_sibling != 0xFFFF:
            visit(slot.next_sibling, parent, True)

    visit(head.slot, root, False)
    return tuple(result)


def _triangle_state(distances: tuple[float, float, float], epsilon: float) -> str:
    above = any(value < -epsilon for value in distances)
    below = any(value > epsilon for value in distances)
    if above and below:
        return "crossing"
    if below:
        return "below"
    if above:
        return "above"
    return "surface"


def _instance_counts(
    model: xobf.Model,
    head: Head,
    zmap: bytes,
    zones: list[bytes],
    epsilon: float,
) -> tuple[Counter[str], Counter[int], float, float]:
    counts: Counter[str] = Counter()
    crossing_groups: Counter[int] = Counter()
    minimum = math.inf
    maximum = -math.inf
    for slot, transform in _slot_nodes(model, head):
        group_index = slot.render_group
        if group_index is None or group_index >= model.group_count:
            continue
        group = model.group(group_index)
        local_scale = FIXED / float(1 << group.scale_shift)
        world_vertices = []
        distances = []
        for vertex in group.vertices:
            local = (
                vertex.x * local_scale,
                vertex.y * local_scale,
                vertex.z * local_scale,
            )
            rotated = _rotate(transform.rotation, local)
            world = tuple(transform.position[index] + rotated[index] for index in range(3))
            world_vertices.append(world)
            height = terrain._terrain_height_from_source(
                round(world[0]), round(world[2]), zmap, zones
            )
            distance = world[1] - height
            distances.append(distance)
            minimum = min(minimum, distance)
            maximum = max(maximum, distance)
        for packet in group.packets:
            if packet.kind in xobf.NON_VERTEX_PACKET_KINDS:
                continue
            face_distances = tuple(distances[index] for index in packet.vertex_indices)
            state = _triangle_state(face_distances, epsilon)
            counts[state] += 1
            if state in {"crossing", "below"}:
                crossing_groups[group_index] += 1
    if math.isinf(minimum):
        minimum = maximum = 0.0
    return counts, crossing_groups, minimum, maximum


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exp", type=Path)
    parser.add_argument("--dialect", choices=("V8", "V8_2"), required=True)
    parser.add_argument(
        "--name",
        action="append",
        default=[],
        help="case-insensitive OBJ name glob; may be repeated",
    )
    parser.add_argument(
        "--epsilon",
        type=float,
        default=256.0,
        help="terrain-distance tolerance in 16.16 units (default: 256)",
    )
    parser.add_argument(
        "--problems-only",
        action="store_true",
        help="omit instances with no crossing or below-terrain triangles",
    )
    args = parser.parse_args()

    document = iff.parse(args.exp.read_bytes())
    models = _models(document, args.dialect)
    zmap, zones = _terrain_payloads(document)
    totals: Counter[str] = Counter()
    reported = 0
    considered = 0
    for occurrence, head in enumerate(_heads(document)):
        if args.name and not any(
            fnmatch.fnmatchcase(head.name.casefold(), pattern.casefold())
            for pattern in args.name
        ):
            continue
        if head.bank < 0 or head.bank >= len(models) or head.slot < 0:
            continue
        considered += 1
        counts, groups, minimum, maximum = _instance_counts(
            models[head.bank], head, zmap, zones, args.epsilon
        )
        totals.update(counts)
        if args.problems_only and not (counts["crossing"] or counts["below"]):
            continue
        reported += 1
        group_text = ",".join(f"{index}:{count}" for index, count in sorted(groups.items()))
        print(
            f"OBJECT occurrence={occurrence} name={head.name!r} type={head.kind} "
            f"flags=0x{head.flags:08X} bank={head.bank} slot={head.slot} "
            f"above={counts['above']} surface={counts['surface']} "
            f"crossing={counts['crossing']} below={counts['below']} "
            f"distance16=[{minimum:.1f},{maximum:.1f}] groups={group_text or '-'}"
        )
    print(
        f"SUMMARY considered={considered} reported={reported} "
        f"above={totals['above']} surface={totals['surface']} "
        f"crossing={totals['crossing']} below={totals['below']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
