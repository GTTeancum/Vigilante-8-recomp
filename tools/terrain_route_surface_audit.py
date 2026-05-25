#!/usr/bin/env python3
"""Headless audit of runtime terrain height over gameplay route data.

This tool decodes the terrain EXP chunks that feed the original runtime:

  ZMAP/ZONE  -> DAT_800911a0 height table consumed by Terrain_HeightAt
  AIMP       -> navigation quadtree leaves
  JUNC/RSEG  -> route graph nodes/edges

It then samples the exact Terrain_HeightAt interpolation along route nodes,
route edges, and AIMP leaf centers.  The goal is to distinguish authored
gameplay terrain from far visual/object placements without relying on
screenshots or the current renderer.
"""
from __future__ import annotations

from collections import Counter
from dataclasses import dataclass
from pathlib import Path
import argparse
import math
import sys


def be32(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 4], "big", signed=False)


def sbe32(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 4], "big", signed=True)


def sbe16(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 2], "big", signed=True)


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
            yield p, size, body + 4, form_type, parent, data[body + 4:body + size]
            yield from walk_iff(data, body + 4, body + size, form_type)
        else:
            yield p, size, body, tag, parent, data[body:body + size]
        p = body + size + (size & 1)


def collect(data: bytes, tag: bytes, parent: bytes | None = None):
    return [(off, size, body, cparent, payload)
            for off, size, body, ctag, cparent, payload in walk_iff(data)
            if ctag == tag and (parent is None or cparent == parent)]


@dataclass
class Junc:
    x: int
    z: int
    flags: int
    size: int


@dataclass
class Rseg:
    route_type: int
    node_a: int
    node_b: int
    ctrl_ax: int
    ctrl_az: int
    ctrl_bx: int
    ctrl_bz: int
    size: int


class TerrainTable:
    def __init__(self):
        flat = bytearray(0x3000)
        for i in range(0, 0x2000, 2):
            flat[i] = 0xff
            flat[i + 1] = 0x45
        self.flat = bytes(flat)
        self.table: list[bytes] = [self.flat for _ in range(32 * 32)]
        self.detail: set[tuple[int, int]] = set()

    @staticmethod
    def convert_zone(zone: bytes) -> bytes:
        if len(zone) < 0x4000:
            raise ValueError("ZONE chunk shorter than 0x4000")
        dst = bytearray(0x3000)
        for row in range(64):
            for col in range(64):
                src_off = (row * 64 + col) * 4
                src = zone[src_off] | (zone[src_off + 1] << 8)
                h = (((src >> 8) | (src << 8)) - 0x0200) & 0xffff
                h |= ((zone[src_off + 2] >> 3) << 11)
                off = row * 0x80 + col * 2
                dst[off:off + 2] = int(h & 0xffff).to_bytes(2, "little")
                dst[0x2000 + row * 0x40 + col] = zone[src_off + 3]
        return bytes(dst)

    @classmethod
    def from_exp(cls, data: bytes) -> "TerrainTable":
        tbl = cls()
        zmap_chunks = collect(data, b"ZMAP")
        zone_chunks = collect(data, b"ZONE")
        if not zmap_chunks:
            raise ValueError("missing ZMAP")
        zmap = zmap_chunks[0][4]
        zones = [cls.convert_zone(payload) for _, _, _, _, payload in zone_chunks]
        for row in range(32):
            for col in range(32):
                idx = int.from_bytes(zmap[(row * 32 + col) * 2:
                                          (row * 32 + col) * 2 + 2], "big")
                if idx == 0:
                    continue
                if idx > len(zones):
                    raise ValueError(f"ZMAP references missing ZONE {idx}")
                tbl.table[col * 32 + row] = zones[idx - 1]
                tbl.detail.add((col, row))
        return tbl

    def sample_cell(self, cx: int, cz: int) -> int:
        chunk_x = (cx >> 6) & 0x1f
        chunk_z = (cz >> 6) & 0x1f
        chunk = self.table[chunk_x * 32 + chunk_z]
        off = ((cx & 0x3f) << 7) | ((cz & 0x3f) << 1)
        return int.from_bytes(chunk[off:off + 2], "little") & 0x7ff

    def height_at(self, x: int, z: int) -> int:
        fx = x & 0xffff
        fz = z & 0xffff
        cx0 = x >> 16
        cz0 = z >> 16
        h00 = self.sample_cell(cx0, cz0)
        if fx + fz < 0x10000:
            h10 = self.sample_cell(cx0 + 1, cz0)
            h01 = self.sample_cell(cx0, cz0 + 1)
            acc = h00 * 0x10000 + fx * (h10 - h00) + fz * (h01 - h00)
        else:
            h11 = self.sample_cell(cx0 + 1, cz0 + 1)
            h10 = self.sample_cell(cx0 + 1, cz0)
            h01 = self.sample_cell(cx0, cz0 + 1)
            acc = h11 * 0x10000 + (0x10000 - fx) * (h01 - h11) \
                + (0x10000 - fz) * (h10 - h11)
        if acc < 0:
            acc += 0x1f
        return acc >> 5

    def is_detail(self, x: int, z: int) -> bool:
        return ((x >> 22) & 0x1f, (z >> 22) & 0x1f) in self.detail


