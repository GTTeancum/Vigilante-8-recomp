#!/usr/bin/env python3
"""Audit terrain visual packet ownership for Vigilante 8 EXP files.

This intentionally does not render.  It walks the terrain EXP containers,
decodes XOBF/BIN group packet streams with FUN_8001b49c's source-kind rules,
and correlates JUNC patch slots and RSEG/XRTP routes with the XOBF groups they
reference.  The output is meant to keep terrain visual work source-driven.
"""
from __future__ import annotations

from collections import Counter, defaultdict
from pathlib import Path
import argparse


SOURCE_STRIDE = [
    0x000c, 0x001c, 0x0014, 0x001c, 0x000c, 0x0014, 0x000c, 0x0014,
    0x0010, 0x0018, 0x000c, 0x0018, 0x0014, 0x0014, 0x0000, 0x0014,
]

CACHED_STRIDE = [
    0x0014, 0x0028, 0x001c, 0x0028, 0x0014, 0x001c, 0x001c, 0x0028,
    0x000c, 0x0014, 0x0020, 0x000c, 0x0020, 0x0020, 0x0000, 0x0020,
]

TEXTURE_SLOT_OFFSETS = {
    1: 0x12,
    5: 0x12,
    9: 0x16,
    12: 0x10,
    13: 0x12,
}


def be32(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 4], "big", signed=False)


def sbe32(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 4], "big", signed=True)


def sbe16(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 2], "big", signed=True)


def le16(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 2], "little", signed=False)


def sle16(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 2], "little", signed=True)


def le32(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 4], "little", signed=False)


def walk_iff(data: bytes, start: int = 0, end: int | None = None, parent: bytes = b""):
    if end is None:
        end = len(data)
    p = start
    while p + 8 <= end:
        tag = data[p:p + 4]
        size = be32(data, p + 4)
        body = p + 8
        if body + size > len(data):
            break
        if tag == b"FORM" and size >= 4:
            form_type = data[body:body + 4]
            payload_start = body + 4
            payload = data[payload_start:body + size]
            yield p, size, payload_start, form_type, parent, payload
            yield from walk_iff(data, payload_start, body + size, form_type)
        else:
            yield p, size, body, tag, parent, data[body:body + size]
        p += 8 + ((size + 1) & ~1)


def collect(data: bytes, tag: bytes, parent: bytes | None = None):
    return [
        (off, size, body, cparent, payload)
        for off, size, body, ctag, cparent, payload in walk_iff(data)
        if ctag == tag and (parent is None or cparent == parent)
    ]


def decode_xrtp(data: bytes):
    out = []
    for off, size, _body, _parent, payload in collect(data, b"XRTP"):
        if size < 12:
            continue
        tim = payload[12:]
        out.append({
            "off": off,
            "size": size,
            "width": sbe32(payload, 0),
            "step": sbe32(payload, 4),
            "tex_id": sbe16(payload, 8),
            "flags": sbe16(payload, 10),
            "payload": len(tim),
            "tim_magic": tim[:4].hex() if tim else "",
        })
    return out


def decode_rseg(data: bytes):
    out = []
    for off, size, _body, _parent, payload in collect(data, b"RSEG"):
        if size < 14:
            continue
        p = 0
        if size == 0x16:
            route_type = sbe16(payload, p)
            order = sbe16(payload, p + 2)
            flags = 0
            p += 4
        else:
            route_type = sbe16(payload, p)
            order = sbe16(payload, p + 2)
            flags = sbe16(payload, p + 4)
            p += 6
        if p + 4 > size:
            continue
        node_a = sbe16(payload, p)
        node_b = sbe16(payload, p + 2)
        p += 4
        ctrl = []
        while p + 4 <= size and len(ctrl) < 4:
            ctrl.append(sbe32(payload, p))
            p += 4
        out.append({
            "off": off,
            "size": size,
            "type": route_type,
            "order": order,
            "flags": flags,
            "node_a": node_a,
            "node_b": node_b,
            "ctrl": ctrl,
        })
    return out


def decode_junc(data: bytes):
    out = []
    for idx, (off, size, _body, _parent, payload) in enumerate(collect(data, b"JUNC")):
        if size < 10:
            continue
        p = 10
        flags = payload[8]
        rec = {
            "index": idx,
            "off": off,
            "size": size,
            "x": sbe32(payload, 0),
            "z": sbe32(payload, 4),
            "flags": flags,
            "edge_count": payload[9],
            "has_patch": False,
        }
        if flags & 2 and p + 4 <= size:
            rec["y"] = sbe32(payload, p)
            p += 4
        if p + 6 <= size:
            rec["has_patch"] = True
            rec["bank"] = sbe16(payload, p)
            rec["slot"] = sbe16(payload, p + 2)
            rec["rot"] = sbe16(payload, p + 4)
            rec["name"] = payload[p + 6:size].split(b"\0", 1)[0].decode("ascii", "replace")
        out.append(rec)
    return out


