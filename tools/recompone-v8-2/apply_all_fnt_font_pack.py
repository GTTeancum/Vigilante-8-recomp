#!/usr/bin/env python3
"""Rebuild every loose V8:2 FNT sheet and add it to the texture pack."""

from __future__ import annotations

import argparse
import importlib.util
from pathlib import Path
import subprocess
import sys

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[2]
PROOF_DIR = ROOT / "build" / "v82_font_source_investigation"
ESRGAN = ROOT / "build" / "realesrgan" / "bin" / "realesrgan-ncnn-vulkan.exe"
KNOWN_FNTS = (
    ROOT / "V8_2_LOOSE" / "SHARED" / "GAME.FNT",
    ROOT / "V8_2_LOOSE" / "SHARED" / "HUD.FNT",
    ROOT / "V8_2_LOOSE" / "SHARED" / "KONG.FNT",
    ROOT / "V8_2_LOOSE" / "SHELL" / "SLOGAN.FNT",
)


def slogan_glyph_mask(sheet, glyph, scale: int) -> Image.Image:
    crop = sheet.atlas.crop(
        (glyph.x, glyph.y, glyph.x + glyph.width, glyph.y + sheet.max_height)
    ).convert("RGBA")
    pixels = crop.tobytes()
    alpha = bytearray(glyph.width * sheet.max_height)
    for index in range(0, len(pixels), 4):
        r, g, b, a = pixels[index:index + 4]
        alpha[index // 4] = 255 if a or r or g or b else 0
    return Image.frombytes(
        "L",
        (glyph.width, sheet.max_height),
        bytes(alpha),
    ).resize(
        (glyph.width * scale, sheet.max_height * scale),
        Image.Resampling.NEAREST,
    )


def render_slogan_text(sheet, text: str, target_w: int, target_h: int) -> Image.Image:
    glyphs = {glyph.char: glyph for glyph in sheet.glyphs}
    scale = 8
    space = 10 * scale
    width = sum(
        glyphs[char].advance * scale if char in glyphs else space
        for char in text
    )
    rendered = Image.new(
        "RGBA",
        (max(1, width), sheet.max_height * scale),
        (0, 0, 0, 0),
    )
    cursor = 0
    for char in text:
        glyph = glyphs.get(char)
        if glyph is None:
            cursor += space
            continue
        mask = slogan_glyph_mask(sheet, glyph, scale)
        source = Image.new("RGBA", mask.size, (250, 250, 250, 255))
        source.putalpha(mask)
        rendered.alpha_composite(source, (cursor, 0))
        cursor += glyph.advance * scale
    return rendered.resize((target_w, target_h), Image.Resampling.LANCZOS)


def paste_with_shadow(
    base: Image.Image,
    layer: Image.Image,
    pos: tuple[int, int],
    shadow: tuple[int, int, int],
    offset: tuple[int, int],
) -> None:
    shadow_layer = Image.new("RGBA", layer.size, shadow + (0,))
    shadow_layer.putalpha(layer.getchannel("A"))
    base.alpha_composite(shadow_layer, (pos[0] + offset[0], pos[1] + offset[1]))
    base.alpha_composite(layer, pos)


def add_zigzag(
    draw: ImageDraw.ImageDraw,
    x0: int,
    x1: int,
    y: int,
    amp: int = 7,
    period: int = 44,
) -> None:
    points: list[tuple[int, int]] = []
    x = x0
    up = True
    while x <= x1 + period:
        points.append((x, y + (0 if up else amp)))
        x += period // 2
        up = not up
    draw.line(points, fill=(7, 7, 7, 255), width=4, joint="curve")


def make_route66_loading_text_overlay(fnt_tool, out_path: Path) -> None:
    sheet = fnt_tool.decode_fnt(ROOT / "V8_2_LOOSE" / "SHELL" / "SLOGAN.FNT")
    overlay = Image.new("RGBA", (1920, 1080), (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)

    strip = (242, 936, 1678, 1018)
    draw.rectangle(strip, fill=(78, 78, 78, 255))
    for y in range(strip[1] + 8, strip[3] - 2, 17):
        add_zigzag(draw, strip[0], strip[2], y)

    paste_with_shadow(
        overlay,
        render_slogan_text(sheet, "Meteor Crater", 790, 106),
        (320, 58),
        (56, 56, 56),
        (7, 7),
    )
    paste_with_shadow(
        overlay,
        render_slogan_text(sheet, "Press X to start...", 680, 64),
        (456, 943),
        (46, 46, 46),
        (4, 4),
    )

    rgb = Image.new("RGB", overlay.size, (0, 0, 0))
    rgb.paste(overlay.convert("RGB"), mask=overlay.getchannel("A"))
    out_path.parent.mkdir(parents=True, exist_ok=True)
    rgb.save(out_path)


def load_module(path: Path, name: str):
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


def upscale_name(fnt_path: Path, scale: int) -> str:
    return f"{fnt_path.stem.lower()}_fnt_source_x4plus_anime_{scale}x.png"


def make_upscale(fnt_tool, fnt_path: Path, out_dir: Path, scale: int) -> Path:
    sheet = fnt_tool.decode_fnt(fnt_path)
    out_dir.mkdir(parents=True, exist_ok=True)
    black = out_dir / f"{fnt_path.stem.lower()}_fnt_source_black_1x.png"
    upscale = out_dir / upscale_name(fnt_path, scale)
    fnt_tool.black_preview(sheet.atlas, 1).save(black)
    fnt_tool.black_preview(sheet.atlas, 3).save(
        out_dir / f"{fnt_path.stem.lower()}_fnt_source_atlas_3x.png"
    )
    fnt_tool.glyph_sheet(sheet, 4).save(
        out_dir / f"{fnt_path.stem.lower()}_fnt_source_glyph_records_4x.png"
    )
    if ESRGAN.exists():
        subprocess.run(
            [
                str(ESRGAN),
                "-i", str(black),
                "-o", str(upscale),
                "-n", "realesrgan-x4plus-anime",
                "-s", str(scale),
                "-t", "64",
                "-f", "png",
            ],
            check=True,
            cwd=ESRGAN.parent,
        )
    return upscale


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--scale", type=int, default=4)
    parser.add_argument("--font-scale", type=int, default=8)
    parser.add_argument("--proof-out", type=Path, default=PROOF_DIR)
    parser.add_argument("--mode", choices=("crisp", "source", "loading"), default="crisp")
    parser.add_argument("--game-font", type=Path, default=Path("C:/Windows/Fonts/LTYPEO.TTF"))
    parser.add_argument("--game-point-size", type=int, default=18)
    parser.add_argument("--game-width-factor", type=float, default=0.94)
    parser.add_argument("--game-shadow-alpha", type=int, default=0)
    parser.add_argument("--slogan-font", type=Path)
    parser.add_argument("--slogan-point-size", type=int, default=28)
    parser.add_argument("--slogan-width-factor", type=float, default=1.0)
    parser.add_argument("--slogan-shadow-alpha", type=int, default=0)
    args = parser.parse_args()

    fnt_tool = load_module(
        ROOT / "tools" / "recompone-v8-2" / "build_fnt_font_proof.py",
        "build_fnt_font_proof",
    )
    pack_tool = load_module(
        ROOT / "tools" / "recompone-v8-2" / "apply_game_fnt_font_pack.py",
        "apply_game_fnt_font_pack",
    )
    base_mode = "source" if args.mode == "source" else "crisp"
    for fnt_path in KNOWN_FNTS:
        if base_mode == "source":
            upscale = make_upscale(fnt_tool, fnt_path, args.proof_out, args.scale)
        else:
            sheet = fnt_tool.decode_fnt(fnt_path)
            args.proof_out.mkdir(parents=True, exist_ok=True)
            fnt_tool.black_preview(sheet.atlas, 3).save(
                args.proof_out / f"{fnt_path.stem.lower()}_fnt_source_atlas_3x.png"
            )
            fnt_tool.glyph_sheet(sheet, 4).save(
                args.proof_out / f"{fnt_path.stem.lower()}_fnt_source_glyph_records_4x.png"
            )
            upscale = args.proof_out / upscale_name(fnt_path, args.scale)
        sys.argv = [
            "apply_game_fnt_font_pack.py",
            "--mode", base_mode,
            "--fnt", str(fnt_path),
            "--scale", str(args.scale),
            "--proof-out", str(args.proof_out),
        ]
        if base_mode == "source":
            sys.argv.extend(["--source-upscale", str(upscale)])
        pack_tool.main()

    if args.mode != "loading":
        return
    if not args.game_font.exists():
        raise SystemExit(f"GAME.FNT donor font missing: {args.game_font}")
    if args.slogan_font is None or not args.slogan_font.exists():
        raise SystemExit("SLOGAN.FNT donor font missing; pass --slogan-font")

    targeted = (
        (
            ROOT / "V8_2_LOOSE" / "SHARED" / "GAME.FNT",
            args.game_font,
            args.game_point_size,
            args.game_width_factor,
            args.game_shadow_alpha,
        ),
        (
            ROOT / "V8_2_LOOSE" / "SHELL" / "SLOGAN.FNT",
            args.slogan_font,
            args.slogan_point_size,
            args.slogan_width_factor,
            args.slogan_shadow_alpha,
        ),
    )
    for fnt_path, font_path, point_size, width_factor, shadow_alpha in targeted:
        sys.argv = [
            "apply_game_fnt_font_pack.py",
            "--mode", "ttf",
            "--fnt", str(fnt_path),
            "--font", str(font_path),
            "--point-size", str(point_size),
            "--width-factor", str(width_factor),
            "--shadow-alpha", str(shadow_alpha),
            "--scale", str(args.font_scale),
            "--proof-out", str(args.proof_out),
        ]
        pack_tool.main()

    make_route66_loading_text_overlay(
        fnt_tool,
        ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" /
        "loading_cards" / "route66_loading_text_1920x1080.ppm",
    )


if __name__ == "__main__":
    main()
