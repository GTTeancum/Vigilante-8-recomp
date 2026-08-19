#!/usr/bin/env python3
"""Verify packet vertex order against the NCLIP operands that admitted it."""

from __future__ import annotations

import argparse
from collections import Counter
from pathlib import Path
import re


def signed_area(points: list[tuple[float, float]]) -> float:
    (x0, y0), (x1, y1), (x2, y2) = points
    return x0 * (y1 - y2) + x1 * (y2 - y0) + x2 * (y0 - y1)


def parse_point(value: str) -> tuple[float, float]:
    x, y = value.split(",", 1)
    return float(x), float(y)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("paths", nargs="+", type=Path)
    parser.add_argument("--epsilon", type=float, default=1e-4)
    args = parser.parse_args()

    counts: Counter[tuple[str, str]] = Counter()
    examples: dict[tuple[str, str], str] = {}
    for path in args.paths:
        candidates = sorted(path.glob("recompone_geometry_frame*.txt")) \
            if path.is_dir() else [path]
        for candidate in candidates:
            for line in candidate.read_text(encoding="utf-8").splitlines():
                fields = line.split()
                if not fields or not fields[0].startswith("nclip=") or \
                        fields[0] == "nclip=unresolved":
                    continue
                packet_index = next(
                    (index for index, field in enumerate(fields)
                     if re.fullmatch(r"[0-9A-Fa-f]{8}", field)),
                    -1)
                if packet_index < 0 or len(fields) < packet_index + 8:
                    continue
                nclip = [parse_point(value) for value in
                         fields[0][len("nclip="):].split(";")]
                vertices = []
                for field in fields[packet_index + 5:packet_index + 8]:
                    values = field.split(",")
                    vertices.append((float(values[2]), float(values[3])))
                na = signed_area(nclip)
                pa = signed_area(vertices)
                if abs(na) <= args.epsilon or abs(pa) <= args.epsilon:
                    relation = "degenerate"
                elif (na > 0) == (pa > 0):
                    relation = "same"
                else:
                    relation = "reversed"
                material = fields[packet_index + 2]
                key = (material, relation)
                counts[key] += 1
                examples.setdefault(key, line)

    totals = Counter()
    for (material, _), count in counts.items():
        totals[material] += count
    for material in sorted(totals, key=int):
        print(f"material {material}: total={totals[material]} "
              f"same={counts[material, 'same']} "
              f"reversed={counts[material, 'reversed']} "
              f"degenerate={counts[material, 'degenerate']}")
    print("overall:", dict(Counter({
        relation: sum(count for (material, rel), count in counts.items()
                      if rel == relation)
        for relation in ("same", "reversed", "degenerate")
    })))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
