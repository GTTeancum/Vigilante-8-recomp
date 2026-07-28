"""Headless Blender checks for donor-free V8/V8:2 production authoring."""

from __future__ import annotations

import json
import difflib
import os
from pathlib import Path
import re
import sys
import tempfile

import bpy


ROOT = Path(__file__).resolve().parents[3]
ADDONS = Path(
    os.environ.get("V8_ADDON_ROOT", ROOT / "tools" / "blender_addons")
).resolve()
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

import vigilante8_vehicle_tools
from vigilante8_vehicle_tools import authored_scene, project, registry


REGISTERED = False


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def clear_scene() -> None:
    global REGISTERED
    bpy.ops.wm.read_factory_settings(use_empty=True)
    if not REGISTERED:
        vigilante8_vehicle_tools.register()
        REGISTERED = True


def require_named_authoring_scene(game: str, collection) -> None:
    render_models = [
        obj for obj in collection.all_objects
        if obj.type == "MESH"
        and obj.get("v8_role") in {"authored_slot", "authored_render_group"}
        and "v8_group_index" in obj
    ]
    require(render_models, f"{game} scene has no editable native models")
    require(
        not any(obj.name.startswith("Render Group ") for obj in render_models),
        f"{game} exposes internal render-group indices in user-facing names",
    )
    require(
        not any(obj.name.startswith("group_") for obj in render_models),
        f"{game} still exposes anonymous group_### render model names",
    )

    require(
        not any(
            child.get("v8_role") == "authored_reference_category"
            for child in bpy.data.collections
        ),
        f"{game} import still creates semantic bucket collections",
    )

    slots = [
        obj for obj in collection.all_objects
        if obj.get("v8_role") == "authored_slot"
    ]
    render_slots = [
        obj for obj in slots if int(obj.get("v8_render_group", -1)) >= 0
    ]
    require(
        render_slots and all(obj.type == "MESH" for obj in render_slots),
        f"{game} render-bearing native slots are not the mesh objects",
    )
    require(
        len({obj.data.as_pointer() for obj in render_slots})
        == len(render_slots),
        f"{game} render-bearing native slots use instantiated mesh data",
    )
    wheel_anchors = [
        obj for obj in slots if obj.get("v8_part_role") == "wheel_anchor"
    ]
    weapon_mounts = [
        obj for obj in slots if obj.get("v8_part_role") == "weapon_mount"
    ]
    vehicle_root = next(
        obj for obj in slots if obj.get("v8_part_role") == "vehicle_root"
    )
    require(
        any(obj.name == "Front Left Wheel Anchor" for obj in wheel_anchors)
        and any(
            obj.name == "Front Right Wheel Anchor"
            for obj in wheel_anchors
        ),
        f"{game} wheel attachment objects are not functionally named",
    )
    require(
        any(obj.name == "Machine Gun Mount" for obj in weapon_mounts)
        and any(obj.name == "Mortar Mount" for obj in weapon_mounts)
        and any(obj.name == "Mine Dripper Mount" for obj in weapon_mounts)
        and any(obj.name == "Special Weapon Mount" for obj in weapon_mounts),
        f"{game} weapon attachment objects are not functionally named",
    )
    require(
        all(obj.parent == vehicle_root for obj in wheel_anchors + weapon_mounts),
        f"{game} attachment objects do not follow the native slot hierarchy",
    )
    require(
        all(
            not obj.show_name and obj.hide_get()
            for obj in wheel_anchors + weapon_mounts
        ),
        f"{game} attachment guides clutter the default modeling view",
    )
    settings = bpy.context.scene.v8_vehicle_settings
    settings.show_attachment_guides = True
    require(
        all(
            obj.show_name and not obj.hide_get()
            for obj in wheel_anchors + weapon_mounts
        ),
        f"{game} attachment-guide toggle does not reveal named guides",
    )
    settings.show_attachment_guides = False
    require(
        all(obj.get("v8_native_key_hex") for obj in wheel_anchors + weapon_mounts),
        f"{game} named attachment objects lost their native keys",
    )
    require(
        all(
            model.get("v8_role") == "authored_slot"
            or not model.get("v8_referencing_slots")
            for model in render_models
        ),
        f"{game} referenced native models are not their actual slot objects",
    )
    require(
        not any(
            model.get("v8_role") == "authored_render_group"
            and model.parent is not None
            for model in render_models
        ),
        f"{game} still inserts a second model layer below native slots",
    )
    require(
        not any(
            obj.get("v8_role") == "authored_render_group_instance"
            for obj in collection.all_objects
        ),
        f"{game} import fabricated render-group instances",
    )
    require(
        not any(
            obj.get("v8_role") == "authored_normal_table"
            for obj in collection.all_objects
        ),
        f"{game} native shading directions are exposed as a point cloud",
    )
    normal_tables = [
        model.get(authored_scene.NATIVE_NORMAL_TABLE_PROPERTY)
        for model in render_models
        if model.get(authored_scene.NATIVE_NORMAL_TABLE_PROPERTY) is not None
    ]
    require(
        all(
            isinstance(table, bpy.types.Mesh)
            and table.attributes.get("v8_normal_pad") is not None
            and not any(obj.data == table for obj in bpy.data.objects)
            for table in normal_tables
        ),
        f"{game} native normal tables are not owned, unlinked mesh data",
    )
    require(
        not any("UNRESOLVED" in obj.name.upper() for obj in slots),
        f"{game} import still exposes an unresolved-parts bucket label",
    )
    forbidden_names = (
        "Visual Submodel",
        "Physical Submodel",
        "Transform Node",
        "Nested Engine Attachment",
        "Unassigned Vehicle Model",
        "Render Group",
    )
    require(
        not any(
            forbidden in obj.name
            for obj in slots
            for forbidden in forbidden_names
        ),
        f"{game} import still exposes a format-centric object name",
    )
    require(
        not any(re.search(r"\.\d{3}$", obj.name) for obj in slots),
        f"{game} semantic names collide and fall back to Blender numeric suffixes",
    )
    require(
        not any(
            "Purpose Unknown" in obj.name
            or obj.name in {"Model", "Collision", "Native Object"}
            for obj in slots
        ),
        f"{game} still exposes an anonymous editable object name",
    )

    if game == "V8_2":
        previews = [
            obj for obj in collection.all_objects
            if obj.get("v8_role")
            == authored_scene.ROLE_TRANSFORMATION_PREVIEW
        ]
        require(
            previews
            and all(obj.get("v8_preview_mode") == "STANDARD" for obj in previews)
            and all(obj.parent in wheel_anchors for obj in previews)
            and len({obj.data.as_pointer() for obj in previews}) == len(previews),
            "V8_2 default wheel preview is not uniquely owned and anchor-mounted",
        )
        require(
            {
                int(obj.get("v8_preview_wheel", -1)) for obj in previews
            } == {
                int(obj.get("v8_attachment_key", -1)) - 0x8000
                for obj in wheel_anchors
            }
            and not any(
                ((int(obj.get("v8_native_object_id", -0x5556)) + 0x8000)
                 & 0xFFFF) - 0x8000 > 0xFF
                for obj in previews
            ),
            "V8_2 preview invents wheels or includes a positive object ID "
            "above the retail constructor's 255 limit",
        )
        right_standard = next(
            obj for obj in previews if int(obj["v8_preview_wheel"]) == 1
        )
        right_rotation = right_standard.matrix_basis.to_3x3()
        require(
            abs(right_rotation[0][0] + 1.0) < 1e-5
            and abs(right_rotation[1][1] - 1.0) < 1e-5
            and abs(right_rotation[2][2] + 1.0) < 1e-5,
            "V8_2 right standard wheel does not use native vr.z=2048 "
            "(Blender Y-axis half-turn)",
        )
        settings.transformation_preview_mode = "FLOAT"
        float_previews = [
            obj for obj in collection.all_objects
            if obj.get("v8_role")
            == authored_scene.ROLE_TRANSFORMATION_PREVIEW
        ]
        float_front_left = next(
            obj for obj in float_previews
            if int(obj["v8_preview_wheel"]) == 0
        )
        float_root_index = int(collection["v8_transform_2_0"])
        require(
            float_front_left.get("v8_preview_source")
            == next(
                obj.name
                for obj in collection.all_objects
                if obj.get("v8_slot_index") == float_root_index
                and obj.get("v8_part_role") == "transformation_wheel_root"
            ),
            "V8_2 Float preview is not sourced from native I_Float/Sea mode 2",
        )
        if str(collection.get("v8_stable_id", "")).startswith("integration."):
            require(
                any(
                    int(obj.get("v8_preview_wheel", -1)) == 0
                    and obj is not float_front_left
                    and (
                        int(obj.get("v8_native_object_id", 0)) & 0xFFFF
                    ) == 0xAAAA
                    for obj in float_previews
                ),
                "V8_2 Float preview discarded a constructible signed 0xAAAA "
                "subassembly",
            )
        settings.transformation_preview_mode = "SKI"
        ski_previews = [
            obj for obj in collection.all_objects
            if obj.get("v8_role")
            == authored_scene.ROLE_TRANSFORMATION_PREVIEW
        ]
        require(
            ski_previews
            and all(obj.get("v8_preview_mode") == "SKI" for obj in ski_previews)
            and all(obj.parent in wheel_anchors for obj in ski_previews),
            "V8_2 Ski preview is not assembled at the real wheel anchors",
        )
        ski_front_left = next(
            obj for obj in ski_previews
            if int(obj["v8_preview_wheel"]) == 0
        )
        ski_root_index = int(collection["v8_transform_3_0"])
        require(
            ski_front_left.get("v8_preview_source")
            == next(
                obj.name
                for obj in collection.all_objects
                if obj.get("v8_slot_index") == ski_root_index
                and obj.get("v8_part_role") == "transformation_wheel_root"
            ),
            "V8_2 Ski preview is not sourced from native Snow mode 3",
        )
        settings.transformation_preview_mode = "STANDARD"
        transform_bank = next(
            child
            for child in collection.children
            if child.get("v8_bank") == "transformation"
        )
        require(
            transform_bank.hide_viewport,
            "V8_2 raw transformation library clutters the default view",
        )
        settings.show_transformation_library = True
        require(
            not transform_bank.hide_viewport,
            "V8_2 transformation-library toggle does not reveal the bank",
        )
        settings.show_transformation_library = False
        transform_roots = [
            obj for obj in slots
            if obj.get("v8_part_role") == "transformation_wheel_root"
        ]
        names = {obj.name for obj in transform_roots}
        require(
            any("Hover" in name and "Wheel" in name for name in names)
            and any("Ski" in name and "Wheel" in name for name in names)
            and any("Float" in name and "Wheel" in name for name in names),
            "V8_2 transformation objects are not named by mode and wheel",
        )
        require(
            all(obj.get("v8_transform_assignment_summary") for obj in transform_roots),
            "V8_2 transformation objects lack readable mode/wheel assignments",
        )

    require(
        all(
            area.spaces.active.shading.type == "MATERIAL"
            for window in bpy.context.window_manager.windows
            for area in window.screen.areas
            if area.type == "VIEW_3D"
        ),
        f"{game} import does not default to textured Material Preview",
    )


