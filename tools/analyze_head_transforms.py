"""analyze_head_transforms.py -- decode HEAD object instance transforms from OILFIELD.EXP.

Hypothesis (BE interpretation):
  bytes  0- 3: flags/LOD  (u32 BE)
  bytes  4- 7: type flags  (u32 BE)
  bytes  8-11: Z world pos (i32 BE, physics units: divide by 131072 -> metres)
  bytes 12-15: ??? (consistently ~31m equivalent - suspected bounding/LOD)
  bytes 16-19: X world pos (i32 BE, physics units)
  bytes 20-23: Y world pos (i32 BE, OpenGL Y-up, physics units)
  bytes 24-25: rot_x (i16 BE, PSX angle units: 4096 = 360 degrees)
  bytes 26-27: rot_y (i16 BE)
  bytes 28-29: rot_z (i16 BE)
  bytes 30-32: unknown (3 bytes)
  bytes 33+  : object name string (to end of chunk, no null terminator)

Also analyses the small BIN (second XOBF BIN in EXP) for vertex extents.
"""
import struct, sys, math

EXP_PATH = 'input/TERRAIN/OILFIELD.EXP'
PHYS_SCALE = 1.0 / 131072.0   # 1 physics unit -> metres (OpenGL)

with open(EXP_PATH, 'rb') as f:
    raw = f.read()

def rd32be(b, o): return struct.unpack_from('>I', b, o)[0]
def rds32be(b, o): return struct.unpack_from('>i', b, o)[0]
def rds16be(b, o): return struct.unpack_from('>h', b, o)[0]
def rd32le(b, o): return struct.unpack_from('<I', b, o)[0]
def rd16le(b, o): return struct.unpack_from('<H', b, o)[0]
def rds16le(b, o): return struct.unpack_from('<h', b, o)[0]
def rds32le(b, o): return struct.unpack_from('<i', b, o)[0]

# ------------------------------------------------------------------ IFF walker
heads = []   # (file_offset, chunk_size, body_bytes, form_type)
bins  = []   # (file_offset, chunk_size)

def walk(p, end, depth=0, parent_form=b'    '):
    while p + 8 <= end:
        tag  = raw[p:p+4]
        csz  = rd32be(raw, p+4)
        body = p + 8
        if body + csz > end:
            break
        if tag == b'FORM':
            ftype = raw[body:body+4]
            walk(body+4, body+csz, depth+1, ftype)
        else:
            if tag == b'HEAD':
                heads.append((p, csz, raw[body:body+csz], parent_form))
            elif tag == b'BIN ':
                bins.append((p, csz, body))
        p = body + csz + (csz & 1)

walk(0, len(raw))

print(f'Total HEAD chunks: {len(heads)}, BIN chunks: {len(bins)}')

# ------------------------------------------------------------------ Decode HEADs

PSX_ANG_SCALE = 360.0 / 4096.0   # PSX angle unit -> degrees

def decode_head(body):
    """Decode object instance HEAD chunk. Returns dict."""
    sz = len(body)
    if sz < 24:
        return None
    flags = rd32be(body, 0)
    type_flags = rd32be(body, 4)
    z_phys = rds32be(body, 8)   # Z world pos (physics units)
    unk12  = rds32be(body, 12)  # unknown field
    x_phys = rds32be(body, 16)  # X world pos
    y_phys = rds32be(body, 20)  # Y world pos (OpenGL Y-up)
    rot_x = rds16be(body, 24) if sz > 25 else 0
    rot_y = rds16be(body, 26) if sz > 27 else 0
    rot_z = rds16be(body, 28) if sz > 29 else 0
    # Name: bytes 33 onwards (byte 33 may be a type byte, name at 34?)
    # Try both: name at 33 and name at 34
    name = ''
    if sz > 33:
        raw_name = body[33:sz]
        # Try to find printable ASCII run
        for start in range(min(3, len(raw_name))):
            candidate = raw_name[start:]
            if all(0x20 <= b <= 0x7e for b in candidate):
                name = candidate.decode('ascii')
                break
    return {
        'flags': flags,
        'type_flags': type_flags,
        'z_m': z_phys * PHYS_SCALE,
        'x_m': x_phys * PHYS_SCALE,
        'y_m': y_phys * PHYS_SCALE,
        'unk12_m': unk12 * PHYS_SCALE,
        'rot_x_deg': rot_x * PSX_ANG_SCALE,
        'rot_y_deg': rot_y * PSX_ANG_SCALE,
        'rot_z_deg': rot_z * PSX_ANG_SCALE,
        'x_phys': x_phys,
        'y_phys': y_phys,
        'z_phys': z_phys,
        'name': name,
    }

