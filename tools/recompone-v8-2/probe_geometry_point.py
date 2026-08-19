#!/usr/bin/env python3
"""List submitted triangles covering one target-space framebuffer point."""
from __future__ import annotations

import argparse
import math
from pathlib import Path


def contains(point, vertices) -> bool:
    px, py = point
    signs = []
    for index in range(3):
        ax, ay = vertices[index]
        bx, by = vertices[(index + 1) % 3]
        signs.append((bx - ax) * (py - ay) - (by - ay) * (px - ax))
    return all(value >= -1e-4 for value in signs) or all(
        value <= 1e-4 for value in signs
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("dump", type=Path)
    parser.add_argument("x", type=float)
    parser.add_argument("y", type=float)
    args = parser.parse_args()

    hits = []
    for order, line in enumerate(
        args.dump.read_text(encoding="utf-8", errors="replace").splitlines()
    ):
        fields = line.split()
        if not fields or fields[0].startswith("#"):
            continue
        offset = 1 if fields[0].startswith("nclip=") else 0
        if len(fields) < offset + 8:
            continue
        vertices = []
        depths = []
        for token in fields[offset + 5:offset + 8]:
            values = token.split(",")
            try:
                projected_x = float(values[2])
                projected_y = float(values[3])
                # Full-display fills and other already-screen-space packets
                # have no reconstructed view position. Use their submitted
                # XY instead of silently dropping them from point ownership.
                if not math.isfinite(projected_x) or not math.isfinite(projected_y):
                    projected_x = float(values[0])
                    projected_y = float(values[1])
                vertices.append((projected_x, projected_y))
                depths.append(float(values[4]))
            except (IndexError, ValueError):
                vertices = []
                break
        if len(vertices) != 3 or not contains((args.x, args.y), vertices):
            continue
        hits.append(
            (
                order,
                fields[offset],
                fields[offset + 1],
                int(fields[offset + 2]),
                fields[offset + 3],
                fields[offset + 4],
                min(depths),
                max(depths),
                vertices,
            )
        )

    print(f"point=({args.x:.3f},{args.y:.3f}) hits={len(hits)}")
    for hit in hits:
        order, packet, owner, material, clut, texpage, zlo, zhi, vertices = hit
        print(
            f"order={order} packet=0x{packet} owner={owner} "
            f"material={material} clut={clut} texpage={texpage} "
            f"z={zlo:.2f}..{zhi:.2f} vertices={vertices}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
