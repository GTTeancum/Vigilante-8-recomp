#!/usr/bin/env python3
"""Extract files from a PlayStation MODE2/2352 data track.

Vigilante 8's CUE has track 01 as MODE2/2352.  ISO9660 directory records live in
the 2048-byte Form 1 payload at sector offset 24, so this avoids relying on
third-party GUI extraction tools.
"""
from __future__ import annotations

import argparse
import os
import re
import shutil
import struct
from pathlib import Path


SECTOR_RAW = 2352
SECTOR_DATA = 2048
MODE2_FORM1_DATA_OFF = 24


class PsxIso:
    def __init__(self, track_path: Path):
        self.track_path = track_path
        self.fp = track_path.open("rb")
        self.sectors = track_path.stat().st_size // SECTOR_RAW

    def close(self) -> None:
        self.fp.close()

    def read_sector(self, lba: int) -> bytes:
        if lba < 0 or lba >= self.sectors:
            raise ValueError(f"LBA {lba} outside track ({self.sectors} sectors)")
        self.fp.seek(lba * SECTOR_RAW + MODE2_FORM1_DATA_OFF)
        data = self.fp.read(SECTOR_DATA)
        if len(data) != SECTOR_DATA:
            raise IOError(f"short sector read at LBA {lba}")
        return data

    def read_extent(self, lba: int, size: int) -> bytes:
        out = bytearray()
        sectors = (size + SECTOR_DATA - 1) // SECTOR_DATA
        for i in range(sectors):
            out += self.read_sector(lba + i)
        return bytes(out[:size])


def parse_dir_records(buf: bytes):
    p = 0
    end = len(buf)
    while p < end:
        n = buf[p]
        if n == 0:
            p = ((p // SECTOR_DATA) + 1) * SECTOR_DATA
            continue
        if p + n > end or n < 34:
            break
        rec = buf[p:p + n]
        extent = struct.unpack_from("<I", rec, 2)[0]
        size = struct.unpack_from("<I", rec, 10)[0]
        flags = rec[25]
        name_len = rec[32]
        name_raw = rec[33:33 + name_len]
        if name_raw == b"\x00":
            name = "."
        elif name_raw == b"\x01":
            name = ".."
        else:
            name = name_raw.decode("ascii", "replace")
            if ";" in name:
                name = name.split(";", 1)[0]
        yield name, extent, size, flags
        p += n


def parse_cue(cue_path: Path):
    tracks = []
    current_file = None
    file_re = re.compile(r'^FILE\s+"(.+)"\s+BINARY$', re.IGNORECASE)
    track_re = re.compile(r"^\s*TRACK\s+(\d+)\s+(.+)$", re.IGNORECASE)
    index_re = re.compile(r"^\s*INDEX\s+(\d+)\s+(\d+):(\d+):(\d+)$", re.IGNORECASE)
    for line in cue_path.read_text(encoding="ascii", errors="replace").splitlines():
        m = file_re.match(line.strip())
        if m:
            current_file = cue_path.parent / m.group(1)
            continue
        m = track_re.match(line)
        if m:
            tracks.append({
                "number": int(m.group(1)),
                "mode": m.group(2).strip().upper(),
                "file": current_file,
                "indices": {},
            })
            continue
        m = index_re.match(line)
        if m and tracks:
            mm, ss, ff = int(m.group(2)), int(m.group(3)), int(m.group(4))
            tracks[-1]["indices"][int(m.group(1))] = (mm * 60 + ss) * 75 + ff
    abs_lba = 0
    for tr in tracks:
        tr["file_sectors"] = tr["file"].stat().st_size // SECTOR_RAW
        tr["abs_file_start"] = abs_lba
        tr["abs_index01"] = abs_lba + tr["indices"].get(1, 0)
        abs_lba += tr["file_sectors"]
    return tracks


def fill_cdda_placeholders(cue_path: Path, placeholders: list[tuple[Path, int, int]]) -> int:
    if not cue_path:
        return 0
    tracks = parse_cue(cue_path)
    audio = [tr for tr in tracks if tr["mode"].startswith("AUDIO")]
    if not audio:
        return 0
    filled = 0
    for raw_file, extent, _iso_size in placeholders:
        if raw_file.suffix.upper() != ".RAW":
            continue
        if raw_file.stat().st_size != 0:
            continue
        matches = [tr for tr in audio if tr["abs_index01"] == extent]
        if not matches:
            print(f"warning: no CUE audio track matches {raw_file} extent={extent}")
            continue
        tr = matches[0]
        index01 = tr["indices"].get(1, 0)
        sectors = tr["file_sectors"] - index01
        with tr["file"].open("rb") as fp, raw_file.open("wb") as out:
            fp.seek(index01 * SECTOR_RAW)
            shutil.copyfileobj(fp, out, length=1024 * 1024)
        print(f"filled CDDA {raw_file} from track {tr['number']:02d} sectors={sectors} bytes={raw_file.stat().st_size}")
        filled += 1
    return filled


def extract_tree(iso: PsxIso, root_extent: int, root_size: int, out_dir: Path):
    files = 0
    dirs = 0
    skipped = 0
    placeholders: list[tuple[Path, int, int]] = []

    def walk(extent: int, size: int, dst: Path) -> None:
        nonlocal files, dirs, skipped
        dst.mkdir(parents=True, exist_ok=True)
        dirs += 1
        data = iso.read_extent(extent, size)
        for name, child_extent, child_size, flags in parse_dir_records(data):
            if name in (".", ".."):
                continue
            child = dst / name
            if flags & 0x02:
                walk(child_extent, child_size, child)
            else:
                child.parent.mkdir(parents=True, exist_ok=True)
                last_sector = child_extent + (child_size + SECTOR_DATA - 1) // SECTOR_DATA
                if last_sector > iso.sectors:
                    child.write_bytes(b"")
                    placeholders.append((child, child_extent, child_size))
                    skipped += 1
                    print(f"warning: placeholder for out-of-track entry {child} extent={child_extent} size={child_size}")
                else:
                    child.write_bytes(iso.read_extent(child_extent, child_size))
                files += 1

    walk(root_extent, root_size, out_dir)
    return files, dirs, skipped, placeholders


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("track01", type=Path)
    ap.add_argument("out_dir", type=Path)
    ap.add_argument("--cue", type=Path, default=None,
                    help="Optional CUE file; fills REDBOOK placeholders from audio tracks.")
    ap.add_argument("--clean", action="store_true",
                    help="Remove the output directory before extraction.")
    args = ap.parse_args()

    if args.clean and args.out_dir.exists():
        shutil.rmtree(args.out_dir)
    args.out_dir.mkdir(parents=True, exist_ok=True)

    iso = PsxIso(args.track01)
    try:
        pvd = iso.read_sector(16)
        if pvd[1:6] != b"CD001":
            raise SystemExit("primary volume descriptor not found at LBA 16")
        root = pvd[156:156 + pvd[156]]
        root_extent = struct.unpack_from("<I", root, 2)[0]
        root_size = struct.unpack_from("<I", root, 10)[0]
        volume = pvd[40:72].decode("ascii", "replace").strip()
        files, dirs, skipped, placeholders = extract_tree(iso, root_extent, root_size, args.out_dir)
        filled = fill_cdda_placeholders(args.cue, placeholders) if args.cue else 0
        print(f"extracted volume='{volume}' files={files} dirs={dirs} out_of_track_placeholders={skipped} cdda_filled={filled} to {args.out_dir}")
    finally:
        iso.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
