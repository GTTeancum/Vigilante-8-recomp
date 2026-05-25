#!/usr/bin/env python3
"""Decode runtime-relevant Vigilante 8 terrain EXP chunks.

This is a headless audit tool for the source-driven terrain investigation.  It
does not render; it decodes the geometry/navigation/collision chunks that feed
runtime systems:

  ZMAP/ZONE  - base terrain height table
  FORM XOBF  - visual groups, slot hierarchy, obstacle streams
  FORM OBJ/HEAD - placed object roots
  AIMP       - AI/navigation quadtree, 10-byte little-endian cells
  JUNC/RSEG  - route graph nodes and edges
  BSP        - recursive big-endian kd-tree used for static object placement
"""
from __future__ import annotations

from collections import Counter
from pathlib import Path
import argparse
import struct
import sys


def be32(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 4], "big", signed=False)


def sbe32(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 4], "big", signed=True)


def sbe16(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 2], "big", signed=True)


def ule16(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 2], "little", signed=False)


def walk_iff(data: bytes, start: int = 0, end: int | None = None,
             parent: bytes = b""):
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
            payload = data[body + 4:body + size]
            yield p, size, body + 4, form_type, parent, payload
            yield from walk_iff(data, body + 4, body + size, form_type)
        else:
            yield p, size, body, tag, parent, data[body:body + size]
        p += 8 + ((size + 1) & ~1)


def collect(data: bytes, tag: bytes, parent: bytes | None = None):
    out = []
    for off, size, body, ctag, cparent, payload in walk_iff(data):
        if ctag == tag and (parent is None or cparent == parent):
            out.append((off, size, body, cparent, payload))
    return out


def fmt_range(vals, scale=1.0) -> str:
    if not vals:
        return "none"
    lo = min(vals) * scale
    hi = max(vals) * scale
    return f"[{lo:.1f}..{hi:.1f}] span={hi - lo:.1f}"


def decode_zmap(payload: bytes):
    cells = []
    for row in range(32):
        for col in range(32):
            idx = int.from_bytes(payload[(row * 32 + col) * 2:(row * 32 + col) * 2 + 2],
                                 "big")
            if idx:
                cells.append((col, row, idx))
    return cells


def decode_terr_head(data: bytes):
    chunks = collect(data, b"HEAD", b"TERR")
    if not chunks:
        return None
    _off, size, _body, _parent, payload = chunks[0]
    vals = [sbe16(payload, i) for i in range(0, min(size, len(payload)), 2)]
    if len(vals) < 5:
        return None
    return {
        "flat_level": vals[0],
        "env_a": vals[1],
        "env_b": vals[2],
        "reserved_a": vals[3],
        "reserved_b": vals[4],
        "table": vals[5:],
    }


def decode_cols(data: bytes):
    chunks = collect(data, b"COLS")
    if not chunks:
        return None
    _off, size, _body, _parent, payload = chunks[0]
    words = [be32(payload, i) for i in range(0, min(size, len(payload)), 4)]
    if len(words) < 7:
        return None
    return {
        "words": words,
        "light_a_rgb": words[0],
        "light_b_rgb": words[1],
        "far_rgb": words[2],
        "sky_rgb": words[3],
        "light_a_vec": words[4],
        "light_b_vec": words[5],
        "ambient_rgb": words[6],
    }


def iter_iff_children(payload: bytes):
    p = 0
    end = len(payload)
    while p + 8 <= end:
        tag = payload[p:p + 4]
        size = be32(payload, p + 4)
        body = p + 8
        if body + size > end:
            break
        yield p, tag, size, payload[body:body + size]
        p = body + ((size + 1) & ~1)


def decode_obj_head(off: int, size: int, payload: bytes):
    if size < 34:
        return None
    template_index = sbe16(payload, 26)
    return {
        "off": off,
        "script_byte": payload[0],
        "type": payload[1],
        "id": sbe16(payload, 2),
        "flags_raw": be32(payload, 4),
        "flags": be32(payload, 4) & 0xfff867fe,
        "x": sbe32(payload, 8),
        "y_raw": sbe32(payload, 12),
        "y": sbe32(payload, 12) - 0x100000,
        "z": sbe32(payload, 16),
        "ry": sbe16(payload, 20),
        "rx": sbe16(payload, 22),
        "rz": sbe16(payload, 24),
        "bank": template_index,
        "template_slot": template_index + 0x12,
        "slot": sbe16(payload, 28),
        "initial_strength": sbe32(payload, 30),
        "name": payload[34:size].split(b"\0", 1)[0].decode("ascii", "replace"),
    }


