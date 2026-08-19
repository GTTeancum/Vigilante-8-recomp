#!/usr/bin/env python3
"""Compare matched terrain OBJ/HEAD records between V8 and V8:2 EXPs."""
from __future__ import annotations

import argparse
from collections import defaultdict
from pathlib import Path
import struct

import v8_n64_level as iff


FIELDS = (
    ("script", ">B", 0),
    ("type", ">B", 1),
    ("id", ">h", 2),
    ("flags", ">I", 4),
    ("x", ">i", 8),
    ("y", ">i", 12),
    ("z", ">i", 16),
    ("rot0", ">h", 20),
    ("rot1", ">h", 22),
    ("rot2", ">h", 24),
    ("bank", ">h", 26),
    ("slot", ">h", 28),
    ("strength", ">i", 30),
)


def object_heads(path: Path) -> dict[str, list[bytes]]:
    result: dict[str, list[bytes]] = defaultdict(list)
    for _offset, tag, payload, parent in iff.iter_chunks(path.read_bytes()):
        if tag != b"HEAD" or parent != b"OBJ " or len(payload) < 34:
            continue
        name = payload[34:].split(b"\0", 1)[0].decode("ascii", "replace")
        result[name].append(payload)
    return result


def decoded(payload: bytes) -> dict[str, int]:
    return {
        name: struct.unpack_from(fmt, payload, offset)[0]
        for name, fmt, offset in FIELDS
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("v8", type=Path)
    parser.add_argument("v82", type=Path)
    args = parser.parse_args()

    v8_heads = object_heads(args.v8)
    v82_heads = object_heads(args.v82)
    matched = 0
    identical = 0
    changed_fields: dict[str, int] = defaultdict(int)

    print(f"V8={args.v8} objects={sum(map(len, v8_heads.values()))}")
    print(f"V82={args.v82} objects={sum(map(len, v82_heads.values()))}")
    for name in sorted(v8_heads.keys() & v82_heads.keys()):
        for index, (left, right) in enumerate(zip(v8_heads[name], v82_heads[name])):
            matched += 1
            if left == right:
                identical += 1
                continue
            left_fields = decoded(left)
            right_fields = decoded(right)
            changes = []
            for field, _fmt, _offset in FIELDS:
                if left_fields[field] == right_fields[field]:
                    continue
                changed_fields[field] += 1
                changes.append(
                    f"{field}={left_fields[field]}->{right_fields[field]}"
                )
            suffix = f"[{index}]" if len(v8_heads[name]) > 1 else ""
            print(f"CHANGED {name}{suffix}: " + ", ".join(changes))

    print(
        f"matched={matched} identical={identical} changed={matched - identical} "
        f"only_v8={len(v8_heads.keys() - v82_heads.keys())} "
        f"only_v82={len(v82_heads.keys() - v8_heads.keys())}"
    )
    print(
        "changed_fields="
        + ",".join(f"{name}:{count}" for name, count in sorted(changed_fields.items()))
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
