#!/usr/bin/env python3
"""Rebuild V8:2 FNT sheets that are safe to replace in the texture pack."""

from __future__ import annotations

import argparse
import importlib.util
import json
from pathlib import Path
import re
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
MOD_FONT_WORK = (
    ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" / "font_work"
)
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


def remove_fnt_pack_entries(manifest_path: Path, label: str) -> None:
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    sources = manifest.setdefault("sources", {})
    remove_keys = {
        key for key, source_list in sources.items()
        if any(str(source).startswith(f"{label} glyph ") for source in source_list)
    }
    if remove_keys:
        manifest["entries"] = [
            entry for entry in manifest["entries"]
            if entry.get("key") not in remove_keys
        ]
        for key in remove_keys:
            sources.pop(key, None)

    generator = manifest.get("generator", "")
    manifest["generator"] = re.sub(
        rf"; {re.escape(label)} [^;]+ font atlas",
        "",
        generator,
    )
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    pack_root = manifest_path.parent
    referenced = {entry.get("image", "") for entry in manifest["entries"]}
    for stale in (pack_root / "images" / "ui").glob(f"{label.lower().replace('.fnt', '')}_fnt_*.dds"):
        relative = str(stale.relative_to(pack_root)).replace("\\", "/")
        if relative not in referenced:
            stale.unlink()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--scale", type=int, default=4)
    parser.add_argument("--font-scale", type=int, default=8)
    parser.add_argument("--proof-out", type=Path, default=MOD_FONT_WORK)
    parser.add_argument(
        "--mode",
        choices=("crisp", "source", "ttf", "loading"),
        default="crisp",
    )
    parser.add_argument("--font", type=Path, default=Path("C:/Windows/Fonts/ROCKBI.TTF"))
    parser.add_argument("--point-size", type=int, default=14)
    parser.add_argument("--width-factor", type=float, default=0.92)
    parser.add_argument("--threshold", type=int, default=32)
    args = parser.parse_args()

    fnt_tool = load_module(
        ROOT / "tools" / "recompone-v8-2" / "build_fnt_font_proof.py",
        "build_fnt_font_proof",
    )
    pack_tool = load_module(
        ROOT / "tools" / "recompone-v8-2" / "apply_game_fnt_font_pack.py",
        "apply_game_fnt_font_pack",
    )
    base_mode = args.mode if args.mode in ("source", "ttf") else "crisp"
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
            "--threshold", str(args.threshold),
            "--proof-out", str(args.proof_out),
        ]
        if base_mode == "source":
            sys.argv.extend(["--source-upscale", str(upscale)])
        elif base_mode == "ttf":
            sys.argv.extend([
                "--font", str(args.font),
                "--point-size", str(args.point_size),
                "--width-factor", str(args.width_factor),
            ])
        pack_tool.main()

    if args.mode != "loading":
        return

    sys.argv = [
        "apply_game_fnt_font_pack.py",
        "--mode", "crisp",
        "--fnt", str(ROOT / "V8_2_LOOSE" / "SHARED" / "GAME.FNT"),
        "--scale", str(args.font_scale),
        "--threshold", str(args.threshold),
        "--proof-out", str(args.proof_out),
    ]
    pack_tool.main()


if __name__ == "__main__":
    main()