def decode_strn(payload: bytes):
    if len(payload) < 4:
        return None
    a = sbe32(payload, 0)
    b = sbe32(payload, 4) if len(payload) > 4 and len(payload) >= 8 else a
    return {"health": a, "max_health": b, "size": len(payload)}


def decode_lght(payload: bytes):
    if len(payload) < 18:
        return None
    rgb_word = be32(payload, 0)
    return {
        "rgb_word": rgb_word,
        "r": payload[0],
        "g": payload[1],
        "b": payload[2],
        "pad": payload[3],
        "inner_radius": sbe32(payload, 4),
        "outer_radius": sbe32(payload, 8),
        "cone_min": sbe16(payload, 12),
        "cone_max": sbe16(payload, 14),
        "intensity": sbe16(payload, 16),
        "size": len(payload),
    }


def decode_objs(data: bytes):
    objs = []
    for off, _size, _body, tag, _parent, payload in walk_iff(data):
        if tag != b"OBJ ":
            continue
        obj = {"off": off, "head": None, "strn": None, "lght": None, "chunks": []}
        for child_off, child_tag, child_size, child_payload in iter_iff_children(payload):
            name = child_tag.decode("latin1")
            obj["chunks"].append(name)
            abs_off = off + 12 + child_off
            if child_tag == b"HEAD":
                obj["head"] = decode_obj_head(abs_off, child_size, child_payload)
            elif child_tag == b"STRN":
                obj["strn"] = decode_strn(child_payload)
            elif child_tag == b"LGHT":
                obj["lght"] = decode_lght(child_payload)
        objs.append(obj)
    return objs


def decode_heads(data: bytes):
    return [o["head"] for o in decode_objs(data) if o["head"] is not None]


def decode_rects(data: bytes):
    rects = []
    for off, size, body, parent, payload in collect(data, b"RECT"):
        if size < 14:
            continue
        x0 = sbe16(payload, 0)
        z0 = sbe16(payload, 2)
        x1 = sbe16(payload, 4)
        z1 = sbe16(payload, 6)
        rects.append({
            "off": off,
            "x": x0,
            "z": z0,
            "w": x1 - x0 + 1,
            "d": z1 - z0 + 1,
            "unused": sbe16(payload, 8),
            "attr_a": sbe16(payload, 10),
            "attr_b": sbe16(payload, 12),
        })
    return rects


def decode_xrtps(data: bytes):
    types = []
    for off, size, body, parent, payload in collect(data, b"XRTP"):
        if size < 12:
            continue
        types.append({
            "off": off,
            "size": size,
            "width": sbe32(payload, 0),
            "step": sbe32(payload, 4),
            "texture_id": sbe16(payload, 8),
            "flags": sbe16(payload, 10),
        })
    return types


def decode_juncs(data: bytes):
    nodes = []
    for off, size, body, parent, payload in collect(data, b"JUNC"):
        if size < 10:
            continue
        flags = payload[8]
        count = payload[9]
        p = 10
        node = {
            "off": off,
            "size": size,
            "x": sbe32(payload, 0),
            "z": sbe32(payload, 4),
            "flags": flags,
            "edge_count": count,
            "has_y": bool(flags & 2),
            "has_patch": False,
        }
        if flags & 2 and p + 4 <= size:
            node["y"] = sbe32(payload, p)
            p += 4
        if p + 6 <= size and p < size:
            node["has_patch"] = True
            node["bank"] = sbe16(payload, p)
            node["slot"] = sbe16(payload, p + 2)
            node["rot"] = sbe16(payload, p + 4)
            node["patch_name"] = payload[p + 6:size].rstrip(b"\0").decode("ascii", "replace")
        nodes.append(node)
    return nodes


