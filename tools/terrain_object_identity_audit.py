#!/usr/bin/env python3
"""Audit terrain HEAD object identity and resolved XOBF geometry.

This is a headless/source-only check for the common failure mode where the
selected EXP is correct but the renderer resolves HEAD bank/slot records
through the wrong XOBF template. It reports per-level object names, bank/slot
usage, and object dimensions after walking the same slot tree as FUN_8001ac44.
"""
from __future__ import annotations

from collections import Counter, defaultdict
from pathlib import Path
import math
import struct


ROOT = Path(__file__).resolve().parents[1]
TERRAIN = ROOT / "input" / "TERRAIN"


def be32(b: bytes, o: int) -> int:
    return struct.unpack_from(">I", b, o)[0]


def sbe32(b: bytes, o: int) -> int:
    return struct.unpack_from(">i", b, o)[0]


def sbe16(b: bytes, o: int) -> int:
    return struct.unpack_from(">h", b, o)[0]


def ule32(b: bytes, o: int) -> int:
    return struct.unpack_from("<I", b, o)[0]


def sle32(b: bytes, o: int) -> int:
    return struct.unpack_from("<i", b, o)[0]


def ule16(b: bytes, o: int) -> int:
    return struct.unpack_from("<H", b, o)[0]


def sle16(b: bytes, o: int) -> int:
    return struct.unpack_from("<h", b, o)[0]


def walk_iff(data: bytes, start: int = 0, end: int | None = None,
             parent: bytes = b""):
    if end is None:
        end = len(data)
    p = start
    while p + 8 <= end:
        tag = data[p:p + 4]
        size = be32(data, p + 4)
        body = p + 8
        if body + size > end:
            break
        if tag == b"FORM" and size >= 4:
            ftype = data[body:body + 4]
            payload = body + 4
            yield p, tag, size, payload, ftype, parent
            yield from walk_iff(data, payload, body + size, ftype)
        else:
            yield p, tag, size, body, tag, parent
        p = body + size + (size & 1)


def collect_bins(path: Path) -> list[bytes]:
    data = path.read_bytes()
    return [data[body:body + size]
            for _off, tag, size, body, _ctag, parent in walk_iff(data)
            if tag == b"BIN " and parent == b"XOBF"]


def collect_heads(path: Path):
    data = path.read_bytes()
    heads = []
    for off, tag, size, body, _ctag, parent in walk_iff(data):
        if tag != b"HEAD" or parent != b"OBJ " or size < 34:
            continue
        h = data[body:body + size]
        name = h[34:].split(b"\0", 1)[0].decode("ascii", "replace")
        heads.append({
            "off": off,
            "type": h[1],
            "flags": be32(h, 4),
            "raw_x": sbe32(h, 8),
            "raw_y": sbe32(h, 12) - 0x100000,
            "raw_z": sbe32(h, 16),
            "x": sbe32(h, 8) / 65536.0,
            "y": (sbe32(h, 12) - 0x100000) / 131072.0,
            "z": sbe32(h, 16) / 65536.0,
            "ry": sbe16(h, 20),
            "rx": sbe16(h, 22),
            "rz": sbe16(h, 24),
            "bank": sbe16(h, 26),
            "slot": sbe16(h, 28),
            "strength": sbe16(h, 32),
            "name": name,
        })
    return heads


def bank_meta(B: bytes):
    if len(B) < 0x1c:
        return None
    group_count = ule32(B, 0)
    group_table = ule32(B, 4)
    slot_count = ule32(B, 0x18)
    if group_table >= len(B) or 0x1c + slot_count * 0x1c > group_table:
        return None
    return {
        "groups": group_count,
        "group_table": group_table,
        "slots": slot_count,
        "obstacles": ule32(B, 8) if len(B) >= 12 else 0,
    }


