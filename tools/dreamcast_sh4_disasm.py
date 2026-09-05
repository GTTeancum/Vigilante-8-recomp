#!/usr/bin/env python3
"""Linearly disassemble an address range from a raw Dreamcast SH-4 image."""

from __future__ import annotations

import argparse
from pathlib import Path

from capstone import (
    CS_ARCH_SH,
    CS_MODE_LITTLE_ENDIAN,
    CS_MODE_SH4,
    CS_MODE_SHFPU,
    Cs,
)


def integer(value: str) -> int:
    return int(value, 0)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path)
    parser.add_argument("address", type=integer)
    parser.add_argument("size", type=integer)
    parser.add_argument("--base", type=integer, default=0x8C010000)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    offset = args.address - args.base
    if offset < 0:
        parser.error("address precedes image base")
    data = args.image.read_bytes()[offset : offset + args.size]
    decoder = Cs(
        CS_ARCH_SH,
        CS_MODE_SH4 | CS_MODE_SHFPU | CS_MODE_LITTLE_ENDIAN,
    )
    decoder.skipdata = True
    lines: list[str] = []
    for instruction in decoder.disasm(data, args.address):
        encoded = instruction.bytes.hex()
        lines.append(
            f"{instruction.address:08x}: {encoded:<8} "
            f"{instruction.mnemonic:<10} {instruction.op_str}"
        )
    output = "\n".join(lines) + "\n"
    if args.output:
        args.output.write_text(output, encoding="utf-8")
    else:
        print(output, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
