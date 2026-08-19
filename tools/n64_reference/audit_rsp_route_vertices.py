#!/usr/bin/env python3
"""Audit Dreamland's runtime-built N64 route vertices and colour ramp.

The hidden reference harness dumps RSP DMEM and RDRAM in host-word byte
order.  This tool reads the active OSTask display-list range, follows the
texture image through TMEM, and decodes every F3DEX2 vertex referenced while
XRTP 0 is selected.  It proves that the road colour is generated per vertex
from the arena COLS ramp instead of being a flat material tint.
"""

from __future__ import annotations

import argparse
from collections import Counter
import json
from pathlib import Path
import struct
import sys


TOOLS = Path(__file__).resolve().parents[1]
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import v8_n64_level as n64  # noqa: E402
from n64_reference.audit_runtime_rdp_textures import (  # noqa: E402
    source_xrtp_textures,
)


def host_u32(data: bytes, offset: int) -> int:
    """Read one canonical N64 word from Mupen's host-word-ordered dump."""

    return int.from_bytes(data[offset:offset + 4], "little")


def canonical(data: bytes, address: int, size: int) -> bytes:
    payload = data[address:address + size]
    if len(payload) != size:
        raise ValueError(f"RDRAM range 0x{address:X}+0x{size:X} is truncated")
    return b"".join(
        payload[offset:offset + 4][::-1]
        for offset in range(0, size, 4)
    )


def trunc_div(numerator: int, denominator: int) -> int:
    """Match signed MIPS/C integer division (truncate toward zero)."""

    magnitude = abs(numerator) // abs(denominator)
    return -magnitude if (numerator < 0) != (denominator < 0) else magnitude


def colour_ramp(low: tuple[int, int, int], high: tuple[int, int, int]) -> list[tuple[int, int, int]]:
    """Build the source's inclusive 32-entry COLS interpolation ramp."""

    return [
        tuple(
            low[channel]
            + trunc_div((high[channel] - low[channel]) * index, 31)
            for channel in range(3)
        )
        for index in range(32)
    ]


def arena_cols(arena: bytes) -> tuple[int, list[tuple[int, int, int, int]]]:
    matches = [
        (offset, payload)
        for offset, tag, payload, _parent in n64.iter_chunks(arena)
        if tag == b"COLS"
    ]
    if len(matches) != 1:
        raise n64.FormatError(f"expected one COLS chunk, found {len(matches)}")
    offset, payload = matches[0]
    if len(payload) != 28:
        raise n64.FormatError(f"COLS at 0x{offset:X} is {len(payload)} bytes, expected 28")
    return offset, [tuple(payload[index:index + 4]) for index in range(0, 28, 4)]


def decode_vertex(rdram: bytes, address: int) -> dict[str, object]:
    raw = canonical(rdram, address, 16)
    x, y, z, flag, s, t, red, green, blue, alpha = struct.unpack(
        ">hhhHhhBBBB", raw
    )
    return {
        "address": address,
        "position": (x, y, z),
        "flag": flag,
        "uv": (s, t),
        "rgba": (red, green, blue, alpha),
    }


