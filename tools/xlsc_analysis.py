#!/usr/bin/env python3
"""
xlsc_analysis.py -- Analyse XLSC, AIMP chunks and verify XOBF BIN mesh vs ZONE heightmap.

Vigilante 8 PS1  --  OILFIELD.EXP IFF structure analysis

Usage:
    python tools/xlsc_analysis.py input/TERRAIN/OILFIELD.EXP
"""

import os
import sys
import struct
import math
import collections

# ---------------------------------------------------------------------------
# IFF helpers (big-endian size field, EA-IFF 85 dialect)
# ---------------------------------------------------------------------------

def be32(data, off):
    return struct.unpack_from(">I", data, off)[0]

def le16(data, off):
    return struct.unpack_from("<H", data, off)[0]

def le16s(data, off):
    return struct.unpack_from("<h", data, off)[0]

def le32(data, off):
    return struct.unpack_from("<I", data, off)[0]

def le32s(data, off):
    return struct.unpack_from("<i", data, off)[0]

def iff_walk(data, start, end, depth=0):
    """Walk an IFF tree; yield (tag_str, body_off, body_size, depth)."""
    p = start
    while p + 8 <= end:
        tag = data[p:p+4]
        if not all(0x20 <= b <= 0x7e for b in tag):
            break
        size = be32(data, p + 4)
        body = p + 8
        if body + size > end:
            break
        tag_s = tag.decode("ascii")
        if tag_s == "FORM":
            ftype = data[body:body+4].decode("ascii")
            yield (f"FORM/{ftype}", body, size, depth)
            yield from iff_walk(data, body + 4, body + size, depth + 1)
        else:
            yield (tag_s, body, size, depth)
        p = body + ((size + 1) & ~1)

def collect_tag(data, start, end, want):
    """Return list of (body_off, size) for all chunks matching 4cc `want`."""
    results = []
    for tag, body, size, _ in iff_walk(data, start, end):
        if tag == want:
            results.append((body, size))
    return results

# ---------------------------------------------------------------------------
# Section 1: XLSC chunk analysis
# ---------------------------------------------------------------------------

