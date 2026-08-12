#!/usr/bin/env python3
"""Upscale every loose V8:2 FNT sheet and add it to the texture pack."""

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
    parser.add_argument("--skip-esrgan", action="store_true")
    args = parser.parse_args()

    fnt_tool = load_module(
        ROOT / "tools" / "recompone-v8-2" / "build_fnt_font_proof.py",
        "build_fnt_font_proof",
    )
    pack_tool = load_module(
        ROOT / "tools" / "recompone-v8-2" / "apply_game_fnt_font_pack.py",
        "apply_game_fnt_font_pack",
    )
    for fnt_path in KNOWN_FNTS:
        if args.skip_esrgan:
            upscale = args.proof_out / upscale_name(fnt_path, args.scale)
            if not upscale.exists():
                sheet = fnt_tool.decode_fnt(fnt_path)
                fnt_tool.black_preview(sheet.atlas, args.scale).save(upscale)
        else:
            upscale = make_upscale(fnt_tool, fnt_path, args.proof_out, args.scale)
        sys.argv = [
            "apply_game_fnt_font_pack.py",
            "--mode", "source",
            "--fnt", str(fnt_path),
            "--source-upscale", str(upscale),
            "--scale", str(args.scale),
            "--proof-out", str(args.proof_out),
        ]
        pack_tool.main()


if __name__ == "__main__":
    main()
