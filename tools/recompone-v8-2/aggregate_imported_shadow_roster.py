#!/usr/bin/env python3
"""Aggregate the per-vehicle imported-shadow acceptance reports.

The individual runs exercise the real imported vehicle package and renderer
path.  This aggregator makes the roster gate explicit: every converted V8
vehicle must enter gameplay and its decoded projected-shadow atlas must retain
authored intensity without producing binary opaque-card coverage.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path


VEHICLES = (
    "chassey_blue",
    "slick_clyde",
    "sheila",
    "john_torque",
    "dave",
    "convoy",
    "loki",
    "houston_3",
    "boogie",
    "beezwax",
    "molo",
    "sid_burn",
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--exe", type=Path)
    args = parser.parse_args()

    root = args.root.resolve()
    results = []
    for vehicle in VEHICLES:
        directory = root / vehicle
        summary_path = directory / "summary.json"
        shadow_path = directory / "shadow-verification.json"
        summary = (
            json.loads(summary_path.read_text(encoding="utf-8"))
            if summary_path.is_file()
            else {}
        )
        shadow = (
            json.loads(shadow_path.read_text(encoding="utf-8"))
            if shadow_path.is_file()
            else {}
        )
        runs = summary.get("runs", [])
        gameplay_passed = (
            len(runs) == 1
            and bool(runs[0].get("passed", False))
            and int(runs[0].get("last_frame", 0)) >= 180
        )
        checks = shadow.get("checks", {})
        shadow_passed = bool(shadow.get("passed", False)) and all(
            bool(checks.get(name, False))
            for name in (
                "shadow_regions_traced",
                "black_subtractive_texels_are_no_op",
                "authored_shadow_intensity_survives",
                "coverage_is_bounded",
                "binary_opaque_card_rejected",
            )
        )
        results.append(
            {
                "vehicle": vehicle,
                "gameplay_passed": gameplay_passed,
                "shadow_passed": shadow_passed,
                "shadow_regions": len(shadow.get("records", [])),
                "passed": gameplay_passed and shadow_passed,
                "summary": str(summary_path),
                "shadow_verification": str(shadow_path),
            }
        )

    report = {
        "schema": 1,
        "acceptance": (
            "all imported V8 vehicles retain authored projected-shadow "
            "intensity without an opaque shadow card"
        ),
        "passed": all(result["passed"] for result in results),
        "vehicles_passed": sum(result["passed"] for result in results),
        "vehicles_total": len(results),
        "executable": (
            str(args.exe.resolve()) if args.exe and args.exe.is_file() else None
        ),
        "executable_sha256": (
            sha256(args.exe.resolve())
            if args.exe and args.exe.is_file()
            else None
        ),
        "results": results,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2) + "\n",
        encoding="utf-8",
    )
    print(
        "[ImportedShadowRoster] "
        f"{'PASS' if report['passed'] else 'FAIL'} "
        f"{report['vehicles_passed']}/{report['vehicles_total']}"
    )
    for result in results:
        print(
            f"  {'PASS' if result['passed'] else 'FAIL'} "
            f"{result['vehicle']} regions={result['shadow_regions']}"
        )
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
