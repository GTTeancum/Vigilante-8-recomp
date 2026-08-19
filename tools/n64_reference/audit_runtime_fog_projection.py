#!/usr/bin/env python3
"""Decode the live N64 task projection and its F3DEX2 fog depth curve."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import struct


def host_u32(data: bytes, offset: int) -> int:
    """Mupen exposes N64 words in little-endian host-word byte order."""

    return int.from_bytes(data[offset:offset + 4], "little")


def canonical(data: bytes, address: int, size: int) -> bytes:
    source = data[address:address + size]
    if len(source) != size:
        raise ValueError(f"RDRAM range 0x{address:X}+0x{size:X} is truncated")
    return b"".join(
        source[offset:offset + 4][::-1]
        for offset in range(0, size, 4)
    )


def decode_mtx(rdram: bytes, address: int) -> list[list[float]]:
    payload = canonical(rdram, address & 0x7FFFFF, 64)
    values: list[float] = []
    for index in range(16):
        integer = int.from_bytes(
            payload[index * 2:index * 2 + 2], "big", signed=True
        )
        fraction = int.from_bytes(
            payload[32 + index * 2:34 + index * 2], "big"
        )
        values.append(((integer << 16) | fraction) / 65536.0)
    return [values[index:index + 4] for index in range(0, 16, 4)]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dmem", type=Path, required=True)
    parser.add_argument("--rdram", type=Path, required=True)
    parser.add_argument("--scale-shift", type=int, default=8)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    dmem = args.dmem.read_bytes()
    rdram = args.rdram.read_bytes()
    if len(dmem) != 0x1000:
        raise ValueError("RSP DMEM dump must be exactly 4096 bytes")
    task = 0xFC0
    data_ptr = host_u32(dmem, task + 0x30) & 0x7FFFFF
    data_size = host_u32(dmem, task + 0x34)
    if data_size <= 0 or data_ptr + data_size > len(rdram):
        raise ValueError("OSTask display-list range is invalid")

    projection_address: int | None = None
    fog_rgba: list[int] | None = None
    fog_multiplier: int | None = None
    fog_displacement: int | None = None
    for offset in range(data_ptr, data_ptr + data_size, 8):
        word0 = host_u32(rdram, offset)
        word1 = host_u32(rdram, offset + 4)
        opcode = word0 >> 24
        if opcode == 0xDA and (word0 & 1):
            projection_address = word1
        elif opcode == 0xF8:
            fog_rgba = [
                (word1 >> 24) & 0xFF,
                (word1 >> 16) & 0xFF,
                (word1 >> 8) & 0xFF,
                word1 & 0xFF,
            ]
        elif opcode == 0xDB and ((word0 >> 16) & 0xFF) == 0x08:
            fog_multiplier = struct.unpack(">h", word1.to_bytes(4, "big")[:2])[0]
            fog_displacement = struct.unpack(">h", word1.to_bytes(4, "big")[2:])[0]

    failures: list[str] = []
    if projection_address is None:
        failures.append("task root has no projection-matrix load")
    if fog_rgba is None:
        failures.append("task root has no RDP fog-color command")
    if fog_multiplier is None or fog_displacement is None:
        failures.append("task root has no F3DEX2 fog-factor move-word")

    matrix = decode_mtx(rdram, projection_address or 0) if projection_address else None
    projection_a = matrix[2][2] if matrix else None
    projection_b = matrix[3][2] if matrix else None
    near = (
        -projection_b / (projection_a + 1.0)
        if projection_a is not None and projection_b is not None else None
    )
    far = (
        -projection_b / (projection_a - 1.0)
        if projection_a is not None and projection_b is not None else None
    )
    scale = float(1 << args.scale_shift)

    minimum_per_mille = maximum_per_mille = None
    start_eye_z = end_eye_z = None
    if fog_multiplier not in (None, 0) and fog_displacement is not None:
        start_ndc = -fog_displacement / fog_multiplier
        end_ndc = (256.0 - fog_displacement) / fog_multiplier
        minimum_per_mille = 500.0 + 500.0 * start_ndc
        maximum_per_mille = 500.0 + 500.0 * end_ndc
        if projection_a is not None and projection_b is not None:
            start_eye_z = projection_b / (start_ndc - projection_a)
            end_eye_z = projection_b / (end_ndc - projection_a)

    expected = (
        fog_rgba == [254, 200, 127, 255]
        and fog_multiplier == 9846
        and fog_displacement == -9550
        and projection_a is not None
        and abs(projection_a - 1.014373779296875) < 1e-12
        and projection_b is not None
        and abs(projection_b - -1.0071868896484375) < 1e-12
    )
    if not expected:
        failures.append("live task does not match Dreamland's expected projection/fog state")

    proof = {
        "schema": "v8.n64-runtime-fog-projection.v1",
        "status": "PASS" if not failures else "FAIL",
        "failures": failures,
        "dmem": str(args.dmem.resolve()),
        "rdram": str(args.rdram.resolve()),
        "task_data_pointer": f"0x{data_ptr:08X}",
        "task_data_size": data_size,
        "projection_address": (
            f"0x{projection_address:08X}" if projection_address is not None else None
        ),
        "projection_matrix": matrix,
        "projection_a": projection_a,
        "projection_b": projection_b,
        "source_near_eye_z": near,
        "source_far_eye_z": far,
        "fog_rgba": fog_rgba,
        "fog_multiplier": fog_multiplier,
        "fog_displacement": fog_displacement,
        "fog_minimum_per_mille": minimum_per_mille,
        "fog_maximum_per_mille": maximum_per_mille,
        "fog_start_eye_z": start_eye_z,
        "fog_end_eye_z": end_eye_z,
        "converted_scale_shift": args.scale_shift,
        "converted_depth_scale": scale,
        "converted_near_view_z": near * scale if near is not None else None,
        "converted_far_view_z": far * scale if far is not None else None,
        "converted_fog_start_view_z": (
            start_eye_z * scale if start_eye_z is not None else None
        ),
        "converted_fog_end_view_z": (
            end_eye_z * scale if end_eye_z is not None else None
        ),
    }
    text = json.dumps(proof, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(text, encoding="utf-8")
    print(text, end="")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
