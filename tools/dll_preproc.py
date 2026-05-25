#!/usr/bin/env python3
"""dll_preproc.py -- pre-relocate a Vigilante 8 .DLL overlay for Ghidra import.

The on-disc image is position-independent: image[0] holds the offset of
the relocation table, followed by reloc entries (low-2-bit tag selects
fixup kind), then a 0xffffffff sentinel.

This tool applies the relocations at a synthetic base VA and writes the
text+data portion (sans reloc table) to <out>.bin. Ghidra can then
import that as a raw PSX MIPS binary at the chosen base.

Usage:
    dll_preproc.py <in.dll> <out.bin> --base 0x80100000
"""
import argparse, struct, sys
from pathlib import Path

RELOC_END = 0xffffffff

def reloc(image: bytearray, base: int) -> None:
    """Apply the in-image relocation table.  Matches Overlay_LoadAndRelocate
    (src/assets/overlay_loader.c) exactly."""
    image_end = struct.unpack_from('<I', image, 0)[0]
    pos = image_end
    while True:
        if pos + 4 > len(image):
            raise RuntimeError(f"reloc walk overran at 0x{pos:x}")
        entry = struct.unpack_from('<I', image, pos)[0]
        if entry == RELOC_END:
            break
        pos += 4
        tag    = entry & 3
        target = entry & 0xfffffffc
        if tag == 0:    # u32 absolute add
            v = (struct.unpack_from('<I', image, target)[0] + base) & 0xffffffff
            struct.pack_into('<I', image, target, v)
        elif tag == 1:  # 16-bit HI16 followed by separate u32 sym offset
            sym = struct.unpack_from('<I', image, pos)[0]
            pos += 4
            v = ((base + sym + 0x8000) >> 16) & 0xffff
            struct.pack_into('<H', image, target, v)
        elif tag == 2:  # u16 absolute add (low half)
            v = (struct.unpack_from('<H', image, target)[0] + (base & 0xffff)) & 0xffff
            struct.pack_into('<H', image, target, v)
        elif tag == 3:  # 26-bit J/JAL fixup
            # The PS1 code performs this with 32-bit unsigned overflow:
            #     word += ((uint32_t)(base << 4)) >> 6
            # Mask before the right shift. Without the overflow, synthetic
            # bases such as 0x80100000 turn JAL/J opcodes into SLTIU/SLTI.
            v = (struct.unpack_from('<I', image, target)[0] +
                 (((base << 4) & 0xffffffff) >> 6)) & 0xffffffff
            struct.pack_into('<I', image, target, v)
        else:
            raise RuntimeError(f"impossible tag {tag} at 0x{pos-4:x}")

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("infile")
    ap.add_argument("outfile")
    ap.add_argument("--base", type=lambda v: int(v, 0), required=True,
                    help="synthetic base VA, e.g. 0x80100000")
    args = ap.parse_args()

    raw = bytearray(Path(args.infile).read_bytes())
    image_end = struct.unpack_from('<I', raw, 0)[0]
    reloc(raw, args.base)
    Path(args.outfile).write_bytes(raw[:image_end])
    print(f"{args.infile}: image_size=0x{image_end:x} base=0x{args.base:x} "
          f"out={args.outfile}")

if __name__ == "__main__":
    main()
