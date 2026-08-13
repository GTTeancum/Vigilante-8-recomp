#!/usr/bin/env python3
"""Add live loading-screen GAME.FNT glyph variants missed by source atlas keys."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
from pathlib import Path
import re

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[2]
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


def load_runtime_crop(dump_dir: Path, glyph: GlyphVariant) -> Image.Image:
    path = dump_dir / f"{glyph.key}_{glyph.width}x{glyph.height}.rgba"
    if not path.exists():
        raise FileNotFoundError(f"missing GAME.FNT glyph runtime dump: {path}")
    rgba = path.read_bytes()
    expected = glyph.width * glyph.height * 4
    if len(rgba) != expected:
        raise ValueError(f"{path} is {len(rgba)} bytes, expected {expected}")
    image = Image.frombytes("RGBA", (glyph.width, glyph.height), rgba)
    actual = runtime_hash(image)
    if f"{actual:016x}" != glyph.key:
        raise ValueError(f"{path} hashes to {actual:016x}")
    return image


def sharpen_source_glyph(image: Image.Image, scale: int, threshold: int) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = rgba.tobytes()
    alpha = bytearray(rgba.width * rgba.height)
    for index in range(0, len(pixels), 4):
        r, g, b, a = pixels[index:index + 4]
        value = max(r, g, b) if (a or r or g or b) else 0
        if value < threshold:
            alpha[index // 4] = 0
        else:
            mapped = int((value - threshold) * 255 / (255 - threshold))
            alpha[index // 4] = 255 if mapped > 232 else mapped
    mask = Image.frombytes("L", rgba.size, bytes(alpha)).resize(
        (rgba.width * scale, rgba.height * scale),
        Image.Resampling.LANCZOS,
    )
    mask = mask.point(lambda value: 0 if value < 10 else 255 if value > 244 else value)
    out = Image.new("RGBA", mask.size, (255, 255, 255, 255))
    out.putalpha(mask)
    return out


def build_atlas(
    variants: list[GlyphVariant],
    dump_dir: Path,
    scale: int,
    threshold: int,
) -> tuple[Image.Image, list[dict[str, object]], dict[str, list[str]], Image.Image]:
    padding = max(2, scale // 2)
    max_width = 1936
    x = y = row_h = padding
    placements: list[tuple[GlyphVariant, Image.Image, int, int]] = []
    previews: list[tuple[GlyphVariant, Image.Image]] = []
    for variant in variants:
        source = load_runtime_crop(dump_dir, variant)
        upscaled = sharpen_source_glyph(source, scale, threshold)
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
            f"threshold={threshold}"
        ]

    label_h = 18
    proof_w = min(max_width, 1280)
    cell_h = max(variant.height for variant in variants) * scale + label_h + padding
    proof_rows: list[Image.Image] = []
    row = Image.new("RGBA", (proof_w, cell_h), (12, 12, 12, 255))
    draw = ImageDraw.Draw(row)
    cx = padding
    for variant, source in previews:
        upscaled = sharpen_source_glyph(source, scale, threshold)
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
    return atlas, entries, sources, proof.convert("RGB")


def strip_existing(manifest: dict[str, object]) -> None:
    entries = manifest.get("entries", [])
    manifest["entries"] = [
        entry for entry in entries
        if not str(entry.get("image", "")).startswith(
            "images/ui/game_fnt_loading_runtime_"
        )
    ]
    sources = manifest.setdefault("sources", {})
    stale_keys = [
        key for key, source_list in sources.items()
        if any(str(source).startswith("GAME.FNT live loading glyph ") for source in source_list)
    ]
    for key in stale_keys:
        sources.pop(key, None)
    manifest["generator"] = re.sub(
        r"; GAME\.FNT live loading glyph supplement [^;]+ font atlas",
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
    parser.add_argument("--dump-dir", type=Path)
    parser.add_argument(
        "--manifest",
        type=Path,
        default=ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" / "manifest.json",
    )
    parser.add_argument("--scale", type=int, default=8)
    parser.add_argument("--threshold", type=int, default=96)
    parser.add_argument(
        "--proof-out",
        type=Path,
        default=MOD_FONT_WORK,
    )
    args = parser.parse_args()

    dump_dir = args.dump_dir or latest_dump_dir()
    variants = collect_game_misses(args.trace_dir)
    if not variants:
        raise SystemExit(f"no live GAME.FNT misses found in {args.trace_dir}")
    atlas, entries, sources, proof = build_atlas(
        variants, dump_dir, args.scale, args.threshold)

    pack_root = args.manifest.parent
    atlas_path = pack_root / "images" / "ui" / f"game_fnt_loading_runtime_{args.scale}x.dds"
    atlas_path.parent.mkdir(parents=True, exist_ok=True)
    atlas.save(atlas_path)
    deployed_atlas = Image.open(atlas_path).convert("RGBA")

    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    strip_existing(manifest)
    manifest["entries"].extend(entries)
    manifest.setdefault("sources", {}).update(sources)
    suffix = f"; GAME.FNT live loading glyph supplement {args.scale}x font atlas"
    if suffix not in manifest.get("generator", ""):
        manifest["generator"] = manifest.get("generator", "") + suffix
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    args.proof_out.mkdir(parents=True, exist_ok=True)
    deployed_atlas.save(args.proof_out / f"game_fnt_loading_runtime_{args.scale}x.png")
    proof.save(args.proof_out / "game_fnt_loading_runtime_proof.png")
    report = [
        f"trace_dir={args.trace_dir}",
        f"dump_dir={dump_dir}",
        f"font_atlas={atlas_path}",
        f"scale={args.scale}",
        f"threshold={args.threshold}",
        f"entries_added={len(entries)}",
    ]
    (args.proof_out / "game_fnt_loading_runtime_report.txt").write_text(
        "\n".join(report) + "\n",
        encoding="utf-8",
    )
    print("\n".join(report))


if __name__ == "__main__":
    main()