def analyze_xlsc(data, xlsc_list):
    print("=" * 70)
    print("XLSC CHUNK ANALYSIS")
    print("=" * 70)

    for idx, (body, size) in enumerate(xlsc_list):
        raw = data[body:body+size]
        print(f"\n--- XLSC[{idx}]  offset=0x{body:x}  size={size} (0x{size:x}) ---")
        print(f"  First 64 bytes hex: {raw[:64].hex()}")

        # Parse the common header bytes we can decode
        if size >= 12:
            b0  = raw[0]
            b1  = raw[1]
            b2  = raw[2]
            seq = raw[3]           # sequential index 00..03
            w4  = le32(raw, 4)     # 0x40017000 -- constant
            w8  = le16(raw, 8)     # varies  (e.g. 0x60, 0xe0, 0xa0, 0xe0 -> count?)
            w10 = le16(raw, 10)    # 0x1500,0x1300,0x1200,0x1300  (element count?)
            w12 = le16(raw, 12)    # 0x3804 constant
            w14 = le16(raw, 14)    # 0x0200 constant
            w16 = le16(raw, 16)    # 0x0020 constant
            w18 = le16(raw, 18)    # 0x8000 constant? (or 0x0080)
            print(f"  Header parse:")
            print(f"    byte[3]  seq_index = {seq}")
            print(f"    word[4]  = 0x{w4:08x}  (constant across chunks)")
            print(f"    u16[8]   = 0x{w8:04x}  = {w8}  (varies per chunk)")
            print(f"    u16[10]  = 0x{w10:04x}  = {w10}  (varies per chunk)")
            print(f"    u16[12]  = 0x{w12:04x}  (constant?)")
            print(f"    u16[14]  = 0x{w14:04x}  (constant?)")
            print(f"    u16[16]  = 0x{w16:04x}  (constant?)")
            print(f"    u16[18]  = 0x{w18:04x}  (constant?)")

        # --- Heightfield grid attempts ---
        print(f"\n  [A] Heightfield grid interpretations (all values as LE u16):")
        n_u16 = size // 2
        print(f"      Total u16 values: {n_u16}")
        dims_to_try = [
            (64, 64, "64x64 = ZONE-style"),
            (128, 64, "128x64"),
            (64, 32, "64x32"),
            (32, 32, "32x32 (with 0x20 hdr?)"),
        ]
        # compute stats on the whole u16 array first
        vals = struct.unpack_from(f"<{n_u16}H", raw)
        nz_vals = [v for v in vals if v != 0]
        if nz_vals:
            print(f"      Non-zero u16 count: {len(nz_vals)} / {n_u16}  ({100*len(nz_vals)/n_u16:.1f}%)")
            print(f"      Min={min(nz_vals):#06x}  Max={max(nz_vals):#06x}  "
                  f"Mean={sum(nz_vals)/len(nz_vals):.0f}")
            # 11-bit height check
            over11 = sum(1 for v in vals if (v >> 11) not in (0, 0x1f))
            print(f"      Values > 11-bit range: {over11} / {n_u16}  "
                  f"({100*over11/n_u16:.1f}%)")
            # high-byte distribution
            high_bytes = collections.Counter(v >> 8 for v in vals)
            top5_high = high_bytes.most_common(5)
            print(f"      Top high-bytes: {top5_high}")
        else:
            print(f"      All zeros!")

        for (rows, cols, label) in dims_to_try:
            n_cells = rows * cols
            if n_cells * 2 > size:
                print(f"      {label}: too large for chunk ({n_cells*2} > {size})")
                continue
            # hdr offset: try 0 and 20 (for a 20-byte header)
            for hdr in (0, 20, 22, 24):
                if hdr + n_cells * 2 > size:
                    continue
                grid = struct.unpack_from(f"<{n_cells}H", raw, hdr)
                nz = sum(1 for v in grid if v != 0)
                mn = min(grid) if grid else 0
                mx = max(grid) if grid else 0
                print(f"      {label} hdr_skip={hdr}: nz={nz}  min={mn:#06x}  max={mx:#06x}")

        # --- Fixed-point coordinate list ---
        print(f"\n  [B] As list of {size//6} 3×i16 vectors (if size%6==0: {size%6==0}):")
        if size % 6 == 0:
            n_vecs = size // 6
            vecs_s = [struct.unpack_from("<3h", raw, i*6) for i in range(min(n_vecs, 8))]
            for i, (x, y, z) in enumerate(vecs_s):
                print(f"      vec[{i}] = ({x}, {y}, {z})")

        print(f"\n  [C] As list of {size//8} (i16,i16,i16,u16) entries (if size%8==0: {size%8==0}):")
        if size % 8 == 0:
            n_ents = size // 8
            ents = [struct.unpack_from("<3hH", raw, i*8) for i in range(min(n_ents, 8))]
            for i, (x, y, z, flags) in enumerate(ents):
                print(f"      ent[{i}] = pos=({x},{y},{z})  flags=0x{flags:04x}")

        # -- Try to read a 'count' at known offsets and see if it makes sense
        print(f"\n  [D] Count-field search (looking for a u16 near top that = #records):")
        for off in range(0, min(32, size-1), 2):
            v = le16(raw, off)
            # does v * some_stride fit the remaining data?
            remaining = size - off - 2
            for stride in (2, 4, 6, 8, 10, 12, 16, 20, 24):
                if v > 0 and v < 4096 and v * stride == remaining:
                    print(f"      u16@{off}={v} * stride={stride} = {remaining} (matches remaining bytes)")

        # -- Pattern in last 8 bytes (sometimes a checksum or terminator)
        if size >= 8:
            print(f"\n  [E] Last 8 bytes: {raw[-8:].hex()}")
            print(f"      Second-to-last u32 LE: 0x{le32(raw, size-8):08x}")
            print(f"      Last u32 LE: 0x{le32(raw, size-4):08x}")

        # -- Byte histogram for first 64 bytes (after the 22-byte header)
        if size > 22:
            sample = raw[22:22+min(256, size-22)]
            low_bytes = [b for b in sample if b < 0x10]
            print(f"\n  [F] First 256 data bytes (after 22-byte hdr) -- "
                  f"low-value (<0x10) byte fraction: {len(low_bytes)/len(sample):.1%}")

    # --- Cross-chunk comparison at known identical region ---
    print("\n\nCross-chunk header comparison (bytes 4-19, expected constant):")
    if xlsc_list:
        for off in range(4, 20, 2):
            vals_at = [le16(data, xlsc_list[i][0] + off) for i in range(len(xlsc_list))]
            same = all(v == vals_at[0] for v in vals_at)
            print(f"  u16@{off:2d}: {[f'0x{v:04x}' for v in vals_at]}  {'CONSTANT' if same else 'varies'}")


# ---------------------------------------------------------------------------
# Section 2: AIMP chunk analysis
# ---------------------------------------------------------------------------

