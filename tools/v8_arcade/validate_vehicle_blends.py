#!/usr/bin/env python3
"""Validate extracted vehicle Blender files and write a JSON report."""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

import bpy


EXPECTED_UV_OFFSETS = {
    20: 12,
    24: 12,
    32: 24,
    44: 36,
    48: 24,
    60: 36,
    92: 36,
}


def material_is_wired(material: bpy.types.Material) -> bool:
    if not material.use_nodes or material.node_tree is None:
        return False
    principled = material.node_tree.nodes.get("Principled BSDF")
    if principled is None:
        return False
    return bool(principled.inputs["Base Color"].links)


def material_has_fancy_rendering(material: bpy.types.Material) -> bool:
    if not material.use_nodes or material.node_tree is None:
        return False
    prohibited = {"NORMAL_MAP", "BUMP"}
    if any(node.type in prohibited for node in material.node_tree.nodes):
        return True
    principled = material.node_tree.nodes.get("Principled BSDF")
    if principled is None:
        return False
    if principled.inputs["Alpha"].links:
        return True
    if abs(float(principled.inputs["Alpha"].default_value) - 1.0) > 0.001:
        return True
    base_links = principled.inputs["Base Color"].links
    if base_links and any(link.from_node.type != "TEX_IMAGE" for link in base_links):
        return True
    if float(principled.inputs["Roughness"].default_value) < 0.999:
        return True
    for name in ("Specular IOR Level", "Coat Weight"):
        if name in principled.inputs and float(principled.inputs[name].default_value) > 0.001:
            return True
    return False


def validate_mesh(mesh: bpy.types.Mesh) -> dict[str, object]:
    layouts = [str(value) for value in mesh.get("vertex_layouts", [])]
    expects_uv = False
    bad_layouts = []
    for layout in layouts:
        prefix = layout.split(" ", 1)[0]
        stride = int(prefix.removeprefix("stride="))
        expected_offset = EXPECTED_UV_OFFSETS.get(stride)
        if expected_offset is None or "TEXCOORD0@" not in layout:
            continue
        expects_uv = True
        token = f"TEXCOORD0@{expected_offset}:"
        if token not in layout:
            bad_layouts.append(
                {
                    "layout": layout,
                    "expected": token,
                }
            )

    uv_layer = mesh.uv_layers.get("TEXCOORD_0")
    finite = True
    uv_bounds = None
    plausible_fraction = None
    if uv_layer is not None and uv_layer.data:
        values = [
            component
            for loop in uv_layer.data
            for component in (float(loop.uv.x), float(loop.uv.y))
        ]
        finite = all(math.isfinite(value) for value in values)
        us = values[0::2]
        vs = values[1::2]
        uv_bounds = [min(us), min(vs), max(us), max(vs)]
        plausible_fraction = sum(
            -0.25 <= u <= 1.25 and -0.25 <= v <= 1.25
            for u, v in zip(us, vs)
        ) / len(us)
    return {
        "name": mesh.name,
        "vertex_count": len(mesh.vertices),
        "triangle_count": len(mesh.polygons),
        "vertex_layouts": layouts,
        "expects_uv": expects_uv,
        "has_uv": uv_layer is not None,
        "finite_uvs": finite,
        "uv_bounds": uv_bounds,
        "plausible_uv_fraction": plausible_fraction,
        "bad_layouts": bad_layouts,
        "valid": (
            len(mesh.vertices) > 0
            and len(mesh.polygons) > 0
            and (not expects_uv or uv_layer is not None)
            and finite
            and not bad_layouts
        ),
    }


