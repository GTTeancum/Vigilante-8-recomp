#!/usr/bin/env python3
"""Convert one retail Vigilante 8 N64 arena to native PS1 EXP data."""

from __future__ import annotations

import argparse
from dataclasses import asdict
import json
from pathlib import Path

from v8_n64_level import V8N64Rom, convert_arena


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("arena", help="archive stem such as DREAMLND")
    parser.add_argument("output", type=Path)
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()

    arena = args.arena.upper()
    rom = V8N64Rom(args.rom)
    source = rom.decoded(f"{arena}.EXP")
    converted, report = convert_arena(source, arena)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(converted)
    report_path = args.report or args.output.with_suffix(".json")
    report_path.write_text(
        json.dumps(asdict(report), indent=2) + "\n", encoding="utf-8"
    )
    print(
        f"{arena}: {len(source)} N64 bytes -> {len(converted)} PS1 bytes; "
        f"XOBF={len(report.xobf)} zones={report.zones} objects={report.objects}"
    )
    for index, model in enumerate(report.xobf):
        print(
            f"  bank {index}: groups={model.groups} faces={model.faces} "
            f"textures={model.textures} collisions={model.collisions} "
            f"slots={model.slots}"
        )
    print(args.output)
    print(report_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
