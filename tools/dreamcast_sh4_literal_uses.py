#!/usr/bin/env python3
"""Find SH-4 PC-relative mov.l instructions that load selected literal slots."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


def integer(value: str) -> int:
    return int(value, 0)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path)
    parser.add_argument("literal", nargs="+", type=integer)
    parser.add_argument("--base", type=integer, default=0x8C010000)
    args = parser.parse_args()

    data = args.image.read_bytes()
    wanted = set(args.literal)
    for offset in range(0, len(data) - 1, 2):
        opcode = struct.unpack_from("<H", data, offset)[0]
        if opcode & 0xF000 != 0xD000:
            continue
        address = args.base + offset
        literal = ((address + 4) & ~3) + ((opcode & 0xFF) * 4)
        if literal not in wanted:
            continue
        register = (opcode >> 8) & 0xF
        print(f"{literal:08x} <- {address:08x}: mov.l @{literal:08x},r{register}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
