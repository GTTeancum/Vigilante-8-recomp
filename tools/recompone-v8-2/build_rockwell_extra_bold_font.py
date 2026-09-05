"""Compile approved GAME.FNT and a filename-routed HD atlas, never VRAM keys."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil

import compile_ttf_fnt as compiler

ROOT = Path(__file__).resolve().parents[2]
LOOSE = ROOT / "V8_2_LOOSE"
OUT = ROOT / "mods" / "rockwell_extra_bold_font"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--deploy", action="store_true")
    args = parser.parse_args()
    source = LOOSE / "SHARED/GAME.FNT"
    font = Path("C:/Windows/Fonts/ROCKEB.TTF")
    reader = compiler.load_fnt_reader()
    template = reader.decode_fnt(source)
    rendered, baseline = compiler.build_glyphs(template, font, 11, 4)
    glyphs, height = compiler.pack_glyphs(rendered, template.first_char, 221,
                                         template.max_height, 4, 242, 0)
    native, hd = compiler.compose_atlases(glyphs, 242, height, template.max_height, 4)
    fnt_path = OUT / "files/SHARED/GAME.FNT"
    atlas_path = OUT / "shared_game_fnt_4x.dds"
    fnt_path.parent.mkdir(parents=True, exist_ok=True)
    fnt_path.write_bytes(compiler.encode_fnt(source, template.first_char, glyphs,
                                            template.max_height, native))
    # Retain TTF coverage in the file atlas. The native FNT remains indexed;
    # Enhanced uses this grayscale alpha directly for smooth letter edges.
    rgba = compiler.alpha_rgba(hd)
    rgba.save(atlas_path)
    decoded = reader.decode_fnt(fnt_path)
    assert decoded.atlas.size == native.size
    for original in template.glyphs:
        if original.width:
            assert any(g.code == original.code and g.width for g in decoded.glyphs)
    entry = {"path": "SHARED/GAME.FNT", "image": "images/fonts/shared_game_fnt_4x.dds",
             "sourceWidth": native.width, "sourceHeight": native.height}
    (OUT / "fontFiles.json").write_text(json.dumps([entry], indent=2) + "\n", encoding="utf-8")
    (OUT / "build.json").write_text(json.dumps({
        "font": str(font), "pointSize": 11, "scale": 4, "baseline": baseline,
        "fontSha256": hashlib.sha256(font.read_bytes()).hexdigest(),
        "glyphs": len(decoded.glyphs), "routing": "SHARED/GAME.FNT whole-file provenance",
        "cyanFontUnchanged": True,
    }, indent=2) + "\n", encoding="utf-8")
    if args.deploy:
        pack = LOOSE / "mods/enhanced_textures_2x"
        mod = LOOSE / "mods/ttf_game_font"
        manifest_path = pack / "manifest.json"
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        backup = ROOT / "artifacts/pause-style-20260904/font-backup"
        backup.mkdir(parents=True, exist_ok=True)
        targets = ((mod / "files/SHARED/GAME.FNT", "GAME.FNT"),
                   (pack / entry["image"], "shared_game_fnt_4x.dds"),
                   (manifest_path, "manifest.json"), (mod / "mod.json", "mod.json"),
                   (mod / "proof/compiled_game_fnt_atlas_4x.tga", "compiled_game_fnt_atlas_4x.tga"))
        for target, name in targets:
            if target.is_file() and not (backup / name).exists():
                shutil.copy2(target, backup / name)
        shutil.copy2(fnt_path, mod / "files/SHARED/GAME.FNT")
        shutil.copy2(atlas_path, pack / entry["image"])
        rgba.save(mod / "proof/compiled_game_fnt_atlas_4x.tga")
        manifest["fontFiles"] = [e for e in manifest.get("fontFiles", [])
                                 if e["path"].upper() != "SHARED/GAME.FNT"] + [entry]
        manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
        metadata = json.loads((mod / "mod.json").read_text(encoding="utf-8"))
        metadata["description"] = "GAME.FNT compiled from approved Rockwell Extra Bold (ROCKEB.TTF)"
        (mod / "mod.json").write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
    print(f"GAME.FNT={native.width}x{native.height}; HD={rgba.width}x{rgba.height}; glyphs={len(decoded.glyphs)}; deployed={args.deploy}")


if __name__ == "__main__":
    main()
