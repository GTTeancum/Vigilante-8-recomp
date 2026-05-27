#!/usr/bin/env python3
"""Audit ZONE material ids against TINF records for terrain EXP files."""
from __future__ import annotations

from collections import Counter
from pathlib import Path

from terrain_runtime_decode import collect, decode_zmap


ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "analysis" / "terrain_material_audit.txt"


def be16(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 2], "big")


def le16(b: bytes, o: int) -> int:
    return int.from_bytes(b[o:o + 2], "little")


def audit_file(exp: Path) -> list[str]:
    data = exp.read_bytes()
    zmap_chunks = collect(data, b"ZMAP")
    zones = collect(data, b"ZONE")
    tinf = collect(data, b"TINF", b"TERR") or collect(data, b"TINF")
    lines: list[str] = []
    if not zmap_chunks or not zones:
        return lines

    cells = decode_zmap(zmap_chunks[0][4])
    mat_counts: Counter[int] = Counter()
    height_min = 999999
    height_max = -999999

    for _col, _row, idx in cells:
        if idx == 0 or idx > len(zones):
            continue
        zone = zones[idx - 1][4]
        for i in range(0, min(len(zone), 0x4000), 4):
            src = zone[i] | (zone[i + 1] << 8)
            h = ((((src >> 8) | ((src << 8) & 0xffff)) - 0x0200) & 0xffff)
            h |= ((zone[i + 2] >> 3) << 11)
            height_min = min(height_min, h & 0x7ff)
            height_max = max(height_max, h & 0x7ff)
            mat_counts[zone[i + 3]] += 1

    used = sorted(mat_counts)
    lines.append(
        f"{exp.name}: zones={len(zones)} zmap_cells={len(cells)} "
        f"material_ids={len(used)} height11=[{height_min}..{height_max}] "
        f"top={mat_counts.most_common(12)}"
    )
    if tinf:
        payload = tinf[0][4]
        rec_size = 0x28
        for mid in used[:16]:
            off = mid * rec_size
            if off + rec_size <= len(payload):
                words_be = [be16(payload, off + i) for i in range(0, rec_size, 2)]
                words_le = [le16(payload, off + i) for i in range(0, rec_size, 2)]
                tile_word = words_be[3]
                flags_word = words_le[0]
                lines.append(
                    f"  mat[{mid:03}] tinf28_be={words_be[:10]} "
                    f"tinf28_le={words_le[:4]} tile={tile_word & 7} "
                    f"flip={(tile_word >> 3) & 1} hidden={(flags_word & 0x1000) != 0}"
                )
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