def decode_juncs(data: bytes) -> list[Junc]:
    out: list[Junc] = []
    for _off, size, _body, _parent, payload in collect(data, b"JUNC"):
        if size >= 10:
            out.append(Junc(
                x=sbe32(payload, 0),
                z=sbe32(payload, 4),
                flags=payload[8],
                size=size,
            ))
    return out


def decode_rsegs(data: bytes) -> list[Rseg]:
    out: list[Rseg] = []
    for _off, size, _body, _parent, payload in collect(data, b"RSEG"):
        if size < 14:
            continue
        p = 0
        if size == 0x16:
            p = 4
        route_type = sbe16(payload, p)
        p += 6
        if p + 20 <= size:
            out.append(Rseg(
                route_type=route_type,
                node_a=sbe16(payload, p),
                node_b=sbe16(payload, p + 2),
                ctrl_ax=sbe32(payload, p + 4),
                ctrl_az=sbe32(payload, p + 8),
                ctrl_bx=sbe32(payload, p + 12),
                ctrl_bz=sbe32(payload, p + 16),
                size=size,
            ))
    return out


def bezier(p0: int, p1: int, p2: int, p3: int, t: float) -> int:
    u = 1.0 - t
    return int(round(u * u * u * p0
                     + 3.0 * u * u * t * p1
                     + 3.0 * u * t * t * p2
                     + t * t * t * p3))


