#!/usr/bin/env python3
"""Match an N64 V8 main-code function to PS1 Ghidra function assembly."""

from __future__ import annotations

import argparse
from difflib import SequenceMatcher
from pathlib import Path
import re

import rabbitizer

from v8_n64_level import V8N64Rom, be32


ASM_LINE = re.compile(
    r"^\s*[0-9a-fA-F]+:\s+_?([a-zA-Z0-9_.]+)"
)


def n64_mnemonics(rom: bytes, address: int, limit: int = 0x1000) -> list[str]:
    # The cart header records the RAM entry/load base for the uncompressed
    # main image.  V8 US starts at 0x80125800; assuming the SDK's common
    # 0x80000400 base silently disassembles archive data for this ROM.
    load_base = be32(rom, 8)
    offset = address - load_base + 0x1000
    if not (0 <= offset + 8 <= len(rom)):
        raise ValueError(f"N64 address 0x{address:08X} is outside main code")
    result: list[str] = []
    for cursor in range(offset, min(len(rom), offset + limit), 4):
        word = be32(rom, cursor)
        result.append(rabbitizer.Instruction(word).getOpcodeName())
        if word == 0x03E00008:
            if cursor + 8 <= len(rom):
                result.append(
                    rabbitizer.Instruction(be32(rom, cursor + 4)).getOpcodeName()
                )
            break
    return result


def ps1_mnemonics(path: Path) -> list[str]:
    result: list[str] = []
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        match = ASM_LINE.match(line)
        if match:
            result.append(match.group(1))
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("ps1_mips", type=Path)
    parser.add_argument("address", type=lambda value: int(value, 0))
    parser.add_argument("--count", type=int, default=10)
    args = parser.parse_args()

    source = n64_mnemonics(V8N64Rom(args.rom).data, args.address)
    matches: list[tuple[float, Path, int]] = []
    for path in args.ps1_mips.glob("*.s"):
        target = ps1_mnemonics(path)
        if not target:
            continue
        ratio = SequenceMatcher(None, source, target, autojunk=False).ratio()
        matches.append((ratio, path, len(target)))
    for ratio, path, length in sorted(matches, reverse=True)[:args.count]:
        print(
            f"{ratio:.6f} 0x{path.stem.upper()} "
            f"n64_ins={len(source)} ps1_ins={length}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
