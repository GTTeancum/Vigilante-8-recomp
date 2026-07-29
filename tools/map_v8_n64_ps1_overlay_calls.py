#!/usr/bin/env python3
"""Infer N64-to-PS1 main-engine call addresses from shared V8 overlays."""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from dataclasses import dataclass
from difflib import SequenceMatcher
from pathlib import Path
import struct

import rabbitizer


@dataclass(frozen=True)
class Export:
    name: str
    offset: int


@dataclass(frozen=True)
class Instruction:
    mnemonic: str
    target: int | None


def u32(data: bytes, offset: int, endian: str) -> int:
    return int.from_bytes(data[offset:offset + 4], endian)


def exports(
    data: bytes, endian: str, base: int = 0
) -> tuple[int, list[Export]]:
    image_size = u32(data, 0, endian)
    table = u32(data, 4, endian)
    if base <= table < base + image_size:
        table -= base
    result: list[Export] = []
    cursor = table
    while cursor + 8 <= image_size:
        name_offset = u32(data, cursor, endian)
        callback = u32(data, cursor + 4, endian)
        cursor += 8
        if name_offset == 0:
            break
        if base <= name_offset < base + image_size:
            name_offset -= base
        if base <= callback < base + image_size:
            callback -= base
        end = data.find(b"\0", name_offset, image_size)
        if end < 0:
            raise ValueError("unterminated overlay export name")
        result.append(Export(data[name_offset:end].decode("ascii"), callback))
    return image_size, result


def relocate_overlay(data: bytes, endian: str, base: int) -> bytes:
    """Relocate a V8 overlay so internal calls are not mistaken for helpers."""
    image_size = u32(data, 0, endian)
    if image_size < 8 or image_size > len(data):
        raise ValueError(f"invalid overlay image size 0x{image_size:X}")
    image = bytearray(data[:image_size])
    cursor = image_size
    relocated_words: set[int] = set()

    def write_word(offset: int, value: int) -> None:
        image[offset:offset + 4] = value.to_bytes(4, endian)

    def relocate_absolute(offset: int) -> None:
        if offset in relocated_words:
            return
        write_word(offset, (u32(image, offset, endian) + base) & 0xFFFFFFFF)
        relocated_words.add(offset)

    while cursor + 4 <= len(data):
        relocation = u32(data, cursor, endian)
        cursor += 4
        if relocation == 0xFFFFFFFF:
            return bytes(image)
        kind = relocation & 3
        offset = relocation & ~3
        if offset + 4 > image_size:
            raise ValueError(f"relocation 0x{relocation:X} outside image")
        word = u32(image, offset, endian)
        if kind == 0:
            relocate_absolute(offset)
            continue
        if kind == 1:
            if cursor + 4 > len(data):
                raise ValueError("truncated HI16 relocation")
            addend_offset = u32(data, cursor, endian)
            cursor += 4
            relocate_absolute(addend_offset)
            word = (word & 0xFFFF0000) | (
                ((word & 0xFFFF) + ((base + 0x8000) >> 16)) & 0xFFFF
            )
        elif kind == 2:
            word = (word & 0xFFFF0000) | (
                ((word & 0xFFFF) + base) & 0xFFFF
            )
        else:
            word = (word & 0xFC000000) | (
                ((word & 0x03FFFFFF) + (base >> 2)) & 0x03FFFFFF
            )
        write_word(offset, word)
    raise ValueError("overlay relocation stream has no terminator")


def external_calls(
    data: bytes, endian: str, base: int, start: int, end: int, image_size: int
) -> list[int]:
    result: list[int] = []
    for offset in range(start, end & ~3, 4):
        word = u32(data, offset, endian)
        opcode = word >> 26
        if opcode not in (2, 3):
            continue
        pc = base + offset
        target = ((pc + 4) & 0xF0000000) | ((word & 0x03FFFFFF) << 2)
        if not (base <= target < base + image_size):
            result.append(target)
    return result


def instructions(
    data: bytes, endian: str, base: int, start: int, end: int, image_size: int
) -> list[Instruction]:
    result: list[Instruction] = []
    for offset in range(start, end & ~3, 4):
        word = u32(data, offset, endian)
        decoded = rabbitizer.Instruction(word)
        mnemonic = decoded.getOpcodeName()
        target = None
        opcode = word >> 26
        if opcode in (2, 3):
            pc = base + offset
            candidate = (
                ((pc + 4) & 0xF0000000)
                | ((word & 0x03FFFFFF) << 2)
            )
            if not (base <= candidate < base + image_size):
                target = candidate
        result.append(Instruction(mnemonic, target))
    return result


def call_sequences(
    data: bytes, endian: str, base: int
) -> dict[str, list[int]]:
    image_size, items = exports(data, endian, base)
    starts = sorted({item.offset for item in items} | {image_size})
    result: dict[str, list[int]] = {}
    for item in items:
        end = next(value for value in starts if value > item.offset)
        result[item.name.lower()] = external_calls(
            data, endian, base, item.offset, end, image_size
        )
    return result


