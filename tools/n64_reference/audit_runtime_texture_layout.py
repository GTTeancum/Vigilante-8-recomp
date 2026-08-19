#!/usr/bin/env python3
"""Compare archived Dreamland atlas row layouts with a runtime RDRAM dump.

The converter used to validate its terrain atlas against the same decoder that
created it.  This audit instead searches the original N64 process image for
the archived linear rows and the proposed odd-row 64-bit-half swap.  It makes
the byte-order conversion used by the emulator dump explicit, so a decoder
mistake cannot certify itself.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import v8_n64_level as n64  # noqa: E402


def host_words(data: bytes) -> bytes:
    """Convert canonical N64 bytes to emulator-native 32-bit word order."""

    if len(data) % 4:
        raise ValueError("runtime search payload is not word aligned")
    return b"".join(
        data[offset:offset + 4][::-1]
        for offset in range(0, len(data), 4)
    )


def odd_row_half_swap(
    data: bytes, width: int, height: int
) -> bytes:
    if width % 8 or len(data) != width * height:
        raise ValueError("CI8 atlas is not an integral array of 64-bit rows")
    output = bytearray()
    for row in range(height):
        source = data[row * width:(row + 1) * width]
        if row & 1:
            source = b"".join(
                source[offset + 4:offset + 8]
                + source[offset:offset + 4]
                for offset in range(0, width, 8)
            )
        output += source
    return bytes(output)


def all_matches(haystack: bytes, needle: bytes) -> list[int]:
    matches: list[int] = []
    cursor = 0
    while True:
        match = haystack.find(needle, cursor)
        if match < 0:
            return matches
        matches.append(match)
        cursor = match + 1


def atlas_tiles(data: bytes, width: int, height: int) -> list[bytes]:
    if width % 32 or height % 32:
        raise ValueError("terrain atlas is not composed of 32x32 materials")
    return [
        b"".join(
            data[(tile_y + row) * width + tile_x:
                 (tile_y + row) * width + tile_x + 32]
            for row in range(32)
        )
        for tile_y in range(0, height, 32)
        for tile_x in range(0, width, 32)
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("rdram", type=Path)
    parser.add_argument(
        "--rdp-report",
        type=Path,
        help="runtime RDP texture audit used to prove the upload has DXT=0",
    )
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    arena = args.source.read_bytes()
    payload = next(
        payload
        for _offset, tag, payload, _parent in n64.iter_chunks(arena)
        if tag == b"XBMP"
    )
    if len(payload) < 0x208 or payload[0:2] != b"\x02\x01":
        raise ValueError("first Dreamland XBMP is not a CI8 terrain atlas")
    width = n64.be16(payload, 4)
    height = n64.be16(payload, 6)
    row_stride = n64.align(width, 8)
    archived = payload[0x208:0x208 + row_stride * height]
    if row_stride != width:
        raise ValueError("terrain atlas contains row padding")
    swapped = odd_row_half_swap(archived, width, height)
    rdram = args.rdram.read_bytes()

    variants = {
        "archived_linear": archived,
        "odd_row_64bit_half_swapped": swapped,
    }
    records = []
    for name, data in variants.items():
        canonical_matches = all_matches(rdram, data)
        host_matches = all_matches(rdram, host_words(data))
        tile_records = []
        for index, tile in enumerate(atlas_tiles(data, width, height)):
            tile_canonical = all_matches(rdram, tile)
            tile_host = all_matches(rdram, host_words(tile))
            if tile_canonical or tile_host:
                tile_records.append({
                    "tile": index,
                    "canonical_rdram_matches": [
                        f"0x{offset:08X}" for offset in tile_canonical
                    ],
                    "host_word_rdram_matches": [
                        f"0x{offset:08X}" for offset in tile_host
                    ],
                })
        records.append({
            "layout": name,
            "sha256": hashlib.sha256(data).hexdigest(),
            "canonical_rdram_matches": [
                f"0x{offset:08X}" for offset in canonical_matches
            ],
            "host_word_rdram_matches": [
                f"0x{offset:08X}" for offset in host_matches
            ],
            "matched_32x32_tiles": tile_records,
            "matched_32x32_tile_count": len(tile_records),
        })

    linear_record = next(
        record for record in records
        if record["layout"] == "archived_linear"
    )
    swapped_record = next(
        record for record in records
        if record["layout"] == "odd_row_64bit_half_swapped"
    )
    runtime_loads: list[dict[str, object]] = []
    if args.rdp_report:
        rdp_report = json.loads(args.rdp_report.read_text(encoding="utf-8"))
        tile_zero = next(
            (
                item for item in swapped_record["matched_32x32_tiles"]
                if item["tile"] == 0
            ),
            None,
        )
        tile_zero_addresses = set(
            [] if tile_zero is None
            else tile_zero["canonical_rdram_matches"]
            + tile_zero["host_word_rdram_matches"]
        )
        runtime_loads = [
            item for item in rdp_report["texture_loads"]
            if (item.get("texture_image") or {}).get("address")
            in tile_zero_addresses
        ]
    all_tiles_swapped = (
        swapped_record["matched_32x32_tile_count"] == (width // 32) * (height // 32)
    )
    no_linear_tiles = linear_record["matched_32x32_tile_count"] == 0
    dxt_zero = bool(runtime_loads) and all(
        item["dxt_or_lower_right_t"] == 0 for item in runtime_loads
    )
    passed = all_tiles_swapped and no_linear_tiles and dxt_zero
    report = {
        "schema": "v8.n64-runtime-texture-layout.v1",
        "status": "PASS" if passed else "FAIL",
        "source": str(args.source.resolve()),
        "rdram": str(args.rdram.resolve()),
        "format": "CI8",
        "width": width,
        "height": height,
        "variants": records,
        "runtime_tile_zero_loads": runtime_loads,
        "interpretation": (
            "The archived atlas is the logical row order. The N64 engine "
            "explicitly repacks every 32x32 tile into odd-row TMEM order and "
            "uploads that pre-swizzled buffer with DXT=0. The PS1 conversion "
            "must therefore retain the archived linear rows."
            if passed
            else "Runtime memory and upload state did not establish one layout."
        ),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if report["status"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
