#!/usr/bin/env python3
"""Rank near-plane and over-screen triangles in Enhanced geometry dumps."""

from __future__ import annotations

import argparse
import json
import math
from collections import Counter
from pathlib import Path


def parse_triangle(line: str) -> dict[str, object] | None:
    fields = line.split()
    if not fields or fields[0].startswith("#"):
        return None
    offset = 1 if fields[0].startswith("nclip=") else 0
    if len(fields) < offset + 8:
        return None
    vertices: list[tuple[float, float, float]] = []
    for token in fields[offset + 5 : offset + 8]:
        values = token.split(",")
        try:
            x = float(values[2])
            y = float(values[3])
            z = float(values[4])
        except (IndexError, ValueError):
            return None
        if not math.isfinite(x) or not math.isfinite(y):
            try:
                x = float(values[0])
                y = float(values[1])
            except (IndexError, ValueError):
                return None
        if not all(math.isfinite(value) for value in (x, y, z)):
            return None
        vertices.append((x, y, z))
    ax, ay, _ = vertices[0]
    bx, by, _ = vertices[1]
    cx, cy, _ = vertices[2]
    area = abs((bx - ax) * (cy - ay) - (by - ay) * (cx - ax)) * 0.5
    xs = [vertex[0] for vertex in vertices]
    ys = [vertex[1] for vertex in vertices]
    zs = [vertex[2] for vertex in vertices]
    return {
        "packet": fields[offset],
        "owner": fields[offset + 1],
        "material": int(fields[offset + 2]),
        "clut": int(fields[offset + 3]),
        "texpage": int(fields[offset + 4]),
        "vertices": vertices,
        "area": area,
        "span_x": max(xs) - min(xs),
        "span_y": max(ys) - min(ys),
        "min_z": min(zs),
        "max_z": max(zs),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dump_dir", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--near", type=float, default=16.0)
    parser.add_argument("--width", type=float, default=427.0)
    parser.add_argument("--height", type=float, default=240.0)
    parser.add_argument("--top", type=int, default=100)
    args = parser.parse_args()

    screen_area = args.width * args.height
    records: list[dict[str, object]] = []
    owner_counts: Counter[str] = Counter()
    files = sorted(args.dump_dir.glob("recompone_geometry_frame*.txt"))
    for dump_path in files:
        frame_text = dump_path.stem.removeprefix("recompone_geometry_frame")
        frame = int(frame_text)
        for order, line in enumerate(
            dump_path.read_text(encoding="utf-8", errors="replace").splitlines()
        ):
            triangle = parse_triangle(line)
            if triangle is None:
                continue
            near = float(triangle["min_z"]) <= args.near + 0.01
            oversized = (
                float(triangle["span_x"]) > args.width * 1.5
                or float(triangle["span_y"]) > args.height * 1.5
                or float(triangle["area"]) > screen_area
            )
            if not near and not oversized:
                continue
            triangle["frame"] = frame
            triangle["order"] = order
            triangle["near"] = near
            triangle["oversized"] = oversized
            owner_counts[str(triangle["owner"])] += 1
            records.append(triangle)

    records.sort(
        key=lambda record: (
            float(record["area"]),
            float(record["span_x"]),
            float(record["span_y"]),
        ),
        reverse=True,
    )
    report = {
        "passed": bool(files),
        "dump_count": len(files),
        "near_plane": args.near,
        "screen": [args.width, args.height],
        "candidate_count": len(records),
        "owner_counts": [
            {"owner": owner, "count": count}
            for owner, count in owner_counts.most_common()
        ],
        "largest": records[: args.top],
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(
        f"{'PASS' if report['passed'] else 'FAIL'}: "
        f"dumps={len(files)} candidates={len(records)} "
        f"owners={len(owner_counts)} output={args.output}"
    )
    for record in records[:10]:
        print(
            f"frame={record['frame']} order={record['order']} "
            f"packet=0x{record['packet']} material={record['material']} "
            f"area={record['area']:.1f} span={record['span_x']:.1f}x"
            f"{record['span_y']:.1f} z={record['min_z']:.2f}.."
            f"{record['max_z']:.2f} owner={record['owner']}"
        )
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
