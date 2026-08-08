#!/usr/bin/env python3
"""Audit render-control and black geometry in converted V8 vehicle banks.

The report is intentionally source-data based.  It identifies every slot that
references a render group, preserves its hierarchy/flags, and describes pure
black untextured faces without relying on screenshots or runtime tpage values.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


def face_area(vertices: list[list[int]], indices: list[int]) -> float:
    points = [vertices[index] for index in indices]
    if len(points) < 3:
        return 0.0
    origin = points[0]
    area = 0.0
    for index in range(1, len(points) - 1):
        a = [points[index][axis] - origin[axis] for axis in range(3)]
        b = [points[index + 1][axis] - origin[axis] for axis in range(3)]
        cross = (
            a[1] * b[2] - a[2] * b[1],
            a[2] * b[0] - a[0] * b[2],
            a[0] * b[1] - a[1] * b[0],
        )
        area += sum(component * component for component in cross) ** 0.5 / 2.0
    return area


def bounds(vertices: list[list[int]], indices: list[int]) -> dict[str, list[int]]:
    points = [vertices[index] for index in indices]
    return {
        "minimum": [min(point[axis] for point in points) for axis in range(3)],
        "maximum": [max(point[axis] for point in points) for axis in range(3)],
    }


def ancestors(slots: list[dict[str, Any]], index: int) -> list[int]:
    result: list[int] = []
    parent = slots[index].get("parent")
    while parent is not None:
        result.append(parent)
        parent = slots[parent].get("parent")
    return result


def audit_bank(bank: dict[str, Any]) -> dict[str, Any]:
    slots = bank["slots"]
    groups = bank["groups"]
    group_references: dict[int, list[int]] = {}
    for slot_index, slot in enumerate(slots):
        group_index = slot.get("render_group")
        if group_index is not None:
            group_references.setdefault(group_index, []).append(slot_index)

    audited_groups = []
    for group_index, group in enumerate(groups):
        vertices = group["vertices"]
        black_faces = []
        for face_index, face in enumerate(group["faces"]):
            if face.get("texture") is not None or face["color"][:3] != [0, 0, 0]:
                continue
            indices = face["vertices"]
            black_faces.append(
                {
                    "face": face_index,
                    "vertices": indices,
                    "bounds": bounds(vertices, indices),
                    "area": round(face_area(vertices, indices), 3),
                    "packet_kind": face["packet_kind"],
                    "packet_flags": face["packet_flags"],
                }
            )
        references = []
        for slot_index in group_references.get(group_index, []):
            slot = slots[slot_index]
            references.append(
                {
                    "slot": slot_index,
                    "name": slot["name"],
                    "render_flags": f"0x{slot['render_flags']:04X}",
                    "render_class": f"0x{slot['render_flags'] & 0xF000:04X}",
                    "position": slot["position"],
                    "parent": slot.get("parent"),
                    "ancestors": ancestors(slots, slot_index),
                    "direct_body_child": slot.get("parent") == 0,
                }
            )
        audited_groups.append(
            {
                "group": group_index,
                "name": group["name"],
                "face_count": len(group["faces"]),
                "black_untextured_face_count": len(black_faces),
                "black_untextured_faces": black_faces,
                "references": references,
            }
        )

    return {
        "slot_count": len(slots),
        "group_count": len(groups),
        "groups": audited_groups,
        "render_control_slots": [
            {
                "slot": index,
                "name": slot["name"],
                "render_group": slot.get("render_group"),
                "render_flags": f"0x{slot['render_flags']:04X}",
                "render_class": f"0x{slot['render_flags'] & 0xF000:04X}",
                "position": slot["position"],
                "parent": slot.get("parent"),
                "ancestors": ancestors(slots, index),
            }
            for index, slot in enumerate(slots)
            if slot.get("render_group") is not None
            and slot["render_flags"] & 0xF000
        ],
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    paths = sorted(args.source.glob("guest.v8.*.json"))
    if not paths:
        paths = [args.source]
    vehicles = []
    for path in paths:
        document = json.loads(path.read_text(encoding="utf-8"))
        vehicles.append(
            {
                "source": str(path),
                "stable_id": document.get("stable_id", path.stem),
                "body": audit_bank(document),
                "selector_preview": (
                    None
                    if document.get("selector_preview_bank") is None
                    else audit_bank(document["selector_preview_bank"])
                ),
            }
        )
    report = {
        "schema": "v82-imported-vehicle-occluder-audit-v1",
        "vehicle_count": len(vehicles),
        "vehicles": vehicles,
    }
    rendered = json.dumps(report, indent=2) + "\n"
    if args.output is None:
        print(rendered, end="")
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered, encoding="utf-8")


if __name__ == "__main__":
    main()
