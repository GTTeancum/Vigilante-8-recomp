#!/usr/bin/env python3
"""List or extract files from a raw-sector Dreamcast GD-ROM ISO-9660 session.

Directory extents can refer to absolute disc LBAs that live in different GDI
data tracks.  Supply each available data track as ``START_LBA=PATH``.
"""

from __future__ import annotations

import argparse
import dataclasses
import struct
from pathlib import Path


RAW_SECTOR_SIZE = 2352
MODE1_PAYLOAD_OFFSET = 16
PAYLOAD_SIZE = 2048


@dataclasses.dataclass(frozen=True)
class Track:
    start_lba: int
    path: Path

    @property
    def sectors(self) -> int:
        return self.path.stat().st_size // RAW_SECTOR_SIZE

    def contains(self, lba: int) -> bool:
        return self.start_lba <= lba < self.start_lba + self.sectors


@dataclasses.dataclass(frozen=True)
class Entry:
    path: str
    extent_lba: int
    size: int
    is_directory: bool


def parse_track(value: str) -> Track:
    start, separator, path = value.partition("=")
    if not separator:
        raise argparse.ArgumentTypeError("track must be START_LBA=PATH")
    return Track(int(start, 0), Path(path))


def read_extent(tracks: list[Track], extent_lba: int, size: int) -> bytes:
    output = bytearray()
    remaining = size
    lba = extent_lba
    while remaining:
        track = next((candidate for candidate in tracks if candidate.contains(lba)), None)
        if track is None:
            raise ValueError(f"no supplied data track contains absolute LBA {lba}")
        offset = (
            (lba - track.start_lba) * RAW_SECTOR_SIZE
            + MODE1_PAYLOAD_OFFSET
        )
        with track.path.open("rb") as stream:
            stream.seek(offset)
            payload = stream.read(PAYLOAD_SIZE)
        if len(payload) != PAYLOAD_SIZE:
            raise EOFError(f"short sector read at absolute LBA {lba}")
        take = min(remaining, PAYLOAD_SIZE)
        output.extend(payload[:take])
        remaining -= take
        lba += 1
    return bytes(output)


def parse_directory(data: bytes, parent: str) -> list[Entry]:
    entries: list[Entry] = []
    offset = 0
    while offset < len(data):
        record_length = data[offset]
        if record_length == 0:
            offset = (offset // PAYLOAD_SIZE + 1) * PAYLOAD_SIZE
            continue
        record = data[offset:offset + record_length]
        if len(record) < 34:
            raise ValueError(f"truncated directory record at byte {offset}")
        name_length = record[32]
        name = record[33:33 + name_length].decode("ascii", errors="replace")
        offset += record_length
        if name in ("\x00", "\x01"):
            continue
        name = name.split(";", 1)[0]
        extent_lba = struct.unpack_from("<I", record, 2)[0]
        size = struct.unpack_from("<I", record, 10)[0]
        is_directory = bool(record[25] & 0x02)
        path = f"{parent}/{name}" if parent else name
        entries.append(Entry(path, extent_lba, size, is_directory))
    return entries


def build_index(tracks: list[Track], pvd_lba: int) -> list[Entry]:
    pvd = read_extent(tracks, pvd_lba, PAYLOAD_SIZE)
    if pvd[:7] != b"\x01CD001\x01":
        raise ValueError(f"LBA {pvd_lba} is not an ISO-9660 primary volume descriptor")
    root = pvd[156:156 + pvd[156]]
    root_lba = struct.unpack_from("<I", root, 2)[0]
    root_size = struct.unpack_from("<I", root, 10)[0]
    queue = [Entry("", root_lba, root_size, True)]
    index: list[Entry] = []
    seen_directories: set[tuple[int, int]] = set()
    while queue:
        directory = queue.pop(0)
        key = (directory.extent_lba, directory.size)
        if key in seen_directories:
            continue
        seen_directories.add(key)
        data = read_extent(tracks, directory.extent_lba, directory.size)
        children = parse_directory(data, directory.path)
        index.extend(children)
        queue.extend(entry for entry in children if entry.is_directory)
    return index


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--track", action="append", type=parse_track, required=True)
    parser.add_argument("--pvd-lba", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--extract", metavar="ISO_PATH")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    tracks = sorted(args.track, key=lambda track: track.start_lba)
    index = build_index(tracks, args.pvd_lba)
    if args.extract:
        if args.output is None:
            parser.error("--output is required with --extract")
        wanted = args.extract.casefold()
        entry = next(
            (candidate for candidate in index if candidate.path.casefold() == wanted),
            None,
        )
        if entry is None or entry.is_directory:
            raise SystemExit(f"file not found: {args.extract}")
        data = read_extent(tracks, entry.extent_lba, entry.size)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(data)
        print(f"wrote {args.output} bytes={entry.size} lba={entry.extent_lba}")
        return 0

    for entry in index:
        kind = "DIR " if entry.is_directory else "FILE"
        print(f"{kind} {entry.extent_lba:7d} {entry.size:10d} {entry.path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
