#!/usr/bin/env python3
"""Decode the retail HUD TIM and audit the two enhanced right-side silhouettes."""

from __future__ import annotations

import argparse
import json
import struct
import xml.etree.ElementTree as ET
from pathlib import Path


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def decode_table_tim(path: Path, record_index: int) -> tuple[
    int, int, list[int], list[int]
]:
    data = path.read_bytes()
    count = u32(data, 0)
    if record_index < 0 or record_index >= count:
        raise ValueError(f"record {record_index} outside table of {count}")
    record = u32(data, 4 + record_index * 4)
    end = (
        u32(data, 4 + (record_index + 1) * 4)
        if record_index + 1 < count
        else len(data)
    )
    if u32(data, record) != 0x10:
        raise ValueError("record is not a PlayStation TIM")
    flags = u32(data, record + 4)
    if flags != 0x08:
        raise ValueError(f"only 4-bpp CLUT TIM is supported, flags=0x{flags:X}")

    clut_size = u32(data, record + 8)
    clut_width = u16(data, record + 16)
    clut_height = u16(data, record + 18)
    palette_start = record + 20
    palette = [
        u16(data, palette_start + index * 2)
        for index in range(clut_width * clut_height)
    ]

    image = record + 8 + clut_size
    image_size = u32(data, image)
    words_per_row = u16(data, image + 8)
    height = u16(data, image + 10)
    width = words_per_row * 4
    packed = data[image + 12:image + image_size]
    expected = words_per_row * 2 * height
    if len(packed) != expected or image + image_size > end:
        raise ValueError(
            f"truncated TIM image: {len(packed)} bytes, expected {expected}"
        )

    indices: list[int] = []
    for value in packed:
        indices.extend((value & 0xF, value >> 4))
    if len(indices) != width * height:
        raise AssertionError("decoded pixel count mismatch")
    return width, height, palette, indices


def rgb8(color: int) -> tuple[int, int, int]:
    return (
        ((color >> 0) & 31) * 255 // 31,
        ((color >> 5) & 31) * 255 // 31,
        ((color >> 10) & 31) * 255 // 31,
    )


def write_ppm(
    output: Path,
    width: int,
    height: int,
    palette: list[int],
    indices: list[int],
) -> None:
    pixels = bytearray()
    for index in indices:
        if index == 0:
            pixels.extend((255, 0, 255))
        else:
            pixels.extend(rgb8(palette[index]))
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(
        f"P6\n{width} {height}\n255\n".encode("ascii") + pixels
    )


def points(value: str) -> list[tuple[float, float]]:
    result: list[tuple[float, float]] = []
    for pair in value.split():
        x, y = pair.split(",", 1)
        result.append((float(x), float(y)))
    return result


def inside(
    point: tuple[float, float],
    polygon: list[tuple[float, float]],
) -> bool:
    x, y = point
    result = False
    previous = polygon[-1]
    for current in polygon:
        ax, ay = current
        bx, by = previous
        if (ay > y) != (by > y):
            edge_x = (bx - ax) * (y - ay) / (by - ay) + ax
            if x < edge_x:
                result = not result
        previous = current
    return result


def row_runs(
    mask: list[bool], width: int, height: int
) -> list[list[list[int]]]:
    result: list[list[list[int]]] = []
    for y in range(height):
        runs: list[list[int]] = []
        x = 0
        while x < width:
            if not mask[y * width + x]:
                x += 1
                continue
            start = x
            while x + 1 < width and mask[y * width + x + 1]:
                x += 1
            runs.append([start, x])
            x += 1
        result.append(runs)
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--table", type=Path, required=True)
    parser.add_argument("--svg", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--preview", type=Path)
    parser.add_argument("--record", type=int, default=1)
    args = parser.parse_args()

    width, height, palette, indices = decode_table_tim(
        args.table, args.record
    )
    if (width, height) != (84, 34):
        raise ValueError(
            f"expected retail status backing 84x34, got {width}x{height}"
        )
    if args.preview:
        write_ppm(args.preview, width, height, palette, indices)

    root = ET.parse(args.svg).getroot()
    polygons = [
        points(element.attrib["points"])
        for element in root.iter()
        if element.tag.rsplit("}", 1)[-1] == "polygon"
        and element.attrib.get("fill") != "none"
    ]
    if len(polygons) < 3:
        raise ValueError("status SVG does not contain its three filled plates")
    weapon_polygon, ammo_polygon = polygons[-2:]

    # The accepted vehicle plate is extended six pixels to the left. The
    # right-side retail geometry therefore starts at source x=40 and appears
    # in SVG-local coordinates at x=46.
    retail_mask = [
        (palette[index] & 0x7FFF) != 0 and x >= 40
        for y in range(height)
        for x, index in enumerate(indices[y * width:(y + 1) * width])
    ]
    vector_mask = [
        inside((x + 0.5 + 6.0, y + 0.5), weapon_polygon)
        or inside((x + 0.5 + 6.0, y + 0.5), ammo_polygon)
        for y in range(height)
        for x in range(width)
    ]
    mismatches = [
        (x, y)
        for y in range(height)
        for x in range(40, width)
        if retail_mask[y * width + x] != vector_mask[y * width + x]
    ]
    report = {
        "schema": 1,
        "table": str(args.table.resolve()),
        "record": args.record,
        "native_size": [width, height],
        "svg": str(args.svg.resolve()),
        "coordinate_transform": "svg_x = retail_x + 6",
        "retail_right_row_runs": row_runs(
            retail_mask, width, height
        ),
        "vector_right_row_runs": row_runs(
            vector_mask, width, height
        ),
        "mismatch_pixels": len(mismatches),
        "mismatch_coordinates": [list(value) for value in mismatches],
        "checks": {
            "retail_status_record_is_84x34": (width, height) == (84, 34),
            "active_weapon_and_ammo_match_retail_mask": not mismatches,
        },
    }
    report["passed"] = all(report["checks"].values())
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    print(
        f"[HudVectorGeometry] {'PASS' if report['passed'] else 'FAIL'} "
        f"mismatches={len(mismatches)}"
    )
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