def retail_hierarchy_round_trip(game: str, path: Path) -> None:
    if not path.exists():
        return
    clear_scene()
    source = project.VehicleProject.from_dict(
        json.loads(path.read_text(encoding="utf-8"))
    )
    collection = authored_scene.project_to_scene(bpy.context, source)
    require_named_authoring_scene(game, collection)
    slots = [
        obj for obj in collection.all_objects
        if obj.get("v8_role") == "authored_slot"
    ]
    names = {obj.name for obj in slots}
    require(
        any(name.endswith("Body - Intact") for name in names)
        and any(name.endswith("Body - Damaged") for name in names),
        f"{game} retail body damage hierarchy is not named by state",
    )
    require(
        all("v8_native_object_id" in obj for obj in slots),
        f"{game} retail objects do not expose their semantic native object ID",
    )
    effect_nodes = [
        obj for obj in slots
        if (
            0x8400 <= int(obj.get("v8_attachment_key", -1)) <= 0x88FF
            or int(obj.get("v8_attachment_key", -1)) >> 12 == 9
        )
    ]
    require(
        effect_nodes
        and all(obj.type == "EMPTY" for obj in effect_nodes)
        and all(
            obj.get("v8_part_role") == "damage_effect_anchor"
            for obj in effect_nodes
        ),
        f"{game} retail damage effects were mistaken for model objects",
    )
    if game == "V8_2":
        required = {
            "Vehicle Distance LOD",
            "HUD Vehicle Icon Texture Source",
            "Acceleration Upgrade Appearance",
            "Armor Upgrade Appearance",
            "Handling Upgrade Appearance",
            "Vehicle Destruction Sequence",
            "Special Weapon Mount",
            "Special Weapon Assembly - Base",
            "Special Weapon Assembly - Articulated Section",
            "Special Weapon Assembly - Emitter",
        }
        require(
            required <= names,
            "V8_2 retail hierarchy is missing semantic objects: "
            + ", ".join(sorted(required - names)),
        )
        face_materials = [
            obj.data.materials[polygon.material_index]
            for obj in slots
            if obj.type == "MESH"
            for polygon in obj.data.polygons
            if (
                polygon.material_index < len(obj.data.materials)
                and obj.data.materials[polygon.material_index] is not None
            )
        ]
        environment_ids = {
            int(material.get("v8_environment_material_id", -1))
            for material in face_materials
            if int(material.get("v8_environment_material_id", -1)) >= 0
        }
        require(
            {0x3FFE, 0x3FFF} <= environment_ids,
            "V8_2 dynamic gloss/reflection packets were not exposed as "
            "materials",
        )
        textured_materials = [
            material
            for material in face_materials
            if int(material.get("v8_texture_index", -1)) >= 0
            and material.get("v8_preview_pass") != "PAIRED_ENVIRONMENT"
        ]
        require(
            textured_materials
            and all(
                material.use_nodes
                and material.surface_render_method == "DITHERED"
                for material in textured_materials
            ),
            "V8_2 ordinary diffuse materials use an unstable blended wheel "
            "or body preview",
        )
        paired_environment_materials = [
            material
            for material in face_materials
            if material.get("v8_preview_pass") == "PAIRED_ENVIRONMENT"
        ]
        paired_surface_materials = [
            material
            for material in face_materials
            if int(
                material.get(
                    "v8_paired_environment_material_id", -1
                )
            ) >= 0
        ]
        require(
            paired_environment_materials
            and paired_surface_materials
            and all(
                material.surface_render_method == "BLENDED"
                and "Paired Native Pass" in material.name
                for material in paired_environment_materials
            ),
            "V8_2 coplanar environment/surface packets do not have an "
            "explicit stable preview pairing",
        )
        hidden_paired_faces = [
            (obj, polygon)
            for obj in slots
            if obj.type == "MESH"
            for polygon in obj.data.polygons
            if (
                polygon.hide
                and obj.data.materials[polygon.material_index].get(
                    "v8_preview_pass"
                )
                == "PAIRED_ENVIRONMENT"
            )
        ]
        require(
            hidden_paired_faces
            and all(
                int(
                    obj.get(
                        "v8_hidden_paired_environment_faces", 0
                    )
                )
                == sum(
                    1
                    for polygon in obj.data.polygons
                    if polygon.hide
                    and obj.data.materials[
                        polygon.material_index
                    ].get("v8_preview_pass")
                    == "PAIRED_ENVIRONMENT"
                )
                for obj, _polygon in hidden_paired_faces
            ),
            "V8_2 paired gloss packets can still depth-occlude their native "
            "surface partners",
        )
        require(
            all(
                material.node_tree.nodes[
                    "Principled BSDF"
                ].inputs["Emission Color"].is_linked
                for material in textured_materials
            ),
            "V8_2 indexed textures are being washed out by Blender studio "
            "lighting instead of shown as native palette color",
        )
        normal_bearing_faces = [
            polygon
            for obj in slots
            if obj.type == "MESH"
            and obj.data.attributes.get("v8_normal_count") is not None
            for polygon in obj.data.polygons
            if obj.data.attributes["v8_normal_count"].data[
                polygon.index
            ].value
            > 0
        ]
        require(
            normal_bearing_faces
            and all(polygon.use_smooth for polygon in normal_bearing_faces),
            "V8_2 native loop normals are decoded but not applied to the "
            "visible mesh",
        )
    rebuilt = authored_scene.scene_to_project(collection)
    require(
        project.to_dict(rebuilt) == project.to_dict(source),
        f"{game} semantic retail hierarchy did not round-trip exactly",
    )
    with tempfile.TemporaryDirectory() as directory:
        blend_path = Path(directory) / f"{game.lower()}_normal_tables.blend"
        bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
        bpy.ops.wm.open_mainfile(filepath=str(blend_path))
        reopened = next(
            item
            for item in bpy.data.collections
            if item.get("v8_role") == authored_scene.ROLE_VEHICLE
        )
        require(
            project.to_dict(authored_scene.scene_to_project(reopened))
            == project.to_dict(source),
            f"{game} saved .blend lost owned native-normal data",
        )


