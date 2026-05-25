"""xobf_collision_audit.py -- decode XOBF slot and obstacle leaf records.

This is a source-driven terrain RE tool.  It follows the loader/runtime path:

  HEAD -> bank slot -> FUN_8001ac44/FUN_8001aaa8
       -> bank header +0x0c obstacle table -> FUN_8001ef74 records

No SDL/GL is used; this is safe for headless smoke and format work.
"""
from __future__ import annotations

import argparse
import glob
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


def sle16(b: bytes, o: int) -> int:
    return struct.unpack_from("<h", b, o)[0]


def sbe16(b: bytes, o: int) -> int:
    return struct.unpack_from(">h", b, o)[0]


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


def rot_yxz(ry: int, rx: int, rz: int):
    ay = ry * (2.0 * math.pi / 4096.0)
    ax = rx * (2.0 * math.pi / 4096.0)
    az = rz * (2.0 * math.pi / 4096.0)
    cy, sy = math.cos(ay), math.sin(ay)
    cx, sx = math.cos(ax), math.sin(ax)
    cz, sz = math.cos(az), math.sin(az)
    ry_m = ((cy, 0.0, sy), (0.0, 1.0, 0.0), (-sy, 0.0, cy))
    rx_m = ((1.0, 0.0, 0.0), (0.0, cx, -sx), (0.0, sx, cx))
    rz_m = ((cz, -sz, 0.0), (sz, cz, 0.0), (0.0, 0.0, 1.0))
    t = mat_mul(ry_m, rx_m)
    return mat_mul(t, rz_m)


def mat_mul(a, b):
    return tuple(tuple(sum(a[r][k] * b[k][c] for k in range(3)) for c in range(3)) for r in range(3))


def xf_point(xf, x: float, y: float, z: float):
    m, tx, ty, tz = xf
    return (
        tx + m[0][0] * x + m[0][1] * y + m[0][2] * z,
        ty + m[1][0] * x + m[1][1] * y + m[1][2] * z,
        tz + m[2][0] * x + m[2][1] * y + m[2][2] * z,
    )


def xf_mul(parent, x: float, y: float, z: float, ry: int, rx: int, rz: int):
    pm, _, _, _ = parent
    rm = rot_yxz(ry, rx, rz)
    m = mat_mul(pm, rm)
    tx, ty, tz = xf_point(parent, x, y, z)
    return (m, tx, ty, tz)


IDENTITY_XF = (((1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0)), 0.0, 0.0, 0.0)


def decode_heads(raw: bytes):
    heads = []
    for off, tag, size, body, parent, data in walk_iff(raw):
        if tag != b"HEAD" or parent != b"OBJ " or size < 34:
            continue
        heads.append({
            "off": off,
            "category": data[0],
            "type": data[1],
            "flags": be32(data, 4),
            # LOAD.DLL 801006f0 stores HEAD +0x08 into object posX and
            # HEAD +0x10 into object posZ.
            "x": sbe32(data, 8) / 65536.0,
            "y": (sbe32(data, 12) - 0x100000) / 131072.0,
            "z": sbe32(data, 16) / 65536.0,
            "ry": sbe16(data, 20),
            "rx": sbe16(data, 22),
            "rz": sbe16(data, 24),
            "bank": sbe16(data, 26),
            "slot": sbe16(data, 28),
            "name": data[34:size].split(b"\0", 1)[0].decode("ascii", "replace"),
        })
    return heads


def decode_banks(raw: bytes):
    banks = []
    for _off, tag, size, _body, parent, data in walk_iff(raw):
        if tag != b"BIN " or parent != b"XOBF" or size < 0x1c:
            continue
        bank = {
            "data": data,
            "size": size,
            "group_count": le32(data, 0x00),
            "group_table": le32(data, 0x04),
            "obstacle_count_minus1": le32(data, 0x08),
            "obstacle_table": le32(data, 0x0c),
            "secondary_table": le32(data, 0x10),
            "obstacle_end": le32(data, 0x14),
            "slot_count": le32(data, 0x18),
        }
        banks.append(bank)
    return banks