def analyze_aimp(data, aimp_list):
    print("\n" + "=" * 70)
    print("AIMP CHUNK ANALYSIS")
    print("=" * 70)

    for idx, (body, size) in enumerate(aimp_list):
        raw = data[body:body+size]
        print(f"\n--- AIMP[{idx}]  offset=0x{body:x}  size={size} (0x{size:x}) ---")
        print(f"  First 64 bytes: {raw[:64].hex()}")

        n_u16 = size // 2
        n_u32 = size // 4

        # Peek at possible record count
        if size >= 4:
            u16_0 = le16(raw, 0)
            u16_2 = le16(raw, 2)
            u32_0 = le32(raw, 0)
            print(f"  First u16 = {u16_0}  (0x{u16_0:04x})")
            print(f"  Second u16 = {u16_2}  (0x{u16_2:04x})")
            print(f"  First u32 LE = 0x{u32_0:08x}")

        # Try as a list of impact / collision patches
        # AIMP might stand for Ai iMPact or AI Map Patches
        # Try 8-byte records: (i16 x, i16 z, u16 radius, u16 flags)
        if size % 8 == 0:
            n_recs = size // 8
            print(f"\n  As {n_recs} × 8-byte records (i16,i16,u16,u16):")
            for i in range(min(n_recs, 12)):
                x, z, r, fl = struct.unpack_from("<hHhH", raw, i*8)
                print(f"    rec[{i:3d}]: x={x:6d}  z={z:6d}  r={r:5d}  flags=0x{fl:04x}")

        # Try 12-byte records
        if size % 12 == 0:
            n_recs = size // 12
            print(f"\n  As {n_recs} × 12-byte records (3×i16,3×u16):")
            for i in range(min(n_recs, 8)):
                vals = struct.unpack_from("<6h", raw, i*12)
                print(f"    rec[{i:3d}]: {vals}")

        # Try 16-byte records
        if size % 16 == 0:
            n_recs = size // 16
            print(f"\n  As {n_recs} × 16-byte records (8×i16):")
            for i in range(min(n_recs, 8)):
                vals = struct.unpack_from("<8h", raw, i*16)
                print(f"    rec[{i:3d}]: {vals}")

        # Try 20-byte records
        if size % 20 == 0:
            n_recs = size // 20
            print(f"\n  As {n_recs} × 20-byte records (10×i16):")
            for i in range(min(n_recs, 6)):
                vals = struct.unpack_from("<10h", raw, i*20)
                print(f"    rec[{i:3d}]: {vals}")

        # Value range analysis
        vals_u16 = struct.unpack_from(f"<{n_u16}H", raw)
        signed_vals = [v if v < 0x8000 else v - 0x10000 for v in vals_u16]
        print(f"\n  u16 stats: min={min(vals_u16):#06x} max={max(vals_u16):#06x}")
        print(f"  s16 stats: min={min(signed_vals)} max={max(signed_vals)}")

        # Check for null-terminated strings
        nulls = [i for i, b in enumerate(raw) if b == 0]
        print(f"  Zero bytes at: {nulls[:20]}")


# ---------------------------------------------------------------------------
# Section 3: Parse XOBF BIN mesh to extract ground triangles
# ---------------------------------------------------------------------------

