#!/usr/bin/env python3
"""Byte-exact audit of converted V8 result voices against the retail XA banks."""

from __future__ import annotations

import argparse
import json
from dataclasses import asdict, dataclass
from pathlib import Path


SECTOR_SIZE = 2336
DRIVER_COUNT = 12
FIRST_DRIVER_CHANNEL = 1
NATIVE_INTERLEAVE = 8


@dataclass(frozen=True)
class VoiceAudit:
    outcome: str
    driver_index: int
    source_channel: int
    source_sectors: int
    converted_sectors: int
    payload_bytes_verified: int
    path: str


def sectors(path: Path) -> list[bytes]:
    data = path.read_bytes()
    if not data or len(data) % SECTOR_SIZE:
        raise ValueError(f"{path} is not a raw 2336-byte-sector XA file")
    return [
        data[offset : offset + SECTOR_SIZE]
        for offset in range(0, len(data), SECTOR_SIZE)
    ]


def audit_bank(
    source_path: Path,
    converted_root: Path,
    outcome: str,
    prefix: str,
) -> list[VoiceAudit]:
    source = sectors(source_path)
    results: list[VoiceAudit] = []
    for driver_index in range(DRIVER_COUNT):
        source_channel = FIRST_DRIVER_CHANNEL + driver_index
        expected = [
            sector
            for sector in source
            if sector[1] == source_channel and sector[2] & 0x04
        ]
        destination = converted_root / f"{prefix}{driver_index:02d}.XA"
        converted = sectors(destination)
        if len(convertverted := converted) != len(expected) * NATIVE_INTERLEAVE:
            raise ValueError(
                f"{destination}: {len(convertverted)} sectors, expected "
                f"{len(expected) * NATIVE_INTERLEAVE}"
            )
        for speech_index, original in enumerate(expected):
            group = converted[
                speech_index * NATIVE_INTERLEAVE :
                (speech_index + 1) * NATIVE_INTERLEAVE
            ]
            for cadence_index, candidate in enumerate(group):
                if candidate[:4] != candidate[4:8]:
                    raise ValueError(
                        f"{destination}: duplicate subheader mismatch at "
                        f"speech={speech_index} cadence={cadence_index}"
                    )
                expected_channel = 0 if cadence_index == 0 else 1
                if candidate[1] != expected_channel:
                    raise ValueError(
                        f"{destination}: channel={candidate[1]} at "
                        f"speech={speech_index} cadence={cadence_index}, "
                        f"expected {expected_channel}"
                    )
                # Only duplicated channel/EOF subheader bytes may differ.
                # The coding byte and every byte consumed by the ADPCM decoder
                # must remain byte-for-byte identical to the retail sector.
                if candidate[3] != original[3] or candidate[8:] != original[8:]:
                    raise ValueError(
                        f"{destination}: audio payload differs from retail "
                        f"channel {source_channel} at speech={speech_index} "
                        f"cadence={cadence_index}"
                    )
                if cadence_index == 0 and candidate[2] & 0x04 == 0:
                    raise ValueError(
                        f"{destination}: accepted sector {speech_index} "
                        "lost its XA audio flag"
                    )
        results.append(
            VoiceAudit(
                outcome=outcome,
                driver_index=driver_index,
                source_channel=source_channel,
                source_sectors=len(expected),
                converted_sectors=len(converted),
                payload_bytes_verified=len(expected)
                * NATIVE_INTERLEAVE
                * (SECTOR_SIZE - 8),
                path=str(destination),
            )
        )
    return results


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--voice-root",
        type=Path,
        default=Path("V8_2_LOOSE/SHARED/V8VOICE"),
    )
    parser.add_argument("--json", type=Path)
    args = parser.parse_args()
    root = args.voice_root.resolve()
    results = [
        *audit_bank(root / "VICTORY.XA", root, "victory", "V"),
        *audit_bank(root / "DEFEAT.XA", root, "defeat", "D"),
    ]
    report = {
        "passed": True,
        "voices": len(results),
        "payload_bytes_verified": sum(
            result.payload_bytes_verified for result in results
        ),
        "results": [asdict(result) for result in results],
    }
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(
            json.dumps(report, indent=2) + "\n", encoding="utf-8"
        )
    print(
        "[ResultVoiceContent] PASS "
        f"voices={report['voices']} "
        f"payload-bytes={report['payload_bytes_verified']} "
        "mapping=retail-channels-1..12"
    )
    for result in results:
        print(
            f"  {result.outcome} driver={result.driver_index:02d} "
            f"source-channel={result.source_channel} "
            f"speech-sectors={result.source_sectors}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
