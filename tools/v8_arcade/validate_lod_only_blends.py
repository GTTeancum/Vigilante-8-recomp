#!/usr/bin/env python3
"""Validate isolated V8 Arcade highest-LOD PS1 Blender files."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import bpy

sys.path.insert(0, str(Path(__file__).resolve().parent))
from validate_vehicle_blends import (
    material_has_fancy_rendering,
    validate_mesh,
)


EXPECTED_COLLECTIONS = {
    "ps1_highest_lod_only": "PS1_HIGHEST_LOD",
}


def validate(path: Path) -> dict[str, object]:
    bpy.ops.wm.open_mainfile(filepath=str(path.resolve()), load_ui=False)
    scene = bpy.context.scene
    version_kind = str(scene.get("version_kind", ""))
    expected_collection_name = EXPECTED_COLLECTIONS.get(version_kind)
    collection = (
        bpy.data.collections.get(expected_collection_name)
        if expected_collection_name
        else None
    )
    objects = list(bpy.data.objects)
    mesh_objects = [obj for obj in objects if obj.type == "MESH"]
    collection_mesh_objects = (
        [obj for obj in collection.objects if obj.type == "MESH"]
        if collection is not None
        else []
    )
    mesh_records = [validate_mesh(mesh) for mesh in bpy.data.meshes]
    bad_meshes = [
        record["name"] for record in mesh_records if not record["valid"]
    ]
    unpacked_images = [
        image.name
        for image in bpy.data.images
        if image.source == "FILE" and not image.packed_file
    ]
    fancy_materials = [
        material.name
        for material in bpy.data.materials
        if material_has_fancy_rendering(material)
    ]
    unnamed_parts = [
        obj.name
        for obj in mesh_objects
        if not obj.get("part_name")
        or not obj.get("part_name_confidence")
        or "source_model_offset" not in obj
    ]
    expected_instances = int(scene.get("model_instance_count", -1))
    triangle_count = sum(
        len(obj.data.polygons) for obj in collection_mesh_objects
    )
    expected_triangles = int(scene.get("triangle_count", -1))
    unexpected_collections = [
        item.name
        for item in scene.collection.children
        if item.name != expected_collection_name
    ]
    valid = (
        expected_collection_name is not None
        and collection is not None
        and expected_instances > 0
        and len(mesh_objects) == expected_instances
        and len(collection_mesh_objects) == expected_instances
        and triangle_count == expected_triangles
        and not unexpected_collections
        and not bad_meshes
        and not unpacked_images
        and not fancy_materials
        and not unnamed_parts
    )
    return {
        "file": str(path.resolve()),
        "version_kind": version_kind,
        "expected_collection": expected_collection_name,
        "mesh_object_count": len(mesh_objects),
        "expected_model_instance_count": expected_instances,
        "triangle_count": triangle_count,
        "expected_triangle_count": expected_triangles,
        "unexpected_collections": unexpected_collections,
        "bad_meshes": bad_meshes,
        "unpacked_images": unpacked_images,
        "fancy_materials": fancy_materials,
        "unnamed_parts": unnamed_parts,
        "valid": valid,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path)
    parser.add_argument("--out", required=True, type=Path)
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None
    args = parser.parse_args(argv)
    records = [validate(path) for path in args.inputs]
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(records, indent=2) + "\n", encoding="utf-8")
    for record in records:
        print(
            f"{Path(record['file']).name}: valid={record['valid']} "
            f"objects={record['mesh_object_count']} "
            f"triangles={record['triangle_count']}"
        )
    return 0 if all(record["valid"] for record in records) else 1


if __name__ == "__main__":
    raise SystemExit(main())
