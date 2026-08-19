#!/usr/bin/env python3
"""Audit N64 model display-list state before native PS1 conversion.

The converter deliberately reduces F3DEX2/RDP state to the fields expressible
by Vigilante 8's PS1 XOBF packets.  This audit makes that reduction explicit
per render group and correlates groups with the authored OBJ/HEAD names, so a
visually suspect building can be investigated without relying on screenshots
or guessed model roles.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from pathlib import Path
import struct

from blender_addons.vigilante8_vehicle_tools.xobf import Model, TextureSlot
import v8_n64_level as n64


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
        name = payload[34:].split(b"\0", 1)[0].decode("ascii", "replace")
        heads.append(
            (
                name,
                n64.be16(payload, 26, signed=True),
                n64.be16(payload, 28, signed=True),
                n64.be32(payload, 4),
            )
        )
    return heads


def object_head_payloads(exp: bytes) -> list[bytes]:
    return [
        payload
        for _offset, tag, payload, parent in n64.iter_chunks(exp)
        if tag == b"HEAD" and parent == b"OBJ "
    ]


def source_group_owners(exp: bytes, bins: list[bytes]) -> dict[tuple[int, int], set[str]]:
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
            next_sibling = struct.unpack_from("<h", native, 24)[0]
            first_child = struct.unpack_from("<h", native, 26)[0]
            if key < 0 and (key != -1 or (walk_flags & 4) != 0):
                if (key & 0xF000) == 0xC000:
                    owners[(bank, key & 0x07FF)].add(name)
                if (walk_flags & 1) and next_sibling != -1:
                    walk(next_sibling, walk_flags, depth + 1)
                return
            if key >= 0:
                owners[(bank, key & 0x07FF)].add(name)
            if (walk_flags & 1) and next_sibling != -1:
                walk(next_sibling, walk_flags, depth + 1)
            if not (walk_flags & 2) and first_child != -1:
                walk(first_child, walk_flags | 1, depth + 1)

        walk(root, (flags & 4) << 1, 0)
    return owners


def display_commands(data: bytes, group_offset: int) -> Counter[tuple[int, int, int]]:
    result: Counter[tuple[int, int, int]] = Counter()
    for relative in (
        n64.be32(data, group_offset + 8),
        n64.be32(data, group_offset + 12),
    ):
        if relative == 0:
            continue
        cursor = group_offset + relative
        for _ in range(0x10000):
            opcode = data[cursor]
            word0 = n64.be32(data, cursor)
            word1 = n64.be32(data, cursor + 4)
            cursor += 8
            result[(opcode, word0, word1)] += 1
            if opcode == 0xDF:
                break
        else:
            raise n64.FormatError(
                f"group at 0x{group_offset:X} has no G_ENDDL"
            )
    return result


def target_group(
    data: bytes, group: int
) -> tuple[list[tuple[int, int, int]], int, int]:
    group_count, group_table = struct.unpack_from("<II", data, 0)
    if group < 0 or group >= group_count:
        raise n64.FormatError(f"converted group {group} is missing")
    relative = struct.unpack_from("<I", data, group_table + group * 4)[0]
    offset = group_table + relative
    vertex_count = struct.unpack_from("<I", data, offset)[0]
    vertex_relative = struct.unpack_from("<I", data, offset + 4)[0]
    polygon_count = struct.unpack_from("<H", data, offset + 0x10)[0]
    scale = data[offset + 0x18]
    vertices = [
        struct.unpack_from("<hhh", data, offset + vertex_relative + index * 8)
        for index in range(vertex_count)
    ]
    return vertices, polygon_count, scale


def n64_rgb5(value: int) -> tuple[int, int, int]:
    return (value >> 11) & 0x1F, (value >> 6) & 0x1F, (value >> 1) & 0x1F


def psx_rgb5(value: int) -> tuple[int, int, int]:
    return value & 0x1F, (value >> 5) & 0x1F, (value >> 10) & 0x1F


def source_texture_rgb(texture: n64.N64Texture) -> list[tuple[int, int, int]]:
    """Decode authored visible texels to the RGB precision represented by PS1."""

    cutout = n64.texture_has_nonblack_alpha_cutout(texture)

    def visible_rgb(value: int) -> tuple[int, int, int]:
        if cutout and not (value & 1):
            return 0, 0, 0
        return n64_rgb5(value)

    if texture.format == 2 and texture.size == 0:
        row_bytes = (texture.width + 1) // 2
        indices = (
            (
                texture.pixels[row * row_bytes + column // 2]
                >> (0 if column & 1 else 4)
            )
            & 0x0F
            for row in range(texture.height)
            for column in range(texture.width)
        )
        palette = texture.palette_rgba5551[:16]
        return [
            visible_rgb(palette[index] if index < len(palette) else 0)
            for index in indices
        ]
    if texture.format == 2 and texture.size == 1:
        if texture.palette_rgba5551:
            return [
                # Native conversion expands every CI8 palette to a 256-word
                # PS1 CLUT; indices beyond the authored palette therefore
                # resolve to its zero-filled tail rather than raising here.
                visible_rgb(
                    texture.palette_rgba5551[index]
                    if index < len(texture.palette_rgba5551)
                    else 0
                )
                for index in texture.pixels
            ]
        return [
            (value >> 3, value >> 3, value >> 3)
            for value in texture.pixels
        ]
    if texture.format == 0 and texture.size == 2:
        return [
            visible_rgb(n64.be16(texture.pixels, offset))
            for offset in range(0, len(texture.pixels), 2)
        ]
    if texture.format == 0 and texture.size == 3:
        result = []
        for offset in range(0, len(texture.pixels), 4):
            value = (
                ((texture.pixels[offset] >> 3) << 11)
                | ((texture.pixels[offset + 1] >> 3) << 6)
                | ((texture.pixels[offset + 2] >> 3) << 1)
                | (1 if texture.pixels[offset + 3] else 0)
            )
            result.append(visible_rgb(value))
        return result
    raise n64.FormatError(
        f"texture {texture.index} uses unsupported format/size "
        f"{texture.format}/{texture.size}"
    )


def target_texture_rgb(
    texture: TextureSlot, authored_width: int, authored_height: int
) -> list[tuple[int, int, int]]:
    """Decode only authored texels, excluding native TIM row padding."""

    if texture.height != authored_height or texture.width < authored_width:
        return []
    if texture.direct_pixels_bgr555:
        colors = texture.direct_pixels_bgr555
    else:
        colors = tuple(texture.palette[index] for index in texture.indices)
    return [
        psx_rgb5(colors[row * texture.width + column])
        for row in range(authored_height)
        for column in range(authored_width)
    ]


def texture_conversion_summary(
    textures: list[n64.N64Texture], target_data: bytes
) -> str:
    target_model = Model(target_data, "V8")
    compared = min(len(textures), target_model.texture_count)
    mismatched_pixels = 0
    mismatched_textures = 0
    dimension_failures = 0
    for index, source in enumerate(textures[:compared]):
        target = target_model.texture(index)
        source_rgb = source_texture_rgb(source)
        target_rgb = target_texture_rgb(target, source.width, source.height)
        if not target_rgb:
            dimension_failures += 1
            mismatched_textures += 1
            mismatched_pixels += len(source_rgb)
            continue
        mismatch = sum(
            source_color != target_color
            for source_color, target_color in zip(source_rgb, target_rgb)
        )
        mismatched_pixels += mismatch
        mismatched_textures += mismatch != 0
    passed = (
        compared == len(textures)
        and mismatched_textures == 0
        and dimension_failures == 0
    )
    return (
        f"texture_base_conversion={'PASS' if passed else 'FAIL'}"
        f"[{compared}/{len(textures)}] "
        f"mismatched_textures={mismatched_textures} "
        f"mismatched_pixels={mismatched_pixels} "
        f"dimension_failures={dimension_failures}"
    )


def expected_packet_mappings(
    vertices: list[n64.Vertex],
    faces: list[n64.Face],
    textures: list[n64.N64Texture],
    variant_indices: dict[tuple[int, int, bool], int],
) -> list[
    tuple[tuple[int, int, int], tuple[tuple[int, int], ...], int | None]
]:
    index_by_source = {
        vertex.source_offset: index for index, vertex in enumerate(vertices)
    }
    result = []
    for face in faces:
        indices = tuple(
            index_by_source[vertex.source_offset] for vertex in face.vertices
        )
        reverse = (2, 1, 0)
        if face.dynamic_texture:
            orders = [reverse] + ([(0, 1, 2)] if face.double_sided else [])
            for order in orders:
                result.append(
                    (
                        tuple(indices[item] for item in order),
                        tuple(face.uv[item] for item in order),
                        0xFFFF,
                    )
                )
            continue
        if face.texture is None and face.semi_transparent:
            for order in (reverse, (0, 1, 2)):
                result.append(
                    (
                        tuple(indices[item] for item in order),
                        ((0, 0), (0, 0), (0, 0)),
                        0x400A,
                    )
                )
            continue
        orders = [reverse] + ([(0, 1, 2)] if face.double_sided else [])
        target_texture = None
        if face.texture is not None:
            texture = textures[face.texture]
            palette_bank = (
                face.palette_bank
                if texture.format == 2
                and texture.size == 0
                and face.palette_bank * 16 < texture.palette_count
                else 0
            )
            alpha_zero_transparent = (
                face.semi_transparent
                or n64.texture_has_nonblack_alpha_cutout(
                    texture, palette_bank
                )
            )
            target_texture = variant_indices[
                (face.texture, palette_bank, alpha_zero_transparent)
            ]
        for order in orders:
            result.append(
                (
                    tuple(indices[item] for item in order),
                    (
                        tuple(face.uv[item] for item in order)
                        if target_texture is not None
                        else ()
                    ),
                    target_texture,
                )
            )
    return result


def target_packet_mappings(
    model: Model, group: int
) -> list[
    tuple[tuple[int, int, int], tuple[tuple[int, int], ...], int | None]
]:
    return [
        (
            packet.vertex_indices,
            packet.uv if packet.texture_slot is not None else (),
            packet.texture_slot,
        )
        for packet in model.group(group).packets
    ]


def audit(
    exp: bytes,
    requested_groups: set[int] | None,
    converted: bytes | None,
) -> str:
    bins = xobf_bins(exp)
    converted_bins = xobf_bins(converted) if converted is not None else []
    owners = source_group_owners(exp, bins)
    lines = [
        "# N64 model conversion audit",
        "",
        f"XOBF banks={len(bins)} objects={len(object_heads(exp))}",
    ]
    if converted is not None:
        source_heads = object_head_payloads(exp)
        target_heads = object_head_payloads(converted)
        heads_match = target_heads[:len(source_heads)] == source_heads
        lines.append(
            f"converted XOBF banks={len(converted_bins)} "
            f"(source banks compared={len(bins)})"
        )
        lines.append(
            "object HEAD preservation="
            + ("PASS" if heads_match else "FAIL")
            + f" source={len(source_heads)} target={len(target_heads)} "
            + f"appended={len(target_heads) - len(source_heads)}"
        )
        if len(converted_bins) < len(bins):
            raise n64.FormatError("converted arena has fewer XOBF banks")
    ignored_opcodes = {0xE3, 0xE6, 0xE7, 0xE8, 0xF0, 0xF3, 0xFB, 0xFC}
    for bank, data in enumerate(bins):
        group_count = n64.be32(data, 0)
        group_table = n64.be32(data, 4)
        texture_count = n64.be32(data, 16)
        texture_table = n64.be32(data, 20)
        texture_offsets = [
            texture_table + n64.be32(data, texture_table + index * 4)
            for index in range(texture_count)
        ]
        textures = [
            n64.parse_n64_texture(
                data,
                offset,
                index,
                texture_offsets[index + 1]
                if index + 1 < texture_count
                else len(data),
            )
            for index, offset in enumerate(texture_offsets)
        ]
        texture_base = texture_offsets[0] if textures else 0
        parsed_groups = []
        for group in range(group_count):
            group_offset = group_table + n64.be32(
                data, group_table + group * 4
            )
            parsed_groups.append(
                n64.parse_group_faces(
                    data, group_offset, textures, texture_base
                )
            )
        base_alpha_zero = {
            texture.index: n64.texture_has_nonblack_alpha_cutout(texture)
            for texture in textures
        }
        variant_indices = {
            (texture.index, 0, base_alpha_zero[texture.index]): texture.index
            for texture in textures
        }
        next_variant = len(textures)
        for _vertices, faces, _scale, _auxiliary, _extent in parsed_groups:
            for face in faces:
                if face.texture is None:
                    continue
                texture = textures[face.texture]
                palette_bank = (
                    face.palette_bank
                    if texture.format == 2
                    and texture.size == 0
                    and face.palette_bank * 16 < texture.palette_count
                    else 0
                )
                alpha_zero_transparent = (
                    face.semi_transparent
                    or n64.texture_has_nonblack_alpha_cutout(
                        texture, palette_bank
                    )
                )
                key = (
                    face.texture,
                    palette_bank,
                    alpha_zero_transparent,
                )
                if key not in variant_indices:
                    variant_indices[key] = next_variant
                    next_variant += 1
        slot_count = n64.be32(data, 0x18)
        slot_status = ""
        target_model = None
        if converted is not None:
            target_data = converted_bins[bank]
            target_model = Model(target_data, "V8")
            target_slot_count = struct.unpack_from("<I", target_data, 0x18)[0]
            slots_match = slot_count == target_slot_count and all(
                target_data[0x1C + index * 0x1C:0x38 + index * 0x1C]
                == n64.convert_slot(
                    data[0x1C + index * 0x1C:0x38 + index * 0x1C]
                )
                for index in range(slot_count)
            )
            slot_status = (
                " slot_conversion="
                + ("PASS" if slots_match else "FAIL")
                + f"[{target_slot_count}/{slot_count}]"
            )
            slot_status += " " + texture_conversion_summary(
                textures, target_data
            )
            mapping_matches = sum(
                expected_packet_mappings(
                    parsed_groups[group][0],
                    parsed_groups[group][1],
                    textures,
                    variant_indices,
                )
                == target_packet_mappings(target_model, group)
                for group in range(group_count)
            )
            slot_status += (
                " packet_mapping_conversion="
                + ("PASS" if mapping_matches == group_count else "FAIL")
                + f"[{mapping_matches}/{group_count}]"
            )
        lines.extend(
            (
                "",
                f"## Bank {bank}",
                "",
                f"groups={group_count} textures={texture_count} "
                f"slots={slot_count}{slot_status}",
            )
        )
        for group in range(group_count):
            if requested_groups is not None and group not in requested_groups:
                continue
            group_offset = group_table + n64.be32(
                data, group_table + group * 4
            )
            vertices, faces, scale, auxiliary, extent = parsed_groups[group]
            commands = display_commands(data, group_offset)
            unresolved_texture_addresses = [
                word1
                for opcode, _word0, word1 in commands.elements()
                if opcode == 0xFD
                and word1 >> 24 != 3
                and n64._texture_for_address(
                    word1, textures, texture_base
                ) is None
            ]
            converted_status = ""
            if converted is not None:
                target_vertices, target_polygons, target_scale = target_group(
                    converted_bins[bank], group
                )
                source_vertices = [(vertex.x, vertex.y, vertex.z) for vertex in vertices]
                expected_polygons = sum(
                    (
                        2 if face.double_sided else 1
                    )
                    if face.dynamic_texture
                    else (
                        2
                        if (
                            face.double_sided
                            or (
                                face.texture is None
                                and face.semi_transparent
                            )
                        )
                        else 1
                    )
                    for face in faces
                )
                checks = {
                    "vertices": source_vertices == target_vertices,
                    "polygons": expected_polygons == target_polygons,
                    "scale": scale == target_scale,
                    "texture_addresses": not unresolved_texture_addresses,
                    "packet_mappings": expected_packet_mappings(
                        vertices, faces, textures, variant_indices
                    )
                    == target_packet_mappings(target_model, group),
                }
                converted_status = (
                    " converted="
                    + ("PASS" if all(checks.values()) else "FAIL")
                    + f"[v={len(target_vertices)}/{len(source_vertices)}"
                    + f",p={target_polygons}/{expected_polygons}"
                    + f",scale={target_scale}/{scale}]"
                )
                if not checks["packet_mappings"]:
                    converted_status += " packet_mappings=FAIL"
            names = ",".join(sorted(owners.get((bank, group), ()))) or "-"
            texture_faces = Counter(face.texture for face in faces)
            opcodes = Counter(opcode for opcode, _word0, _word1 in commands.elements())
            lines.append(
                f"group={group:3d} owners={names} vertices={len(vertices):3d} "
                f"faces={len(faces):3d} scale={scale} aux={auxiliary} "
                f"extent={extent} opcodes="
                + " ".join(f"{opcode:02x}:{count}" for opcode, count in sorted(opcodes.items()))
                + converted_status
            )
            if unresolved_texture_addresses:
                lines.append(
                    "  unresolved_texture_addresses="
                    + ",".join(
                        f"0x{address:08x}"
                        for address in unresolved_texture_addresses
                    )
                )
            lines.append(
                "  faces textures="
                + " ".join(
                    f"{'-' if texture is None else texture}:{count}"
                    for texture, count in sorted(
                        texture_faces.items(),
                        key=lambda item: (-1 if item[0] is None else item[0]),
                    )
                )
                + f" lit={sum(face.lighting for face in faces)}"
                + f" double={sum(face.double_sided for face in faces)}"
                + f" translucent={sum(face.semi_transparent for face in faces)}"
            )
            for texture_index, count in sorted(
                texture_faces.items(),
                key=lambda item: (-1 if item[0] is None else item[0]),
            ):
                if texture_index is None:
                    continue
                texture = textures[texture_index]
                relevant = [face for face in faces if face.texture == texture_index]
                us = [uv[0] for face in relevant for uv in face.uv]
                vs = [uv[1] for face in relevant for uv in face.uv]
                palette_banks = Counter(face.palette_bank for face in relevant)
                out_of_bounds = sum(
                    u >= texture.width or v >= texture.height
                    for face in relevant
                    for u, v in face.uv
                )
                lines.append(
                    f"  texture={texture_index:3d} faces={count:3d} "
                    f"fmt={texture.format}/{texture.size} "
                    f"size={texture.width}x{texture.height} "
                    f"palette={texture.palette_count} "
                    f"banks={dict(sorted(palette_banks.items()))} "
                    f"uv=({min(us)}..{max(us)},{min(vs)}..{max(vs)}) "
                    f"uv_outside_image={out_of_bounds} "
                    "alpha_zero_transparent="
                    + str(
                        any(
                            face.semi_transparent
                            or n64.texture_has_nonblack_alpha_cutout(
                                texture, face.palette_bank
                            )
                            for face in relevant
                        )
                    )
                )
            for opcode, word0, word1 in sorted(commands):
                if opcode in ignored_opcodes:
                    lines.append(
                        f"  reduced_state opcode={opcode:02x} "
                        f"word0={word0:08x} word1={word1:08x} "
                        f"count={commands[(opcode, word0, word1)]}"
                    )
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("arena")
    parser.add_argument(
        "--decoded-source",
        type=Path,
        help="use an existing decoded arena EXP instead of extracting the ROM",
    )
    parser.add_argument(
        "--groups",
        help="comma-separated source group numbers; default audits all groups",
    )
    parser.add_argument(
        "--converted",
        type=Path,
        help="native PS1 EXP to verify against the decoded source groups",
    )
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    requested = (
        {int(value, 0) for value in args.groups.split(",")}
        if args.groups
        else None
    )
    exp = (
        args.decoded_source.read_bytes()
        if args.decoded_source is not None
        else n64.V8N64Rom(args.rom).decoded(f"{args.arena.upper()}.EXP")
    )
    converted = args.converted.read_bytes() if args.converted else None
    report = audit(exp, requested, converted)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(report, encoding="utf-8")
        print(args.output)
    else:
        print(report, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
