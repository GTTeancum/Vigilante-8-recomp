#!/usr/bin/env python3
"""Cross-level terrain anomaly report.

This is a source-data audit. It does not use renderer screenshots. The goal is
to flag levels whose gameplay/navigation/object data disagree with the decoded
terrain carriers we currently understand.
"""
from __future__ import annotations

from collections import Counter
from pathlib import Path
import struct

from terrain_runtime_decode import (
    collect,
    decode_aimp,
    decode_bsp,
    decode_heads,
    decode_juncs,
    decode_rsegs,
    decode_xrtps,
    decode_zmap,
    bsp_find_leaf,
)


ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "analysis" / "terrain_anomaly_report.txt"


def ule32(b: bytes, o: int) -> int:
    return struct.unpack_from("<I", b, o)[0]


def sle16(b: bytes, o: int) -> int:
    return struct.unpack_from("<h", b, o)[0]


def image_meta(payload: bytes) -> str:
    if len(payload) < 0x220:
        return "too_small"
    flags = ule32(payload, 4)
    depth = flags & 3
    image_off = ule32(payload, 8)
    if image_off + 0x14 > len(payload):
        return f"bad_image_off=0x{image_off:x}"
    cx, cy, cw, ch = (sle16(payload, 0x0c + i) for i in (0, 2, 4, 6))
    ix, iy, iw, ih = (sle16(payload, image_off + 0x0c + i) for i in (0, 2, 4, 6))
    pix = len(payload) - (image_off + 0x14)
    if depth == 0:
        pixel_w = iw * 4
    elif depth == 1:
        pixel_w = iw * 2
    else:
        pixel_w = iw
    expected = pixel_w * ih
    ok = "ok" if expected >= 0 and pix >= expected else "short"
    return (f"flags=0x{flags:x} depth={depth} clut=({cx},{cy},{cw},{ch}) "
            f"rect_words=({ix},{iy},{iw},{ih}) pixels=({pixel_w},{ih}) "
            f"pix={pix}/{expected} {ok}")


def fixed_to_cell(v: int) -> int:
    return v >> 16


def in_zmap_chunk(zcells: set[tuple[int, int]], x: int, z: int) -> bool:
    return ((fixed_to_cell(x) >> 6), (fixed_to_cell(z) >> 6)) in zcells


def bbox(vals: list[float]) -> str:
    if not vals:
        return "none"
    return f"[{min(vals):.1f}..{max(vals):.1f}] span={max(vals) - min(vals):.1f}"


