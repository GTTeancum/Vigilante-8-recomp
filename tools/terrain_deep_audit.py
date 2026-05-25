"""terrain_deep_audit.py -- headless EXP terrain/placement audit.

Scans every terrain EXP for:
  * chunk inventory
  * ZMAP/ZONEx detailed height tile footprint
  * FORM OBJ/HEAD instance coordinates
  * JUNC road graph coordinates
  * XOBF BIN vertex extents
  * unknown chunks containing plausible 16.16 coordinate pairs

This intentionally does not open a window or use SDL/GL.
"""
from __future__ import annotations

import argparse
import math
import os
import struct
from collections import Counter, defaultdict


def be32(b: bytes, o: int) -> int:
    return struct.unpack_from(">I", b, o)[0]


def sbe32(b: bytes, o: int) -> int:
    return struct.unpack_from(">i", b, o)[0]


def le32(b: bytes, o: int) -> int:
    return struct.unpack_from("<I", b, o)[0]


def sle32(b: bytes, o: int) -> int:
    return struct.unpack_from("<i", b, o)[0]


def le16(b: bytes, o: int) -> int:
    return struct.unpack_from("<H", b, o)[0]


def sle16(b: bytes, o: int) -> int:
    return struct.unpack_from("<h", b, o)[0]


def walk_iff(raw: bytes):
    def walk_range(p: int, end: int, parent: bytes):
        while p + 8 <= end:
            tag = raw[p:p + 4]
            size = be32(raw, p + 4)
            body = p + 8
            if body + size > end:
                break
            yield p, tag, size, body, parent, raw[body:body + size]
            if tag == b"FORM" and size >= 4:
                form_type = raw[body:body + 4]
                yield from walk_range(body + 4, body + size, form_type)
            p = body + size + (size & 1)
    yield from walk_range(0, len(raw), b"")


def collect(raw: bytes, tag_name: bytes):
    return [(off, size, body, parent, data)
            for off, tag, size, body, parent, data in walk_iff(raw)
            if tag == tag_name]


def extent(vals):
    return (min(vals), max(vals)) if vals else None


def fmt_ext(ext):
    if not ext:
        return "none"
    return f"[{ext[0]:.1f}..{ext[1]:.1f}] span={ext[1] - ext[0]:.1f}"


def decode_zmap(data: bytes):
    cells = []
    if len(data) != 0x800:
        return cells
    for row in range(32):
        for col in range(32):
            idx = (data[(row * 32 + col) * 2] << 8) | data[(row * 32 + col) * 2 + 1]
            if idx:
                cells.append((row, col, idx))
    return cells


def decode_heads(raw: bytes):
    heads = []
    for off, size, body, parent, data in collect(raw, b"HEAD"):
        if parent != b"OBJ " or size < 34:
            continue
        # LOAD.DLL 801006f0 reads object HEAD as:
        #   u8@0 category, u8@1 type, i16@2 sort/id, u32@4 flags,
        #   i32@8 z, i32@12 y, i32@16 x, i16@20/22/24 rot YXZ,
        #   i16@26 XOBF table selector, i16@28 bone slot, i32@30 aux,
        #   ascii name @34.
        # MIPS delay-slot note: Ghidra's 801006f0 pseudo-C shifts the final
        # two i16 roles by one read.  The value used for DAT_800737a0[index]
        # is the return value from the +26 read in the delay slot before the
        # next read; +28 is passed to FUN_80021b80 as the bone/tree slot.
        # The previous audit treated 8002249c as u16 and shifted every field.
        x = sbe32(data, 8) / 65536.0
        y = (sbe32(data, 12) - 0x100000) / 131072.0
        z = sbe32(data, 16) / 65536.0
        raw_name = data[34:size].split(b"\0", 1)[0]
        name = raw_name.decode("ascii", "replace")
        heads.append({
            "x": x,
            "y": y,
            "z": z,
            "raw_coord8": x,
            "raw_coord16": z,
            "name": name,
            "off": off,
            "category": data[0],
            "type": data[1],
            "id": struct.unpack_from(">h", data, 2)[0],
            "flags": be32(data, 4),
            "rot_y": struct.unpack_from(">h", data, 20)[0],
            "rot_x": struct.unpack_from(">h", data, 22)[0],
            "rot_z": struct.unpack_from(">h", data, 24)[0],
            "bank": struct.unpack_from(">h", data, 26)[0],
            "slot": struct.unpack_from(">h", data, 28)[0],
            "aux": sbe32(data, 30),
        })
    return heads


