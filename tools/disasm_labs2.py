#!/usr/bin/env python3
"""Extended disassembly to see full function structure of key LAB_ targets."""

import struct
from capstone import *
from capstone.mips import *

BINARY_PATH = "C:/Programming/GitHub/Vigilante 8 recomp/input/SLUS_005.10"
LOAD_ADDR = 0x80010000
HEADER_SIZE = 0x800
GP_REG = 0x80065304

def file_offset(psx_addr):
    return HEADER_SIZE + (psx_addr - LOAD_ADDR)

def disasm_range(f, start_addr, end_addr):
    size = end_addr - start_addr
    f.seek(file_offset(start_addr))
    data = f.read(size)
    md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS32 | CS_MODE_LITTLE_ENDIAN)
    md.detail = True
    insns = list(md.disasm(data, start_addr))
    return insns

def fmt(insns, gp=GP_REG):
    import re
    lines = []
    for insn in insns:
        op_str = insn.op_str
        ann = ""
        if '$gp' in op_str:
            m = re.search(r'(-?\d+)\(\$gp\)', op_str)
            if m:
                off = int(m.group(1))
                ann = f"  ; -> 0x{gp+off:08x}"
        # skip pure nop lines for brevity unless they're delay slots
        if insn.mnemonic == 'nop':
            lines.append(f"  {insn.address:08x}:  nop")
        else:
            lines.append(f"  {insn.address:08x}:  {insn.mnemonic:<8} {op_str}{ann}")
    return "\n".join(lines)

with open(BINARY_PATH, 'rb') as f:
    # LAB_80032aa4: ends before FUN_80032e48 at 0x80032e48
    print("="*70)
    print("LAB_80032aa4  full (0x80032aa4 .. 0x80032e48)")
    print("="*70)
    insns = disasm_range(f, 0x80032aa4, 0x80032e48)
    print(f"  [{len(insns)} instructions]")
    print(fmt(insns))

    # LAB_80033290 full: it's inside a larger region, let's check up to 0x80033510
    print()
    print("="*70)
    print("LAB_80033290  full (0x80033290 .. 0x80033510)")
    print("="*70)
    insns = disasm_range(f, 0x80033290, 0x80033510)
    print(f"  [{len(insns)} instructions]")
    print(fmt(insns))

    # LAB_8003403c: let's read more - check up to some end
    print()
    print("="*70)
    print("LAB_8003403c  extended (0x8003403c .. 0x800346c0)")
    print("="*70)
    insns = disasm_range(f, 0x8003403c, 0x800346c0)
    print(f"  [{len(insns)} instructions]")
    print(fmt(insns))

    # LAB_80031634 full - to 0x80031864
    print()
    print("="*70)
    print("LAB_80031634  full (0x80031634 .. 0x80031864)")
    print("="*70)
    insns = disasm_range(f, 0x80031634, 0x80031864)
    print(f"  [{len(insns)} instructions]")
    print(fmt(insns))

print("Done.")
