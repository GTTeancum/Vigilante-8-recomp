#!/usr/bin/env python3
"""Patch the loose V8:2 texture pack with source-sheet FNT font atlases."""

from __future__ import annotations

import argparse
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


def load_fnt_tool():
    spec = importlib.util.spec_from_file_location("build_fnt_font_proof", PROOF_TOOL)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {PROOF_TOOL}")
    module = importlib.util.module_from_spec(spec)
    sys.modules["build_fnt_font_proof"] = module
    spec.loader.exec_module(module)
    return module


def useful_character(char: str) -> bool:
    return ord(char) >= 0x20


def safe_stem(path: Path) -> str:
    return re.sub(r"[^a-z0-9]+", "_", path.stem.lower()).strip("_")


def source_stem(path: Path) -> str:
    stem = safe_stem(path)
    stem = re.sub(r"^[a-z0-9]+_fnt_source_", "", stem)
    stem = re.sub(r"_[0-9]+x$", "", stem)
    return stem or "native_nearest"


def font_label(path: Path) -> str:
    return f"{path.stem.upper()}.FNT"


def atlas_prefix(path: Path) -> str:
    return f"{path.stem.lower()}_fnt"


def transparent_font_atlas(image: Image.Image, threshold: int = 24) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = bytearray(rgba.tobytes())
    for index in range(0, len(pixels), 4):
        r, g, b, _a = pixels[index:index + 4]
        alpha = max(r, g, b)
        if alpha < threshold:
            pixels[index:index + 4] = b"\x00\x00\x00\x00"
        else:
            alpha = max(
                0,
                min(255, int((alpha - threshold) * 255 / (255 - threshold))),
            )
            if alpha < 10:
                alpha = 0
            if alpha > 232:
                alpha = 255
            pixels[index:index + 4] = bytes((255, 255, 255, alpha))
    return Image.frombytes("RGBA", rgba.size, bytes(pixels))