def decode_junc(raw: bytes):
    pts = []
    for off, size, body, parent, data in collect(raw, b"JUNC"):
        if size >= 8:
            x = sbe32(data, 0) / 65536.0
            z = sbe32(data, 4) / 65536.0
            pts.append((x, z, off, size))
    return pts


PKT_SIZE = [12, 28, 20, 28, 12, 20, 12, 20, 16, 24, 12, 24, 20, 20, 0, 20]
IS_QUAD = [0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]


def xobf_bins(raw: bytes):
    bins = []
    for off, tag, size, body, parent, data in walk_iff(raw):
        if tag == b"BIN " and parent == b"XOBF":
            bins.append((body, size, data))
    return bins


def bin_extents(data: bytes, scale: float = 1.0 / 16.0):
    if len(data) < 8:
        return None
    nb = le32(data, 0)
    btbase = le32(data, 4)
    xs, ys, zs = [], [], []
    tris = 0
    bad = 0
    for bi in range(nb):
        if btbase + bi * 4 + 4 > len(data):
            break
        rel = le32(data, btbase + bi * 4)
        bd = btbase + rel
        if bd + 0x18 > len(data):
            bad += 1
            continue
        vc = le32(data, bd)
        vr = le32(data, bd + 4)
        pc = le16(data, bd + 0x10)
        pr = le32(data, bd + 0x14)
        vo = bd + vr
        po = bd + pr
        for vi in range(vc):
            oe = vo + vi * 8
            if oe + 6 > len(data):
                break
            xs.append(sle16(data, oe) * scale)
            ys.append(sle16(data, oe + 2) * scale)
            zs.append(sle16(data, oe + 4) * scale)
        for _ in range(pc):
            if po + 4 > len(data):
                break
            nib = data[po + 3] & 0xf
            sz = PKT_SIZE[nib]
            if sz == 0 or po + sz > len(data):
                bad += 1
                po += 4
                continue
            tris += 2 if IS_QUAD[nib] else 1
            po += sz
    if not xs:
        return None
    return {
        "nb": nb,
        "verts": len(xs),
        "tris": tris,
        "bad": bad,
        "x": extent(xs),
        "y": extent(ys),
        "z": extent(zs),
    }


def plausible_pairs(raw: bytes, min_span: float = 250.0):
    """Find chunks containing many plausible 16.16 X/Z pairs.

    This is a blunt instrument: it looks at aligned BE/LE i32 pairs and reports
    chunks whose pair cloud spans more than min_span cells in either axis.
    """
    hits = []
    skip_tags = {b"ZONE", b"ZMAP", b"XBMP", b"BIN ", b"ANM ", b"HEAD", b"JUNC"}
    for off, tag, size, body, parent, data in walk_iff(raw):
        if tag in skip_tags or tag == b"FORM" or size < 16:
            continue
        for endian_name, rd in (("BE", sbe32), ("LE", sle32)):
            pairs = []
            for o in range(0, min(size - 7, 0x20000), 2):
                try:
                    a = rd(data, o) / 65536.0
                    c = rd(data, o + 4) / 65536.0
                except struct.error:
                    continue
                if -4096.0 <= a <= 4096.0 and -4096.0 <= c <= 4096.0:
                    pairs.append((a, c, o))
            if len(pairs) >= 8:
                xs = [p[0] for p in pairs]
                zs = [p[1] for p in pairs]
                span = max(max(xs) - min(xs), max(zs) - min(zs))
                if span >= min_span:
                    hits.append((tag.decode("ascii", "replace"), parent.decode("ascii", "replace"),
                                 off, size, endian_name, len(pairs), extent(xs), extent(zs)))
    return hits


