#!/usr/bin/env python3
"""Build a sector-accurate test CUE from a complete V8:2 loose tree."""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
from pathlib import Path


RAW_SECTOR = 2352
COOKED_SECTOR = 2048
STREAM_SECTOR = 2336


def msf(sectors: int) -> str:
    minute, remainder = divmod(sectors, 75 * 60)
    second, frame = divmod(remainder, 75)
    return f"{minute:02d}:{second:02d}:{frame:02d}"


def write_data_track(
    loose: Path, fallback: Path | None, manifest: dict, target: Path
) -> None:
    data_end = manifest["tracks"][0]["endLba"]
    with target.open("w+b") as output:
        output.truncate(data_end * RAW_SECTOR)
        for entry in manifest["files"]:
            source = loose / Path(entry["path"])
            payload = source.read_bytes()
            raw_stream = source.suffix.upper() in (".STR", ".XA")
            stored_sector = STREAM_SECTOR if raw_stream else COOKED_SECTOR
            sector_offset = 16 if raw_stream else 24
            expected = (
                ((entry["size"] + COOKED_SECTOR - 1) // COOKED_SECTOR)
                * STREAM_SECTOR
                if raw_stream
                else entry["size"]
            )
            if len(payload) != expected and fallback is not None:
                candidate = fallback / Path(entry["path"])
                if candidate.is_file() and candidate.stat().st_size == expected:
                    source = candidate
                    payload = source.read_bytes()
            if len(payload) != expected:
                raise ValueError(
                    f"{entry['path']}: {len(payload)} bytes, expected {expected}"
                )
            for index in range(0, len(payload), stored_sector):
                sector = index // stored_sector
                output.seek((entry["lba"] + sector) * RAW_SECTOR + sector_offset)
                output.write(payload[index : index + stored_sector])


def decode_track(ffmpeg: str, source: Path, target: Path, frames: int) -> None:
    expected = frames * RAW_SECTOR
    temporary = target.with_suffix(".pcm")
    subprocess.run(
        [
            ffmpeg,
            "-hide_banner",
            "-loglevel",
            "error",
            "-y",
            "-i",
            str(source),
            "-f",
            "s16le",
            "-ar",
            "44100",
            "-ac",
            "2",
            str(temporary),
        ],
        check=True,
    )
    actual = temporary.stat().st_size
    if actual < expected:
        temporary.unlink(missing_ok=True)
        raise ValueError(
            f"{source.name}: decoded {actual} bytes, need at least {expected}"
        )
    if actual > expected:
        with temporary.open("r+b") as stream:
            stream.truncate(expected)
    temporary.replace(target)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--loose-root", required=True, type=Path)
    parser.add_argument("--fallback-root", type=Path)
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    loose = args.loose_root.resolve()
    fallback = args.fallback_root.resolve() if args.fallback_root else None
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    ffmpeg = shutil.which("ffmpeg")
    if ffmpeg is None:
        raise FileNotFoundError("ffmpeg is required to build the test image")

    cue_lines: list[str] = []
    data_name = "Vigilante 8 - 2nd Offensive [U] [SLUS-00868] (Track 01).bin"
    write_data_track(loose, fallback, manifest, output / data_name)
    cue_lines.extend(
        [
            f'FILE "{data_name}" BINARY',
            "  TRACK 01 MODE2/2352",
            "    INDEX 01 00:00:00",
        ]
    )

    for track in manifest["tracks"][1:]:
        number = track["number"]
        name = (
            "Vigilante 8 - 2nd Offensive [U] [SLUS-00868] "
            f"(Track {number:02d}).bin"
        )
        target = output / name
        pregap = track["startLba"] - track["index0Lba"]
        audio_sectors = track["endLba"] - track["startLba"]
        decoded = target.with_suffix(".audio")
        decode_track(
            ffmpeg,
            loose / Path(track["source"]),
            decoded,
            audio_sectors,
        )
        with target.open("wb") as stream:
            stream.truncate(pregap * RAW_SECTOR)
            stream.seek(pregap * RAW_SECTOR)
            with decoded.open("rb") as source:
                shutil.copyfileobj(source, stream, 1024 * 1024)
        decoded.unlink()
        cue_lines.extend(
            [
                f'FILE "{name}" BINARY',
                f"  TRACK {number:02d} AUDIO",
                "    INDEX 00 00:00:00",
                f"    INDEX 01 {msf(pregap)}",
            ]
        )

    cue = output / "Vigilante 8 - 2nd Offensive [U] [SLUS-00868].cue"
    cue.write_text("\n".join(cue_lines) + "\n", encoding="ascii")
    print(cue)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
