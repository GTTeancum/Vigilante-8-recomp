"""Build retail-data transformation previews on a conventional four-wheel car."""

from __future__ import annotations

from dataclasses import replace
import json
import math
from pathlib import Path
import sys

import bpy
from mathutils import Quaternion


ROOT = Path(__file__).resolve().parents[3]
ADDONS = ROOT / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))
if str(Path(__file__).resolve().parent) not in sys.path:
    sys.path.insert(0, str(Path(__file__).resolve().parent))

import vigilante8_vehicle_tools
from vigilante8_vehicle_tools import authored_scene, registry, stats
from build_source_free_default_replacements import bank, extract_roots, write


VEHICLE_INDEX = 1
PACKAGE = (
    ROOT
    / "artifacts"
    / "vehicle_plugin_proofs"
    / "v82_thunderbolt_preview_source"
)
OUTPUT = ROOT / "artifacts" / "vehicle_plugin_proofs"


def build_vehicle():
    source = (
        ROOT
        / "artifacts"
        / "dual_game_default_roundtrip"
        / "V82_COMMON_ORIGINAL.EXP"
    )
    stats_file = stats.StatsFile(
        (ROOT / "V8_2_LOOSE" / "SLUS_008.68").read_bytes(),
        "V8_2",
    )
    record = stats_file.record(VEHICLE_INDEX)
    values = record.values()
    values.pop("vehicle_type")
    values["rear_suspension_damping"] = int.from_bytes(
        record.raw[0x0C:0x0E], "little"
    )

    body, body_roots = extract_roots(
        bank(source, "V8_2", VEHICLE_INDEX), {0}
    )
    transform_source = bank(source, "V8_2", 18)
    native_modes = stats_file.transform_modes()
    required_roots = {
        values["wheel_kind_front"],
        values["wheel_kind_rear"],
        *(
            kind
            for mode in native_modes[1:]
            for kind in mode
        ),
    }
    transform, root_map = extract_roots(
        transform_source, required_roots
    )
    values["wheel_kind_front"] = root_map[
        values["wheel_kind_front"]
    ]
    values["wheel_kind_rear"] = root_map[
        values["wheel_kind_rear"]
    ]
    mapped_modes = tuple(
        tuple(
            0 if mode_index == 0 else root_map[kind]
            for kind in mode
        )
        for mode_index, mode in enumerate(native_modes)
    )

    base = authored_scene.new_project(
        "V8_2", "proof.v82.thunderbolt.transform_preview"
    )
    vehicle = replace(
        base,
        display_name="Thunderbolt — Conventional Four-Wheel Preview",
        groups=body.groups,
        slots=body.slots,
        collisions=body.collisions,
        textures=body.textures,
        animations=body.animations,
        stats=values,
        body_kind=body_roots[0],
        transformation_bank=transform,
        transform_modes=mapped_modes,
        powerups=stats_file.powerup_values(),
    )
    vehicle.validate()
    write(PACKAGE, vehicle)
    return vehicle


def configure_window() -> None:
    window = bpy.context.window_manager.windows[0]
    visible = [
        obj
        for obj in bpy.context.scene.objects
        if obj.type == "MESH" and not obj.hide_get()
    ]
    bpy.ops.object.select_all(action="DESELECT")
    for obj in visible:
        obj.select_set(True)
    if visible:
        bpy.context.view_layer.objects.active = visible[0]
    for area in window.screen.areas:
        if area.type == "VIEW_3D":
            area.spaces.active.shading.type = "MATERIAL"
            region_3d = area.spaces.active.region_3d
            region_3d.view_perspective = "PERSP"
            region = next(
                item for item in area.regions if item.type == "WINDOW"
            )
            with bpy.context.temp_override(
                window=window, area=area, region=region
            ):
                bpy.ops.view3d.view_selected(use_all_regions=False)
            region_3d.view_rotation = (
                Quaternion((0.0, 0.0, 1.0), math.pi)
                @ region_3d.view_rotation
            )
            region_3d.view_distance *= 0.86
        elif area.type == "PROPERTIES":
            area.spaces.active.context = "SCENE"
    for obj in visible:
        obj.select_set(False)


