#!/usr/bin/env python3
"""Find little-endian 32-bit pointer values in a raw Dreamcast image."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


def integer(value: str) -> int:
    return int(value, 0)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path)
    parser.add_argument("values", nargs="+", type=integer)
    parser.add_argument("--base", type=integer, default=0x8C010000)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    data = args.image.read_bytes()
    lines: list[str] = []
    for value in args.values:
        encoded = struct.pack("<I", value & 0xFFFFFFFF)
        hits: list[str] = []
        offset = 0
        while True:
            offset = data.find(encoded, offset)
            if offset < 0:
                break
            hits.append(f"0x{args.base + offset:08x}")
            offset += 1
        lines.append(f"0x{value & 0xFFFFFFFF:08x}: {', '.join(hits) if hits else '-'}")

    output = "\n".join(lines) + "\n"
    if args.output:
        args.output.write_text(output, encoding="utf-8")
    else:
        print(output, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
