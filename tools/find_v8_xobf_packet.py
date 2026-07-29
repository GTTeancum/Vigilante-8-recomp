#!/usr/bin/env python3
"""Resolve a native V8 XOBF polygon packet to its bank, group, and OBJ owners."""

from __future__ import annotations

import argparse
from collections import defaultdict
from pathlib import Path

import v8_n64_level as n64
from blender_addons.vigilante8_vehicle_tools.xobf import Model


def xobf_bins(exp: bytes) -> list[bytes]:
    result: list[bytes] = []
    for child in n64.root_children(exp):
        if not child.is_form or child.form_type != b"XOBF":
            continue
        nested = n64.form_children(
            n64.iff_form(b"XOBF", [child.payload]), b"XOBF"
        )
        result.append(next(item.payload for item in nested if item.tag == b"BIN "))
    return result


def object_heads(exp: bytes) -> list[tuple[str, int, int, int]]:
    result: list[tuple[str, int, int, int]] = []
    for _offset, tag, payload, parent in n64.iter_chunks(exp):
        if tag != b"HEAD" or parent != b"OBJ " or len(payload) < 34:
            continue
        result.append(
            (
                payload[34:].split(b"\0", 1)[0].decode("ascii", "replace"),
                n64.be16(payload, 26, signed=True),
                n64.be16(payload, 28, signed=True),
                n64.be32(payload, 4),
            )
        )
    return result


def group_owners(
    exp: bytes, models: list[Model]
) -> dict[tuple[int, int], set[str]]:
    result: dict[tuple[int, int], set[str]] = defaultdict(set)
    for name, bank, root, object_flags in object_heads(exp):
        if bank < 0 or bank >= len(models):
            continue
        model = models[bank]
        slots = model.slots()
        seen: set[int] = set()

        def walk(slot_index: int, walk_flags: int, depth: int) -> None:
            if (
                slot_index < 0
                or slot_index >= len(slots)
                or slot_index in seen
                or depth > 256
            ):
                return
            seen.add(slot_index)
            slot = slots[slot_index]
            key = slot.render_key
            sibling = (
                -1 if slot.next_sibling == 0xFFFF else slot.next_sibling
            )
            child = -1 if slot.first_child == 0xFFFF else slot.first_child

            if key < 0 and (key != -1 or (walk_flags & 4) != 0):
                group = slot.render_group
                if group is not None and group < model.group_count:
                    result[(bank, group)].add(name)
                if walk_flags & 1 and sibling != -1:
                    walk(sibling, walk_flags, depth + 1)
                return

            group = slot.render_group
            if group is not None and group < model.group_count:
                result[(bank, group)].add(name)
            if walk_flags & 1 and sibling != -1:
                walk(sibling, walk_flags, depth + 1)
            if not (walk_flags & 2) and child != -1:
                walk(child, walk_flags | 1, depth + 1)

        walk(root, (object_flags & 4) << 1, 0)
    return result


def parse_uv(value: str) -> tuple[tuple[int, int], ...]:
    result = []
    for pair in value.split(";"):
        u, v = pair.split(",", 1)
        result.append((int(u), int(v)))
    if len(result) not in {3, 4}:
        raise argparse.ArgumentTypeError("UV requires three or four U,V pairs")
    return tuple(result)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exp", type=Path)
    parser.add_argument("--uv", required=True, type=parse_uv)
    parser.add_argument(
        "--normalize",
        action="store_true",
        help="compare UV shape after subtracting the first pair",
    )
    args = parser.parse_args()

    exp = args.exp.read_bytes()
    bins = xobf_bins(exp)
    models = [Model(data, "V8") for data in bins]
    owners = group_owners(exp, models)
    matches = 0
    for bank, model in enumerate(models):
        for group_index in range(model.group_count):
            group = model.group(group_index)
            for packet_index, packet in enumerate(group.packets):
                actual_uv = packet.uv
                expected_uv = args.uv
                if args.normalize and actual_uv and expected_uv:
                    actual_uv = tuple(
                        (u - actual_uv[0][0], v - actual_uv[0][1])
                        for u, v in actual_uv
                    )
                    expected_uv = tuple(
                        (u - expected_uv[0][0], v - expected_uv[0][1])
                        for u, v in expected_uv
                    )
                if actual_uv != expected_uv:
                    continue
                matches += 1
                names = ",".join(sorted(owners.get((bank, group_index), ())))
                vertices = [
                    group.vertices[index]
                    for index in packet.vertex_indices
                ]
                coordinates = ";".join(
                    f"{vertex.x},{vertex.y},{vertex.z}" for vertex in vertices
                )
                print(
                    f"bank={bank} group={group_index} packet={packet_index} "
                    f"kind={packet.kind} texture={packet.texture_slot} "
                    f"indices={','.join(str(index) for index in packet.vertex_indices)} "
                    f"vertices={coordinates} scale_shift={group.scale_shift} "
                    f"owners={names or '-'}"
                )
    print(f"matches={matches}")
    return 0 if matches else 1


if __name__ == "__main__":
    raise SystemExit(main())
