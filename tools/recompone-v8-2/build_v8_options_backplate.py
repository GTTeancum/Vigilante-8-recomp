#!/usr/bin/env python3
"""Replace V8:2's native Options backplate inside OPTTABLE.TBL.

The Options left panel is record 58 in the retail shell table: a 240x421
PlayStation MDEC BS v2 still image.  This builder preserves every other table
record byte-for-byte, encodes the supplied artwork with psxavenc, rebuilds the
offset table, and emits a normal loose-file mod override.  Runtime VRAM
matching is deliberately not involved.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

from PIL import Image


ROOT = Path(__file__).resolve().parents[2]
MOD = ROOT / "mods" / "v8_classic_menu"
OPTIONS_RECORD = 58
NATIVE_SIZE = (240, 421)
ENCODE_SIZE = (240, 432)


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def split_table(data: bytes) -> list[bytes]:
    if len(data) < 8:
        raise ValueError("OPTTABLE.TBL is truncated")
    count = struct.unpack_from("<I", data)[0]
    if count <= OPTIONS_RECORD or count > 256:
        raise ValueError(f"implausible OPTTABLE.TBL record count {count}")
    table_end = 4 + (count + 1) * 4
    if table_end > len(data):
        raise ValueError("OPTTABLE.TBL offset table is truncated")
    offsets = struct.unpack_from(f"<{count + 1}I", data, 4)
    if (
        offsets[0] != table_end
        or tuple(sorted(offsets)) != offsets
        or offsets[-1] != len(data)
    ):
        raise ValueError("OPTTABLE.TBL offsets are invalid")
    return [data[offsets[i]:offsets[i + 1]] for i in range(count)]


def join_table(records: list[bytes]) -> bytes:
    table_end = 4 + (len(records) + 1) * 4
    offsets = [table_end]
    for record in records:
        offsets.append(offsets[-1] + len(record))
    return (
        struct.pack("<I", len(records))
        + struct.pack(f"<{len(offsets)}I", *offsets)
        + b"".join(records)
    )


def trim_sbs_frame(data: bytes) -> bytes:
    end = len(data.rstrip(b"\0"))
    end = (end + 3) & ~3
    frame = data[:end]
    if len(frame) < 8 or frame[2:4] != b"\x00\x38":
        raise ValueError("psxavenc output is not a PlayStation MDEC BS frame")
    if frame[6:8] != b"\x02\x00":
        raise ValueError("psxavenc output is not MDEC BS version 2")
    return frame


def encode_panel(source: Path, psxavenc: Path) -> tuple[bytes, str]:
    with Image.open(source) as image:
        if image.size != (224, 480):
            raise ValueError(
                f"Options source must be 224x480, got {image.width}x{image.height}"
            )
        fitted = image.convert("RGB").resize(
            NATIVE_SIZE, Image.Resampling.LANCZOS
        )
        padded = Image.new("RGB", ENCODE_SIZE)
        padded.paste(fitted, (0, 0))
        # The native height is not macroblock-aligned. Extend its last visible
        # row into the encoder-only padding so a decoder cannot acquire a dark
        # fringe at the authored 421-pixel crop.
        last_row = fitted.crop((0, NATIVE_SIZE[1] - 1, NATIVE_SIZE[0], NATIVE_SIZE[1]))
        padded.paste(
            last_row.resize(
                (NATIVE_SIZE[0], ENCODE_SIZE[1] - NATIVE_SIZE[1]),
                Image.Resampling.NEAREST,
            ),
            (0, NATIVE_SIZE[1]),
        )

    with tempfile.TemporaryDirectory(prefix="v82_options_mdec_") as temp_name:
        temp = Path(temp_name)
        bitmap = temp / "options_left_240x432.bmp"
        encoded = temp / "options_left.sbs"
        padded.save(bitmap)
        subprocess.run(
            [
                str(psxavenc),
                "-q",
                "-t",
                "sbs",
                "-v",
                "v2",
                "-s",
                "240x432",
                "-I",
                "-a",
                "32768",
                str(bitmap),
                str(encoded),
            ],
            creationflags=getattr(subprocess, "IDLE_PRIORITY_CLASS", 0),
            check=True,
        )
        frame = trim_sbs_frame(encoded.read_bytes())
    record = struct.pack("<HH", *NATIVE_SIZE) + frame
    return record, digest(frame)


def parse_args() -> argparse.Namespace:
    discovered = os.environ.get("PSXAVENC") or shutil.which("psxavenc")
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--source",
        type=Path,
        default=MOD / "source" / "options_left.png",
    )
    parser.add_argument(
        "--template",
        type=Path,
        default=ROOT / "V8_2_LOOSE" / "SHELL" / "OPTTABLE.TBL",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=MOD / "files" / "SHELL" / "OPTTABLE.TBL",
    )
    parser.add_argument(
        "--psxavenc",
        type=Path,
        default=Path(discovered) if discovered else None,
        help="psxavenc v0.3.1+ executable (or set PSXAVENC)",
    )
    args = parser.parse_args()
    if args.psxavenc is None:
        parser.error("psxavenc is required; pass --psxavenc or set PSXAVENC")
    return args


def main() -> int:
    args = parse_args()
    template = args.template.read_bytes()
    original_records = split_table(template)
    records = list(original_records)
    native_record = records[OPTIONS_RECORD]
    if len(native_record) < 12 or struct.unpack_from("<HH", native_record) != NATIVE_SIZE:
        raise ValueError(
            f"OPTTABLE record {OPTIONS_RECORD} is not the native 240x421 panel"
        )
    replacement, frame_hash = encode_panel(
        args.source.resolve(), args.psxavenc.resolve()
    )
    if len(replacement) > len(native_record):
        raise ValueError(
            "encoded Options panel exceeds its native table record: "
            f"{len(replacement)} > {len(native_record)} bytes"
        )
    # Keep the replacement record and the complete table sector-exact in size.
    # The native shell caches OPTTABLE.TBL's retail descriptor before the mod
    # override is resolved, so shortening the table leaves that descriptor one
    # sector too long in standalone-loose mode. MDEC frames carry their own word
    # count; zero-filling the record tail is the native-compatible padding form.
    replacement = replacement.ljust(len(native_record), b"\0")
    records[OPTIONS_RECORD] = replacement
    output = join_table(records)
    reparsed = split_table(output)
    if reparsed[OPTIONS_RECORD] != replacement:
        raise AssertionError("replacement Options record did not round-trip")
    for index, record in enumerate(reparsed):
        if index != OPTIONS_RECORD and record != original_records[index]:
            raise AssertionError(f"unrelated OPTTABLE record {index} changed")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(output)
    report = {
        "format": "v82-native-options-backplate-v1",
        "source": str(args.source.resolve()),
        "source_sha256": digest(args.source.read_bytes()),
        "template": str(args.template.resolve()),
        "template_sha256": digest(template),
        "record": OPTIONS_RECORD,
        "dimensions": list(NATIVE_SIZE),
        "codec": "PlayStation MDEC BS v2",
        "mdec_sha256": frame_hash,
        "native_record_sha256": digest(native_record),
        "replacement_record_sha256": digest(replacement),
        "output": str(args.output.resolve()),
        "output_bytes": len(output),
        "output_sha256": digest(output),
        "unchanged_records": len(records) - 1,
    }
    print(json.dumps(report, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
