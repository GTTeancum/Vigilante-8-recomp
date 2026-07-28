#!/usr/bin/env python3
"""Extract a ZFS archive directly from a raw-sector ISO9660 disc image."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

from extract_psx_iso import (
    MODE1_DATA_OFF,
    MODE2_FORM1_DATA_OFF,
    PsxIso,
    SECTOR_DATA,
    SECTOR_RAW,
    parse_dir_records,
)
from i76_zfs_extract import extract_archive_bytes


def detect_data_offset(track_path: Path) -> int:
    with track_path.open("rb") as fp:
        for candidate in (MODE1_DATA_OFF, MODE2_FORM1_DATA_OFF):
            fp.seek(16 * SECTOR_RAW + candidate)
            pvd = fp.read(SECTOR_DATA)
            if pvd[1:6] == b"CD001":
                return candidate
    raise ValueError("primary volume descriptor not found at LBA 16")


def find_iso_file(iso: PsxIso, wanted_path: str) -> tuple[int, int]:
    pvd = iso.read_sector(16)
    root = pvd[156 : 156 + pvd[156]]
    extent = struct.unpack_from("<I", root, 2)[0]
    size = struct.unpack_from("<I", root, 10)[0]
    parts = [part.upper() for part in Path(wanted_path).parts]

    for index, wanted in enumerate(parts):
        records = {
            name.upper(): (child_extent, child_size, flags)
            for name, child_extent, child_size, flags
            in parse_dir_records(iso.read_extent(extent, size))
            if name not in (".", "..")
        }
        if wanted not in records:
            raise FileNotFoundError(
                f"{wanted_path}: component {wanted!r} not found"
            )
        extent, size, flags = records[wanted]
        is_last = index == len(parts) - 1
        if is_last and flags & 0x02:
            raise IsADirectoryError(wanted_path)
        if not is_last and not flags & 0x02:
            raise NotADirectoryError(wanted)
    return extent, size


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("disc_image", type=Path)
    parser.add_argument("iso_path")
    parser.add_argument("output", type=Path)
    parser.add_argument(
        "--match",
        action="append",
        help="Extract only matching case-insensitive filename globs.",
    )
    args = parser.parse_args()

    data_offset = detect_data_offset(args.disc_image)
    iso = PsxIso(args.disc_image, data_offset)
    try:
        extent, size = find_iso_file(iso, args.iso_path)
        archive = iso.read_extent(extent, size)
    finally:
        iso.close()

    source = (
        f"{args.disc_image.resolve()}::{args.iso_path} "
        f"(extent={extent}, size={size})"
    )
    manifest = extract_archive_bytes(
        archive,
        source,
        args.output,
        args.match,
    )
    extraction = manifest["extraction"]
    print(
        f"Extracted {extraction['extracted_file_count']} files "
        f"({extraction['extracted_payload_bytes']} payload bytes) to "
        f"{extraction['output']}"
    )
    print(
        f"Archive SHA256: {manifest['archive_sha256']}; "
        f"vehicle .vdf files: {extraction['vehicle_model_vdf_count']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