def parse_bin_mesh(data, bin_body, bin_size):
    """
    Attempt to extract triangle vertices from the XOBF BIN blob.

    PSX BIN mesh format (inferred from Renderer_BuildObject context):
    - The format is proprietary; we use heuristic geometry detection.
    - We scan for plausible vertex triples (i16 x, i16 y, i16 z with |v| < 16384)
      and try to find coherent triangles by looking at stride patterns.

    The BIN is a PSX display-list format with GTE primitives.
    PSX GT3/G3 packets have:
      GT3: cmd(4) + rgb0(4)+rgb1(4)+rgb2(4) + v0(2)+pad(2)+v1(2)+pad(2)+v2(2)+pad(2) -> 28 bytes
      G3:  cmd(4) + rgb(4) + v0(2)+pad(2)+v1(2)+pad(2)+v2(2)+pad(2) -> 24 bytes

    But vertex data in PSX display lists is separate: the vertex buffer
    is typically a flat array of SVECTOR {s16 vx,vy,vz,pad} = 8 bytes each.

    We'll use two strategies:
    1. Look for a block of 8-byte SVECTOR-like entries near the front
    2. Scan for geometric primitives using magic-byte patterns

    Returns list of triangle tuples ((x0,y0,z0),(x1,y1,z1),(x2,y2,z2)).
    """
    raw = data[bin_body:bin_body+bin_size]

    # ----- Strategy 1: Header-driven vertex table -----
    # Check if the first few u32s look like an object header with counts
    triangles = []

    if bin_size < 32:
        return triangles

    # PSX object header unresolved field map:
    # offset 0: flags / id
    # offset 4: vertex count (u16)
    # offset 6: normal count (u16)  -- often same as vertex count
    # offset 8: primitive count (u16)
    # etc.
    possible_vert_counts = []
    for off in range(0, 32, 2):
        v = le16(raw, off)
        if 4 <= v <= 8192:
            possible_vert_counts.append((off, v))

    print(f"  [Mesh] BIN size = {bin_size:#x} ({bin_size} bytes)")
    print(f"  [Mesh] First 64 bytes: {raw[:64].hex()}")
    print(f"  [Mesh] Plausible count fields (4..8192): {possible_vert_counts[:12]}")

    # Try reading header as u16[8]
    hdr = struct.unpack_from("<8H", raw, 0)
    print(f"  [Mesh] First 8 u16s: {[hex(h) for h in hdr]}")

    # Try reading header as u32[8]
    hdr32 = struct.unpack_from("<8I", raw, 0)
    print(f"  [Mesh] First 8 u32s: {[hex(h) for h in hdr32]}")

    # ----- Strategy 2: Scan for SVECTOR blocks -----
    # An SVECTOR is {s16 vx, s16 vy, s16 vz, s16 pad} = 8 bytes
    # Valid world-space vertices for V8 terrain: |coord| < 32768 in PSX units
    # Typical PSX terrain scale: 1 unit ~ 1/16 or 1/32 metre
    # With world origin at (608, -2.25, 464) metres and PSX units
    # we expect vertices roughly in range [-2000..2000] PSX units for Y

    # Find longest run of "valid SVECTOR" at 8-byte alignment
    best_run_start = 0
    best_run_len   = 0
    run_start = None
    run_len   = 0
    COORD_LIMIT = 0x4000  # 16384 PSX units

    for i in range(0, min(bin_size - 8, 200000), 8):
        vx = le16s(raw, i)
        vy = le16s(raw, i + 2)
        vz = le16s(raw, i + 4)
        pad = le16(raw, i + 6)
        if abs(vx) < COORD_LIMIT and abs(vy) < COORD_LIMIT and abs(vz) < COORD_LIMIT:
            if run_start is None:
                run_start = i
                run_len = 1
            else:
                run_len += 1
            if run_len > best_run_len:
                best_run_len = run_len
                best_run_start = run_start
        else:
            run_start = None
            run_len = 0

    print(f"\n  [Mesh] Best SVECTOR run: {best_run_len} entries at byte offset {best_run_start:#x}")

    if best_run_len >= 3:
        verts = []
        for i in range(best_run_len):
            off = best_run_start + i * 8
            vx = le16s(raw, off)
            vy = le16s(raw, off + 2)
            vz = le16s(raw, off + 4)
            verts.append((vx, vy, vz))

        # Sample some vertices
        print(f"  [Mesh] First 12 vertices (vx,vy,vz):")
        for i, v in enumerate(verts[:12]):
            print(f"         [{i:4d}] {v}")
        if len(verts) > 12:
            print(f"         ...")
            for i, v in enumerate(verts[-4:]):
                print(f"         [{len(verts)-4+i:4d}] {v}")

        # Compute bounding box
        xs = [v[0] for v in verts]
        ys = [v[1] for v in verts]
        zs = [v[2] for v in verts]
        print(f"  [Mesh] Bounding box (PSX units):")
        print(f"         X: [{min(xs)}, {max(xs)}]  span={max(xs)-min(xs)}")
        print(f"         Y: [{min(ys)}, {max(ys)}]  span={max(ys)-min(ys)}")
        print(f"         Z: [{min(zs)}, {max(zs)}]  span={max(zs)-min(zs)}")

        return verts, best_run_start, best_run_len

    return [], 0, 0


# ---------------------------------------------------------------------------
# Section 4: ZONE heightmap parsing
# ---------------------------------------------------------------------------

def parse_zones(data, zmap_body, zmap_size, zone_list):
    """
    Returns:
      zone_map: dict (cx,cz) -> zone_index (1-based)
      zone_heights: list of 2D arrays (64x64 u11 heights)
    """
    # ZMAP: 32x32 BE u16, value=0 means empty, N means ZONE[N-1]
    zone_map = {}
    for cx in range(32):
        for cz in range(32):
            off = (cx * 32 + cz) * 2
            idx = be32(data, zmap_body + off) >> 16  # it's stored as BE u16 in 2 bytes
            # Actually ZMAP is just 2048 bytes = 32*32*2, each is a BE u16:
            idx = ((data[zmap_body + off] << 8) | data[zmap_body + off + 1])
            if idx != 0:
                zone_map[(cx, cz)] = idx

    zone_heights = []
    for body, size in zone_list:
        # Each ZONE is 16384 bytes = 128x64 u16 (or 64x128?)
        # Actually: 64 rows * 64 cols * 2 bytes = 0x2000; if zone is 0x4000 there's something extra
        # terrain_height.c stride: row_stride=0x80 (64 u16), col_stride=2
        # So layout is [col][row]: cell = raw[col_x * 0x80 + col_z * 2] & 0x7ff
        # = 64 X-cols × 64 Z-rows × 2 bytes = 0x2000 per heightmap layer
        # ZONE might be 0x4000 = two layers (heights + material IDs)
        zone_sz = size
        heights = []
        # Read 64 x-columns, each of 64 z-rows
        for cx_cell in range(64):
            row = []
            for cz_cell in range(64):
                off = body + cx_cell * 0x80 + cz_cell * 2
                if off + 2 <= body + zone_sz:
                    h = le16(data, off) & 0x7ff
                    row.append(h)
                else:
                    row.append(0)
            heights.append(row)
        zone_heights.append(heights)

    return zone_map, zone_heights


