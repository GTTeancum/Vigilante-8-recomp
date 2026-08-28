#!/usr/bin/env python3
"""Author whole-subresource HD font atlases from SHELL/LOAD.TBL."""

from __future__ import annotations

from dataclasses import dataclass
import importlib.util
from pathlib import Path
import sys

from PIL import Image, ImageFilter, ImageFont


ROOT = Path(__file__).resolve().parents[2]
FNT_TOOL = ROOT / "tools" / "recompone-v8-2" / "build_fnt_font_proof.py"
COMPILER_TOOL = ROOT / "tools" / "recompone-v8-2" / "compile_ttf_fnt.py"
DEFAULT_SOURCE = ROOT / "V8_2_LOOSE" / "SHELL" / "LOAD.TBL"
DEFAULT_FONT = Path("C:/Windows/Fonts/ROCK.TTF")


@dataclass(frozen=True)
class EmbeddedGlyph:
    code: int
    x: int
    y: int
    width: int


@dataclass(frozen=True)
class EmbeddedFont:
    atlas: Image.Image
    glyphs: tuple[EmbeddedGlyph, ...]
    max_height: int
    upload_width_words: int
    upload_height: int


def load_module(path: Path, name: str):
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


def embedded_font(payload: bytes, offset: int, decoder) -> EmbeddedFont:
    tim_offset = int.from_bytes(payload[offset:offset + 4], "little")
    count, first, _max_width, max_height = payload[offset + 4:offset + 8]
    glyphs: list[EmbeddedGlyph] = []
    for index in range(count):
        record = payload[
            offset + 8 + index * 5:offset + 8 + index * 5 + 5
        ]
        if len(record) != 5:
            raise ValueError("truncated embedded LOAD.TBL font record")
        x, y, width, _advance, _glyph_offset = record
        if width:
            glyphs.append(EmbeddedGlyph(first + index, x, y, width))
    tim = payload[offset + tim_offset:]
    atlas = decoder.decode_tim_image(tim, f"LOAD.TBL@0x{offset:X}")
    flags = int.from_bytes(tim[4:8], "little")
    depth = flags & 3
    cursor = 8
    if flags & 8:
        cursor += int.from_bytes(tim[cursor:cursor + 4], "little")
    width_words = int.from_bytes(tim[cursor + 8:cursor + 10], "little")
    height = int.from_bytes(tim[cursor + 10:cursor + 12], "little")
    pixels_per_word = 4 if depth == 0 else 2 if depth == 1 else 1
    if atlas.size != (width_words * pixels_per_word, height):
        raise ValueError("decoded LOAD.TBL font dimensions disagree with TIM")
    return EmbeddedFont(
        atlas, tuple(glyphs), max_height, width_words, height
    )


def native_fallback(atlas: Image.Image, scale: int) -> Image.Image:
    return atlas.convert("RGBA").resize(
        (atlas.width * scale, atlas.height * scale),
        Image.Resampling.NEAREST,
    )


def sharpen_title_cell(image: Image.Image, scale: int) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = rgba.tobytes()
    mask = bytearray(rgba.width * rgba.height)
    for index in range(0, len(pixels), 4):
        r, g, b, a = pixels[index:index + 4]
        value = max(r, g, b) if (a or r or g or b) else 0
        mask[index // 4] = 0 if value <= 10 else value
    alpha = Image.frombytes("L", rgba.size, bytes(mask)).resize(
        (rgba.width * scale, rgba.height * scale),
        Image.Resampling.LANCZOS,
    )
    alpha = alpha.filter(
        ImageFilter.UnsharpMask(radius=0.45, percent=120, threshold=0)
    )
    alpha = alpha.point(
        lambda value: 0 if value < 14 else 255 if value > 238 else value
    )
    result = Image.new("RGBA", alpha.size, (255, 255, 255, 255))
    result.putalpha(alpha)
    return result


def build(source: Path, output_dir: Path, scale: int = 4) -> list[dict[str, object]]:
    decoder = load_module(FNT_TOOL, "v82_load_tbl_font_decoder")
    compiler = load_module(COMPILER_TOOL, "v82_load_tbl_font_compiler")
    payload = source.read_bytes()
    # LOAD.TBL contains two complete FNT resources. The first is the loading
    # paragraph (uploaded as 60x72 words/rows); the second is the arena title
    # (59x200). Treating them as distinct subresources prevents later images
    # from the same table from being mistaken for fonts.
    body = embedded_font(payload, 0x18, decoder)
    title = embedded_font(payload, 0x2674, decoder)
    if (body.upload_width_words, body.upload_height) != (60, 72):
        raise ValueError("unexpected LOAD.TBL loading-body subresource")
    if (title.upload_width_words, title.upload_height) != (59, 200):
        raise ValueError("unexpected LOAD.TBL loading-title subresource")

    output_dir.mkdir(parents=True, exist_ok=True)
    body_hd = native_fallback(body.atlas, scale)
    rockwell = ImageFont.truetype(str(DEFAULT_FONT), 14 * scale)
    for glyph in body.glyphs:
        if not (0x21 <= glyph.code <= 0x7E):
            continue
        natural, _advance, _offset, _width = compiler.render_ttf_glyph(
            rockwell, chr(glyph.code), scale, 18, 14
        )
        cell = compiler.alpha_rgba(compiler.binary_alpha(
            compiler.fit_mask_to_runtime_cell(
                natural, (glyph.width * scale, 18 * scale)
            ),
            128,
        ))
        body_hd.paste(
            (0, 0, 0, 0),
            (
                glyph.x * scale,
                glyph.y * scale,
                (glyph.x + glyph.width) * scale,
                min(body_hd.height, (glyph.y + 18) * scale),
            ),
        )
        body_hd.alpha_composite(cell, (glyph.x * scale, glyph.y * scale))

    title_hd = native_fallback(title.atlas, scale)
    for glyph in title.glyphs:
        crop = title.atlas.crop((
            glyph.x,
            glyph.y,
            glyph.x + glyph.width,
            min(title.atlas.height, glyph.y + title.max_height),
        ))
        cell = sharpen_title_cell(crop, scale)
        title_hd.paste(
            (0, 0, 0, 0),
            (
                glyph.x * scale,
                glyph.y * scale,
                (glyph.x + glyph.width) * scale,
                min(title_hd.height, (glyph.y + title.max_height) * scale),
            ),
        )
        title_hd.alpha_composite(cell, (glyph.x * scale, glyph.y * scale))

    body_name = f"shell_load_tbl_body_{scale}x.dds"
    title_name = f"shell_load_tbl_title_{scale}x.dds"
    body_hd.save(output_dir / body_name)
    title_hd.save(output_dir / title_name)
    return [
        {
            "path": "SHELL/LOAD.TBL",
            "image": f"images/fonts/{body_name}",
            "sourceWidth": body.atlas.width,
            "sourceHeight": body.atlas.height,
            "uploadWidthWords": body.upload_width_words,
            "uploadHeight": body.upload_height,
        },
        {
            "path": "SHELL/LOAD.TBL",
            "image": f"images/fonts/{title_name}",
            "sourceWidth": title.atlas.width,
            "sourceHeight": title.atlas.height,
            "uploadWidthWords": title.upload_width_words,
            "uploadHeight": title.upload_height,
        },
    ]


if __name__ == "__main__":
    entries = build(
        DEFAULT_SOURCE,
        ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" /
        "images" / "fonts",
    )
    for entry in entries:
        print(entry)
