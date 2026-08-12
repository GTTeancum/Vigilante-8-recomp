#!/usr/bin/env python3
"""Rebuild every loose V8:2 FNT sheet and add it to the texture pack."""

from __future__ import annotations

import argparse
import importlib.util
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
PROOF_DIR = ROOT / "build" / "v82_font_source_investigation"
ESRGAN = ROOT / "build" / "realesrgan" / "bin" / "realesrgan-ncnn-vulkan.exe"
KNOWN_FNTS = (
    ROOT / "V8_2_LOOSE" / "SHARED" / "GAME.FNT",
    ROOT / "V8_2_LOOSE" / "SHARED" / "HUD.FNT",
    ROOT / "V8_2_LOOSE" / "SHARED" / "KONG.FNT",
    ROOT / "V8_2_LOOSE" / "SHELL" / "SLOGAN.FNT",
)


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
    parser.add_argument("--proof-out", type=Path, default=PROOF_DIR)
    parser.add_argument("--mode", choices=("crisp", "source", "loading"), default="crisp")
    parser.add_argument("--game-font", type=Path, default=Path("C:/Windows/Fonts/LTYPEO.TTF"))
    parser.add_argument("--game-point-size", type=int, default=18)
    parser.add_argument("--game-width-factor", type=float, default=0.94)
    parser.add_argument("--game-shadow-alpha", type=int, default=135)
    parser.add_argument("--slogan-font", type=Path)
    parser.add_argument("--slogan-point-size", type=int, default=28)
    parser.add_argument("--slogan-width-factor", type=float, default=1.0)
    parser.add_argument("--slogan-shadow-alpha", type=int, default=125)
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
            "--scale", str(args.scale),
            "--proof-out", str(args.proof_out),
        ]
        pack_tool.main()


if __name__ == "__main__":
    main()
