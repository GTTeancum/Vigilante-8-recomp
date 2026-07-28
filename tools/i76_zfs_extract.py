#!/usr/bin/env python3
"""Extract an Interstate '76 ZFSF archive without modifying the source."""

from __future__ import annotations

import argparse
import fnmatch
import hashlib
import json
import os
import struct
from collections import Counter
from dataclasses import dataclass
from pathlib import Path


HEADER = struct.Struct("<IIIII4sI")
ENTRY_TRAILER = struct.Struct("<IIIII")
MAGIC = b"ZFSF"


@dataclass(frozen=True)
class Entry:
    name: str
    offset: int
    entry_id: int
    size: int
    mtime: int
    unknown: int


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def safe_leaf_name(raw_name: bytes) -> str:
    name = raw_name.split(b"\0", 1)[0].decode("ascii", errors="strict")
    if not name or name in {".", ".."}:
        raise ValueError(f"invalid empty or relative archive name: {name!r}")
    if Path(name).name != name or "/" in name or "\\" in name:
        raise ValueError(f"archive entry is not a leaf filename: {name!r}")
    return name


def parse_entries(
    archive: bytes,
    name_length: int,
    files_per_page: int,
    file_count: int,
    first_page_offset: int,
) -> list[Entry]:
    if not 1 <= name_length <= 255:
        raise ValueError(f"invalid name length: {name_length}")
    if not 1 <= files_per_page <= 1_000_000:
        raise ValueError(f"invalid files-per-page value: {files_per_page}")
    if first_page_offset < HEADER.size or first_page_offset >= len(archive):
        raise ValueError(f"invalid first page offset: {first_page_offset:#x}")

    entry_size = name_length + ENTRY_TRAILER.size
    page_offset = first_page_offset
    entries: list[Entry] = []
    visited_pages: set[int] = set()

    while len(entries) < file_count:
        if page_offset in visited_pages:
            raise ValueError(f"page chain loop at {page_offset:#x}")
        visited_pages.add(page_offset)
        if page_offset + 4 > len(archive):
            raise ValueError(f"page pointer outside archive at {page_offset:#x}")

        next_page_offset = struct.unpack_from("<I", archive, page_offset)[0]
        count_on_page = min(files_per_page, file_count - len(entries))
        table_end = page_offset + 4 + count_on_page * entry_size
        if table_end > len(archive):
            raise ValueError(f"entry table outside archive at {page_offset:#x}")

        for index in range(count_on_page):
            record_offset = page_offset + 4 + index * entry_size
            name = safe_leaf_name(
                archive[record_offset : record_offset + name_length]
            )
            values = ENTRY_TRAILER.unpack_from(
                archive, record_offset + name_length
            )
            entry = Entry(name, *values)
            if entry.offset < table_end:
                raise ValueError(
                    f"{entry.name}: data offset {entry.offset:#x} overlaps page table"
                )
            if entry.offset + entry.size > len(archive):
                raise ValueError(
                    f"{entry.name}: data range extends beyond the archive"
                )
            entries.append(entry)

        if len(entries) < file_count:
            if next_page_offset == 0:
                raise ValueError("page chain ended before the declared file count")
            page_offset = next_page_offset
        elif next_page_offset != 0:
            raise ValueError("final page has a nonzero next-page pointer")

    ids = [entry.entry_id for entry in entries]
    if ids != list(range(file_count)):
        raise ValueError("entry IDs are not the expected contiguous sequence")
    folded_names = [entry.name.casefold() for entry in entries]
    if len(set(folded_names)) != len(folded_names):
        raise ValueError("archive has case-insensitive duplicate filenames")
    return entries


def extract_archive_bytes(
    archive: bytes,
    source: str,
    output_dir: Path,
    include_patterns: list[str] | None = None,
) -> dict[str, object]:
    output_dir = output_dir.resolve()
    if len(archive) < HEADER.size:
        raise ValueError("archive is shorter than the ZFSF header")

    (
        magic_value,
        version,
        name_length,
        files_per_page,
        file_count,
        xor_key,
        first_page_offset,
    ) = HEADER.unpack_from(archive)
    magic = magic_value.to_bytes(4, "little")
    if magic != MAGIC:
        raise ValueError(f"unexpected archive magic {magic!r}")
    if any(xor_key):
        raise ValueError(
            "this extractor deliberately refuses nonzero XOR keys until that "
            "dialect is verified against a retail archive"
        )

    archive_entries = parse_entries(
        archive,
        name_length,
        files_per_page,
        file_count,
        first_page_offset,
    )
    entries = (
        [
            entry
            for entry in archive_entries
            if any(
                fnmatch.fnmatch(entry.name.lower(), pattern.lower())
                for pattern in include_patterns
            )
        ]
        if include_patterns
        else archive_entries
    )
    if output_dir.exists() and any(output_dir.iterdir()):
        raise FileExistsError(f"output directory is not empty: {output_dir}")
    output_dir.mkdir(parents=True, exist_ok=True)

    manifest_entries: list[dict[str, object]] = []
    extension_counts: Counter[str] = Counter()
    extracted_bytes = 0
    for entry in entries:
        payload = archive[entry.offset : entry.offset + entry.size]
        destination = output_dir / entry.name
        destination.write_bytes(payload)
        os.utime(destination, (entry.mtime, entry.mtime))
        written = destination.read_bytes()
        if written != payload:
            raise OSError(f"post-write verification failed for {destination}")
        digest = sha256_bytes(payload)
        extension_counts[destination.suffix.lower() or "<none>"] += 1
        extracted_bytes += len(payload)
        manifest_entries.append(
            {
                "id": entry.entry_id,
                "name": entry.name,
                "offset": entry.offset,
                "size": entry.size,
                "mtime_unix": entry.mtime,
                "unknown": entry.unknown,
                "sha256": digest,
            }
        )

    vehicle_models = [
        item
        for item in manifest_entries
        if str(item["name"]).lower().endswith(".vdf")
    ]
    manifest: dict[str, object] = {
        "format": "Activision Z-engine ZFSF",
        "source": source,
        "archive_size": len(archive),
        "archive_sha256": sha256_bytes(archive),
        "header": {
            "magic": magic.decode("ascii"),
            "version": version,
            "name_length": name_length,
            "files_per_page": files_per_page,
            "file_count": file_count,
            "xor_key_hex": xor_key.hex().upper(),
            "first_page_offset": first_page_offset,
        },
        "extraction": {
            "output": str(output_dir),
            "extracted_file_count": len(entries),
            "extracted_payload_bytes": extracted_bytes,
            "extension_counts": dict(sorted(extension_counts.items())),
            "vehicle_model_vdf_count": len(vehicle_models),
            "vehicle_model_vdf_names": [item["name"] for item in vehicle_models],
            "verified_byte_for_byte_after_write": True,
            "selection_patterns": include_patterns or [],
        },
        "entries": manifest_entries,
    }
    manifest_path = output_dir / "_i76_zfs_manifest.json"
    manifest_path.write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8", newline="\n"
    )
    return manifest


def extract(archive_path: Path, output_dir: Path) -> dict[str, object]:
    archive_path = archive_path.resolve()
    return extract_archive_bytes(
        archive_path.read_bytes(),
        str(archive_path),
        output_dir,
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path, help="source I76.ZFS path")
    parser.add_argument("output", type=Path, help="new or empty output directory")
    args = parser.parse_args()
    manifest = extract(args.archive, args.output)
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
