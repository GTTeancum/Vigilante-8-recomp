"""Build the preferred Rockwell GAME.FNT and deploy it through file provenance."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil

import compile_ttf_fnt as compiler

ROOT = Path(__file__).resolve().parents[2]
LOOSE = ROOT / "V8_2_LOOSE"
OUT = ROOT / "mods" / "rockwell_regular_font"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--deploy", action="store_true")
    args = parser.parse_args()

    source = LOOSE / "SHARED/GAME.FNT"
    font = Path("C:/Windows/Fonts/ROCK.TTF")
    point_size = 14
    scale = 4
    reader = compiler.load_fnt_reader()
    template = reader.decode_fnt(source)
    rendered, baseline = compiler.build_glyphs(template, font, point_size, scale)
    glyphs, height = compiler.pack_glyphs(
        rendered, template.first_char, 221, template.max_height, scale, 242, 0
    )
    native, hd = compiler.compose_atlases(
        glyphs, 242, height, template.max_height, scale
    )

    fnt_path = OUT / "files/SHARED/GAME.FNT"
    atlas_path = OUT / "shared_game_fnt_4x.dds"
    fnt_path.parent.mkdir(parents=True, exist_ok=True)
    fnt_path.write_bytes(
        compiler.encode_fnt(
            source, template.first_char, glyphs, template.max_height, native
        )
    )
    # Preserve the exact crisp-edged atlas the user preferred before the
    # Extra Bold experiment. Runtime output scaling still antialiases edges.
    rgba = compiler.alpha_rgba(compiler.binary_alpha(hd, 128))
    rgba.save(atlas_path)

    decoded = reader.decode_fnt(fnt_path)
    assert decoded.atlas.size == native.size
    for original in template.glyphs:
        if original.width:
            assert any(g.code == original.code and g.width for g in decoded.glyphs)

    entry = {
        "path": "SHARED/GAME.FNT",
        "image": "images/fonts/shared_game_fnt_4x.dds",
        "sourceWidth": native.width,
        "sourceHeight": native.height,
    }
    (OUT / "fontFiles.json").write_text(
        json.dumps([entry], indent=2) + "\n", encoding="utf-8"
    )
    (OUT / "build.json").write_text(
        json.dumps(
            {
                "font": str(font),
                "pointSize": point_size,
                "scale": scale,
                "baseline": baseline,
                "alphaThreshold": 128,
                "fontSha256": hashlib.sha256(font.read_bytes()).hexdigest(),
                "glyphs": len(decoded.glyphs),
                "routing": "SHARED/GAME.FNT whole-file provenance",
                "cyanFontUnchanged": True,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )

    if args.deploy:
        pack = LOOSE / "mods/enhanced_textures_2x"
        mod = LOOSE / "mods/ttf_game_font"
        manifest_path = pack / "manifest.json"
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        shutil.copy2(fnt_path, mod / "files/SHARED/GAME.FNT")
        shutil.copy2(atlas_path, pack / entry["image"])
        rgba.save(mod / "proof/compiled_game_fnt_atlas_4x.tga")
        manifest["fontFiles"] = [
            item
            for item in manifest.get("fontFiles", [])
            if item["path"].upper() != "SHARED/GAME.FNT"
        ] + [entry]
        manifest_path.write_text(
            json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
        )
        metadata_path = mod / "mod.json"
        metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
        metadata["description"] = (
            "GAME.FNT compiled from ROCK.TTF; filename-routed across all GAME.FNT users"
        )
        metadata_path.write_text(
            json.dumps(metadata, indent=2) + "\n", encoding="utf-8"
        )

    print(
        f"GAME.FNT={native.width}x{native.height}; "
        f"HD={rgba.width}x{rgba.height}; glyphs={len(decoded.glyphs)}; "
        f"deployed={args.deploy}"
    )


if __name__ == "__main__":
    main()
