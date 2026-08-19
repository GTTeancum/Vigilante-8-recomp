#!/usr/bin/env python3
"""Audit final shaded-triangle values from hidden N64 RDP command captures.

The CXD4 trace proxy writes emulator-native, host-word-swapped command spans.
This tool restores canonical RDP byte order, processes spans in submission order,
keeps inherited combiner state across spans, and decodes the 16.16 shade block
carried by every shaded triangle command.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import struct


COMMAND_BYTES = {
    0x08: 32,
    0x09: 48,
    0x0A: 96,
    0x0B: 112,
    0x0C: 96,
    0x0D: 112,
    0x0E: 160,
    0x0F: 176,
    0x24: 16,
    0x25: 16,
}
SHADED_TRIANGLES = {0x0C, 0x0D, 0x0E, 0x0F}
TRIANGLES = set(range(0x08, 0x10))


def canonicalize(data: bytes) -> bytes:
    if len(data) % 4:
        raise ValueError(f"RDP span length {len(data)} is not word-aligned")
    return b"".join(data[offset:offset + 4][::-1] for offset in range(0, len(data), 4))


def signed32(value: int) -> int:
    return value - 0x100000000 if value & 0x80000000 else value


def fixed16(value: int) -> float:
    return signed32(value) / 65536.0


def packed_rgba(words: tuple[int, ...], high: int, low: int) -> tuple[float, ...]:
    """Decode RDP's split RGBA 16.16 lanes from two pairs of words."""
    return (
        fixed16((words[high] & 0xFFFF0000) | ((words[low] >> 16) & 0xFFFF)),
        fixed16(((words[high] << 16) & 0xFFFF0000) | (words[low] & 0xFFFF)),
        fixed16((words[high + 1] & 0xFFFF0000) | ((words[low + 1] >> 16) & 0xFFFF)),
        fixed16(((words[high + 1] << 16) & 0xFFFF0000) | (words[low + 1] & 0xFFFF)),
    )


def shade_block(words: tuple[int, ...]) -> dict[str, tuple[float, ...]]:
    # The edge block is words 0..7.  Each following pair stores the high
    # halves for R/G and B/A, with the paired low halves four words later.
    return {
        "start": packed_rgba(words, 8, 12),
        "dx": packed_rgba(words, 10, 14),
        "de": packed_rgba(words, 16, 20),
        "dy": packed_rgba(words, 18, 22),
    }


def rgb_key(values: tuple[float, ...]) -> tuple[int, int, int]:
    return tuple(max(0, min(255, int(round(value)))) for value in values[:3])


def summarize(samples: list[tuple[float, ...]]) -> dict[str, object]:
    if not samples:
        return {
            "samples": 0,
            "mean_rgb": None,
            "mean_modulation_factor": None,
            "mean_alpha": None,
            "mean_alpha_factor": None,
            "minimum_rgb": None,
            "maximum_rgb": None,
            "top_clamped_rounded_rgb": [],
        }
    counts = Counter(rgb_key(sample) for sample in samples)
    mean = [
        sum(sample[channel] for sample in samples) / len(samples)
        for channel in range(3)
    ]
    mean_alpha = sum(sample[3] for sample in samples) / len(samples)
    return {
        "samples": len(samples),
        "mean_rgb": [round(value, 6) for value in mean],
        "mean_modulation_factor": [round(value / 255.0, 8) for value in mean],
        "mean_alpha": round(mean_alpha, 6),
        "mean_alpha_factor": round(mean_alpha / 255.0, 8),
        "minimum_rgb": [round(min(sample[c] for sample in samples), 6) for c in range(3)],
        "maximum_rgb": [round(max(sample[c] for sample in samples), 6) for c in range(3)],
        "top_clamped_rounded_rgb": [
            {"rgb": list(rgb), "triangles": count}
            for rgb, count in counts.most_common(32)
        ],
    }


