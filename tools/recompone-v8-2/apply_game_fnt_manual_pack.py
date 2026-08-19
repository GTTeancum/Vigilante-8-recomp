#!/usr/bin/env python3
"""Deploy a measured manual GAME.FNT donor as one canonical texture atlas."""

from __future__ import annotations

import argparse
import importlib.util
import json
from pathlib import Path
import re
import sys

from PIL import Image


ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / "tools" / "recompone-v8-2"
FNT_TOOL = TOOLS / "build_fnt_font_proof.py"
PACK_TOOL = TOOLS / "apply_game_fnt_font_pack.py"
TEXTMAP_TOOL = TOOLS / "apply_loading_game_font_textmap.py"
DEFAULT_FNT = ROOT / "V8_2_LOOSE" / "SHARED" / "GAME.FNT"
DEFAULT_LOAD = ROOT / "V8_2_LOOSE" / "SHELL" / "LOAD.DLL"
DEFAULT_MANIFEST = (
    ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" / "manifest.json"
)
DEFAULT_DONOR = (
    ROOT / "V8_2_LOOSE" / "mods" / "enhanced_textures_2x" / "font_work" /
    "game_fnt_manual_donor" / "game_fnt_manual_aligned_4x.tga"
)


def load_module(path: Path, name: str):
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


def game_source(source: object) -> bool:
    text = str(source)
    return (
        text.startswith("GAME.FNT glyph ") or
        text.startswith("GAME.FNT live loading glyph ")
    )


def strip_game_entries(manifest: dict[str, object]) -> set[str]:
    sources = manifest.setdefault("sources", {})
    stale_keys = {
        str(key)
        for key, source_list in sources.items()
        if any(game_source(source) for source in source_list)
    }
    manifest["entries"] = [
        entry for entry in manifest.get("entries", [])
        if str(entry.get("key", "")) not in stale_keys
    ]
    for key in stale_keys:
        sources.pop(key, None)
    manifest["generator"] = re.sub(
        r"; GAME\.FNT [^;]+ font atlas",
        "",
        str(manifest.get("generator", "")),
    )
    return stale_keys


