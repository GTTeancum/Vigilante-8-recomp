#!/usr/bin/env python3
"""Infer V8:2 XTIN average-color sampling from retail V8 arena ports."""

from __future__ import annotations

import argparse
from pathlib import Path
import struct
import sys


REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "tools"))
sys.path.insert(0, str(REPO / "tools" / "blender_addons"))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from vigilante8_vehicle_tools import iff
import port_dreamland_assets as dreamland


DEFAULT_PAIRS = ("CASNOCTY.EXP", "OILFIELD.EXP", "SANDFACT.EXP", "VALLYFRM.EXP")


def _terrain(path: Path) -> iff.IffChunk:
    roots = list(iff.parse(path.read_bytes()).forms(b"TERR"))
    if len(roots) != 1:
        raise ValueError(f"{path} has {len(roots)} TERR forms")
    return roots[0]


def _payload(root: iff.IffChunk, tag: bytes) -> bytes:
    return next(
        node.payload
        for node in root.children
        if (node.form_type if node.is_form else node.tag) == tag
    )


def _integral(width: int, height: int, values: list[int]) -> list[int]:
    stride = width + 1
    result = [0] * (stride * (height + 1))
    for y in range(height):
        row_total = 0
        source_row = y * width
        target_row = (y + 1) * stride
        previous_row = y * stride
        for x in range(width):
            row_total += values[source_row + x]
            result[target_row + x + 1] = result[previous_row + x + 1] + row_total
    return result


def _rect_sum(
    integral: list[int], stride: int, x: int, y: int, width: int, height: int
) -> int:
    x1 = x + width
    y1 = y + height
    return (
        integral[y1 * stride + x1]
        - integral[y * stride + x1]
        - integral[y1 * stride + x]
        + integral[y * stride + x]
    )


def _pair_records(v8_path: Path, v82_path: Path, mapping):
    source = _terrain(v8_path)
    target = _terrain(v82_path)
    tinf = _payload(source, b"TINF")
    xtin = _payload(target, b"XTIN")
    width, height, palette, pixels = dreamland._decode_xbmp(
        _payload(source, b"XBMP")
    )
    channels = []
    for shift in (0, 5, 10):
        values = []
        for index in pixels:
            component = (palette[index] >> shift) & 0x1F
            values.append(mapping(component))
        channels.append(_integral(width, height, values))
    records = []
    for index in range(256):
        record = tinf[index * 40:(index + 1) * 40]
        x, y = struct.unpack_from(">HH", record, 2)
        records.append((x, y, xtin[index * 36 + 32:index * 36 + 35]))
    return width + 1, channels, records


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--v8-dir", type=Path, default=REPO / "PS1 game" / "TERRAIN")
    parser.add_argument("--v82-dir", type=Path, default=REPO / "V8_2_LOOSE" / "LEVELS" / "V8")
    parser.add_argument("--minimum-size", type=int, default=32)
    args = parser.parse_args()

    mappings = {
        "replicate": lambda value: (value << 3) | (value >> 2),
        "shift": lambda value: value << 3,
        "linear-floor": lambda value: value * 255 // 31,
        "linear-round": lambda value: (value * 255 + 15) // 31,
        "linear-ceil": lambda value: (value * 255 + 30) // 31,
    }
    mapping_results = []
    for mapping_name, mapping in mappings.items():
        mapping_pairs = [
            _pair_records(
                args.v8_dir / name,
                args.v82_dir / name,
                mapping,
            )
            for name in DEFAULT_PAIRS
        ]
        count = 48 * 48
        mismatches = {"floor": 0, "round": 0, "ceil": 0}
        for stride, channels, records in mapping_pairs:
            for origin_x, origin_y, target in records:
                for channel, integral in enumerate(channels):
                    total = _rect_sum(
                        integral, stride, origin_x, origin_y, 48, 48
                    )
                    values = {
                        "floor": total // count,
                        "round": (total + count // 2) // count,
                        "ceil": (total + count - 1) // count,
                    }
                    for mode, value in values.items():
                        mismatches[mode] += value != target[channel]
        for mode, mismatch_count in mismatches.items():
            mapping_results.append((mismatch_count, mapping_name, mode))

    print("full-cell mappings")
    for mismatch_count, mapping_name, mode in sorted(mapping_results):
        print(
            f"mismatches={mismatch_count:4d} mapping={mapping_name:12s} "
            f"mode={mode}"
        )

    pairs = [
        _pair_records(
            args.v8_dir / name,
            args.v82_dir / name,
            mappings["replicate"],
        )
        for name in DEFAULT_PAIRS
    ]
    results = []
    for size in range(args.minimum_size, 49):
        count = size * size
        for offset_y in range(49 - size):
            for offset_x in range(49 - size):
                mismatches = {"floor": 0, "round": 0, "ceil": 0}
                for stride, channels, records in pairs:
                    for origin_x, origin_y, target in records:
                        for channel, integral in enumerate(channels):
                            total = _rect_sum(
                                integral,
                                stride,
                                origin_x + offset_x,
                                origin_y + offset_y,
                                size,
                                size,
                            )
                            values = {
                                "floor": total // count,
                                "round": (total + count // 2) // count,
                                "ceil": (total + count - 1) // count,
                            }
                            for mode, value in values.items():
                                mismatches[mode] += value != target[channel]
                for mode, mismatch_count in mismatches.items():
                    results.append(
                        (mismatch_count, size, offset_x, offset_y, mode)
                    )

    print("crop search using replicate mapping")
    for mismatch_count, size, offset_x, offset_y, mode in sorted(results)[:30]:
        print(
            f"mismatches={mismatch_count:4d} size={size:2d} "
            f"offset={offset_x:2d},{offset_y:2d} mode={mode}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
