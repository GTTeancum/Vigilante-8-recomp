#!/usr/bin/env python3
"""Headless scale audit for vehicle BINs versus placed terrain objects.

This is intentionally data-only: no renderer, no screenshots.  It compares
vehicle mesh extents from SHELL/VEHICLES.EXP against placed OilField XOBF
objects, especially OilPump_1, using the source-confirmed group descriptor
scale byte at +0x18.
"""
from __future__ import annotations

from collections import Counter, defaultdict
from pathlib import Path
import math
import struct
import sys


ROOT = Path(__file__).resolve().parents[1]
VEHICLES = ROOT / "input" / "SHELL" / "VEHICLES.EXP"
OILFIELD = ROOT / "input" / "TERRAIN" / "OILFIELD.EXP"

PKT_SIZE = [12, 28, 20, 28, 12, 20, 12, 20, 16, 24, 12, 24, 20, 20, 0, 20]
IS_QUAD = [0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]


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
            payload_start = body + 4
            yield p, tag, size, payload_start, ftype, parent
            yield from walk_iff(data, payload_start, body + size, ftype)
        else:
            yield p, tag, size, body, tag, parent
        p = body + size + (size & 1)


def collect_bins(path: Path, parent: bytes | None = None) -> list[bytes]:
    data = path.read_bytes()
    out = []
    for _off, tag, size, body, _ctag, cparent in walk_iff(data):
        if tag == b"BIN " and (parent is None or cparent == parent):
            out.append(data[body:body + size])
    return out


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
            "name": name,
        })
    return heads


def ext(vals):
    return (min(vals), max(vals)) if vals else (0.0, 0.0)


def span(vals) -> float:
    lo, hi = ext(vals)
    return hi - lo


def fmt_dims(xs, ys, zs) -> str:
    return f"{span(xs):6.2f} x {span(ys):6.2f} x {span(zs):6.2f}"


def group_desc(B: bytes, group: int, terrain: bool):
    if len(B) < 8:
        return None
    count = ule32(B, 0)
    table = ule32(B, 4)
    if group < 0 or group >= count or table + group * 4 + 4 > len(B):
        return None
    bd = table + ule32(B, table + group * 4)
    if bd + 0x1a > len(B):
        return None
    vc = ule32(B, bd + 0)
    vr = ule32(B, bd + 4)
    pc = ule16(B, bd + 0x10)
    pr = ule32(B, bd + 0x14)
    scale = B[bd + 0x18]
    vo = bd + vr
    po = bd + pr
    if vc > 100000 or vo + vc * 8 > len(B) or po >= len(B):
        return None
    return {
        "bd": bd, "vc": vc, "pc": pc, "scale": scale,
        "vo": vo, "po": po, "count": count, "table": table,
    }


def iter_groups(B: bytes, terrain: bool):
    if len(B) < 8:
        return
    count = ule32(B, 0)
    for group in range(count):
        d = group_desc(B, group, terrain)
        if d is not None:
            yield group, d


def group_raw_vertices(B: bytes, desc):
    vo = desc["vo"]
    for vi in range(desc["vc"]):
        oe = vo + vi * 8
        yield sle16(B, oe), sle16(B, oe + 2), sle16(B, oe + 4)


def vehicle_extents(B: bytes, use_desc_scale: bool):
    xs, ys, zs = [], [], []
    scales = Counter()
    tris = 0
    for _group, desc in iter_groups(B, terrain=False):
        scale = desc["scale"]
        scales[scale] += 1
        denom = float(1 << scale) if scale <= 15 else 160.0
        for x, y, z in group_raw_vertices(B, desc):
            xs.append(x / denom)
            ys.append(-y / denom)
            zs.append(z / denom)
        po = desc["po"]
        for _ in range(desc["pc"]):
            if po + 4 > len(B):
                break
            nib = B[po + 3] & 0xf
            sz = PKT_SIZE[nib]
            if sz == 0 or po + sz > len(B):
                po += 4
                continue
            tris += 2 if IS_QUAD[nib] else 1
            po += sz
    return xs, ys, zs, scales, tris


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

    R = mm(mm(Ry, Rx), Rz)
    return [[int(round(v * 4096.0)) for v in row] for row in R]


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


def sra(v: int, shift: int) -> int:
    if shift <= 0:
        return v
    return v >> shift


