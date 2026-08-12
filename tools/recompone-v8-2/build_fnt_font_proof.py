#!/usr/bin/env python3
"""Decode V8:2 FNT sheets and build source-font comparison proofs."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import sys
import unicodedata

from PIL import Image, ImageDraw, ImageFilter, ImageFont


ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / "tools"
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

FNV_OFFSET = 14695981039346656037
FNV_PRIME = 1099511628211


def expand5(value: int) -> int:
    return (value << 3) | (value >> 2)


def runtime_hash(image: Image.Image) -> int:
    width, height = image.size
    value = FNV_OFFSET
    for byte in (width & 0xFF, width >> 8, height & 0xFF, height >> 8):
        value ^= byte
        value = (value * FNV_PRIME) & 0xFFFFFFFFFFFFFFFF
    for byte in image.tobytes():
        value ^= byte
        value = (value * FNV_PRIME) & 0xFFFFFFFFFFFFFFFF
    return value


@dataclass(frozen=True)
class GlyphRecord:
    code: int
    char: str
    x: int
    y: int
    width: int
    advance: int
    offset: int


@dataclass(frozen=True)
class FontSheet:
    path: Path
    first_char: int
    max_width: int
    max_height: int
    glyphs: tuple[GlyphRecord, ...]
    atlas: Image.Image


def little_u32(payload: bytes, offset: int) -> int:
    return int.from_bytes(payload[offset:offset + 4], "little")


def little_u16(payload: bytes, offset: int) -> int:
    return int.from_bytes(payload[offset:offset + 2], "little")


def decode_tim_image(payload: bytes, context: str) -> Image.Image:
    if len(payload) < 20 or little_u32(payload, 0) != 0x10:
        raise ValueError(f"{context} is not a TIM payload")
    flags = little_u32(payload, 4)
    depth = flags & 3
    has_clut = bool(flags & 8)
    offset = 8
    palette: tuple[int, ...] = ()
    if has_clut:
        block_size = little_u32(payload, offset)
        color_count = little_u16(payload, offset + 8)
        palette_count = little_u16(payload, offset + 10)
        palette = tuple(
            little_u16(payload, offset + 12 + index * 2)
            for index in range(color_count * palette_count)
        )
        offset += block_size
    image_size = little_u32(payload, offset)
    width_words = little_u16(payload, offset + 8)
    height = little_u16(payload, offset + 10)
    pixels = payload[offset + 12:offset + image_size]
    pixels_per_word = 4 if depth == 0 else 2 if depth == 1 else 1
    width = width_words * pixels_per_word
    rgba = bytearray(width * height * 4)
    for y in range(height):
        row = y * width_words * 2
        for x in range(width):
            if depth == 0:
                packed = pixels[row + (x >> 1)]
                palette_index = packed & 0x0F if (x & 1) == 0 else packed >> 4
                pixel = palette[palette_index]
            elif depth == 1:
                pixel = palette[pixels[row + x]]
            else:
                pixel = little_u16(pixels, row + x * 2)
            output = (y * width + x) * 4
            rgba[output] = expand5(pixel & 0x1F)
            rgba[output + 1] = expand5((pixel >> 5) & 0x1F)
            rgba[output + 2] = expand5((pixel >> 10) & 0x1F)
            rgba[output + 3] = 255 if pixel & 0x8000 else 0
    return Image.frombytes("RGBA", (width, height), bytes(rgba))


def decode_fnt(path: Path) -> FontSheet:
    payload = path.read_bytes()
    tim_offset = little_u32(payload, 0)
    glyph_count = payload[4]
    first_char = payload[5]
    max_width = payload[6]
    max_height = payload[7]
    glyphs: list[GlyphRecord] = []
    for index in range(glyph_count):
        record = payload[8 + index * 5:8 + index * 5 + 5]
        if len(record) != 5:
            break
        x, y, width, advance, glyph_offset = record
        code = first_char + index
        if width == 0:
            continue
        if x + width > 255 or y + max_height > 255:
            continue
        glyphs.append(
            GlyphRecord(code, chr(code), x, y, width, advance, glyph_offset)
        )
    return FontSheet(
        path=path,
        first_char=first_char,
        max_width=max_width,
        max_height=max_height,
        glyphs=tuple(glyphs),
        atlas=decode_tim_image(payload[tim_offset:], str(path)),
    )


def opaque_crop(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = bytearray(rgba.tobytes())
    for index in range(0, len(pixels), 4):
        if pixels[index + 3] == 0 and (pixels[index] or pixels[index + 1] or pixels[index + 2]):
            pixels[index + 3] = 255
    return Image.frombytes("RGBA", rgba.size, bytes(pixels))


def black_preview(image: Image.Image, scale: int) -> Image.Image:
    base = Image.new("RGBA", (image.width * scale, image.height * scale), (0, 0, 0, 255))
    enlarged = opaque_crop(image).resize(base.size, Image.Resampling.NEAREST)
    base.alpha_composite(enlarged)
    return base.convert("RGB")


def glyph_sheet(sheet: FontSheet, scale: int) -> Image.Image:
    columns = 16
    label_height = 9
    cell_w = sheet.max_width * scale + 18
    cell_h = sheet.max_height * scale + label_height + 10
    rows = (len(sheet.glyphs) + columns - 1) // columns
    out = Image.new("RGB", (columns * cell_w, rows * cell_h), (18, 18, 18))
    draw = ImageDraw.Draw(out)
    for index, glyph in enumerate(sheet.glyphs):
        cx = (index % columns) * cell_w
        cy = (index // columns) * cell_h
        draw.text((cx + 2, cy + 1), f"{glyph.code:02X}", fill=(180, 180, 180))
        crop = sheet.atlas.crop(
            (glyph.x, glyph.y, glyph.x + glyph.width, glyph.y + sheet.max_height)
        )
        preview = black_preview(crop, scale)
        out.paste(preview, (cx + 8, cy + label_height))
    return out


def render_ttf_atlas(sheet: FontSheet, font_path: Path, point_size: int, scale: int) -> Image.Image:
    out = Image.new("RGBA", (sheet.atlas.width * scale, sheet.atlas.height * scale), (0, 0, 0, 0))
    draw = ImageDraw.Draw(out)
    font = ImageFont.truetype(str(font_path), point_size * scale)
    for glyph in sheet.glyphs:
        if not donor_text_character(glyph.char):
            paste_source_glyph(out, sheet, glyph, scale)
            continue
        x = glyph.x * scale
        y = (glyph.y - 5 + glyph.offset) * scale
        char = glyph.char
        draw.text(
            (x + scale, y + scale),
            char,
            font=font,
            fill=(99, 99, 99, 255),
        )
        draw.text((x, y), char, font=font, fill=(247, 247, 247, 255))
    return out


def donor_text_character(char: str) -> bool:
    if unicodedata.category(char)[0] == "C":
        return False
    return ord(char) >= 0xA0 or (
        "A" <= char <= "Z" or
        "a" <= char <= "z" or
        "0" <= char <= "9" or
        char in ".,:;!?+-/()'\" "
    )


def paste_source_glyph(
    out: Image.Image,
    sheet: FontSheet,
    glyph: GlyphRecord,
    scale: int,
) -> None:
    crop = opaque_crop(
        sheet.atlas.crop(
            (glyph.x, glyph.y, glyph.x + glyph.width, glyph.y + sheet.max_height)
        )
    )
    source = crop.convert("RGBA")
    pixels = source.tobytes()
    alpha = bytearray(source.width * source.height)
    for index in range(0, len(pixels), 4):
        r, g, b, a = pixels[index:index + 4]
        alpha[index // 4] = 255 if a or r or g or b else 0
    mask = Image.frombytes("L", source.size, bytes(alpha))
    scaled_mask = mask.resize(
        (glyph.width * scale, sheet.max_height * scale),
        Image.Resampling.NEAREST,
    )
    scaled = Image.new("RGBA", scaled_mask.size, (255, 255, 255, 255))
    scaled.putalpha(scaled_mask)
    out.alpha_composite(scaled, (glyph.x * scale, glyph.y * scale))


def glyph_ink_box(image: Image.Image) -> tuple[int, int, int, int] | None:
    rgba = image.convert("RGBA")
    data = rgba.tobytes()
    min_x = min_y = 1 << 30
    max_x = max_y = -1
    for y in range(rgba.height):
        for x in range(rgba.width):
            offset = (y * rgba.width + x) * 4
            r, g, b, a = data[offset:offset + 4]
            if a != 0 or r != 0 or g != 0 or b != 0:
                min_x = min(min_x, x)
                min_y = min(min_y, y)
                max_x = max(max_x, x + 1)
                max_y = max(max_y, y + 1)
    if max_x < min_x or max_y < min_y:
        return None
    return min_x, min_y, max_x, max_y


def render_fitted_ttf_atlas(
    sheet: FontSheet,
    font_path: Path,
    point_size: int,
    scale: int,
    width_factor: float = 0.92,
    shadow_alpha: int = 185,
) -> Image.Image:
    out = Image.new(
        "RGBA",
        (sheet.atlas.width * scale, sheet.atlas.height * scale),
        (0, 0, 0, 0),
    )
    font = ImageFont.truetype(str(font_path), point_size * scale)
    for glyph in sheet.glyphs:
        if not donor_text_character(glyph.char):
            paste_source_glyph(out, sheet, glyph, scale)
            continue
        source_box = glyph_ink_box(
            sheet.atlas.crop(
                (glyph.x, glyph.y, glyph.x + glyph.width, glyph.y + sheet.max_height)
            )
        )
        if source_box is None:
            continue
        temp = Image.new("RGBA", (sheet.max_width * scale * 4, sheet.max_height * scale * 4), (0, 0, 0, 0))
        draw = ImageDraw.Draw(temp)
        bbox = draw.textbbox((0, 0), glyph.char, font=font)
        draw.text(
            (-bbox[0] + scale, -bbox[1] + scale),
            glyph.char,
            font=font,
            fill=(255, 255, 255, 255),
        )
        rendered_box = temp.getbbox()
        if rendered_box is None:
            continue
        rendered = temp.crop(rendered_box)
        sx0, sy0, sx1, sy1 = source_box
        target_w = max(1, int(round((sx1 - sx0) * scale * width_factor)))
        target_h = max(1, (sy1 - sy0) * scale)
        fitted = rendered.resize((target_w, target_h), Image.Resampling.LANCZOS)
        shadow = Image.new("RGBA", fitted.size, (99, 99, 99, 0))
        shadow_alpha_image = fitted.getchannel("A").filter(
            ImageFilter.GaussianBlur(max(0.25, scale * 0.10))
        ).point(lambda value: int(value * shadow_alpha / 255))
        shadow.putalpha(shadow_alpha_image)
        base_x = (glyph.x + sx0) * scale
        base_y = (glyph.y + sy0) * scale
        out.alpha_composite(shadow, (base_x + scale, base_y + scale))
        white = Image.new("RGBA", fitted.size, (247, 247, 247, 0))
        white.putalpha(fitted.getchannel("A"))
        out.alpha_composite(white, (base_x, base_y))
    return out


def glyph_mask(image: Image.Image, threshold: int = 16) -> Image.Image:
    rgba = image.convert("RGBA")
    return rgba.getchannel("A").point(lambda value: 255 if value > threshold else 0)


def score_ttf(sheet: FontSheet, font_path: Path, point_size: int) -> float:
    rendered = render_ttf_atlas(sheet, font_path, point_size, 1)
    total = 0
    diff = 0
    for glyph in sheet.glyphs:
        box = (glyph.x, glyph.y, glyph.x + glyph.width, glyph.y + sheet.max_height)
        original = glyph_mask(sheet.atlas.crop(box))
        candidate = glyph_mask(rendered.crop(box))
        original_bytes = original.tobytes()
        candidate_bytes = candidate.tobytes()
        total += len(original_bytes)
        diff += sum(1 for a, b in zip(original_bytes, candidate_bytes) if a != b)
    return diff / total if total else 1.0


def comparison_proof(sheet: FontSheet, candidates: list[tuple[Path, int, float]], scale: int) -> Image.Image:
    native = black_preview(sheet.atlas, scale)
    panels = [(f"Original {sheet.path.stem.upper()}.FNT TIM atlas", native)]
    for font_path, point_size, score in candidates:
        image = render_ttf_atlas(sheet, font_path, point_size, scale)
        label = f"{font_path.name} {point_size}px mask diff {score:.3f}"
        panels.append((label, black_preview(image, 1)))
    width = max(panel.width for _, panel in panels)
    label_h = 24
    gap = 10
    height = sum(label_h + panel.height for _, panel in panels) + gap * (len(panels) - 1)
    out = Image.new("RGB", (width, height), (10, 10, 10))
    draw = ImageDraw.Draw(out)
    y = 0
    for label, panel in panels:
        draw.text((8, y + 5), label, fill=(230, 230, 230))
        y += label_h
        out.paste(panel, (0, y))
        y += panel.height + gap
    return out


def fitted_proof(
    sheet: FontSheet,
    font_path: Path,
    point_size: int,
    scale: int,
) -> Image.Image:
    native = black_preview(sheet.atlas, scale)
    fitted = black_preview(
        render_fitted_ttf_atlas(sheet, font_path, point_size, scale),
        1,
    )
    width = max(native.width, fitted.width)
    label_h = 24
    gap = 12
    out = Image.new("RGB", (width, label_h * 2 + native.height + fitted.height + gap), (10, 10, 10))
    draw = ImageDraw.Draw(out)
    y = 0
    draw.text((8, y + 5), f"Original {sheet.path.stem.upper()}.FNT source atlas", fill=(230, 230, 230))
    y += label_h
    out.paste(native, (0, y))
    y += native.height + gap
    draw.text(
        (8, y + 5),
        f"Fitted high-res TTF atlas: {font_path.name} {point_size}px",
        fill=(230, 230, 230),
    )
    y += label_h
    out.paste(fitted, (0, y))
    return out


def find_font_candidates(font_dir: Path) -> list[Path]:
    names = (
        "CALISTBI.TTF", "ACaslonPro-SemiboldItalic.otf", "CALISTI.TTF",
        "ACaslonPro-BoldItalic.otf", "SCHLBKBI.TTF", "ROCKI.TTF",
        "AGaramondPro-BoldItalic.otf", "MinionPro-It.otf", "timesbi.ttf",
        "georgiaz.ttf", "BOOKOSBI.TTF", "BELLI.TTF", "cambriaz.ttf",
        "timesbd.ttf", "georgiab.ttf", "BELLB.TTF",
    )
    result = []
    for name in names:
        path = font_dir / name
        if path.exists():
            result.append(path)
    return result


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--fnt", type=Path, default=ROOT / "V8_2_LOOSE" / "SHARED" / "GAME.FNT")
    parser.add_argument("--out", type=Path, default=ROOT / "build" / "v82_font_source_investigation")
    parser.add_argument("--font-dir", type=Path, default=Path("C:/Windows/Fonts"))
    args = parser.parse_args()

    sheet = decode_fnt(args.fnt)
    args.out.mkdir(parents=True, exist_ok=True)
    black_preview(sheet.atlas, 3).save(args.out / "game_fnt_source_atlas_3x.png")
    glyph_sheet(sheet, 4).save(args.out / "game_fnt_source_glyph_records_4x.png")

    scored: list[tuple[Path, int, float]] = []
    for font_path in find_font_candidates(args.font_dir):
        best: tuple[int, float] | None = None
        for size in range(12, 23):
            score = score_ttf(sheet, font_path, size)
            if best is None or score < best[1]:
                best = (size, score)
        assert best is not None
        scored.append((font_path, best[0], best[1]))
    scored.sort(key=lambda item: item[2])
    comparison_proof(sheet, scored[:5], 3).save(args.out / "game_fnt_ttf_candidate_comparison.png")
    best_font, best_size, best_score = scored[0]
    render_ttf_atlas(sheet, best_font, best_size, 4).save(args.out / "game_fnt_best_ttf_atlas_4x.png")
    render_fitted_ttf_atlas(sheet, best_font, best_size, 4).save(
        args.out / "game_fnt_best_ttf_fitted_atlas_4x.png"
    )
    fitted_proof(sheet, best_font, best_size, 3).save(
        args.out / "game_fnt_best_ttf_fitted_proof.png"
    )

    report = [
        f"source={sheet.path}",
        f"atlas={sheet.atlas.width}x{sheet.atlas.height}",
        f"first_char=0x{sheet.first_char:02x}",
        f"max_cell={sheet.max_width}x{sheet.max_height}",
        f"glyph_records={len(sheet.glyphs)}",
        f"atlas_hash={runtime_hash(sheet.atlas):016x}",
        "",
        "candidate_scores_lower_is_better:",
    ]
    for font_path, point_size, score in scored:
        report.append(f"{score:.6f} {font_path.name} {point_size}px {font_path}")
    (args.out / "game_fnt_source_report.txt").write_text("\n".join(report) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