def decode_aimp_leaves(data: bytes) -> list[tuple[int, int, int, int]]:
    chunks = collect(data, b"AIMP")
    if not chunks:
        return []
    payload = chunks[0][4]
    cells = [tuple(int.from_bytes(payload[i * 10 + j * 2:i * 10 + j * 2 + 2],
                                  "little")
                   for j in range(5))
             for i in range(len(payload) // 10)]
    leaves: list[tuple[int, int, int, int]] = []

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
                leaves.append((qx, qz, half, child))
            else:
                walk(idx + child, qx, qz, half, seen)

    walk(0, 0, 0, 2048, set())
    return leaves


def summarize_samples(label: str, samples: list[tuple[int, int, int, bool]], out):
    if not samples:
        print(f"{label}: none", file=out)
        return
    xs = [s[0] / 65536 for s in samples]
    zs = [s[1] / 65536 for s in samples]
    ys = [s[2] for s in samples]
    detail = sum(1 for s in samples if s[3])
    print(
        f"{label}: samples={len(samples)} detail={detail}/{len(samples)} "
        f"X=[{min(xs):.1f}..{max(xs):.1f}] Z=[{min(zs):.1f}..{max(zs):.1f}] "
        f"Ypsx=[0x{min(ys):x}..0x{max(ys):x}] "
        f"Ygl=[{-max(ys) / 131072.0:.2f}..{-min(ys) / 131072.0:.2f}] "
        f"span={(max(ys) - min(ys)) / 131072.0:.2f}m",
        file=out,
    )


def audit(path: Path, out):
    data = path.read_bytes()
    terrain = TerrainTable.from_exp(data)
    juncs = decode_juncs(data)
    rsegs = decode_rsegs(data)
    aimp = decode_aimp_leaves(data)

    print(f"== {path.name} ==", file=out)
    if terrain.detail:
        xs = [x for x, _ in terrain.detail]
        zs = [z for _, z in terrain.detail]
        print(
            f"ZMAP detail chunks={len(terrain.detail)} "
            f"chunkX=[{min(xs)}..{max(xs)}] chunkZ=[{min(zs)}..{max(zs)}]",
            file=out,
        )
    else:
        print("ZMAP detail chunks=0", file=out)

    node_samples = [(j.x, j.z, terrain.height_at(j.x, j.z),
                     terrain.is_detail(j.x, j.z))
                    for j in juncs]
    summarize_samples("JUNC nodes", node_samples, out)

    edge_samples: list[tuple[int, int, int, bool]] = []
    bad_edges = 0
    edge_lengths = []
    for seg in rsegs:
        if seg.node_a < 0 or seg.node_b < 0 \
                or seg.node_a >= len(juncs) or seg.node_b >= len(juncs):
            bad_edges += 1
            continue
        a = juncs[seg.node_a]
        b = juncs[seg.node_b]
        px = (a.x, a.x + seg.ctrl_ax, b.x + seg.ctrl_bx, b.x)
        pz = (a.z, a.z + seg.ctrl_az, b.z + seg.ctrl_bz, b.z)
        dist = 0.0
        last_x, last_z = px[0], pz[0]
        for i in range(1, 33):
            t = i / 32
            x = bezier(*px, t)
            z = bezier(*pz, t)
            dist += math.hypot((x - last_x) / 65536.0,
                               (z - last_z) / 65536.0)
            last_x, last_z = x, z
        edge_lengths.append(dist)
        steps = max(2, min(96, int(math.ceil(dist / 1.5))))
        for i in range(steps + 1):
            t = i / steps
            x = bezier(*px, t)
            z = bezier(*pz, t)
            edge_samples.append((x, z, terrain.height_at(x, z),
                                 terrain.is_detail(x, z)))
    summarize_samples("RSEG edges", edge_samples, out)
    if edge_lengths:
        print(
            f"RSEG lengths: count={len(edge_lengths)} "
            f"min={min(edge_lengths):.1f} max={max(edge_lengths):.1f} "
            f"avg={sum(edge_lengths) / len(edge_lengths):.1f} bad_refs={bad_edges}",
            file=out,
        )
    else:
        print(f"RSEG lengths: count=0 bad_refs={bad_edges}", file=out)

    aimp_samples = []
    leaf_sizes = Counter()
    leaf_flags = Counter()
    for x, z, size, flag in aimp:
        cx = (x + size // 2) << 16
        cz = (z + size // 2) << 16
        aimp_samples.append((cx, cz, terrain.height_at(cx, cz),
                             terrain.is_detail(cx, cz)))
        leaf_sizes[size] += 1
        leaf_flags[flag] += 1
    summarize_samples("AIMP leaf centers", aimp_samples, out)
    if leaf_sizes:
        print(
            "AIMP leaf sizes: "
            + ", ".join(f"{k}:{v}" for k, v in leaf_sizes.most_common(8)),
            file=out,
        )
        print(
            "AIMP flags top: "
            + ", ".join(f"0x{k:04x}:{v}" for k, v in leaf_flags.most_common(8)),
            file=out,
        )
    print("", file=out)


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("paths", nargs="*", help="terrain EXP files or directories")
    ap.add_argument("-o", "--output", help="write report here")
    ns = ap.parse_args(argv)

    paths: list[Path] = []
    inputs = ns.paths or ["PS1 game/Terrain"]
    for item in inputs:
        p = Path(item)
        if p.is_dir():
            paths.extend(sorted(p.glob("*.EXP")))
        else:
            paths.append(p)

    if ns.output:
        out_path = Path(ns.output)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        with out_path.open("w", encoding="utf-8") as out:
            for path in paths:
                audit(path, out)
    else:
        for path in paths:
            audit(path, sys.stdout)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
