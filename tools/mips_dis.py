#!/usr/bin/env python3
"""Quick MIPS R3000 disassembler -- enough to read raw-hex dumps.
Covers the instruction subset PSX MIPS uses; renders unknown ops as
"??? 0xWORD".
"""
import sys, re, pathlib

REGS = ['zero','at','v0','v1','a0','a1','a2','a3',
        't0','t1','t2','t3','t4','t5','t6','t7',
        's0','s1','s2','s3','s4','s5','s6','s7',
        't8','t9','k0','k1','gp','sp','fp','ra']

def s16(x): return x-0x10000 if x & 0x8000 else x

def dis(addr, w):
    op  = (w >> 26) & 0x3f
    rs  = (w >> 21) & 0x1f
    rt  = (w >> 16) & 0x1f
    rd  = (w >> 11) & 0x1f
    sh  = (w >> 6)  & 0x1f
    fn  = w & 0x3f
    imm = w & 0xffff
    simm = s16(imm)
    tgt = (w & 0x03ffffff) << 2 | (addr+4) & 0xf0000000

    # special / R-type
    if op == 0:
        if   fn == 0x00: return 'nop' if w==0 else f'sll {REGS[rd]},{REGS[rt]},{sh}'
        elif fn == 0x02: return f'srl {REGS[rd]},{REGS[rt]},{sh}'
        elif fn == 0x03: return f'sra {REGS[rd]},{REGS[rt]},{sh}'
        elif fn == 0x04: return f'sllv {REGS[rd]},{REGS[rt]},{REGS[rs]}'
        elif fn == 0x06: return f'srlv {REGS[rd]},{REGS[rt]},{REGS[rs]}'
        elif fn == 0x07: return f'srav {REGS[rd]},{REGS[rt]},{REGS[rs]}'
        elif fn == 0x08: return f'jr {REGS[rs]}'
        elif fn == 0x09: return f'jalr {REGS[rd]},{REGS[rs]}' if rd!=31 else f'jalr {REGS[rs]}'
        elif fn == 0x10: return f'mfhi {REGS[rd]}'
        elif fn == 0x12: return f'mflo {REGS[rd]}'
        elif fn == 0x18: return f'mult {REGS[rs]},{REGS[rt]}'
        elif fn == 0x19: return f'multu {REGS[rs]},{REGS[rt]}'
        elif fn == 0x1a: return f'div {REGS[rs]},{REGS[rt]}'
        elif fn == 0x1b: return f'divu {REGS[rs]},{REGS[rt]}'
        elif fn == 0x20: return f'add {REGS[rd]},{REGS[rs]},{REGS[rt]}'
        elif fn == 0x21: return f'addu {REGS[rd]},{REGS[rs]},{REGS[rt]}'
        elif fn == 0x22: return f'sub {REGS[rd]},{REGS[rs]},{REGS[rt]}'
        elif fn == 0x23: return f'subu {REGS[rd]},{REGS[rs]},{REGS[rt]}'
        elif fn == 0x24: return f'and {REGS[rd]},{REGS[rs]},{REGS[rt]}'
        elif fn == 0x25: return f'or {REGS[rd]},{REGS[rs]},{REGS[rt]}'
        elif fn == 0x26: return f'xor {REGS[rd]},{REGS[rs]},{REGS[rt]}'
        elif fn == 0x27: return f'nor {REGS[rd]},{REGS[rs]},{REGS[rt]}'
        elif fn == 0x2a: return f'slt {REGS[rd]},{REGS[rs]},{REGS[rt]}'
        elif fn == 0x2b: return f'sltu {REGS[rd]},{REGS[rs]},{REGS[rt]}'
    elif op == 0x01:
        # REGIMM: rt selects
        if   rt == 0x00: return f'bltz {REGS[rs]},0x{addr+4+(simm<<2):x}'
        elif rt == 0x01: return f'bgez {REGS[rs]},0x{addr+4+(simm<<2):x}'
        elif rt == 0x10: return f'bltzal {REGS[rs]},0x{addr+4+(simm<<2):x}'
        elif rt == 0x11: return f'bgezal {REGS[rs]},0x{addr+4+(simm<<2):x}'
    elif op == 0x02: return f'j 0x{tgt:x}'
    elif op == 0x03: return f'jal 0x{tgt:x}'
    elif op == 0x04: return f'beq {REGS[rs]},{REGS[rt]},0x{addr+4+(simm<<2):x}'
    elif op == 0x05: return f'bne {REGS[rs]},{REGS[rt]},0x{addr+4+(simm<<2):x}'
    elif op == 0x06: return f'blez {REGS[rs]},0x{addr+4+(simm<<2):x}'
    elif op == 0x07: return f'bgtz {REGS[rs]},0x{addr+4+(simm<<2):x}'
    elif op == 0x08: return f'addi {REGS[rt]},{REGS[rs]},{simm}'
    elif op == 0x09: return f'addiu {REGS[rt]},{REGS[rs]},{simm}'
    elif op == 0x0a: return f'slti {REGS[rt]},{REGS[rs]},{simm}'
    elif op == 0x0b: return f'sltiu {REGS[rt]},{REGS[rs]},{simm}'
    elif op == 0x0c: return f'andi {REGS[rt]},{REGS[rs]},0x{imm:x}'
    elif op == 0x0d: return f'ori {REGS[rt]},{REGS[rs]},0x{imm:x}'
    elif op == 0x0e: return f'xori {REGS[rt]},{REGS[rs]},0x{imm:x}'
    elif op == 0x0f: return f'lui {REGS[rt]},0x{imm:x}'
    elif op == 0x12:
        # COP2 (GTE)
        return f'cop2 0x{w & 0x01ffffff:07x}'
    elif op == 0x20: return f'lb {REGS[rt]},{simm}({REGS[rs]})'
    elif op == 0x21: return f'lh {REGS[rt]},{simm}({REGS[rs]})'
    elif op == 0x23: return f'lw {REGS[rt]},{simm}({REGS[rs]})'
    elif op == 0x24: return f'lbu {REGS[rt]},{simm}({REGS[rs]})'
    elif op == 0x25: return f'lhu {REGS[rt]},{simm}({REGS[rs]})'
    elif op == 0x28: return f'sb {REGS[rt]},{simm}({REGS[rs]})'
    elif op == 0x29: return f'sh {REGS[rt]},{simm}({REGS[rs]})'
    elif op == 0x2b: return f'sw {REGS[rt]},{simm}({REGS[rs]})'
    return f'??? 0x{w:08x}'


if __name__ == '__main__':
    path = pathlib.Path(sys.argv[1])
    text = path.read_text()
    line_re = re.compile(r'^0x([0-9a-fA-F]+):\s*0x([0-9a-fA-F]+)\s*$')
    for line in text.splitlines():
        m = line_re.match(line.strip())
        if not m:
            print(line)
            continue
        addr = int(m.group(1), 16)
        word = int(m.group(2), 16)
        print(f'0x{addr:08x}: {dis(addr, word)}')
