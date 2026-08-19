#!/usr/bin/env python3
"""Classify Dreamland texture storage from original-game RDRAM evidence.

The large terrain atlas is copied tile-by-tile and repacked by the game, while
standalone XOBF/XRTP images can be uploaded from their archived storage.  This
audit searches both the archived byte layout and its odd-row 64-bit-half-swap
counterpart in a runtime RDRAM dump, then correlates matches with captured RDP
LoadBlock addresses.  It prevents one texture class from imposing its storage
rule on every other class.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import v8_n64_level as n64  # noqa: E402


def host_words(data: bytes) -> bytes:
    if len(data) % 4:
        raise ValueError("runtime search payload is not word aligned")
    return b"".join(
        data[offset:offset + 4][::-1]
        for offset in range(0, len(data), 4)
    )


def odd_row_half_swap(
    data: bytes, row_stride: int, height: int
) -> bytes:
    if row_stride % 8 or len(data) != row_stride * height:
        raise ValueError("texture storage is not an integral array of 64-bit rows")
    output = bytearray()
    for row in range(height):
        source = data[row * row_stride:(row + 1) * row_stride]
        if row & 1:
            source = b"".join(
                source[offset + 4:offset + 8]
                + source[offset:offset + 4]
                for offset in range(0, row_stride, 8)
            )
        output += source
    return bytes(output)


def all_matches(haystack: bytes, needle: bytes) -> list[int]:
    result: list[int] = []
    cursor = 0
    while needle:
        match = haystack.find(needle, cursor)
        if match < 0:
            break
        result.append(match)
        cursor = match + 1
    return result


def texture_storage(
    data: bytes, offset: int, end: int
) -> tuple[int, int, int, int, int, bytes]:
    if offset + 8 > end:
        raise n64.FormatError("texture header is truncated")
    image_format = data[offset]
    image_size = data[offset + 1]
    palette_count = n64.be16(data, offset + 2)
    width = n64.be16(data, offset + 4)
    height = n64.be16(data, offset + 6)
    bits_per_texel = (4, 8, 16, 32)[image_size]
    row_bytes = (width * bits_per_texel + 7) // 8
    row_stride = n64.align(row_bytes, 8)
    palette_end = offset + n64.align(8 + palette_count * 2, 8)
    pixel_end = palette_end + row_stride * height
    if pixel_end > end:
        raise n64.FormatError("texture pixels are truncated")
    return (
        image_format,
        image_size,
        width,
        height,
        row_stride,
        data[palette_end:pixel_end],
    )


def source_textures(arena: bytes) -> list[dict[str, object]]:
    result: list[dict[str, object]] = []
    xobf_bank = 0
    for child in n64.root_children(arena):
        if child.is_form and child.form_type == b"XOBF":
            nested = n64.form_children(
                n64.iff_form(b"XOBF", [child.payload]), b"XOBF"
            )
            data = next(item.payload for item in nested if item.tag == b"BIN ")
            texture_count = n64.be32(data, 16)
            texture_table = n64.be32(data, 20)
            offsets = [
                texture_table + n64.be32(data, texture_table + index * 4)
                for index in range(texture_count)
            ]
            for index, offset in enumerate(offsets):
                end = offsets[index + 1] if index + 1 < len(offsets) else len(data)
                fmt, size, width, height, stride, pixels = texture_storage(
                    data, offset, end
                )
                result.append({
                    "class": "XOBF",
                    "label": f"bank{xobf_bank}.texture{index}",
                    "format": fmt,
                    "size": size,
                    "width": width,
                    "height": height,
                    "row_stride": stride,
                    "pixels": pixels,
                })
            xobf_bank += 1

    chunk_counts: dict[bytes, int] = {}
    for _offset, tag, payload, _parent in n64.iter_chunks(arena):
        if tag not in {b"XBMP", b"XBGM", b"XRTP"}:
            continue
        index = chunk_counts.get(tag, 0)
        chunk_counts[tag] = index + 1
        texture_offset = 12 if tag == b"XRTP" else 0
        if len(payload) <= texture_offset + 8:
            continue
        try:
            fmt, size, width, height, stride, pixels = texture_storage(
                payload, texture_offset, len(payload)
            )
        except (IndexError, n64.FormatError):
            continue
        result.append({
            "class": tag.decode("ascii"),
            "label": f"{tag.decode('ascii')}.{index}",
            "format": fmt,
            "size": size,
            "width": width,
            "height": height,
            "row_stride": stride,
            "pixels": pixels,
        })
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("rdram", type=Path)
    parser.add_argument("rdp_report", type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    arena = args.source.read_bytes()
    rdram = args.rdram.read_bytes()
    rdp = json.loads(args.rdp_report.read_text(encoding="utf-8"))
    loads_by_address: dict[str, list[dict[str, object]]] = {}
    for load in rdp["texture_loads"]:
        address = (load.get("texture_image") or {}).get("address")
        if address:
            loads_by_address.setdefault(address, []).append(load)

    records: list[dict[str, object]] = []
    for source in source_textures(arena):
        pixels = source.pop("pixels")
        swapped = odd_row_half_swap(
            pixels, int(source["row_stride"]), int(source["height"])
        )
        raw_matches = all_matches(rdram, host_words(pixels))
        swapped_matches = all_matches(rdram, host_words(swapped))
        raw_addresses = [f"0x{address:08X}" for address in raw_matches]
        swapped_addresses = [f"0x{address:08X}" for address in swapped_matches]
        raw_loads = [
            load
            for address in raw_addresses
            for load in loads_by_address.get(address, [])
        ]
        swapped_loads = [
            load
            for address in swapped_addresses
            for load in loads_by_address.get(address, [])
        ]
        classification = "not_present_in_capture"
        if raw_loads and not swapped_loads:
            classification = "archive_storage_uploaded_directly"
        elif swapped_loads and not raw_loads:
            classification = "runtime_odd_row_repack_uploaded"
        elif raw_loads and swapped_loads:
            classification = "ambiguous_both_layouts_loaded"
        records.append({
            **source,
            "archive_storage_rdram_addresses": raw_addresses,
            "odd_row_swapped_rdram_addresses": swapped_addresses,
            "archive_storage_rdp_loads": raw_loads,
            "odd_row_swapped_rdp_loads": swapped_loads,
            "classification": classification,
        })

    direct = [
        record for record in records
        if record["classification"] == "archive_storage_uploaded_directly"
    ]
    repacked = [
        record for record in records
        if record["classification"] == "runtime_odd_row_repack_uploaded"
    ]
    direct_classes = sorted({str(record["class"]) for record in direct})
    repacked_classes = sorted({str(record["class"]) for record in repacked})
    xrtp_direct = any(record["class"] == "XRTP" for record in direct)
    xobf_direct = any(record["class"] == "XOBF" for record in direct)
    passed = xrtp_direct and xobf_direct
    report = {
        "schema": "v8.n64-runtime-texture-classes.v1",
        "status": "PASS" if passed else "FAIL",
        "source": str(args.source.resolve()),
        "rdram": str(args.rdram.resolve()),
        "rdp_report": str(args.rdp_report.resolve()),
        "texture_records": len(records),
        "direct_upload_records": len(direct),
        "runtime_repack_records": len(repacked),
        "direct_upload_classes": direct_classes,
        "runtime_repack_classes": repacked_classes,
        "records": records,
        "interpretation": (
            "Standalone XOBF/XRTP archive storage reaches LoadBlock directly; "
            "its odd-row halves must be undone for PS1 images. The terrain "
            "XBMP is a separate tile-repack path proven by the atlas audit."
            if passed else
            "The capture did not establish direct XOBF and XRTP uploads."
        ),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({key: value for key, value in report.items() if key != "records"}, indent=2))
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
