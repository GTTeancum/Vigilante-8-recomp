#!/usr/bin/env python3
"""Build the native PS1 overlay directory for the Dreamland arena.

The executable callbacks are host-dispatched in the PC port, but the original
V8 loader still consumes this ordinary PS1 overlay export table.  Every export
has its own native MIPS return stub so no retail terrain DLL is used as a
template or runtime dependency.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import struct


EXPORTS = (
    ("DreamLnd", 0x200),
    ("Butterfly", 0x208),
    ("cobblebridge", 0x210),
    ("DBridge", 0x218),
    ("Rainbow", 0x220),
    ("well", 0x228),
    ("castle", 0x230),
    ("Canon", 0x238),
    ("mushroom_good", 0x240),
    ("mushroom_bad", 0x248),
    ("Knight", 0x250),
    ("Footman_Walk", 0x258),
    ("Chicken_Walk", 0x260),
    ("Piggy", 0x268),
)


def build() -> bytes:
    export_table = 8
    string_cursor = export_table + (len(EXPORTS) + 1) * 8
    strings = bytearray()
    name_offsets: list[int] = []
    for name, _callback in EXPORTS:
        name_offsets.append(string_cursor + len(strings))
        strings += name.encode("ascii") + b"\0"
        while len(strings) & 3:
            strings.append(0)

    image_size = 0x270
    output = bytearray(image_size)
    struct.pack_into("<II", output, 0, image_size, export_table)
    for index, ((_, callback), name_offset) in enumerate(
        zip(EXPORTS, name_offsets)
    ):
        struct.pack_into(
            "<II", output, export_table + index * 8, name_offset, callback
        )
    output[string_cursor:string_cursor + len(strings)] = strings

    # jr ra / nop. Dispatcher replaces each linked address with its semantic
    # host callback; these remain valid native PS1 stubs for loader integrity.
    stub = struct.pack("<II", 0x03E00008, 0)
    for _name, callback in EXPORTS:
        output[callback:callback + len(stub)] = stub
    relocations = [4]
    for index in range(len(EXPORTS)):
        relocations.extend(
            (export_table + index * 8, export_table + index * 8 + 4)
        )
    return bytes(output) + struct.pack(
        f"<{len(relocations) + 1}I", *relocations, 0xFFFFFFFF
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    data = build()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(data)
    print(f"{args.output}: {len(data)} bytes, {len(EXPORTS)} exports")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
