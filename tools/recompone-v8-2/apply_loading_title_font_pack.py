#!/usr/bin/env python3
"""Add targeted high-res replacements for the V8:2 loading title font."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
from pathlib import Path
import re

from PIL import Image, ImageDraw, ImageFilter


ROOT = Path(__file__).resolve().parents[2]
FNV_OFFSET = 14695981039346656037
FNV_PRIME = 1099511628211


@dataclass(frozen=True)
class TitleGlyph:
    key: str
    width: int
    height: int
    uv: str


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


def collect_title_glyphs(trace_dir: Path) -> list[TitleGlyph]:
    pattern = re.compile(
        r"V82LoadingUiResolve.*key=([0-9a-f]{16}) hit=0 .*"
        r"size=([0-9]+)x([0-9]+) uv=([^ ]+) "
        r"tpage=0x0(?:28|48) clut=0x780C"
    )
    glyphs: dict[str, TitleGlyph] = {}
    for log in sorted(trace_dir.glob("*.stderr.log")):
        for line in log.read_text(encoding="utf-8", errors="ignore").splitlines():
            match = pattern.search(line)
            if match is None:
                continue
            key, width_text, height_text, uv = match.groups()
            width = int(width_text)
            height = int(height_text)
            if key == "0000000000000000" or width <= 0 or height <= 0:
                continue
            glyphs.setdefault(key, TitleGlyph(key, width, height, uv))
    return sorted(glyphs.values(), key=lambda glyph: (glyph.height, glyph.uv, glyph.key))


def load_runtime_crop(dump_dir: Path, glyph: TitleGlyph) -> Image.Image:
    path = dump_dir / f"{glyph.key}_{glyph.width}x{glyph.height}.rgba"
    if not path.exists():
        raise FileNotFoundError(f"missing title glyph runtime dump: {path}")
    rgba = path.read_bytes()
    expected = glyph.width * glyph.height * 4
    if len(rgba) != expected:
        raise ValueError(f"{path} is {len(rgba)} bytes, expected {expected}")
    image = Image.frombytes("RGBA", (glyph.width, glyph.height), rgba)
    actual = runtime_hash(image)
    if f"{actual:016x}" != glyph.key:
        raise ValueError(f"{path} hashes to {actual:016x}")
    return image


def sharpen_font_crop(image: Image.Image, scale: int) -> Image.Image:
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
    alpha = alpha.filter(ImageFilter.UnsharpMask(radius=0.45, percent=120, threshold=0))
    alpha = alpha.point(lambda value: 0 if value < 14 else 255 if value > 238 else value)
    out = Image.new("RGBA", alpha.size, (255, 255, 255, 255))
    out.putalpha(alpha)
    return out


def build_atlas(
    glyphs: list[TitleGlyph],
    dump_dir: Path,
    scale: int,
) -> tuple[Image.Image, list[dict[str, object]], dict[str, list[str]], Image.Image]:
    padding = max(2, scale)
    max_width = 1936
    x = y = row_h = padding
    placements: list[tuple[TitleGlyph, Image.Image, int, int]] = []
    source_previews: list[tuple[TitleGlyph, Image.Image]] = []
    for glyph in glyphs:
        source = load_runtime_crop(dump_dir, glyph)
        upscaled = sharpen_font_crop(source, scale)
        source_previews.append((glyph, source))
        if x + upscaled.width + padding > max_width:
            x = padding
            y += row_h + padding
            row_h = padding
        placements.append((glyph, upscaled, x, y))
        x += upscaled.width + padding
        row_h = max(row_h, upscaled.height)
    atlas = Image.new("RGBA", (max_width, y + row_h + padding), (0, 0, 0, 0))
    entries: list[dict[str, object]] = []
    sources: dict[str, list[str]] = {}
    image_name = f"loading_title_fnt_runtime_{scale}x.dds"
    for glyph, upscaled, px, py in placements:
        atlas.alpha_composite(upscaled, (px, py))
        entries.append({
            "key": glyph.key,
            "image": f"images/ui/{image_name}",
            "x": px,
            "y": py,
            "width": upscaled.width,
            "height": upscaled.height,
        })
        sources[glyph.key] = [
            f"LOADING_TITLE.FNT glyph live crop uv={glyph.uv} size={glyph.width}x{glyph.height}"
        ]

    label_h = 18
    cell_h = max(g.height for g in glyphs) * scale + label_h + padding
    proof_w = min(max_width, 1280)
    proof_rows = []
    row = Image.new("RGBA", (proof_w, cell_h), (12, 12, 12, 255))
    draw = ImageDraw.Draw(row)
    cx = padding
    for glyph, source in source_previews:
        up = sharpen_font_crop(source, scale)
        if cx + up.width + padding > proof_w:
            proof_rows.append(row)
            row = Image.new("RGBA", (proof_w, cell_h), (12, 12, 12, 255))
            draw = ImageDraw.Draw(row)
            cx = padding
        draw.text((cx, 2), glyph.key[:8], fill=(190, 190, 190))
        row.alpha_composite(up, (cx, label_h))
        cx += up.width + padding
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
            "images/ui/loading_title_fnt_"
        )
    ]
    sources = manifest.setdefault("sources", {})
    stale_keys = [
        key for key, source_list in sources.items()
        if any(str(source).startswith("LOADING_TITLE.FNT glyph ") for source in source_list)
    ]
    for key in stale_keys:
        sources.pop(key, None)
    generator = str(manifest.get("generator", ""))
    manifest["generator"] = re.sub(
        r"; LOADING_TITLE\.FNT [^;]+ font atlas",
        "",
        generator,
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
    parser.add_argument("--scale", type=int, default=4)
    parser.add_argument(
        "--proof-out",
        type=Path,
        default=ROOT / "build" / "v82_font_source_investigation",
    )
    args = parser.parse_args()

    dump_dir = args.dump_dir or latest_dump_dir()
    glyphs = collect_title_glyphs(args.trace_dir)
    if not glyphs:
        raise SystemExit(f"no loading-title glyphs found in {args.trace_dir}")
    atlas, entries, sources, proof = build_atlas(glyphs, dump_dir, args.scale)

    pack_root = args.manifest.parent
    atlas_path = pack_root / "images" / "ui" / f"loading_title_fnt_runtime_{args.scale}x.dds"
    atlas_path.parent.mkdir(parents=True, exist_ok=True)
    atlas.save(atlas_path)

    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    strip_existing(manifest)
    manifest["entries"].extend(entries)
    manifest_sources = manifest.setdefault("sources", {})
    manifest_sources.update(sources)
    suffix = f"; LOADING_TITLE.FNT runtime loading-title {args.scale}x font atlas"
    if suffix not in manifest.get("generator", ""):
        manifest["generator"] = manifest.get("generator", "") + suffix
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    args.proof_out.mkdir(parents=True, exist_ok=True)
    atlas.save(args.proof_out / f"loading_title_fnt_runtime_{args.scale}x.png")
    proof.save(args.proof_out / "loading_title_fnt_runtime_proof.png")
    report = [
        f"trace_dir={args.trace_dir}",
        f"dump_dir={dump_dir}",
        f"font_atlas={atlas_path}",
        f"scale={args.scale}",
        f"entries_added={len(entries)}",
    ]
    (args.proof_out / "loading_title_fnt_runtime_report.txt").write_text(
        "\n".join(report) + "\n",
        encoding="utf-8",
    )
    print("\n".join(report))


if __name__ == "__main__":
    main()
