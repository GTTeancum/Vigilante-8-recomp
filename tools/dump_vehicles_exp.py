import struct, sys, os

PATH = r"C:\Programming\GitHub\Vigilante 8 recomp\input\SHELL\VEHICLES.EXP"

def read32be(data, off):
    return struct.unpack_from('>i', data, off)[0]

def read32le(data, off):
    return struct.unpack_from('<i', data, off)[0]

def read16le(data, off):
    return struct.unpack_from('<h', data, off)[0]

def read16ule(data, off):
    return struct.unpack_from('<H', data, off)[0]

def walk_iff(data, off, end, depth=0, bins=None):
    if bins is None:
        bins = []
    while off + 8 <= end:
        chunk_id = data[off:off+4]
        size = struct.unpack_from('>I', data, off+4)[0]
        chunk_end = off + 8 + size
        id_str = chunk_id.decode('latin-1')
        if id_str in ('FORM', 'LIST'):
            inner_type = data[off+8:off+12].decode('latin-1')
            walk_iff(data, off+12, chunk_end, depth+1, bins)
        elif id_str == 'BIN ':
            bins.append((off+8, size))  # payload start
        off = chunk_end
        if off % 2 == 1:
            off += 1
    return bins

data = open(PATH, 'rb').read()
print(f"File size: {len(data)} bytes")

bins = walk_iff(data, 0, len(data))
print(f"Found {len(bins)} BIN chunks\n")

for veh_idx in range(min(3, len(bins))):
    bin_start, bin_size = bins[veh_idx]
    print(f"=== Vehicle {veh_idx} (BIN payload at 0x{bin_start:X}, size {bin_size}) ===")

    num_bones       = read32le(data, bin_start + 0)
    bone_table_off  = read32le(data, bin_start + 4)
    count2          = read32le(data, bin_start + 8)
    offset3         = read32le(data, bin_start + 12)
    num_segments    = read32le(data, bin_start + 16)
    seg_table_off   = read32le(data, bin_start + 20)

    print(f"  num_bones={num_bones} bone_table_off=0x{bone_table_off:X} count2={count2} num_segments={num_segments}")

    bone_table_base = bin_start + bone_table_off

    total_tris = 0
    total_quads = 0

    for bi in range(num_bones):
        entry_off = bone_table_base + bi * 4
        rel = read32le(data, entry_off)
        desc = bone_table_base + rel

        vert_count     = read32le(data, desc + 0x00)
        vert_table_off = read32le(data, desc + 0x04)
        norm_count     = read32le(data, desc + 0x08)
        norm_table_off = read32le(data, desc + 0x0C)
        poly_count     = read16le(data, desc + 0x10)
        unk            = read16le(data, desc + 0x12)
        poly_data_off  = read32le(data, desc + 0x14)

        vert_base = desc + vert_table_off
        xs = [read16le(data, vert_base + v*8 + 0) for v in range(vert_count)]
        ys = [read16le(data, vert_base + v*8 + 2) for v in range(vert_count)]
        zs = [read16le(data, vert_base + v*8 + 4) for v in range(vert_count)]

        if vert_count > 0:
            xr = (min(xs), max(xs))
            yr = (min(ys), max(ys))
            zr = (min(zs), max(zs))
        else:
            xr = yr = zr = (0, 0)

        print(f"  bone {bi:2d}: verts={vert_count:3d} polys={poly_count:3d}  "
              f"X[{xr[0]:6d},{xr[1]:6d}] Y[{yr[0]:6d},{yr[1]:6d}] Z[{zr[0]:6d},{zr[1]:6d}]")

        if veh_idx == 0 and bi == 0:
            print(f"    First 6 verts of vehicle 0 bone 0:")
            for v in range(min(6, vert_count)):
                vx = read16le(data, vert_base + v*8 + 0)
                vy = read16le(data, vert_base + v*8 + 2)
                vz = read16le(data, vert_base + v*8 + 4)
                vw = read16le(data, vert_base + v*8 + 6)
                print(f"      vert[{v}]: x={vx} y={vy} z={vz} w={vw}")

        # Count tris vs quads for vehicle 0
        if veh_idx == 0 and poly_count > 0:
            poly_base = desc + poly_data_off
            off = 0
            tris = 0
            quads = 0
            for pi in range(poly_count):
                if poly_base + off + 4 > len(data):
                    break
                raw_type = data[poly_base + off + 3]
                lo = raw_type & 0x0F
                # Determine if tri or quad by checking vertex index count
                # PSX GP0 primitives: type byte encodes format
                # bit2 of type = quad flag typically
                # We use the GT3/GT4 / F3/F4 pattern from PSX GPU
                # type & 0x08 → shaded quad (4 verts), type & 0x04 → textured
                # Standard: 0x34=GT3, 0x3C=GT4, 0x24=FT3, 0x2C=FT4, etc.
                # Low nibble: 0x4=tri-textured, 0xC=quad-textured in some encodings
                # Try: check hi nibble for quad vs tri
                hi = (raw_type >> 4) & 0x0F
                is_quad = bool(raw_type & 0x08)  # bit 3 usually = quad

                # Estimate packet size by type
                # PSX standard:
                #   F3 (0x20)=8w, FT3(0x24)=9w, G3(0x30)=9w, GT3(0x34)=12w
                #   F4(0x28)=9w, FT4(0x2C)=12w, G4(0x38)=12w, GT4(0x3C)=16w
                # sizes in bytes (words*4):
                type_map = {
                    0x20: 12, 0x22: 12,  # F3
                    0x24: 24, 0x26: 24,  # FT3
                    0x28: 16, 0x2A: 16,  # F4
                    0x2C: 28, 0x2E: 28,  # FT4
                    0x30: 16, 0x32: 16,  # G3
                    0x34: 28, 0x36: 28,  # GT3
                    0x38: 20, 0x3A: 20,  # G4
                    0x3C: 36, 0x3E: 36,  # GT4
                    0x40: 12,            # sprite etc
                }
                pkt_size = type_map.get(raw_type & 0xFE, 0)
                if pkt_size == 0:
                    # fallback heuristic
                    pkt_size = 12 if lo < 4 else 20

                if is_quad:
                    quads += 1
                else:
                    tris += 1
                off += pkt_size

            total_tris += tris
            total_quads += quads
            if poly_count > 0:
                pass  # reported per-bone below

    if veh_idx == 0:
        print(f"\n  Vehicle 0 polygon summary: tris={total_tris} quads={total_quads}")
    print()