# ---------------------------------------------------------------------------
# Section 5: Verify XOBF mesh Y vs ZONE heightmap Y
# ---------------------------------------------------------------------------

# V8 terrain scale factors (from terrain_height.c analysis):
#
# Terrain_HeightAt returns: (h * 0x10000 + bilinear) >> 5
# So the raw 11-bit height h maps to world-Y via: world_Y = h * 0x800 (approx)
# In metres (assuming 1 PSX unit = 1/something metres):
#
# From the level: ZONE starts at cx=13,cz=18 in the 32x32 chunk grid.
# Each chunk covers 64 cells; each cell in fixed-point is 1/65536 of the
# 22-bit chunk address space? No -- from terrain_height.c:
#   chunk_x = x >> 22  => chunk is 2^22 = 4M units wide in world space
#   cell_x  = (x >> 16) & 0x3f => 64 cells per chunk, each 2^16 wide
#   => 1 cell = 65536 world units
#
# The world-coord system: from the mesh discussion, world origin is
# (608, -2.25, 464) metres when the PSX level-space origin is (0,0,0).
# But the XOBF BIN vertices are in PSX model-space already.
#
# The height return: (h * 0x10000) >> 5 = h * 2048 in world Y units.
# If world Y units are the same as world X/Z units (they should be),
# then 1 cell = 65536 world units, and Y range for h in [0..2047] is
# [0..2047*2048] = [0..4M] world units -- same order as a chunk width.
# So 1 world unit is NOT 1 metre.
#
# From physics: PSX fixed-point world: 1 unit = 1/4096 metre (common PSX scale)
# => 65536 world units per cell = 16 metres per cell
# => 64 cells per chunk = 1024 metres per chunk = ~1 km
# That's implausibly large for V8 levels.
#
# More likely: world-coord is just cell coordinates shifted left 16 bits
# (i.e., the integer part selects the cell, the fraction is interpolation).
# Physical scale: from the problem statement, ZONE covers 64x96 m.
# That's 64 cells * 1 m/cell (x) and 96 cells * 1 m/cell (z) = fine.
# So 1 cell = 1 metre, and world_coord_int_part = metres.
#
# In that case h * 2048 >> 5 = h * 64 world units = h * 64 metres?? No.
# Actually: h * 0x10000 >> 5 = h * 0x800 = h * 2048. But the bilinear is
# in the same world units as x/z, so if x is in metres as integer,
# the result is in metres. Then h=1 -> Y=2048 m which is absurd.
#
# Re-read: the >>5 converts to the "engine's 17.15-ish world-Y units".
# So the return value is in 17.15 fixed-point. Actual metres = return/32768.
# Then h=1 -> Y = 1 * 2048 / 32768 = 0.0625 metres per unit.
# Range h in [0..2047]: Y in [0..128] metres. That's plausible for terrain.
#
# XOBF BIN vertices (PSX model space, s16):
# If the BIN vertices are in the same 1/4096 m PSX units, then
# vert_Y (in metres) = vert_Y_s16 / 4096.
# But model space might be metres already at 1:1.
#
# Cross-check: we need the world-space origin of the BIN model.
# The problem statement says XOBF is placed at world (608, -2.25, 464) metres.
# ZONE chunks start at chunk (13,18): world_x = 13*64 = 832, world_z = 18*64 = 1152 cells.
# So ZONE spans x=[832..896] cells, z=[1152..1248] cells (the 6 zones: 2×3 or 3×2 arrangement).
# Actually with cx=[13..15] and cz=[18..19]: spans x=3 chunks, z=2 chunks = 192x128 cells = 192x128 m.
# The problem says 64x96 m, so the cell = 1/3 m? Or some ZONE chunks are empty.
#
# We'll just do an empirical comparison: compute the height predicted by the ZONE for
# cells inside it, and compare with XOBF BIN mesh Y at the same XZ location.

PSX_UNIT_TO_METRES = 1.0 / 4096.0   # if PSX vertices are in 1/4096 m units

# Terrain height conversion: h (11-bit) -> metres
# Return from Terrain_HeightAt is 17.15 fixed-point of world Y (in metres if units are metres)
def zone_height_to_metres(h):
    """Convert 11-bit ZONE height sample to metres (world Y)."""
    # (h * 0x10000) >> 5 = h * 2048
    # That result is in "world Y units". If world X units are 1 m/cell,
    # and the bilinear is in x-fraction (0..0xffff), the ratio of Y-range to X-cell-size
    # must be consistent. Let's treat the 17.15 result as: metres = val / 32768.
    # Then h -> h * 2048 / 32768 = h / 16.
    # So h=1 is 0.0625 m, h=2047 is 127.9 m. Plausible for PS1 terrain.
    return h / 16.0


