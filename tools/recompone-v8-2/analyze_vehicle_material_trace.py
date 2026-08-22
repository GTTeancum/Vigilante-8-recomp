#!/usr/bin/env python3
"""Summarize packet-level vehicle material traces without visual guessing."""

from __future__ import annotations

import argparse
import collections
import json
import re
from pathlib import Path


LINE = re.compile(
    r"\[VehicleMaterialTriangle\] "
    r"gameplay=(?P<gameplay>[01]) "
    r"(?:frame=(?P<frame>\d+) tick=(?P<tick>-?\d+) )?"
    r"packet=(?P<packet>\S+) ot=(?P<ot>\d+) "
    r"owner=\"(?P<owner>[^\"]*)\" "
    r"material=(?P<material>\w+) tex=(?P<textured>[01]) "
    r"(?:semi=(?P<semi>[01]) )?"
    r"raw=(?P<raw>[01]) "
    r"tpage=(?P<tpage>\S+) clut=(?P<clut>\S+) "
    r"(?:tw=(?P<texture_window>\S+) )?"
    r"(?:ps1-zero-key=(?P<ps1_zero_key>\S+) )?"
    r"xy=(?P<xy>.*?) uv=(?P<uv>.*?) rgb=(?P<rgb>.*?) "
    r"gte=(?P<gte>.*)$"
)
POINT = re.compile(
    r"\((-?\d+(?:\.\d+)?),(-?\d+(?:\.\d+)?)\)"
)


def parse_probe(value: str) -> tuple[float, float]:
    fields = value.split(",")
    if len(fields) != 2:
        raise argparse.ArgumentTypeError("probe must be X,Y")
    return float(fields[0]), float(fields[1])


def parse_triangle(value: str) -> list[tuple[float, float]]:
    return [
        (float(match.group(1)), float(match.group(2)))
        for match in POINT.finditer(value)
    ]


def point_in_triangle(
    point: tuple[float, float],
    triangle: list[tuple[float, float]],
) -> bool:
    if len(triangle) != 3:
        return False
    px, py = point
    (ax, ay), (bx, by), (cx, cy) = triangle
    denominator = (by - cy) * (ax - cx) + (cx - bx) * (ay - cy)
    if abs(denominator) < 1e-7:
        return False
    alpha = ((by - cy) * (px - cx) + (cx - bx) * (py - cy))
    beta = ((cy - ay) * (px - cx) + (ax - cx) * (py - cy))
    if denominator < 0:
        denominator = -denominator
        alpha = -alpha
        beta = -beta
    gamma = denominator - alpha - beta
    return alpha >= 0 and beta >= 0 and gamma >= 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument(
        "--require-reflection",
        action="store_true",
        help="fail unless native vehicle-reflection packets were observed",
    )
    parser.add_argument(
        "--probe",
        type=parse_probe,
        action="append",
        default=[],
        metavar="X,Y",
        help=(
            "report vehicle triangles covering a native framebuffer point; "
            "repeat for multiple probes"
        ),
    )
    args = parser.parse_args()

    records: list[dict[str, object]] = []
    signatures: collections.Counter[tuple[object, ...]] = (
        collections.Counter()
    )
    for raw_line in args.trace.read_text(
        encoding="utf-8", errors="replace"
    ).splitlines():
        match = LINE.search(raw_line)
        if match is None:
            continue
        row = match.groupdict()
        signature = (
            int(row["gameplay"]),
            row["material"],
            int(row["textured"]),
            int(row["semi"]) if row["semi"] is not None else None,
            int(row["raw"]),
            row["tpage"],
            row["clut"],
            row["uv"],
            row["rgb"],
        )
        signatures[signature] += 1
        records.append(
            {
                "gameplay": int(row["gameplay"]),
                "frame": (
                    int(row["frame"])
                    if row["frame"] is not None
                    else None
                ),
                "tick": (
                    int(row["tick"])
                    if row["tick"] is not None
                    else None
                ),
                "packet": row["packet"],
                "ot": int(row["ot"]),
                "owner": row["owner"],
                "material": row["material"],
                "textured": bool(int(row["textured"])),
                "semi_transparent": (
                    bool(int(row["semi"]))
                    if row["semi"] is not None
                    else None
                ),
                "raw_texture": bool(int(row["raw"])),
                "tpage": row["tpage"],
                "clut": row["clut"],
                "texture_window": row["texture_window"],
                "ps1_zero_key": row["ps1_zero_key"],
                "xy": row["xy"],
                "uv": row["uv"],
                "rgb": row["rgb"],
                "gte": row["gte"],
            }
        )

    grouped = [
        {
            "count": count,
            "gameplay": key[0],
            "material": key[1],
            "textured": bool(key[2]),
            "semi_transparent": (
                bool(key[3]) if key[3] is not None else None
            ),
            "raw_texture": bool(key[4]),
            "tpage": key[5],
            "clut": key[6],
            "uv": key[7],
            "rgb": key[8],
        }
        for key, count in signatures.most_common()
    ]
    misclassified_subtractive = [
        item for item in grouped
        if item["material"] == "Glass"
        and ((int(str(item["tpage"]), 16) >> 5) & 3) == 2
    ]
    forbidden_imported_materials = [
        item for item in grouped
        if str(item["material"]).startswith("Imported")
    ]
    reflections = [
        item for item in grouped
        if item["material"] == "VehicleReflection"
    ]
    probe_hits = []
    for probe_x, probe_y in args.probe:
        hits = [
            record for record in records
            if point_in_triangle(
                (probe_x, probe_y),
                parse_triangle(str(record["xy"])),
            )
        ]
        probe_hits.append(
            {
                "probe": [probe_x, probe_y],
                "hit_count": len(hits),
                "hits": hits,
            }
        )
    report = {
        "schema": 2,
        "trace": str(args.trace.resolve()),
        "triangle_count": len(records),
        "signature_count": len(grouped),
        "checks": {
            "has_vehicle_material_triangles": bool(records),
            "no_subtractive_effect_classified_as_glass":
                not misclassified_subtractive,
            "no_imported_only_materials": not forbidden_imported_materials,
            "required_native_reflection_observed":
                bool(reflections) or not args.require_reflection,
        },
        "native_reflection_signatures": len(reflections),
        "passed": (
            bool(records)
            and not misclassified_subtractive
            and not forbidden_imported_materials
            and (bool(reflections) or not args.require_reflection)
        ),
        "forbidden_imported_material_signatures":
            forbidden_imported_materials,
        "misclassified_subtractive_signatures":
            misclassified_subtractive,
        "probe_hits": probe_hits,
        "signatures": grouped,
        "first_triangles": records[:32],
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    print(
        f"[VehicleMaterialTrace] triangles={len(records)} "
        f"signatures={len(grouped)}"
    )
    for item in grouped[:20]:
        print(
            f"  {item['count']:5d} gameplay={item['gameplay']} "
            f"{item['material']} tex={int(item['textured'])} "
            f"semi={item['semi_transparent']} "
            f"tpage={item['tpage']} clut={item['clut']} "
            f"uv={item['uv']}"
        )
    for item in probe_hits:
        print(
            f"  probe={item['probe']} hits={item['hit_count']}"
        )
        for hit in item["hits"][:20]:
            print(
                f"    frame={hit['frame']} tick={hit['tick']} "
                f"packet={hit['packet']} material={hit['material']} "
                f"semi={hit['semi_transparent']} "
                f"tpage={hit['tpage']} clut={hit['clut']} "
                f"xy={hit['xy']} uv={hit['uv']}"
            )
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