def triangle_indices(opcode: int, word0: int, word1: int) -> tuple[tuple[int, int, int], ...]:
    if opcode == 0x05:
        return (((word0 >> 16) & 0xFF) >> 1,
                ((word0 >> 8) & 0xFF) >> 1,
                (word0 & 0xFF) >> 1),
    if opcode == 0x06:
        return (
            (((word0 >> 16) & 0xFF) >> 1,
             ((word0 >> 8) & 0xFF) >> 1,
             (word0 & 0xFF) >> 1),
            (((word1 >> 16) & 0xFF) >> 1,
             ((word1 >> 8) & 0xFF) >> 1,
             (word1 & 0xFF) >> 1),
        )
    return ()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--arena", type=Path, required=True)
    parser.add_argument("--dmem", type=Path, required=True)
    parser.add_argument("--rdram", type=Path, required=True)
    parser.add_argument("--xrtp-index", type=int, default=0)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    arena = args.arena.read_bytes()
    dmem = args.dmem.read_bytes()
    rdram = args.rdram.read_bytes()
    if len(dmem) != 0x1000:
        raise ValueError("RSP DMEM dump must be exactly 4096 bytes")

    task = 0xFC0
    display_start = host_u32(dmem, task + 0x30) & 0x7FFFFF
    display_size = host_u32(dmem, task + 0x34)
    display_end = display_start + display_size
    if display_size <= 0 or display_end > len(rdram) or display_size % 8:
        raise ValueError("OSTask display-list range is invalid")

    xrtp = source_xrtp_textures(arena, rdram)
    if not 0 <= args.xrtp_index < len(xrtp):
        raise ValueError(f"XRTP index {args.xrtp_index} is outside 0..{len(xrtp) - 1}")
    target_addresses = {
        int(value, 16) for value in xrtp[args.xrtp_index]["rdram_addresses"]
    }

    cols_offset, cols = arena_cols(arena)
    ramp_low = cols[3][:3]
    ramp_high = cols[4][:3]
    ramp = colour_ramp(ramp_low, ramp_high)
    ramp_index = {value: index for index, value in enumerate(ramp)}

    current_image: int | None = None
    combiner: tuple[int, int] | None = None
    other_mode_low: tuple[int, int] | None = None
    environment: tuple[int, int, int, int] | None = None
    render_tile = 0
    tile_tmem = [0] * 8
    tmem_images: dict[int, int] = {}
    vertex_cache: dict[int, dict[str, object]] = {}
    referenced: Counter[int] = Counter()
    triangles: list[dict[str, object]] = []
    route_states: Counter[tuple[object, ...]] = Counter()
    failures: list[str] = []
    warnings: list[str] = []

    for command_address in range(display_start, display_end, 8):
        word0 = host_u32(rdram, command_address)
        word1 = host_u32(rdram, command_address + 4)
        opcode = word0 >> 24

        if opcode == 0xFD:
            current_image = word1 & 0x03FFFFFF
        elif opcode == 0xF5:
            tile_tmem[(word1 >> 24) & 7] = word0 & 0x1FF
        elif opcode in {0xF0, 0xF3, 0xF4}:
            tile = (word1 >> 24) & 7
            if current_image is not None:
                tmem_images[tile_tmem[tile]] = current_image
        elif opcode == 0xD7:
            render_tile = (word0 >> 8) & 7
        elif opcode == 0xFC:
            combiner = (word0, word1)
        elif opcode == 0xE2:
            other_mode_low = (word0, word1)
        elif opcode == 0xFB:
            environment = (
                (word1 >> 24) & 0xFF,
                (word1 >> 16) & 0xFF,
                (word1 >> 8) & 0xFF,
                word1 & 0xFF,
            )
        elif opcode == 0x01:
            count = (word0 >> 12) & 0xFF
            end_index = (word0 >> 1) & 0x7F
            first_index = end_index - count
            source = word1 & 0x7FFFFF
            if count <= 0 or first_index < 0 or end_index > 32:
                warnings.append(
                    f"invalid G_VTX at 0x{command_address:08X}: "
                    f"count={count} first={first_index} end={end_index}"
                )
                continue
            for index in range(count):
                vertex_cache[first_index + index] = decode_vertex(
                    rdram, source + index * 16
                )

        if opcode not in {0x05, 0x06}:
            continue
        texture = tmem_images.get(tile_tmem[render_tile])
        if texture not in target_addresses:
            continue
        state = (combiner, other_mode_low, environment, render_tile, texture)
        command_triangles = triangle_indices(opcode, word0, word1)
        route_states[state] += len(command_triangles)
        for indices in command_triangles:
            missing = [index for index in indices if index not in vertex_cache]
            if missing:
                failures.append(
                    f"triangle at 0x{command_address:08X} references missing "
                    f"vertex-cache indices {missing}"
                )
                continue
            vertices = [vertex_cache[index] for index in indices]
            for vertex in vertices:
                referenced[int(vertex["address"])] += 1
            triangles.append({
                "command_address": f"0x{command_address:08X}",
                "cache_indices": list(indices),
                "vertex_addresses": [
                    f"0x{int(vertex['address']):08X}" for vertex in vertices
                ],
            })

    unique_vertices = [
        vertex
        for address, vertex in sorted(
            (address, decode_vertex(rdram, address)) for address in referenced
        )
    ]
    colour_uses: Counter[tuple[int, int, int, int]] = Counter()
    colour_vertices: Counter[tuple[int, int, int, int]] = Counter()
    gradient_indices: Counter[int] = Counter()
    vertex_records: list[dict[str, object]] = []
    for vertex in unique_vertices:
        rgba = tuple(vertex["rgba"])
        rgb = rgba[:3]
        index = ramp_index.get(rgb)
        colour_vertices[rgba] += 1
        colour_uses[rgba] += referenced[int(vertex["address"])]
        if index is None:
            failures.append(
                f"vertex 0x{int(vertex['address']):08X} colour {rgba} "
                "is not in the COLS ramp"
            )
        else:
            gradient_indices[index] += 1
        vertex_records.append({
            "address": f"0x{int(vertex['address']):08X}",
            "position": list(vertex["position"]),
            "uv": list(vertex["uv"]),
            "rgba": list(rgba),
            "gradient_index": index,
            "triangle_references": referenced[int(vertex["address"])],
        })

    expected_combiner = (0xFC55FE04, 0x1FFCF3F8)
    expected_mode = (0xE200001C, 0x0C193078)
    expected_environment = (*cols[2][:3], 255)
    if not target_addresses:
        failures.append("XRTP pixel payload was not found in captured RDRAM")
    if not triangles:
        failures.append("no triangles used the selected XRTP texture")
    if any(rgba[3] != 255 for rgba in colour_vertices):
        failures.append("route vertex alpha is not uniformly opaque")
    if any(state[0] != expected_combiner for state in route_states):
        failures.append("route combiner differs from FC55FE04/1FFCF3F8")
    if any(state[1] != expected_mode for state in route_states):
        failures.append("route other-mode differs from E200001C/0C193078")
    if any(state[2] != expected_environment for state in route_states):
        failures.append("route environment colour differs from COLS word 2")

    def state_json(state: tuple[object, ...], count: int) -> dict[str, object]:
        combine, mode, env, tile, texture = state
        return {
            "triangles": count,
            "combiner_word0": None if combine is None else f"0x{combine[0]:08X}",
            "combiner_word1": None if combine is None else f"0x{combine[1]:08X}",
            "other_mode_command": None if mode is None else f"0x{mode[0]:08X}",
            "other_mode_low": None if mode is None else f"0x{mode[1]:08X}",
            "environment_rgba": None if env is None else list(env),
            "render_tile": tile,
            "texture_address": f"0x{texture:08X}",
        }

    report = {
        "schema": "v8.n64-runtime-rsp-route-vertices.v1",
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "warnings": warnings,
        "arena": str(args.arena.resolve()),
        "dmem": str(args.dmem.resolve()),
        "rdram": str(args.rdram.resolve()),
        "task_display_list": {
            "start": f"0x{display_start:08X}",
            "size": display_size,
            "end": f"0x{display_end:08X}",
        },
        "source_xrtp_index": args.xrtp_index,
        "source_xrtp": xrtp[args.xrtp_index],
        "matched_texture_addresses": [
            f"0x{address:08X}" for address in sorted(target_addresses)
        ],
        "cols": {
            "chunk_offset": f"0x{cols_offset:08X}",
            "rgba_words": [list(value) for value in cols],
            "environment_rgba": list(expected_environment),
            "route_ramp_low_rgb": list(ramp_low),
            "route_ramp_high_rgb": list(ramp_high),
            "interpolation": "low + trunc_toward_zero((high-low)*index/31)",
            "ramp_rgb": [list(value) for value in ramp],
        },
        "rsp_triangles": len(triangles),
        "triangle_vertex_references": sum(referenced.values()),
        "unique_vertex_addresses": len(unique_vertices),
        "unique_vertex_colours": len(colour_vertices),
        "gradient_indices_present": [
            {"index": index, "vertices": count, "rgb": list(ramp[index])}
            for index, count in sorted(gradient_indices.items())
        ],
        "colour_usage": [
            {
                "rgba": list(rgba),
                "unique_vertices": colour_vertices[rgba],
                "triangle_references": count,
                "gradient_index": ramp_index.get(rgba[:3]),
            }
            for rgba, count in colour_uses.most_common()
        ],
        "route_states": [
            state_json(state, count) for state, count in route_states.most_common()
        ],
        "triangles": triangles,
        "vertices": vertex_records,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({
        "status": report["status"],
        "rsp_triangles": report["rsp_triangles"],
        "unique_vertex_addresses": report["unique_vertex_addresses"],
        "unique_vertex_colours": report["unique_vertex_colours"],
        "gradient_indices_present": report["gradient_indices_present"],
        "route_states": report["route_states"],
        "failures": failures,
        "warnings": warnings,
    }, indent=2))
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