def parse_obstacle_stream(bank, index: int):
    data = bank["data"]
    table = bank["obstacle_table"]
    count = bank["obstacle_count_minus1"] + 1
    if index < 0 or index >= count or table + index * 4 + 4 > len(data):
        return None, ["bad-index"]
    rel = le32(data, table + index * 4)
    next_rel = le32(data, table + (index + 1) * 4) if index + 1 < count else bank["obstacle_end"] - table
    start = table + rel
    end = table + next_rel
    if not (table <= start <= end <= len(data)):
        return None, ["bad-range"]

    records = []
    errors = []
    p = start
    while p + 2 <= end:
        kind = sle16(data, p)
        if kind == 0:
            records.append({"kind": 0, "off": p - table})
            break
        if kind == 1:
            if p + 0x1c > end:
                errors.append("truncated-aabb")
                break
            records.append({
                "kind": 1,
                "off": p - table,
                "min_x": sle32(data, p + 4) / 65536.0,
                "y": sle32(data, p + 8) / 65536.0,
                "min_z": sle32(data, p + 12) / 65536.0,
                "max_x": sle32(data, p + 16) / 65536.0,
                "max_y": sle32(data, p + 20) / 65536.0,
                "max_z": sle32(data, p + 24) / 65536.0,
            })
            p += 0x1c
        elif kind == 2:
            if p + 4 > end:
                errors.append("truncated-plane-header")
                break
            nplanes = struct.unpack_from("<H", data, p + 2)[0]
            if p + 4 + nplanes * 12 > end:
                errors.append("truncated-planes")
                break
            planes = []
            q = p + 4
            for _ in range(nplanes):
                planes.append((sle16(data, q), sle16(data, q + 2), sle16(data, q + 4), sle32(data, q + 8)))
                q += 12
            records.append({"kind": 2, "off": p - table, "planes": planes})
            p = q
        else:
            errors.append(f"unknown-kind-{kind}@0x{p - table:x}")
            break
    return records, errors


def traverse_slots(bank, slot: int, parent_xf, is_root: bool, flags: int, depth: int, seen: set[int], out: list[dict]):
    data = bank["data"]
    slot_count = bank["slot_count"]
    if slot < 0 or slot >= slot_count or depth > 512 or slot in seen:
        return
    seen.add(slot)
    e = 0x1c + slot * 0x1c
    if e + 0x1c > len(data):
        return
    key0 = sle16(data, e)
    key1 = sle16(data, e + 2)
    lx = sle32(data, e + 4) / 65536.0
    ly = -sle32(data, e + 8) / 131072.0
    lz = sle32(data, e + 12) / 65536.0
    ry = sle16(data, e + 16)
    rx = sle16(data, e + 18)
    rz = sle16(data, e + 20)
    local_flags = sle16(data, e + 22)
    # FUN_8001ac44 stores slot +0x18 in object +0x34 (next sibling)
    # and slot +0x1a in object +0x38 (first child).
    next_sibling = sle16(data, e + 24)
    first_child = sle16(data, e + 26)

    if key0 < 0 and (key0 != -1 or ((flags & 4) != 0)):
        if (flags & 1) != 0 and next_sibling != -1:
            traverse_slots(bank, next_sibling, parent_xf, False, flags, depth + 1, seen, out)
        return

    self_xf = parent_xf if is_root else xf_mul(parent_xf, lx, ly, lz, ry, rx, rz)
    out.append({
        "slot": slot,
        "key0": key0,
        "key1": key1,
        "flags": local_flags,
        "next_sibling": next_sibling,
        "first_child": first_child,
        "xf": self_xf,
    })
    if (flags & 1) != 0 and next_sibling != -1:
        traverse_slots(bank, next_sibling, parent_xf, False, flags, depth + 1, seen, out)
    if (flags & 2) == 0 and first_child != -1:
        traverse_slots(bank, first_child, self_xf, False, flags | 1, depth + 1, seen, out)