def decode_slot_group(bin_data: bytes, slot_index: int) -> int | None:
    if len(bin_data) < 0x1c:
        return None
    slot_count = le32(bin_data, 0x18) if len(bin_data) >= 0x1c else 0
    if slot_index < 0 or slot_index >= slot_count:
        return None
    o = 0x1c + slot_index * 0x1c
    if o + 2 > len(bin_data):
        return None
    return sle16(bin_data, o) & 0x07ff


def iter_group_packets(bin_data: bytes, group: int):
    if len(bin_data) < 0x18:
        return
    group_count = le32(bin_data, 0)
    group_table = le32(bin_data, 4)
    if group < 0 or group >= group_count or group_table + group * 4 + 4 > len(bin_data):
        return
    bd = group_table + le32(bin_data, group_table + group * 4)
    if bd + 0x1a > len(bin_data):
        return
    vert_count = le32(bin_data, bd + 0x00)
    prim_count = le16(bin_data, bd + 0x10)
    tex_base = sle16(bin_data, bd + 0x12)
    poly_off = bd + le32(bin_data, bd + 0x14)
    p = poly_off
    for pi in range(prim_count):
        if p + 4 > len(bin_data):
            yield {"error": "short_header", "pi": pi, "offset": p}
            break
        typ = bin_data[p + 3]
        source_kind = typ & 0x0f
        runtime_kind_if_unexpanded = (typ >> 2) & 0x0f
        s_stride = SOURCE_STRIDE[source_kind]
        c_stride = CACHED_STRIDE[source_kind]
        rec = {
            "pi": pi,
            "offset": p,
            "type": typ,
            "source_kind": source_kind,
            "runtime_kind_if_unexpanded": runtime_kind_if_unexpanded,
            "source_stride": s_stride,
            "cached_stride": c_stride,
            "tex_base": tex_base,
            "vert_count": vert_count,
        }
        if s_stride == 0 or p + s_stride > len(bin_data):
            rec["error"] = "bad_source_stride"
            yield rec
            break
        if source_kind in TEXTURE_SLOT_OFFSETS:
            so = TEXTURE_SLOT_OFFSETS[source_kind]
            rec["raw_slot"] = le16(bin_data, p + so) if p + so + 2 <= len(bin_data) else None
        if source_kind == 10 and p + 0x0c <= len(bin_data):
            rec["tile_count"] = le16(bin_data, p + 0x0a)
        yield rec
        if source_kind == 10:
            p += le16(bin_data, p + 0x0a) * 4
        p += s_stride


def audit_xobf(payload: bytes, bank_index: int):
    group_count = le32(payload, 0) if len(payload) >= 4 else 0
    group_table = le32(payload, 4) if len(payload) >= 8 else 0
    slot_count = le32(payload, 0x18) if len(payload) >= 0x1c else 0
    texture_slot_count = le32(payload, 0x10) if len(payload) >= 0x14 else 0
    out = {
        "bank": bank_index,
        "size": len(payload),
        "group_count": group_count,
        "group_table": group_table,
        "slot_count": slot_count,
        "texture_slot_count": texture_slot_count,
        "groups": {},
        "kind_counts": Counter(),
        "runtime_kind_counts": Counter(),
        "texture_refs": Counter(),
        "invalid_texture_refs": 0,
        "tile_runs": 0,
        "tile_packets": 0,
        "errors": Counter(),
    }
    for group in range(group_count):
        g = {
            "kind_counts": Counter(),
            "runtime_kind_counts": Counter(),
            "texture_refs": Counter(),
            "tile_runs": 0,
            "tile_packets": 0,
            "errors": Counter(),
        }
        for pkt in iter_group_packets(payload, group):
            if "error" in pkt:
                g["errors"][pkt["error"]] += 1
                out["errors"][pkt["error"]] += 1
                continue
            sk = pkt["source_kind"]
            rk = pkt["runtime_kind_if_unexpanded"]
            g["kind_counts"][sk] += 1
            g["runtime_kind_counts"][rk] += 1
            out["kind_counts"][sk] += 1
            out["runtime_kind_counts"][rk] += 1
            raw_slot = pkt.get("raw_slot")
            if raw_slot is not None and raw_slot != 0xffff:
                slot = pkt["tex_base"] + (raw_slot & 0x3fff)
                g["texture_refs"][slot] += 1
                out["texture_refs"][slot] += 1
                if slot < 0 or slot >= texture_slot_count:
                    g["errors"]["tex_slot_oob"] += 1
                    out["invalid_texture_refs"] += 1
            if sk == 10:
                count = pkt.get("tile_count", 0)
                g["tile_runs"] += 1
                g["tile_packets"] += count
                out["tile_runs"] += 1
                out["tile_packets"] += count
        if sum(g["kind_counts"].values()) or sum(g["errors"].values()):
            out["groups"][group] = g
    return out