def group_vertex_world(mat, scale: int, vx: int, vy: int, vz: int):
    shift = 16 - scale
    tx, ty, tz = (sra(mat["t"][i], shift) for i in range(3))
    rx = (mat["m"][0][0] * vx + mat["m"][0][1] * vy + mat["m"][0][2] * vz) >> 12
    ry = (mat["m"][1][0] * vx + mat["m"][1][1] * vy + mat["m"][1][2] * vz) >> 12
    rz = (mat["m"][2][0] * vx + mat["m"][2][1] * vy + mat["m"][2][2] * vz) >> 12
    unshift = float(1 << shift)
    return ((tx + rx) * unshift / 65536.0,
            -(ty + ry) * unshift / 131072.0,
            (tz + rz) * unshift / 65536.0)


def terrain_bank_meta(B: bytes):
    if len(B) < 0x1c:
        return None
    group_table = ule32(B, 4)
    slot_count = ule32(B, 0x18)
    if group_table >= len(B) or 0x1c + slot_count * 0x1c > group_table:
        return None
    return {"groups": ule32(B, 0), "group_table": group_table, "slots": slot_count}


def terrain_object_points(banks, head):
    if head["bank"] < 0 or head["bank"] >= len(banks):
        return []
    B = banks[head["bank"]]
    meta = terrain_bank_meta(B)
    if meta is None or head["slot"] < 0 or head["slot"] >= meta["slots"]:
        return []
    root = mat_from_raw(head["raw_x"], head["raw_y"], head["raw_z"],
                        head["ry"], head["rx"], head["rz"])
    pts = []
    seen = set()

    def walk(slot: int, parent_mat, is_root: bool, flags: int, depth: int):
        if slot < 0 or slot >= meta["slots"] or depth > 256 or slot in seen:
            return
        seen.add(slot)
        e = 0x1c + slot * 0x1c
        key = sle16(B, e + 0)
        lx, ly, lz = sle32(B, e + 4), sle32(B, e + 8), sle32(B, e + 12)
        ry, rx, rz = sle16(B, e + 16), sle16(B, e + 18), sle16(B, e + 20)
        next_sibling = sle16(B, e + 24)
        first_child = sle16(B, e + 26)
        if key < 0 and (key != -1 or (flags & 4) != 0):
            if (key & 0xf000) == 0xc000:
                emit(key & 0x7ff, parent_mat)
            if (flags & 1) != 0 and next_sibling != -1:
                walk(next_sibling, parent_mat, False, flags, depth + 1)
            return
        self_mat = parent_mat if is_root else mat_compose(parent_mat, lx, ly, lz,
                                                          ry, rx, rz)
        if key >= 0:
            emit(key & 0x7ff, self_mat)
        if (flags & 1) != 0 and next_sibling != -1:
            walk(next_sibling, parent_mat, False, flags, depth + 1)
        if (flags & 2) == 0 and first_child != -1:
            walk(first_child, self_mat, False, flags | 1, depth + 1)

    def emit(group: int, mat):
        desc = group_desc(B, group, terrain=True)
        if desc is None or desc["scale"] > 16:
            return
        for x, y, z in group_raw_vertices(B, desc):
            pts.append(group_vertex_world(mat, desc["scale"], x, y, z))

    walk(head["slot"], root, True, (head["flags"] & 4) << 1, 0)
    return pts


def print_vehicle_report(out):
    bins = collect_bins(VEHICLES)
    print(f"VEHICLE_BINS {len(bins)} file={VEHICLES}", file=out)
    all_scales = Counter()
    desc_dims = []
    current_dims = []
    for i, B in enumerate(bins[:14]):
        cxs, cys, czs, cscale, ctris = vehicle_extents(B, use_desc_scale=False)
        dxs, dys, dzs, dscale, dtris = vehicle_extents(B, use_desc_scale=True)
        all_scales.update(dscale)
        current_dims.append((span(cxs), span(cys), span(czs)))
        desc_dims.append((span(dxs), span(dys), span(dzs)))
        print(f"veh[{i:02d}] tris={dtris:4d} group_scales={dict(sorted(dscale.items()))} "
              f"host_descriptor={fmt_dims(cxs, cys, czs)} "
              f"descriptor={fmt_dims(dxs, dys, dzs)}", file=out)
    print(f"vehicle_scale_histogram {dict(sorted(all_scales.items()))}", file=out)
    avg_cur = tuple(sum(v[j] for v in current_dims) / len(current_dims) for j in range(3))
    avg_desc = tuple(sum(v[j] for v in desc_dims) / len(desc_dims) for j in range(3))
    print(f"vehicle_avg_host_descriptor {avg_cur[0]:.2f} x {avg_cur[1]:.2f} x {avg_cur[2]:.2f}", file=out)
    print(f"vehicle_avg_descriptor {avg_desc[0]:.2f} x {avg_desc[1]:.2f} x {avg_desc[2]:.2f}", file=out)
    print("", file=out)
    return avg_cur, avg_desc