def group_desc(B: bytes, group: int):
    meta = bank_meta(B)
    if meta is None:
        return None
    if group < 0 or group >= meta["groups"]:
        return None
    table = meta["group_table"]
    if table + group * 4 + 4 > len(B):
        return None
    bd = table + ule32(B, table + group * 4)
    if bd + 0x1a > len(B):
        return None
    vc = ule32(B, bd)
    vr = ule32(B, bd + 4)
    pc = ule16(B, bd + 0x10)
    pr = ule32(B, bd + 0x14)
    scale = B[bd + 0x18]
    vo = bd + vr
    if vc > 100000 or vo + vc * 8 > len(B):
        return None
    return {"vc": vc, "pc": pc, "scale": scale, "vo": vo, "po": bd + pr}


def group_vertices(B: bytes, desc):
    for i in range(desc["vc"]):
        o = desc["vo"] + i * 8
        yield sle16(B, o), sle16(B, o + 2), sle16(B, o + 4)


def rot_yxz_q12(ry: int, rx: int, rz: int):
    tau = 2.0 * math.pi / 4096.0
    ay, ax, az = ry * tau, rx * tau, rz * tau
    cy, sy = math.cos(ay), math.sin(ay)
    cx, sx = math.cos(ax), math.sin(ax)
    cz, sz = math.cos(az), math.sin(az)
    Ry = ((cy, 0.0, sy), (0.0, 1.0, 0.0), (-sy, 0.0, cy))
    Rx = ((1.0, 0.0, 0.0), (0.0, cx, -sx), (0.0, sx, cx))
    Rz = ((cz, -sz, 0.0), (sz, cz, 0.0), (0.0, 0.0, 1.0))

    def mm(a, b):
        return tuple(tuple(sum(a[r][k] * b[k][c] for k in range(3))
                           for c in range(3)) for r in range(3))

    return [[int(round(v * 4096.0)) for v in row] for row in mm(mm(Ry, Rx), Rz)]


def mat_from_raw(x: int, y: int, z: int, ry: int, rx: int, rz: int):
    return {"m": rot_yxz_q12(ry, rx, rz), "t": [x, y, z]}


def mat_compose(parent, x: int, y: int, z: int, ry: int, rx: int, rz: int):
    local = mat_from_raw(x, y, z, ry, rx, rz)
    out_m = [[0, 0, 0] for _ in range(3)]
    for r in range(3):
        for c in range(3):
            out_m[r][c] = int(round(sum(parent["m"][r][k] * local["m"][k][c]
                                        for k in range(3)) / 4096.0))
    out_t = []
    for r in range(3):
        out_t.append(parent["t"][r] +
                     int(round(sum(parent["m"][r][k] * local["t"][k]
                                   for k in range(3)) / 4096.0)))
    return {"m": out_m, "t": out_t}


def group_vertex_world(mat, scale: int, vx: int, vy: int, vz: int):
    shift = 16 - scale
    tx, ty, tz = (mat["t"][i] >> shift for i in range(3))
    rx = (mat["m"][0][0] * vx + mat["m"][0][1] * vy + mat["m"][0][2] * vz) >> 12
    ry = (mat["m"][1][0] * vx + mat["m"][1][1] * vy + mat["m"][1][2] * vz) >> 12
    rz = (mat["m"][2][0] * vx + mat["m"][2][1] * vy + mat["m"][2][2] * vz) >> 12
    unshift = float(1 << shift)
    return ((tx + rx) * unshift / 65536.0,
            -(ty + ry) * unshift / 131072.0,
            (tz + rz) * unshift / 65536.0)


