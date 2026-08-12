#!/usr/bin/env python3
"""Audit deployed GAME.FNT texture-pack entries from final DDS readback."""

from __future__ import annotations

import argparse
import importlib.util
import json
from pathlib import Path
import sys

from PIL import Image


ROOT = Path(__file__).resolve().parents[2]
FNT_TOOL = ROOT / "tools" / "recompone-v8-2" / "build_fnt_font_proof.py"
PACK_TOOL = ROOT / "tools" / "recompone-v8-2" / "apply_game_fnt_font_pack.py"


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
        default=ROOT / "build" / "v82_font_source_investigation" /
        "game_fnt_deployed_audit.txt",
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
    low_alpha_errors: list[str] = []
    empty_errors: list[str] = []
    bounds_errors: list[str] = []
    for key in sorted(expected_keys):
        if key not in entries:
            continue
        entry = entries[key]
        image_name = entry["image"]
        if image_name not in image_cache:
            image_cache[image_name] = Image.open(pack_root / image_name).convert("RGBA")
        image = image_cache[image_name]
        x, y = int(entry["x"]), int(entry["y"])
        width, height = int(entry["width"]), int(entry["height"])
        if x < 0 or y < 0 or x + width > image.width or y + height > image.height:
            bounds_errors.append(f"{key} outside {image_name}")
            continue
        crop = image.crop((x, y, x + width, y + height))
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
        image_name = entry["image"]
        if image_name not in image_cache:
            image_cache[image_name] = Image.open(pack_root / image_name).convert("RGBA")
        image = image_cache[image_name]
        x, y = int(entry["x"]), int(entry["y"])
        width, height = int(entry["width"]), int(entry["height"])
        crop = image.crop((x, y, x + width, y + height))
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
    details = missing + bounds_errors + low_alpha_errors + empty_errors + \
        live_low_alpha_errors + live_empty_errors
    if details:
        report.append("details:")
        report.extend(details[:200])
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text("\n".join(report) + "\n", encoding="utf-8")
    print("\n".join(report))
    return 1 if details else 0


if __name__ == "__main__":
    raise SystemExit(main())
