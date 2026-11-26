#!/usr/bin/env python3
"""Build a private, updated pair of V8 roster files without Blender or a full asset rebuild.

Use the installed guest roster and your original V8 USA disc data track or HUD/ARMS.EXP.
Nothing in the input folder changes. Use the output pair together with the new host.
"""
from __future__ import annotations

import argparse
from contextlib import closing
from dataclasses import replace
from pathlib import Path
import re
import sys
import tempfile

# The same source runs in a checkout and as the allowlisted zipapp.
if Path(__file__).parent.name == "recompone-v8-2":
    tools_dir = Path(__file__).resolve().parents[1]
    sys.path[:0] = [str(tools_dir), str(tools_dir / "blender_addons")]

from extract_psx_iso import MODE2_FORM1_DATA_OFF, PsxIso, parse_dir_records
from vigilante8_vehicle_tools import registry
from vigilante8_vehicle_tools.guest_audio import original_engine_sounds


def original_arms_from_disc(path: Path) -> bytes:
    # Read only the two required file extents and the directories containing
    # them. No full-disc extraction and no dependencies on audio tracks.
    with closing(PsxIso(path, MODE2_FORM1_DATA_OFF)) as disc:
        pvd = disc.read_sector(16)
        if pvd[:7] != b"\x01CD001\x01":
            raise ValueError("expected the original USA game's 2352-byte Track 01 BIN")

        def lookup(directory: bytes, name: str, *, is_dir: bool, limit: int) -> bytes:
            entries = [(lba, size) for entry, lba, size, flags in parse_dir_records(directory)
                       if entry.upper() == name and bool(flags & 2) == is_dir]
            if len(entries) != 1:
                raise ValueError(f"disc does not uniquely contain {name}")
            lba, size = entries[0]
            if not 0 < size <= limit:
                raise ValueError(f"unexpected size of {name}")
            return disc.read_extent(lba, size)

        root = lookup(pvd[156:156 + pvd[156]], ".", is_dir=True, limit=1024 * 1024)
        system = lookup(root, "SYSTEM.CNF", is_dir=False, limit=4096)
        if not re.search(rb"(?im)^\s*BOOT\s*=\s*cdrom:[/\\]*SLUS_005\.10;1\s*$", system):
            raise ValueError("wrong disc: expected original Vigilante 8 USA (SLUS_005.10), not the sequel")
        hud = lookup(root, "HUD", is_dir=True, limit=1024 * 1024)
        return lookup(hud, "ARMS.EXP", is_dir=False, limit=4 * 1024 * 1024)


def upgrade(archive: bytes, records: bytes, arms: bytes) -> tuple[bytes, bytes]:
    engines = original_engine_sounds(arms)
    old = registry.decompile_package(archive, records)
    if not any(vehicle.game == "V8_2" and vehicle.stable_id in engines for vehicle in old):
        raise ValueError("no original V8 drivers in this V8:2 package")
    # Refuse to silently canonicalize a third-party archive containing edits
    # the authoring tools cannot round trip losslessly.
    baseline = registry.compile_package(old)
    if (baseline.archive, baseline.registry) != (archive, records):
        raise ValueError("input is not a byte-exact authoring round trip; keep the installed files")
    updated = []
    for vehicle in old:
        engine = engines.get(vehicle.stable_id)
        if vehicle.game == "V8_2" and engine is not None:
            if vehicle.ground_engine_sample:
                if vehicle.sounds[vehicle.ground_engine_sample] != engine:
                    raise ValueError(f"{vehicle.stable_id} already declares a different engine")
            elif not vehicle.sounds or vehicle.sounds[0] != engine:
                vehicle = replace(vehicle, ground_engine_sample=len(vehicle.sounds),
                                  sounds=vehicle.sounds + (engine,))
        updated.append(vehicle)
    result = registry.compile_package(updated)
    return result.archive, result.registry


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package", type=Path, required=True, help="original guest roster directory")
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--arms", type=Path, help="first game's extracted HUD/ARMS.EXP")
    source.add_argument("--disc-bin", type=Path, help="original Vigilante 8 USA (or Rev 1) Track 01 BIN")
    parser.add_argument("--output", type=Path, required=True, help="new, nonexistent directory")
    args = parser.parse_args()
    if args.output.resolve().is_relative_to(args.package.resolve()):
        parser.error("--output must be outside the input package")
    if args.output.exists():
        parser.error("--output must not already exist; the input package is never overwritten")
    try:
        arms = original_arms_from_disc(args.disc_bin) if args.disc_bin else args.arms.read_bytes()
        data, records = upgrade((args.package / "CUSTOM.EXP").read_bytes(),
                                (args.package / "VEHICLES.V8R").read_bytes(), arms)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=args.output.parent) as tmp:
            pair = Path(tmp) / "pair"
            pair.mkdir()
            (pair / "CUSTOM.EXP").write_bytes(data)
            (pair / "VEHICLES.V8R").write_bytes(records)
            pair.rename(args.output)
    except (OSError, ValueError) as error:
        parser.exit(1, f"Roster not updated: {error}\n")
    print(f"Updated roster pair: {args.output} (source files unchanged)")


if __name__ == "__main__":
    main()