def authored_round_trip(game: str) -> None:
    clear_scene()
    original = authored_scene.new_project(
        game, f"integration.{game.lower()}.original"
    )
    collection = authored_scene.project_to_scene(bpy.context, original)
    require_named_authoring_scene(game, collection)
    body_meshes = [
        obj for obj in collection.all_objects
        if obj.type == "MESH"
        and obj.get("v8_role") in {"authored_slot", "authored_render_group"}
        and "v8_group_index" in obj
        and ".body." in obj.data.name
    ]
    require(body_meshes, f"{game} scene has no editable body mesh")
    require(
        max(vertex.co.z for obj in body_meshes for vertex in obj.data.vertices)
        > 0.0,
        f"{game} native negative-Y body height did not map to Blender Z-up",
    )
    require(
        not any(text.get("v8_embedded_native_source") for text in bpy.data.texts),
        "authored project embedded source/blob text",
    )
    require("v8_source_text" not in collection, "authored project has a source archive")
    rebuilt = authored_scene.scene_to_project(collection)
    original_json = json.dumps(
        project.to_dict(original), indent=2, sort_keys=True
    ).splitlines()
    rebuilt_json = json.dumps(
        project.to_dict(rebuilt), indent=2, sort_keys=True
    ).splitlines()
    require(
        original_json == rebuilt_json,
        f"{game} Blender round trip changed decoded project data:\n" +
        "\n".join(difflib.unified_diff(
            original_json,
            rebuilt_json,
            fromfile="original",
            tofile="rebuilt",
            n=3,
        )),
    )
    package = registry.compile_package((rebuilt,))
    parsed_game, entries = registry.parse_registry(package.registry)
    require(parsed_game == game, f"{game} registry game changed")
    require(len(entries) == 1, f"{game} registry entry count changed")
    require(entries[0].stable_id == original.stable_id, "stable identity changed")

    decoded = registry.decompile_package(package.archive, package.registry)
    require(len(decoded) == 1, f"{game} native package entry count changed")
    clear_scene()
    native_collection = authored_scene.project_to_scene(
        bpy.context, decoded[0])
    require_named_authoring_scene(game, native_collection)
    require(
        not any(text.get("v8_embedded_native_source") for text in bpy.data.texts),
        "native package import embedded source/blob text",
    )
    native_rebuilt = authored_scene.scene_to_project(native_collection)
    native_package = registry.compile_package((native_rebuilt,))
    require(
        native_package.archive == package.archive,
        f"{game} native package Blender round trip changed CUSTOM.EXP",
    )
    require(
        native_package.registry == package.registry,
        f"{game} native package Blender round trip changed VEHICLES.V8R",
    )

    clear_scene()
    split_source = authored_scene.new_project(
        game, f"integration.{game.lower()}.split"
    )
    split_collection = authored_scene.project_to_scene(
        bpy.context, split_source
    )
    split_body_bank = next(
        child
        for child in split_collection.children
        if child.get("v8_bank") == "body"
    )
    shared_models = sorted(
        (
            obj
            for obj in split_body_bank.all_objects
            if obj.get("v8_role") == "authored_slot"
            and int(obj.get("v8_render_group", -1)) == 0
            and obj.type == "MESH"
        ),
        key=lambda obj: int(obj["v8_slot_index"]),
    )
    require(
        len(shared_models) > 1,
        f"{game} split-model fixture has no shared native model",
    )
    shared_models[1].data.vertices[0].co.x += 0.25
    shared_models[1].data.update()
    split_result = authored_scene.scene_to_project(split_collection)
    first_slot = int(shared_models[0]["v8_slot_index"])
    edited_slot = int(shared_models[1]["v8_slot_index"])
    require(
        len(split_result.groups) == len(split_source.groups) + 1
        and split_result.slots[first_slot].render_group
        != split_result.slots[edited_slot].render_group,
        f"{game} independent mesh edit did not create a unique native model "
        f"(groups {len(split_source.groups)}->{len(split_result.groups)}, "
        f"slots {first_slot}:"
        f"{split_result.slots[first_slot].render_group} and {edited_slot}:"
        f"{split_result.slots[edited_slot].render_group})",
    )
    registry.compile_package((split_result,))

    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / f"{game.lower()}.json"
        path.write_text(
            json.dumps(project.to_dict(rebuilt), indent=2) + "\n",
            encoding="utf-8",
        )
        reparsed = project.VehicleProject.from_dict(
            json.loads(path.read_text(encoding="utf-8"))
        )
        require(
            project.to_dict(reparsed) == project.to_dict(original),
            f"{game} strict JSON round trip changed project data",
        )