def analyze_mesh_vs_zone(data, bin_body, bin_size, zmap_body, zmap_size, zone_list,
                         world_origin_x_m=608.0, world_origin_y_m=-2.25, world_origin_z_m=464.0):
    """
    Check whether XOBF BIN mesh Y values match ZONE heights at the same XZ.
    """
    print("\n" + "=" * 70)
    print("XOBF BIN MESH vs ZONE HEIGHTMAP CROSS-CHECK")
    print("=" * 70)

    # Parse ZONE heightmap
    zone_map, zone_heights = parse_zones(data, zmap_body, zmap_size, zone_list)
    print(f"\nZMAP non-empty cells: {len(zone_map)}")
    for (cx, cz), idx in sorted(zone_map.items()):
        print(f"  chunk ({cx:2d},{cz:2d}) -> ZONE[{idx-1}]")

    # Parse BIN mesh vertices
    verts_result = parse_bin_mesh(data, bin_body, bin_size)
    if not verts_result or not verts_result[0]:
        print("  No valid vertex run found in BIN mesh -- skipping cross-check.")
        return

    verts, vert_start, vert_count = verts_result
    if not verts:
        print("  Empty vertex list -- skipping.")
        return

    print(f"\nExtracted {vert_count} vertices from BIN mesh (SVECTOR scan).")
    print(f"World origin of model: X={world_origin_x_m} m, Y={world_origin_y_m} m, Z={world_origin_z_m} m")

    # The XOBF mesh is placed at (world_origin_x_m, world_origin_y_m, world_origin_z_m)
    # in metres. The ZONE grid cell (cx, cz) in world-cell coords maps to:
    #   world_x_m = cx * 1.0 (if cells are 1m)  -- needs calibration
    # We don't know the PSX model scale for BIN vertices yet, so we try several scales.

    # The ZONE chunk (cx=13..15, cz=18..19) in world cell coords:
    # If we assume 1 cell = 1 metre: ZONE covers x=[832..896], z=[1152..1248] metres (ish)
    # Model placed at (608, -2.25, 464) metres.
    # So ZONE is at offset +224..288 m in X and +688..784 m in Z from model origin.
    # That makes ZONE seem far from origin; unlikely.
    #
    # Alternative: the "32x32 chunk grid" with "64 cells/chunk" actually means
    # the ENTIRE 32*64 = 2048 cells cover the V8 level (say ~512 m).
    # => each cell = 512/2048 = 0.25 metres.
    # ZONE cx=13..15: x = 13*64*0.25 = 208 m to 15*64*0.25 = 240 m (Δ=32 m per chunk, 3 chunks=96 m)
    # ZONE cz=18..19: z = 18*64*0.25 = 288 m to 19*64*0.25 = 304 m (Δ=16 m per chunk, 2 chunks=32 m)
    # Hmm, 96x32 m; problem says 64x96 m (possibly x and z are swapped).
    #
    # Alternatively with world_origin being the mesh corner (not center):
    # If the mesh is 169 m wide (x) and 180 m deep (z), placed with its origin at (608,-2.25,464),
    # and the ZONE covers the top-right quadrant...
    #
    # We'll just enumerate a bunch of (scale, offset) combinations and compute correlation.

    CELL_SCALES = [0.25, 0.5, 1.0, 2.0]  # metres per cell

    print("\n--- Trying different cell scales for ZONE->world mapping ---")
    for cell_scale in CELL_SCALES:
        # ZONE starts at chunk cx0=13, cz0=18 (the minimum non-empty chunk).
        # Within chunk cx, cells run [cx*64 .. (cx+1)*64 - 1].
        cx0_cells = min(cx for (cx, cz) in zone_map) * 64
        cz0_cells = min(cz for (cx, cz) in zone_map) * 64
        # World coords of ZONE lower-left corner:
        zone_x0_m = cx0_cells * cell_scale
        zone_z0_m = cz0_cells * cell_scale

        print(f"\n  Cell scale = {cell_scale} m/cell:")
        print(f"    ZONE world origin: x={zone_x0_m:.1f} m, z={zone_z0_m:.1f} m")
        print(f"    Model world origin: x={world_origin_x_m:.1f} m, z={world_origin_z_m:.1f} m")
        # Offset of ZONE in model-space (PSX model coords):
        delta_x_m = zone_x0_m - world_origin_x_m
        delta_z_m = zone_z0_m - world_origin_z_m
        print(f"    ZONE in model-space: delta_x={delta_x_m:.1f} m, delta_z={delta_z_m:.1f} m")

        # If PSX verts are in PSX units (1/4096 m), convert to metres:
        for psx_scale in [1.0/4096, 1.0/256, 1.0/64, 1.0/16, 1.0]:
            # Count how many vertices fall within the ZONE footprint (in model-space)
            zone_x_end_m = (max(cx for (cx,cz) in zone_map) + 1) * 64 * cell_scale
            zone_z_end_m = (max(cz for (cx,cz) in zone_map) + 1) * 64 * cell_scale
            # Model-space bounds of ZONE:
            mx0 = delta_x_m / psx_scale if psx_scale != 0 else 0
            mz0 = delta_z_m / psx_scale if psx_scale != 0 else 0
            mx1 = (zone_x_end_m - world_origin_x_m) / psx_scale
            mz1 = (zone_z_end_m - world_origin_z_m) / psx_scale

            inside = [(vx, vy, vz) for (vx, vy, vz) in verts
                      if mx0 <= vx <= mx1 and mz0 <= vz <= mz1]

            if len(inside) > 10:
                print(f"      PSX_scale={psx_scale:.6f} m/unit: {len(inside)} verts inside ZONE region")
                print(f"        Model-space box: x=[{mx0:.1f}..{mx1:.1f}]  z=[{mz0:.1f}..{mz1:.1f}]")

                # For each inside vertex, look up ZONE height:
                mismatches = 0
                matches = 0
                sample_rows = []
                for (vx, vy, vz) in inside[:200]:
                    # Convert model vert to world metres:
                    wx_m = vx * psx_scale + world_origin_x_m
                    wy_m = vy * psx_scale + world_origin_y_m
                    wz_m = vz * psx_scale + world_origin_z_m
                    # Convert world metres to cell coords:
                    cell_x = wx_m / cell_scale
                    cell_z = wz_m / cell_scale
                    # Look up chunk and cell:
                    cx_chunk = int(cell_x) // 64
                    cz_chunk = int(cell_z) // 64
                    cx_cell  = int(cell_x) % 64
                    cz_cell  = int(cell_z) % 64
                    key = (cx_chunk, cz_chunk)
                    if key in zone_map:
                        zone_idx = zone_map[key] - 1
                        if zone_idx < len(zone_heights):
                            h = zone_heights[zone_idx][cx_cell][cz_cell]
                            zone_y_m = zone_height_to_metres(h)
                            delta_y = abs(wy_m - zone_y_m)
                            if delta_y < 2.0:
                                matches += 1
                            else:
                                mismatches += 1
                            if len(sample_rows) < 5:
                                sample_rows.append(
                                    f"          vert=({vx},{vy},{vz}) world=({wx_m:.1f},{wy_m:.2f},{wz_m:.1f})"
                                    f" cell=({cx_chunk}.{cx_cell},{cz_chunk}.{cz_cell})"
                                    f" h={h} zone_y={zone_y_m:.2f}m  dy={delta_y:.2f}m"
                                )
                for r in sample_rows:
                    print(r)
                if matches + mismatches > 0:
                    print(f"        MATCH RATE: {matches}/{matches+mismatches} "
                          f"= {100*matches/(matches+mismatches):.1f}% (Δy < 2m)")