def audit_file(path: str, out):
    raw = open(path, "rb").read()
    name = os.path.basename(path)
    banks = decode_banks(raw)
    heads = decode_heads(raw)
    print(f"\n=== {name} ===", file=out)
    print(f"banks={len(banks)} heads={len(heads)}", file=out)

    all_flat = []
    kind_counts = Counter()
    errors = Counter()
    slot_key_counts = Counter()
    used_obstacles = set()
    placed = 0
    roots = 0

    for bi, bank in enumerate(banks):
        derived_slots = (bank["group_table"] - 0x1c) // 0x1c if bank["group_table"] >= 0x1c else 0
        print(
            f"  BIN[{bi}] groups={bank['group_count']} slots={bank['slot_count']} "
            f"derived_slots={derived_slots} obstacle_count={bank['obstacle_count_minus1'] + 1} "
            f"group_table=0x{bank['group_table']:x} obstacle_table=0x{bank['obstacle_table']:x} "
            f"secondary=0x{bank['secondary_table']:x} obstacle_end=0x{bank['obstacle_end']:x}",
            file=out,
        )
        for si in range(bank["slot_count"]):
            e = 0x1c + si * 0x1c
            if e + 4 > len(bank["data"]):
                break
            key0 = sle16(bank["data"], e)
            key1 = sle16(bank["data"], e + 2)
            slot_key_counts["key0>=0" if key0 >= 0 else "key0<0"] += 1
            slot_key_counts["key1>=0" if key1 >= 0 else "key1<0"] += 1

    for head in heads:
        if head["type"] in (1, 5, 6) or head["type"] > 6:
            continue
        if head["bank"] < 0 or head["bank"] >= len(banks) or head["slot"] < 0:
            continue
        bank = banks[head["bank"]]
        root_m = rot_yxz(head["ry"], head["rx"], head["rz"])
        root_xf = (root_m, head["x"], -head["y"], head["z"])
        slots = []
        traverse_slots(bank, head["slot"], root_xf, True, (head["flags"] & 4) << 1, 0, set(), slots)
        roots += 1
        for slot in slots:
            placed += 1
            key1 = slot["key1"]
            if key1 < 0:
                continue
            used_obstacles.add((head["bank"], key1))
            records, rec_errors = parse_obstacle_stream(bank, key1)
            for err in rec_errors:
                errors[err] += 1
            if not records:
                continue
            for rec in records:
                kind_counts[rec["kind"]] += 1
                if rec["kind"] != 1:
                    continue
                corners = [
                    (rec["min_x"], rec["y"], rec["min_z"]),
                    (rec["min_x"], rec["y"], rec["max_z"]),
                    (rec["max_x"], rec["y"], rec["min_z"]),
                    (rec["max_x"], rec["y"], rec["max_z"]),
                    (rec["min_x"], rec["max_y"], rec["min_z"]),
                    (rec["min_x"], rec["max_y"], rec["max_z"]),
                    (rec["max_x"], rec["max_y"], rec["min_z"]),
                    (rec["max_x"], rec["max_y"], rec["max_z"]),
                ]
                wc = [xf_point(slot["xf"], *c) for c in corners]
                all_flat.append((head, slot, rec, wc))

    print(f"  slot key counts: {dict(slot_key_counts)}", file=out)
    print(f"  original traversal roots={roots} placed_slots={placed} used_obstacle_streams={len(used_obstacles)}", file=out)
    print(f"  obstacle record counts: {dict(kind_counts)} errors={dict(errors)}", file=out)
    if all_flat:
        xs = [p[0] for _, _, _, wc in all_flat for p in wc]
        ys = [p[1] for _, _, _, wc in all_flat for p in wc]
        zs = [p[2] for _, _, _, wc in all_flat for p in wc]
        print(
            f"  flat/AABB world extents X=[{min(xs):.1f}..{max(xs):.1f}] "
            f"Y=[{min(ys):.1f}..{max(ys):.1f}] Z=[{min(zs):.1f}..{max(zs):.1f}]",
            file=out,
        )
        samples = sorted(all_flat, key=lambda item: (item[0]["bank"], item[1]["key1"], item[2]["off"]))[:12]
        for head, slot, rec, wc in samples:
            xs = [p[0] for p in wc]
            ys = [p[1] for p in wc]
            zs = [p[2] for p in wc]
            print(
                f"    stream={slot['key1']:02d} slot={slot['slot']:03d} head={head['name'][:18]!r} "
                f"kind1 off=0x{rec['off']:x} X=[{min(xs):.1f}..{max(xs):.1f}] "
                f"Y=[{min(ys):.1f}..{max(ys):.1f}] Z=[{min(zs):.1f}..{max(zs):.1f}]",
                file=out,
            )


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("paths", nargs="+")
    ap.add_argument("--out")
    args = ap.parse_args()
    paths = []
    for pat in args.paths:
        hit = glob.glob(pat)
        paths.extend(hit if hit else [pat])
    with (open(args.out, "w", encoding="utf-8") if args.out else os.fdopen(os.dup(1), "w", encoding="utf-8")) as out:
        for path in paths:
            audit_file(path, out)


if __name__ == "__main__":
    main()