def audit_file(exp: Path) -> list[str]:
    data = exp.read_bytes()
    lines: list[str] = [f"== {exp.name} =="]
    anomalies: list[str] = []

    zmap_chunks = collect(data, b"ZMAP")
    zones = collect(data, b"ZONE")
    if not zmap_chunks:
        anomalies.append("missing ZMAP")
        zcells: set[tuple[int, int]] = set()
        zindices: list[int] = []
    else:
        cells = decode_zmap(zmap_chunks[0][4])
        zcells = {(x, z) for x, z, _idx in cells}
        zindices = [idx for _x, _z, idx in cells]
        if zindices and max(zindices) > len(zones):
            anomalies.append(f"ZMAP references missing ZONE index {max(zindices)} > {len(zones)}")
        if len(zindices) != len(zones):
            anomalies.append(f"ZONE count {len(zones)} != populated ZMAP cells {len(zindices)}")
        xs = [x for x, _z in zcells]
        zs = [z for _x, z in zcells]
        lines.append(f"ZMAP cells={len(zcells)} chunksX={bbox([float(x) for x in xs])} "
                     f"chunksZ={bbox([float(z) for z in zs])} zones={len(zones)}")

    tinf = collect(data, b"TINF", b"TERR") or collect(data, b"TINF")
    material_ids = Counter()
    hvals: list[int] = []
    for _off, _size, _body, _parent, zone in zones:
        for p in range(0, min(len(zone), 0x4000), 4):
            src = zone[p] | (zone[p + 1] << 8)
            h = ((((src >> 8) | ((src << 8) & 0xffff)) - 0x0200) & 0xffff)
            h |= ((zone[p + 2] >> 3) << 11)
            hvals.append(h & 0x7ff)
            material_ids[zone[p + 3]] += 1
    if tinf and tinf[0][1] != 0x2800:
        anomalies.append(f"TINF size 0x{tinf[0][1]:x} != 0x2800")
    if hvals:
        lines.append(f"ZONE height11={min(hvals)}..{max(hvals)} "
                     f"materials={len(material_ids)} top={material_ids.most_common(5)}")

    heads = decode_heads(data)
    if heads:
        hx = [h["x"] / 65536 for h in heads]
        hz = [h["z"] / 65536 for h in heads]
        htypes = Counter(h["type"] for h in heads)
        outside = [h for h in heads if not in_zmap_chunk(zcells, h["x"], h["z"])]
        type0 = [h for h in heads if h["type"] == 0]
        type0_outside = [h for h in type0 if not in_zmap_chunk(zcells, h["x"], h["z"])]
        lines.append(f"HEAD count={len(heads)} types={dict(sorted(htypes.items()))} "
                     f"X={bbox(hx)} Z={bbox(hz)} outside_zmap={len(outside)} "
                     f"type0_outside_zmap={len(type0_outside)}")
        if outside:
            labels = [
                f"{h['name'] or '<unnamed>'}@({h['x'] / 65536:.1f},{h['z'] / 65536:.1f})"
                f"/type{h['type']}"
                for h in outside[:8]
            ]
            lines.append("HEAD outside_zmap sample: " + ", ".join(labels)
                         + (" ..." if len(outside) > 8 else ""))

    aimp_chunks = collect(data, b"AIMP")
    if aimp_chunks:
        _cells, leaves = decode_aimp(aimp_chunks[0][4])
        outside = []
        xs: list[float] = []
        zs: list[float] = []
        for x, z, sz, _flag, _idx, _q in leaves:
            cx = (x + sz // 2) << 16
            cz = (z + sz // 2) << 16
            xs.extend([x, x + sz])
            zs.extend([z, z + sz])
            if not in_zmap_chunk(zcells, cx, cz):
                outside.append((x, z, sz))
        if outside:
            anomalies.append(f"AIMP leaf centers outside ZMAP={len(outside)}/{len(leaves)}")
        lines.append(f"AIMP leaves={len(leaves)} X={bbox(xs)} Z={bbox(zs)} outside_zmap={len(outside)}")

    juncs = decode_juncs(data)
    if juncs:
        outside = [j for j in juncs if not in_zmap_chunk(zcells, j["x"], j["z"])]
        if outside:
            anomalies.append(f"JUNC nodes outside ZMAP={len(outside)}/{len(juncs)}")
        lines.append(f"JUNC count={len(juncs)} patches={sum(1 for j in juncs if j['has_patch'])} "
                     f"X={bbox([j['x'] / 65536 for j in juncs])} "
                     f"Z={bbox([j['z'] / 65536 for j in juncs])} outside_zmap={len(outside)}")

    rsegs = decode_rsegs(data)
    xrtps = decode_xrtps(data)
    if rsegs:
        bad_nodes = sum(1 for s in rsegs
                        if s["node_a"] < 0 or s["node_b"] < 0
                        or s["node_a"] >= len(juncs) or s["node_b"] >= len(juncs))
        bad_types = sum(1 for s in rsegs if s["u10"] < 0 or s["u10"] >= len(xrtps))
        if bad_nodes:
            anomalies.append(f"RSEG bad node refs={bad_nodes}")
        if bad_types:
            anomalies.append(f"RSEG bad XRTP refs={bad_types}")
        lines.append(f"RSEG count={len(rsegs)} XRTP={len(xrtps)} bad_nodes={bad_nodes} bad_types={bad_types}")

    bsp_chunks = collect(data, b"BSP ")
    if bsp_chunks:
        tree = decode_bsp(bsp_chunks[0][4])
        if tree["errors"] or tree["trailing"]:
            anomalies.append(f"BSP decode errors={len(tree['errors'])} trailing={tree['trailing']}")
        if heads and tree["root"] is not None:
            type0 = [h for h in heads if h["type"] == 0]
            missed = sum(1 for h in type0 if bsp_find_leaf(tree["root"], h["x"], h["z"]) is None)
            if missed:
                anomalies.append(f"BSP missed type0 insertions={missed}/{len(type0)}")
            lines.append(f"BSP nodes={len(tree['splits']) + len(tree['leaves'])} "
                         f"leaves={len(tree['leaves'])} missed_type0={missed}")

    for tag in (b"XBMP", b"XBGM"):
        chunks = collect(data, tag, b"TERR") or collect(data, tag)
        if len(chunks) != 1:
            anomalies.append(f"{tag.decode()} count={len(chunks)}")
        for _off, _size, _body, _parent, payload in chunks:
            lines.append(f"{tag.decode()} {image_meta(payload)}")

    if anomalies:
        lines.append("ANOMALIES:")
        lines.extend(f"  - {a}" for a in anomalies)
    else:
        lines.append("ANOMALIES: none")
    lines.append("")
    return lines


def main() -> int:
    lines: list[str] = []
    for exp in sorted((ROOT / "input" / "TERRAIN").glob("*.EXP")):
        lines.extend(audit_file(exp))
    OUT.write_text("\n".join(lines), encoding="ascii")
    print(OUT)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
