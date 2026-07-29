#!/usr/bin/env python3
"""Append Super Dreamland 64's unique sounds to a native V8 PS1 SND bank.

The original PlayStation entries are preserved byte-for-byte and retain their
indices.  Dreamland's eight N64-exclusive samples are decoded from the retail
ROM, encoded as native SPU ADPCM, and appended as entries 66 through 73.
No ROM data or N64 codec payload is copied to the output.
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
import struct

import numpy as np

from map_v8_n64_ps1_sound_ids import (
    N64_SFX_POINTER_TABLE_ROM_OFFSET,
    decode_n64_sample,
)
from v8_n64_level import V8N64Rom, be32


SOURCE_IDS = (
    0x45, 0x46, 0x48, 0x49, 0x4A,
    0x4B, 0x4C, 0x4E, 0x4F, 0x50, 0x5F,
)
FILTERS = ((0, 0), (60, 0), (115, -52), (98, -55), (122, -60))


def clamp16(value: int) -> int:
    return max(-32768, min(32767, value))


def signed_byte(value: int) -> int:
    return value - 256 if value >= 128 else value


def encode_frame(
    samples: np.ndarray, previous_1: int, previous_2: int
) -> tuple[bytes, int, int, float]:
    """Encode one 28-sample SPU ADPCM frame by exhaustive predictor search."""
    if samples.size != 28:
        raise ValueError("SPU frames contain exactly 28 samples")

    best: tuple[float, int, int, list[int], int, int] | None = None
    for filter_index, (filter_1, filter_2) in enumerate(FILTERS):
        for shift in range(13):
            history_1 = previous_1
            history_2 = previous_2
            nibbles: list[int] = []
            error = 0.0
            scale = 1 << shift
            for source in samples:
                prediction = (
                    history_1 * filter_1 + history_2 * filter_2 + 32
                ) >> 6
                residual = int(source) - prediction
                quantized = int(round(residual * scale / 4096.0))
                quantized = max(-8, min(7, quantized))
                decoded = clamp16((quantized << 12) // scale + prediction)
                difference = int(source) - decoded
                error += float(difference * difference)
                nibbles.append(quantized & 0x0F)
                history_2, history_1 = history_1, decoded
            candidate = (
                error, filter_index, shift, nibbles, history_1, history_2
            )
            if best is None or candidate[:3] < best[:3]:
                best = candidate

    assert best is not None
    error, filter_index, shift, nibbles, history_1, history_2 = best
    frame = bytearray(16)
    frame[0] = (filter_index << 4) | shift
    for index in range(14):
        frame[2 + index] = nibbles[index * 2] | (
            nibbles[index * 2 + 1] << 4
        )
    return bytes(frame), history_1, history_2, error


def encode_spu_adpcm(
    samples: np.ndarray, loop_start: int | None
) -> tuple[bytes, dict[str, object]]:
    source = np.asarray(
        np.clip(np.rint(samples), -32768, 32767), dtype=np.int64
    )
    frame_count = max(1, math.ceil(source.size / 28))
    padded = np.pad(source, (0, frame_count * 28 - source.size))

    # Retail V8 banks put one silent frame before every sample.  It primes the
    # SPU predictor history and makes a zero loop point addressable.
    output = bytearray(16)
    previous_1 = 0
    previous_2 = 0
    total_error = 0.0
    encoded_frames: list[bytearray] = []
    for index in range(frame_count):
        frame, previous_1, previous_2, error = encode_frame(
            padded[index * 28:(index + 1) * 28],
            previous_1,
            previous_2,
        )
        encoded_frames.append(bytearray(frame))
        total_error += error

    loop_frame: int | None = None
    if loop_start is None:
        encoded_frames[-1][1] = 1
    else:
        # Match the retail V8 N64-to-PS1 assets: all playable frames have the
        # repeat bit, the nearest source frame gets loop-start, and the final
        # frame combines end+repeat.
        loop_frame = max(1, int(round(loop_start / 28.0)))
        loop_frame = min(loop_frame, len(encoded_frames))
        for frame in encoded_frames:
            frame[1] = 2
        encoded_frames[loop_frame - 1][1] = 6
        encoded_frames[-1][1] = 3

    for frame in encoded_frames:
        output.extend(frame)
    mse = total_error / max(1, source.size)
    return bytes(output), {
        "source_sample_count": int(source.size),
        "spu_frame_count": len(encoded_frames) + 1,
        "loop_source_sample": loop_start,
        "loop_spu_frame": loop_frame,
        "mean_squared_error": round(mse, 3),
    }


def sample_metadata(rom: bytes, sample_id: int) -> dict[str, int | None]:
    bank = N64_SFX_POINTER_TABLE_ROM_OFFSET
    pointer_array = bank + be32(rom, bank + 0x2C)
    wave = bank + be32(rom, pointer_array + sample_id * 4)
    loop_offset = be32(rom, wave + 0x0C)
    loop_start = (
        be32(rom, bank + loop_offset) if loop_offset != 0 else None
    )
    semitone_table = bank + be32(rom, bank + 0x24)
    detune_table = bank + be32(rom, bank + 0x28)
    semitones = signed_byte(rom[semitone_table + sample_id])
    cents = signed_byte(rom[detune_table + sample_id * 4])
    # SPU pitch 0x1000 is 44.1 kHz. V8's authored baseline (-12 semitones)
    # therefore yields 0x400 (11.025 kHz). The retail values use truncation.
    pitch = int(
        1024.0 * 2.0 ** ((semitones + 12.0 + cents / 100.0) / 12.0)
    )
    return {
        "semitones": semitones,
        "cents": cents,
        "spu_pitch": pitch,
        "loop_start": loop_start,
    }


def parse_bank(data: bytes) -> tuple[list[tuple[int, int]], bytes]:
    count, size_in_8b = struct.unpack_from("<HH", data)
    header_size = 4 + count * 4
    expected_size = header_size + size_in_8b * 8
    if expected_size != len(data):
        raise ValueError(
            f"SND length mismatch: header describes {expected_size}, "
            f"file contains {len(data)}"
        )
    entries = [
        struct.unpack_from("<HH", data, 4 + index * 4)
        for index in range(count)
    ]
    return entries, data[header_size:]


def build(
    rom: bytes, source_bank: bytes
) -> tuple[bytes, dict[str, object]]:
    entries, payload = parse_bank(source_bank)
    if len(entries) != 66:
        raise ValueError(
            f"expected the retail 66-entry MAIN.SND, got {len(entries)}"
        )

    appended = bytearray()
    report_entries: list[dict[str, object]] = []
    for sample_id in SOURCE_IDS:
        metadata = sample_metadata(rom, sample_id)
        encoded, quality = encode_spu_adpcm(
            decode_n64_sample(rom, sample_id),
            metadata["loop_start"],
        )
        offset_in_8b = (len(payload) + len(appended)) // 8
        entries.append((offset_in_8b, int(metadata["spu_pitch"])))
        ps1_id = len(entries) - 1
        appended.extend(encoded)
        report_entries.append({
            "n64_id": sample_id,
            "n64_id_hex": f"0x{sample_id:02X}",
            "ps1_id": ps1_id,
            "ps1_id_hex": f"0x{ps1_id:02X}",
            "encoded_bytes": len(encoded),
            **metadata,
            **quality,
        })

    new_payload = payload + bytes(appended)
    if len(new_payload) % 8:
        raise AssertionError("SPU ADPCM payload must stay 8-byte aligned")
    size_in_8b = len(new_payload) // 8
    if size_in_8b > 0xFFFF:
        raise ValueError("extended SND exceeds its 16-bit size field")
    header = struct.pack("<HH", len(entries), size_in_8b)
    header += b"".join(struct.pack("<HH", *entry) for entry in entries)
    output = header + new_payload
    report: dict[str, object] = {
        "original_entry_count": 66,
        "extended_entry_count": len(entries),
        "original_payload_bytes": len(payload),
        "appended_payload_bytes": len(appended),
        "extended_payload_bytes": len(new_payload),
        "original_entries_preserved": entries[:66] == parse_bank(source_bank)[0],
        "original_payload_preserved": new_payload[:len(payload)] == payload,
        "entries": report_entries,
    }
    return output, report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("source_snd", type=Path)
    parser.add_argument("output_snd", type=Path)
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()

    rom = V8N64Rom(args.rom).data
    output, report = build(rom, args.source_snd.read_bytes())
    args.output_snd.parent.mkdir(parents=True, exist_ok=True)
    args.output_snd.write_bytes(output)
    if args.report is not None:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(
            json.dumps(report, indent=2) + "\n", encoding="utf-8"
        )
    print(
        f"Wrote {args.output_snd}: "
        f"{report['original_entry_count']} -> "
        f"{report['extended_entry_count']} entries, "
        f"+{report['appended_payload_bytes']} bytes"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