# ---------------------------------------------------------------------------
# Section 6: Ground-triangle filter on XOBF BIN (normal-based)
# ---------------------------------------------------------------------------

def analyze_ground_triangles(data, bin_body, bin_size, zmap_body, zmap_size, zone_list):
    """
    Since we can't easily decode the PSX display list primitives without full
    format knowledge, we instead look for normal-like data or estimate normals
    from triangles reconstructed from the vertex stream.

    Report how many vertex clusters have Y values in the "ground band" [-15..12 m]
    at common PSX scales.
    """
    print("\n" + "=" * 70)
    print("XOBF BIN GROUND TRIANGLE FILTER")
    print("=" * 70)

    raw = data[bin_body:bin_body+bin_size]

    # The ground band from the problem statement: centroid Y between -15m and +12m
    # and normal.y > 0.5 (upward-facing polygon).
    # Without a decoded primitive list we work on the vertex soup.

    # Try all plausible stride/start combinations to find vertex soup
    for stride in [8, 6, 4]:
        for start in [0, 8, 16, 24, 32, 48, 64]:
            if start + stride * 3 > bin_size:
                continue
            # Compute max coord at this stride/start
            count = (bin_size - start) // stride
            if count < 3:
                continue
            coords = []
            all_sane = True
            for i in range(min(count, 2000)):
                vx = le16s(raw, start + i * stride)
                vy = le16s(raw, start + i * stride + 2)
                if stride >= 6:
                    vz = le16s(raw, start + i * stride + 4)
                else:
                    vz = 0
                if abs(vx) > 16383 or abs(vy) > 16383 or abs(vz) > 16383:
                    all_sane = False
                    break
                coords.append((vx, vy, vz))
            if not all_sane or len(coords) < 3:
                continue
            ys = [c[1] for c in coords]
            y_range = max(ys) - min(ys)
            if y_range < 10 or y_range > 30000:
                continue
            print(f"\n  stride={stride} start={start}: {len(coords)} sane verts")
            print(f"    Y range (PSX): [{min(ys)}, {max(ys)}]  span={y_range}")
            xs = [c[0] for c in coords]
            zs = [c[2] for c in coords]
            print(f"    X range: [{min(xs)}, {max(xs)}]")
            print(f"    Z range: [{min(zs)}, {max(zs)}]")

            # For each plausible PSX scale, show how many verts are in ground band
            for psx_to_m in [1.0/4096, 1.0/256, 1.0/64, 1.0/16, 1.0/4, 1.0]:
                ground_band = [c for c in coords
                               if -15.0 <= c[1] * psx_to_m <= 12.0]
                if len(ground_band) > len(coords) * 0.1:
                    print(f"    scale={psx_to_m:.5f} m/PSX: {len(ground_band)}/{len(coords)} "
                          f"verts in Y=[-15..+12]m")

            break  # only first sane interpretation per stride