def fmt_counter(c: Counter) -> str:
    if not c:
        return "none"
    return " ".join(f"{k:x}:{v}" if isinstance(k, int) else f"{k}:{v}"
                    for k, v in sorted(c.items()))


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("exp", type=Path)
    ap.add_argument("--out", type=Path)
    args = ap.parse_args()

    data = args.exp.read_bytes()
    xobfs = [payload for _off, _size, _body, parent, payload in collect(data, b"BIN ")
             if parent == b"XOBF"]
    banks = [audit_xobf(payload, i) for i, payload in enumerate(xobfs)]
    juncs = decode_junc(data)
    rsegs = decode_rseg(data)
    xrtps = decode_xrtp(data)

    lines: list[str] = []
    lines.append(f"# Terrain Visual Format Audit: {args.exp.name}")
    lines.append("")
    lines.append(f"XOBF banks={len(banks)} JUNC={len(juncs)} RSEG={len(rsegs)} XRTP={len(xrtps)}")
    lines.append("")
    lines.append("## XRTP/RSEG")
    for i, rt in enumerate(xrtps):
        lines.append(
            f"XRTP[{i}] off=0x{rt['off']:x} width={rt['width']} step={rt['step']} "
            f"tex_id={rt['tex_id']} flags=0x{rt['flags'] & 0xffff:04x} payload={rt['payload']} "
            f"tim={rt['tim_magic']}"
        )
    lines.append("RSEG type counts: " + fmt_counter(Counter(r["type"] for r in rsegs)))
    lines.append("")
    lines.append("## XOBF Packet Census")
    for b in banks:
        lines.append(
            f"bank {b['bank']}: size=0x{b['size']:x} groups={b['group_count']} "
            f"slots={b['slot_count']} textures={b['texture_slot_count']}"
        )
        lines.append(f"  source kinds: {fmt_counter(b['kind_counts'])}")
        lines.append(
            "  shifted runtime kinds if source expansion is skipped: "
            f"{fmt_counter(b['runtime_kind_counts'])}"
        )
        lines.append(
            f"  texture refs={sum(b['texture_refs'].values())} unique={len(b['texture_refs'])} "
            f"invalid={b['invalid_texture_refs']} tile_runs={b['tile_runs']} tile_packets={b['tile_packets']} "
            f"errors={fmt_counter(b['errors'])}"
        )
    lines.append("")
    lines.append("## JUNC Patch Ownership")
    patch_counts = Counter((j.get("bank"), j.get("slot")) for j in juncs if j.get("has_patch"))
    lines.append(f"patch nodes={sum(patch_counts.values())} unique_bank_slots={len(patch_counts)}")
    group_to_nodes = defaultdict(list)
    for j in juncs:
        if not j.get("has_patch"):
            continue
        bank = j["bank"]
        slot = j["slot"]
        group = decode_slot_group(xobfs[bank], slot) if 0 <= bank < len(xobfs) else None
        group_to_nodes[(bank, group)].append(j)
    for (bank, group), nodes in sorted(group_to_nodes.items()):
        if bank is None or group is None or bank < 0 or bank >= len(banks):
            lines.append(f"bank={bank} group={group} nodes={len(nodes)} unresolved")
            continue
        g = banks[bank]["groups"].get(group)
        names = Counter(j.get("name", "") for j in nodes)
        name_text = ", ".join(f"{k or '<blank>'}:{v}" for k, v in names.most_common(6))
        if g is None:
            lines.append(f"bank={bank} group={group} nodes={len(nodes)} packets=none names={name_text}")
        else:
            lines.append(
                f"bank={bank} group={group} nodes={len(nodes)} "
                f"kinds={fmt_counter(g['kind_counts'])} "
                f"shifted_if_unexpanded={fmt_counter(g['runtime_kind_counts'])} "
                f"texrefs={sum(g['texture_refs'].values())}/{len(g['texture_refs'])} "
                f"tiles={g['tile_runs']}/{g['tile_packets']} errors={fmt_counter(g['errors'])} "
                f"names={name_text}"
            )
    lines.append("")
    lines.append("## Texture-Heavy Groups")
    for b in banks:
        ranked = []
        for group, g in b["groups"].items():
            textured = sum(g["texture_refs"].values()) + g["tile_packets"]
            if textured:
                ranked.append((textured, group, g))
        for textured, group, g in sorted(ranked, reverse=True)[:16]:
            lines.append(
                f"bank={b['bank']} group={group} textured={textured} "
                f"kinds={fmt_counter(g['kind_counts'])} texrefs={sum(g['texture_refs'].values())} "
                f"tiles={g['tile_runs']}/{g['tile_packets']}"
            )

    text = "\n".join(lines) + "\n"
    if args.out:
        args.out.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
