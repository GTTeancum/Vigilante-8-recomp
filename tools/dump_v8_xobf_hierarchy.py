#!/usr/bin/env python3
"""Print a native V8 XOBF slot hierarchy with transforms and render groups."""

from __future__ import annotations

import argparse
from pathlib import Path

import v8_n64_level as n64
from blender_addons.vigilante8_vehicle_tools.xobf import Model


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("exp", type=Path)
    parser.add_argument("--bank", type=int, default=0)
    parser.add_argument("--root", type=int, required=True)
    parser.add_argument("--flags", type=lambda value: int(value, 0), default=0)
    args = parser.parse_args()

    bins = []
    for child in n64.root_children(args.exp.read_bytes()):
        if not child.is_form or child.form_type != b"XOBF":
            continue
        nested = n64.form_children(
            n64.iff_form(b"XOBF", [child.payload]), b"XOBF"
        )
        bins.append(next(item.payload for item in nested if item.tag == b"BIN "))
    model = Model(bins[args.bank], "V8")
    slots = model.slots()
    visited: set[int] = set()

    def walk(index: int, walk_flags: int, depth: int) -> None:
        if index == 0xFFFF:
            return
        if index >= len(slots):
            print("  " * depth + f"slot={index} INVALID")
            return
        if index in visited:
            print("  " * depth + f"slot={index} CYCLE")
            return
        visited.add(index)
        slot = slots[index]
        print(
            "  " * depth
            + f"slot={index} key={slot.render_key} group={slot.render_group} "
            + f"obstacle={slot.obstacle_index} "
            + f"pos=({slot.x},{slot.y},{slot.z}) "
            + f"rot_yxz=({slot.rot_y},{slot.rot_x},{slot.rot_z}) "
            + f"flags=0x{slot.flags & 0xFFFF:04X} "
            + f"sibling={slot.next_sibling} child={slot.first_child}"
        )
        structural_stop = (
            slot.render_key < 0
            and (slot.render_key != -1 or (walk_flags & 4) != 0)
        )
        if structural_stop:
            if walk_flags & 1:
                walk(slot.next_sibling, walk_flags, depth)
            return
        if walk_flags & 1:
            walk(slot.next_sibling, walk_flags, depth)
        if not (walk_flags & 2):
            walk(slot.first_child, walk_flags | 1, depth + 1)

    walk(args.root, (args.flags & 4) << 1, 0)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
