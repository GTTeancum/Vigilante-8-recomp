#!/usr/bin/env python3
"""Audit Dreamland XOBF texture conversion against retail V8:2 ports."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys


REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "tools" / "blender_addons"))

from vigilante8_vehicle_tools import iff, registry, xobf


def _decode(form: iff.IffChunk, dialect: str):
    form = iff.IffChunk(
        tag=b"FORM",
        form_type=b"XOBF",
        children=[child for child in form.children if child.tag in {b"BIN ", b"ANM "}],
    )
    decode_face = registry._decode_face

    def decode_terrain_face(packet, packet_index, native_group, model):
        texture_count = model.texture_count
        model.texture_count = 0x4000
        try:
            return decode_face(packet, packet_index, native_group, model)
        finally:
            model.texture_count = texture_count

    registry._decode_face = decode_terrain_face
    try:
        return registry._decode_bank(form, dialect)
    finally:
        registry._decode_face = decode_face


def _texture_record(texture) -> dict[str, object]:
    palette_bytes = b"".join(
        int(color).to_bytes(2, "little") for color in texture.palette_bgr555
    )
    direct_bytes = b"".join(
        int(color).to_bytes(2, "little")
        for color in texture.direct_pixels_bgr555
    )
    transparent_indices = [
        index
        for index, color in enumerate(texture.palette_bgr555)
        if color == 0
    ]
    return {
        "name": texture.name,
        "width": texture.width,
        "height": texture.height,
        "depth": texture.depth,
        "compressed": texture.compressed,
        "palette_origin": texture.palette_origin,
        "image_origin": texture.image_origin,
        "palette_count": len(texture.palette_bgr555),
        "palette_sha256": hashlib.sha256(palette_bytes).hexdigest(),
        "indices_sha256": hashlib.sha256(texture.indices).hexdigest(),
        "direct_sha256": hashlib.sha256(direct_bytes).hexdigest(),
        "transparent_palette_indices": transparent_indices,
        "transparent_texels": sum(
            1 for index in texture.indices if index in transparent_indices
        ),
        "opaque_texels": sum(
            1 for index in texture.indices if index not in transparent_indices
        ),
    }


def _bank_records(path: Path, dialect: str) -> list[dict[str, object]]:
    forms = list(iff.parse(path.read_bytes()).forms(b"XOBF"))
    records = []
    for bank_index, form in enumerate(forms):
        bank = _decode(form, dialect)
        records.append(
            {
                "bank": bank_index,
                "groups": len(bank.groups),
                "slots": len(bank.slots),
                "collisions": len(bank.collisions),
                "animations": len(bank.animations),
                "textures": [_texture_record(texture) for texture in bank.textures],
            }
        )
    return records


def _semantic_texture(texture) -> tuple[object, ...]:
    return (
        texture.width,
        texture.height,
        texture.depth,
        getattr(texture, "palette_bgr555", getattr(texture, "palette", ())),
        texture.indices,
        texture.direct_pixels_bgr555,
        texture.palette_origin,
        texture.image_origin,
    )


def _native_textures(form: iff.IffChunk, dialect: str):
    chunks = {child.tag: child.payload for child in form.children}
    return tuple(xobf.Model(chunks[b"BIN "], dialect=dialect).textures())


def _conversion_parity(source: Path, target: Path) -> list[dict[str, object]]:
    source_forms = list(iff.parse(source.read_bytes()).forms(b"XOBF"))
    target_forms = list(iff.parse(target.read_bytes()).forms(b"XOBF"))
    records = []
    for bank_index in range(min(len(source_forms), len(target_forms))):
        source_textures = _native_textures(source_forms[bank_index], "V8")
        target_textures = _native_textures(target_forms[bank_index], "V8_2")
        count = min(len(source_textures), len(target_textures))
        mismatches = [
            index
            for index in range(count)
            if _semantic_texture(source_textures[index])
            != _semantic_texture(target_textures[index])
        ]
        records.append(
            {
                "bank": bank_index,
                "source_textures": len(source_textures),
                "target_textures": len(target_textures),
                "semantic_texture_mismatches": mismatches,
            }
        )
    return records


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--output",
        type=Path,
        default=REPO / "artifacts" / "v82-open016-xobf-texture-audit.json",
    )
    args = parser.parse_args()
    dreamland_source = REPO / "PS1 game" / "TERRAIN" / "DREAMLND.EXP"
    dreamland_target = (
        REPO
        / "V8_2_LOOSE"
        / "mods"
        / "v82_n64_super_dreamland"
        / "files"
        / "LEVELS"
        / "N64"
        / "DREAMLND.EXP"
    )
    retail_pairs = []
    for name in ("CASNOCTY.EXP", "OILFIELD.EXP", "SANDFACT.EXP", "VALLYFRM.EXP"):
        source = REPO / "PS1 game" / "TERRAIN" / name
        target = REPO / "V8_2_LOOSE" / "LEVELS" / "V8" / name
        retail_pairs.append(
            {"arena": name, "conversion_parity": _conversion_parity(source, target)}
        )
    report = {
        "dreamland_source": _bank_records(dreamland_source, "V8"),
        "dreamland_target": _bank_records(dreamland_target, "V8_2"),
        "dreamland_conversion_parity": _conversion_parity(
            dreamland_source, dreamland_target
        ),
        "retail_pairs": retail_pairs,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="ascii")
    print(args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
