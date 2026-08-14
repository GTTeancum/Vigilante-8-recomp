#!/usr/bin/env python3
"""Compile a TrueType font into V8: 2nd Offense FNT and 4x DDS assets."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import importlib.util
import json
import math
from pathlib import Path
import re
import struct
import sys

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[2]
FNT_READER = ROOT / "tools" / "recompone-v8-2" / "build_fnt_font_proof.py"
TEXTMAP_TOOL = ROOT / "tools" / "recompone-v8-2" / "apply_loading_game_font_textmap.py"
DEFAULT_TEMPLATE = ROOT / "V8_2_LOOSE" / "SHARED" / "GAME.FNT"
DEFAULT_MOD = ROOT / "V8_2_LOOSE" / "mods" / "ttf_game_font"
DEFAULT_TEXTURE_PACK = ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x"
DEFAULT_FONT = Path("C:/Windows/Fonts/ROCK.TTF")


@dataclass(frozen=True)
class CompiledGlyph:
    code: int
    x: int
    y: int
    width: int
    advance: int
    offset: int
    native: Image.Image
    high_res: Image.Image
    source: str


def load_fnt_reader():
    spec = importlib.util.spec_from_file_location("v82_fnt_reader", FNT_READER)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {FNT_READER}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def load_module(path: Path, name: str):
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


def align(value: int, alignment: int) -> int:
    return (value + alignment - 1) & -alignment


def printable_ttf_code(code: int) -> bool:
    return 0x20 <= code <= 0x7E or 0xA0 <= code <= 0xFF


def render_ttf_glyph(
    font: ImageFont.FreeTypeFont,
    char: str,
    scale: int,
    logical_height: int,
    baseline: int,
) -> tuple[Image.Image, int, int, int]:
    bbox = font.getbbox(char, anchor="ls")
    advance = max(1, int(round(font.getlength(char) / scale)))
    if bbox is None or bbox[2] <= bbox[0]:
        return Image.new("L", (0, logical_height * scale)), advance, 0, 0

    left, _top, right, _bottom = bbox
    logical_left = math.floor(left / scale)
    logical_right = math.ceil(right / scale)
    width = max(1, logical_right - logical_left)
    offset = max(0, min(255, logical_left))

    mask = Image.new("L", (width * scale, logical_height * scale), 0)
    draw = ImageDraw.Draw(mask)
    # The FNT offset field cannot encode a negative italic bearing. Normalize
    # that overhang to the cell edge while retaining the TTF advance.
    draw.text(
        (-left, baseline * scale),
        char,
        font=font,
        fill=255,
        anchor="ls",
    )
    return mask, advance, offset, width


def fallback_glyph(template, glyph, scale: int) -> tuple[Image.Image, Image.Image]:
    crop = template.atlas.crop(
        (glyph.x, glyph.y, glyph.x + glyph.width, glyph.y + template.max_height)
    ).convert("RGBA")
    alpha = Image.new("L", crop.size, 0)
    source = crop.load()
    target = alpha.load()
    for y in range(crop.height):
        for x in range(crop.width):
            r, g, b, a = source[x, y]
            target[x, y] = max(r, g, b) if a or r or g or b else 0
    return alpha, alpha.resize(
        (glyph.width * scale, template.max_height * scale),
        Image.Resampling.NEAREST,
    )


def choose_baseline(
    font: ImageFont.FreeTypeFont,
    codes: list[int],
    scale: int,
    logical_height: int,
) -> int:
    boxes = [font.getbbox(chr(code), anchor="ls") for code in codes]
    boxes = [box for box in boxes if box is not None]
    top = min(box[1] for box in boxes)
    bottom = max(box[3] for box in boxes)
    ink_height = bottom - top
    available = logical_height * scale
    if ink_height > available:
        raise ValueError(
            f"TTF ink height {ink_height / scale:.2f}px exceeds "
            f"FNT cell height {logical_height}px"
        )
    top_padding = (available - ink_height) // 2
    return int(round((top_padding - top) / scale))


def build_glyphs(template, font_path: Path, point_size: int, scale: int):
    font = ImageFont.truetype(str(font_path), point_size * scale)
    ttf_codes = [
        glyph.code for glyph in template.glyphs
        if printable_ttf_code(glyph.code)
    ]
    baseline = choose_baseline(
        font, ttf_codes, scale, template.max_height
    )
    rendered: dict[int, tuple[Image.Image, Image.Image, int, int, int, str]] = {}
    by_code = {glyph.code: glyph for glyph in template.glyphs}

    for code in range(template.first_char, template.first_char + 221):
        source_glyph = by_code.get(code)
        if code == 0x20:
            advance = max(1, int(round(font.getlength(" ") / scale)))
            rendered[code] = (
                Image.new("L", (0, template.max_height), 0),
                Image.new("L", (0, template.max_height * scale), 0),
                0,
                advance,
                0,
                "ttf-space",
            )
            continue
        if source_glyph is None:
            rendered[code] = (
                Image.new("L", (0, template.max_height), 0),
                Image.new("L", (0, template.max_height * scale), 0),
                0,
                0,
                0,
                "missing",
            )
            continue
        if printable_ttf_code(code):
            high, advance, offset, width = render_ttf_glyph(
                font,
                chr(code),
                scale,
                template.max_height,
                baseline,
            )
            native = high.resize(
                (width, template.max_height), Image.Resampling.LANCZOS
            )
            rendered[code] = (
                native, high, width, min(255, advance), offset, "ttf"
            )
        else:
            native, high = fallback_glyph(template, source_glyph, scale)
            rendered[code] = (
                native,
                high,
                source_glyph.width,
                source_glyph.advance,
                source_glyph.offset,
                "original-symbol",
            )
    return rendered, baseline


def pack_glyphs(
    rendered,
    first_char: int,
    count: int,
    row_height: int,
    scale: int,
    atlas_width: int,
    gutter: int,
) -> tuple[list[CompiledGlyph], int]:
    x = 0
    row = 0
    packed: list[CompiledGlyph] = []
    for code in range(first_char, first_char + count):
        native, high, width, advance, offset, source = rendered[code]
        if width == 0:
            packed.append(
                CompiledGlyph(code, 0, 0, 0, advance, offset, native, high, source)
            )
            continue
        if x and x + width > atlas_width:
            row += 1
            x = 0
        if x + width > atlas_width:
            raise ValueError(
                f"glyph 0x{code:02X} width {width} exceeds atlas width {atlas_width}"
            )
        y = row * row_height
        if y > 255:
            raise ValueError("packed FNT y coordinate exceeds its one-byte field")
        packed.append(
            CompiledGlyph(code, x, y, width, advance, offset, native, high, source)
        )
        x += width + gutter
    height = (row + 1) * row_height
    if height > 255:
        raise ValueError("packed FNT atlas exceeds 255 pixels in height")
    return packed, height


def compose_atlases(
    glyphs: list[CompiledGlyph],
    width: int,
    height: int,
    row_height: int,
    scale: int,
) -> tuple[Image.Image, Image.Image]:
    native = Image.new("L", (width, height), 0)
    high = Image.new("L", (width * scale, height * scale), 0)
    for glyph in glyphs:
        if glyph.width == 0:
            continue
        native.paste(glyph.native, (glyph.x, glyph.y))
        high.paste(glyph.high_res, (glyph.x * scale, glyph.y * scale))
    return native, high


def read_tim_header(template_path: Path) -> tuple[bytes, list[int], tuple[int, int]]:
    payload = template_path.read_bytes()
    tim_offset = struct.unpack_from("<I", payload, 0)[0]
    if struct.unpack_from("<I", payload, tim_offset)[0] != 0x10:
        raise ValueError(f"{template_path} has no embedded TIM")
    clut_size = struct.unpack_from("<I", payload, tim_offset + 8)[0]
    clut_header = payload[tim_offset + 12:tim_offset + 20]
    colors = list(
        struct.unpack_from(
            f"<{(clut_size - 12) // 2}H", payload, tim_offset + 20
        )
    )
    image_offset = tim_offset + 8 + clut_size
    image_x, image_y = struct.unpack_from("<HH", payload, image_offset + 4)
    return clut_header, colors, (image_x, image_y)


def encode_tim(
    mask: Image.Image,
    clut_header: bytes,
    colors: list[int],
    image_origin: tuple[int, int],
) -> bytes:
    width, height = mask.size
    # V8 FNT sheets use an 8bpp TIM with a deliberately small grayscale CLUT.
    # TIM stores two 8bpp pixels per 16-bit width word.
    width_words = (width + 1) // 2
    storage_width = width_words * 2
    pixels = mask.load()
    packed = bytearray(width_words * 2 * height)
    for y in range(height):
        for x in range(storage_width):
            alpha = pixels[x, y] if x < width else 0
            palette_index = max(0, min(15, int(round(alpha * 15 / 255))))
            packed[y * width_words * 2 + x] = palette_index

    clut_payload = struct.pack(f"<{len(colors)}H", *colors)
    clut = struct.pack("<I", 12 + len(clut_payload)) + clut_header + clut_payload
    image_x, image_y = image_origin
    image = (
        struct.pack("<IHHHH", 12 + len(packed), image_x, image_y, width_words, height)
        + packed
    )
    return struct.pack("<II", 0x10, 0x09) + clut + image


def encode_fnt(
    template_path: Path,
    first_char: int,
    glyphs: list[CompiledGlyph],
    max_height: int,
    native_atlas: Image.Image,
) -> bytes:
    count = len(glyphs)
    table_size = 8 + count * 5
    tim_offset = align(table_size, 4)
    max_width = max(glyph.width for glyph in glyphs)
    header = struct.pack(
        "<IBBBB", tim_offset, count, first_char, max_width, max_height
    )
    records = b"".join(
        bytes((glyph.x, glyph.y, glyph.width, glyph.advance, glyph.offset))
        for glyph in glyphs
    )
    clut_header, colors, image_origin = read_tim_header(template_path)
    tim = encode_tim(native_atlas, clut_header, colors, image_origin)
    return header + records + bytes(tim_offset - table_size) + tim


def alpha_rgba(mask: Image.Image) -> Image.Image:
    rgba = Image.new("RGBA", mask.size, (255, 255, 255, 0))
    rgba.putalpha(mask)
    return rgba


def binary_alpha(mask: Image.Image, threshold: int) -> Image.Image:
    return mask.point(lambda value: 255 if value >= threshold else 0, mode="L")


def fit_mask_to_runtime_cell(
    mask: Image.Image,
    target_size: tuple[int, int],
) -> Image.Image:
    """Place a glyph in a runtime cell without widening its natural ink."""
    target_width, target_height = target_size
    if target_width <= 0 or target_height <= 0:
        raise ValueError(f"invalid runtime cell size {target_size}")

    fitted = mask
    if fitted.height != target_height:
        width = max(1, int(round(fitted.width * target_height / fitted.height)))
        fitted = fitted.resize((width, target_height), Image.Resampling.LANCZOS)
    if fitted.width > target_width:
        fitted = fitted.resize((target_width, target_height), Image.Resampling.LANCZOS)

    cell = Image.new("L", target_size, 0)
    cell.paste(fitted, (0, 0))
    return cell


def clean_previous_font_entries(manifest: dict, label: str) -> None:
    sources = manifest.setdefault("sources", {})
    stale_keys = {
        key
        for key, values in sources.items()
        if any(str(value).startswith(f"{label} ") for value in values)
    }
    manifest["entries"] = [
        entry for entry in manifest["entries"]
        if entry.get("key") not in stale_keys
    ]
    for key in stale_keys:
        sources.pop(key, None)


def update_texture_pack(
    reader,
    compiled_fnt: Path,
    high_atlas: Image.Image,
    texture_pack: Path,
    font_stem: str,
    scale: int,
    variant_radius: int,
    alpha_threshold: int,
) -> tuple[Path, int]:
    manifest_path = texture_pack / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    sheet = reader.decode_fnt(compiled_fnt)
    atlas_name = f"game_fnt_compiled_{font_stem}_{scale}x.dds"
    atlas_path = texture_pack / "images" / "ui" / atlas_name
    atlas_path.parent.mkdir(parents=True, exist_ok=True)
    alpha_rgba(binary_alpha(high_atlas, alpha_threshold)).save(atlas_path)
    deployed = Image.open(atlas_path).convert("RGBA")

    label = "GAME.FNT"
    clean_previous_font_entries(manifest, label)
    sources = manifest.setdefault("sources", {})
    entries_by_key: dict[str, dict] = {}
    source_by_key: dict[str, list[str]] = {}
    for glyph in sheet.glyphs:
        for dy in range(-variant_radius, variant_radius + 1):
            for dx in range(-variant_radius, variant_radius + 1):
                x = glyph.x + dx
                y = glyph.y + dy
                if x < 0 or y < 0:
                    continue
                if x + glyph.width > sheet.atlas.width:
                    continue
                if y + sheet.max_height > sheet.atlas.height:
                    continue
                crop = sheet.atlas.crop(
                    (x, y, x + glyph.width, y + sheet.max_height)
                )
                if crop.getbbox() is None:
                    continue
                key = f"{reader.runtime_hash(crop):016x}"
                entries_by_key.setdefault(
                    key,
                    {
                        "key": key,
                        "image": f"images/ui/{atlas_name}",
                        "x": x * scale,
                        "y": y * scale,
                        "width": glyph.width * scale,
                        "height": sheet.max_height * scale,
                    },
                )
                source_by_key.setdefault(key, []).append(
                    f"{label} compiled glyph 0x{glyph.code:02X} "
                    f"{glyph.char!r} crop dx={dx} dy={dy}"
                )

    manifest["entries"].extend(entries_by_key.values())
    sources.update(source_by_key)
    generator = re.sub(r"; GAME\.FNT [^;]+ font atlas", "", manifest.get("generator", ""))
    manifest["generator"] = (
        generator + f"; GAME.FNT compiled {font_stem} {scale}x font atlas"
    )
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return atlas_path, len(entries_by_key)


def update_live_loading_keys(
    reader,
    compiled_fnt: Path,
    texture_pack: Path,
    trace_dirs: list[Path],
    load_dll: Path,
    font_path: Path,
    point_size: int,
    baseline: int,
    scale: int,
    alpha_threshold: int,
) -> int:
    textmap = load_module(TEXTMAP_TOOL, "v82_loading_font_textmap")
    sheet = reader.decode_fnt(compiled_fnt)
    tips = textmap.extract_tips(load_dll)
    uses_by_log = textmap.collect_uses(trace_dirs)
    if not uses_by_log:
        raise ValueError("no live GAME.FNT loading trace was found")

    key_chars: dict[str, str] = {}
    key_uses: dict[str, object] = {}
    for log, uses in uses_by_log.items():
        rows = textmap.row_uses(uses)
        text_rows = textmap.best_text_rows(sheet, tips, rows)
        if text_rows is None:
            raise ValueError(f"cannot match loading tip text in {log}")
        for row_text, row in zip(text_rows, rows):
            for char, use in zip(textmap.nonspace(row_text), row):
                previous = key_chars.get(use.key)
                if previous is not None and previous != char:
                    raise ValueError(
                        f"runtime key {use.key} maps to both {previous!r} and {char!r}"
                    )
                key_chars[use.key] = char
                key_uses[use.key] = use

        prompt = [
            use for use in sorted(uses, key=lambda item: (item.y, item.x))
            if use.tpage == "0x0A5" and use.clut == "0x7800" and
            use.height == sheet.max_height and use.width > 0 and
            use.key != "0000000000000000"
        ]
        # Keep the native controller symbol in the sixth position. It is not
        # the Latin X glyph even though the displayed instruction reads X.
        prompt_chars: list[str | None] = list("Press") + [None] + list("tostart...")
        if prompt and len(prompt) != len(prompt_chars):
            raise ValueError(
                f"{log} has {len(prompt)} prompt glyphs, expected {len(prompt_chars)}"
            )
        for char, use in zip(prompt_chars, prompt):
            if char is None:
                continue
            previous = key_chars.get(use.key)
            if previous is not None and previous != char:
                raise ValueError(
                    f"prompt key {use.key} maps to both {previous!r} and {char!r}"
                )
            key_chars[use.key] = char
            key_uses[use.key] = use

    manifest_path = texture_pack / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    existing = {str(entry.get("key", "")) for entry in manifest["entries"]}
    glyphs = {glyph.char: glyph for glyph in sheet.glyphs}
    font = ImageFont.truetype(str(font_path), point_size * scale)
    placements: list[tuple[str, str, object, Image.Image, int, int]] = []
    padding = max(2, scale // 2)
    atlas_width = 1936
    x = y = row_height = padding
    for key, char in sorted(key_chars.items()):
        if key == "0000000000000000" or key in existing:
            continue
        glyph = glyphs.get(char)
        if glyph is None:
            raise ValueError(f"compiled FNT has no glyph for {char!r}")
        use = key_uses[key]
        target_size = (use.width * scale, use.height * scale)
        if printable_ttf_code(ord(char)):
            natural, _advance, _offset, _width = render_ttf_glyph(
                font, char, scale, use.height, baseline
            )
        else:
            source = sheet.atlas.crop((
                glyph.x,
                glyph.y,
                glyph.x + glyph.width,
                glyph.y + sheet.max_height,
            )).convert("RGBA")
            natural = source.getchannel("A")
            if natural.getbbox() is None:
                natural = source.convert("L")
            natural = natural.resize(
                (glyph.width * scale, sheet.max_height * scale),
                Image.Resampling.NEAREST,
            )
        cell = alpha_rgba(binary_alpha(
            fit_mask_to_runtime_cell(natural, target_size),
            alpha_threshold,
        ))
        if x + cell.width + padding > atlas_width:
            x = padding
            y += row_height + padding
            row_height = padding
        placements.append((key, char, use, cell, x, y))
        x += cell.width + padding
        row_height = max(row_height, cell.height)

    live_atlas = Image.new(
        "RGBA", (atlas_width, y + row_height + padding), (0, 0, 0, 0)
    )
    live_name = (
        f"game_fnt_compiled_{font_path.stem.lower()}_live_{scale}x.dds"
    )
    live_path = texture_pack / "images" / "ui" / live_name
    live_path.parent.mkdir(parents=True, exist_ok=True)
    image_name = f"images/ui/{live_name}"
    added = 0
    for key, char, use, cell, px, py in placements:
        live_atlas.alpha_composite(cell, (px, py))
        manifest["entries"].append({
            "key": key,
            "image": image_name,
            "x": px,
            "y": py,
            "width": cell.width,
            "height": cell.height,
        })
        manifest.setdefault("sources", {})[key] = [
            f"GAME.FNT live compiled glyph char={char!r} "
            f"uv={use.uv} size={use.width}x{use.height} "
            f"tpage={use.tpage} clut={use.clut} "
            f"direct-ttf-cell={point_size}pt baseline={baseline}"
        ]
        existing.add(key)
        added += 1

    live_atlas.save(live_path)

    suffix = "; GAME.FNT live compiled loading keys"
    if suffix not in manifest.get("generator", ""):
        manifest["generator"] = manifest.get("generator", "") + suffix
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return added


def render_metric_proof(
    glyphs: list[CompiledGlyph],
    high_atlas: Image.Image,
    scale: int,
    lines: list[str],
) -> Image.Image:
    by_code = {glyph.code: glyph for glyph in glyphs}
    margin = 32
    line_height = 25 * scale
    widths = []
    for line in lines:
        widths.append(sum(by_code.get(ord(char), by_code[0x20]).advance for char in line) * scale)
    out = Image.new(
        "RGB",
        (max(widths) + margin * 2, line_height * len(lines) + margin * 2),
        (0, 0, 0),
    )
    for row, line in enumerate(lines):
        pen = margin
        top = margin + row * line_height
        for char in line:
            glyph = by_code.get(ord(char), by_code[0x20])
            if glyph.width:
                crop = high_atlas.crop(
                    (
                        glyph.x * scale,
                        glyph.y * scale,
                        (glyph.x + glyph.width) * scale,
                        (glyph.y + 18) * scale,
                    )
                )
                colored = Image.new("RGBA", crop.size, (180, 180, 180, 0))
                colored.putalpha(crop)
                out.paste(
                    colored.convert("RGB"),
                    (pen + glyph.offset * scale, top),
                    colored.getchannel("A"),
                )
            pen += glyph.advance * scale
    return out


def write_audit(
    path: Path,
    font_path: Path,
    point_size: int,
    scale: int,
    baseline: int,
    glyphs: list[CompiledGlyph],
    atlas_size: tuple[int, int],
    entries: int,
) -> None:
    rows = [
        "code,char,x,y,width,advance,offset,source",
        *[
            f"0x{glyph.code:02X},{json.dumps(chr(glyph.code))},"
            f"{glyph.x},{glyph.y},{glyph.width},{glyph.advance},"
            f"{glyph.offset},{glyph.source}"
            for glyph in glyphs
        ],
    ]
    path.with_suffix(".csv").write_text("\n".join(rows) + "\n", encoding="utf-8")
    report = {
        "font": str(font_path),
        "pointSize": point_size,
        "scale": scale,
        "baseline": baseline,
        "atlasWidth": atlas_size[0],
        "atlasHeight": atlas_size[1],
        "glyphRecords": len(glyphs),
        "nonemptyGlyphs": sum(glyph.width > 0 for glyph in glyphs),
        "ttfGlyphs": sum(glyph.source == "ttf" for glyph in glyphs),
        "originalSymbols": sum(glyph.source == "original-symbol" for glyph in glyphs),
        "textureKeys": entries,
    }
    path.with_suffix(".json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--template", type=Path, default=DEFAULT_TEMPLATE)
    parser.add_argument("--font", type=Path, default=DEFAULT_FONT)
    parser.add_argument("--point-size", type=int, default=14)
    parser.add_argument("--scale", type=int, default=4)
    parser.add_argument("--atlas-width", type=int, default=242)
    parser.add_argument("--gutter", type=int, default=0)
    parser.add_argument("--variant-radius", type=int, default=2)
    parser.add_argument("--alpha-threshold", type=int, default=128)
    parser.add_argument("--mod-dir", type=Path, default=DEFAULT_MOD)
    parser.add_argument("--texture-pack", type=Path, default=DEFAULT_TEXTURE_PACK)
    parser.add_argument("--trace-dir", type=Path, action="append", default=[])
    parser.add_argument("--dump-dir", type=Path, action="append", default=[])
    parser.add_argument(
        "--load-dll",
        type=Path,
        default=ROOT / "V8_2_LOOSE" / "SHELL" / "LOAD.DLL",
    )
    args = parser.parse_args()

    if not args.template.is_file():
        raise FileNotFoundError(args.template)
    if not args.font.is_file():
        raise FileNotFoundError(args.font)
    if args.scale < 2:
        raise ValueError("high-resolution DDS scale must be at least 2")
    if not 1 <= args.alpha_threshold <= 255:
        raise ValueError("alpha threshold must be between 1 and 255")

    reader = load_fnt_reader()
    template = reader.decode_fnt(args.template)
    payload = args.template.read_bytes()
    count = payload[4]
    if count != 221:
        raise ValueError(f"expected 221 GAME.FNT records, found {count}")

    rendered, baseline = build_glyphs(
        template, args.font, args.point_size, args.scale
    )
    glyphs, atlas_height = pack_glyphs(
        rendered,
        template.first_char,
        count,
        template.max_height,
        args.scale,
        args.atlas_width,
        args.gutter,
    )
    native, high = compose_atlases(
        glyphs,
        args.atlas_width,
        atlas_height,
        template.max_height,
        args.scale,
    )

    relative_fnt = Path("SHARED") / args.template.name
    output_fnt = args.mod_dir / "files" / relative_fnt
    output_fnt.parent.mkdir(parents=True, exist_ok=True)
    output_fnt.write_bytes(
        encode_fnt(
            args.template,
            template.first_char,
            glyphs,
            template.max_height,
            native,
        )
    )
    args.mod_dir.mkdir(parents=True, exist_ok=True)
    (args.mod_dir / "mod.json").write_text(
        json.dumps(
            {
                "id": "ttf_game_font",
                "name": "Compiled GAME.FNT",
                "version": "1.0.0",
                "description": f"GAME.FNT compiled from {args.font.name}",
            },
            indent=2,
        ) + "\n",
        encoding="utf-8",
    )

    decoded = reader.decode_fnt(output_fnt)
    if decoded.atlas.size != native.size:
        raise AssertionError(
            f"FNT round trip atlas mismatch: {decoded.atlas.size} != {native.size}"
        )
    if len(decoded.glyphs) != sum(glyph.width > 0 for glyph in glyphs):
        raise AssertionError("FNT round trip lost nonempty glyph records")

    font_stem = re.sub(r"[^a-z0-9]+", "_", args.font.stem.lower()).strip("_")
    atlas_path, entry_count = update_texture_pack(
        reader,
        output_fnt,
        high,
        args.texture_pack,
        font_stem,
        args.scale,
        args.variant_radius,
        args.alpha_threshold,
    )
    live_entries = 0
    if args.trace_dir:
        live_entries = update_live_loading_keys(
            reader,
            output_fnt,
            args.texture_pack,
            args.trace_dir,
            args.load_dll,
            args.font,
            args.point_size,
            baseline,
            args.scale,
            args.alpha_threshold,
        )
    proof_dir = args.mod_dir / "proof"
    proof_dir.mkdir(parents=True, exist_ok=True)
    alpha_rgba(binary_alpha(high, args.alpha_threshold)).save(
        proof_dir / "compiled_game_fnt_atlas_4x.tga"
    )
    render_metric_proof(
        glyphs,
        high,
        args.scale,
        [
            "TIP: If your engine stalls, press and hold the Gas",
            "button until it starts up again. It's a matter of timing.",
            "The Brown Crate contains a surprise - it could contain a special weapon!",
            "Press X to start...",
        ],
    ).save(proof_dir / "compiled_game_fnt_metric_proof.png")
    write_audit(
        proof_dir / "compiled_game_fnt_audit",
        args.font,
        args.point_size,
        args.scale,
        baseline,
        glyphs,
        native.size,
        entry_count + live_entries,
    )
    print(f"compiled_fnt={output_fnt}")
    print(f"high_res_dds={atlas_path}")
    print(f"atlas={native.width}x{native.height} scale={args.scale}")
    print(
        f"baseline={baseline} glyphs={len(decoded.glyphs)} "
        f"texture_keys={entry_count} live_keys={live_entries}"
    )


if __name__ == "__main__":
    main()
