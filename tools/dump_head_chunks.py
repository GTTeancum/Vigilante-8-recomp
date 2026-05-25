"""dump_head_chunks.py -- Walk OILFIELD.EXP IFF and dump HEAD chunk raw bytes."""
import struct, sys

path = 'input/TERRAIN/OILFIELD.EXP'
with open(path, 'rb') as f:
    data = f.read()

def rd32be(b, o): return struct.unpack_from('>I', b, o)[0]
def rd32le(b, o): return struct.unpack_from('<I', b, o)[0]
def rds16le(b, o): return struct.unpack_from('<h', b, o)[0]
def rds32le(b, o): return struct.unpack_from('<i', b, o)[0]

heads = []
bins  = []

def walk(p, end, depth=0):
    while p + 8 <= end:
        tag  = data[p:p+4]
        csz  = rd32be(data, p+4)
        body = p + 8
        if body + csz > end:
            break
        indent = '  ' * depth
        if tag == b'FORM':
            ftype = data[body:body+4]
            print(f'{indent}FORM {ftype.decode("latin-1")} @0x{p:x} size={csz}')
            walk(body + 4, body + csz, depth + 1)
        else:
            tag_str = tag.decode('latin-1')
            print(f'{indent}{tag_str!r} @0x{p:x} size={csz}')
            if tag == b'HEAD':
                heads.append((p, csz, data[body:body+csz]))
            elif tag == b'BIN ':
                bins.append((p, csz))
        p = body + csz + (csz & 1)

walk(0, len(data))

print()
print(f'Total HEAD chunks: {len(heads)}, BIN chunks: {len(bins)}')
sizes = {}
for _, sz, _ in heads:
    sizes[sz] = sizes.get(sz, 0) + 1
print(f'HEAD size distribution: {dict(sorted(sizes.items()))}')

print()
print('=== First 16 HEAD chunks (raw hex) ===')
for i, (off, sz, body) in enumerate(heads[:16]):
    print(f'HEAD[{i:3d}] @0x{off:x} sz={sz:3d}: {body[:sz].hex()}')

# Try to decode HEAD chunks as: 9×i16 rotation matrix + 3×i32 position
print()
print('=== First 16 HEAD chunks decoded (rot9×i16 + pos3×i32) ===')
for i, (off, sz, body) in enumerate(heads[:16]):
    if sz >= 30:
        rot = [rds16le(body, k*2) for k in range(9)]
        pos = [rds32le(body, 18 + k*4) for k in range(3)]
        print(f'HEAD[{i:3d}]: rot={rot}  pos={pos}')
    else:
        print(f'HEAD[{i:3d}]: too small ({sz} bytes)')

# Try alternate: 3×i32 position first, then 9×i16 rotation
print()
print('=== First 16 HEAD chunks decoded (pos3×i32 + rot9×i16) ===')
for i, (off, sz, body) in enumerate(heads[:16]):
    if sz >= 30:
        pos = [rds32le(body, k*4) for k in range(3)]
        rot = [rds16le(body, 12 + k*2) for k in range(9)]
        print(f'HEAD[{i:3d}]: pos={pos}  rot={rot}')
    else:
        print(f'HEAD[{i:3d}]: too small ({sz} bytes)')
