"""analyze_bin_structure.py -- Deep analysis of XOBF BIN structure.

Determines whether BIN vertices are in global (level-relative) or
bone-local space, and looks for per-bone transform data in the
mystery block between the BIN header and the bone table.

Usage: python analyze_bin_structure.py
"""
import struct, sys, math

EXP_PATH = 'input/TERRAIN/OILFIELD.EXP'
TERR_SCALE = 1.0 / 16.0
WORLD_X = 608.0
WORLD_Z = 464.0

def rd32be(b, o): return struct.unpack_from('>I', b, o)[0]
def rd32le(b, o): return struct.unpack_from('<I', b, o)[0]
def rd16le(b, o): return struct.unpack_from('<H', b, o)[0]
def rds16le(b, o): return struct.unpack_from('<h', b, o)[0]
def rds32le(b, o): return struct.unpack_from('<i', b, o)[0]

with open(EXP_PATH, 'rb') as f:
    raw = f.read()

# --- Find first BIN inside FORM XOBF ---
def find_xobf_bin(data, off, end, depth=0):
    p = off
    while p + 8 <= end:
        tag = data[p:p+4]
        csz = rd32be(data, p+4)
        body = p + 8
        if body + csz > end: break
        if tag == b'FORM':
            ftype = data[body:body+4]
            if ftype == b'XOBF':
                # Walk children for BIN
                q = body + 4
                qend = body + csz
                while q + 8 <= qend:
                    btag = data[q:q+4]
                    bsz  = rd32be(data, q+4)
                    bbody = q + 8
                    if bbody + bsz > qend: break
                    if btag == b'BIN ':
                        return bbody, bsz
                    q = bbody + bsz + (bsz & 1)
            else:
                r = find_xobf_bin(data, body+4, body+csz, depth+1)
                if r: return r
        p = body + csz + (csz & 1)
    return None

result = find_xobf_bin(raw, 0, len(raw))
if not result:
    print("No BIN found!")
    sys.exit(1)

bin_off, bin_sz = result
B = raw[bin_off:bin_off+bin_sz]
print(f"BIN at 0x{bin_off:x}, size 0x{bin_sz:x} ({bin_sz} bytes)")

nb     = rd32le(B, 0)
btbase = rd32le(B, 4)
print(f"Bones: {nb}")
print(f"Bone table base: 0x{btbase:x} ({btbase} bytes from BIN start)")
print(f"Mystery block: bytes [8 .. 0x{btbase:x}] = {btbase-8} bytes")
if nb > 0:
    print(f"  Per-bone average: {(btbase-8)/nb:.1f} bytes")
print()

# --- Dump first 64 bytes of the mystery block in several stride interpretations ---
mystery_start = 8
mystery_end = btbase
mystery_len = mystery_end - mystery_start

print("=== Mystery block: first 128 bytes (hex) ===")
for i in range(0, min(128, mystery_len), 16):
    chunk = B[mystery_start+i : mystery_start+i+16]
    print(f"  +0x{i:04x}:  {chunk.hex(' ')}")
print()

# Try to interpret mystery block as per-bone transform structs
# PSX Object matrix at +0x10: 9 × i16 (18 bytes) + 3 × i32 position (12 bytes) = 30 bytes
# Or maybe: 3 × i32 position (12 bytes) + 9 × i16 rotation (18 bytes) = 30 bytes
# Or: 3 × i32 position only (12 bytes per bone)
# Or: 3 × i16 position (6 bytes) + pad

for stride in [12, 16, 20, 24, 28, 30, 32, 36, 40, 48, 56, 64, 80, 84, 88]:
    if mystery_len % stride == 0:
        n = mystery_len // stride
        print(f"  stride {stride}: mystery_len / stride = {n} (nb={nb})")

print()

# Try stride=12 (3×i32 position per bone)
if mystery_len >= nb * 12:
    print(f"=== Mystery block as 3×i32 position (stride 12) for first 10 bones ===")
    for bi in range(min(10, nb)):
        o = mystery_start + bi * 12
        px = rds32le(B, o+0)
        py = rds32le(B, o+4)
        pz = rds32le(B, o+8)
        # Scale as BIN units → metres
        wx = px * TERR_SCALE + WORLD_X
        wy = -py * TERR_SCALE
        wz = pz * TERR_SCALE + WORLD_Z
        print(f"  bone[{bi:3d}]: raw=({px:6d},{py:6d},{pz:6d})  world=({wx:.1f}, {wy:.2f}, {wz:.1f})")
    print()

# Try stride=16 (3×i32 position + 4 bytes pad)
if mystery_len >= nb * 16:
    print(f"=== Mystery block as 3×i32 + pad (stride 16) for first 10 bones ===")
    for bi in range(min(10, nb)):
        o = mystery_start + bi * 16
        px = rds32le(B, o+0)
        py = rds32le(B, o+4)
        pz = rds32le(B, o+8)
        xtra = rd32le(B, o+12)
        wx = px * TERR_SCALE + WORLD_X
        wy = -py * TERR_SCALE
        wz = pz * TERR_SCALE + WORLD_Z
        print(f"  bone[{bi:3d}]: raw=({px:6d},{py:6d},{pz:6d}) xtra=0x{xtra:08x}  world=({wx:.1f}, {wy:.2f}, {wz:.1f})")
    print()

