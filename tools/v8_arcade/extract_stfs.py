#!/usr/bin/env python3
"""List or extract files from the Vigilante 8 Arcade LIVE/STFS package.

The block mapping and directory-entry layout follow the BSD-licensed
CrownParkComputing/xbox360-ports extract_game.py implementation.  This version
adds path validation, manifests, and a list-only mode for repeatable analysis.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
from dataclasses import asdict, dataclass
from pathlib import Path, PurePosixPath
from typing import BinaryIO


BLOCK_SIZE = 0x1000
FILE_TABLE_OFFSET = 0xC000
FILE_TABLE_ENTRY_SIZE = 0x40
ROOT_PARENT = 0xFFFF
END_OF_CHAIN = 0xFFFFFF


@dataclass(frozen=True)
class Entry:
    index: int
    path: str
    is_directory: bool
    flags: int
    block_count: int
    start_block: int
    parent: int
    size: int


@dataclass(frozen=True)
class RawEntry:
    index: int
    name: str
    flags: int
    block_count: int
    start_block: int
    parent: int
    size: int

    @property
    def is_directory(self) -> bool:
        return (self.flags & 0x80) != 0


def read_uint24_le(data: bytes) -> int:
    if len(data) != 3:
        raise ValueError("expected three bytes")
    return data[0] | (data[1] << 8) | (data[2] << 16)


def read_uint24_be(data: bytes) -> int:
    if len(data) != 3:
        raise ValueError("expected three bytes")
    return (data[0] << 16) | (data[1] << 8) | data[2]


def physical_block(logical_block: int) -> int:
    """Map a type-1 STFS logical data block to its package block."""
    group = logical_block // 0xAA
    level1_groups = group // 0xAA
    level1_overhead = level1_groups + 1 if level1_groups > 0 else 0
    return logical_block + 0x0C + group + (1 if group > 0 else 0) + level1_overhead


def physical_offset(logical_block: int) -> int:
    return physical_block(logical_block) * BLOCK_SIZE


def hash_entry_offset(logical_block: int) -> int:
    group = logical_block // 0xAA
    index = logical_block % 0xAA
    level1_groups = group // 0xAA
    level1_overhead = level1_groups + 1 if level1_groups > 0 else 0
    table_block = 0x0B + (group * 0xAB) + (1 if group > 0 else 0) + level1_overhead
    return table_block * BLOCK_SIZE + index * 0x18


def next_block(source: BinaryIO, logical_block: int) -> int:
    source.seek(hash_entry_offset(logical_block) + 0x15)
    return read_uint24_be(source.read(3))


def parse_raw_entries(package: Path) -> list[RawEntry]:
    entries: list[RawEntry] = []
    with package.open("rb") as source:
        if source.read(4) != b"LIVE":
            raise RuntimeError(f"{package} is not an Xbox 360 LIVE/STFS package")

        source.seek(FILE_TABLE_OFFSET)
        index = 0
        while True:
            raw = source.read(FILE_TABLE_ENTRY_SIZE)
            if len(raw) != FILE_TABLE_ENTRY_SIZE or raw == bytes(FILE_TABLE_ENTRY_SIZE):
                break

            name_flags = raw[0x28]
            name_length = name_flags & 0x3F
            if name_length == 0:
                break

            name = raw[:name_length].decode("ascii", errors="strict")
            entries.append(
                RawEntry(
                    index=index,
                    name=name,
                    flags=name_flags,
                    block_count=read_uint24_le(raw[0x29:0x2C]),
                    start_block=read_uint24_le(raw[0x2F:0x32]),
                    parent=int.from_bytes(raw[0x32:0x34], "big"),
                    size=int.from_bytes(raw[0x34:0x38], "big"),
                )
            )
            index += 1

    if not entries:
        raise RuntimeError("the STFS file table was empty or unreadable")
    return entries


def resolve_entry_path(entry: RawEntry, entries: list[RawEntry]) -> PurePosixPath:
    parts = [entry.name]
    parent = entry.parent
    seen = {entry.index}

    while parent != ROOT_PARENT:
        if parent >= len(entries) or parent in seen:
            raise RuntimeError(f"invalid parent chain for entry {entry.index}: {entry.name}")
        parent_entry = entries[parent]
        parts.append(parent_entry.name)
        seen.add(parent)
        parent = parent_entry.parent

    path = PurePosixPath(*reversed(parts))
    if path.is_absolute() or ".." in path.parts:
        raise RuntimeError(f"unsafe STFS path: {path}")
    return path


def parse_entries(package: Path) -> tuple[list[RawEntry], list[Entry]]:
    raw_entries = parse_raw_entries(package)
    entries = [
        Entry(
            index=entry.index,
            path=resolve_entry_path(entry, raw_entries).as_posix(),
            is_directory=entry.is_directory,
            flags=entry.flags,
            block_count=entry.block_count,
            start_block=entry.start_block,
            parent=entry.parent,
            size=entry.size,
        )
        for entry in raw_entries
    ]
    return raw_entries, entries


def extract_file(package: Path, entry: Entry, destination: Path) -> str:
    destination.parent.mkdir(parents=True, exist_ok=True)
    remaining = entry.size
    blocks_to_copy = max(entry.block_count, math.ceil(entry.size / BLOCK_SIZE))
    logical_block = entry.start_block
    digest = hashlib.sha256()

    with package.open("rb") as source, destination.open("wb") as output:
        for block_index in range(blocks_to_copy):
            if remaining <= 0:
                break
            if logical_block == END_OF_CHAIN:
                raise RuntimeError(f"unexpected block-chain end while extracting {entry.path}")

            source.seek(physical_offset(logical_block))
            chunk = source.read(min(BLOCK_SIZE, remaining))
            if not chunk:
                raise RuntimeError(f"unexpected package EOF while extracting {entry.path}")
            output.write(chunk)
            digest.update(chunk)
            remaining -= len(chunk)

            if block_index + 1 < blocks_to_copy:
                logical_block = next_block(source, logical_block)

    if remaining != 0:
        raise RuntimeError(f"{entry.path} is short by {remaining} bytes")
    return digest.hexdigest().upper()


def package_sha256(package: Path) -> str:
    digest = hashlib.sha256()
    with package.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("package", type=Path, help="Path to a LIVE/STFS package")
    parser.add_argument("--out", type=Path, help="Extraction destination")
    parser.add_argument("--list-only", action="store_true", help="List entries without extracting")
    parser.add_argument("--manifest", type=Path, help="Write a JSON inventory")
    args = parser.parse_args()

    package = args.package.resolve()
    _, entries = parse_entries(package)

    if args.list_only or args.out is None:
        for entry in entries:
            kind = "dir " if entry.is_directory else "file"
            print(f"{kind} {entry.size:10d}  {entry.path}")

    extracted_hashes: dict[str, str] = {}
    if args.out is not None and not args.list_only:
        output_root = args.out.resolve()
        output_root.mkdir(parents=True, exist_ok=True)
        for entry in entries:
            destination = output_root.joinpath(*PurePosixPath(entry.path).parts)
            if entry.is_directory:
                destination.mkdir(parents=True, exist_ok=True)
            else:
                extracted_hashes[entry.path] = extract_file(package, entry, destination)
        print(f"Extracted {len(extracted_hashes)} files to {output_root}")

    if args.manifest:
        manifest_path = args.manifest.resolve()
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        payload = {
            "format": "Xbox 360 LIVE/STFS",
            "package": str(package),
            "package_size": package.stat().st_size,
            "package_sha256": package_sha256(package),
            "entries": [
                {
                    **asdict(entry),
                    **(
                        {"sha256": extracted_hashes[entry.path]}
                        if entry.path in extracted_hashes
                        else {}
                    ),
                }
                for entry in entries
            ],
        }
        manifest_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
        print(f"Wrote {manifest_path}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
