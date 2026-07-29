#!/usr/bin/env python3
"""Raise only Dreamland's water HEAD for a non-deployed underside render proof."""

from __future__ import annotations

import argparse
from pathlib import Path
import struct
import sys

from terrain_object_identity_audit import collect_heads


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument(
        "--world-y",
        type=float,
        default=12.0,
        help="diagnostic water-plane Y in host world units (default: 12)",
    )
    args = parser.parse_args()

    heads = [
        head for head in collect_heads(args.source)
        if head["name"] == "DreamlandWater"
    ]
    if len(heads) != 1:
        raise ValueError(
            f"expected one DreamlandWater HEAD, found {len(heads)}"
        )

    data = bytearray(args.source.read_bytes())
    stored_y = round(args.world_y * 131072.0) + 0x100000
    if not -(1 << 31) <= stored_y < (1 << 31):
        raise ValueError(f"diagnostic water Y is out of range: {stored_y}")
    # collect_heads reports the HEAD chunk start. The payload starts at +8,
    # and HEAD's stored Y is payload +0x0c.
    struct.pack_into(">i", data, heads[0]["off"] + 8 + 0x0C, stored_y)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(data)
    print(
        f"{args.output}: DreamlandWater y "
        f"{heads[0]['y']:.6f} -> {args.world_y:.6f}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise
