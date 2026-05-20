#!/usr/bin/env python3
"""Scan SLUS_005.10 for `sw rT, OFFSET(rB)` instructions to a given byte
offset.  Used to find writers to Vehicle struct fields whose semantics
we want to nail down."""
import struct, json, sys, pathlib

ROOT = pathlib.Path(__file__).resolve().parent.parent
target_off = int(sys.argv[1], 0) if len(sys.argv) > 1 else 0xd8

with open(ROOT / 'input/SLUS_005.10', 'rb') as f:
    data = f.read()
hdr = data[:0x800]
_, _, load_addr, text_sz = struct.unpack_from('<IIII', hdr, 0x10)
text = data[0x800:0x800 + text_sz]

with open(ROOT / 'analysis/SLUS_005.10/functions.json') as f:
    fns = json.load(f)
items = sorted(
    (int(x['address'], 16), x.get('size', 0), x.get('name', '?'))
    for x in fns
)


def containing(tgt):
    best = None
    for a, s, n in items:
        if a <= tgt < a + (s or 0):
            best = (a, s, n)
        elif a > tgt:
            break
    return best


hits = []
for off in range(0, len(text) - 4, 4):
    w = struct.unpack('<I', text[off:off + 4])[0]
    op = (w >> 26) & 0x3f
    imm = w & 0xffff
    # opcode 0x2b = sw, 0x29 = sh, 0x28 = sb
    if op in (0x28, 0x29, 0x2b) and imm == target_off:
        base = (w >> 21) & 0x1f
        if base in (29, 30):
            continue
        op_name = {0x28: 'sb', 0x29: 'sh', 0x2b: 'sw'}[op]
        hits.append((load_addr + off, base, (w >> 16) & 0x1f, op_name))

print(f'sw rT, 0x{target_off:x}(rB) -- {len(hits)} non-stack sites')
fns_hit = {}
for a, base, rt, opn in hits:
    c = containing(a)
    key = c[2] if c else f'gap@0x{a:x}'
    fns_hit.setdefault(key, []).append((a, base, rt, opn))
for k in sorted(fns_hit, key=lambda k: -len(fns_hit[k])):
    sites = fns_hit[k]
    print(f'  {k:50s}  sites: {len(sites)}')
    for a, base, rt, opn in sites[:3]:
        print(f'    0x{a:x}  {opn} ${rt}, +0x{target_off:x}(${base})')