def instruction_sequences(
    data: bytes, endian: str, base: int
) -> dict[str, list[Instruction]]:
    image_size, items = exports(data, endian, base)
    starts = sorted({item.offset for item in items} | {image_size})
    result: dict[str, list[Instruction]] = {}
    for item in items:
        end = next(value for value in starts if value > item.offset)
        result[item.name.lower()] = instructions(
            data, endian, base, item.offset, end, image_size
        )
    return result


def aligned_call_pairs(
    left: list[Instruction], right: list[Instruction]
) -> list[tuple[int, int]]:
    matcher = SequenceMatcher(
        None,
        [item.mnemonic for item in left],
        [item.mnemonic for item in right],
        autojunk=False,
    )
    result: list[tuple[int, int]] = []
    previous_left = previous_right = 0
    for block in matcher.get_matching_blocks():
        gap_left = [
            item for item in left[previous_left:block.a]
            if item.target is not None
        ]
        gap_right = [
            item for item in right[previous_right:block.b]
            if item.target is not None
        ]
        if (
            len(gap_left) == len(gap_right)
            and [item.mnemonic for item in gap_left]
            == [item.mnemonic for item in gap_right]
        ):
            result.extend(
                (a.target, b.target)
                for a, b in zip(gap_left, gap_right)
                if a.target is not None and b.target is not None
            )
        for index in range(block.size):
            a = left[block.a + index]
            b = right[block.b + index]
            if a.target is not None and b.target is not None:
                result.append((a.target, b.target))
        previous_left = block.a + block.size
        previous_right = block.b + block.size
    return result


def contextual_call_pairs(
    left: list[Instruction], right: list[Instruction], radius: int = 10
) -> list[tuple[int, int]]:
    def signature(items: list[Instruction], index: int) -> list[str]:
        start = max(0, index - radius)
        end = min(len(items), index + radius + 1)
        return [
            "call" if item.target is not None else item.mnemonic
            for item in items[start:end]
        ]

    right_calls = [
        (index, item)
        for index, item in enumerate(right)
        if item.target is not None
    ]
    result: list[tuple[int, int]] = []
    for left_index, left_item in enumerate(left):
        if left_item.target is None:
            continue
        candidates: list[tuple[float, int]] = []
        left_signature = signature(left, left_index)
        for right_index, right_item in right_calls:
            if right_item.mnemonic != left_item.mnemonic:
                continue
            score = SequenceMatcher(
                None,
                left_signature,
                signature(right, right_index),
                autojunk=False,
            ).ratio()
            candidates.append((score, right_item.target or 0))
        if not candidates:
            continue
        candidates.sort(reverse=True)
        best_score, best_target = candidates[0]
        runner_up = candidates[1][0] if len(candidates) > 1 else 0.0
        if best_score >= 0.58 and best_score - runner_up >= 0.04:
            result.append((left_item.target, best_target))
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("--minimum", type=int, default=2)
    args = parser.parse_args()

    votes: dict[int, Counter[int]] = defaultdict(Counter)
    details: list[str] = []
    for n64_path in sorted(args.directory.glob("*_N64.DLL")):
        stem = n64_path.name.removesuffix("_N64.DLL")
        ps1_path = args.directory / f"{stem}.DLL"
        if not ps1_path.exists():
            continue
        n64_data = n64_path.read_bytes()
        ps1_data = ps1_path.read_bytes()
        n64_data = relocate_overlay(n64_data, "big", 0x80200000)
        ps1_data = relocate_overlay(ps1_data, "little", 0x80100000)
        n64 = call_sequences(n64_data, "big", 0x80200000)
        ps1 = call_sequences(ps1_data, "little", 0x80100000)
        n64_instructions = instruction_sequences(
            n64_data, "big", 0x80200000
        )
        ps1_instructions = instruction_sequences(
            ps1_data, "little", 0x80100000
        )
        for name in sorted(n64.keys() & ps1.keys()):
            left, right = n64[name], ps1[name]
            for source, target in aligned_call_pairs(
                n64_instructions[name], ps1_instructions[name]
            ):
                votes[source][target] += 2
            for source, target in contextual_call_pairs(
                n64_instructions[name], ps1_instructions[name]
            ):
                votes[source][target] += 1
            if len(left) != len(right):
                details.append(
                    f"{stem}:{name} calls differ {len(left)} != {len(right)}"
                )
                continue
            for source, target in zip(left, right):
                votes[source][target] += 1

    for source in sorted(votes):
        target, count = votes[source].most_common(1)[0]
        total = sum(votes[source].values())
        if count >= args.minimum or count == total:
            alternatives = ", ".join(
                f"0x{address:08X}:{votes_count}"
                for address, votes_count in votes[source].most_common()
            )
            print(
                f"0x{source:08X} -> 0x{target:08X} "
                f"({count}/{total}; {alternatives})"
            )
    if details:
        print("\n# Unpaired export call sequences")
        print("\n".join(details))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
