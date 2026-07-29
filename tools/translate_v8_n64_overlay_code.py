#!/usr/bin/env python3
"""Translate a V8 N64 overlay's MIPS/code metadata for PS1 recompilation.

This is an analysis-stage semantic translation, not a ROM payload wrapper:
instructions become little-endian R3000 words, pointer-bearing data words use
the native V8 relocation stream, and the original export names remain text.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import struct

from v8_n64_level import FormatError, be32


DREAMLAND_FUNCTIONS = (
    # Internal callback installed by DreamLnd at object+0x64.  It is not
    # present in the overlay export table, but it is native executable code
    # and must be translated/recompiled with the named exports.
    ("DreamContactActor", 0x02C0, 0x063C),
    ("DreamLnd", 0x063C, 0x09E8),
    ("Butterfly", 0x09E8, 0x0A3C),
    ("cobblebridge", 0x0A3C, 0x0B84),
    ("DBridge", 0x0B84, 0x0CAC),
    ("Rainbow", 0x0CAC, 0x0E1C),
    ("well", 0x0E1C, 0x10A8),
    ("castle", 0x10A8, 0x1248),
    ("Canon", 0x1248, 0x150C),
    ("DreamProjectileSpawn", 0x150C, 0x15EC),
    ("DreamGoodProjectile", 0x15EC, 0x1904),
    ("mushroom_good", 0x1904, 0x1B78),
    ("DreamBadProjectile", 0x1B78, 0x1E30),
    ("mushroom_bad", 0x1E30, 0x20EC),
    ("DreamOrbitingActor", 0x20EC, 0x2200),
    ("DreamActorDestroy", 0x2200, 0x2254),
    ("DreamActorBurst", 0x2254, 0x23C8),
    ("DreamActorPathStart", 0x23C8, 0x243C),
    ("DreamActorPathStep", 0x243C, 0x2690),
    ("Knight", 0x2690, 0x2880),
    ("Footman_Walk", 0x2880, 0x2B50),
    ("Chicken_Walk", 0x2B50, 0x2FE0),
    ("Piggy", 0x2FE0, 0x3208),
)

# The N64 port retained the engine's high-level object ABI, but the main
# executable was relinked and several platform helpers have no PS1 address.
# Every target below was recovered from cross-version terrain-overlay call
# sequences, then checked against the PS1 decompilation's argument semantics.
# The three 0x8FFF01xx entries are explicit PS1 compatibility routines
# registered by V8DreamlandCompat; they are not unresolved calls or payload
# shims.
DREAMLAND_N64_TO_PS1_CALLS = {
    0x80128EA8: 0x80012068,
    0x8012DD14: 0x8004C800,
    0x8012DDD8: 0x8004C844,
    0x8012DEB4: 0x8004C6E4,
    0x8012ECD4: 0x80043408,
    0x8012F5B8: 0x800434F8,
    0x8012FAA8: 0x8004DAB4,
    0x80132BD8: 0x80016AAC,
    0x80132CF0: 0x80016BD8,
    0x80132EB8: 0x80016DA8,
    0x80133094: 0x80016FA8,
    0x801331E4: 0x800170C8,
    0x80133254: 0x80017160,
    0x80133644: 0x80017594,
    0x80136AEC: 0x8001BDDC,
    0x80136FDC: 0x8001AC08,
    0x80137028: 0x8001AC44,
    0x80137184: 0x8001ADD0,
    0x801372EC: 0x8001AF48,
    0x801373D0: 0x8001B038,
    0x80137458: 0x8001B0C4,
    0x80137CD8: 0x8001D470,
    0x80137DAC: 0x8001D564,
    0x80137DE4: 0x8001D5A0,
    0x80137E50: 0x8001D624,
    0x80137EB8: 0x8001D68C,
    0x80137F0C: 0x8001D6E0,
    0x80137F30: 0x8001D708,
    0x80137F6C: 0x8001D748,
    0x8013805C: 0x8001D840,
    0x8013843C: 0x8001DC1C,
    0x8013A010: 0x8001F974,
    0x8013A658: 0x8001FFD4,
    0x8013A918: 0x800202F4,
    0x8013A980: 0x8002036C,
    0x8013ABDC: 0x800205F8,
    0x8013AC00: 0x80020620,
    0x8013AD14: 0x80020744,
    0x8013AD44: 0x80020778,
    0x8013AD88: 0x800207C4,
    0x8013ADB8: 0x800207F8,
    0x8013ADFC: 0x80020844,
    0x8013AE40: 0x80020890,
    0x8013B998: 0x80021808,
    # N64 extended nearest-object query. The PS1 engine only retains a
    # narrower retail variant, so Dreamland uses the native compatibility
    # implementation back-ported in V8DreamlandCompat.
    0x8013C1D0: 0x8FFF010C,
    0x8013C3EC: 0x80022120,
    0x8013C6B0: 0x80022320,
    0x8013C728: 0x8002239C,
    0x8013DF40: 0x80023D00,
    0x8013EA90: 0x80024718,
    0x8013F698: 0x80025400,
    0x8013F8E0: 0x80025648,
    0x801461C0: 0x8002C3AC,
    0x801466D4: 0x8002C958,
    0x80146710: 0x8FFF0100,
    0x8014A6A0: 0x80031294,
    0x8014A864: 0x80031454,
    0x80155E1C: 0x8003D080,
    0x801573BC: 0x8003E76C,
    0x80158810: 0x8003FC94,
    0x801587CC: 0x8003FC50,
    0x80158884: 0x8003FD24,
    0x80158928: 0x8003FDCC,
    0x80158A04: 0x8003FEA8,
    0x80159900: 0x8FFF0104,
    0x80159E54: 0x8FFF0108,
    0x8015ACC8: 0x80042390,
    0x8015AFB0: 0x80042698,
    0x8015B024: 0x80042724,
    0x8015B57C: 0x80042CDC,
    0x8015BFD4: 0x8004410C,
    0x8015C098: 0x800441C8,
    0x8015C0D8: 0x8FFF0118,
    0x8015C2C8: 0x80044574,
    0x8015C5A4: 0x800446DC,
    # N64 uses a 198-entry global SFX bank while PS1 MAIN.SND has 66
    # entries. The compatibility calls translate Dreamland's four decoded
    # waveform identities before entering the otherwise equivalent PS1
    # playback routines.
    0x8015C844: 0x8FFF0110,
    0x8015C898: 0x8FFF0114,
    0x8015CA08: 0x800449BC,
    0x8015CB14: 0x80044AC8,
    0x8015E2CC: 0x80045088,
    0x8015E378: 0x80045134,
}

DREAMLAND_N64_TO_PS1_GLOBALS = {
    # N64 0x80157410 is the callable child-dispatch entry following
    # func_801573BC.  The PS1 linker split that entry into its own symbol at
    # 0x8003E7B4; preserving the N64 byte delta would land at 0x8003E7C0 in
    # the middle of the PS1 prologue.
    0x80157410: 0x8003E7B4,
    0x801829B0: 0x800607B4,
    0x8019015C: 0x800605F8,
    0x801A0394: 0x800659FC,
    0x801A03A8: 0x80065A10,
    0x801A03B0: 0x80065A18,
    0x801A03E0: 0x80065A50,
    0x801A04B0: 0x800737E8,
}


def relocation_entries(data: bytes, image_size: int) -> list[tuple[int, int | None]]:
    result: list[tuple[int, int | None]] = []
    cursor = image_size
    while cursor + 4 <= len(data):
        entry = be32(data, cursor)
        cursor += 4
        if entry == 0xFFFFFFFF:
            return result
        symbol = None
        if (entry & 3) == 1:
            if cursor + 4 > len(data):
                raise FormatError("truncated HI16 relocation")
            symbol = be32(data, cursor)
            cursor += 4
        result.append((entry, symbol))
    raise FormatError("overlay relocation stream has no terminator")


def translate(data: bytes, code_start: int, code_end: int) -> bytes:
    image_size = be32(data, 0)
    if not (0 <= code_start < code_end <= image_size <= len(data)):
        raise FormatError("invalid code/image range")
    result = bytearray(data)

    # RecompOne reads these fixed header fields before applying the overlay's
    # relocation program, so they must be native little-endian even when the
    # N64 stream does not explicitly list offset zero as a relocation target.
    struct.pack_into("<I", result, 0, image_size)
    struct.pack_into("<I", result, 4, be32(data, 4))

    # Every instruction is a numeric word; changing byte order does not alter
    # any opcode, immediate, or delay-slot relationship.
    for offset in range(code_start, code_end, 4):
        struct.pack_into("<I", result, offset, be32(data, offset))

    # Translate direct engine calls. Internal overlay J/JAL targets remain
    # untouched and are relocated normally by RecompOne.
    for offset in range(code_start, code_end, 4):
        word = be32(data, offset)
        if word >> 26 not in (2, 3):
            continue
        target = 0x80000000 | ((word & 0x03FFFFFF) << 2)
        translated_target = DREAMLAND_N64_TO_PS1_CALLS.get(target)
        if translated_target is None:
            continue
        translated_word = (
            (word & 0xFC000000) |
            ((translated_target & 0x0FFFFFFF) >> 2)
        )
        struct.pack_into("<I", result, offset, translated_word)

    # Translate fixed main-RAM references. The compiler emits each of these
    # as a LUI followed shortly by an I-type load/store/add using that register.
    # Match the actual base register and signed low half, so unrelated values
    # sharing a high half cannot be changed.
    for source, target in DREAMLAND_N64_TO_PS1_GLOBALS.items():
        source_hi = ((source + 0x8000) >> 16) & 0xFFFF
        source_lo = source & 0xFFFF
        target_hi = ((target + 0x8000) >> 16) & 0xFFFF
        target_lo = target & 0xFFFF
        for offset in range(code_start, code_end, 4):
            lui = be32(data, offset)
            if lui >> 26 != 0x0F or (lui & 0xFFFF) != source_hi:
                continue
            register = (lui >> 16) & 0x1F
            for use_offset in range(
                offset + 4, min(code_end, offset + 1024), 4
            ):
                use = be32(data, use_offset)
                opcode = use >> 26
                base = (use >> 21) & 0x1F
                if base != register or (use & 0xFFFF) != source_lo:
                    continue
                if opcode not in {
                    0x08, 0x09, 0x0C, 0x0D, 0x20, 0x21, 0x23,
                    0x24, 0x25, 0x28, 0x29, 0x2B,
                }:
                    continue
                translated_lui = (lui & 0xFFFF0000) | target_hi
                translated_use = (use & 0xFFFF0000) | target_lo
                struct.pack_into("<I", result, offset, translated_lui)
                struct.pack_into("<I", result, use_offset, translated_use)
                break

    # Absolute image words include the overlay header, export pointers, jump
    # tables and callback/data pointers. Other relocation forms point into
    # instruction words already converted above.
    entries = relocation_entries(data, image_size)
    for entry, _symbol in entries:
        if (entry & 3) != 0:
            continue
        offset = entry & ~3
        struct.pack_into("<I", result, offset, be32(data, offset))

    # Dreamland's final internal sound-choice table is three signed BE16
    # values plus its native sentinel.
    if code_end == 0x3208 and image_size >= 0x3218:
        for offset in range(0x3210, 0x3218, 2):
            struct.pack_into("<H", result, offset, int.from_bytes(
                data[offset:offset + 2], "big"
            ))

    # Preserve the exact relocation program while changing its word order.
    cursor = image_size
    while cursor + 4 <= len(data):
        value = be32(data, cursor)
        struct.pack_into("<I", result, cursor, value)
        cursor += 4
        if value == 0xFFFFFFFF:
            break
        if (value & 3) == 1:
            symbol = be32(data, cursor)
            struct.pack_into("<I", result, cursor, symbol)
            cursor += 4
    return bytes(result)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("function_map", type=Path)
    parser.add_argument("--base", type=lambda value: int(value, 0),
                        default=0x80100000)
    parser.add_argument("--code-start", type=lambda value: int(value, 0),
                        default=0x2C0)
    parser.add_argument("--code-end", type=lambda value: int(value, 0),
                        default=0x3208)
    args = parser.parse_args()

    translated = translate(
        args.input.read_bytes(), args.code_start, args.code_end
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(translated)
    functions = [
        {
            "address": f"0x{args.base + start:08X}",
            "name": name,
            "size": end - start,
        }
        for name, start, end in DREAMLAND_FUNCTIONS
    ]
    args.function_map.parent.mkdir(parents=True, exist_ok=True)
    args.function_map.write_text(
        json.dumps({"functions": functions, "labels": []}, indent=2) + "\n",
        encoding="utf-8",
    )
    print(
        f"{args.output}: {len(translated)} bytes; "
        f"{len(functions)} functions"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
