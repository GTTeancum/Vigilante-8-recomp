#!/usr/bin/env python3
"""Validate native V8:2 material use for every converted roster vehicle."""

from __future__ import annotations

import argparse
from collections import Counter
import json
from pathlib import Path
import re


CREATED = re.compile(
    r"\[V82Vehicles\] created (?P<stable_id>\S+) .* "
    r"object=(?P<object>0x[0-9A-Fa-f]+)"
)
MATERIAL = re.compile(
    r"\[VehicleMaterialTriangle\].* "
    r"owner=\"v82-vehicle-object=(?P<object>0x[0-9A-Fa-f]+)\" "
    r"material=(?P<material>\w+)"
)
ERROR = re.compile(
    r"fatal|unmapped address|malformed geometry|"
    r"collision-stream rejected|rejected malformed hierarchy",
    re.IGNORECASE,
)
MEDIA_SUFFIXES = {
    ".bmp", ".gif", ".jpeg", ".jpg", ".png", ".ppm", ".webp"
}


def analyze_run(directory: Path) -> dict[str, object]:
    summary_path = directory / "summary.json"
    if not summary_path.is_file():
        raise FileNotFoundError(f"missing soak summary: {summary_path}")
    summary = json.loads(summary_path.read_text(encoding="utf-8"))
    runs = summary.get("runs", [])
    if len(runs) != 1:
        raise ValueError(f"expected one run in {summary_path}, got {len(runs)}")
    run = runs[0]
    runtime_path = Path(run["runtime_log"])
    text = runtime_path.read_text(encoding="utf-8", errors="replace")

    created = CREATED.search(text)
    stable_id = created.group("stable_id") if created else "unresolved"
    object_address = created.group("object").upper() if created else ""
    materials: Counter[str] = Counter()
    all_materials: Counter[str] = Counter()
    for match in MATERIAL.finditer(text):
        material = match.group("material")
        all_materials[material] += 1
        if match.group("object").upper() == object_address:
            materials[material] += 1

    forbidden = {
        material: count
        for material, count in all_materials.items()
        if material.startswith("Imported")
    }
    error_hits = [
        line for line in text.splitlines()
        if ERROR.search(line)
    ]
    media = [
        str(path)
        for path in directory.rglob("*")
        if path.is_file() and path.suffix.lower() in MEDIA_SUFFIXES
    ]
    checks = {
        "soak_passed": bool(run.get("passed")),
        "guest_object_resolved": stable_id != "unresolved",
        "guest_materials_observed": sum(materials.values()) > 0,
        "guest_native_reflection_observed":
            materials["VehicleReflection"] > 0,
        "no_imported_only_materials": not forbidden,
        "no_runtime_error_tokens": not error_hits,
        "text_only_no_media": not media,
    }
    return {
        "stable_id": stable_id,
        "directory": str(directory.resolve()),
        "object": object_address,
        "last_frame": run.get("last_frame"),
        "materials": dict(materials.most_common()),
        "forbidden_materials": forbidden,
        "error_hits": error_hits[:32],
        "media": media,
        "checks": checks,
        "passed": all(checks.values()),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--expected", type=int, default=12)
    args = parser.parse_args()

    root = args.root.resolve()
    results = [
        analyze_run(directory)
        for directory in sorted(root.iterdir())
        if directory.is_dir() and (directory / "summary.json").is_file()
    ]
    checks = {
        "expected_vehicle_count": len(results) == args.expected,
        "stable_ids_unique":
            len({item["stable_id"] for item in results}) == len(results),
        "all_vehicle_runs_passed": all(item["passed"] for item in results),
    }
    report = {
        "schema": 1,
        "root": str(root),
        "vehicle_count": len(results),
        "checks": checks,
        "passed": all(checks.values()),
        "vehicles": results,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    print(
        f"[NativeVehicleRoster] vehicles={len(results)} "
        f"passed={sum(bool(item['passed']) for item in results)}"
    )
    for item in results:
        print(
            f"  {item['stable_id']} object={item['object']} "
            f"reflection={item['materials'].get('VehicleReflection', 0)} "
            f"triangles={sum(item['materials'].values())} "
            f"passed={item['passed']}"
        )
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