def decode_rsegs(data: bytes):
    segs = []
    for off, size, body, parent, payload in collect(data, b"RSEG"):
        if size < 14:
            continue
        p = 0
        seg = {"off": off, "size": size}
        if size == 0x16:
            seg["u10"] = int.from_bytes(payload[p:p + 2], "big"); p += 2
            seg["u08"] = int.from_bytes(payload[p:p + 2], "big"); p += 2
            seg["u0c"] = 0
        seg["u10"] = sbe16(payload, p); p += 2
        seg["u08"] = sbe16(payload, p); p += 2
        seg["u0c"] = sbe16(payload, p); p += 2
        seg["node_a"] = sbe16(payload, p); p += 2
        seg["node_b"] = sbe16(payload, p); p += 2
        vals = []
        while p + 4 <= size and len(vals) < 4:
            vals.append(sbe32(payload, p))
            p += 4
        seg["ctrl"] = vals
        segs.append(seg)
    return segs


def decode_aimp(payload: bytes):
    cells = []
    for i in range(len(payload) // 10):
        cells.append(struct.unpack_from("<5H", payload, i * 10))
    leaves = []

    def walk(idx: int, x0: int, z0: int, size: int, seen: set[int]):
        if idx < 0 or idx >= len(cells) or idx in seen or size <= 0:
            return
        seen.add(idx)
        half = size // 2
        for q, child in enumerate(cells[idx][1:]):
            qx = x0 + (q & 1) * half
            qz = z0 + ((q >> 1) & 1) * half
            if child == 0:
                continue
            if child & 0x8000:
                leaves.append((qx, qz, half, child, idx, q))
            else:
                walk(idx + child, qx, qz, half, seen)

    walk(0, 0, 0, 2048, set())
    return cells, leaves


def decode_bsp(payload: bytes):
    """Decode LOAD 80100148's recursive BSP stream.

    On disc each node begins with a big-endian i16 kind.  Kind 0 is a leaf and
    consumes no more file data; the runtime allocates a 0x10-byte tree node and
    initializes an empty object-list sentinel at node+4.  Kinds 1 and 2 are
    split-X and split-Z nodes: they carry one big-endian i32 split coordinate
    followed by the two child nodes in preorder.  Any other kind is rejected by
    the loader after consuming the kind word.
    """
    splits = []
    leaves = []
    errors = []

    root = None

    def walk(p: int, depth: int, path: str):
        if p + 2 > len(payload):
            errors.append(f"{path}:short_kind@0x{p:x}")
            return None, p
        kind = sbe16(payload, p)
        start = p
        p += 2
        if kind == 0:
            leaf = {"off": start, "kind": 0, "depth": depth, "path": path}
            leaves.append(leaf)
            return leaf, p
        if kind not in (1, 2):
            errors.append(f"{path}:bad_kind={kind}@0x{start:x}")
            return None, p
        if p + 4 > len(payload):
            errors.append(f"{path}:short_coord@0x{p:x}")
            return None, len(payload)
        coord = sbe32(payload, p)
        p += 4
        node = {
            "off": start,
            "kind": kind,
            "coord": coord,
            "depth": depth,
            "path": path,
        }
        splits.append(node)
        node["left"], p = walk(p, depth + 1, path + "L")
        node["right"], p = walk(p, depth + 1, path + "R")
        node["end"] = p
        return node, p

    root, end = walk(0, 0, "")
    trailing = len(payload) - end
    return {
        "root": root,
        "splits": splits,
        "leaves": leaves,
        "errors": errors,
        "end": end,
        "trailing": trailing,
    }


def bsp_find_leaf(root, x: int, z: int):
    node = root
    while node is not None and node.get("kind") in (1, 2):
        coord = node["coord"]
        value = x if node["kind"] == 1 else z
        node = node["right"] if coord < value else node["left"]
    return node if node is not None and node.get("kind") == 0 else None


def summarize(path: Path, out):
    data = path.read_bytes()
    tags = Counter(tag.decode("latin1") for _, _, _, tag, _, _ in walk_iff(data)
                   if tag != b"FORM")
    print(f"== {path.name} ==", file=out)
    print("chunks: " + " ".join(f"{k}={v}" for k, v in sorted(tags.items())), file=out)

    zmap = collect(data, b"ZMAP")
    if zmap:
        cells = decode_zmap(zmap[0][4])
        xs = [c[0] for c in cells]
        zs = [c[1] for c in cells]
        print(f"ZMAP populated={len(cells)} chunkX={fmt_range(xs)} chunkZ={fmt_range(zs)}",
              file=out)

    terr_head = decode_terr_head(data)
    if terr_head:
        table = terr_head["table"]
        print(f"TERR_HEAD flat=0x{terr_head['flat_level'] & 0xffff:04x} "
              f"env=0x{terr_head['env_a'] & 0xffff:04x}/0x{terr_head['env_b'] & 0xffff:04x} "
              f"reserved=0x{terr_head['reserved_a'] & 0xffff:04x}/0x{terr_head['reserved_b'] & 0xffff:04x} "
              f"table_words={len(table)} "
              f"table=" + ",".join(f"0x{v & 0xffff:04x}" for v in table),
              file=out)

    cols = decode_cols(data)
    if cols:
        print("COLS color_words="
              + ",".join(f"0x{w:08x}" for w in cols["words"])
              + f" far_rgb=0x{cols['far_rgb']:08x} ambient=0x{cols['ambient_rgb']:08x}",
              file=out)

    objs = decode_objs(data)
    heads = [o["head"] for o in objs if o["head"] is not None]
    if objs:
        strn = [o["strn"] for o in objs if o["strn"] is not None]
        lght = [o["lght"] for o in objs if o["lght"] is not None]
        extras = Counter(ch for o in objs for ch in o["chunks"]
                         if ch not in ("HEAD", "STRN", "LGHT"))
        print(f"OBJ forms={len(objs)} heads={len(heads)} "
              f"STRN={len(strn)} LGHT={len(lght)} "
              f"extra_chunks="
              + (", ".join(f"{k}:{v}" for k, v in extras.most_common())
                 if extras else "none"),
              file=out)
    if heads:
        xs = [h["x"] / 65536 for h in heads]
        ys = [h["y"] / 131072 for h in heads]
        zs = [h["z"] / 65536 for h in heads]
        print(f"HEAD count={len(heads)} X={fmt_range(xs)} Y={fmt_range(ys)} Z={fmt_range(zs)}",
              file=out)
        print("HEAD types: " + ", ".join(f"{k}:{v}" for k, v in Counter(h["type"] for h in heads).most_common()),
              file=out)
        print("HEAD bank/slot: banks="
              + ", ".join(f"{k}:{v}" for k, v in sorted(Counter(h["bank"] for h in heads).items()))
              + f" slot={fmt_range([h['slot'] for h in heads])} "
              + f"initial_strength={fmt_range([h['initial_strength'] for h in heads])}",
              file=out)
    if objs:
        lght = [o["lght"] for o in objs if o["lght"] is not None]
        if lght:
            print(f"LGHT lights={len(lght)} "
                  f"outer_radius={fmt_range([b['outer_radius'] / 65536 for b in lght])} "
                  f"inner_radius={fmt_range([b['inner_radius'] / 65536 for b in lght])} "
                  f"cone_min={fmt_range([b['cone_min'] for b in lght])} "
                  f"cone_max={fmt_range([b['cone_max'] for b in lght])} "
                  f"intensity={fmt_range([b['intensity'] for b in lght])} "
                  f"rgb=" + ", ".join(
                      f"0x{rgb:08x}:{n}"
                      for rgb, n in Counter(b["rgb_word"] for b in lght).most_common(8)),
                  file=out)

    rects = decode_rects(data)
    if rects:
        xs, zs = [], []
        attrs = Counter()
        for r in rects:
            xs.extend([r["x"], r["x"] + r["w"] - 1])
            zs.extend([r["z"], r["z"] + r["d"] - 1])
            attrs[(r["attr_a"], r["attr_b"])] += 1
        print(f"RECT count={len(rects)} X={fmt_range(xs)} Z={fmt_range(zs)} "
              f"attrs="
              + ", ".join(f"0x{a & 0xffff:04x}/0x{b & 0xffff:04x}:{n}"
                          for (a, b), n in attrs.most_common(8)),
              file=out)

    xrtps = decode_xrtps(data)
    if xrtps:
        print(f"XRTP route_types={len(xrtps)} "
              + ", ".join(
                  f"{i}:width={x['width'] / 65536:.2f} "
                  f"step={x['step'] / 65536:.2f} "
                  f"tex={x['texture_id']} flags=0x{x['flags'] & 0xffff:04x} "
                  f"size=0x{x['size']:x}"
                  for i, x in enumerate(xrtps)),
              file=out)

    aimp = collect(data, b"AIMP")
    if aimp:
        cells, leaves = decode_aimp(aimp[0][4])
        xs, zs, sizes, flags = [], [], [], []
        for x, z, sz, flag, _, _ in leaves:
            xs.extend([x, x + sz])
            zs.extend([z, z + sz])
            sizes.append(sz)
            flags.append(flag)
        print(f"AIMP cells={len(cells)} leaves={len(leaves)} X={fmt_range(xs)} Z={fmt_range(zs)}",
              file=out)
        print("AIMP leaf sizes: " + ", ".join(f"{k}:{v}" for k, v in Counter(sizes).most_common()),
              file=out)
        print("AIMP flags top: " + ", ".join(f"0x{k:04x}:{v}" for k, v in Counter(flags).most_common(8)),
              file=out)

    juncs = decode_juncs(data)
    if juncs:
        xs = [j["x"] / 65536 for j in juncs]
        zs = [j["z"] / 65536 for j in juncs]
        print(f"JUNC count={len(juncs)} X={fmt_range(xs)} Z={fmt_range(zs)} "
              f"patches={sum(1 for j in juncs if j['has_patch'])}", file=out)
        patch_names = Counter(j["patch_name"] for j in juncs if j.get("patch_name"))
        if patch_names:
            slots = sorted({(j.get("bank", -1), j.get("slot", -1))
                            for j in juncs if j["has_patch"]})
            print("JUNC patch slots: "
                  + ", ".join(f"{bank}:{slot}" for bank, slot in slots)
                  + " names="
                  + ", ".join(f"{name}:{count}" for name, count in patch_names.most_common()),
                  file=out)

    rsegs = decode_rsegs(data)
    if rsegs:
        type_counts = Counter(s["u10"] for s in rsegs)
        flag_counts = Counter(s["u0c"] for s in rsegs)
        print(f"RSEG count={len(rsegs)} types="
              + ", ".join(f"{k}:{v}" for k, v in sorted(type_counts.items()))
              + " flags="
              + ", ".join(f"0x{k & 0xffff:04x}:{v}" for k, v in sorted(flag_counts.items()))
              + " node_pairs="
              + ", ".join(f"{s['node_a']}-{s['node_b']}" for s in rsegs[:12])
              + (" ..." if len(rsegs) > 12 else ""), file=out)
        ctrl = [v / 65536 for s in rsegs for v in s["ctrl"]]
        if ctrl:
            print(f"RSEG ctrl_offsets={fmt_range(ctrl)}", file=out)

    bsp = collect(data, b"BSP ")
    if bsp:
        tree = decode_bsp(bsp[0][4])
        recs = tree["splits"]
        x = [r["coord"] / 65536 for r in recs if r["kind"] == 1]
        z = [r["coord"] / 65536 for r in recs if r["kind"] == 2]
        depths = [r["depth"] for r in recs] + [l["depth"] for l in tree["leaves"]]
        print(f"BSP nodes={len(recs) + len(tree['leaves'])} "
              f"splits={len(recs)} leaves={len(tree['leaves'])} "
              f"depth={max(depths) if depths else 0} "
              f"Xplanes={fmt_range(x)} Zplanes={fmt_range(z)} "
              f"trailing={tree['trailing']} errors={len(tree['errors'])}",
              file=out)
        if heads and tree["root"] is not None:
            occ = Counter()
            missed = 0
            type0 = [h for h in heads if h["type"] == 0]
            for h in type0:
                leaf = bsp_find_leaf(tree["root"], h["x"], h["z"])
                if leaf is None:
                    missed += 1
                else:
                    occ[leaf["path"]] += 1
            print(f"BSP type0_object_insertions={len(type0)} "
                  f"occupied_leaves={len(occ)} missed={missed} "
                  f"max_leaf_count={max(occ.values()) if occ else 0}",
                  file=out)
        if tree["errors"]:
            print("BSP errors: " + "; ".join(tree["errors"][:6])
                  + (" ..." if len(tree["errors"]) > 6 else ""), file=out)

    print("", file=out)


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("paths", nargs="*", help="terrain EXP files or directories")
    ap.add_argument("-o", "--output", help="write report here")
    ns = ap.parse_args(argv)

    inputs = ns.paths or ["PS1 game/Terrain"]
    paths = []
    for item in inputs:
        p = Path(item)
        if p.is_dir():
            paths.extend(sorted(p.glob("*.EXP")))
        else:
            paths.append(p)
    if ns.output:
        out_path = Path(ns.output)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        with out_path.open("w", encoding="utf-8") as f:
            for p in paths:
                summarize(p, f)
    else:
        for p in paths:
            summarize(p, sys.stdout)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
