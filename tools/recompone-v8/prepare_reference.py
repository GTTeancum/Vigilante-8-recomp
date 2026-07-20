#!/usr/bin/env python3
"""Prepare RecompOne maps/config from the checked-in Vigilante 8 analysis."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


OVERLAY_BASE = "0x80100000"


def read_json(path: Path):
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def write_json(path: Path, value) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        json.dump(value, stream, indent=2)
        stream.write("\n")


def convert_inventory(source: Path, output: Path, address_min: int, address_max: int) -> int:
    entries = read_json(source)
    functions = []
    for entry in entries:
        address = int(entry["address"], 16)
        size = int(entry.get("size", 0))
        if not (address_min <= address < address_max) or size <= 0:
            continue
        functions.append(
            {
                "address": f"0x{address:08X}",
                "name": entry["name"],
                "size": size,
            }
        )
    functions.sort(key=lambda item: int(item["address"], 16))
    write_json(output, {"functions": functions, "labels": []})
    return len(functions)


def relative_posix(target: Path, start: Path) -> str:
    import os

    return Path(os.path.relpath(target, start)).as_posix()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cue", default="reference/assets/disc/Vigilante8.cue")
    parser.add_argument("--output", default="reference/generated")
    args = parser.parse_args()

    repo = Path(__file__).resolve().parents[2]
    output = (repo / args.output).resolve()
    maps = output / "function-maps"
    config_dir = output
    overlay_manifest = read_json(Path(__file__).with_name("overlay_paths.json"))

    counts = {}
    main_source = repo / "analysis" / "SLUS_005.10" / "functions.json"
    main_map = maps / "SLUS_005.10.json"
    counts["main"] = convert_inventory(main_source, main_map, 0x80000000, 0x80200000)

    overlays = []
    for name, path_info in overlay_manifest.items():
        source = repo / "analysis" / "dll" / name / "functions.json"
        if not source.exists():
            raise FileNotFoundError(f"missing overlay inventory: {source}")
        function_map = maps / f"{name}.json"
        counts[name] = convert_inventory(source, function_map, 0x80100000, 0x80200000)
        overlays.append(
            {
                "name": name,
                "funcMap": relative_posix(function_map, config_dir),
                "base": OVERLAY_BASE,
                "file": path_info["file"],
            }
        )

    cue = (repo / args.cue).resolve()
    config = {
        "game": {
            "id": "SLUS-00510",
            "name": "Vigilante8Reference",
            "output": "recompiled",
        },
        "cue": relative_posix(cue, config_dir),
        "funcMap": relative_posix(main_map, config_dir),
        "debug": False,
        "linearSweep": False,
        "overlays": overlays,
        "stubs": [],
        "ignored": [],
        "patches": [],
    }
    config_path = output / "v8.recompone.json"
    write_json(config_path, config)

    print(f"Wrote {config_path}")
    print(f"Main functions: {counts['main']}")
    print(f"Overlay functions: {sum(v for k, v in counts.items() if k != 'main')}")
    if not cue.exists():
        print(f"Assets not present yet (expected cue: {cue})")
    else:
        print(f"Cue found: {cue}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