print()
print('=== Object instances (HEAD chunks in FORM OBJ) ===')
obj_heads = [(off, sz, body, pf) for off, sz, body, pf in heads if pf == b'OBJ ']
print(f'FORM OBJ instances: {len(obj_heads)}')
print()
print(f'{"#":>4}  {"Name":<25}  {"X":>8}  {"Y":>8}  {"Z":>8}  {"rotY_deg":>9}  {"unk12_m":>8}')
print('-'*90)

for i, (off, sz, body, pf) in enumerate(obj_heads[:145]):
    d = decode_head(body)
    if d:
        print(f'{i:4d}  {d["name"]:<25}  {d["x_m"]:8.1f}  {d["y_m"]:8.2f}  {d["z_m"]:8.1f}  {d["rot_y_deg"]:9.1f}  {d["unk12_m"]:8.1f}')

# Check extents
xs = [decode_head(body)['x_m'] for _, _, body, pf in obj_heads if decode_head(body)]
zs = [decode_head(body)['z_m'] for _, _, body, pf in obj_heads if decode_head(body)]
ys = [decode_head(body)['y_m'] for _, _, body, pf in obj_heads if decode_head(body)]
print()
print(f'X range: [{min(xs):.1f} .. {max(xs):.1f}]  (OilField X: ~523-693m)')
print(f'Z range: [{min(zs):.1f} .. {max(zs):.1f}]  (OilField Z: ~374-554m)')
print(f'Y range: [{min(ys):.2f} .. {max(ys):.2f}]  (terrain gl_y = -2.25m)')

# ------------------------------------------------------------------ Small BIN analysis
print()
print('=== Small BIN analysis ===')
if len(bins) >= 2:
    small_off, small_sz, small_body_off = bins[1]
    B = raw[small_body_off:small_body_off+small_sz]
    nb     = rd32le(B, 0)
    btbase = rd32le(B, 4)
    print(f'Small BIN @0x{small_body_off:x}, size 0x{small_sz:x}')
    print(f'Bones: {nb}, bone_table_base: 0x{btbase:x}')

    PKT_SIZE = [12,28,20,28, 12,20,12,20, 16,24,12,24, 20,20,0,20]
    IS_QUAD  = [0,0,0,0, 1,1,0,1, 0,0,0,0, 0,0,0,0]

    print(f'\n{"bone":>5}  {"vc":>5}  {"pc":>5}  {"vx_range":>20}  {"vy_range":>20}  {"vz_range":>20}')
    print('-'*90)
    all_vx, all_vy, all_vz = [], [], []
    for bi in range(nb):
        if btbase + bi*4 + 4 > small_sz: break
        rel = rd32le(B, btbase + bi*4)
        bd  = btbase + rel
        if bd + 0x18 > small_sz: break
        vc = rd32le(B, bd + 0x00)
        vr = rd32le(B, bd + 0x04)
        pc = rd16le(B, bd + 0x10)
        pr = rd32le(B, bd + 0x14)
        vo = bd + vr
        vxs, vys, vzs = [], [], []
        for vi in range(vc):
            oe = vo + vi*8
            if oe + 6 > small_sz: break
            vxs.append(rds16le(B, oe+0))
            vys.append(rds16le(B, oe+2))
            vzs.append(rds16le(B, oe+4))
        if vxs:
            all_vx += vxs; all_vy += vys; all_vz += vzs
            xr = f'[{min(vxs):6d}..{max(vxs):6d}]'
            yr = f'[{min(vys):6d}..{max(vys):6d}]'
            zr = f'[{min(vzs):6d}..{max(vzs):6d}]'
            print(f'{bi:5d}  {vc:5d}  {pc:5d}  {xr:>20}  {yr:>20}  {zr:>20}')

    if all_vx:
        print()
        print(f'Small BIN global vertex extents (raw units):')
        print(f'  X: [{min(all_vx)} .. {max(all_vx)}]')
        print(f'  Y: [{min(all_vy)} .. {max(all_vy)}]')
        print(f'  Z: [{min(all_vz)} .. {max(all_vz)}]')
        print()
        # Try scale 1/160 (vehicle scale) and 1/16 (terrain scale)
        for scale, label in [(1/160.0, '1/160 (vehicle)'), (1/16.0, '1/16 (terrain)')]:
            print(f'  At scale {label}:')
            print(f'    X: [{min(all_vx)*scale:.2f} .. {max(all_vx)*scale:.2f}]m')
            print(f'    Y: [{min(all_vy)*scale:.2f} .. {max(all_vy)*scale:.2f}]m')
            print(f'    Z: [{min(all_vz)*scale:.2f} .. {max(all_vz)*scale:.2f}]m')
else:
    print('Only one BIN found - no small BIN')
