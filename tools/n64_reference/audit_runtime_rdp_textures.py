#!/usr/bin/env python3
"""Correlate runtime N64 triangles with loaded texture images and depth state.

The command capture is emitted in emulator-native word order.  This audit
restores canonical RDP byte order, follows SetTextureImage/SetTile/LoadBlock
state into TMEM, and records the texture image inherited by each triangle's
render tile.  The result is intended to distinguish runtime-built route strips
from translucent water/effect passes without relying on framebuffer colour.
"""

from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct

from audit_runtime_rdp_shade import (
    COMMAND_BYTES,
    SHADED_TRIANGLES,
    TRIANGLES,
    canonicalize,
    render_mode,
    shade_block,
    summarize,
)


def image_descriptor(words: tuple[int, ...]) -> dict[str, int]:
    return {
        "format": (words[0] >> 21) & 7,
        "size": (words[0] >> 19) & 3,
        "width": (words[0] & 0x3FF) + 1,
        "address": words[1] & 0x03FFFFFF,
    }


def source_xrtp_textures(arena: bytes, rdram: bytes) -> list[dict[str, object]]:
    """Find each N64 XRTP pixel payload in the captured host-order RDRAM."""
    found: list[dict[str, object]] = []

    def walk(start: int, end: int) -> None:
        offset = start
        while offset + 8 <= end:
            tag = arena[offset:offset + 4]
            size = struct.unpack_from(">I", arena, offset + 4)[0]
            body = offset + 8
            chunk_end = body + size
            if chunk_end > end:
                return
            if tag == b"FORM" and size >= 4:
                walk(body + 4, chunk_end)
            elif tag == b"XRTP" and size > 20:
                payload = arena[body:chunk_end]
                tex = payload[12:]
                fmt = tex[0]
                pixel_size = tex[1]
                palette_count = struct.unpack_from(">H", tex, 2)[0]
                width, height = struct.unpack_from(">HH", tex, 4)
                palette_end = (8 + palette_count * 2 + 7) & ~7
                row_bytes = (width * (4, 8, 16, 32)[pixel_size] + 7) // 8
                row_stride = (row_bytes + 7) & ~7
                pixels = tex[palette_end:palette_end + row_stride * height]
                host_pixels = b"".join(
                    pixels[index:index + 4][::-1]
                    for index in range(0, len(pixels), 4)
                )
                matches: list[int] = []
                cursor = 0
                while host_pixels:
                    match = rdram.find(host_pixels, cursor)
                    if match < 0:
                        break
                    matches.append(match)
                    cursor = match + 1
                found.append({
                    "xrtp_index": len(found),
                    "chunk_offset": offset,
                    "format": fmt,
                    "size": pixel_size,
                    "width": width,
                    "height": height,
                    "pixel_bytes": len(pixels),
                    "pixel_sha256": hashlib.sha256(pixels).hexdigest().upper(),
                    "rdram_addresses": [f"0x{address:08X}" for address in matches],
                })
            offset = chunk_end + (size & 1)

    walk(0, len(arena))
    return found


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--arena", type=Path)
    parser.add_argument("--rdram", type=Path)
    args = parser.parse_args()

    if (args.arena is None) != (args.rdram is None):
        parser.error("--arena and --rdram must be provided together")

    trace_path = args.capture / "rdp_command_trace.jsonl"
    records = [
        json.loads(line)
        for line in trace_path.read_text(encoding="utf-8").splitlines()
        if line
    ]
    records.sort(key=lambda item: (int(item["frame"]), int(item["dlist_sequence"])))

    current_image: tuple[int, int, int, int] | None = None
    combiner: tuple[int, int] | None = None
    other_mode: tuple[int, int] | None = None
    fog_color: tuple[int, int, int, int] | None = None
    environment_color: tuple[int, int, int, int] | None = None
    tile_tmem = [0] * 8
    tmem_images: dict[int, tuple[int, int, int, int]] = {}
    states: Counter[tuple[object, ...]] = Counter()
    state_shades: dict[tuple[object, ...], list[tuple[float, ...]]] = {}
    loads: Counter[tuple[object, ...]] = Counter()
    warnings: list[str] = []
    command_index = 0

    for record in records:
        filename = (
            f"n64_frame_{record['frame']}_rdp_{record['dlist_sequence']}_"
            f"{record['start']}_{record['end']}.bin"
        )
        path = args.capture / filename
        if not path.exists():
            warnings.append(f"missing command span: {filename}")
            continue
        data = canonicalize(path.read_bytes())
        cursor = 0
        while cursor < len(data):
            if cursor + 8 > len(data):
                warnings.append(f"{filename}: truncated command at {cursor}")
                break
            word0 = struct.unpack_from(">I", data, cursor)[0]
            opcode = (word0 >> 24) & 0x3F
            length = COMMAND_BYTES.get(opcode, 8)
            if cursor + length > len(data):
                warnings.append(
                    f"{filename}: opcode 0x{opcode:02X} at {cursor} needs {length} bytes"
                )
                break
            words = struct.unpack_from(f">{length // 4}I", data, cursor)
            if opcode == 0x3D:
                desc = image_descriptor(words)
                current_image = (
                    desc["address"], desc["format"], desc["size"], desc["width"]
                )
            elif opcode == 0x35:
                tile = (words[1] >> 24) & 7
                tile_tmem[tile] = words[0] & 0x1FF
            elif opcode in {0x30, 0x33, 0x34}:
                tile = (words[1] >> 24) & 7
                tmem = tile_tmem[tile]
                if current_image is not None:
                    tmem_images[tmem] = current_image
                loads[(
                    opcode,
                    tile,
                    tmem,
                    current_image,
                    words[0] & 0x00FFFFFF,
                    words[1] & 0x00FFFFFF,
                )] += 1
            elif opcode == 0x3C:
                combiner = (words[0], words[1])
            elif opcode == 0x2F:
                other_mode = (words[0] & 0x00FFFFFF, words[1])
            elif opcode == 0x38:
                fog_color = (
                    (words[1] >> 24) & 0xFF,
                    (words[1] >> 16) & 0xFF,
                    (words[1] >> 8) & 0xFF,
                    words[1] & 0xFF,
                )
            elif opcode == 0x3B:
                environment_color = (
                    (words[1] >> 24) & 0xFF,
                    (words[1] >> 16) & 0xFF,
                    (words[1] >> 8) & 0xFF,
                    words[1] & 0xFF,
                )

            if opcode in TRIANGLES:
                tile = (words[0] >> 16) & 7
                tmem = tile_tmem[tile]
                image = tmem_images.get(tmem)
                state = (
                    opcode,
                    tile,
                    tmem,
                    image,
                    combiner,
                    other_mode,
                    fog_color,
                    environment_color,
                )
                states[state] += 1
                if opcode in SHADED_TRIANGLES:
                    state_shades.setdefault(state, []).append(shade_block(words)["start"])

            cursor += length
            command_index += 1

    def image_json(image: tuple[int, int, int, int] | None) -> object:
        if image is None:
            return None
        address, fmt, size, width = image
        return {
            "address": f"0x{address:08X}",
            "format": fmt,
            "size": size,
            "width": width,
        }

    source_textures = (
        source_xrtp_textures(args.arena.read_bytes(), args.rdram.read_bytes())
        if args.arena is not None and args.rdram is not None
        else []
    )
    source_by_address = {
        int(address, 16): int(item["xrtp_index"])
        for item in source_textures
        for address in item["rdram_addresses"]
    }

    report = {
        "schema": "v8.n64-runtime-rdp-texture-state.v1",
        "capture": str(args.capture.resolve()),
        "commands": command_index,
        "triangle_states": [
            {
                "triangles": count,
                "triangle_opcode": f"0x{opcode:02X}",
                "tile": tile,
                "tmem": tmem,
                "texture_image": image_json(image),
                "source_xrtp_index": (
                    None if image is None else source_by_address.get(image[0])
                ),
                "shade_start": summarize(
                    state_shades.get(
                        (
                            opcode,
                            tile,
                            tmem,
                            image,
                            combine,
                            mode,
                            fog,
                            env_color,
                        ),
                        [],
                    )
                ),
                "fog_color": (
                    None if fog is None else list(fog)
                ),
                "environment_color": (
                    None if env_color is None else list(env_color)
                ),
                "combiner_word0": None if combine is None else f"0x{combine[0]:08X}",
                "combiner_word1": None if combine is None else f"0x{combine[1]:08X}",
                "other_mode_high": None if mode is None else f"0x{mode[0]:06X}",
                "other_mode_low": None if mode is None else f"0x{mode[1]:08X}",
                "render_mode": None if mode is None else render_mode(mode[1]),
            }
            for (
                opcode,
                tile,
                tmem,
                image,
                combine,
                mode,
                fog,
                env_color,
            ), count in states.most_common()
        ],
        "source_xrtp_textures": source_textures,
        "texture_loads": [
            {
                "loads": count,
                "opcode": f"0x{opcode:02X}",
                "tile": tile,
                "tmem": tmem,
                "texture_image": image_json(image),
                "upper_left_s": (load_word0 >> 12) & 0xFFF,
                "upper_left_t": load_word0 & 0xFFF,
                "lower_right_s": (load_word1 >> 12) & 0xFFF,
                "dxt_or_lower_right_t": load_word1 & 0xFFF,
            }
            for (
                opcode,
                tile,
                tmem,
                image,
                load_word0,
                load_word1,
            ), count in loads.most_common()
        ],
        "warnings": warnings,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({
        "commands": command_index,
        "triangle_states": report["triangle_states"],
        "warnings": len(warnings),
    }, indent=2))
    return 0 if not warnings else 1


if __name__ == "__main__":
    raise SystemExit(main())
