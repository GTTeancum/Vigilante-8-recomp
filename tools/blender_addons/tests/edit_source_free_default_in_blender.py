"""Make and export a visible edit through the strict Blender authoring scene."""

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
from vigilante8_vehicle_tools import authored_scene, project, registry


def arguments() -> argparse.Namespace:
    values = sys.argv
    values = values[values.index("--") + 1 :] if "--" in values else []
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("blend", type=Path)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--width-scale", type=float, default=1.18)
    return parser.parse_args(values)


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def main() -> None:
    args = arguments()
    source = args.source.resolve()
    output = args.output.resolve()
    archive = (source / "CUSTOM.EXP").read_bytes()
    roster = (source / "VEHICLES.V8R").read_bytes()
    original = registry.decompile_package(archive, roster)[0]

    bpy.ops.wm.read_factory_settings(use_empty=True)
    vigilante8_vehicle_tools.register()
    collection = authored_scene.project_to_scene(bpy.context, original)
    body_bank = next(
        child
        for child in collection.children
        if child.get("v8_bank") == "body"
    )
    body_groups = [
        obj
        for obj in body_bank.all_objects
        if obj.type == "MESH"
        and obj.get("v8_role") in {"authored_slot", "authored_render_group"}
        and int(obj["v8_group_index"]) == 0
    ]
    if not body_groups:
        raise AssertionError("Blender scene has no native model 0")
    before = tuple(
        tuple(vertex.co) for vertex in body_groups[0].data.vertices
    )
    for body_group in body_groups:
        for vertex in body_group.data.vertices:
            vertex.co.x *= args.width_scale
        body_group.data.update()
    after = tuple(
        tuple(vertex.co) for vertex in body_groups[0].data.vertices
    )
    if before == after:
        raise AssertionError("Blender body edit changed no vertices")
    for body_group in body_groups:
        body_group["roundtrip_edit"] = (
            f"native model 0 width multiplied by {args.width_scale:.3f}"
        )

    edited_images = 0
    for image in bpy.data.images:
        if (
            image.get("v8_role") != "authored_texture"
            or image.get("v8_vehicle_collection") != collection.name
            or image.get("v8_bank") != "body"
        ):
            continue
        pixels = list(image.pixels)
        for offset in range(0, len(pixels), 4):
            red, green, blue, alpha = pixels[offset : offset + 4]
            if alpha < 0.5:
                continue
            luminance = (red + green + blue) / 3.0
            pixels[offset] = min(1.0, luminance * 0.25)
            pixels[offset + 1] = min(1.0, 0.30 + luminance)
            pixels[offset + 2] = min(1.0, luminance * 0.20)
        image.pixels.foreach_set(pixels)
        image.update()
        edited_images += 1
    if edited_images == 0:
        raise AssertionError("strict import exposed no body textures")

    rebuilt_project = authored_scene.scene_to_project(collection)
    rebuilt = registry.compile_package((rebuilt_project,))
    reparsed = registry.decompile_package(rebuilt.archive, rebuilt.registry)[0]
    if project.to_dict(reparsed) != project.to_dict(rebuilt_project):
        raise AssertionError("Blender export did not semantically round-trip")
    if rebuilt.archive == archive:
        raise AssertionError("visible Blender edit produced an unchanged archive")

    output.mkdir(parents=True, exist_ok=True)
    (output / "CUSTOM.EXP").write_bytes(rebuilt.archive)
    (output / "VEHICLES.V8R").write_bytes(rebuilt.registry)
    (output / "project.json").write_text(
        json.dumps(project.to_dict(reparsed), indent=2) + "\n",
        encoding="utf-8",
    )
    args.blend.resolve().parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(args.blend.resolve()))

    result = {
        "game": rebuilt.game,
        "source": str(source),
        "output": str(output),
        "blend": str(args.blend.resolve()),
        "edited_group": 0,
        "edited_vertices": len(body_group.data.vertices),
        "edited_body_textures": edited_images,
        "edit": body_group["roundtrip_edit"],
        "source_archive_sha256": sha256(archive),
        "export_archive_sha256": sha256(rebuilt.archive),
        "source_registry_sha256": sha256(roster),
        "export_registry_sha256": sha256(rebuilt.registry),
        "contains_embedded_source": False,
        "contains_opaque_packet_blob": False,
    }
    args.manifest.resolve().write_text(
        json.dumps(result, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
