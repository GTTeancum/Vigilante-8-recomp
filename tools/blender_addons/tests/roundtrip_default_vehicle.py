"""Blender-driven retail gameplay vehicle replacement proof.

This harness imports one real COMMON.EXP entry into Blender, applies a small
visible mesh edit using Blender's mesh API, exports the archive through the
native vehicle exporter, and saves the editable .blend used for the export.
It is a verification harness, not part of the production add-on package.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys

import bpy


ROOT = Path(__file__).resolve().parents[3]
ADDONS = ROOT / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

import vigilante8_vehicle_tools
from vigilante8_vehicle_tools import blender_ui


def arguments() -> argparse.Namespace:
    values = sys.argv
    values = values[values.index("--") + 1 :] if "--" in values else []
    parser = argparse.ArgumentParser()
    parser.add_argument("game", choices=("V8", "V8_2"))
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("blend", type=Path)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--entry", type=int, default=0)
    parser.add_argument("--width-scale", type=float, default=1.18)
    return parser.parse_args(values)


def main() -> None:
    args = arguments()
    source = args.source.resolve()
    output = args.output.resolve()
    blend = args.blend.resolve()
    manifest = args.manifest.resolve()

    bpy.ops.wm.read_factory_settings(use_empty=True)
    vigilante8_vehicle_tools.register()
    root, count = blender_ui.import_archive_into_scene(
        bpy.context,
        str(source),
        args.entry,
        False,
        True,
    )
    if count != 1:
        raise AssertionError(f"expected one imported vehicle, received {count}")
    collection = next(
        child
        for child in root.children
        if child.get("v8_role") == "vehicle_entry"
    )
    group = next(
        obj
        for obj in collection.all_objects
        if obj.get("v8_role") == "render_group"
        and int(obj["v8_group_index"]) == 0
    )
    before = tuple(tuple(vertex.co) for vertex in group.data.vertices)
    for vertex in group.data.vertices:
        vertex.co.x *= args.width_scale
    after = tuple(tuple(vertex.co) for vertex in group.data.vertices)
    if before == after:
        raise AssertionError("Blender mesh edit changed no vertices")
    group.data.update()
    group["roundtrip_edit"] = (
        f"default gameplay body width x{args.width_scale:.3f}; green texture reskin"
    )

    edited_images = 0
    for image in bpy.data.images:
        if image.get("v8_vehicle_collection") != collection.name:
            continue
        pixels = list(image.pixels)
        for offset in range(0, len(pixels), 4):
            red, green, blue, alpha = pixels[offset : offset + 4]
            if alpha < 0.5:
                continue
            luminance = (red + green + blue) / 3.0
            pixels[offset] = min(1.0, luminance * 0.30)
            pixels[offset + 1] = min(1.0, 0.28 + luminance * 0.95)
            pixels[offset + 2] = min(1.0, luminance * 0.22)
        image.pixels.foreach_set(pixels)
        image.update()
        edited_images += 1
    if edited_images == 0:
        raise AssertionError("retail vehicle import produced no editable textures")

    output.parent.mkdir(parents=True, exist_ok=True)
    blender_ui.export_collection_to_archive(collection, str(output))
    blend.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(blend))

    source_data = source.read_bytes()
    output_data = output.read_bytes()
    if source_data == output_data:
        raise AssertionError("export is byte-identical after the Blender edit")
    result = {
        "game": args.game,
        "entry": args.entry,
        "source": str(source),
        "output": str(output),
        "blend": str(blend),
        "edit": group["roundtrip_edit"],
        "edited_group": 0,
        "edited_vertices": len(group.data.vertices),
        "edited_textures": edited_images,
        "source_bytes": len(source_data),
        "output_bytes": len(output_data),
        "source_sha256": hashlib.sha256(source_data).hexdigest().upper(),
        "output_sha256": hashlib.sha256(output_data).hexdigest().upper(),
    }
    manifest.parent.mkdir(parents=True, exist_ok=True)
    manifest.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