def transparent_source_copy(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = bytearray(rgba.tobytes())
    for index in range(0, len(pixels), 4):
        r, g, b, _a = pixels[index:index + 4]
        alpha = max(r, g, b)
        if alpha <= 6:
            pixels[index:index + 4] = b"\x00\x00\x00\x00"
        else:
            pixels[index + 3] = alpha
    return Image.frombytes("RGBA", rgba.size, bytes(pixels))


def scale2x(image: Image.Image) -> Image.Image:
    source = image.convert("RGBA")
    pixels = source.load()
    out = Image.new("RGBA", (source.width * 2, source.height * 2), (0, 0, 0, 0))
    target = out.load()
    for y in range(source.height):
        for x in range(source.width):
            e = pixels[x, y]
            b = pixels[x, y - 1] if y > 0 else e
            d = pixels[x - 1, y] if x > 0 else e
            f = pixels[x + 1, y] if x + 1 < source.width else e
            h = pixels[x, y + 1] if y + 1 < source.height else e
            if b != h and d != f:
                e0 = d if d == b else e
                e1 = f if b == f else e
                e2 = d if d == h else e
                e3 = f if h == f else e
            else:
                e0 = e1 = e2 = e3 = e
            ox = x * 2
            oy = y * 2
            target[ox, oy] = e0
            target[ox + 1, oy] = e1
            target[ox, oy + 1] = e2
            target[ox + 1, oy + 1] = e3
    return out


def clean_mask_atlas(image: Image.Image, threshold: int = 96) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = bytearray(rgba.tobytes())
    for index in range(0, len(pixels), 4):
        r, g, b, a = pixels[index:index + 4]
        intensity = max(r, g, b) if a else 0
        if intensity < threshold:
            pixels[index:index + 4] = b"\x00\x00\x00\x00"
        else:
            alpha = max(
                0,
                min(255, int((intensity - threshold) * 255 / (255 - threshold))),
            )
            if alpha < 10:
                alpha = 0
            if alpha > 232:
                alpha = 255
            pixels[index:index + 4] = bytes((255, 255, 255, alpha))
    return Image.frombytes("RGBA", rgba.size, bytes(pixels))


def alpha_only_font_atlas(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = bytearray(rgba.tobytes())
    for index in range(0, len(pixels), 4):
        alpha = pixels[index + 3]
        if alpha < 96:
            pixels[index:index + 4] = b"\x00\x00\x00\x00"
        else:
            pixels[index:index + 4] = b"\xff\xff\xff\xff"
    return Image.frombytes("RGBA", rgba.size, bytes(pixels))


def read_deployed_atlas(path: Path) -> Image.Image:
    return Image.open(path).convert("RGBA")


def deployed_font_proof(sheet, image: Image.Image, label: str) -> Image.Image:
    native = fnt.black_preview(sheet.atlas, 3)
    scaled = fnt.black_preview(image, 1).resize(
        (sheet.atlas.width * 3, sheet.atlas.height * 3),
        Image.Resampling.LANCZOS,
    )
    width = max(native.width, scaled.width)
    label_h = 24
    gap = 12
    proof = Image.new(
        "RGB",
        (width, label_h * 2 + native.height + scaled.height + gap),
        (10, 10, 10),
    )
    draw = ImageDraw.Draw(proof)
    draw.text(
        (8, 6),
        f"Original {font_label(sheet.path)} source atlas",
        fill=(230, 230, 230),
    )
    proof.paste(native, (0, label_h))
    y = label_h + native.height + gap
    draw.text((8, y + 5), label, fill=(230, 230, 230))
    proof.paste(scaled, (0, y + label_h))
    return proof


def make_crisp_source_atlas(sheet, scale: int, threshold: int) -> Image.Image:
    image = clean_mask_atlas(fnt.opaque_crop(sheet.atlas), threshold)
    current_scale = 1
    while current_scale * 2 <= scale:
        image = scale2x(image)
        current_scale *= 2
    if current_scale != scale:
        image = image.resize(
            (sheet.atlas.width * scale, sheet.atlas.height * scale),
            Image.Resampling.NEAREST,
        )
    return image


def make_model_source_atlas(
    sheet,
    scale: int,
    source_upscale: Path | None,
    threshold: int,
) -> Image.Image:
    target_size = (sheet.atlas.width * scale, sheet.atlas.height * scale)
    if source_upscale is not None and source_upscale.exists():
        image = Image.open(source_upscale).convert("RGBA")
        if image.size != target_size:
            image = image.resize(target_size, Image.Resampling.LANCZOS)
        return transparent_font_atlas(image, threshold)
    source = fnt.opaque_crop(sheet.atlas).resize(target_size, Image.Resampling.NEAREST)
    return transparent_source_copy(source)


def strip_previous_generator_suffixes(generator: str, label: str) -> str:
    known_suffixes = (
        "; GAME.FNT fitted Cambria Bold Italic source-sheet font atlas",
        "; GAME.FNT fitted Times New Roman Bold Italic source-sheet font atlas",
        "; GAME.FNT fitted timesbi source-sheet font atlas",
        "; GAME.FNT fitted CALISTI source-sheet font atlas",
        "; GAME.FNT fitted calisti source-sheet font atlas",
    )
    for suffix in known_suffixes:
        generator = generator.replace(suffix, "")
    escaped = re.escape(label)
    return re.sub(
        rf"; {escaped} [^;]+ font atlas",
        "",
        generator,
    )


def crop_hash_entries(
    sheet,
    scale: int,
    variant_radius: int,
    atlas_name: str,
    replacement: Image.Image | None = None,
):
    seen: set[int] = set()
    for glyph in sheet.glyphs:
        if not useful_character(glyph.char) or glyph.width <= 0:
            continue
        for dy in range(-variant_radius, variant_radius + 1):
            for dx in range(-variant_radius, variant_radius + 1):
                x = glyph.x + dx
                y = glyph.y + dy
                if x < 0 or y < 0:
                    continue
                if x + glyph.width > sheet.atlas.width:
                    continue
                if y + sheet.max_height > sheet.atlas.height:
                    continue
                crop = sheet.atlas.crop(
                    (x, y, x + glyph.width, y + sheet.max_height)
                )
                if crop.getbbox() is None:
                    continue
                scaled_box = (
                    x * scale,
                    y * scale,
                    (x + glyph.width) * scale,
                    (y + sheet.max_height) * scale,
                )
                if replacement is not None:
                    replacement_crop = replacement.crop(scaled_box)
                    if replacement_crop.getchannel("A").getbbox() is None:
                        continue
                key = fnt.runtime_hash(crop)
                if key in seen:
                    continue
                seen.add(key)
                yield {
                    "key": f"{key:016x}",
                    "image": f"images/ui/{atlas_name}",
                    "x": scaled_box[0],
                    "y": scaled_box[1],
                    "width": glyph.width * scale,
                    "height": sheet.max_height * scale,
                }, (
                    f"{font_label(sheet.path)} glyph 0x{glyph.code:02X} "
                    f"{glyph.char!r} crop dx={dx} dy={dy}"
                )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--fnt", type=Path, default=DEFAULT_FNT)
    parser.add_argument("--manifest", type=Path, default=ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" / "manifest.json")
    parser.add_argument("--mode", choices=("crisp", "source", "ttf"), default="crisp")
    parser.add_argument("--font", type=Path, default=Path("C:/Windows/Fonts/timesbi.ttf"))
    parser.add_argument("--point-size", type=int, default=18)
    parser.add_argument("--width-factor", type=float, default=0.92)
    parser.add_argument("--shadow-alpha", type=int, default=0)
    parser.add_argument("--scale", type=int, default=4)
    parser.add_argument("--threshold", type=int, default=96)
    parser.add_argument("--variant-radius", type=int, default=2)
    parser.add_argument("--proof-out", type=Path, default=MOD_FONT_WORK)
    parser.add_argument(
        "--source-upscale",
        type=Path,
        default=None,
    )
    args = parser.parse_args()

    global fnt
    fnt = load_fnt_tool()
    sheet = fnt.decode_fnt(args.fnt)
    prefix = atlas_prefix(args.fnt)
    source_upscale = args.source_upscale or (
        args.proof_out / f"{prefix}_source_x4plus_anime_{args.scale}x.png"
    )
    if args.mode == "ttf":
        image = fnt.render_fitted_ttf_atlas(
            sheet,
            args.font,
            args.point_size,
            args.scale,
            width_factor=args.width_factor,
            shadow_alpha=args.shadow_alpha,
        )
        image = alpha_only_font_atlas(image)
        atlas_label = f"hard-mask fitted {args.font.stem}"
        atlas_name = f"{prefix}_{safe_stem(args.font)}_{args.scale}x.dds"
    elif args.mode == "source":
        image = make_model_source_atlas(
            sheet,
            args.scale,
            source_upscale,
            args.threshold,
        )
        source_label = source_stem(source_upscale) if source_upscale.exists() else "native_nearest"
        atlas_label = f"source-sheet {source_label} threshold{args.threshold}"
        atlas_name = f"{prefix}_source_{source_label}_{args.scale}x.dds"
    else:
        image = make_crisp_source_atlas(sheet, args.scale, args.threshold)
        atlas_label = f"cutoff-alpha source-sheet threshold{args.threshold}"
        atlas_name = f"{prefix}_source_scale2x_{args.scale}x.dds"

    pack_root = args.manifest.parent
    atlas_relative = Path("images") / "ui" / atlas_name
    atlas_path = pack_root / atlas_relative
    atlas_path.parent.mkdir(parents=True, exist_ok=True)
    image.save(atlas_path)
    deployed_image = read_deployed_atlas(atlas_path)

    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    generated = list(
        crop_hash_entries(
            sheet,
            args.scale,
            args.variant_radius,
            atlas_name,
            deployed_image,
        )
    )
    font_entries = [entry for entry, _source in generated]
    font_keys = {entry["key"] for entry in font_entries}
    superseded = [
        entry for entry in manifest["entries"]
        if entry.get("key") in font_keys
    ]
    manifest["entries"] = [
        entry for entry in manifest["entries"]
        if entry.get("key") not in font_keys
    ]
    manifest["entries"].extend(font_entries)
    sources = manifest.setdefault("sources", {})
    for entry, source in generated:
        sources[entry["key"]] = [source]
    label = font_label(args.fnt)
    manifest["generator"] = strip_previous_generator_suffixes(manifest.get("generator", ""), label)
    generator = manifest.get("generator", "")
    suffix = f"; {label} {atlas_label} font atlas"
    if suffix not in generator:
        manifest["generator"] = generator + suffix
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    referenced = {entry["image"] for entry in manifest["entries"]}
    for entry in superseded:
        image_name = entry.get("image", "")
        if not image_name.startswith("images/ui/") or image_name in referenced:
            continue
        stale = pack_root / image_name
        if stale.name.lower() == atlas_relative.name.lower():
            continue
        if stale.exists():
            stale.unlink()

    args.proof_out.mkdir(parents=True, exist_ok=True)
    proof_prefix = atlas_prefix(args.fnt)
    deployed_image.save(args.proof_out / f"{proof_prefix}_deployed_font_atlas_4x.png")
    deployed_font_proof(
        sheet,
        deployed_image,
        f"Final DDS readback: {atlas_label}",
    ).save(args.proof_out / f"{proof_prefix}_deployed_font_atlas_proof.png")
    report = [
        f"mode={args.mode}",
        f"font_atlas={atlas_path}",
        f"scale={args.scale}",
        f"threshold={args.threshold}",
        f"variant_radius={args.variant_radius}",
        f"entries_added_or_replaced={len(font_entries)}",
        f"unique_keys={len(font_keys)}",
    ]
    if args.mode == "ttf":
        report[2:2] = [
            f"font={args.font}",
            f"point_size={args.point_size}",
            f"width_factor={args.width_factor}",
            f"shadow_alpha={args.shadow_alpha}",
        ]
    elif args.mode == "source":
        report[2:2] = [f"source_upscale={source_upscale}"]
    else:
        report[2:2] = ["source_upscale=scale2x_from_decoded_fnt"]
    (args.proof_out / f"{proof_prefix}_deployed_font_pack_report.txt").write_text(
        "\n".join(report) + "\n",
        encoding="utf-8",
    )


if __name__ == "__main__":
    main()
