#!/usr/bin/env python3
"""Extract V8:2 Dreamcast XBMP terrain textures without VRAM capture.

The outer level EXP is big-endian EA-IFF.  Its XBMP payload starts with the
authored atlas dimensions, one little-endian CL32 palette, then one 64x64
PAL8_TWIDDLED_MIPMAP PVRT per terrain cell.  PVR mip data is stored from the
smallest level to the largest, with the three-byte PAL8 prefix used by the
retail assets.

The default output is the native 64-pixel-per-cell atlas.  ``--cell-size`` can
resample every decoded cell independently for renderer diagnostics or for an
HD sidecar; cells are never filtered across one another.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import struct

from PIL import Image


LEVELS = (1, 2, 4, 8, 16, 32, 64)


def morton2(x: int, y: int) -> int:
    result = 0
    bit = 0
    while (1 << bit) <= max(x, y):
        result |= ((y >> bit) & 1) << (2 * bit)
        result |= ((x >> bit) & 1) << (2 * bit + 1)
        bit += 1
    return result


def top_level_chunk(data: bytes, wanted: bytes) -> bytes:
    if len(data) < 12 or data[:4] != b"FORM":
        raise ValueError("input is not an EA-IFF FORM")
    form_end = min(len(data), 8 + struct.unpack_from(">I", data, 4)[0])
    offset = 12
    matches: list[bytes] = []
    while offset + 8 <= form_end:
        tag = data[offset:offset + 4]
        size = struct.unpack_from(">I", data, offset + 4)[0]
        start = offset + 8
        end = start + size
        if end > form_end:
            raise ValueError(f"truncated top-level chunk at {offset:#x}")
        if tag == wanted:
            matches.append(data[start:end])
        offset = end + (size & 1)
    if len(matches) != 1:
        raise ValueError(
            f"expected exactly one {wanted.decode('ascii')} chunk, got {len(matches)}"
        )
    return matches[0]


def decode_xbmp(data: bytes) -> tuple[int, int, list[Image.Image]]:
    body = top_level_chunk(data, b"XBMP")
    if len(body) < 16:
        raise ValueError("XBMP is truncated")
    atlas_width, atlas_height = struct.unpack_from(">II", body, 0)
    if atlas_width % 64 or atlas_height % 64:
        raise ValueError(
            f"XBMP atlas is not a 64-pixel tile grid: {atlas_width}x{atlas_height}"
        )

    if body[8:12] != b"CL32":
        raise ValueError("XBMP does not begin with CL32 after its dimensions")
    cl32_size = struct.unpack_from("<I", body, 12)[0]
    color_count = struct.unpack_from("<H", body, 20)[0]
    if color_count != 256 or cl32_size < 8 + color_count * 4:
        raise ValueError(
            f"expected a complete 256-color XBMP palette, got {color_count}"
        )
    palette = [tuple(body[24 + i * 4:28 + i * 4]) for i in range(color_count)]

    offset = 8 + 8 + cl32_size
    textures: list[Image.Image] = []
    expected_count = atlas_width // 64 * (atlas_height // 64)
    while offset + 16 <= len(body):
        if body[offset:offset + 4] != b"PVRT":
            raise ValueError(f"expected PVRT at XBMP+{offset:#x}")
        pvrt_size = struct.unpack_from("<I", body, offset + 4)[0]
        pixel_format = body[offset + 8]
        data_format = body[offset + 9]
        width, height = struct.unpack_from("<HH", body, offset + 12)
        if (pixel_format, data_format, width, height) != (0, 8, 64, 64):
            raise ValueError(
                "expected 64x64 ARGB1555/PAL8_TWIDDLED_MIPMAP, got "
                f"{pixel_format:02x}/{data_format:02x} {width}x{height}"
            )
        end = offset + 8 + pvrt_size
        if end > len(body):
            raise ValueError(f"truncated PVRT at XBMP+{offset:#x}")
        pixels = body[offset + 16:end]
        base_offset = 3 + sum(level * level for level in LEVELS[:-1])
        if len(pixels) < base_offset + 64 * 64:
            raise ValueError(f"short PAL8 mip chain at XBMP+{offset:#x}")
        rgba = [
            palette[pixels[base_offset + morton2(x, y)]]
            for y in range(64)
            for x in range(64)
        ]
        texture = Image.new("RGBA", (64, 64))
        texture.putdata(rgba)
        textures.append(texture)
        offset = end

    if len(textures) != expected_count:
        raise ValueError(
            f"XBMP declares {expected_count} cells but contains {len(textures)} PVRTs"
        )
    return atlas_width, atlas_height, textures


def build_atlas(
    native_width: int,
    native_height: int,
    textures: list[Image.Image],
    cell_size: int,
) -> Image.Image:
    columns = native_width // 64
    rows = native_height // 64
    atlas = Image.new("RGBA", (columns * cell_size, rows * cell_size))
    resampling = Image.Resampling.LANCZOS
    for index, texture in enumerate(textures):
        cell = texture if cell_size == 64 else texture.resize(
            (cell_size, cell_size), resampling
        )
        atlas.paste(
            cell,
            ((index % columns) * cell_size, (index // columns) * cell_size),
        )
    return atlas


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--cell-size", type=int, default=64)
    args = parser.parse_args()
    if args.cell_size <= 0 or args.cell_size > 1024:
        parser.error("--cell-size must be in 1..1024")

    source = args.input.read_bytes()
    native_width, native_height, textures = decode_xbmp(source)
    atlas = build_atlas(
        native_width, native_height, textures, args.cell_size
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    atlas.save(args.output)
    print(
        f"XBMP {native_width}x{native_height}, {len(textures)} authored "
        f"64x64 mipmapped cells -> {atlas.width}x{atlas.height} {args.output}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