# ---------------------------------------------------------------------------
# Section 7: ZMAP and ZONE quick summary for context
# ---------------------------------------------------------------------------

def summarize_zmap_zones(data, zmap_body, zmap_size, zone_list):
    print("\n" + "=" * 70)
    print("ZMAP + ZONE SUMMARY")
    print("=" * 70)

    print(f"\nZMAP size: {zmap_size} bytes (expected 0x800=2048)")
    zone_map = {}
    non_zero = []
    for cx in range(32):
        for cz in range(32):
            off = (cx * 32 + cz) * 2
            idx = (data[zmap_body + off] << 8) | data[zmap_body + off + 1]
            if idx != 0:
                zone_map[(cx, cz)] = idx
                non_zero.append((cx, cz, idx))
    print(f"Non-zero ZMAP entries: {len(non_zero)}")
    for cx, cz, idx in non_zero:
        print(f"  ({cx:2d},{cz:2d}) -> ZONE[{idx-1}]")

    print(f"\nZONE chunks: {len(zone_list)}")
    for i, (body, size) in enumerate(zone_list):
        raw = data[body:body+size]
        # First 8 u16s
        preview = [le16(raw, j*2) for j in range(8)]
        # Height stats (first 0x2000 bytes as u11 heights)
        n = min(size // 2, 0x1000)
        hs = [le16(raw, j*2) & 0x7ff for j in range(n)]
        nz_h = [h for h in hs if h > 0]
        print(f"  ZONE[{i}] size={size:#x}: first_u16s={[hex(v) for v in preview]}")
        print(f"          heights: non-zero={len(nz_h)}/{n}  "
              f"min={min(hs)}  max={max(hs)}  "
              f"mean={sum(nz_h)/len(nz_h):.1f}" if nz_h else "all zero")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    if len(sys.argv) < 2:
        path = r"input\TERRAIN\OILFIELD.EXP"
    else:
        path = sys.argv[1]

    if not os.path.exists(path):
        print(f"ERROR: file not found: {path}")
        sys.exit(1)

    with open(path, "rb") as fh:
        data = fh.read()
    print(f"File: {path}")
    print(f"Size: {len(data):#x} ({len(data)} bytes)")

    # Walk IFF tree for chunk inventory
    start = 0
    end   = len(data)
    xlsc_list  = collect_tag(data, start, end, "XLSC")
    aimp_list  = collect_tag(data, start, end, "AIMP")
    zmap_list  = collect_tag(data, start, end, "ZMAP")
    zone_list  = collect_tag(data, start, end, "ZONE")
    bin_list   = collect_tag(data, start, end, "BIN ")

    print(f"\nChunk counts: XLSC={len(xlsc_list)} AIMP={len(aimp_list)} "
          f"ZMAP={len(zmap_list)} ZONE={len(zone_list)} BIN={len(bin_list)}")

    # Show raw chunk positions
    print("\nAll top-level + nested chunks:")
    for tag, body, size, depth in iff_walk(data, start, end):
        indent = "  " * depth
        print(f"  {indent}{tag:12s} body=0x{body:x}  size={size:#x} ({size})")

    # Run analyses
    if xlsc_list:
        analyze_xlsc(data, xlsc_list)

    if aimp_list:
        analyze_aimp(data, aimp_list)

    zmap_body, zmap_size = zmap_list[0] if zmap_list else (0, 0)

    if zmap_body and zone_list:
        summarize_zmap_zones(data, zmap_body, zmap_size, zone_list)

    if bin_list:
        bin_body, bin_size = bin_list[0]
        print("\n" + "=" * 70)
        print("XOBF BIN MESH RAW ANALYSIS")
        print("=" * 70)
        parse_bin_mesh(data, bin_body, bin_size)

        if zmap_body and zone_list:
            analyze_mesh_vs_zone(data, bin_body, bin_size,
                                 zmap_body, zmap_size, zone_list)
            analyze_ground_triangles(data, bin_body, bin_size,
                                     zmap_body, zmap_size, zone_list)

    print("\n\nDONE.")


if __name__ == "__main__":
    main()
