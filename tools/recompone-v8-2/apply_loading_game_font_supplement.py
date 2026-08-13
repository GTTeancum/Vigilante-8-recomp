#!/usr/bin/env python3
"""Add live loading-screen GAME.FNT glyph variants missed by source atlas keys."""

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


@dataclass(frozen=True)
class MatchedGlyph:
    variant: GlyphVariant
    code: int
    char: str
    x: int
    y: int
    score: float


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


def parse_uv(uv: str) -> tuple[int, int]:
    first, _last = uv.split("-", 1)
    x_text, y_text = first.split(",", 1)
    return int(x_text), int(y_text)


def crop_mask(image: Image.Image) -> bytes:
    rgba = image.convert("RGBA")
    pixels = rgba.tobytes()
    mask = bytearray(rgba.width * rgba.height)
    for index in range(0, len(pixels), 4):
        r, g, b, a = pixels[index:index + 4]
        mask[index // 4] = 1 if a or r or g or b else 0
    return bytes(mask)


def mask_diff(a: bytes, b: bytes) -> float:
    if len(a) != len(b):
        return 1.0
    if not a:
        return 0.0
    return sum(1 for left, right in zip(a, b) if left != right) / len(a)


def useful_character(char: str) -> bool:
    return ord(char) >= 0x20


def match_live_variants(
    sheet,
    variants: list[GlyphVariant],
    dump_dirs: list[Path],
    radius: int,
) -> list[MatchedGlyph]:
    fnt = load_fnt_tool()
    source_atlas = fnt.opaque_crop(sheet.atlas)

    matched: list[MatchedGlyph] = []
    for variant in variants:
        try:
            runtime_crop = load_runtime_crop(dump_dirs, variant)
        except FileNotFoundError as error:
            print(f"[warn] {error}")
            continue
        runtime_mask = crop_mask(runtime_crop)
        best: tuple[int, str, int, int, float] | None = None
        for glyph in sheet.glyphs:
            if not useful_character(glyph.char) or glyph.width <= 0:
                continue
            if abs(glyph.width - variant.width) > radius:
                continue
            for dy in range(-radius, radius + 1):
                for dx in range(-radius, radius + 1):
                    x = glyph.x + dx
                    y = glyph.y + dy
                    if x < 0 or y < 0:
                        continue
                    if x + variant.width > source_atlas.width:
                        continue
                    if y + variant.height > source_atlas.height:
                        continue
                    crop = source_atlas.crop(
                        (x, y, x + variant.width, y + variant.height))
                    score = mask_diff(runtime_mask, crop_mask(crop))
                    score += abs(glyph.width - variant.width) * 0.015
                    if best is None or score < best[4]:
                        best = (glyph.code, glyph.char, x, y, score)
        if best is None:
            raise ValueError(
                f"no GAME.FNT source glyph candidate for live crop "
                f"{variant.key} {variant.width}x{variant.height}"
            )
        code, char, x, y, score = best
        matched.append(MatchedGlyph(variant, code, char, x, y, score))
    return matched


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
        source = load_runtime_crop([dump_dir], variant)
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
    parser.add_argument("--variant-radius", type=int, default=2)
    parser.add_argument(
        "--reuse-active-atlas",
        action="store_true",
        help="map live loading keys into matched cells of the active GAME.FNT atlas",
    )
    parser.add_argument(
        "--proof-out",
        type=Path,
        default=MOD_FONT_WORK,
    )
    args = parser.parse_args()

    dump_dir = args.dump_dir or latest_dump_dir()
    dump_dirs = [dump_dir] + args.extra_dump_dir
    if args.reuse_active_atlas:
        variants = collect_game_variants([args.trace_dir] + args.extra_trace_dir)
    else:
        variants = collect_game_misses(args.trace_dir)
    if not variants:
        raise SystemExit(f"no live GAME.FNT misses found in {args.trace_dir}")

    pack_root = args.manifest.parent
    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    strip_existing(manifest)
    if args.reuse_active_atlas:
        fnt = load_fnt_tool()
        sheet = fnt.decode_fnt(args.fnt)
        active_image = active_game_fnt_image(manifest)
        active_path = pack_root / active_image
        active_atlas = Image.open(active_path).convert("RGBA")
        entries: list[dict[str, object]] = []
        sources: dict[str, list[str]] = {}
        existing_keys = {str(entry.get("key", "")) for entry in manifest["entries"]}
        missing_variants = [
            variant for variant in variants
            if variant.key not in existing_keys
        ]
        matches = match_live_variants(
            sheet, missing_variants, dump_dirs, args.variant_radius)
        for match in matches:
            variant = match.variant
            entry = {
                "key": variant.key,
                "image": active_image,
                "x": match.x * args.scale,
                "y": match.y * args.scale,
                "width": variant.width * args.scale,
                "height": variant.height * args.scale,
            }
            if (
                entry["x"] < 0 or entry["y"] < 0 or
                entry["x"] + entry["width"] > active_atlas.width or
                entry["y"] + entry["height"] > active_atlas.height
            ):
                raise ValueError(f"{variant.key} crop outside {active_image}")
            entries.append(entry)
            sources[variant.key] = [
                f"GAME.FNT live loading glyph uv={variant.uv} "
                f"size={variant.width}x{variant.height} "
                f"tpage={variant.tpage} clut={variant.clut} "
                f"matched=0x{match.code:02X}/{match.char!r} "
                f"source={match.x},{match.y} "
                f"score={match.score:.4f} reuse={active_image}"
            ]
        atlas_path = active_path
        proof = None
    else:
        atlas, entries, sources, proof = build_atlas(
            variants, dump_dir, args.scale, args.threshold)
        atlas_path = pack_root / "images" / "ui" / f"game_fnt_loading_runtime_{args.scale}x.dds"
        atlas_path.parent.mkdir(parents=True, exist_ok=True)
        atlas.save(atlas_path)
        deployed_atlas = Image.open(atlas_path).convert("RGBA")

    manifest["entries"].extend(entries)
    manifest.setdefault("sources", {}).update(sources)
    suffix = (
        f"; GAME.FNT live loading glyph supplement {args.scale}x "
        f"{'active-atlas' if args.reuse_active_atlas else 'font atlas'}"
    )
    if suffix not in manifest.get("generator", ""):
        manifest["generator"] = manifest.get("generator", "") + suffix
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    args.proof_out.mkdir(parents=True, exist_ok=True)
    if proof is not None:
        deployed_atlas.save(args.proof_out / f"game_fnt_loading_runtime_{args.scale}x.png")
        proof.save(args.proof_out / "game_fnt_loading_runtime_proof.png")
    report = [
        f"trace_dir={args.trace_dir}",
        f"dump_dir={dump_dir}",
        f"extra_dump_dirs={';'.join(str(path) for path in args.extra_dump_dir)}",
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