def render_mode(low: int) -> dict[str, object]:
    z_mode_names = ("opaque", "interpenetrating", "translucent", "decal")
    return {
        "low16": f"0x{low & 0xFFFF:04X}",
        "aa": bool(low & 0x0008),
        "z_compare": bool(low & 0x0010),
        "z_update": bool(low & 0x0020),
        "image_read": bool(low & 0x0040),
        "z_mode": z_mode_names[(low >> 10) & 3],
        "coverage_times_alpha": bool(low & 0x1000),
        "alpha_coverage_select": bool(low & 0x2000),
        "force_blend": bool(low & 0x4000),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    trace_path = args.capture / "rdp_command_trace.jsonl"
    records = [json.loads(line) for line in trace_path.read_text(encoding="utf-8").splitlines() if line]
    records.sort(key=lambda item: (int(item["frame"]), int(item["dlist_sequence"])))

    opcodes: Counter[int] = Counter()
    warnings: list[str] = []
    all_shades: list[tuple[float, ...]] = []
    by_combiner: dict[tuple[int, int] | None, list[tuple[float, ...]]] = defaultdict(list)
    triangle_by_combiner: Counter[tuple[int, int] | None] = Counter()
    triangle_states: Counter[
        tuple[tuple[int, int] | None, tuple[int, int] | None, int]
    ] = Counter()
    gradients = {name: [] for name in ("dx", "de", "dy")}
    combiner: tuple[int, int] | None = None
    other_mode: tuple[int, int] | None = None
    total_commands = 0
    total_triangles = 0

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
            total_commands += 1
            opcodes[opcode] += 1
            if opcode == 0x3C:
                combiner = (words[0], words[1])
            elif opcode == 0x2F:
                other_mode = (words[0] & 0x00FFFFFF, words[1])
            if opcode in TRIANGLES:
                total_triangles += 1
                triangle_by_combiner[combiner] += 1
                triangle_states[(combiner, other_mode, opcode)] += 1
            if opcode in SHADED_TRIANGLES:
                block = shade_block(words)
                all_shades.append(block["start"])
                by_combiner[combiner].append(block["start"])
                for name in gradients:
                    gradients[name].append(block[name])
            cursor += length

    combiner_records = []
    for state, triangles in triangle_by_combiner.most_common():
        shades = by_combiner.get(state, [])
        combiner_records.append({
            "word0": None if state is None else f"0x{state[0]:08X}",
            "word1": None if state is None else f"0x{state[1]:08X}",
            "triangles": triangles,
            "shaded_triangles": len(shades),
            "shade_start": summarize(shades),
        })

    report = {
        "schema": "v8.n64-runtime-rdp-shade.v1",
        "capture": str(args.capture.resolve()),
        "trace": str(trace_path.resolve()),
        "spans_declared": len(records),
        "commands": total_commands,
        "opcode_counts": {f"0x{opcode:02X}": count for opcode, count in sorted(opcodes.items())},
        "triangles": total_triangles,
        "shaded_triangles": len(all_shades),
        "shade_start": summarize(all_shades),
        "shade_gradients": {name: summarize(values) for name, values in gradients.items()},
        "combiners": combiner_records,
        "triangle_states": [
            {
                "triangles": count,
                "triangle_opcode": f"0x{opcode:02X}",
                "has_shade": opcode in SHADED_TRIANGLES,
                "has_texture": opcode in {0x0A, 0x0B, 0x0E, 0x0F},
                "has_z": opcode in {0x09, 0x0B, 0x0D, 0x0F},
                "combiner_word0": (
                    None if combine_state is None else f"0x{combine_state[0]:08X}"
                ),
                "combiner_word1": (
                    None if combine_state is None else f"0x{combine_state[1]:08X}"
                ),
                "other_mode_high": (
                    None if mode_state is None else f"0x{mode_state[0]:06X}"
                ),
                "other_mode_low": (
                    None if mode_state is None else f"0x{mode_state[1]:08X}"
                ),
                "render_mode": (
                    None if mode_state is None else render_mode(mode_state[1])
                ),
            }
            for (combine_state, mode_state, opcode), count
            in triangle_states.most_common()
        ],
        "warnings": warnings,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({
        "spans": len(records),
        "commands": total_commands,
        "triangles": total_triangles,
        "shaded_triangles": len(all_shades),
        "shade_start": report["shade_start"],
        "combiners": [
            {
                "word0": item["word0"],
                "word1": item["word1"],
                "triangles": item["triangles"],
                "shaded_triangles": item["shaded_triangles"],
                "mean_rgb": item["shade_start"]["mean_rgb"],
            }
            for item in combiner_records
        ],
        "warnings": len(warnings),
    }, indent=2))
    return 0 if not warnings else 1


if __name__ == "__main__":
    raise SystemExit(main())
