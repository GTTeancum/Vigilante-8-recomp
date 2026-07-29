#!/usr/bin/env python3
"""Decompress Vigilante 8 Arcade .ibz bundles into their native ISOP .ib form."""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
import zlib
from pathlib import Path


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def decompress_ibz(source_path: Path) -> bytes:
    source = source_path.read_bytes()
    if len(source) < 6:
        raise RuntimeError(f"{source_path} is too small to be an IBZ file")

    expected_size = struct.unpack_from("<I", source)[0]
    if source[4:6] not in (b"\x78\x01", b"\x78\x5e", b"\x78\x9c", b"\x78\xda"):
        raise RuntimeError(f"{source_path} has no zlib stream at offset 4")

    decompressor = zlib.decompressobj()
    output = decompressor.decompress(source[4:])
    output += decompressor.flush()
    if not decompressor.eof:
        raise RuntimeError(f"{source_path} contains a truncated zlib stream")
    if decompressor.unused_data:
        raise RuntimeError(
            f"{source_path} has {len(decompressor.unused_data)} unexpected trailing bytes"
        )
    if len(output) != expected_size:
        raise RuntimeError(
            f"{source_path} declares {expected_size} bytes but expands to {len(output)}"
        )
    if output[:4] != b"ISOP":
        raise RuntimeError(f"{source_path} output does not begin with ISOP")
    return output


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path, help="IBZ files or directories")
    parser.add_argument("--out", type=Path, required=True, help="Output directory")
    parser.add_argument("--manifest", type=Path, help="Write decompression metadata as JSON")
    args = parser.parse_args()

    input_paths: list[Path] = []
    for candidate in args.inputs:
        if candidate.is_dir():
            input_paths.extend(sorted(candidate.glob("*.ibz")))
        else:
            input_paths.append(candidate)
    if not input_paths:
        raise RuntimeError("no .ibz inputs found")

    output_root = args.out.resolve()
    output_root.mkdir(parents=True, exist_ok=True)
    records = []
    for source_path in input_paths:
        source_path = source_path.resolve()
        output = decompress_ibz(source_path)
        destination = output_root / f"{source_path.stem}.ib"
        destination.write_bytes(output)
        source_data = source_path.read_bytes()
        records.append(
            {
                "source": str(source_path),
                "source_size": len(source_data),
                "source_sha256": sha256(source_data),
                "output": str(destination),
                "output_size": len(output),
                "output_sha256": sha256(output),
            }
        )
        print(f"{source_path.name}: {len(source_data)} -> {len(output)} bytes")

    if args.manifest:
        manifest_path = args.manifest.resolve()
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        manifest_path.write_text(json.dumps(records, indent=2) + "\n", encoding="utf-8")
        print(f"Wrote {manifest_path}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
