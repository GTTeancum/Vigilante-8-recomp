#!/usr/bin/env python3
"""Compare Dreamland's authored N64 water frames with converted PS1 textures."""

from __future__ import annotations

import argparse
from collections import Counter
import math
from pathlib import Path

import v8_n64_level as n64
from blender_addons.vigilante8_vehicle_tools.xobf import Model, TextureSlot
from terrain_object_identity_audit import collect_bins, collect_heads


PHASE_TILES = (0x23, 0x24, 0x25, 0x26, 0x27)


def n64_rgb(texture: n64.N64Texture) -> list[tuple[int, int, int]]:
    result = []
    for palette_index in texture.pixels:
        color = texture.palette_rgba5551[palette_index]
        result.append(
            (
                ((color >> 11) & 31) * 255 // 31,
                ((color >> 6) & 31) * 255 // 31,
                ((color >> 1) & 31) * 255 // 31,
            )
        )
    return result


def ps1_rgb(texture: TextureSlot) -> list[tuple[int, int, int]]:
    source = (
        texture.direct_pixels_bgr555
        if texture.direct_pixels_bgr555
        else tuple(texture.palette[index] for index in texture.indices)
    )
    return [
        (
            (color & 31) * 255 // 31,
            ((color >> 5) & 31) * 255 // 31,
            ((color >> 10) & 31) * 255 // 31,
        )
        for color in source
    ]


def mean_rgb(pixels: list[tuple[int, int, int]]) -> tuple[float, float, float]:
    return tuple(
        sum(pixel[channel] for pixel in pixels) / len(pixels)
        for channel in range(3)
    )


def mean_luma(pixels: list[tuple[int, int, int]]) -> float:
    return sum(
        0.2126 * red + 0.7152 * green + 0.0722 * blue
        for red, green, blue in pixels
    ) / len(pixels)


def winding_signatures(indices: tuple[int, ...]) -> tuple[
    tuple[int, ...], tuple[int, ...]
]:
    if len(indices) != 3:
        return indices, indices
    a, b, c = indices
    forward = min((a, b, c), (b, c, a), (c, a, b))
    reverse = min((a, c, b), (c, b, a), (b, a, c))
    return forward, reverse


