#!/usr/bin/env python3
"""Audit deployed GAME.FNT texture-pack entries from final DDS readback."""

from __future__ import annotations

import argparse
import csv
import importlib.util
import json
from pathlib import Path
import sys

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[2]
FNT_TOOL = ROOT / "tools" / "recompone-v8-2" / "build_fnt_font_proof.py"
PACK_TOOL = ROOT / "tools" / "recompone-v8-2" / "apply_game_fnt_font_pack.py"
MOD_FONT_WORK = (
    ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" / "font_work"
)


def load_module(path: Path, name: str):
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


def alpha_values(image: Image.Image) -> set[int]:
    return {
        value for value, count in enumerate(image.convert("RGBA")
        .getchannel("A")
        .histogram())
        if count
    }


def active_game_fnt_image(manifest: dict[str, object]) -> str:
    counts: dict[str, int] = {}
    sources = manifest.get("sources", {})
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
        raise ValueError("manifest has no GAME.FNT glyph source entries")
    return max(counts.items(), key=lambda item: item[1])[0]


def entry_crop(
    pack_root: Path,
    image_cache: dict[str, Image.Image],
    entry: dict[str, object],
) -> Image.Image:
    image_name = str(entry["image"])
    if image_name not in image_cache:
        image_cache[image_name] = Image.open(pack_root / image_name).convert("RGBA")
    image = image_cache[image_name]
    x, y = int(entry["x"]), int(entry["y"])
    width, height = int(entry["width"]), int(entry["height"])
    return image.crop((x, y, x + width, y + height))


def preview_crop(crop: Image.Image) -> Image.Image:
    base = Image.new("RGBA", crop.size, (0, 0, 0, 255))
    base.alpha_composite(crop.convert("RGBA"))
    return base.convert("RGB")


def safe_char(char: str) -> str:
    code = ord(char)
    if code < 0x20 or code == 0x7F:
        return f"0x{code:02X}"
    if code >= 0x80:
        return f"0x{code:02X}"
    if char == "\\":
        return "\\\\"
    return char


