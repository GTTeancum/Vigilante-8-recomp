#!/usr/bin/env python3
"""Disassemble LAB_ functions from SLUS_005.10 PSX binary."""

import struct
from capstone import *
from capstone.mips import *

BINARY_PATH = "C:/Programming/GitHub/Vigilante 8 recomp/input/SLUS_005.10"
LOAD_ADDR = 0x80010000
HEADER_SIZE = 0x800
GP_REG = 0x80065304

def file_offset(psx_addr):
    return HEADER_SIZE + (psx_addr - LOAD_ADDR)

def read_bytes(f, psx_addr, count):
    f.seek(file_offset(psx_addr))
    return f.read(count)

def disasm(f, psx_addr, max_insns=100, stop_on_jr_ra=True):
    data = read_bytes(f, psx_addr, max_insns * 4 + 8)
    md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS32 | CS_MODE_LITTLE_ENDIAN)
    md.detail = True
    insns = []
    for insn in md.disasm(data, psx_addr):
        insns.append(insn)
        if len(insns) >= max_insns:
            break
        if stop_on_jr_ra and insn.id == MIPS_INS_JR:
            # include delay slot
            for extra in md.disasm(data[len(insns)*4:len(insns)*4+4], insn.address+4):
                insns.append(extra)
                break
            break
    return insns

def format_insns(insns, gp=GP_REG, known_labels=None):
    lines = []
    for insn in insns:
        addr = insn.address
        mnemonic = insn.mnemonic
        op_str = insn.op_str

        # Annotate GP-relative loads
        annotation = ""
        if known_labels and op_str:
            pass

        # Try to resolve GP-relative: addiu/lw/sw with $gp
        if '$gp' in op_str:
            # extract offset
            import re
            m = re.search(r'(-?\d+)\(\$gp\)', op_str)
            if m:
                offset = int(m.group(1))
                resolved = gp + offset
                annotation = f"  ; -> 0x{resolved:08x}"

        lines.append(f"  {addr:08x}:  {mnemonic:<8} {op_str}{annotation}")
    return "\n".join(lines)

def analyze_event_handler(insns):
    """Check if this looks like an event dispatch (compare $a1 to constants)."""
    comparisons = []
    calls = []
    jumps = []

    for insn in insns:
        op = insn.op_str
        mnem = insn.mnemonic

        # Look for: beq/bne $a1, $vN, target  or  slti/sltiu $reg, $a1, const
        if mnem in ('beq', 'bne', 'beql', 'bnel') and '$a1' in op:
            comparisons.append(f"{insn.address:08x}: {mnem} {op}")
        if mnem in ('addiu', 'li') and '$a1' in op:
            comparisons.append(f"{insn.address:08x}: {mnem} {op}  [load a1?]")
        # Look for slti/sltu comparing a1
        if mnem in ('slti', 'sltiu', 'sltu', 'slt') and '$a1' in op:
            comparisons.append(f"{insn.address:08x}: {mnem} {op}")
        # Calls (jal, jalr)
        if mnem in ('jal', 'jalr'):
            calls.append(f"{insn.address:08x}: {mnem} {op}")
        # Jumps (j, jr not ra)
        if mnem == 'j':
            jumps.append(f"{insn.address:08x}: {mnem} {op}")

    return comparisons, calls, jumps

TARGETS = [
    (0x80031634, "LAB_80031634"),
    (0x80031afc, "after FUN_80031864 (check LAB_80031bbc)"),
    (0x80031bbc, "LAB_80031bbc"),
    (0x80032aa4, "LAB_80032aa4"),
    (0x80033290, "LAB_80033290"),
    (0x8003403c, "LAB_8003403c"),
    (0x80033c74, "LAB_80033c74"),
]

with open(BINARY_PATH, 'rb') as f:
    for psx_addr, label in TARGETS:
        print(f"\n{'='*70}")
        print(f"  {label}  @ 0x{psx_addr:08x}  (file offset 0x{file_offset(psx_addr):x})")
        print(f"{'='*70}")

        insns = disasm(f, psx_addr, max_insns=100)
        print(f"  [{len(insns)} instructions, last @ 0x{insns[-1].address:08x}]")
        print()
        print(format_insns(insns))

        comps, calls, jumps = analyze_event_handler(insns)
        if comps:
            print(f"\n  -- $a1 comparisons (event dispatch?) --")
            for c in comps:
                print(f"    {c}")
        if calls:
            print(f"\n  -- calls --")
            for c in calls:
                print(f"    {c}")
        if jumps:
            print(f"\n  -- jumps --")
            for j in jumps:
                print(f"    {j}")

print("\nDone.")
