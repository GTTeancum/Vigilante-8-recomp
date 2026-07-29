#!/usr/bin/env python3
"""Report serialized little-endian references to selected ISOP Model3 objects."""

from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from extract_vehicle_lods import decode_bundle, find_models


def all_occurrences(data: bytes, needle: bytes) -> list[int]:
    offsets = []
    cursor = 0
    while True:
        offset = data.find(needle, cursor)
        if offset < 0:
            return offsets
        offsets.append(offset)
        cursor = offset + 1


def inspect(
    path: Path, selected: set[int], arbitrary_offsets: set[int]
) -> dict[str, object]:
    data, container = decode_bundle(path)
    models = find_models(data)
    if not selected:
        selected = {model.offset for model in models}
    records = []
    for model in models:
        if model.offset not in selected:
            continue
        references = all_occurrences(data, struct.pack("<I", model.offset))
        records.append(
            {
                "model_offset": model.offset,
                "vertex_count": model.vertex_count,
                "triangle_count": model.triangle_count,
                "references": [
                    {
                        "offset": reference,
                        "aligned": reference % 4 == 0,
                        "context_hex": data[
                            max(0, reference - 32) : reference + 36
                        ].hex(" "),
                    }
                    for reference in references
                ],
            }
        )
    arbitrary_records = []
    for value in sorted(arbitrary_offsets):
        references = all_occurrences(data, struct.pack("<I", value))
        arbitrary_records.append(
            {
                "value": value,
                "references": [
                    {
                        "offset": reference,
                        "aligned": reference % 4 == 0,
                        "context_hex": data[
                            max(0, reference - 32) : reference + 36
                        ].hex(" "),
                    }
                    for reference in references
                ],
            }
        )
    return {
        "source": str(path.resolve()),
        "container": container,
        "models": records,
        "arbitrary_offsets": arbitrary_records,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("--model", action="append", default=[], type=lambda v: int(v, 0))
    parser.add_argument("--offset", action="append", default=[], type=lambda v: int(v, 0))
    parser.add_argument("--out", type=Path)
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None
    args = parser.parse_args(argv)
    result = inspect(args.input, set(args.model), set(args.offset))
    text = json.dumps(result, indent=2) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