def write_base_glyph_proof(
    path: Path,
    sheet,
    scale: int,
    deployed_source: Image.Image,
) -> None:
    columns = 8
    label_h = 18
    gap = 8
    cell_w = sheet.max_width * scale + gap
    cell_h = sheet.max_height * scale + label_h + gap
    rows = (len(sheet.glyphs) + columns - 1) // columns
    proof = Image.new("RGB", (columns * cell_w, rows * cell_h), (10, 10, 10))
    draw = ImageDraw.Draw(proof)
    for index, glyph in enumerate(sheet.glyphs):
        x = (index % columns) * cell_w
        y = (index // columns) * cell_h
        crop = deployed_source.crop((
            glyph.x * scale,
            glyph.y * scale,
            (glyph.x + glyph.width) * scale,
            (glyph.y + sheet.max_height) * scale,
        ))
        draw.text(
            (x + 2, y + 2),
            f"{glyph.code:02X} {safe_char(glyph.char)}",
            fill=(210, 210, 210),
        )
        proof.paste(preview_crop(crop), (x + 2, y + label_h))
    path.parent.mkdir(parents=True, exist_ok=True)
    proof.save(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--fnt",
        type=Path,
        default=ROOT / "V8_2_LOOSE" / "SHARED" / "GAME.FNT",
    )
    parser.add_argument(
        "--manifest",
        type=Path,
        default=ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" /
        "manifest.json",
    )
    parser.add_argument("--scale", type=int, default=8)
    parser.add_argument("--variant-radius", type=int, default=2)
    parser.add_argument("--min-visible-alpha", type=int, default=10)
    parser.add_argument(
        "--report",
        type=Path,
        default=MOD_FONT_WORK / "game_fnt_deployed_audit.txt",
    )
    parser.add_argument(
        "--per-glyph-report",
        type=Path,
        default=MOD_FONT_WORK / "game_fnt_deployed_glyph_audit.csv",
    )
    parser.add_argument(
        "--proof",
        type=Path,
        default=MOD_FONT_WORK / "game_fnt_final_dds_glyph_audit_sheet.png",
    )
    args = parser.parse_args()

    fnt = load_module(FNT_TOOL, "build_fnt_font_proof")
    pack = load_module(PACK_TOOL, "apply_game_fnt_font_pack")
    pack.fnt = fnt
    sheet = fnt.decode_fnt(args.fnt)
    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    entries = {entry["key"]: entry for entry in manifest["entries"]}
    pack_root = args.manifest.parent
    source_image = active_game_fnt_image(manifest)
    deployed_source = Image.open(pack_root / source_image).convert("RGBA")
    atlas_name = Path(source_image).name

    generated = list(
        pack.crop_hash_entries(
            sheet,
            args.scale,
            args.variant_radius,
            atlas_name,
            deployed_source,
        )
    )
    expected_keys = {entry["key"] for entry, _source in generated}
    missing = sorted(key for key in expected_keys if key not in entries)

    image_cache: dict[str, Image.Image] = {}
    base_missing: list[str] = []
    base_low_alpha_errors: list[str] = []
    base_empty_errors: list[str] = []
    base_rows: list[dict[str, object]] = []
    for glyph in sheet.glyphs:
        x = glyph.x
        y = glyph.y
        crop = sheet.atlas.crop(
            (x, y, x + glyph.width, y + sheet.max_height)
        )
        if crop.getbbox() is None:
            continue
        key = f"{fnt.runtime_hash(crop):016x}"
        if key not in entries:
            base_missing.append(f"0x{glyph.code:02X} {safe_char(glyph.char)} {key}")
            continue
        deployed_crop = entry_crop(pack_root, image_cache, entries[key])
        values = alpha_values(deployed_crop)
        faint = [value for value in values if 0 < value < args.min_visible_alpha]
        if faint:
            base_low_alpha_errors.append(
                f"0x{glyph.code:02X} {safe_char(glyph.char)} {key} alpha={faint[:16]}"
            )
        if 255 not in values:
            base_empty_errors.append(f"0x{glyph.code:02X} {safe_char(glyph.char)} {key}")
        base_rows.append({
            "code": f"0x{glyph.code:02X}",
            "char": safe_char(glyph.char),
            "key": key,
            "width": glyph.width,
            "height": sheet.max_height,
            "alpha_min": min(values) if values else "",
            "alpha_max": max(values) if values else "",
            "alpha_values": " ".join(str(value) for value in sorted(values)),
        })

    low_alpha_errors: list[str] = []
    empty_errors: list[str] = []
    bounds_errors: list[str] = []
    for key in sorted(expected_keys):
        if key not in entries:
            continue
        entry = entries[key]
        image_name = str(entry["image"])
        if image_name not in image_cache:
            image_cache[image_name] = Image.open(pack_root / image_name).convert("RGBA")
        image = image_cache[image_name]
        x, y = int(entry["x"]), int(entry["y"])
        width, height = int(entry["width"]), int(entry["height"])
        if x < 0 or y < 0 or x + width > image.width or y + height > image.height:
            bounds_errors.append(f"{key} outside {image_name}")
            continue
        crop = entry_crop(pack_root, image_cache, entry)
        values = alpha_values(crop)
        faint = [value for value in values if 0 < value < args.min_visible_alpha]
        if faint:
            low_alpha_errors.append(f"{key} alpha={faint[:16]}")
        if 255 not in values:
            empty_errors.append(key)

    live_entries = [
        entry for entry in manifest["entries"]
        if str(entry.get("image", "")).startswith(
            "images/ui/game_fnt_loading_runtime_"
        )
    ]
    live_low_alpha_errors: list[str] = []
    live_empty_errors: list[str] = []
    for entry in live_entries:
        crop = entry_crop(pack_root, image_cache, entry)
        values = alpha_values(crop)
        faint = [value for value in values if 0 < value < args.min_visible_alpha]
        if faint:
            live_low_alpha_errors.append(f"{entry['key']} alpha={faint[:16]}")
        if 255 not in values:
            live_empty_errors.append(entry["key"])

    report = [
        f"fnt={args.fnt}",
        f"manifest={args.manifest}",
        f"source_image={source_image}",
        f"glyph_records={len(sheet.glyphs)}",
        f"base_glyph_keys={len(base_rows)}",
        f"base_missing_keys={len(base_missing)}",
        f"base_low_alpha_errors={len(base_low_alpha_errors)}",
        f"base_empty_crops={len(base_empty_errors)}",
        f"source_variant_keys={len(expected_keys)}",
        f"missing_source_keys={len(missing)}",
        f"source_bounds_errors={len(bounds_errors)}",
        f"min_visible_alpha={args.min_visible_alpha}",
        f"source_low_alpha_errors={len(low_alpha_errors)}",
        f"source_empty_crops={len(empty_errors)}",
        f"live_supplement_entries={len(live_entries)}",
        f"live_low_alpha_errors={len(live_low_alpha_errors)}",
        f"live_empty_crops={len(live_empty_errors)}",
    ]
    details = base_missing + base_low_alpha_errors + base_empty_errors + \
        missing + bounds_errors + low_alpha_errors + empty_errors + \
        live_low_alpha_errors + live_empty_errors
    if details:
        report.append("details:")
        report.extend(details[:200])
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text("\n".join(report) + "\n", encoding="utf-8")
    args.per_glyph_report.parent.mkdir(parents=True, exist_ok=True)
    with args.per_glyph_report.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=(
                "code",
                "char",
                "key",
                "width",
                "height",
                "alpha_min",
                "alpha_max",
                "alpha_values",
            ),
        )
        writer.writeheader()
        writer.writerows(base_rows)
    write_base_glyph_proof(args.proof, sheet, args.scale, deployed_source)
    print("\n".join(report))
    return 1 if details else 0


if __name__ == "__main__":
    raise SystemExit(main())
