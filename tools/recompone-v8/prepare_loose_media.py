#!/usr/bin/env python3
"""Build Vigilante 8's runtime-only loose-disc metadata and asset tree.

The resulting JSON contains only ISO/CUE layout metadata.  STR and XA outputs
retain each Mode 2 sector's 2336 bytes (subheader plus payload), so the PC host
can stream them without reopening a BIN image.
"""
from __future__ import annotations

import argparse
import base64
import json
import os
import re
import shutil
import struct
import subprocess
from pathlib import Path


RAW_SECTOR = 2352
COOKED_SECTOR = 2048
MODE2_DATA_OFFSET = 24
MODE2_STREAM_OFFSET = 16
MODE2_STREAM_SIZE = 2336


def msf_to_sectors(value: str) -> int:
    minute, second, frame = (int(part) for part in value.split(":"))
    return (minute * 60 + second) * 75 + frame


def parse_cue(cue_path: Path) -> list[dict]:
    tracks: list[dict] = []
    current_file: Path | None = None
    for raw_line in cue_path.read_text(encoding="ascii", errors="replace").splitlines():
        line = raw_line.strip()
        file_match = re.match(r'^FILE\s+"(.+)"\s+BINARY$', line, re.IGNORECASE)
        if file_match:
            current_file = cue_path.parent / file_match.group(1)
            continue
        track_match = re.match(r"^TRACK\s+(\d+)\s+(.+)$", line, re.IGNORECASE)
        if track_match:
            if current_file is None:
                raise ValueError("TRACK appeared before FILE")
            tracks.append({
                "number": int(track_match.group(1)),
                "mode": track_match.group(2).upper(),
                "file": current_file,
                "indices": {},
            })
            continue
        index_match = re.match(
            r"^INDEX\s+(\d+)\s+(\d+:\d+:\d+)$", line, re.IGNORECASE)
        if index_match and tracks:
            tracks[-1]["indices"][int(index_match.group(1))] = msf_to_sectors(
                index_match.group(2))

    file_base_lba = 0
    for file_path, grouped in _group_tracks_by_file(tracks):
        file_sectors = file_path.stat().st_size // RAW_SECTOR
        for track in grouped:
            index0 = track["indices"].get(0, track["indices"].get(1, 0))
            index1 = track["indices"].get(1, 0)
            track["fileBaseLba"] = file_base_lba
            track["index0Lba"] = file_base_lba + index0
            track["startLba"] = file_base_lba + index1
        ordered = sorted(grouped, key=lambda item: item["index0Lba"])
        for index, track in enumerate(ordered):
            track["endLba"] = (
                ordered[index + 1]["index0Lba"]
                if index + 1 < len(ordered)
                else file_base_lba + file_sectors
            )
        file_base_lba += file_sectors
    return tracks


def _group_tracks_by_file(tracks: list[dict]):
    groups: list[tuple[Path, list[dict]]] = []
    for track in tracks:
        for path, grouped in groups:
            if path == track["file"]:
                grouped.append(track)
                break
        else:
            groups.append((track["file"], [track]))
    return groups