def audit_file(path: str, out):
    with open(path, "rb") as f:
        raw = f.read()
    name = os.path.basename(path)
    print(f"\n=== {name} ===", file=out)
    print(f"size=0x{len(raw):x}", file=out)

    counts = Counter(tag for _, tag, _, _, _, _ in walk_iff(raw))
    print("chunks: " + " ".join(f"{k.decode('ascii','replace')}={v}" for k, v in sorted(counts.items())), file=out)

    zmap_chunks = collect(raw, b"ZMAP")
    zone_chunks = collect(raw, b"ZONE")
    if zmap_chunks:
        cells = decode_zmap(zmap_chunks[0][4])
        xs = [c[1] for c in cells]
        zs = [c[0] for c in cells]
        print(f"ZMAP zones={len(zone_chunks)} populated={len(cells)} "
              f"chunk_x={extent(xs)} chunk_z={extent(zs)} "
              f"cell_x={fmt_ext((min(xs)*64, (max(xs)+1)*64) if xs else None)} "
              f"cell_z={fmt_ext((min(zs)*64, (max(zs)+1)*64) if zs else None)}",
              file=out)

    heads = decode_heads(raw)
    if heads:
        xs = [h["x"] for h in heads]
        ys = [h["y"] for h in heads]
        zs = [h["z"] for h in heads]
        names = Counter((h["name"] or "?") for h in heads)
        types = Counter(h["type"] for h in heads)
        banks = Counter(h["bank"] for h in heads)
        slots = Counter(h["slot"] for h in heads)
        print(f"HEAD obj={len(heads)} X={fmt_ext(extent(xs))} Y={fmt_ext(extent(ys))} Z={fmt_ext(extent(zs))}",
              file=out)
        print("HEAD types: " + ", ".join(f"{k}:{v}" for k, v in types.most_common(8)), file=out)
        print("HEAD banks top: " + ", ".join(f"{k}:{v}" for k, v in banks.most_common(8)), file=out)
        print("HEAD slots top: " + ", ".join(f"{k}:{v}" for k, v in slots.most_common(8)), file=out)
        print("HEAD names top: " + ", ".join(f"{k}:{v}" for k, v in names.most_common(8)), file=out)

    junc = decode_junc(raw)
    if junc:
        xs = [p[0] for p in junc]
        zs = [p[1] for p in junc]
        print(f"JUNC points={len(junc)} X={fmt_ext(extent(xs))} Z={fmt_ext(extent(zs))}", file=out)

    for i, (_body, size, data) in enumerate(xobf_bins(raw)):
        ex = bin_extents(data)
        if not ex:
            continue
        print(f"XOBF BIN[{i}] size=0x{size:x} bones={ex['nb']} verts={ex['verts']} tris={ex['tris']} "
              f"raw_scaled X={fmt_ext(ex['x'])} Y={fmt_ext(ex['y'])} Z={fmt_ext(ex['z'])} bad={ex['bad']}",
              file=out)

    hits = plausible_pairs(raw)
    if hits:
        print("coordinate-like unknown chunks:", file=out)
        for tag, parent, off, size, endian, n, xext, zext in hits[:12]:
            print(f"  {tag}/{parent} @0x{off:x} size=0x{size:x} {endian} pairs={n} "
                  f"X={fmt_ext(xext)} Z={fmt_ext(zext)}", file=out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("paths", nargs="*", default=[os.path.join("input", "TERRAIN", "*.EXP")])
    ap.add_argument("--out", default=None)
    args = ap.parse_args()

    import glob
    paths = []
    for p in args.paths:
        paths.extend(glob.glob(p))
    paths = sorted(set(paths))
    if args.out:
        os.makedirs(os.path.dirname(args.out), exist_ok=True)
        with open(args.out, "w", encoding="utf-8") as f:
            for path in paths:
                audit_file(path, f)
    else:
        for path in paths:
            audit_file(path, None)


if __name__ == "__main__":
    main()
