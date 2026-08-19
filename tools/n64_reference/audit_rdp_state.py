#!/usr/bin/env python3
"""Audit authored F3DEX2 depth/render state in a V8 N64 arena.

The native PS1 converter intentionally reduces N64 display-list state.  This
tool records the state active at every authored triangle so omissions such as
Z compare, Z update, and decal/interpenetrating Z mode cannot hide behind a
geometry/UV-only conversion pass.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from dataclasses import dataclass
import json
from pathlib import Path
import struct
import sys
from typing import Any


TOOLS = Path(__file__).resolve().parents[1]
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import v8_n64_level as n64  # noqa: E402


AA_EN = 0x0008
Z_CMP = 0x0010
Z_UPD = 0x0020
IM_RD = 0x0040
ZMODE_MASK = 0x0C00
ZMODE_NAMES = {
    0x0000: "opaque",
    0x0400: "interpenetrating",
    0x0800: "translucent",
    0x0C00: "decal",
}
G_ZBUFFER = 0x00000001
G_FOG = 0x00010000
G_LIGHTING = 0x00020000


@dataclass(frozen=True)
class TriangleState:
    bank: int
    group: int
    owner: tuple[str, ...]
    display: str
    command_offset: int
    triangle_in_command: int
    vertex_offsets: tuple[int, int, int]
    positions: tuple[tuple[int, int, int], ...]
    vertex_colors: tuple[tuple[int, int, int, int], ...]
    other_mode_l: int
    other_mode_h: int
    geometry_mode: int
    z_compare: bool
    z_update: bool
    z_buffer_geometry: bool | None
    fog_geometry: bool | None
    lighting_geometry: bool | None
    z_mode: str
    aa: bool
    image_read: bool
    render_mode_low16: int


def xobf_bins(exp: bytes) -> list[bytes]:
    bins: list[bytes] = []
    for child in n64.root_children(exp):
        if not child.is_form or child.form_type != b"XOBF":
            continue
        nested = n64.form_children(
            n64.iff_form(b"XOBF", [child.payload]), b"XOBF"
        )
        bins.append(next(item.payload for item in nested if item.tag == b"BIN "))
    return bins


def object_heads(exp: bytes) -> list[tuple[str, int, int, int]]:
    heads: list[tuple[str, int, int, int]] = []
    for _offset, tag, payload, parent in n64.iter_chunks(exp):
        if tag != b"HEAD" or parent != b"OBJ " or len(payload) < 34:
            continue
        heads.append(
            (
                payload[34:].split(b"\0", 1)[0].decode("ascii", "replace"),
                n64.be16(payload, 26, signed=True),
                n64.be16(payload, 28, signed=True),
                n64.be32(payload, 4),
            )
        )
    return heads


def source_group_owners(
    exp: bytes, bins: list[bytes]
) -> dict[tuple[int, int], set[str]]:
    owners: dict[tuple[int, int], set[str]] = defaultdict(set)
    for name, bank, root, flags in object_heads(exp):
        if bank < 0 or bank >= len(bins):
            continue
        data = bins[bank]
        slot_count = n64.be32(data, 0x18)
        seen: set[int] = set()

        def walk(slot: int, walk_flags: int, depth: int) -> None:
            if slot < 0 or slot >= slot_count or slot in seen or depth > 256:
                return
            seen.add(slot)
            offset = 0x1C + slot * 0x1C
            native = n64.convert_slot(data[offset:offset + 0x1C])
            key = struct.unpack_from("<h", native, 0)[0]
            sibling = struct.unpack_from("<h", native, 24)[0]
            child = struct.unpack_from("<h", native, 26)[0]
            if key < 0 and (key != -1 or (walk_flags & 4) != 0):
                if (key & 0xF000) == 0xC000:
                    owners[(bank, key & 0x07FF)].add(name)
                if (walk_flags & 1) and sibling != -1:
                    walk(sibling, walk_flags, depth + 1)
                return
            if key >= 0:
                owners[(bank, key & 0x07FF)].add(name)
            if (walk_flags & 1) and sibling != -1:
                walk(sibling, walk_flags, depth + 1)
            if not (walk_flags & 2) and child != -1:
                walk(child, walk_flags | 1, depth + 1)

        walk(root, (flags & 4) << 1, 0)
    return owners


def update_other_mode(current: int, word0: int, word1: int) -> int:
    size = (word0 & 0xFF) + 1
    offset = max(0, 32 - ((word0 >> 8) & 0xFF) - size)
    mask = ((1 << size) - 1) << offset
    return (current & ~mask) | (word1 & mask)


def state_record(
    bank: int,
    group: int,
    owners: dict[tuple[int, int], set[str]],
    display: str,
    command_offset: int,
    command_triangle: int,
    indices: tuple[int, int, int],
    cache: dict[int, n64.Vertex],
    other_l: int,
    other_h: int,
    geometry: int,
    geometry_known: int,
) -> TriangleState:
    vertices = tuple(cache[index] for index in indices)
    return TriangleState(
        bank=bank,
        group=group,
        owner=tuple(sorted(owners.get((bank, group), ()))),
        display=display,
        command_offset=command_offset,
        triangle_in_command=command_triangle,
        vertex_offsets=tuple(vertex.source_offset for vertex in vertices),
        positions=tuple((vertex.x, vertex.y, vertex.z) for vertex in vertices),
        vertex_colors=tuple(vertex.color for vertex in vertices),
        other_mode_l=other_l,
        other_mode_h=other_h,
        geometry_mode=geometry,
        z_compare=bool(other_l & Z_CMP),
        z_update=bool(other_l & Z_UPD),
        z_buffer_geometry=(
            bool(geometry & G_ZBUFFER)
            if geometry_known & G_ZBUFFER
            else None
        ),
        fog_geometry=(
            bool(geometry & G_FOG)
            if geometry_known & G_FOG
            else None
        ),
        lighting_geometry=(
            bool(geometry & G_LIGHTING)
            if geometry_known & G_LIGHTING
            else None
        ),
        z_mode=ZMODE_NAMES[other_l & ZMODE_MASK],
        aa=bool(other_l & AA_EN),
        image_read=bool(other_l & IM_RD),
        render_mode_low16=other_l & 0xFFFF,
    )


def trace_group(
    data: bytes,
    bank: int,
    group: int,
    group_offset: int,
    owners: dict[tuple[int, int], set[str]],
) -> tuple[list[TriangleState], list[str]]:
    result: list[TriangleState] = []
    warnings: list[str] = []
    vertex_relative = n64.be32(data, group_offset + 4)
    vertex_base = group_offset + vertex_relative
    for display_name, relative in (
        ("opaque", n64.be32(data, group_offset + 8)),
        ("alternate", n64.be32(data, group_offset + 12)),
    ):
        if relative == 0:
            continue
        cursor = group_offset + relative
        cache: dict[int, n64.Vertex] = {}
        other_l = 0
        other_h = 0
        geometry = 0
        geometry_known = 0
        saw_other_l = False
        saw_geometry = False
        for _ in range(0x10000):
            if cursor + 8 > len(data):
                raise n64.FormatError(
                    f"bank {bank} group {group} display list is truncated"
                )
            command_offset = cursor
            command = data[cursor:cursor + 8]
            opcode = command[0]
            word0 = n64.be32(command, 0)
            word1 = n64.be32(command, 4)
            cursor += 8
            if opcode == 0xDF:
                break
            if opcode == 0xE2:
                other_l = update_other_mode(other_l, word0, word1)
                saw_other_l = True
                continue
            if opcode == 0xE3:
                other_h = update_other_mode(other_h, word0, word1)
                continue
            if opcode == 0xD9:
                keep = word0 & 0x00FFFFFF
                geometry = (geometry & keep) | word1
                geometry_known = (
                    (geometry_known & keep)
                    | ((~keep) & 0x00FFFFFF)
                    | word1
                )
                saw_geometry = True
                continue
            if opcode == 0x01:
                count = (word0 >> 12) & 0xFF
                end_index = (word0 >> 1) & 0x7F
                first_index = end_index - count
                source = vertex_base + (word1 & 0xFFFFFF)
                for item in range(count):
                    offset = source + item * 16
                    cache[first_index + item] = n64.Vertex(
                        x=n64.be16(data, offset, signed=True),
                        y=n64.be16(data, offset + 2, signed=True),
                        z=n64.be16(data, offset + 4, signed=True),
                        s=n64.be16(data, offset + 8, signed=True),
                        t=n64.be16(data, offset + 10, signed=True),
                        color=tuple(data[offset + 12:offset + 16]),
                        source_offset=offset,
                    )
                continue
            triangles: tuple[tuple[int, int, int], ...] = ()
            if opcode == 0x05:
                triangles = (
                    (command[1] >> 1, command[2] >> 1, command[3] >> 1),
                )
            elif opcode == 0x06:
                triangles = (
                    (command[1] >> 1, command[2] >> 1, command[3] >> 1),
                    (command[5] >> 1, command[6] >> 1, command[7] >> 1),
                )
            for triangle_index, indices in enumerate(triangles):
                missing = [index for index in indices if index not in cache]
                if missing:
                    raise n64.FormatError(
                        f"bank {bank} group {group} missing cache {missing}"
                    )
                if not saw_other_l:
                    warnings.append(
                        f"bank={bank} group={group} display={display_name} "
                        f"triangle=0x{command_offset:X} inherits other_mode_l"
                    )
                if not saw_geometry:
                    warnings.append(
                        f"bank={bank} group={group} display={display_name} "
                        f"triangle=0x{command_offset:X} inherits geometry_mode"
                    )
                elif not (geometry_known & G_ZBUFFER):
                    warnings.append(
                        f"bank={bank} group={group} display={display_name} "
                        f"triangle=0x{command_offset:X} inherits G_ZBUFFER"
                    )
                result.append(
                    state_record(
                        bank,
                        group,
                        owners,
                        display_name,
                        command_offset,
                        triangle_index,
                        indices,
                        cache,
                        other_l,
                        other_h,
                        geometry,
                        geometry_known,
                    )
                )
        else:
            raise n64.FormatError(
                f"bank {bank} group {group} has no G_ENDDL"
            )
    return result, warnings


def audit(exp: bytes) -> dict[str, Any]:
    bins = xobf_bins(exp)
    owners = source_group_owners(exp, bins)
    triangles: list[TriangleState] = []
    warnings: list[str] = []
    for bank, data in enumerate(bins):
        group_count = n64.be32(data, 0)
        group_table = n64.be32(data, 4)
        for group in range(group_count):
            group_offset = group_table + n64.be32(
                data, group_table + group * 4
            )
            group_triangles, group_warnings = trace_group(
                data, bank, group, group_offset, owners
            )
            triangles.extend(group_triangles)
            warnings.extend(group_warnings)

    state_counts = Counter(
        (
            triangle.z_compare,
            triangle.z_update,
            triangle.z_buffer_geometry,
            triangle.z_mode,
            triangle.aa,
            triangle.image_read,
            triangle.render_mode_low16,
        )
        for triangle in triangles
    )
    group_counts: Counter[tuple[int, int, tuple[str, ...], str]] = Counter()
    for triangle in triangles:
        if (
            triangle.z_mode != "opaque"
            or not triangle.z_compare
            or not triangle.z_update
            or triangle.z_buffer_geometry is False
        ):
            group_counts[
                (
                    triangle.bank,
                    triangle.group,
                    triangle.owner,
                    triangle.z_mode,
                )
            ] += 1

    chunks = Counter(
        tag.decode("ascii", "replace")
        for _offset, tag, _payload, _parent in n64.iter_chunks(exp)
    )
    return {
        "summary": {
            "xobf_banks": len(bins),
            "triangles": len(triangles),
            "unique_triangle_states": len(state_counts),
            "warnings": len(set(warnings)),
            "chunk_counts": dict(sorted(chunks.items())),
            "fogged_triangles": sum(
                triangle.fog_geometry is True for triangle in triangles
            ),
            "lit_triangles": sum(
                triangle.lighting_geometry is True for triangle in triangles
            ),
        },
        "state_counts": [
            {
                "triangles": count,
                "z_compare": key[0],
                "z_update": key[1],
                "z_buffer_geometry": key[2],
                "z_mode": key[3],
                "aa": key[4],
                "image_read": key[5],
                "render_mode_low16": f"0x{key[6]:04X}",
            }
            for key, count in sorted(
                state_counts.items(), key=lambda item: (-item[1], item[0])
            )
        ],
        "reduced_depth_groups": [
            {
                "bank": key[0],
                "group": key[1],
                "owners": list(key[2]),
                "z_mode": key[3],
                "triangles": count,
            }
            for key, count in sorted(group_counts.items())
        ],
        "warnings": sorted(set(warnings)),
        "triangles": [
            {
                **triangle.__dict__,
                "other_mode_l": f"0x{triangle.other_mode_l:08X}",
                "other_mode_h": f"0x{triangle.other_mode_h:08X}",
                "geometry_mode": f"0x{triangle.geometry_mode:08X}",
                "render_mode_low16": f"0x{triangle.render_mode_low16:04X}",
            }
            for triangle in triangles
        ],
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path, nargs="?")
    parser.add_argument("arena")
    parser.add_argument(
        "--source",
        type=Path,
        help="existing decoded N64 arena EXP; bypasses ROM extraction",
    )
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    if args.source:
        exp = args.source.read_bytes()
    elif args.rom:
        exp = n64.V8N64Rom(args.rom).decoded(f"{args.arena.upper()}.EXP")
    else:
        parser.error("ROM is required unless --source is supplied")
    report = audit(exp)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    summary = report["summary"]
    print(
        f"{args.arena.upper()}: triangles={summary['triangles']} "
        f"states={summary['unique_triangle_states']} "
        f"inherited-state-warnings={summary['warnings']}"
    )
    print(args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
