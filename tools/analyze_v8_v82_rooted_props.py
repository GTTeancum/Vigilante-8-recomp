#!/usr/bin/env python3
"""Compare rooted static-prop model semantics in retail V8 arena ports.

The report matches OBJ/HEAD records by authored name and occurrence, then
follows each record's native root hierarchy into its XOBF groups.  It is
intended to expose the data conversions used for vegetation or other models
which cross the terrain plane without relying on screenshots or map-specific
names in the converter.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from dataclasses import dataclass
import fnmatch
from pathlib import Path
import struct
import sys


REPO = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO / "tools" / "blender_addons"))

from vigilante8_vehicle_tools import iff, xobf


@dataclass(frozen=True)
class Head:
    name: str
    flags: int
    position: tuple[int, int, int]
    bank: int
    slot: int


def _heads(document: iff.IffDocument) -> dict[str, list[Head]]:
    result: dict[str, list[Head]] = defaultdict(list)
    for node in document.walk():
        if node.tag != b"HEAD" or len(node.payload) < 34:
            continue
        name = node.payload[34:].split(b"\0", 1)[0].decode("ascii", "replace")
        result[name].append(
            Head(
                name=name,
                flags=struct.unpack_from(">I", node.payload, 4)[0],
                position=struct.unpack_from(">iii", node.payload, 8),
                bank=struct.unpack_from(">h", node.payload, 26)[0],
                slot=struct.unpack_from(">h", node.payload, 28)[0],
            )
        )
    return result


def _models(document: iff.IffDocument, dialect: str) -> tuple[xobf.Model, ...]:
    result = []
    for form in document.forms(b"XOBF"):
        bins = [child.payload for child in form.children if child.tag == b"BIN "]
        if len(bins) != 1:
            raise ValueError("XOBF does not contain exactly one BIN chunk")
        result.append(xobf.Model(bins[0], dialect=dialect))
    return tuple(result)


def _root_groups(model: xobf.Model, root: int) -> tuple[int, ...]:
    slots = model.slots()
    if root < 0 or root >= len(slots):
        return ()
    groups: set[int] = set()
    seen: set[int] = set()

    def visit(index: int, include_sibling: bool) -> None:
        if index == 0xFFFF or index >= len(slots) or index in seen:
            return
        seen.add(index)
        slot = slots[index]
        if (
            slot.render_group is not None
            and slot.render_group < model.group_count
        ):
            groups.add(slot.render_group)
        if slot.first_child != 0xFFFF:
            visit(slot.first_child, True)
        if include_sibling and slot.next_sibling != 0xFFFF:
            visit(slot.next_sibling, True)

    visit(root, False)
    return tuple(sorted(groups))


def _group_signature(model: xobf.Model, indices: tuple[int, ...]) -> str:
    pieces = []
    for index in indices:
        group = model.group(index)
        if group.vertices:
            axes = tuple(
                tuple(getattr(vertex, component) for vertex in group.vertices)
                for component in ("x", "y", "z")
            )
            bounds = tuple((min(axis), max(axis)) for axis in axes)
        else:
            bounds = ((0, 0), (0, 0), (0, 0))
        kinds = Counter(packet.kind for packet in group.packets)
        flags = Counter(packet.raw[3] & 0xF0 for packet in group.packets)
        pieces.append(
            f"g{index}:shift={group.scale_shift}:extent={group.render_extent}:"
            f"bbox={bounds}:kinds={dict(sorted(kinds.items()))}:"
            f"flags={dict(sorted(flags.items()))}"
        )
    return ";".join(pieces) if pieces else "no-render-group"


def _physical_y_bounds(model: xobf.Model, indices: tuple[int, ...]) -> tuple[float, float] | None:
    values: list[float] = []
    for index in indices:
        group = model.group(index)
        scale = float(1 << group.scale_shift)
        values.extend(vertex.y / scale for vertex in group.vertices)
    return (min(values), max(values)) if values else None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("v8", type=Path)
    parser.add_argument("v82", type=Path)
    parser.add_argument(
        "--left-dialect",
        choices=("V8", "V8_2"),
        default="V8",
        help="packet dialect for the left EXP (default: V8)",
    )
    parser.add_argument(
        "--right-dialect",
        choices=("V8", "V8_2"),
        default="V8_2",
        help="packet dialect for the right EXP (default: V8_2)",
    )
    parser.add_argument(
        "--ground-crossing-only",
        action="store_true",
        help="report only source roots with vertices on both sides of local Y=0",
    )
    parser.add_argument(
        "--packet-kind",
        type=int,
        help="report only roots containing this source packet kind",
    )
    parser.add_argument(
        "--name",
        action="append",
        default=[],
        help="case-insensitive OBJ name glob; may be repeated",
    )
    args = parser.parse_args()

    left_doc = iff.parse(args.v8.read_bytes())
    right_doc = iff.parse(args.v82.read_bytes())
    left_heads = _heads(left_doc)
    right_heads = _heads(right_doc)
    left_models = _models(left_doc, args.left_dialect)
    right_models = _models(right_doc, args.right_dialect)

    matched = 0
    reported = 0
    for name in sorted(left_heads.keys() & right_heads.keys()):
        if args.name and not any(
            fnmatch.fnmatchcase(name.casefold(), pattern.casefold())
            for pattern in args.name
        ):
            continue
        for occurrence, (left, right) in enumerate(
            zip(left_heads[name], right_heads[name])
        ):
            matched += 1
            if not (0 <= left.bank < len(left_models)) or not (
                0 <= right.bank < len(right_models)
            ):
                continue
            left_model = left_models[left.bank]
            right_model = right_models[right.bank]
            left_groups = _root_groups(left_model, left.slot)
            right_groups = _root_groups(right_model, right.slot)
            y_bounds = _physical_y_bounds(left_model, left_groups)
            if args.ground_crossing_only and not (
                y_bounds is not None and y_bounds[0] < 0 < y_bounds[1]
            ):
                continue
            if args.packet_kind is not None and not any(
                packet.kind == args.packet_kind
                for index in left_groups
                for packet in left_model.group(index).packets
            ):
                continue
            reported += 1
            suffix = f"[{occurrence}]" if len(left_heads[name]) > 1 else ""
            print(
                f"OBJECT {name}{suffix} flags=0x{left.flags:08X}->0x{right.flags:08X} "
                f"headY={left.position[1]}->{right.position[1]} "
                f"bank/slot={left.bank}/{left.slot}->{right.bank}/{right.slot} "
                f"sourceY={y_bounds}"
            )
            print("  V8  " + _group_signature(left_model, left_groups))
            print("  V82 " + _group_signature(right_model, right_groups))

    print(f"SUMMARY matched={matched} reported={reported}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