def resolved_object(banks: list[bytes], head):
    if head["bank"] < 0 or head["bank"] >= len(banks):
        return None
    B = banks[head["bank"]]
    meta = bank_meta(B)
    if meta is None or head["slot"] < 0 or head["slot"] >= meta["slots"]:
        return None
    root = mat_from_raw(head["raw_x"], head["raw_y"], head["raw_z"],
                        head["ry"], head["rx"], head["rz"])
    pts = []
    groups = Counter()
    slots = []
    seen = set()

    def emit(group: int, mat):
        desc = group_desc(B, group)
        if desc is None or desc["scale"] > 16:
            return
        groups[(group, desc["scale"])] += 1
        for x, y, z in group_vertices(B, desc):
            pts.append(group_vertex_world(mat, desc["scale"], x, y, z))

    def walk(slot: int, parent, is_root: bool, flags: int, depth: int):
        if slot < 0 or slot >= meta["slots"] or depth > 256 or slot in seen:
            return
        seen.add(slot)
        slots.append(slot)
        e = 0x1c + slot * 0x1c
        key = sle16(B, e)
        lx, ly, lz = sle32(B, e + 4), sle32(B, e + 8), sle32(B, e + 12)
        ry, rx, rz = sle16(B, e + 16), sle16(B, e + 18), sle16(B, e + 20)
        next_sibling = sle16(B, e + 24)
        first_child = sle16(B, e + 26)
        if key < 0 and (key != -1 or (flags & 4) != 0):
            if (key & 0xf000) == 0xc000:
                emit(key & 0x7ff, parent)
            if (flags & 1) and next_sibling != -1:
                walk(next_sibling, parent, False, flags, depth + 1)
            return
        self_mat = parent if is_root else mat_compose(parent, lx, ly, lz, ry, rx, rz)
        if key >= 0:
            emit(key & 0x7ff, self_mat)
        if (flags & 1) and next_sibling != -1:
            walk(next_sibling, parent, False, flags, depth + 1)
        if not (flags & 2) and first_child != -1:
            walk(first_child, self_mat, False, flags | 1, depth + 1)

    walk(head["slot"], root, True, (head["flags"] & 4) << 1, 0)
    if not pts:
        return {"slots": slots, "groups": groups, "verts": 0}
    xs, ys, zs = zip(*pts)
    return {
        "slots": slots,
        "groups": groups,
        "verts": len(pts),
        "dims": (max(xs) - min(xs), max(ys) - min(ys), max(zs) - min(zs)),
        "bounds": (min(xs), max(xs), min(ys), max(ys), min(zs), max(zs)),
    }


def summarize_level(path: Path, out):
    banks = collect_bins(path)
    heads = collect_heads(path)
    print(f"LEVEL {path.name} bytes={path.stat().st_size} banks={len(banks)} heads={len(heads)}", file=out)
    for i, B in enumerate(banks):
        meta = bank_meta(B)
        print(f"  bank[{i}] {meta}", file=out)
    print("  names " + ", ".join(f"{n}:{c}" for n, c in Counter(h["name"] for h in heads).most_common(18)), file=out)
    print("  type_bank " + ", ".join(f"{k}:{v}" for k, v in sorted(Counter((h["type"], h["bank"]) for h in heads).items())), file=out)

    by_name = defaultdict(list)
    for h in heads:
        if h["type"] in (1, 6) or h["type"] > 6:
            continue
        r = resolved_object(banks, h)
        if r and r.get("verts", 0):
            by_name[h["name"]].append((h, r))

    print("  resolved samples", file=out)
    for name, items in sorted(by_name.items(), key=lambda kv: (-len(kv[1]), kv[0]))[:24]:
        dims = [it[1]["dims"] for it in items]
        avg = tuple(sum(d[i] for d in dims) / len(dims) for i in range(3))
        first = items[0][0]
        groups = items[0][1]["groups"].most_common(5)
        print(f"    {name:<22} n={len(items):3d} type={first['type']} bank={first['bank']} slot={first['slot']} "
              f"avg={avg[0]:7.2f} x {avg[1]:7.2f} x {avg[2]:7.2f} groups={groups}",
              file=out)
    print("", file=out)


def main() -> int:
    out_path = ROOT / "analysis" / "terrain_object_identity_audit.txt"
    out_path.parent.mkdir(exist_ok=True)
    levels = [
        TERRAIN / "SKIRESRT.EXP",
        TERRAIN / "OILFIELD.EXP",
        TERRAIN / "WILDWEST.EXP",
        TERRAIN / "HOOVRDAM.EXP",
    ]
    with out_path.open("w", encoding="ascii") as out:
        for path in levels:
            summarize_level(path, out)
    print(out_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
