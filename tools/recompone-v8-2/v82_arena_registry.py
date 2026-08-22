#!/usr/bin/env python3
"""Build and validate append-only V8:2 native arena registries."""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path
from pathlib import PureWindowsPath


MAGIC = b"V8AR"
VERSION = 1
GAME_V82 = 2
HEADER = struct.Struct("<4sHHHH")
RECORD = struct.Struct("<IIIIHHHH")
NATIVE_NAME_MAX = 18
NATIVE_SUBTITLE_MAX = 18


@dataclass(frozen=True)
class ArenaEntry:
    stable_id: str
    name: str
    subtitle: str
    path: str
    marker_x: int
    marker_y: int
    preview_index: int


def _validate_entry(entry: ArenaEntry) -> None:
    if not entry.stable_id.strip():
        raise ValueError("arena stable ID cannot be empty")
    if not entry.name.strip():
        raise ValueError(f"arena {entry.stable_id} has no display name")
    subtitle = entry.subtitle or entry.name
    for value, label, maximum in (
        (entry.name, "display name", NATIVE_NAME_MAX),
        (subtitle, "subtitle", NATIVE_SUBTITLE_MAX),
    ):
        try:
            value.encode("ascii")
        except UnicodeEncodeError as error:
            raise ValueError(
                f"arena {entry.stable_id} {label} is not ASCII"
            ) from error
        if len(value) > maximum:
            raise ValueError(
                f"arena {entry.stable_id} {label} exceeds the native "
                f"{maximum}-character selector field"
            )
    normalized = entry.path.replace("/", "\\")
    if not normalized or normalized.startswith("\\") or ":" in normalized:
        raise ValueError(f"arena {entry.stable_id} has an invalid path")
    if ".." in normalized.split("\\"):
        raise ValueError(f"arena {entry.stable_id} path escapes the game root")
    try:
        normalized.encode("ascii")
    except UnicodeEncodeError as error:
        raise ValueError(
            f"arena {entry.stable_id} path is not ASCII"
        ) from error
    for value, label in (
        (entry.marker_x, "marker_x"),
        (entry.marker_y, "marker_y"),
        (entry.preview_index, "preview_index"),
    ):
        if not 0 <= value <= 0xFFFF:
            raise ValueError(f"arena {entry.stable_id} {label} is outside u16")


def primary_export_name(dll: bytes) -> str:
    """Return the first native overlay export used as the arena callback."""

    if len(dll) < 24:
        raise ValueError("arena DLL header is truncated")
    image_size, export_table = struct.unpack_from("<II", dll)
    if (
        image_size == 0
        or image_size > len(dll)
        or export_table < 8
        or export_table + 8 > image_size
    ):
        raise ValueError("arena DLL header has an invalid export table")
    name_offset, callback_offset = struct.unpack_from("<II", dll, export_table)
    if (
        name_offset < export_table + 8
        or name_offset >= image_size
        or callback_offset >= image_size
    ):
        raise ValueError("arena DLL primary export is outside its image")
    end = dll.find(b"\0", name_offset, image_size)
    if end < 0:
        raise ValueError("arena DLL primary export name is unterminated")
    try:
        name = dll[name_offset:end].decode("ascii")
    except UnicodeDecodeError as error:
        raise ValueError("arena DLL primary export name is not ASCII") from error
    if not name:
        raise ValueError("arena DLL primary export name is empty")
    return name


def validate_primary_export(entry: ArenaEntry, dll: bytes) -> str:
    """Enforce V8:2's case-sensitive path-stem/export lookup contract."""

    export = primary_export_name(dll)
    stem = PureWindowsPath(entry.path).stem
    if stem != export:
        raise ValueError(
            f"arena {entry.stable_id} path stem {stem!r} does not exactly "
            f"match primary DLL export {export!r}"
        )
    return export


def build_registry(entries: list[ArenaEntry]) -> bytes:
    if len(entries) > 0xFFFF:
        raise ValueError("arena registry contains more than 65535 entries")
    seen: set[str] = set()
    for entry in entries:
        _validate_entry(entry)
        folded = entry.stable_id.casefold()
        if folded in seen:
            raise ValueError(f"duplicate arena stable ID {entry.stable_id}")
        seen.add(folded)

    strings = bytearray()
    offsets: dict[str, int] = {}
    base = HEADER.size + len(entries) * RECORD.size

    def intern(value: str) -> int:
        if value in offsets:
            return offsets[value]
        encoded = value.encode("utf-8")
        if b"\0" in encoded:
            raise ValueError("arena registry strings cannot contain NUL")
        offset = base + len(strings)
        offsets[value] = offset
        strings.extend(encoded)
        strings.append(0)
        return offset

    records = bytearray()
    for entry in entries:
        records.extend(RECORD.pack(
            intern(entry.stable_id),
            intern(entry.name),
            intern(entry.subtitle or entry.name),
            intern(entry.path.replace("/", "\\")),
            entry.marker_x,
            entry.marker_y,
            entry.preview_index,
            0,
        ))
    return (
        HEADER.pack(MAGIC, VERSION, GAME_V82, len(entries), RECORD.size)
        + records
        + strings
    )


def parse_registry(data: bytes) -> list[ArenaEntry]:
    if len(data) < HEADER.size:
        raise ValueError("ARENAS.V8R header is truncated")
    magic, version, game, count, record_size = HEADER.unpack_from(data)
    if magic != MAGIC or version != VERSION or game != GAME_V82:
        raise ValueError("ARENAS.V8R header is invalid or targets another game")
    if record_size != RECORD.size:
        raise ValueError(f"unsupported ARENAS.V8R record size {record_size}")
    table_end = HEADER.size + count * record_size
    if table_end > len(data):
        raise ValueError("ARENAS.V8R record table is truncated")

    def string_at(offset: int) -> str:
        if not table_end <= offset < len(data):
            raise ValueError(f"ARENAS.V8R string offset 0x{offset:X} is invalid")
        end = data.find(b"\0", offset)
        if end < 0:
            raise ValueError("ARENAS.V8R string is unterminated")
        return data[offset:end].decode("utf-8")

    entries: list[ArenaEntry] = []
    for index in range(count):
        values = RECORD.unpack_from(data, HEADER.size + index * record_size)
        stable, name, subtitle, path, x, y, preview, reserved = values
        if reserved != 0:
            raise ValueError(f"ARENAS.V8R entry {index} reserved field is nonzero")
        entry = ArenaEntry(
            string_at(stable), string_at(name), string_at(subtitle),
            string_at(path), x, y, preview,
        )
        _validate_entry(entry)
        entries.append(entry)
    return entries


def write_registry(path: Path, entries: list[ArenaEntry]) -> bytes:
    result = build_registry(entries)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(result)
    if parse_registry(result) != entries:
        raise AssertionError("ARENAS.V8R did not round-trip")
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("registry", type=Path)
    args = parser.parse_args()
    entries = parse_registry(args.registry.read_bytes())
    print(f"ARENAS.V8R V8:2 entries={len(entries)}")
    for index, entry in enumerate(entries, start=18):
        print(
            f"  slot={index} id={entry.stable_id} name={entry.name!r} "
            f"marker=({entry.marker_x},{entry.marker_y}) "
            f"preview={entry.preview_index} path={entry.path}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
