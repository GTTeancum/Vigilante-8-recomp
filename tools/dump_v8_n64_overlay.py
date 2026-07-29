#!/usr/bin/env python3
"""Prepare a relocated V8 N64 overlay and explicit m2c jump tables."""

from __future__ import annotations

import argparse
from pathlib import Path

from v8_n64_level import be32, relocate_overlay_image


def parse_table(value: str) -> tuple[int, int]:
    try:
        offset_text, count_text = value.split(":", 1)
        offset = int(offset_text, 0)
        count = int(count_text, 0)
    except (ValueError, TypeError) as exc:
        raise argparse.ArgumentTypeError(
            "jump table must use OFFSET:COUNT"
        ) from exc
    if offset < 0 or count <= 0:
        raise argparse.ArgumentTypeError("invalid jump-table range")
    return offset, count


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("output_image", type=Path)
    parser.add_argument("output_tables", type=Path)
    parser.add_argument("--output-symbols", type=Path)
    parser.add_argument("--base", type=lambda value: int(value, 0), required=True)
    parser.add_argument(
        "--jump-table",
        action="append",
        default=[],
        type=parse_table,
        metavar="OFFSET:COUNT",
    )
    parser.add_argument(
        "--function",
        action="append",
        default=[],
        type=lambda value: int(value, 0),
        metavar="OFFSET",
    )
    args = parser.parse_args()

    image = relocate_overlay_image(args.input.read_bytes(), args.base)
    args.output_image.parent.mkdir(parents=True, exist_ok=True)
    args.output_image.write_bytes(image)

    lines = [".section .data", ""]
    symbol_lines: list[str] = []
    jump_targets: set[int] = set()
    for offset, count in args.jump_table:
        if offset + count * 4 > len(image):
            raise ValueError(
                f"jump table at 0x{offset:X} crosses the overlay image"
            )
        address = args.base + offset
        lines.append(f"jtbl_{address:08X}:")
        symbol_lines.append(
            f"jtbl_{address:08X} = 0x{address:08X}; "
            f"// type:jtbl size:0x{count * 4:X}"
        )
        for index in range(count):
            target = be32(image, offset + index * 4)
            jump_targets.add(target)
            lines.append(f"    .word .L{target:08X}")
        lines.append("")
    args.output_tables.parent.mkdir(parents=True, exist_ok=True)
    args.output_tables.write_text("\n".join(lines), encoding="ascii")
    if args.output_symbols is not None:
        symbol_lines.extend(
            f"func_{args.base + offset:08X} = "
            f"0x{args.base + offset:08X}; // type:func"
            for offset in sorted(set(args.function))
        )
        symbol_lines.extend(
            f".L{target:08X} = 0x{target:08X}; // type:jtbl_label"
            for target in sorted(jump_targets)
        )
        args.output_symbols.parent.mkdir(parents=True, exist_ok=True)
        args.output_symbols.write_text(
            "\n".join(symbol_lines) + "\n", encoding="ascii"
        )
    print(
        f"{args.output_image}: {len(image)} relocated bytes; "
        f"{len(args.jump_table)} jump tables"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
