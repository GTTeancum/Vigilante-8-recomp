#!/usr/bin/env python3
"""Extract every loose V8:2 FNT and install whole-file HD atlas sidecars."""

from __future__ import annotations

import argparse
import importlib.util
import json
from pathlib import Path
import re
import sys

from PIL import Image


ROOT = Path(__file__).resolve().parents[2]
LOOSE = ROOT / "V8_2_LOOSE"
PACK = LOOSE / "mods" / "enhanced_textures_2x"
FNT_TOOL = ROOT / "tools" / "recompone-v8-2" / "build_fnt_font_proof.py"
LOAD_TBL_TOOL = (
    ROOT / "tools" / "recompone-v8-2" /
    "build_load_tbl_font_replacements.py"
)
HD_ATLASES = {
    "SHARED/GAME.FNT": (
        LOOSE / "mods" / "ttf_game_font" / "files" / "SHARED" / "GAME.FNT",
        LOOSE / "mods" / "ttf_game_font" / "proof" /
        "compiled_game_fnt_atlas_4x.tga",
    ),
}
def load_module(path: Path, name: str):
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


def font_entry(source_list: object, image: str) -> bool:
    sources = source_list if isinstance(source_list, list) else [source_list]
    return (
        any(".FNT" in str(source).upper() for source in sources) or
        "_fnt_" in image.casefold() or
        "font" in Path(image).name.casefold()
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--loose", type=Path, default=LOOSE)
    parser.add_argument("--pack", type=Path, default=PACK)
    parser.add_argument("--scale", type=int, default=4)
    args = parser.parse_args()
    if args.scale < 2:
        parser.error("file-font scale must be at least 2x")

    fnt = load_module(FNT_TOOL, "v82_file_font_decoder")
    load_tbl = load_module(LOAD_TBL_TOOL, "v82_load_tbl_font_builder")
    manifest_path = args.pack / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    sources = manifest.get("sources", {})

    removed_images: set[str] = set()
    retained_entries: list[dict[str, object]] = []
    removed_keys: set[str] = set()
    for entry in manifest.get("entries", []):
        key = str(entry.get("key", ""))
        image = str(entry.get("image", ""))
        if font_entry(sources.get(key, []), image):
            removed_keys.add(key)
            removed_images.add(image)
        else:
            retained_entries.append(entry)
    manifest["entries"] = retained_entries
    for key in removed_keys:
        sources.pop(key, None)

    image_dir = args.pack / "images" / "fonts"
    source_dir = args.pack / "font_sources"
    image_dir.mkdir(parents=True, exist_ok=True)
    source_dir.mkdir(parents=True, exist_ok=True)
    font_files: list[dict[str, object]] = []
    font_paths = sorted(
        (
            path.relative_to(args.loose)
            for path in args.loose.rglob("*.FNT")
            if "mods" not in {
                part.casefold() for part in path.relative_to(args.loose).parts
            }
        ),
        key=lambda path: path.as_posix().casefold(),
    )
    for relative in font_paths:
        path = args.loose / relative
        if not path.is_file():
            continue
        sheet = fnt.decode_fnt(path)
        source = fnt.opaque_crop(sheet.atlas).convert("RGBA")
        safe = re.sub(r"[^a-z0-9]+", "_", relative.as_posix().lower()).strip("_")
        source_path = source_dir / f"{safe}.png"
        source.save(source_path)
        normalized = relative.as_posix().upper()
        replacement = HD_ATLASES.get(normalized)
        if replacement is None:
            print(f"fallback original {relative}: no authored HD font sidecar")
            continue
        replacement_fnt, replacement_atlas = replacement
        if not replacement_fnt.is_file() or not replacement_atlas.is_file():
            print(f"fallback original {relative}: HD font files unavailable")
            continue
        replacement_sheet = fnt.decode_fnt(replacement_fnt)
        with Image.open(replacement_atlas) as generated:
            hd = generated.convert("RGBA")
        expected_size = (
            replacement_sheet.atlas.width * args.scale,
            replacement_sheet.atlas.height * args.scale,
        )
        if hd.size != expected_size:
            raise ValueError(
                f"HD atlas size mismatch for {relative}: "
                f"{hd.size} != {expected_size}"
            )
        image_relative = f"images/fonts/{safe}_{args.scale}x.dds"
        hd.save(args.pack / image_relative)
        font_files.append({
            "path": relative.as_posix(),
            "image": image_relative,
            "sourceWidth": replacement_sheet.atlas.width,
            "sourceHeight": replacement_sheet.atlas.height,
        })

    font_files.extend(load_tbl.build(
        args.loose / "SHELL" / "LOAD.TBL",
        image_dir,
        args.scale,
    ))

    manifest["fontFiles"] = font_files
    manifest["sources"] = sources
    generator_parts = [
        part.strip()
        for part in str(manifest.get("generator", "")).split(";")
        if part.strip() and "FNT" not in part.upper()
    ]
    generator_parts.append(
        f"file-provenance FNT atlases {args.scale}x"
    )
    manifest["generator"] = "; ".join(generator_parts)
    manifest_path.write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )

    referenced = {
        str(entry.get("image", "")) for entry in manifest["entries"]
    } | {str(entry["image"]) for entry in font_files}
    deleted: list[str] = []
    for relative in sorted(removed_images):
        if relative in referenced:
            continue
        target = (args.pack / relative).resolve()
        image_root = (args.pack / "images").resolve()
        if image_root not in target.parents:
            raise ValueError(f"refusing to remove font image outside pack: {target}")
        if target.is_file():
            target.unlink()
            deleted.append(relative)
    for target in sorted(image_dir.glob("*.dds")):
        relative = target.relative_to(args.pack).as_posix()
        if relative not in referenced:
            target.unlink()
            deleted.append(relative)
    # compile_ttf_fnt historically emitted hash-routed intermediate atlases
    # under images/ui.  The file-provenance build copies the authored atlas to
    # images/fonts, so those exact generated intermediates are obsolete.
    for target in sorted(
            (args.pack / "images" / "ui").glob("game_fnt_compiled_*.dds")):
        relative = target.relative_to(args.pack).as_posix()
        if relative not in referenced:
            target.unlink()
            deleted.append(relative)

    print(f"file_fonts={len(font_files)}")
    for entry in font_files:
        print(
            f"{entry['path']} -> {entry['image']} "
            f"source={entry['sourceWidth']}x{entry['sourceHeight']}"
        )
    print(f"removed_hash_regions={len(removed_keys)}")
    print(f"removed_obsolete_images={len(deleted)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
