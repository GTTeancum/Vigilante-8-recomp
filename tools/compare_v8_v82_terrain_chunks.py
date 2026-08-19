#!/usr/bin/env python3
"""Compare compact semantic terrain chunks in retail V8/V8:2 arena pairs."""

from __future__ import annotations

import argparse
from pathlib import Path

import v8_n64_level as iff


TAGS = (b"SUNA", b"COLS")


def chunks(path: Path) -> dict[bytes, bytes]:
    result: dict[bytes, bytes] = {}
    for _offset, tag, payload, _parent in iff.iter_chunks(path.read_bytes()):
        if tag in TAGS:
            result[tag] = payload
    return result


def words(payload: bytes) -> str:
    return " ".join(payload[index:index + 4].hex() for index in range(0, len(payload), 4))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("v8", type=Path)
    parser.add_argument("v82", type=Path)
    args = parser.parse_args()

    names = sorted(
        path.name
        for path in args.v8.glob("*.EXP")
        if (args.v82 / path.name).is_file()
    )
    for name in names:
        left = chunks(args.v8 / name)
        right = chunks(args.v82 / name)
        print(name)
        for tag in TAGS:
            a = left.get(tag, b"")
            b = right.get(tag, b"")
            print(f"  {tag.decode('ascii')} V8[{len(a)}]  {words(a)}")
            print(f"  {tag.decode('ascii')} V82[{len(b)}] {words(b)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
