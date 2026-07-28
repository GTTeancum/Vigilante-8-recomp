#!/usr/bin/env python3
"""Compile strict JSON vehicle projects into native donor-free packages."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from vigilante8_vehicle_tools import project, registry


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "projects",
        nargs="+",
        type=Path,
        help="schema-v2 vehicle project JSON files",
    )
    parser.add_argument(
        "--output",
        required=True,
        type=Path,
        help="directory for CUSTOM.EXP and VEHICLES.V8R",
    )
    args = parser.parse_args()

    vehicles = []
    for path in args.projects:
        value = json.loads(path.read_text(encoding="utf-8"))
        vehicles.append(project.VehicleProject.from_dict(value))
    package = registry.compile_package(vehicles)

    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "CUSTOM.EXP").write_bytes(package.archive)
    (args.output / "VEHICLES.V8R").write_bytes(package.registry)
    print(
        f"compiled {len(vehicles)} {package.game} vehicle(s): "
        f"CUSTOM.EXP={len(package.archive)} bytes, "
        f"VEHICLES.V8R={len(package.registry)} bytes"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
