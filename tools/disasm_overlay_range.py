#!/usr/bin/env python3
"""Relocate and disassemble one address range from a PS1 overlay."""

from __future__ import annotations

import argparse
from pathlib import Path
import struct

import dll_preproc
import mips_dis


def value(text: str) -> int:
    return int(text, 0)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("overlay", type=Path)
    parser.add_argument("--base", type=value, default=0x80100000)
    parser.add_argument("--start", type=value, required=True)
    parser.add_argument("--end", type=value, required=True)
    args = parser.parse_args()

    image = bytearray(args.overlay.read_bytes())
    dll_preproc.reloc(image, args.base)
    start = args.start - args.base
    end = args.end - args.base
    if start < 0 or end > len(image) or start >= end:
        raise ValueError("requested range falls outside the overlay")
    for offset in range(start, end, 4):
        address = args.base + offset
        word = struct.unpack_from("<I", image, offset)[0]
        print(f"{address:08x}: {word:08x}  {mips_dis.dis(address, word)}")


if __name__ == "__main__":
    main()
