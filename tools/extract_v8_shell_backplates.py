#!/usr/bin/env python3
"""Decode shell MDEC records directly from V8/V8:2 TBL assets.

The shell table begins with a count followed by count+1 absolute record
offsets.  Full-screen image records contain a little-endian width/height pair
followed by one standard PlayStation BS/MDEC v2 frame.  This tool wraps only
that compressed frame in a one-frame AVI container so FFmpeg's native MDEC
decoder can produce lossless PNGs.  It never launches the game or reads VRAM.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

from extract_psx_iso import (
    MODE1_DATA_OFF,
    MODE2_FORM1_DATA_OFF,
    SECTOR_DATA,
    SECTOR_RAW,
    PsxIso,
    parse_dir_records,
)


def chunk(tag: bytes, payload: bytes) -> bytes:
    result = tag + struct.pack("<I", len(payload)) + payload
    if len(payload) & 1:
        result += b"\0"
    return result


def list_chunk(kind: bytes, payload: bytes) -> bytes:
    return chunk(b"LIST", kind + payload)


def one_frame_avi(width: int, height: int, frame: bytes) -> bytes:
    usec_per_frame = 1_000_000
    avih = struct.pack(
        "<14I",
        usec_per_frame,
        len(frame),
        0,
        0x10,
        1,
        0,
        1,
        len(frame),
        width,
        height,
        0,
        0,
        0,
        0,
    )
    strh = struct.pack(
        "<4s4sIHHIIIIIIIIiiii",
        b"vids",
        b"MDEC",
        0,
        0,
        0,
        0,
        1,
        1,
        0,
        1,
        len(frame),
        0xFFFFFFFF,
        0,
        0,
        0,
        width,
        height,
    )
    strf = struct.pack(
        "<IiiHH4sIiiII",
        40,
        width,
        height,
        1,
        16,
        b"MDEC",
        len(frame),
        0,
        0,
        0,
        0,
    )
    hdrl = list_chunk(
        b"hdrl",
        chunk(b"avih", avih) +
        list_chunk(b"strl", chunk(b"strh", strh) + chunk(b"strf", strf)),
    )
    movi_payload = chunk(b"00dc", frame)
    movi = list_chunk(b"movi", movi_payload)
    # The offset is relative to the first byte after the 'movi' list type.
    idx1 = chunk(b"idx1", struct.pack("<4sIII", b"00dc", 0x10, 4, len(frame)))
    body = b"AVI " + hdrl + movi + idx1
    return b"RIFF" + struct.pack("<I", len(body)) + body


def records(data: bytes) -> list[bytes]:
    if len(data) < 8:
        raise ValueError("RESOURCE.TBL is truncated")
    count = struct.unpack_from("<I", data)[0]
    if count <= 0 or count > 256:
        raise ValueError(f"implausible RESOURCE.TBL record count {count}")
    table_end = 4 + (count + 1) * 4
    if table_end > len(data):
        raise ValueError("RESOURCE.TBL offset table is truncated")
    offsets = struct.unpack_from(f"<{count + 1}I", data, 4)
    if offsets[0] < table_end or tuple(sorted(offsets)) != offsets or offsets[-1] != len(data):
        raise ValueError("RESOURCE.TBL offsets are invalid")
    return [data[offsets[i]:offsets[i + 1]] for i in range(count)]


def read_iso_file(track_path: Path, iso_path: str) -> bytes:
    data_offset = None
    with track_path.open("rb") as fp:
        for candidate in (MODE1_DATA_OFF, MODE2_FORM1_DATA_OFF):
            fp.seek(16 * SECTOR_RAW + candidate)
            pvd = fp.read(SECTOR_DATA)
            if pvd[1:6] == b"CD001":
                data_offset = candidate
                break
    if data_offset is None:
        raise ValueError("primary volume descriptor not found at LBA 16")

    iso = PsxIso(track_path, data_offset)
    try:
        pvd = iso.read_sector(16)
        root = pvd[156:156 + pvd[156]]
        extent = struct.unpack_from("<I", root, 2)[0]
        size = struct.unpack_from("<I", root, 10)[0]
        parts = [part for part in iso_path.replace("\\", "/").split("/") if part]
        if not parts:
            raise ValueError("ISO path is empty")
        for part_index, part in enumerate(parts):
            directory = iso.read_extent(extent, size)
            entries = {
                name.upper(): (child_extent, child_size, flags)
                for name, child_extent, child_size, flags in parse_dir_records(directory)
                if name not in (".", "..")
            }
            key = part.upper()
            if key not in entries:
                raise FileNotFoundError(f"{iso_path}: missing ISO entry {part}")
            extent, size, flags = entries[key]
            is_last = part_index == len(parts) - 1
            if is_last:
                if flags & 0x02:
                    raise IsADirectoryError(iso_path)
                return iso.read_extent(extent, size)
            if not (flags & 0x02):
                raise NotADirectoryError("/".join(parts[:part_index + 1]))
        raise AssertionError("unreachable ISO traversal state")
    finally:
        iso.close()


def is_mdec_record(record: bytes) -> bool:
    if len(record) < 16:
        return False
    width, height = struct.unpack_from("<HH", record)
    # BS v2 starts with a word count, magic 0x3800, quantization scale, v2.
    return (
        width > 0 and height > 0 and width <= 1024 and height <= 1024 and
        record[6:8] == b"\x00\x38" and record[10:12] == b"\x02\x00"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "source",
        type=Path,
        help="Loose shell TBL, or raw Track 01 BIN when --iso-path is used.",
    )
    parser.add_argument("output", type=Path)
    parser.add_argument("--ffmpeg", default="ffmpeg")
    parser.add_argument(
        "--iso-path",
        help="Read this file directly from a raw-sector PlayStation track.",
    )
    args = parser.parse_args()

    data = (
        read_iso_file(args.source, args.iso_path)
        if args.iso_path
        else args.source.read_bytes()
    )
    args.output.mkdir(parents=True, exist_ok=True)
    decoded: list[dict[str, object]] = []
    for index, record in enumerate(records(data)):
        if not is_mdec_record(record):
            continue
        width, height = struct.unpack_from("<HH", record)
        frame = record[4:]
        avi = one_frame_avi(width, height, frame)
        avi_path = args.output / f"record_{index:02d}_{width}x{height}.avi"
        png_path = args.output / f"record_{index:02d}_{width}x{height}.png"
        avi_path.write_bytes(avi)
        subprocess.run(
            [
                args.ffmpeg,
                "-hide_banner",
                "-loglevel",
                "error",
                "-y",
                "-c:v",
                "mdec",
                "-i",
                str(avi_path),
                "-frames:v",
                "1",
                str(png_path),
            ],
            check=True,
        )
        avi_path.unlink()
        decoded.append({
            "record": index,
            "dimensions": [width, height],
            "record_bytes": len(record),
            "record_sha256": hashlib.sha256(record).hexdigest().upper(),
            "png": png_path.name,
            "png_sha256": hashlib.sha256(png_path.read_bytes()).hexdigest().upper(),
        })

    report = {
        "format": "v8-shell-resource-direct-extract-v1",
        "source": str(args.source.resolve()),
        "iso_path": args.iso_path,
        "source_bytes": len(data),
        "source_sha256": hashlib.sha256(data).hexdigest().upper(),
        "decoded": decoded,
    }
    (args.output / "manifest.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(report, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