def print_oilfield_report(out, vehicle_avg_current):
    banks = collect_bins(OILFIELD, parent=b"XOBF")
    heads = collect_heads(OILFIELD)
    print(f"OILFIELD banks={len(banks)} heads={len(heads)} file={OILFIELD}", file=out)
    hx = [h["x"] for h in heads]
    hy = [h["y"] for h in heads]
    hz = [h["z"] for h in heads]
    print(f"head_extent X=[{min(hx):.2f}..{max(hx):.2f}] span={span(hx):.2f} "
          f"Y=[{min(hy):.2f}..{max(hy):.2f}] span={span(hy):.2f} "
          f"Z=[{min(hz):.2f}..{max(hz):.2f}] span={span(hz):.2f}", file=out)
    car_len = vehicle_avg_current[2]
    if car_len > 0:
        print(f"head_extent_in_avg_car_lengths X={span(hx) / car_len:.1f} "
              f"Z={span(hz) / car_len:.1f}", file=out)
    print("head_types " + " ".join(f"{k}:{v}" for k, v in sorted(Counter(h["type"] for h in heads).items())),
          file=out)
    names = Counter(h["name"] for h in heads)
    print("common_heads " + ", ".join(f"{k}:{v}" for k, v in names.most_common(15)), file=out)
    targets = [h for h in heads if h["name"].lower().startswith("oilpump")]
    if not targets:
        targets = [h for h in heads if "pump" in h["name"].lower()]
    print(f"oilpump_heads={len(targets)}", file=out)
    object_dims = defaultdict(list)
    for h in heads:
        if h["type"] in (1, 6) or h["type"] > 6:
            continue
        pts = terrain_object_points(banks, h)
        if not pts:
            continue
        xs, ys, zs = zip(*pts)
        object_dims[h["name"]].append((span(xs), span(ys), span(zs), len(pts)))
        if h in targets:
            print(f"OilPump head@0x{h['off']:x} type={h['type']} bank={h['bank']} slot={h['slot']} "
                  f"pos=({h['x']:.2f},{h['y']:.2f},{h['z']:.2f}) "
                  f"dims={fmt_dims(xs, ys, zs)} verts={len(pts)}", file=out)
    if targets:
        dims = [d for h in targets for d in object_dims.get(h["name"], [])]
        if dims:
            avg = tuple(sum(d[i] for d in dims) / len(dims) for i in range(3))
            print(f"oilpump_avg_dims {avg[0]:.2f} x {avg[1]:.2f} x {avg[2]:.2f}", file=out)
            print(f"oilpump_vs_avg_vehicle host_width={avg[0] / vehicle_avg_current[0]:.2f} "
                  f"host_height={avg[1] / vehicle_avg_current[1]:.2f} "
                  f"host_length={avg[2] / vehicle_avg_current[2]:.2f}", file=out)
    print("object_dim_samples", file=out)
    for name, dims in sorted(object_dims.items(), key=lambda kv: (-len(kv[1]), kv[0]))[:24]:
        avg = tuple(sum(d[i] for d in dims) / len(dims) for i in range(3))
        print(f"  {name:<24} n={len(dims):3d} avg={avg[0]:6.2f} x {avg[1]:6.2f} x {avg[2]:6.2f}",
              file=out)


def main() -> int:
    out_path = ROOT / "analysis" / "scale_audit.txt"
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with out_path.open("w", encoding="utf-8") as out:
        avg_cur, _avg_desc = print_vehicle_report(out)
        print_oilfield_report(out, avg_cur)
    print(out_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
