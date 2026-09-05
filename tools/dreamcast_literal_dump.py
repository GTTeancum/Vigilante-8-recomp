#!/usr/bin/env python3
"""Dump typed values from raw Dreamcast executable addresses."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


def integer(value: str) -> int:
    return int(value, 0)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path)
    parser.add_argument("addresses", nargs="+", type=integer)
    parser.add_argument("--base", type=integer, default=0x8C010000)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    data = args.image.read_bytes()
    lines: list[str] = []
    for address in args.addresses:
        offset = address - args.base
        if offset < 0 or offset + 4 > len(data):
            lines.append(f"{address:08x}: out of range")
            continue
        raw = data[offset : offset + 4]
        u16 = struct.unpack_from("<H", raw)[0]
        s16 = struct.unpack_from("<h", raw)[0]
        u32 = struct.unpack_from("<I", raw)[0]
        f32 = struct.unpack_from("<f", raw)[0]
        lines.append(
            f"{address:08x}: {raw.hex()} u16={u16} s16={s16} "
            f"u32=0x{u32:08x} f32={f32:.9g}"
        )

    output = "\n".join(lines) + "\n"
    if args.output:
        args.output.write_text(output, encoding="utf-8")
    else:
        print(output, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