def main() -> None:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    vigilante8_vehicle_tools.register()
    vehicle = build_vehicle()
    collection = authored_scene.project_to_scene(bpy.context, vehicle)
    settings = bpy.context.scene.v8_vehicle_settings
    settings.vehicle_collection = collection.name
    configure_window()

    anchors = sorted(
        (
            obj
            for obj in collection.all_objects
            if (
                obj.get("v8_part_role") == "wheel_anchor"
                and obj.parent is not None
                and int(obj.get("v8_attachment_key", -1))
                in range(0x8000, 0x8004)
            )
        ),
        key=lambda obj: int(obj["v8_attachment_key"]),
    )
    native_positions = [
        authored_scene._blender_vector_to_native(
            anchor.location, authored_scene.FIXED
        )
        for anchor in anchors
    ]
    intact_body = [
        obj
        for obj in collection.all_objects
        if (
            obj.type == "MESH"
            and obj.get("v8_part_role") == "body"
            and obj.get("v8_damage_state") == "INTACT"
        )
    ]
    body_faces = [
        (obj, polygon)
        for obj in intact_body
        for polygon in obj.data.polygons
    ]
    body_materials = [
        obj.data.materials[polygon.material_index]
        for obj, polygon in body_faces
        if (
            polygon.material_index < len(obj.data.materials)
            and obj.data.materials[polygon.material_index] is not None
        )
    ]
    environment_body_faces = sum(
        1
        for material in body_materials
        if int(material.get("v8_environment_material_id", -1)) >= 0
    )
    paired_environment_body_faces = sum(
        1
        for obj, polygon in body_faces
        if (
            polygon.hide
            and polygon.material_index < len(obj.data.materials)
            and obj.data.materials[polygon.material_index] is not None
            and obj.data.materials[polygon.material_index].get(
                "v8_preview_pass"
            )
            == "PAIRED_ENVIRONMENT"
        )
    )
    textured_body_faces = sum(
        1
        for material in body_materials
        if (
            int(material.get("v8_texture_index", -1)) >= 0
            and int(material.get("v8_environment_material_id", -1)) < 0
        )
    )
    environment_ids = sorted(
        {
            int(material["v8_environment_material_id"])
            for material in body_materials
            if int(material.get("v8_environment_material_id", -1)) >= 0
        }
    )
    summary = {
        "vehicle": "Thunderbolt",
        "retail_form_and_stat_index": VEHICLE_INDEX,
        "wheel_anchors": native_positions,
        "front_track": [
            abs(native_positions[0][0]),
            abs(native_positions[1][0]),
        ],
        "rear_track": [
            abs(native_positions[2][0]),
            abs(native_positions[3][0]),
        ],
        "modes": ["Standard", "Hover", "Float", "Ski"],
        "intact_body_material_evidence": {
            "faces_total": len(body_faces),
            "faces_using_owned_native_textures": textured_body_faces,
            "faces_using_dynamic_environment_maps": environment_body_faces,
            "faces_using_native_flat_or_gouraud_color": (
                len(body_faces)
                - textured_body_faces
                - environment_body_faces
            ),
            "dynamic_environment_material_ids": [
                f"0x{material_id:04X}"
                for material_id in environment_ids
            ],
            "coplanar_multipass_pairs": paired_environment_body_faces,
            "paired_environment_faces_hidden_in_default_preview": (
                paired_environment_body_faces
            ),
            "decoded_native_textures": len(vehicle.textures),
        },
        "preview_parts": {},
    }
    OUTPUT.mkdir(parents=True, exist_ok=True)

    for mode in ("STANDARD", "HOVER", "FLOAT", "SKI"):
        settings.transformation_preview_mode = mode
        bpy.context.view_layer.update()
        summary["preview_parts"][mode.title()] = [
            {
                "wheel": int(obj["v8_preview_wheel"]),
                "source": str(obj["v8_preview_source"]),
                "native_object_id": int(
                    obj.get("v8_native_object_id", -0x5556)
                ),
                "native_render_flags": int(
                    obj.get("v8_render_flags", 0)
                ),
            }
            for obj in collection.all_objects
            if obj.get("v8_role")
            == authored_scene.ROLE_TRANSFORMATION_PREVIEW
        ]
        bpy.ops.wm.save_as_mainfile(
            filepath=str(
                OUTPUT
                / f"v82_thunderbolt_{mode.lower()}_preview.blend"
            )
        )
    (OUTPUT / "v82_thunderbolt_preview_data.json").write_text(
        json.dumps(summary, indent=2) + "\n",
        encoding="utf-8",
    )


if __name__ == "__main__":
    main()
