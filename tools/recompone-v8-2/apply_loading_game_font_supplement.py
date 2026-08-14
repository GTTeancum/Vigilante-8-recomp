#!/usr/bin/env python3
"""Map live loading-screen GAME.FNT variants into one canonical atlas."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import importlib.util
import json
from pathlib import Path
import re
import sys

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[2]
PROOF_TOOL = ROOT / "tools" / "recompone-v8-2" / "build_fnt_font_proof.py"
DEFAULT_FNT = ROOT / "V8_2_LOOSE" / "SHARED" / "GAME.FNT"
MOD_FONT_WORK = (
    ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" / "font_work"
)
FNV_OFFSET = 14695981039346656037
FNV_PRIME = 1099511628211


@dataclass(frozen=True)
class GlyphVariant:
    key: str
    width: int
    height: int
    uv: str
    tpage: str
    clut: str


def load_fnt_tool():
    spec = importlib.util.spec_from_file_location("build_fnt_font_proof", PROOF_TOOL)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {PROOF_TOOL}")
    module = importlib.util.module_from_spec(spec)
    sys.modules["build_fnt_font_proof"] = module
    spec.loader.exec_module(module)
    return module


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


def latest_dump_dir() -> Path:
    candidates = sorted(
        (ROOT / "build").glob("v82_loading_title_runtime_dumps_*"),
        key=lambda path: path.stat().st_mtime,
        reverse=True,
    )
    if not candidates:
        raise SystemExit("no build/v82_loading_title_runtime_dumps_* directory found")
    return candidates[0]


def collect_game_misses(trace_dir: Path) -> list[GlyphVariant]:
    pattern = re.compile(
        r"V82LoadingUiResolve.*key=([0-9a-f]{16}) hit=0 .*"
        r"size=([0-9]+)x([0-9]+) uv=([^ ]+) "
        r"tpage=(0x0(?:08|A5)) clut=(0x780[0C])"
    )
    variants: dict[str, GlyphVariant] = {}
    for log in sorted(trace_dir.glob("*.stderr.log")):
        for line in log.read_text(encoding="utf-8", errors="ignore").splitlines():
            match = pattern.search(line)
            if match is None:
                continue
            key, width_text, height_text, uv, tpage, clut = match.groups()
            width = int(width_text)
            height = int(height_text)
            if key == "0000000000000000" or width <= 0 or height <= 0:
                continue
            if height != 18:
                continue
            variants.setdefault(
                key,
                GlyphVariant(key, width, height, uv, tpage, clut),
            )
    return sorted(variants.values(), key=lambda glyph: (glyph.tpage, glyph.uv, glyph.key))


def collect_game_misses_from_dirs(trace_dirs: list[Path]) -> list[GlyphVariant]:
    variants: dict[str, GlyphVariant] = {}
    for trace_dir in trace_dirs:
        for variant in collect_game_misses(trace_dir):
            variants.setdefault(variant.key, variant)
    return sorted(variants.values(), key=lambda glyph: (glyph.tpage, glyph.uv, glyph.key))


def collect_game_variants(trace_dirs: list[Path]) -> list[GlyphVariant]:
    pattern = re.compile(
        r"V82LoadingUiResolve.*key=([0-9a-f]{16}) hit=[01] .*"
        r"size=([0-9]+)x([0-9]+) uv=([^ ]+) "
        r"tpage=(0x0(?:08|A5)) clut=(0x780[0C])"
    )
    variants: dict[str, GlyphVariant] = {}
    for trace_dir in trace_dirs:
        for log in sorted(trace_dir.glob("*.stderr.log")):
            for line in log.read_text(encoding="utf-8", errors="ignore").splitlines():
                match = pattern.search(line)
                if match is None:
                    continue
                key, width_text, height_text, uv, tpage, clut = match.groups()
                width = int(width_text)
                height = int(height_text)
                if key == "0000000000000000" or width <= 0 or height != 18:
                    continue
                variants.setdefault(
                    key,
                    GlyphVariant(key, width, height, uv, tpage, clut),
                )
    return sorted(variants.values(), key=lambda glyph: (glyph.tpage, glyph.uv, glyph.key))


def active_game_fnt_image(manifest: dict[str, object]) -> str:
    sources = manifest.get("sources", {})
    counts: dict[str, int] = {}
    for entry in manifest.get("entries", []):
        key = str(entry.get("key", ""))
        if not any(
            str(source).startswith("GAME.FNT glyph ")
            for source in sources.get(key, [])
        ):
            continue
        image = str(entry.get("image", ""))
        counts[image] = counts.get(image, 0) + 1
    if not counts:
        raise ValueError("manifest has no active GAME.FNT glyph atlas")
    return max(counts.items(), key=lambda item: item[1])[0]


def load_runtime_crop(dump_dirs: list[Path], glyph: GlyphVariant) -> Image.Image:
    stem = f"{glyph.key}_{glyph.width}x{glyph.height}.rgba"
    path = next((dump_dir / stem for dump_dir in dump_dirs if (dump_dir / stem).exists()), None)
    if path is None:
        searched = ", ".join(str(dump_dir) for dump_dir in dump_dirs)
        raise FileNotFoundError(
            f"missing GAME.FNT glyph runtime dump {stem}; searched {searched}"
        )
    rgba = path.read_bytes()
    expected = glyph.width * glyph.height * 4
    if len(rgba) != expected:
        raise ValueError(f"{path} is {len(rgba)} bytes, expected {expected}")
    image = Image.frombytes("RGBA", (glyph.width, glyph.height), rgba)
    actual = runtime_hash(image)
    if f"{actual:016x}" != glyph.key:
        raise ValueError(f"{path} hashes to {actual:016x}")
    return image


def sharpen_source_glyph(
    image: Image.Image,
    scale: int,
    threshold: int,
    nearest: bool = False,
    hard_alpha: bool = False,
) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = rgba.tobytes()
    alpha = bytearray(rgba.width * rgba.height)
    for index in range(0, len(pixels), 4):
        r, g, b, a = pixels[index:index + 4]
        value = max(r, g, b) if (a or r or g or b) else 0
        if value < threshold:
            alpha[index // 4] = 0
        else:
            if hard_alpha:
                alpha[index // 4] = 255
            else:
                mapped = int((value - threshold) * 255 / (255 - threshold))
                alpha[index // 4] = 255 if mapped > 232 else mapped
    mask = Image.frombytes("L", rgba.size, bytes(alpha)).resize(
        (rgba.width * scale, rgba.height * scale),
        Image.Resampling.NEAREST if nearest else Image.Resampling.LANCZOS,
    )
    if hard_alpha:
        mask = mask.point(lambda value: 255 if value else 0)
    else:
        mask = mask.point(lambda value: 0 if value < 10 else 255 if value > 244 else value)
    out = Image.new("RGBA", mask.size, (255, 255, 255, 255))
    out.putalpha(mask)
    return out


def build_atlas(
    variants: list[GlyphVariant],
    dump_dirs: list[Path],
    scale: int,
    threshold: int,
    nearest: bool = False,
    hard_alpha: bool = False,
) -> tuple[Image.Image, list[dict[str, object]], dict[str, list[str]], Image.Image, list[GlyphVariant]]:
    padding = max(2, scale // 2)
    max_width = 1936
    x = y = row_h = padding
    placements: list[tuple[GlyphVariant, Image.Image, int, int]] = []
    previews: list[tuple[GlyphVariant, Image.Image]] = []
    skipped: list[GlyphVariant] = []
    for variant in variants:
        try:
            source = load_runtime_crop(dump_dirs, variant)
        except FileNotFoundError:
            skipped.append(variant)
            continue
        upscaled = sharpen_source_glyph(source, scale, threshold, nearest, hard_alpha)
        previews.append((variant, source))
        if x + upscaled.width + padding > max_width:
            x = padding
            y += row_h + padding
            row_h = padding
        placements.append((variant, upscaled, x, y))
        x += upscaled.width + padding
        row_h = max(row_h, upscaled.height)

    atlas = Image.new("RGBA", (max_width, y + row_h + padding), (0, 0, 0, 0))
    image_name = f"game_fnt_loading_runtime_{scale}x.dds"
    entries: list[dict[str, object]] = []
    sources: dict[str, list[str]] = {}
    for variant, upscaled, px, py in placements:
        atlas.alpha_composite(upscaled, (px, py))
        entries.append({
            "key": variant.key,
            "image": f"images/ui/{image_name}",
            "x": px,
            "y": py,
            "width": upscaled.width,
            "height": upscaled.height,
        })
        sources[variant.key] = [
            f"GAME.FNT live loading glyph uv={variant.uv} "
            f"size={variant.width}x{variant.height} "
            f"tpage={variant.tpage} clut={variant.clut} "
            f"threshold={threshold} nearest={1 if nearest else 0} "
            f"hard_alpha={1 if hard_alpha else 0}"
        ]

    label_h = 18
    proof_w = min(max_width, 1280)
    cell_h = max((variant.height for variant, _source in previews), default=18) * scale + label_h + padding
    proof_rows: list[Image.Image] = []
    row = Image.new("RGBA", (proof_w, cell_h), (12, 12, 12, 255))
    draw = ImageDraw.Draw(row)
    cx = padding
    for variant, source in previews:
        upscaled = sharpen_source_glyph(source, scale, threshold, nearest, hard_alpha)
        if cx + upscaled.width + padding > proof_w:
            proof_rows.append(row)
            row = Image.new("RGBA", (proof_w, cell_h), (12, 12, 12, 255))
            draw = ImageDraw.Draw(row)
            cx = padding
        draw.text((cx, 2), variant.key[:8], fill=(190, 190, 190))
        row.alpha_composite(upscaled, (cx, label_h))
        cx += upscaled.width + padding
    proof_rows.append(row)
    proof = Image.new(
        "RGBA",
        (proof_w, len(proof_rows) * cell_h),
        (12, 12, 12, 255),
    )
    for index, row_image in enumerate(proof_rows):
        proof.paste(row_image, (0, index * cell_h))
    return atlas, entries, sources, proof.convert("RGB"), skipped


def strip_existing(manifest: dict[str, object]) -> None:
    entries = manifest.get("entries", [])
    sources = manifest.setdefault("sources", {})
    stale_keys = {
        key for key, source_list in sources.items()
        if any(str(source).startswith("GAME.FNT live loading glyph ") for source in source_list)
    }
    manifest["entries"] = [
        entry for entry in entries
        if entry.get("key") not in stale_keys
    ]
    for key in stale_keys:
        sources.pop(key, None)
    manifest["generator"] = re.sub(
        r"; GAME\.FNT live loading glyph (?:supplement|textmap) [^;]+(?: font atlas| active-atlas)",
        "",
        str(manifest.get("generator", "")),
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--trace-dir",
        type=Path,
        default=ROOT / "build" / "v82_loading_title_dump_smoke",
    )
    parser.add_argument("--extra-trace-dir", type=Path, action="append", default=[])
    parser.add_argument("--dump-dir", type=Path)
    parser.add_argument("--extra-dump-dir", type=Path, action="append", default=[])
    parser.add_argument("--fnt", type=Path, default=DEFAULT_FNT)
    parser.add_argument(
        "--manifest",
        type=Path,
        default=ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" / "manifest.json",
    )
    parser.add_argument("--scale", type=int, default=8)
    parser.add_argument("--threshold", type=int, default=96)
    parser.add_argument("--nearest", action="store_true")
    parser.add_argument("--hard-alpha", action="store_true")
    parser.add_argument(
        "--include-hits",
        action="store_true",
        help="also add logged live GAME.FNT variants that already hit",
    )
    parser.add_argument(
        "--separate-runtime-atlas",
        action="store_true",
        help="legacy diagnostic mode: build a separate runtime-capture atlas",
    )
    parser.add_argument(
        "--proof-out",
        type=Path,
        default=MOD_FONT_WORK,
    )
    args = parser.parse_args()
    canonical_atlas = not args.separate_runtime_atlas

    dump_dir = args.dump_dir or latest_dump_dir()
    dump_dirs = [dump_dir] + args.extra_dump_dir
    if args.include_hits:
        variants = collect_game_variants([args.trace_dir] + args.extra_trace_dir)
    else:
        variants = collect_game_misses_from_dirs([args.trace_dir] + args.extra_trace_dir)
    if not variants:
        raise SystemExit(
            "no live GAME.FNT misses found in "
            + ", ".join(str(path) for path in [args.trace_dir] + args.extra_trace_dir)
        )

    pack_root = args.manifest.parent
    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    strip_existing(manifest)
    existing_keys = {str(entry.get("key", "")) for entry in manifest.get("entries", [])}
    variants = [
        variant for variant in variants
        if variant.key not in existing_keys
    ]
    if not variants:
        raise SystemExit("all logged live GAME.FNT variants are already covered")

    runtime_atlas, entries, sources, proof, skipped = build_atlas(
        variants,
        dump_dirs,
        args.scale,
        args.threshold,
        args.nearest,
        args.hard_alpha,
    )
    runtime_name = f"game_fnt_loading_runtime_{args.scale}x.dds"
    active_image = active_game_fnt_image(manifest)
    if canonical_atlas:
        fnt = load_fnt_tool()
        sheet = fnt.decode_fnt(args.fnt)
        active_path = pack_root / active_image
        active_atlas = Image.open(active_path).convert("RGBA")
        base_height = sheet.atlas.height * args.scale
        if active_atlas.width < sheet.atlas.width * args.scale:
            raise ValueError(f"{active_image} is narrower than the decoded GAME.FNT atlas")
        if active_atlas.height < base_height:
            raise ValueError(f"{active_image} is shorter than the decoded GAME.FNT atlas")
        base_atlas = active_atlas.crop((0, 0, active_atlas.width, base_height))
        combined_width = max(base_atlas.width, runtime_atlas.width)
        combined = Image.new(
            "RGBA",
            (combined_width, base_height + runtime_atlas.height),
            (0, 0, 0, 0),
        )
        combined.alpha_composite(base_atlas, (0, 0))
        combined.alpha_composite(runtime_atlas, (0, base_height))
        canonical_name = f"game_fnt_canonical_{args.scale}x.dds"
        canonical_image = f"images/ui/{canonical_name}"
        atlas_path = pack_root / canonical_image
        atlas_path.parent.mkdir(parents=True, exist_ok=True)
        combined.save(atlas_path)
        deployed_atlas = Image.open(atlas_path).convert("RGBA")

        base_keys = {
            str(key)
            for key, source_list in manifest.get("sources", {}).items()
            if any(str(source).startswith("GAME.FNT glyph ") for source in source_list)
        }
        for entry in manifest.get("entries", []):
            if str(entry.get("key", "")) in base_keys:
                entry["image"] = canonical_image
        for entry in entries:
            entry["image"] = canonical_image
            entry["y"] = int(entry["y"]) + base_height
    else:
        atlas_path = pack_root / "images" / "ui" / runtime_name
        atlas_path.parent.mkdir(parents=True, exist_ok=True)
        runtime_atlas.save(atlas_path)
        deployed_atlas = Image.open(atlas_path).convert("RGBA")

    manifest["entries"].extend(entries)
    manifest.setdefault("sources", {}).update(sources)
    if canonical_atlas:
        manifest["generator"] = re.sub(
            r"; GAME\.FNT [^;]+ font atlas",
            "",
            str(manifest.get("generator", "")),
        )
        suffix = f"; GAME.FNT canonical source and live loading {args.scale}x font atlas"
    else:
        suffix = f"; GAME.FNT live loading glyph supplement {args.scale}x font atlas"
    if suffix not in manifest.get("generator", ""):
        manifest["generator"] = manifest.get("generator", "") + suffix
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    referenced_images = {
        str(entry.get("image", "")) for entry in manifest.get("entries", [])
    }
    stale_candidates = {
        active_image,
        f"images/ui/{runtime_name}",
    }
    for stale_image in stale_candidates:
        if stale_image in referenced_images or stale_image == f"images/ui/{atlas_path.name}":
            continue
        stale_path = pack_root / stale_image
        if stale_path.exists():
            stale_path.unlink()

    args.proof_out.mkdir(parents=True, exist_ok=True)
    deployed_atlas.save(args.proof_out / f"game_fnt_{'canonical' if canonical_atlas else 'loading_runtime'}_{args.scale}x.png")
    proof.save(args.proof_out / "game_fnt_loading_runtime_proof.png")
    report = [
        f"trace_dir={args.trace_dir}",
        f"dump_dir={dump_dir}",
        f"extra_dump_dirs={';'.join(str(path) for path in args.extra_dump_dir)}",
        f"font_atlas={atlas_path}",
        f"scale={args.scale}",
        f"threshold={args.threshold}",
        f"atlas_mode={'canonical' if canonical_atlas else 'separate-runtime'}",
        f"include_hits={1 if args.include_hits else 0}",
        f"entries_added={len(entries)}",
        f"entries_skipped_missing_dump={len(skipped)}",
    ]
    if skipped:
        report.extend(
            f"skipped={variant.key} {variant.width}x{variant.height} {variant.uv}"
            for variant in skipped[:50]
        )
    (args.proof_out / "game_fnt_loading_runtime_report.txt").write_text(
        "\n".join(report) + "\n",
        encoding="utf-8",
    )
    print("\n".join(report))


if __name__ == "__main__":
    main()