# Try stride=24 (3×i16 + ... or 6×i32)
if mystery_len >= nb * 24:
    print(f"=== Mystery block as stride 24 for first 10 bones ===")
    for bi in range(min(10, nb)):
        o = mystery_start + bi * 24
        # Try as 2× 3×i32
        v1 = [rds32le(B, o+k*4) for k in range(3)]
        v2 = [rds32le(B, o+12+k*4) for k in range(3)]
        print(f"  bone[{bi:3d}]:  ({v1[0]:8d},{v1[1]:8d},{v1[2]:8d})  ({v2[0]:8d},{v2[1]:8d},{v2[2]:8d})")
    print()

# --- Analyze bone vertex ranges to determine global vs local space ---
PKT_SIZE = [12,28,20,28, 12,20,12,20, 16,24,12,24, 20,20,0,20]
IS_QUAD  = [0,0,0,0, 1,1,0,1, 0,0,0,0, 0,0,0,0]

print("=== Per-bone vertex centroid (raw BIN coords, no offset) ===")
bone_centroids = []  # [(cx, cz)] in BIN units
all_vx, all_vy, all_vz = [], [], []

for bi in range(nb):
    if btbase + bi*4 + 4 > bin_sz: break
    rel = rd32le(B, btbase + bi*4)
    bd  = btbase + rel
    if bd + 0x18 > bin_sz: break

    vc = rd32le(B, bd + 0x00)
    vr = rd32le(B, bd + 0x04)
    pc = rd16le(B, bd + 0x10)
    pr = rd32le(B, bd + 0x14)

    vo = bd + vr

    # Gather all vertex coords for this bone
    vxs, vys, vzs = [], [], []
    for vi in range(vc):
        oe = vo + vi * 8
        if oe + 6 > bin_sz: break
        vx = rds16le(B, oe+0)
        vy = rds16le(B, oe+2)
        vz = rds16le(B, oe+4)
        vxs.append(vx); vys.append(vy); vzs.append(vz)
        all_vx.append(vx); all_vy.append(vy); all_vz.append(vz)

    if vxs:
        cx = sum(vxs)/len(vxs)
        cy = sum(vys)/len(vys)
        cz = sum(vzs)/len(vzs)
        bone_centroids.append((cx, cy, cz))
        if bi < 20:
            print(f"  bone[{bi:3d}]: vc={vc:3d} pc={pc:3d}  "
                  f"centroid=({cx:8.1f},{cy:8.1f},{cz:8.1f})  "
                  f"xrange=[{min(vxs):5d}..{max(vxs):5d}]  "
                  f"zrange=[{min(vzs):5d}..{max(vzs):5d}]")
    else:
        bone_centroids.append((0,0,0))

print()

# Global vertex extents
if all_vx:
    print(f"=== Global raw vertex extents ===")
    print(f"  X: [{min(all_vx):.0f} .. {max(all_vx):.0f}]  ({(max(all_vx)-min(all_vx))*TERR_SCALE:.1f}m wide)")
    print(f"  Y: [{min(all_vy):.0f} .. {max(all_vy):.0f}]")
    print(f"  Z: [{min(all_vz):.0f} .. {max(all_vz):.0f}]  ({(max(all_vz)-min(all_vz))*TERR_SCALE:.1f}m deep)")
    print()
    print(f"  World X with WORLD_X={WORLD_X}: [{min(all_vx)*TERR_SCALE+WORLD_X:.1f} .. {max(all_vx)*TERR_SCALE+WORLD_X:.1f}]")
    print(f"  World Z with WORLD_Z={WORLD_Z}: [{min(all_vz)*TERR_SCALE+WORLD_Z:.1f} .. {max(all_vz)*TERR_SCALE+WORLD_Z:.1f}]")

# Centroid spread
if bone_centroids:
    cx_vals = [c[0] for c in bone_centroids]
    cz_vals = [c[2] for c in bone_centroids]
    print()
    print(f"=== Bone centroid spread (raw BIN units) ===")
    print(f"  Centroid X: [{min(cx_vals):.0f} .. {max(cx_vals):.0f}]  spread={max(cx_vals)-min(cx_vals):.0f} units  ({(max(cx_vals)-min(cx_vals))*TERR_SCALE:.1f}m)")
    print(f"  Centroid Z: [{min(cz_vals):.0f} .. {max(cz_vals):.0f}]  spread={max(cz_vals)-min(cz_vals):.0f} units  ({(max(cz_vals)-min(cz_vals))*TERR_SCALE:.1f}m)")
    print()
    print(f"  World centroid X: [{min(cx_vals)*TERR_SCALE+WORLD_X:.1f} .. {max(cx_vals)*TERR_SCALE+WORLD_X:.1f}]")
    print(f"  World centroid Z: [{min(cz_vals)*TERR_SCALE+WORLD_Z:.1f} .. {max(cz_vals)*TERR_SCALE+WORLD_Z:.1f}]")

# --- Key conclusion ---
print()
print("=== CONCLUSION ===")
print("If bone centroids span a large area (100+ m), vertices ARE in global space.")
print("If bone centroids all cluster near 0, vertices are in bone-local space (need per-bone transform).")
