#!/usr/bin/env python3
"""Decompress selected LZO entries from an extracted Activision ZFS archive."""

from __future__ import annotations

import argparse
import fnmatch
import json
import struct
import subprocess
import tempfile
from pathlib import Path


HEADER = struct.Struct("<IIIII4sI")
ENTRY_TRAILER = struct.Struct("<IIIII")
FILES_PER_PAGE = 100
NAME_LENGTH = 16
ENTRY_SIZE = NAME_LENGTH + ENTRY_TRAILER.size
PAGE_HEADER_SIZE = HEADER.size + 4
DATA_OFFSET = PAGE_HEADER_SIZE + FILES_PER_PAGE * ENTRY_SIZE


def build_batch_archive(
    asset_dir: Path,
    entries: list[dict[str, object]],
    archive_path: Path,
) -> None:
    payloads = [
        (asset_dir / str(entry["name"])).read_bytes()
        for entry in entries
    ]
    header = HEADER.pack(
        int.from_bytes(b"ZFSF", "little"),
        1,
        NAME_LENGTH,
        FILES_PER_PAGE,
        len(entries),
        b"\0\0\0\0",
        HEADER.size,
    )
    table = bytearray(FILES_PER_PAGE * ENTRY_SIZE)
    data_offset = DATA_OFFSET
    for index, (entry, payload) in enumerate(zip(entries, payloads)):
        name = str(entry["name"]).encode("ascii")
        if len(name) >= NAME_LENGTH:
            raise ValueError(f"archive name is too long: {entry['name']}")
        record_offset = index * ENTRY_SIZE
        table[record_offset : record_offset + len(name)] = name
        ENTRY_TRAILER.pack_into(
            table,
            record_offset + NAME_LENGTH,
            data_offset,
            index,
            len(payload),
            int(entry["mtime_unix"]),
            int(entry["unknown"]),
        )
        data_offset += len(payload)
    archive_path.write_bytes(
        header
        + struct.pack("<I", 0)
        + bytes(table)
        + b"".join(payloads)
        + struct.pack("<I", 0)
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("asset_dir", type=Path)
    parser.add_argument("unzfs_exe", type=Path)
    parser.add_argument(
        "--match",
        action="append",
        required=True,
        help="Case-insensitive filename glob; may be repeated.",
    )
    args = parser.parse_args()

    asset_dir = args.asset_dir.resolve()
    executable = args.unzfs_exe.resolve()
    manifest_path = asset_dir / "_i76_zfs_manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    selected = [
        entry
        for entry in manifest["entries"]
        if any(
            fnmatch.fnmatch(str(entry["name"]).lower(), pattern.lower())
            for pattern in args.match
        )
        and (int(entry["unknown"]) & 0xFF) in {2, 4}
        and (
            asset_dir / str(entry["name"])
        ).stat().st_size != (int(entry["unknown"]) >> 8)
    ]
    if not selected:
        raise SystemExit("no matching compressed entries")

    with tempfile.TemporaryDirectory(
        prefix="i76_unzfs_", dir=asset_dir.parent
    ) as temporary:
        temporary_path = Path(temporary)
        for batch_index in range(0, len(selected), FILES_PER_PAGE):
            batch = selected[batch_index : batch_index + FILES_PER_PAGE]
            archive_path = temporary_path / "selected.zfs"
            build_batch_archive(asset_dir, batch, archive_path)
            result = subprocess.run(
                [str(executable), "-u", str(archive_path)],
                cwd=asset_dir,
                check=False,
                capture_output=True,
                text=True,
            )
            # The original Windows utility returns TRUE (1) on success.
            if result.returncode not in {0, 1}:
                raise RuntimeError(
                    f"UNZFS failed ({result.returncode}): "
                    f"{result.stdout}{result.stderr}"
                )
            for entry in batch:
                expected_size = int(entry["unknown"]) >> 8
                output_path = asset_dir / str(entry["name"])
                actual_size = output_path.stat().st_size
                if actual_size != expected_size:
                    raise RuntimeError(
                        f"{entry['name']}: decompressed to {actual_size}, "
                        f"expected {expected_size}"
                    )
            archive_path.unlink()

    print(
        f"I76_ZFS_DECOMPRESS_OK entries={len(selected)} "
        f"patterns={','.join(args.match)} directory={asset_dir}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
