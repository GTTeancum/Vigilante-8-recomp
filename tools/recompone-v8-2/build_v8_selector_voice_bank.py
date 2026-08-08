#!/usr/bin/env python3
"""Append V8's original driver-selection voices to V8:2's shell SND bank.

The V8 selector's roster table at SHELL.DLL virtual/file offset 0x11C68
contains a 20-byte record per driver.  Byte +0x0C is passed directly to the
native SND player when that driver is accepted.  This tool validates that
retail mapping, copies the referenced SPU-ADPCM samples byte-for-byte, and
appends them in roster order to V8:2's existing shell bank.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import struct


V8_ROSTER_RECORD_OFFSET = 0x11C68
V8_ROSTER_RECORD_SIZE = 20
V8_ROSTER_COUNT = 12
V8_SELECTION_VOICE_OFFSET = 0x0C
V82_RETAIL_SHELL_ENTRY_COUNT = 14

# Chassey, Clyde, Sheila, Torque, Dave, Convoy, Loki, Houston, Boogie,
# Beezwax, Molo, Sid.  These are decoded from the original roster records,
# not inferred from SND entry order.
EXPECTED_V8_SELECTION_VOICES = (
    29, 25, 27, 17, 21, 20, 22, 28, 19, 18, 23, 24,
)


def parse_bank(data: bytes) -> tuple[list[tuple[int, int]], bytes]:
    if len(data) < 4:
        raise ValueError("SND bank is truncated")
    count, size_in_8b = struct.unpack_from("<HH", data)
    header_size = 4 + count * 4
    expected_size = header_size + size_in_8b * 8
    if expected_size != len(data):
        raise ValueError(
            f"SND length mismatch: header describes {expected_size}, "
            f"file contains {len(data)}"
        )
    entries = [
        struct.unpack_from("<HH", data, 4 + index * 4)
        for index in range(count)
    ]
    if entries and entries[0][0] != 0:
        raise ValueError("SND payload does not begin at entry zero")
    for index, (offset, _) in enumerate(entries):
        if offset > size_in_8b:
            raise ValueError(f"SND entry {index} starts past the payload")
        if index and offset < entries[index - 1][0]:
            raise ValueError("SND entry offsets are not monotonic")
    return entries, data[header_size:]


def selection_voice_ids(shell_dll: bytes) -> tuple[int, ...]:
    required = (
        V8_ROSTER_RECORD_OFFSET
        + V8_ROSTER_RECORD_SIZE * V8_ROSTER_COUNT
    )
    if len(shell_dll) < required:
        raise ValueError("V8 SHELL.DLL is too small for its roster table")
    voices = tuple(
        shell_dll[
            V8_ROSTER_RECORD_OFFSET
            + index * V8_ROSTER_RECORD_SIZE
            + V8_SELECTION_VOICE_OFFSET
        ]
        for index in range(V8_ROSTER_COUNT)
    )
    if voices != EXPECTED_V8_SELECTION_VOICES:
        raise ValueError(
            "V8 selector voice mapping does not match the supported USA "
            f"retail table: found {voices}"
        )
    return voices


def sample_bytes(
    entries: list[tuple[int, int]], payload: bytes, index: int
) -> bytes:
    if index < 0 or index >= len(entries):
        raise ValueError(f"SND entry {index} is outside {len(entries)} entries")
    start = entries[index][0] * 8
    end = (
        entries[index + 1][0] * 8
        if index + 1 < len(entries)
        else len(payload)
    )
    if end <= start:
        raise ValueError(f"SND entry {index} has no payload")
    return payload[start:end]


def build(
    v8_shell_dll: bytes, v8_snd: bytes, v82_snd: bytes
) -> tuple[bytes, dict[str, object]]:
    voices = selection_voice_ids(v8_shell_dll)
    v8_entries, v8_payload = parse_bank(v8_snd)
    v82_entries, v82_payload = parse_bank(v82_snd)
    original_v82_count = len(v82_entries)
    if original_v82_count == V82_RETAIL_SHELL_ENTRY_COUNT + V8_ROSTER_COUNT:
        for roster_index, source_index in enumerate(voices):
            destination_index = V82_RETAIL_SHELL_ENTRY_COUNT + roster_index
            if (
                v82_entries[destination_index][1]
                != v8_entries[source_index][1]
                or sample_bytes(
                    v82_entries, v82_payload, destination_index
                )
                != sample_bytes(v8_entries, v8_payload, source_index)
            ):
                raise ValueError(
                    "existing appended V8 selector voice bank does not "
                    f"match roster index {roster_index}"
                )
        return v82_snd, {
            "original_v82_entries": V82_RETAIL_SHELL_ENTRY_COUNT,
            "extended_v82_entries": original_v82_count,
            "appended_payload_bytes": 0,
            "entries": [],
        }
    if original_v82_count != V82_RETAIL_SHELL_ENTRY_COUNT:
        raise ValueError(
            "V8:2 shell SND bank must contain either the retail 14 entries "
            "or the validated 26-entry V8 extension; "
            f"found {original_v82_count}"
        )

    appended = bytearray()
    report_entries: list[dict[str, int]] = []
    for roster_index, source_index in enumerate(voices):
        source = sample_bytes(v8_entries, v8_payload, source_index)
        destination_index = len(v82_entries)
        destination_offset = (len(v82_payload) + len(appended)) // 8
        pitch = v8_entries[source_index][1]
        v82_entries.append((destination_offset, pitch))
        appended.extend(source)
        report_entries.append(
            {
                "roster_index": roster_index,
                "source_snd_index": source_index,
                "destination_snd_index": destination_index,
                "pitch": pitch,
                "payload_bytes": len(source),
            }
        )

    payload = v82_payload + bytes(appended)
    if len(payload) % 8:
        raise AssertionError("SPU ADPCM payload lost 8-byte alignment")
    size_in_8b = len(payload) // 8
    if len(v82_entries) > 0xFFFF or size_in_8b > 0xFFFF:
        raise ValueError("extended SND bank exceeds its native 16-bit fields")

    header = struct.pack("<HH", len(v82_entries), size_in_8b)
    header += b"".join(struct.pack("<HH", *entry) for entry in v82_entries)
    output = header + payload
    reparsed, reparsed_payload = parse_bank(output)
    if len(reparsed) != original_v82_count + V8_ROSTER_COUNT:
        raise AssertionError("extended SND entry count is incorrect")
    if reparsed_payload[: len(v82_payload)] != v82_payload:
        raise AssertionError("the original V8:2 SND payload changed")
    for item in report_entries:
        copied = sample_bytes(
            reparsed, reparsed_payload, item["destination_snd_index"]
        )
        source = sample_bytes(
            v8_entries, v8_payload, item["source_snd_index"]
        )
        if copied != source:
            raise AssertionError(
                f"roster voice {item['roster_index']} was not copied exactly"
            )

    return output, {
        "original_v82_entries": original_v82_count,
        "extended_v82_entries": len(v82_entries),
        "appended_payload_bytes": len(appended),
        "entries": report_entries,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--v8-shell-dll", type=Path, required=True)
    parser.add_argument("--v8-snd", type=Path, required=True)
    parser.add_argument("--v82-snd", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    output, report = build(
        args.v8_shell_dll.read_bytes(),
        args.v8_snd.read_bytes(),
        args.v82_snd.read_bytes(),
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(output)
    print(
        f"{args.output}: {report['original_v82_entries']} -> "
        f"{report['extended_v82_entries']} entries, "
        f"+{report['appended_payload_bytes']} byte-exact SPU-ADPCM bytes"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
