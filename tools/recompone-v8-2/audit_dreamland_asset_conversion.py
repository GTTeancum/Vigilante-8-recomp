#!/usr/bin/env python3
"""Audit Dreamland's V8:2 asset conversion against retail arena ports."""

from __future__ import annotations

import argparse
from collections import Counter
import hashlib
from pathlib import Path
import struct
import sys


REPO = Path(__file__).resolve().parents[2]
TOOLS = REPO / "tools"
ADDONS = TOOLS / "blender_addons"
sys.path.insert(0, str(TOOLS))
sys.path.insert(0, str(ADDONS))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from vigilante8_vehicle_tools import iff
import port_dreamland_assets as dreamland


def terrain(path: Path) -> iff.IffChunk:
    roots = list(iff.parse(path.read_bytes()).forms(b"TERR"))
    if len(roots) != 1:
        raise ValueError(f"{path} has {len(roots)} TERR forms")
    return roots[0]


def chunks(root: iff.IffChunk) -> dict[bytes, list[iff.IffChunk]]:
    result: dict[bytes, list[iff.IffChunk]] = {}
    for child in root.children:
        key = child.form_type if child.is_form else child.tag
        result.setdefault(key, []).append(child)
    return result


def digest(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest().upper()


def tile_average_5bit(
    width: int,
    height: int,
    palette: tuple[int, ...],
    pixels: bytes,
    x: int,
    y: int,
) -> bytes:
    totals = [0, 0, 0, 0]
    count = 0
    for row in range(y, min(y + 48, height)):
        for column in range(x, min(x + 48, width)):
            color = palette[pixels[row * width + column]]
            totals[0] += color & 0x1F
            totals[1] += (color >> 5) & 0x1F
            totals[2] += (color >> 10) & 0x1F
            totals[3] += 255 if color & 0x8000 else 0
            count += 1
    if count == 0:
        return b"\0\0\0\0"
    rgb = [dreamland._expand5(total // count) for total in totals[:3]]
    return bytes(rgb + [totals[3] // count])


def tile_average_rounded(
    width: int,
    height: int,
    palette: tuple[int, ...],
    pixels: bytes,
    x: int,
    y: int,
) -> bytes:
    totals = [0, 0, 0, 0]
    count = 0
    for row in range(y, min(y + 48, height)):
        for column in range(x, min(x + 48, width)):
            color = palette[pixels[row * width + column]]
            totals[0] += dreamland._expand5(color & 0x1F)
            totals[1] += dreamland._expand5((color >> 5) & 0x1F)
            totals[2] += dreamland._expand5((color >> 10) & 0x1F)
            totals[3] += 255 if color & 0x8000 else 0
            count += 1
    if count == 0:
        return b"\0\0\0\0"
    return bytes(
        [(total + count // 2) // count for total in totals[:3]] +
        [totals[3] // count]
    )


def tile_average_linear(
    width: int,
    height: int,
    palette: tuple[int, ...],
    pixels: bytes,
    x: int,
    y: int,
    rounded: bool,
) -> bytes:
    totals = [0, 0, 0, 0]
    count = 0
    for row in range(y, min(y + 48, height)):
        for column in range(x, min(x + 48, width)):
            color = palette[pixels[row * width + column]]
            totals[0] += color & 0x1F
            totals[1] += (color >> 5) & 0x1F
            totals[2] += (color >> 10) & 0x1F
            totals[3] += 255 if color & 0x8000 else 0
            count += 1
    if count == 0:
        return b"\0\0\0\0"
    denominator = 31 * count
    bias = denominator // 2 if rounded else 0
    return bytes(
        [(total * 255 + bias) // denominator for total in totals[:3]] +
        [totals[3] // count]
    )


def tile_average_native32(
    width: int,
    palette: tuple[int, ...],
    pixels: bytes,
    x: int,
    y: int,
    centered: bool,
) -> bytes:
    totals = [0, 0, 0, 0]
    for row in range(32):
        sy = y + (row * 3 + (1 if centered else 0)) // 2
        for column in range(32):
            sx = x + (column * 3 + (1 if centered else 0)) // 2
            color = palette[pixels[sy * width + sx]]
            totals[0] += color & 0x1F
            totals[1] += (color >> 5) & 0x1F
            totals[2] += (color >> 10) & 0x1F
            totals[3] += 255 if color & 0x8000 else 0
    denominator = 31 * 1024
    return bytes(
        [(total * 255 + denominator // 2) // denominator for total in totals[:3]] +
        [totals[3] // 1024]
    )


def compare_pair(v8_path: Path, v82_path: Path) -> list[str]:
    left = chunks(terrain(v8_path))
    right = chunks(terrain(v82_path))
    lines = [v8_path.name]

    tinf = left[b"TINF"][0].payload
    xbmp = left[b"XBMP"][0].payload
    target_xbmp = right[b"XBMP"][0].payload
    xtin = right[b"XTIN"][0].payload
    if len(tinf) != 256 * 40 or len(xtin) != 256 * 36:
        raise ValueError(f"{v8_path.name} has unexpected TINF/XTIN sizes")
    width, height, palette, pixels = dreamland._decode_xbmp(xbmp)
    target_width, target_height, target_palette, target_pixels = (
        dreamland._decode_xbmp(target_xbmp)
    )
    active = 0
    sentinels: dict[tuple[int, int], set[bytes]] = {}
    mismatches = 0
    mismatch_offsets: Counter[int] = Counter()
    mismatch_examples: list[str] = []
    average_mismatches: Counter[str] = Counter()
    for index in range(256):
        source = tinf[index * 40:(index + 1) * 40]
        target = xtin[index * 36:(index + 1) * 36]
        x = struct.unpack_from(">H", source, 2)[0]
        y = struct.unpack_from(">H", source, 4)[0]
        if x == 0xFFFF or y == 0xFFFF:
            sentinels.setdefault((x, y), set()).add(target)
            continue
        if x % 48 or y % 48 or x // 48 > 15 or y // 48 > 15:
            raise ValueError(f"{v8_path.name} record {index} has origin {x},{y}")
        expected = bytearray(source[0:2])
        expected.append((y // 48) << 4 | (x // 48))
        expected.append(source[7])
        expected += source[8:36]
        expected += dreamland._tile_average(
            width, height, palette, pixels, x, y
        )
        averages = {
            "source-authored": source[36:40],
            "source-expanded": expected[32:36],
            "source-5bit": tile_average_5bit(
                width, height, palette, pixels, x, y
            ),
            "source-rounded": tile_average_rounded(
                width, height, palette, pixels, x, y
            ),
            "source-linear-floor": tile_average_linear(
                width, height, palette, pixels, x, y, False
            ),
            "source-linear-round": tile_average_linear(
                width, height, palette, pixels, x, y, True
            ),
            "source-native32-edge": tile_average_native32(
                width, palette, pixels, x, y, False
            ),
            "source-native32-center": tile_average_native32(
                width, palette, pixels, x, y, True
            ),
            "target-expanded": dreamland._tile_average(
                target_width,
                target_height,
                target_palette,
                target_pixels,
                x,
                y,
            ),
            "target-5bit": tile_average_5bit(
                target_width,
                target_height,
                target_palette,
                target_pixels,
                x,
                y,
            ),
            "target-rounded": tile_average_rounded(
                target_width,
                target_height,
                target_palette,
                target_pixels,
                x,
                y,
            ),
            "target-linear-floor": tile_average_linear(
                target_width,
                target_height,
                target_palette,
                target_pixels,
                x,
                y,
                False,
            ),
            "target-linear-round": tile_average_linear(
                target_width,
                target_height,
                target_palette,
                target_pixels,
                x,
                y,
                True,
            ),
        }
        for name, average in averages.items():
            average_mismatches[name] += sum(
                a != b for a, b in zip(average, target[32:36])
            )
        record_offsets = [
            offset
            for offset, (a, b) in enumerate(zip(expected, target))
            if a != b
        ]
        mismatches += len(record_offsets)
        mismatch_offsets.update(record_offsets)
        if record_offsets and len(mismatch_examples) < 3:
            mismatch_examples.append(
                f"{index}:" + ",".join(
                    f"{offset}={expected[offset]:02X}>{target[offset]:02X}"
                    for offset in record_offsets
                )
            )
        active += 1
    sentinel_summary = ",".join(
        f"{x:04X}:{y:04X}=>{len(encodings)}"
        for (x, y), encodings in sorted(sentinels.items())
    ) or "none"
    lines.append(
        f"  XTIN active-record-exact={mismatches == 0} active={active} "
        f"mismatched_bytes={mismatches} sentinels={sentinel_summary}"
    )
    lines.append(
        "  XTIN mismatch-offsets=" +
        (",".join(f"{offset}:{count}" for offset, count in sorted(mismatch_offsets.items())) or "none")
    )
    lines.append(
        "  XTIN mismatch-examples=" + ("; ".join(mismatch_examples) or "none")
    )
    lines.append(
        "  XTIN average-mismatches=" + ",".join(
            f"{name}:{count}" for name, count in average_mismatches.items()
        )
    )

    for tag in (b"SUNA", b"ZONE", b"ZMAP", b"JUNC", b"RSEG", b"XRTP"):
        source = b"".join(node.payload for node in left.get(tag, []))
        target = b"".join(node.payload for node in right.get(tag, []))
        lines.append(
            f"  {tag.decode('ascii')} exact={source == target} "
            f"v8={len(source)} v82={len(target)} "
            f"v8sha={digest(source)[:12]} v82sha={digest(target)[:12]}"
        )

    source_cols = left[b"COLS"][0].payload
    target_cols = right[b"COLS"][0].payload
    converted_cols = dreamland._convert_cols(source_cols)
    lines.append(
        f"  COLS zero-append-exact={converted_cols == target_cols} "
        f"retail-eighth={target_cols[28:32].hex()}"
    )

    source_xbmp = left[b"XBMP"][0].payload
    compact_xbmp = dreamland._compact_xbmp(source_xbmp)
    sw, sh, sp, sx = dreamland._decode_xbmp(source_xbmp)
    tw, th, tp, tx = dreamland._decode_xbmp(target_xbmp)
    lines.append(
        f"  XBMP v8={sw}x{sh}/{len(source_xbmp)} "
        f"v82={tw}x{th}/{len(target_xbmp)} palette_exact={sp == tp} "
        f"texel_exact={sx == tx} compact-binary-exact={compact_xbmp == target_xbmp}"
    )
    return lines


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--v8-dir", type=Path, default=REPO / "PS1 game" / "TERRAIN"
    )
    parser.add_argument(
        "--v82-dir", type=Path, default=REPO / "V8_2_LOOSE" / "LEVELS" / "V8"
    )
    parser.add_argument(
        "--dreamland-port",
        type=Path,
        default=(
            REPO / "V8_2_LOOSE" / "mods" / "v82_n64_super_dreamland" /
            "files" / "LEVELS" / "N64" / "DREAMLND.EXP"
        ),
    )
    args = parser.parse_args()

    names = sorted(
        path.name
        for path in args.v8_dir.glob("*.EXP")
        if path.name != "DREAMLND.EXP" and (args.v82_dir / path.name).is_file()
    )
    output: list[str] = []
    for name in names:
        output.extend(compare_pair(args.v8_dir / name, args.v82_dir / name))

    source = chunks(terrain(args.v8_dir / "DREAMLND.EXP"))
    port = chunks(terrain(args.dreamland_port))
    expected_xtin = dreamland._convert_tinf(
        source[b"TINF"][0].payload,
        source[b"XBMP"][0].payload,
    )
    output.append("DREAMLND.EXP")
    output.append(
        f"  port-XTIN-exact={expected_xtin == port[b'XTIN'][0].payload} "
        f"size={len(port[b'XTIN'][0].payload)}"
    )
    output.append(
        f"  port-COLS-exact={dreamland._convert_cols(source[b'COLS'][0].payload) == port[b'COLS'][0].payload} "
        f"size={len(port[b'COLS'][0].payload)}"
    )
    source_xbmp = source[b"XBMP"][0].payload
    port_xbmp = port[b"XBMP"][0].payload
    sw, sh, sp, sx = dreamland._decode_xbmp(source_xbmp)
    pw, ph, pp, px = dreamland._decode_xbmp(port_xbmp)
    output.append(
        f"  port-XBMP-compact-exact={dreamland._compact_xbmp(source_xbmp) == port_xbmp} "
        f"source={sw}x{sh}/clut{len(sp)} port={pw}x{ph}/clut{len(pp)} "
        f"semantic-pixels-exact={all(sp[a] == pp[b] for a, b in zip(sx, px))}"
    )
    print("\n".join(output))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