def automatic_collision_export(game: str) -> None:
    clear_scene()
    original = authored_scene.new_project(
        game, f"integration.{game.lower()}.automatic_collision"
    )
    collection = authored_scene.project_to_scene(bpy.context, original)
    collection["v8_collision_export"] = "AUTO_ALWAYS"
    rebuilt = authored_scene.scene_to_project(collection)
    banks = [rebuilt]
    if rebuilt.transformation_bank is not None:
        banks.append(rebuilt.transformation_bank)
    require(
        all(
            stream.shapes
            and len(stream.shapes) == 1
            and isinstance(stream.shapes[0], project.CollisionAabb)
            for bank in banks
            for stream in bank.collisions
        ),
        f"{game} automatic export did not emit one native box per collision owner",
    )
    require(
        any(
            obj.get("v8_generated_from_mesh")
            for obj in collection.all_objects
        ),
        f"{game} automatic collision was not calculated during export",
    )
    registry.compile_package((rebuilt,))


def main() -> None:
    require(
        Path(vigilante8_vehicle_tools.__file__).resolve().is_relative_to(ADDONS),
        f"loaded add-on outside requested root: {vigilante8_vehicle_tools.__file__}",
    )
    authored_round_trip("V8_2")
    authored_round_trip("V8")
    automatic_collision_export("V8_2")
    automatic_collision_export("V8")
    retail_hierarchy_round_trip(
        "V8_2",
        ROOT / "artifacts" / "dual_game_default_roundtrip"
        / "source_free_v82" / "project.json",
    )
    retail_hierarchy_round_trip(
        "V8",
        ROOT / "artifacts" / "dual_game_default_roundtrip"
        / "source_free_v8" / "project.json",
    )
    print("V8 donor-free Blender integration: PASS")


if __name__ == "__main__":
    main()
