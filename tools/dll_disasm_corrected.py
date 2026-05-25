#!/usr/bin/env python3
"""Disassemble a relocated V8 overlay with the repository relocation logic.

Ghidra exports made before the tag-3 relocation overflow fix can contain bad
opcodes for overlays imported at synthetic bases such as 0x80100000.  This
tool applies tools/dll_preproc.py first, then emits per-function MIPS listings
from the corrected image.
"""
from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

from capstone import Cs, CS_ARCH_MIPS, CS_MODE_LITTLE_ENDIAN, CS_MODE_MIPS32

import dll_preproc


def parse_int(text: str) -> int:
    return int(text, 0)


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("dll", help="overlay DLL bytes, e.g. input/SHELL/LOAD.DLL")
    ap.add_argument("functions", help="functions.json with address/size fields")
    ap.add_argument("out_dir", help="directory for corrected .s listings")
    ap.add_argument("--base", type=parse_int, default=0x80100000)
    ns = ap.parse_args(argv)

    raw = bytearray(Path(ns.dll).read_bytes())
    if len(raw) < 4:
        raise SystemExit("overlay is too small")
    image_end = struct.unpack_from("<I", raw, 0)[0]
    if image_end <= 0 or image_end > len(raw):
        raise SystemExit(f"bad image size 0x{image_end:x}")

    dll_preproc.reloc(raw, ns.base)
    image = bytes(raw[:image_end])
    funcs = json.loads(Path(ns.functions).read_text(encoding="utf-8"))

    out_dir = Path(ns.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS32 | CS_MODE_LITTLE_ENDIAN)
    for func in funcs:
        addr = parse_int(func["address"])
        size = int(func["size"])
        name = func.get("name", f"sub_{addr:08x}")
        off = addr - ns.base
        if off < 0 or off + size > len(image):
            continue
        lines = [f"# addr: 0x{addr:08x}  name: {name}  size: 0x{size:x}"]
        for insn in md.disasm(image[off:off + size], addr):
            lines.append(f"{insn.address:08x}:  {insn.mnemonic:<8} {insn.op_str}".rstrip())
        (out_dir / f"{addr:08x}.s").write_text("\n".join(lines) + "\n",
                                               encoding="ascii")

    print(f"wrote {len(list(out_dir.glob('*.s')))} corrected listings to {out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