class DataTrack:
    def __init__(self, path: Path):
        self.path = path
        self.stream = path.open("rb")
        self.sectors = path.stat().st_size // RAW_SECTOR

    def close(self) -> None:
        self.stream.close()

    def read_cooked_sector(self, lba: int) -> bytes:
        return self._read(lba, MODE2_DATA_OFFSET, COOKED_SECTOR)

    def read_stream_sector(self, lba: int) -> bytes:
        return self._read(lba, MODE2_STREAM_OFFSET, MODE2_STREAM_SIZE)

    def _read(self, lba: int, offset: int, size: int) -> bytes:
        if lba < 0 or lba >= self.sectors:
            raise ValueError(f"LBA {lba} is outside the data track")
        self.stream.seek(lba * RAW_SECTOR + offset)
        data = self.stream.read(size)
        if len(data) != size:
            raise IOError(f"short read at data-track LBA {lba}")
        return data

    def read_extent(self, lba: int, size: int) -> bytes:
        output = bytearray()
        for sector in range((size + COOKED_SECTOR - 1) // COOKED_SECTOR):
            output.extend(self.read_cooked_sector(lba + sector))
        return bytes(output[:size])


def directory_records(data: bytes):
    offset = 0
    while offset < len(data):
        length = data[offset]
        if length == 0:
            offset = (offset // COOKED_SECTOR + 1) * COOKED_SECTOR
            continue
        if length < 34 or offset + length > len(data):
            break
        record = data[offset:offset + length]
        name_bytes = record[33:33 + record[32]]
        if name_bytes == b"\x00":
            name = "."
        elif name_bytes == b"\x01":
            name = ".."
        else:
            name = name_bytes.decode("ascii", "replace").split(";", 1)[0]
        yield {
            "name": name,
            "lba": struct.unpack_from("<I", record, 2)[0],
            "size": struct.unpack_from("<I", record, 10)[0],
            "directory": bool(record[25] & 0x02),
        }
        offset += length


def catalog_files(track: DataTrack) -> tuple[str, list[dict], list[tuple[int, int]]]:
    pvd = track.read_cooked_sector(16)
    if pvd[1:6] != b"CD001":
        raise ValueError("primary volume descriptor not found")
    volume = pvd[40:72].decode("ascii", "replace").strip()
    root_record = pvd[156:156 + pvd[156]]
    root_lba = struct.unpack_from("<I", root_record, 2)[0]
    root_size = struct.unpack_from("<I", root_record, 10)[0]
    files: list[dict] = []
    directories: list[tuple[int, int]] = []

    def walk(lba: int, size: int, parent: str) -> None:
        directories.append((lba, size))
        for record in directory_records(track.read_extent(lba, size)):
            if record["name"] in (".", ".."):
                continue
            path = f'{parent}/{record["name"]}' if parent else record["name"]
            if record["directory"]:
                walk(record["lba"], record["size"], path)
            else:
                files.append({
                    "path": path,
                    "lba": record["lba"],
                    "size": record["size"],
                })

    walk(root_lba, root_size, "")
    files.sort(key=lambda item: (item["lba"], item["path"]))
    return volume, files, directories


def add_music_sources(files: list[dict], tracks: list[dict]) -> None:
    by_lba = {entry["lba"]: entry for entry in files}
    for track in tracks:
        if track["mode"] != "AUDIO":
            continue
        entry = by_lba.get(track["startLba"])
        stem = (
            Path(entry["path"]).stem
            if entry is not None and
            entry["path"].upper().startswith("REDBOOK/")
            else f'track{track["number"]:02d}'
        )
        track["source"] = f"music/{stem}.ogg"


def write_loose_tree(track: DataTrack, files: list[dict], loose_root: Path) -> None:
    for entry in files:
        suffix = Path(entry["path"]).suffix.upper()
        target = loose_root / Path(entry["path"])
        target.parent.mkdir(parents=True, exist_ok=True)
        temporary = target.with_name(target.name + ".recompone-tmp")
        if suffix in (".STR", ".XA"):
            sector_count = (
                entry["size"] + COOKED_SECTOR - 1
            ) // COOKED_SECTOR
            with temporary.open("wb") as output:
                for index in range(sector_count):
                    output.write(track.read_stream_sector(entry["lba"] + index))
            storage = "raw stream"
        else:
            temporary.write_bytes(
                track.read_extent(entry["lba"], entry["size"]))
            sector_count = (
                entry["size"] + COOKED_SECTOR - 1
            ) // COOKED_SECTOR
            storage = "cooked"
        os.replace(temporary, target)
        print(
            f"{storage} {entry['path']} sectors={sector_count} "
            f"bytes={target.stat().st_size}")


def write_music_tracks(tracks: list[dict], loose_root: Path) -> None:
    ffmpeg = shutil.which("ffmpeg")
    if ffmpeg is None:
        raise FileNotFoundError(
            "ffmpeg is required to prepare standalone CD-audio tracks")
    for track in tracks:
        source = track.get("source")
        if not source:
            continue
        target = loose_root / Path(source)
        target.parent.mkdir(parents=True, exist_ok=True)
        temporary = target.with_name(target.name + ".recompone-tmp.ogg")
        start_sector = track["startLba"] - track["fileBaseLba"]
        sector_count = track["endLba"] - track["startLba"]
        command = [
            ffmpeg, "-hide_banner", "-loglevel", "error", "-y",
            "-f", "s16le", "-ar", "44100", "-ac", "2", "-i", "pipe:0",
            "-c:a", "libvorbis", "-q:a", "6", str(temporary),
        ]
        with track["file"].open("rb") as source_file:
            source_file.seek(start_sector * RAW_SECTOR)
            process = subprocess.Popen(command, stdin=subprocess.PIPE)
            assert process.stdin is not None
            remaining = sector_count * RAW_SECTOR
            while remaining:
                chunk = source_file.read(min(1024 * 1024, remaining))
                if not chunk:
                    process.kill()
                    raise IOError(
                        f"short CD-audio read for track {track['number']}")
                process.stdin.write(chunk)
                remaining -= len(chunk)
            process.stdin.close()
            result = process.wait()
        if result != 0:
            temporary.unlink(missing_ok=True)
            raise subprocess.CalledProcessError(result, command)
        os.replace(temporary, target)
        print(
            f"music track={track['number']} source={source} "
            f"sectors={sector_count} bytes={target.stat().st_size}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cue", required=True, type=Path)
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--loose-root", type=Path)
    args = parser.parse_args()

    cue_path = args.cue.resolve()
    tracks = parse_cue(cue_path)
    data_tracks = [item for item in tracks if item["mode"] != "AUDIO"]
    if len(data_tracks) != 1:
        raise SystemExit(f"expected one data track, found {len(data_tracks)}")
    data_track = DataTrack(data_tracks[0]["file"])
    try:
        volume, files, directories = catalog_files(data_track)
        add_music_sources(files, tracks)
        metadata_lbas = set(range(16, min(item["lba"] for item in files)))
        for directory_lba, directory_size in directories:
            metadata_lbas.update(range(
                directory_lba,
                directory_lba + (directory_size + COOKED_SECTOR - 1) // COOKED_SECTOR,
            ))
        metadata_sectors = {
            str(lba): base64.b64encode(data_track.read_cooked_sector(lba)).decode("ascii")
            for lba in sorted(metadata_lbas)
        }
        if args.loose_root:
            loose_root = args.loose_root.resolve()
            write_loose_tree(data_track, files, loose_root)
            write_music_tracks(tracks, loose_root)
    finally:
        data_track.close()

    manifest_tracks = [{
        "number": item["number"],
        "index0Lba": item["index0Lba"],
        "startLba": item["startLba"],
        "endLba": item["endLba"],
        **({"source": item["source"]} if "source" in item else {}),
    } for item in tracks]
    manifest = {
        "formatVersion": 1,
        "volume": volume,
        "leadOutLba": max(item["endLba"] for item in tracks),
        "metadataSectors": metadata_sectors,
        "files": files,
        "tracks": manifest_tracks,
    }
    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(
        f"manifest volume={volume!r} files={len(files)} tracks={len(tracks)} "
        f"path={args.manifest}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
