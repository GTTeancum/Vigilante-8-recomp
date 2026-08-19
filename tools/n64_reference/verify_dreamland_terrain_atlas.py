#!/usr/bin/env python3
"""Prove Dreamland conversion preserves the runtime-established archive rows."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import v8_n64_level as n64  # noqa: E402


def ps1_ci8(payload: bytes) -> tuple[int, int, tuple[int, ...], bytes]:
    if len(payload) < 0x20 or payload[4:8] != b"\x09\0\0\0":
        raise ValueError("converted XBMP is not a native PS1 CI8 image")
    # Native XBMP begins with palette block size/type/image-offset words; its
    # palette RECT starts at 0x0c, unlike the image block whose RECT follows
    # its own four-byte block size.
    palette_rect = 0x0C
    if palette_rect + 8 > len(payload):
        raise ValueError("converted XBMP palette is truncated")
    palette_width = int.from_bytes(
        payload[palette_rect + 4:palette_rect + 6], "little"
    )
    palette_height = int.from_bytes(
        payload[palette_rect + 6:palette_rect + 8], "little"
    )
    if palette_width <= 0 or palette_height <= 0 or palette_width * palette_height > 256:
        raise ValueError("converted XBMP palette dimensions are invalid")
    palette_start = palette_rect + 8
    palette_words = tuple(
        int.from_bytes(payload[offset:offset + 2], "little")
        for offset in range(
            palette_start,
            palette_start + palette_width * palette_height * 2,
            2,
        )
    )
    image = int.from_bytes(payload[8:12], "little")
    if image + 0x14 > len(payload):
        raise ValueError("converted XBMP image descriptor is truncated")
    words = int.from_bytes(payload[image + 0x10:image + 0x12], "little")
    height = int.from_bytes(payload[image + 0x12:image + 0x14], "little")
    width = words * 2
    start = image + 0x14
    end = start + width * height
    if end > len(payload):
        raise ValueError("converted XBMP texels are truncated")
    return width, height, palette_words, payload[start:end]


def n64_rgb555(value: int) -> tuple[int, int, int]:
    return ((value >> 11) & 31, (value >> 6) & 31, (value >> 1) & 31)


def ps1_rgb555(value: int) -> tuple[int, int, int]:
    return (value & 31, (value >> 5) & 31, (value >> 10) & 31)


def scaled(source: bytes, width: int, height: int) -> bytes:
    output_width = width * 3 // 2
    output_height = height * 3 // 2
    return bytes(
        source[min(height - 1, y * height // output_height) * width
               + min(width - 1, x * width // output_width)]
        for y in range(output_height)
        for x in range(output_width)
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--rom", type=Path, default=ROOT / "Vigilante 8 (U) (!).n64"
    )
    parser.add_argument(
        "--source",
        type=Path,
        help="existing decoded N64 DREAMLND.EXP; bypasses ROM extraction",
    )
    parser.add_argument(
        "--converted",
        type=Path,
        help=(
            "existing converted DREAMLND.EXP; avoids rebuilding the complete "
            "arena when auditing an already-generated candidate"
        ),
    )
    parser.add_argument(
        "--runtime-layout-proof",
        type=Path,
        default=(
            ROOT / "artifacts" / "n64_reference"
            / "dreamland-runtime-texture-layout-proof-20260814.json"
        ),
        help=(
            "runtime RDRAM/RDP proof that distinguishes logical archive rows "
            "from the N64 engine's pre-TMEM odd-row repack"
        ),
    )
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    arena = (
        args.source.read_bytes()
        if args.source
        else n64.V8N64Rom(args.rom).decoded("DREAMLND.EXP")
    )
    source_xbmp = next(
        payload for _offset, tag, payload, _parent in n64.iter_chunks(arena)
        if tag == b"XBMP"
    )
    texture = n64.parse_n64_texture(
        source_xbmp,
        0,
        0,
        len(source_xbmp),
        archive_storage_is_tmem_order=False,
    )
    raw = source_xbmp[0x208:0x208 + texture.width * texture.height]
    expected = scaled(texture.pixels, texture.width, texture.height)
    archived_linear = scaled(raw, texture.width, texture.height)

    runtime_layout = json.loads(
        args.runtime_layout_proof.read_text(encoding="utf-8")
    )
    runtime_layout_passed = (
        runtime_layout.get("schema") == "v8.n64-runtime-texture-layout.v1"
        and runtime_layout.get("status") == "PASS"
        and "archived atlas is the logical row order"
        in runtime_layout.get("interpretation", "")
    )

    converted = (
        args.converted.read_bytes()
        if args.converted
        else n64.convert_arena(arena, "DREAMLND")[0]
    )
    converted_xbmp = next(
        payload for _offset, tag, payload, _parent in n64.iter_chunks(converted)
        if tag == b"XBMP"
    )
    width, height, converted_palette, actual = ps1_ci8(converted_xbmp)
    mismatches = sum(a != b for a, b in zip(actual, expected))
    archived_linear_mismatches = sum(
        a != b for a, b in zip(archived_linear, expected)
    )
    palette_pairs = list(
        zip(texture.palette_rgba5551, converted_palette[:256])
    )
    palette_rgb_mismatches = sum(
        n64_rgb555(source) != ps1_rgb555(converted)
        for source, converted in palette_pairs
    )
    palette_stp_missing = sum(
        source != 0 and not (converted & 0x8000)
        for source, converted in palette_pairs
    )
    status = (
        "PASS"
        if (width, height) == (480, 192)
        and len(actual) == len(expected)
        and mismatches == 0
        and archived_linear_mismatches == 0
        and runtime_layout_passed
        and len(palette_pairs) == 256
        and palette_rgb_mismatches == 0
        and palette_stp_missing == 0
        else "FAIL"
    )
    proof = {
        "schema": "v8.n64-dreamland-terrain-atlas.v1",
        "status": status,
        "n64_source": (
            str(args.source.resolve()) if args.source else str(args.rom.resolve())
        ),
        "converted_source": (
            str(args.converted.resolve()) if args.converted else "fresh conversion"
        ),
        "source_dimensions": [texture.width, texture.height],
        "converted_dimensions": [width, height],
        "source_texels": len(texture.pixels),
        "source_archive_vs_decoder_mismatches": sum(
            a != b for a, b in zip(raw, texture.pixels)
        ),
        "converted_vs_archived_linear_mismatches": mismatches,
        "scaled_archive_vs_decoder_mismatches": archived_linear_mismatches,
        "runtime_layout_proof": str(args.runtime_layout_proof.resolve()),
        "runtime_layout_proof_passed": runtime_layout_passed,
        "palette_entries_compared": len(palette_pairs),
        "palette_rgb555_mismatches": palette_rgb_mismatches,
        "palette_visible_stp_missing": palette_stp_missing,
        "source_palette_rgb555_sha256": hashlib.sha256(
            bytes(channel for value in texture.palette_rgba5551
                  for channel in n64_rgb555(value))
        ).hexdigest(),
        "converted_palette_rgb555_sha256": hashlib.sha256(
            bytes(channel for value in converted_palette[:256]
                  for channel in ps1_rgb555(value))
        ).hexdigest(),
        "converted_texel_sha256": hashlib.sha256(actual).hexdigest(),
    }
    text = json.dumps(proof, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text, encoding="utf-8")
    print(text, end="")
    return 0 if status == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