def build_live_textmap(textmap, sheet, tips, trace_dirs: list[Path]):
    uses_by_log = textmap.collect_uses(trace_dirs)
    key_chars: dict[str, str] = {}
    key_uses: dict[str, object] = {}
    unresolved: list[str] = []
    for log, uses in uses_by_log.items():
        rows = textmap.row_uses(uses)
        text_rows = textmap.best_text_rows(sheet, tips, rows)
        if text_rows is None:
            unresolved.append(str(log))
            continue
        for row_text, row in zip(text_rows, rows):
            for char, use in zip(textmap.nonspace(row_text), row):
                previous = key_chars.get(use.key)
                if previous is not None and previous != char:
                    raise ValueError(
                        f"runtime key {use.key} maps to both {previous!r} and {char!r}")
                key_chars[use.key] = char
                key_uses[use.key] = use
        prompt = [
            use for use in sorted(uses, key=lambda item: (item.y, item.x))
            if use.tpage == "0x0A5" and use.clut == "0x7800" and
            use.height in (15, 18) and use.width > 0 and
            use.key != "0000000000000000"
        ]
        prompt_chars = "PressXtostart..."
        if prompt:
            if len(prompt) != len(prompt_chars):
                raise ValueError(
                    f"{log} has {len(prompt)} prompt glyphs, "
                    f"expected {len(prompt_chars)}")
            for char, use in zip(prompt_chars, prompt):
                previous = key_chars.get(use.key)
                if previous is not None and previous != char:
                    raise ValueError(
                        f"prompt key {use.key} maps to both "
                        f"{previous!r} and {char!r}")
                key_chars[use.key] = char
                key_uses[use.key] = use
    if unresolved:
        raise ValueError("unresolved loading logs:\n" + "\n".join(unresolved))
    return key_chars, key_uses, len(uses_by_log)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--donor", type=Path, default=DEFAULT_DONOR)
    parser.add_argument("--fnt", type=Path, default=DEFAULT_FNT)
    parser.add_argument("--load-dll", type=Path, default=DEFAULT_LOAD)
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--trace-dir", type=Path, action="append", required=True)
    parser.add_argument("--scale", type=int, default=4)
    parser.add_argument(
        "--source-variants",
        action="store_true",
        help="also deploy broad source-sheet hash variants (diagnostic only)",
    )
    parser.add_argument(
        "--image-name",
        default="game_fnt_manual_aligned_4x.dds",
    )
    args = parser.parse_args()

    fnt = load_module(FNT_TOOL, "build_fnt_font_proof")
    pack = load_module(PACK_TOOL, "apply_game_fnt_font_pack")
    textmap = load_module(TEXTMAP_TOOL, "apply_loading_game_font_textmap")
    pack.fnt = fnt
    sheet = fnt.decode_fnt(args.fnt)
    donor = Image.open(args.donor).convert("RGBA")
    expected_size = (sheet.atlas.width * args.scale, sheet.atlas.height * args.scale)
    if donor.size != expected_size:
        raise ValueError(f"{args.donor} is {donor.size}, expected {expected_size}")

    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    old_images = {
        str(entry.get("image", ""))
        for entry in manifest.get("entries", [])
        if any(
            game_source(source)
            for source in manifest.get("sources", {}).get(str(entry.get("key", "")), [])
        )
    }
    strip_game_entries(manifest)

    image_relative = f"images/ui/{args.image_name}"
    generated = (
        list(pack.crop_hash_entries(
            sheet,
            args.scale,
            2,
            args.image_name,
            donor,
        ))
        if args.source_variants else []
    )
    glyphs_by_code = {glyph.code: glyph for glyph in sheet.glyphs}
    for entry, source in generated:
        code_match = re.search(r"glyph 0x([0-9A-Fa-f]{2})", source)
        if code_match is None:
            raise ValueError(f"cannot recover glyph code from {source}")
        glyph = glyphs_by_code[int(code_match.group(1), 16)]
        entry.update({
            "x": glyph.x * args.scale,
            "y": glyph.y * args.scale,
            "width": glyph.width * args.scale,
            "height": sheet.max_height * args.scale,
        })
    entries = [entry for entry, _source in generated]
    sources = manifest.setdefault("sources", {})
    for entry, source in generated:
        sources[entry["key"]] = [source + " manual-donor measured-cell"]

    tips = textmap.extract_tips(args.load_dll)
    key_chars, key_uses, logs_read = build_live_textmap(
        textmap, sheet, tips, args.trace_dir)
    glyphs = {glyph.char: glyph for glyph in sheet.glyphs}
    existing_keys = {str(entry.get("key", "")) for entry in entries}
    live_added = 0
    for key, char in sorted(key_chars.items()):
        if key == "0000000000000000":
            continue
        glyph = glyphs.get(char)
        if glyph is None:
            raise ValueError(f"GAME.FNT has no glyph record for {char!r}")
        use = key_uses[key]
        if key in existing_keys:
            entries = [entry for entry in entries if str(entry.get("key", "")) != key]
        entries.append({
            "key": key,
            "image": image_relative,
            "x": glyph.x * args.scale,
            "y": glyph.y * args.scale,
            "width": glyph.width * args.scale,
            "height": sheet.max_height * args.scale,
        })
        sources[key] = [
            f"GAME.FNT live loading glyph textmap char={char!r} "
            f"uv={use.uv} size={use.width}x{use.height} "
            f"tpage={use.tpage} clut={use.clut} "
            "manual-donor measured-cell"
        ]
        existing_keys.add(key)
        live_added += 1

    replacement_keys = {str(entry["key"]) for entry in entries}
    manifest["entries"] = [
        entry for entry in manifest.get("entries", [])
        if str(entry.get("key", "")) not in replacement_keys
    ]
    manifest["entries"].extend(entries)
    suffix = "; GAME.FNT measured manual donor canonical 4x font atlas"
    manifest["generator"] = str(manifest.get("generator", "")) + suffix

    atlas_path = args.manifest.parent / image_relative
    atlas_path.parent.mkdir(parents=True, exist_ok=True)
    donor.save(atlas_path)
    args.manifest.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    referenced = {
        str(entry.get("image", "")) for entry in manifest.get("entries", [])
    }
    for old_image in old_images:
        if old_image in referenced or old_image == image_relative:
            continue
        old_path = args.manifest.parent / old_image
        if old_path.exists():
            old_path.unlink()

    print(f"font_atlas={atlas_path}")
    print(f"source_entries={len(generated)}")
    print(f"runtime_keys_mapped={len(key_chars)}")
    print(f"runtime_entries_added={live_added}")
    print(f"trace_logs_read={logs_read}")
    print(f"total_game_entries={len(entries)}")


if __name__ == "__main__":
    main()
