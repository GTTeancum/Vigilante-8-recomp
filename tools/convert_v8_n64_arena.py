#!/usr/bin/env python3
"""Convert one retail Vigilante 8 N64 arena to native PS1 EXP data."""

from __future__ import annotations

import argparse
from dataclasses import asdict
import json
from pathlib import Path

from v8_n64_level import (
    V8N64Rom,
    convert_arena,
    overlay_exports,
    psx_executable_exports,
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("arena", help="archive stem such as DREAMLND")
    parser.add_argument("output", type=Path)
    parser.add_argument("--report", type=Path)
    parser.add_argument(
        "--runtime-executable",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "PS1 game" / "SLUS_005.10",
        help="target PS-X EXE supplying resident object callbacks",
    )
    parser.add_argument(
        "--decoded-source",
        type=Path,
        help="use an existing decoded arena EXP instead of extracting the ROM",
    )
    parser.add_argument(
        "--source-output",
        type=Path,
        help="optionally retain the decoded N64 EXP before conversion",
    )
    args = parser.parse_args()

    arena = args.arena.upper()
    rom = V8N64Rom(args.rom)
    source = (
        args.decoded_source.read_bytes()
        if args.decoded_source is not None
        else rom.decoded(f"{arena}.EXP")
    )
    if args.source_output is not None:
        args.source_output.parent.mkdir(parents=True, exist_ok=True)
        args.source_output.write_bytes(source)
    callbacks = frozenset(
        item.name
        for item in (
            *overlay_exports(rom.decoded(f"{arena}.DLL")),
            *psx_executable_exports(args.runtime_executable.read_bytes()),
        )
    )
    converted, report = convert_arena(
        source, arena, object_callback_names=callbacks
    )
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
            f"slots={model.slots} rooted-variants={model.rooted_variants} "
            f"terrain-clipped={model.terrain_clipped_faces} "
            f"terrain-discarded={model.terrain_discarded_faces}"
        )
    print(args.output)
    print(report_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
