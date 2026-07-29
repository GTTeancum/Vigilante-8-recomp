#!/usr/bin/env python3
"""Decode selected big-endian N64 XOBF animation channels."""

from __future__ import annotations

import argparse
from pathlib import Path

import audit_v8_n64_model_conversion as audit
import v8_n64_level as n64


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("exp", type=Path)
    parser.add_argument("--bank", type=int, default=0)
    parser.add_argument("--slots", required=True)
    args = parser.parse_args()

    exp = args.exp.read_bytes()
    bins = audit.xobf_bins(exp)
    slot_count = n64.be32(bins[args.bank], 24)
    xobf_forms = [
        child
        for child in n64.root_children(exp)
        if child.is_form and child.form_type == b"XOBF"
    ]
    nested = n64.form_children(
        n64.iff_form(b"XOBF", [xobf_forms[args.bank].payload]), b"XOBF"
    )
    animation = next(item.payload for item in nested if item.tag == b"ANM ")
    offsets = [
        n64.be32(animation, 4 + index * 4) for index in range(slot_count)
    ]
    requested = [int(value) for value in args.slots.split(",")]
    for slot in requested:
        start = offsets[slot]
        end = min(
            (value for value in offsets if value > start),
            default=len(animation),
        )
        print(f"slot={slot} offset=0x{start:X} end=0x{end:X}")
        cursor = start
        while start and cursor + 4 <= end:
            frame_delta = n64.be16(animation, cursor, signed=True)
            flags = n64.be16(animation, cursor + 2, signed=True)
            cursor += 4
            fields = []
            if flags >= 0:
                if flags & 0x01:
                    values = tuple(
                        n64.be16(animation, cursor + item * 2, signed=True)
                        for item in range(4)
                    )
                    fields.append(("rotation", values))
                    cursor += 8
                if flags & 0x02:
                    values = tuple(
                        n64.be32(animation, cursor + item * 4, signed=True)
                        for item in range(3)
                    )
                    fields.append(("translation", values))
                    cursor += 12
                if flags & 0x08:
                    values = tuple(
                        n64.be16(animation, cursor + item * 2, signed=True)
                        for item in range(4)
                    )
                    fields.append(("scale_or_rotation_08", values))
                    cursor += 8
                if flags & 0x10:
                    values = []
                    while cursor + 4 <= end:
                        target = n64.be16(animation, cursor)
                        texture = n64.be16(animation, cursor + 2)
                        values.append((target, texture))
                        cursor += 4
                        if target & 0x8000:
                            break
                    fields.append(("texture", tuple(values)))
                if flags & 0x20:
                    values = tuple(
                        n64.be16(animation, cursor + item * 2, signed=True)
                        for item in range(4)
                    )
                    fields.append(("scale_or_rotation_20", values))
                    cursor += 8
                if flags & 0x40:
                    count = n64.be32(animation, cursor)
                    cursor += 4
                    values = tuple(
                        tuple(
                            n64.be16(
                                animation,
                                cursor + record * 8 + item * 2,
                                signed=True,
                            )
                            for item in range(4)
                        )
                        for record in range(count)
                    )
                    cursor += count * 8
                    fields.append(("table_40", values))
            print(
                f"  frame_delta={frame_delta} flags={flags} "
                + " ".join(f"{name}={value}" for name, value in fields)
            )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
