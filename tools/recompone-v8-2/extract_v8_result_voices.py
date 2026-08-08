#!/usr/bin/env python3
"""Convert V8's interleaved result banks to V8:2-native per-driver XA files."""

from __future__ import annotations

import argparse
from pathlib import Path


SECTOR_SIZE = 2336
DRIVER_COUNT = 12
FIRST_DRIVER_CHANNEL = 1
NATIVE_MONO_INTERLEAVE = 8


def validate(destination: Path, expected_audio_sectors: int) -> int:
    data = destination.read_bytes()
    if not data or len(data) % SECTOR_SIZE:
        raise ValueError(f"{destination} is not sector aligned")
    sectors = [
        data[offset : offset + SECTOR_SIZE]
        for offset in range(0, len(data), SECTOR_SIZE)
    ]
    accepted = 0
    for index, sector in enumerate(sectors):
        if sector[:4] != sector[4:8]:
            raise ValueError(
                f"{destination} sector {index} has mismatched XA subheaders"
            )
        if sector[2] & 0x04 == 0:
            raise ValueError(
                f"{destination} sector {index} is not an XA audio sector"
            )
        if index % NATIVE_MONO_INTERLEAVE == 0:
            if sector[1] != 0:
                raise ValueError(
                    f"{destination} sector {index} is not accepted channel zero"
                )
            accepted += 1
        elif sector[1] == 0:
            raise ValueError(
                f"{destination} filler sector {index} would pass the XA filter"
            )
        if index + 1 < len(sectors) and sector[2] & 0x80:
            raise ValueError(
                f"{destination} sector {index} has an early EOF marker"
            )
    if accepted != expected_audio_sectors:
        raise ValueError(
            f"{destination} has {accepted} accepted audio sectors; "
            f"expected {expected_audio_sectors}"
        )
    if len(sectors) != expected_audio_sectors * NATIVE_MONO_INTERLEAVE:
        raise ValueError(
            f"{destination} has {len(sectors)} sectors; expected "
            f"{expected_audio_sectors * NATIVE_MONO_INTERLEAVE}"
        )
    if sectors[-1][2] & 0x80 == 0:
        raise ValueError(f"{destination} has no EOF marker")
    return len(sectors)


def extract(bank: Path, output: Path, prefix: str) -> None:
    data = bank.read_bytes()
    if not data or len(data) % SECTOR_SIZE:
        raise ValueError(f"{bank} is not a raw 2336-byte-sector XA bank")

    sectors = [
        data[offset : offset + SECTOR_SIZE]
        for offset in range(0, len(data), SECTOR_SIZE)
    ]
    # Channel zero is the bank's non-driver/control lane.  The twelve
    # playable drivers occupy XA filter channels 1..12 in roster order.
    # Treating channel zero as Chassey shifted every imported voice by one and
    # made Sid consume Molo's line.
    for output_channel in range(DRIVER_COUNT):
        source_channel = FIRST_DRIVER_CHANNEL + output_channel
        selected: list[bytes] = []
        for source in sectors:
            # Raw Mode-2 sectors begin with the duplicated XA subheader.
            # Retain only audio sectors for this roster-ordered channel.
            if source[1] != source_channel or source[2] & 0x04 == 0:
                continue
            selected.append(source)
        if not selected:
            raise ValueError(
                f"{bank} has no audio sectors for channel {source_channel}"
            )

        # V8's shared bank interleaves sixteen result channels. V8:2's native
        # result files use eight-sector interleave for a 37.8 kHz mono XA
        # stream: one accepted sector supplies exactly the audio consumed
        # while the next seven CD sectors pass. Preserve every selected
        # speech sector byte-for-byte (apart from the required channel/EOF
        # subheader fields) and insert filter-rejected copies only to retain
        # that native cadence. Packing the selected sectors contiguously reads
        # the whole line eight times too quickly and lets the native Stop
        # command discard most of its queued audio.
        output_sectors: list[bytearray] = []
        for source in selected:
            audio = bytearray(source)
            audio[1] = 0
            audio[5] = 0
            audio[2] &= ~0x80
            audio[6] &= ~0x80
            output_sectors.append(audio)
            for _ in range(NATIVE_MONO_INTERLEAVE - 1):
                filler = bytearray(source)
                filler[1] = 1
                filler[5] = 1
                filler[2] &= ~0x80
                filler[6] &= ~0x80
                output_sectors.append(filler)

        output_sectors[-1][2] |= 0x80
        output_sectors[-1][6] |= 0x80
        destination = output / f"{prefix}{output_channel:02d}.XA"
        destination.write_bytes(b"".join(output_sectors))
        sector_count = validate(destination, len(selected))
        print(
            f"{destination}: source-channel={source_channel} "
            f"{len(selected)} speech + "
            f"{sector_count - len(selected)} cadence sectors, "
            f"{destination.stat().st_size} bytes"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--victory", type=Path, required=True)
    parser.add_argument("--defeat", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    extract(args.victory, args.output, "V")
    extract(args.defeat, args.output, "D")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