def validate(path: Path) -> dict[str, object]:
    bpy.ops.wm.open_mainfile(filepath=str(path.resolve()), load_ui=False)
    scene = bpy.context.scene
    version_kind = str(scene.get("version_kind", ""))
    meshes = list(bpy.data.meshes)
    images = list(bpy.data.images)
    materials = list(bpy.data.materials)
    mesh_records = [validate_mesh(mesh) for mesh in meshes]
    missing_images = [
        image.name
        for image in images
        if image.source == "FILE" and not image.packed_file and not Path(image.filepath).exists()
    ]
    unpacked_images = [
        image.name
        for image in images
        if image.source == "FILE" and not image.packed_file
    ]
    bad_meshes = [
        record["name"] for record in mesh_records if not record["valid"]
    ]
    assembly_name = (
        "AUXILIARY_UNRESOLVED_MODELS"
        if version_kind == "auxiliary_unresolved_models"
        else "VEHICLE_ASSEMBLY"
    )
    assembly = bpy.data.collections.get(assembly_name)
    assembly_mesh_objects = (
        [obj for obj in assembly.objects if obj.type == "MESH"]
        if assembly is not None
        else []
    )
    unnamed_parts = [
        obj.name
        for obj in assembly_mesh_objects
        if not obj.get("part_name")
        or not obj.get("part_name_confidence")
    ]
    lod1 = bpy.data.collections.get("XBOX360_DAMAGE_ASSEMBLY")
    lod1_mesh_objects = (
        [obj for obj in lod1.objects if obj.type == "MESH"]
        if lod1 is not None
        else []
    )
    expected_lod1_count = int(scene.get("damage_model_instance_count", 0))
    unnamed_lod1_parts = [
        obj.name
        for obj in lod1_mesh_objects
        if not obj.get("part_name")
        or not obj.get("part_name_confidence")
        or not bool(obj.get("damage_state_model", False))
    ]
    lod1_valid = (
        expected_lod1_count == 0
        or (
            lod1 is not None
            and len(lod1_mesh_objects) == expected_lod1_count
            and lod1.hide_viewport
            and lod1.hide_render
            and not unnamed_lod1_parts
        )
    )
    sampler_materials = [
        material
        for material in materials
        if material.get("source_sampler_bindings")
    ]
    diffuse_materials = [
        material
        for material in sampler_materials
        if any(
            "diffuse" in str(binding).lower()
            or "albedo" in str(binding).lower()
            or "base" in str(binding).lower()
            for binding in material["source_sampler_bindings"]
        )
    ]
    unwired_diffuse_materials = [
        material.name
        for material in diffuse_materials
        if not material_is_wired(material)
    ]
    missing_shader_metadata = [
        material.name
        for material in materials
        if (
            "source_technique_offset" not in material
            or "source_parameter_offset" not in material
            or "decoded_material_parameters" not in material
        )
    ]
    fancy_materials = [
        material.name
        for material in materials
        if material_has_fancy_rendering(material)
    ]
    valid = (
        bool(meshes)
        and bool(version_kind)
        and assembly is not None
        and bool(assembly_mesh_objects)
        and not bad_meshes
        and not missing_images
        and not unpacked_images
        and not unnamed_parts
        and lod1_valid
        and not unwired_diffuse_materials
        and not missing_shader_metadata
        and not fancy_materials
    )
    return {
        "file": str(path.resolve()),
        "size": path.stat().st_size,
        "version_kind": version_kind,
        "assembly_collection": assembly_name,
        "mesh_count": len(meshes),
        "assembly_mesh_object_count": len(assembly_mesh_objects),
        "object_count": len(bpy.data.objects),
        "material_count": len(materials),
        "image_count": len(images),
        "packed_image_count": sum(bool(image.packed_file) for image in images),
        "meshes": mesh_records,
        "bad_meshes": bad_meshes,
        "missing_images": missing_images,
        "unpacked_images": unpacked_images,
        "unnamed_parts": unnamed_parts,
        "damage_expected_model_instance_count": expected_lod1_count,
        "damage_mesh_object_count": len(lod1_mesh_objects),
        "damage_hidden_by_default": (
            bool(lod1.hide_viewport and lod1.hide_render)
            if lod1 is not None
            else None
        ),
        "unnamed_damage_parts": unnamed_lod1_parts,
        "damage_valid": lod1_valid,
        "diffuse_material_count": len(diffuse_materials),
        "unwired_diffuse_materials": unwired_diffuse_materials,
        "missing_shader_metadata": missing_shader_metadata,
        "fancy_materials": fancy_materials,
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
            f"meshes={record['mesh_count']} images="
            f"{record['packed_image_count']}/{record['image_count']} packed"
        )
    return 0 if all(record["valid"] for record in records) else 1


if __name__ == "__main__":
    raise SystemExit(main())