def audit_retail_reference(path: Path) -> None:
    banks = collect_bins(path)
    heads = [
        head for head in collect_heads(path)
        if head["name"].lower().startswith("water")
        and head["bank"] >= 0 and head["slot"] >= 0
    ]
    if not heads:
        raise ValueError(f"{path} has no modeled water object")
    head = heads[0]
    model = Model(banks[head["bank"]], "V8")
    slots = model.slots()
    groups: list[int] = []
    seen: set[int] = set()

    def walk(index: int) -> None:
        if index == 0xFFFF or index in seen:
            return
        seen.add(index)
        slot = slots[index]
        if slot.render_group is not None:
            groups.append(slot.render_group)
        walk(slot.first_child)
        walk(slot.next_sibling)

    walk(head["slot"])
    print(
        f"retail reference: {path.name} object={head['name']} "
        f"bank={head['bank']} root={head['slot']} groups={groups}"
    )
    textures = tuple(model.textures())
    for group_index in groups:
        group = model.group(group_index)
        orientations = {
            winding_signatures(packet.vertex_indices)[0]
            for packet in group.packets
        }
        reverse_orientations = {
            winding_signatures(packet.vertex_indices)[1]
            for packet in group.packets
        }
        paired = len(orientations & reverse_orientations) // 2
        texture_slots = sorted({
            packet.texture_slot
            for packet in group.packets
            if packet.texture_slot is not None
        })
        print(
            f"  group={group_index} vertices={group.vertex_count} "
            f"faces={len(group.packets)} opposite_winding_pairs={paired} "
            f"packet_codes={sorted({packet.raw[3] for packet in group.packets})} "
            f"colors={sorted({packet.color for packet in group.packets})} "
            f"textures={texture_slots}"
        )
        for texture_index in texture_slots:
            if texture_index >= len(textures):
                continue
            pixels = ps1_rgb(textures[texture_index])
            rgb = mean_rgb(pixels)
            print(
                f"    texture={texture_index} "
                f"mean_rgb={rgb[0]:.2f},{rgb[1]:.2f},{rgb[2]:.2f} "
                f"luma={mean_luma(pixels):.2f}"
            )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("n64_exp", type=Path)
    parser.add_argument("ps1_exp", type=Path)
    parser.add_argument(
        "--retail-reference",
        type=Path,
        help="optional shipped PS1 level whose water hierarchy is audited",
    )
    args = parser.parse_args()

    xbmps = [
        payload
        for _offset, tag, payload, _parent in n64.iter_chunks(
            args.n64_exp.read_bytes()
        )
        if tag == b"XBMP"
    ]
    if len(xbmps) != 1:
        raise ValueError(f"expected one N64 XBMP, found {len(xbmps)}")
    atlas = n64.parse_n64_texture(
        xbmps[0],
        0,
        0,
        len(xbmps[0]),
        archive_storage_is_tmem_order=False,
    )

    bins = collect_bins(args.ps1_exp)
    converted_model = Model(bins[-1], "V8")
    converted = tuple(converted_model.textures())
    if len(converted) < len(PHASE_TILES):
        raise ValueError(
            f"expected at least {len(PHASE_TILES)} converted frames, "
            f"found {len(converted)}"
        )
    for group in converted_model.groups():
        descriptor_count = bins[-1][group.descriptor_offset + 0x19]
        if descriptor_count != len(converted):
            raise ValueError(
                f"water group {group.index} caches {descriptor_count} "
                f"texture descriptors, expected {len(converted)}"
            )
        oriented = Counter(
            (winding_signatures(packet.vertex_indices)[0], packet.texture_slot)
            for packet in group.packets
        )
        reversed_oriented = Counter(
            (winding_signatures(packet.vertex_indices)[1], packet.texture_slot)
            for packet in group.packets
        )
        if oriented != reversed_oriented:
            raise ValueError(
                f"water group {group.index} does not contain one opposite "
                "winding for every textured triangle"
            )
        if any(
            first.texture_slot != second.texture_slot or
            first.vertex_indices != tuple(reversed(second.vertex_indices))
            for first, second in zip(group.packets[::2], group.packets[1::2])
        ):
            raise ValueError(
                f"water group {group.index} does not store adjacent "
                "forward/reverse pairs like Casino City"
            )
        print(
            f"converted water group={group.index} "
            f"faces={len(group.packets)} "
            f"descriptor_count={descriptor_count} "
            "two_sided_pairs=PASS"
        )

    print("tile  source_mean_rgb  ps1_mean_rgb  source_luma  ps1_luma  rmse")
    for tile, target in zip(PHASE_TILES, converted):
        source_texture = n64._dreamland_water_texture(atlas, tile)
        source = n64_rgb(source_texture)
        encoded = ps1_rgb(target)
        if len(source) != len(encoded):
            raise ValueError(
                f"tile 0x{tile:X}: {len(source)} source pixels, "
                f"{len(encoded)} converted pixels"
            )
        source_mean = mean_rgb(source)
        target_mean = mean_rgb(encoded)
        rmse = math.sqrt(
            sum(
                (source_pixel[channel] - target_pixel[channel]) ** 2
                for source_pixel, target_pixel in zip(source, encoded)
                for channel in range(3)
            )
            / (len(source) * 3)
        )
        print(
            f"0x{tile:02X}  "
            f"{source_mean[0]:6.2f},{source_mean[1]:6.2f},{source_mean[2]:6.2f}  "
            f"{target_mean[0]:6.2f},{target_mean[1]:6.2f},{target_mean[2]:6.2f}  "
            f"{mean_luma(source):11.2f}  {mean_luma(encoded):8.2f}  {rmse:5.2f}"
        )

    print(
        "N64 draw call: RGB modulation=255,255,255; alpha=160/255. "
        "Converted PS1 packets: RGB modulation=128,128,128; ABR=0 "
        "(source/destination average)."
    )
    if len(converted) == len(PHASE_TILES) * 2:
        print(
            "Converted alpha approximation: one ABR=0 base pass plus a "
            "50%-coverage ABR=0 pass = 5/8 source contribution "
            "(0.625 versus N64 0.62745)."
        )
    if args.retail_reference is not None:
        audit_retail_reference(args.retail_reference)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
