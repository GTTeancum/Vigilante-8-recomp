#!/usr/bin/env python3
"""Find reflected bitmap allocations and texture references in vehicle ISOPs."""

from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from extract_vehicle_lods import decode_bundle, find_models


def u16be(data: bytes, offset: int) -> int:
    return struct.unpack_from(">H", data, offset)[0]


def u32le(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def allocation_size(data: bytes, pointer: int, image_limit: int) -> int | None:
    if pointer < 4 or pointer >= image_limit or pointer % 4:
        return None
    count = u32le(data, pointer - 4)
    if count > image_limit - pointer:
        return None
    return count


def bitmap_candidates(data: bytes, image_limit: int) -> list[dict[str, object]]:
    """Scan the reflected 20-byte bitmap layout.

    bitmap = flags:u32, width:u16, height:u16, depth:u16, pad:u16,
             clut:pointer, texels:pointer
    """

    candidates = []
    for offset in range(0x20, image_limit - 20, 4):
        width = u16be(data, offset + 4)
        height = u16be(data, offset + 6)
        depth = u16be(data, offset + 8)
        if not (1 <= width <= 8192 and 1 <= height <= 8192):
            continue
        if depth not in (1, 2, 4, 8, 16, 24, 32, 64, 128):
            continue
        clut = u32le(data, offset + 12)
        texels = u32le(data, offset + 16)
        if clut and allocation_size(data, clut, image_limit) is None:
            continue
        texel_count = allocation_size(data, texels, image_limit)
        if texel_count is None or texel_count == 0:
            continue
        # Counts can be bytes, elements, or compressed-block units.  Keep a
        # broad but useful bound and report it for later format decoding.
        if texel_count > width * height * max(depth, 32):
            continue
        candidates.append(
            {
                "offset": offset,
                "flags_be": struct.unpack_from(">I", data, offset)[0],
                "width": width,
                "height": height,
                "depth": depth,
                "clut_offset": clut,
                "clut_count": allocation_size(data, clut, image_limit) if clut else 0,
                "texels_offset": texels,
                "texel_count": texel_count,
            }
        )
    return candidates


def xbox360_texture_candidates(
    data: bytes, image_limit: int, physical_offset: int
) -> list[dict[str, object]]:
    """Find 52-byte Xbox360Texture payloads containing a fetch constant."""

    candidates = []
    for offset in range(0x20, image_limit - 52, 4):
        dwords = struct.unpack_from(">6I", data, offset + 28)
        d0, d1, d2, d3, d4, d5 = dwords
        if (d0 & 3) != 2:
            continue
        pitch = (d0 >> 22) & 0x1FF
        tiled = (d0 >> 31) & 1
        texture_format = d1 & 0x3F
        endian = (d1 >> 6) & 3
        base_address = ((d1 >> 12) & 0xFFFFF) << 12
        width = (d2 & 0x1FFF) + 1
        height = ((d2 >> 13) & 0x1FFF) + 1
        dimension = (d5 >> 9) & 3
        mip_address = ((d5 >> 12) & 0xFFFFF) << 12
        if not (1 <= pitch <= 0x1FF and tiled == 1):
            continue
        if texture_format not in (6, 18, 19, 20, 49, 58, 59, 60):
            continue
        if endian not in (0, 1, 2, 3) or dimension not in (1, 3):
            continue
        if not (1 <= width <= 8192 and 1 <= height <= 8192):
            continue
        if physical_offset + base_address >= len(data):
            continue
        # The Isopod resource payload consistently begins with resource type 3
        # and one reference at +0/+4.  This removes accidental fetch-like data.
        if struct.unpack_from(">I", data, offset)[0] != 3:
            continue
        if struct.unpack_from(">I", data, offset + 4)[0] == 0:
            continue
        candidates.append(
            {
                "offset": offset,
                "width": width,
                "height": height,
                "pitch_pixels": pitch * 32,
                "tiled": bool(tiled),
                "format": texture_format,
                "endian": endian,
                "dimension": dimension,
                "base_address": base_address,
                "base_file_offset": physical_offset + base_address,
                "mip_address": mip_address,
                "fetch_dwords": dwords,
            }
        )
    return candidates


def referenced_offsets(data: bytes, models) -> set[int]:
    result: set[int] = set()
    for model in models:
        for submesh in model.submeshes:
            result.add(submesh.technique_offset)
            result.add(submesh.parameters_offset)
    return result


def inspect(path: Path) -> dict[str, object]:
    data, container = decode_bundle(path)
    image_limit = u32le(data, 8)
    models = find_models(data)
    bitmaps = bitmap_candidates(data, image_limit)
    xbox_textures = xbox360_texture_candidates(data, image_limit, image_limit)
    texture_offsets = {texture["offset"] for texture in xbox_textures}
    material_texture_refs = []
    referenced_texture_offsets: set[int] = set()
    for model in models:
        for submesh in model.submeshes:
            refs = []
            scan_end = min(submesh.parameters_offset + 160, image_limit)
            for scan in range(submesh.parameters_offset, scan_end - 3, 4):
                value = u32le(data, scan)
                if value in texture_offsets and value not in refs:
                    refs.append(value)
            referenced_texture_offsets.update(refs)
            material_texture_refs.append(
                {
                    "submesh_offset": submesh.offset,
                    "parameters_offset": submesh.parameters_offset,
                    "texture_offsets": refs,
                }
            )
    references = referenced_offsets(data, models)
    return {
        "source": str(path.resolve()),
        "container": container,
        "memory_image_size": image_limit,
        "bitmap_candidates": bitmaps,
        "xbox360_texture_candidates": xbox_textures,
        "referenced_texture_offsets": sorted(referenced_texture_offsets),
        "material_texture_refs": material_texture_refs,
        "model_technique_and_parameter_offsets": sorted(references),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path)
    parser.add_argument("--out", type=Path)
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None
    args = parser.parse_args(argv)
    text = json.dumps([inspect(path) for path in args.inputs], indent=2) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
