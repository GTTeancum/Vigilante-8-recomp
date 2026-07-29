#!/usr/bin/env python3
"""Match V8 N64 global SFX IDs to original PlayStation MAIN.SND entries.

The N64 port uses a 198-entry N64 PtrTablesV2 bank while the PlayStation
release uses a 66-entry SPU ADPCM bank.  This tool decodes both banks, trims
encoder padding, and compares sample length plus normalized magnitude spectra.
It is intentionally an evidence generator; it does not accept ordinal or
nearest-number guesses.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import struct

import numpy as np

from v8_n64_level import V8N64Rom, be32


N64_SFX_POINTER_TABLE_ROM_OFFSET = 0x52FA0
N64_SFX_WAVE_TABLE_ROM_OFFSET = 0x110940
N64_ARENA_SOUND_IDS = (0x45, 0x46, 0x49, 0x4A, 0x4B, 0x4C,
                       0x4F, 0x50, 0x8D, 0x8E, 0x97, 0xA6)


def signed_nibble(value: int) -> int:
    return value - 16 if value > 7 else value


def decode_n64_sample(rom: bytes, sample_id: int) -> np.ndarray:
    bank = N64_SFX_POINTER_TABLE_ROM_OFFSET
    count = be32(rom, bank + 0x20)
    if not 0 <= sample_id < count:
        raise ValueError(f"N64 SFX ID {sample_id} is outside {count} entries")
    pointer_array = bank + be32(rom, bank + 0x2C)
    sample = bank + be32(rom, pointer_array + sample_id * 4)
    wave_offset = be32(rom, sample)
    wave_size = be32(rom, sample + 4)
    book = bank + be32(rom, sample + 0x10)
    order = be32(rom, book)
    predictor_count = be32(rom, book + 4)
    coefficient_count = order * predictor_count * 8
    coefficients = np.frombuffer(
        rom,
        dtype=">i2",
        count=coefficient_count,
        offset=book + 8,
    ).astype(np.int64).reshape(predictor_count, order, 8)
    encoded = rom[
        N64_SFX_WAVE_TABLE_ROM_OFFSET + wave_offset:
        N64_SFX_WAVE_TABLE_ROM_OFFSET + wave_offset + wave_size
    ]

    state = np.zeros(8, dtype=np.int64)
    decoded: list[int] = []
    for frame_offset in range(0, len(encoded) - 8, 9):
        frame = encoded[frame_offset:frame_offset + 9]
        scale = frame[0] >> 4
        predictor_index = frame[0] & 0x0F
        if predictor_index >= predictor_count:
            raise ValueError(
                f"N64 SFX 0x{sample_id:02X} uses predictor "
                f"{predictor_index}/{predictor_count}"
            )
        predictor = coefficients[predictor_index]
        for vector_index in range(2):
            accumulators = np.zeros(8, dtype=np.int64)
            for coefficient_index in range(order):
                accumulators += (
                    state[8 - order + coefficient_index] *
                    predictor[coefficient_index]
                )
            residuals = [
                signed_nibble(nibble)
                for byte in frame[
                    1 + vector_index * 4:5 + vector_index * 4
                ]
                for nibble in (byte >> 4, byte & 0x0F)
            ]
            for sample_index, residual_nibble in enumerate(residuals):
                residual = residual_nibble * (1 << scale)
                accumulators[sample_index] += residual << 11
                for following in range(7 - sample_index):
                    accumulators[sample_index + following + 1] += (
                        residual * predictor[-1, following]
                    )
            vector = np.clip(
                accumulators >> 11, -32768, 32767
            ).astype(np.int64)
            decoded.extend(int(value) for value in vector)
            state[:] = vector
    return np.asarray(decoded, dtype=np.float64)


def parse_ps1_bank(data: bytes) -> list[tuple[np.ndarray, int]]:
    count, size_in_8b = struct.unpack_from("<HH", data)
    entries = [
        struct.unpack_from("<HH", data, 4 + index * 4)
        for index in range(count)
    ]
    payload = 4 + count * 4
    filters = ((0, 0), (60, 0), (115, -52), (98, -55), (122, -60))
    result: list[tuple[np.ndarray, int]] = []
    for index, (offset_in_8b, pitch) in enumerate(entries):
        end_in_8b = (
            entries[index + 1][0] if index + 1 < count else size_in_8b
        )
        encoded = data[
            payload + offset_in_8b * 8:payload + end_in_8b * 8
        ]
        previous_1 = 0
        previous_2 = 0
        decoded: list[int] = []
        for frame_offset in range(0, len(encoded) - 15, 16):
            frame = encoded[frame_offset:frame_offset + 16]
            shift = frame[0] & 0x0F
            filter_index = frame[0] >> 4
            if filter_index >= len(filters):
                raise ValueError(
                    f"PS1 SFX {index} uses filter {filter_index}"
                )
            filter_1, filter_2 = filters[filter_index]
            for byte in frame[2:]:
                for nibble in (byte & 0x0F, byte >> 4):
                    sample = (signed_nibble(nibble) << 12) >> shift
                    sample += (
                        previous_1 * filter_1 +
                        previous_2 * filter_2 + 32
                    ) >> 6
                    sample = max(-32768, min(32767, sample))
                    decoded.append(sample)
                    previous_2, previous_1 = previous_1, sample
            if frame[1] & 1:
                break
        result.append((np.asarray(decoded, dtype=np.float64), pitch))
    return result


def trim_padding(samples: np.ndarray) -> np.ndarray:
    if samples.size == 0:
        return samples
    threshold = max(16.0, float(np.max(np.abs(samples))) * 0.003)
    audible = np.flatnonzero(np.abs(samples) > threshold)
    if audible.size == 0:
        return samples[:0]
    return samples[audible[0]:audible[-1] + 1]


def spectral_similarity(left: np.ndarray, right: np.ndarray) -> float:
    sample_count = min(left.size, right.size)
    if sample_count < 2:
        return 0.0
    window = np.hanning(sample_count)
    left_spectrum = np.abs(np.fft.rfft(left[:sample_count] * window))
    right_spectrum = np.abs(np.fft.rfft(right[:sample_count] * window))
    denominator = (
        np.linalg.norm(left_spectrum) * np.linalg.norm(right_spectrum)
    )
    return (
        float(np.dot(left_spectrum, right_spectrum) / denominator)
        if denominator else 0.0
    )


def match(
    rom: bytes,
    ps1_bank: list[tuple[np.ndarray, int]],
    sample_id: int,
) -> dict[str, object]:
    source = trim_padding(decode_n64_sample(rom, sample_id))
    candidates: list[dict[str, object]] = []
    for ps1_id, (decoded, pitch) in enumerate(ps1_bank):
        target = trim_padding(decoded)
        length_delta = abs(source.size - target.size)
        similarity = spectral_similarity(source, target)
        candidates.append({
            "ps1_id": ps1_id,
            "ps1_id_hex": f"0x{ps1_id:02X}",
            "pitch": pitch,
            "n64_samples": int(source.size),
            "ps1_samples": int(target.size),
            "length_delta": int(length_delta),
            "spectral_similarity": round(similarity, 9),
        })
    candidates.sort(
        key=lambda item: (
            item["length_delta"],
            -item["spectral_similarity"],
        )
    )
    best = candidates[0]
    return {
        "n64_id": sample_id,
        "n64_id_hex": f"0x{sample_id:02X}",
        "match": best,
        "runner_up": candidates[1],
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("main_snd", type=Path)
    parser.add_argument(
        "--ids",
        nargs="*",
        type=lambda value: int(value, 0),
        default=list(N64_ARENA_SOUND_IDS),
    )
    parser.add_argument("--json", type=Path)
    args = parser.parse_args()

    rom = V8N64Rom(args.rom).data
    ps1_bank = parse_ps1_bank(args.main_snd.read_bytes())
    results = [match(rom, ps1_bank, sample_id) for sample_id in args.ids]
    for result in results:
        best = result["match"]
        assert isinstance(best, dict)
        print(
            f"{result['n64_id_hex']} -> {best['ps1_id_hex']} "
            f"samples={best['n64_samples']}/{best['ps1_samples']} "
            f"delta={best['length_delta']} "
            f"spectrum={best['spectral_similarity']:.6f}"
        )
    if args.json is not None:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(
            json.dumps(results, indent=2) + "\n", encoding="utf-8"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
