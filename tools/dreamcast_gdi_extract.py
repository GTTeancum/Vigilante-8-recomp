#!/usr/bin/env python3
"""Extract a file extent from a raw 2352-byte Dreamcast GDI data track.

The ISO-9660 directory records in a GD-ROM use absolute disc LBAs.  This tool
maps an extent into one raw data track, strips the 16-byte Mode-1 sector
header, and can descramble a retail Dreamcast boot binary.
"""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path


RAW_SECTOR_SIZE = 2352
MODE1_PAYLOAD_OFFSET = 16
MODE1_PAYLOAD_SIZE = 2048
MAX_SCRAMBLE_CHUNK = 2 * 1024 * 1024
SCRAMBLE_SLICE_SIZE = 32


class DreamcastRandom:
    def __init__(self, seed: int) -> None:
        self.seed = seed & 0xFFFF

    def next(self) -> int:
        self.seed = (self.seed * 2109 + 9273) & 0x7FFF
        return (self.seed + 0xC000) & 0xFFFF


def extract_mode1_extent(
    track: Path,
    track_start_lba: int,
    extent_lba: int,
    size: int,
) -> bytes:
    relative_lba = extent_lba - track_start_lba
    if relative_lba < 0:
        raise ValueError("extent starts before the selected track")

    output = bytearray()
    remaining = size
    with track.open("rb") as stream:
        for sector in range((size + MODE1_PAYLOAD_SIZE - 1) // MODE1_PAYLOAD_SIZE):
            raw_offset = (
                (relative_lba + sector) * RAW_SECTOR_SIZE
                + MODE1_PAYLOAD_OFFSET
            )
            stream.seek(raw_offset)
            payload = stream.read(MODE1_PAYLOAD_SIZE)
            if len(payload) != MODE1_PAYLOAD_SIZE:
                raise EOFError(
                    f"short sector read at relative LBA {relative_lba + sector}"
                )
            take = min(remaining, MODE1_PAYLOAD_SIZE)
            output.extend(payload[:take])
            remaining -= take
    return bytes(output)


def descramble_boot_binary(data: bytes) -> bytes:
    rng = DreamcastRandom(len(data))
    source_offset = 0
    remaining = len(data)
    output = bytearray(len(data))

    chunk_size = MAX_SCRAMBLE_CHUNK
    while chunk_size >= SCRAMBLE_SLICE_SIZE:
        while remaining >= chunk_size:
            chunk_base = source_offset
            slice_count = chunk_size // SCRAMBLE_SLICE_SIZE
            indices = list(range(slice_count))
            for index in range(slice_count - 1, -1, -1):
                replacement = (rng.next() * index) >> 16
                indices[index], indices[replacement] = (
                    indices[replacement],
                    indices[index],
                )
                source_end = source_offset + SCRAMBLE_SLICE_SIZE
                destination = chunk_base + indices[index] * SCRAMBLE_SLICE_SIZE
                output[destination:destination + SCRAMBLE_SLICE_SIZE] = (
                    data[source_offset:source_end]
                )
                source_offset = source_end
            remaining -= chunk_size
        chunk_size //= 2

    if remaining:
        output[source_offset:source_offset + remaining] = data[
            source_offset:source_offset + remaining
        ]
    return bytes(output)


def integer(value: str) -> int:
    return int(value, 0)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("track", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--track-start-lba", type=integer, required=True)
    parser.add_argument("--extent-lba", type=integer, required=True)
    parser.add_argument("--size", type=integer, required=True)
    parser.add_argument("--descramble", action="store_true")
    args = parser.parse_args()

    data = extract_mode1_extent(
        args.track,
        args.track_start_lba,
        args.extent_lba,
        args.size,
    )
    if args.descramble:
        data = descramble_boot_binary(data)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(data)
    print(
        f"wrote {args.output} bytes={len(data)} "
        f"sha256={hashlib.sha256(data).hexdigest().upper()}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
