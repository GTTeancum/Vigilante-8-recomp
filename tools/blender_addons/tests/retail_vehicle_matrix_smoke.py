"""Smoke every retail vehicle and authoring-visible variant in Blender.

This is deliberately broader than the focused integration fixture. It imports
all 13 V8 and all 18 V8:2 vehicle/stat entries, audits every native face and
texture, cycles every decoded body-damage state, cycles all four sequel wheel
modes, performs a small Eevee material render for every combination, then
saves, reopens, and recompiles each vehicle byte-exactly.
"""

from __future__ import annotations

from array import array
from dataclasses import replace
import hashlib
import json
import math
import os
from pathlib import Path
import subprocess
import sys
import tempfile

import bpy
from mathutils import Vector


ROOT = Path(__file__).resolve().parents[3]
ADDONS = Path(
    os.environ.get("V8_ADDON_ROOT", ROOT / "tools" / "blender_addons")
).resolve()
TESTS = Path(__file__).resolve().parent
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))
if str(TESTS) not in sys.path:
    sys.path.insert(0, str(TESTS))

import vigilante8_vehicle_tools
from vigilante8_vehicle_tools import (
    authored_scene,
    project,
    registry,
    stats,
)
from build_source_free_default_replacements import (
    bank,
    extract_roots,
    merge_v8_body_and_wheels,
)


OUTPUT = (
    ROOT
    / "artifacts"
    / "vehicle_plugin_smoke"
    / "all_retail_vehicle_variant_smoke.json"
)
V8_COMMON = (
    ROOT
    / "artifacts"
    / "dual_game_default_roundtrip"
    / "V8_COMMON_ORIGINAL.EXP"
)
V82_COMMON = (
    ROOT
    / "artifacts"
    / "dual_game_default_roundtrip"
    / "V82_COMMON_ORIGINAL.EXP"
)
V8_EXE = ROOT / "PS1 game" / "SLUS_005.10"
V82_EXE = ROOT / "V8_2_LOOSE" / "SLUS_008.68"
BODY_STATE_ORDER = (
    "INTACT",
    "DAMAGED",
    "HEAVILY_DAMAGED",
    "DESTROYED",
)
V82_MODE_ORDER = ("STANDARD", "HOVER", "FLOAT", "SKI")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def clear_scene() -> None:
    bpy.ops.wm.read_factory_settings(use_empty=True)


def build_vehicle(game: str, vehicle_index: int) -> project.VehicleProject:
    if game == "V8":
        stats_file = stats.StatsFile(V8_EXE.read_bytes(), game)
        values = stats_file.record(vehicle_index).values()
        values.pop("vehicle_type")
        body = bank(V8_COMMON, game, vehicle_index)
        wheel_source = bank(V8_COMMON, game, 13)
        wheel_roots = {
            values["wheel_kind_front"],
            values["wheel_kind_rear"],
        }
        wheels, root_map = extract_roots(wheel_source, wheel_roots)
        merged, wheel_slot_base = merge_v8_body_and_wheels(body, wheels)
        values["wheel_kind_front"] = (
            wheel_slot_base + root_map[values["wheel_kind_front"]]
        )
        values["wheel_kind_rear"] = (
            wheel_slot_base + root_map[values["wheel_kind_rear"]]
        )
        base = authored_scene.new_project(
            game, f"smoke.v8.retail_vehicle_{vehicle_index:02d}"
        )
        result = replace(
            base,
            display_name=f"V8 Retail Vehicle {vehicle_index:02d}",
            groups=merged.groups,
            slots=merged.slots,
            collisions=merged.collisions,
            textures=merged.textures,
            animations=merged.animations,
            stats=values,
            body_kind=0,
        )
    else:
        stats_file = stats.StatsFile(V82_EXE.read_bytes(), game)
        record = stats_file.record(vehicle_index)
        values = record.values()
        values.pop("vehicle_type")
        values["rear_suspension_damping"] = int.from_bytes(
            record.raw[0x0C:0x0E], "little"
        )
        body = bank(V82_COMMON, game, vehicle_index)
        transform_source = bank(V82_COMMON, game, 18)
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
            game, f"smoke.v82.retail_vehicle_{vehicle_index:02d}"
        )
        result = replace(
            base,
            display_name=f"V8:2 Retail Vehicle {vehicle_index:02d}",
            groups=body.groups,
            slots=body.slots,
            collisions=body.collisions,
            textures=body.textures,
            animations=body.animations,
            stats=values,
            body_kind=0,
            transformation_bank=transform,
            transform_modes=mapped_modes,
            powerups=stats_file.powerup_values(),
        )
    result.validate()
    return result


def expected_pairs(
    group: project.RenderGroup,
) -> tuple[set[int], dict[int, int]]:
    triangles: dict[tuple[int, int, int], list[int]] = {}
    for face_index, face in enumerate(group.faces):
        triangles.setdefault(tuple(sorted(face.vertices)), []).append(
            face_index
        )
    environment_faces: set[int] = set()
    surface_environment: dict[int, int] = {}
    for face_indices in triangles.values():
        for position, face_index in enumerate(face_indices):
            face = group.faces[face_index]
            if (
                face.packet_kind != 12
                or not face.environment_parameters
            ):
                continue
            surface_index = next(
                (
                    candidate
                    for candidate in face_indices[position + 1 :]
                    if group.faces[candidate].packet_kind != 12
                ),
                None,
            )
            if surface_index is None:
                continue
            environment_faces.add(face_index)
            surface_environment[surface_index] = (
                face.environment_parameters[0] & 0x3FFF
            )
    return environment_faces, surface_environment


def material_uses_emission(material: bpy.types.Material) -> bool:
    if not material.use_nodes or material.node_tree is None:
        return False
    shader = material.node_tree.nodes.get("Principled BSDF")
    return (
        shader is not None
        and shader.inputs["Emission Color"].is_linked
        and shader.inputs["Emission Strength"].default_value > 0.0
    )


def audit_bank(
    collection: bpy.types.Collection,
    bank_name: str,
    source_bank: project.VehicleProject | project.ObjectBank,
) -> dict[str, int]:
    bank_collections = [
        child
        for child in collection.children
        if child.get("v8_bank") == bank_name
    ]
    require(
        len(bank_collections) == 1,
        f"{collection.name} must contain exactly one {bank_name} bank collection",
    )
    meshes_by_group: dict[int, list[bpy.types.Object]] = {}
    for obj in bank_collections[0].all_objects:
        if obj.type == "MESH" and "v8_group_index" in obj:
            meshes_by_group.setdefault(
                int(obj["v8_group_index"]), []
            ).append(obj)
    require(
        set(meshes_by_group) == set(range(len(source_bank.groups))),
        f"{collection.name} {bank_name} bank did not expose every render group",
    )

    result = {
        "faces": 0,
        "textured_faces": 0,
        "environment_faces": 0,
        "paired_environment_faces": 0,
        "normal_bearing_faces": 0,
        "direct_color_textures": 0,
    }
    for group_index, group in enumerate(source_bank.groups):
        paired_environment, paired_surfaces = expected_pairs(group)
        result["faces"] += len(group.faces)
        result["paired_environment_faces"] += len(
            paired_environment
        )
        for obj in meshes_by_group[group_index]:
            require(
                len(obj.data.polygons) == len(group.faces),
                f"{obj.name} polygon count changed during import",
            )
            hidden_count = 0
            for face_index, (face, polygon) in enumerate(
                zip(group.faces, obj.data.polygons, strict=True)
            ):
                require(
                    polygon.material_index < len(obj.data.materials),
                    f"{obj.name} face {face_index} has no material",
                )
                material = obj.data.materials[polygon.material_index]
                require(
                    material is not None,
                    f"{obj.name} face {face_index} material is null",
                )
                environment_id = (
                    face.environment_parameters[0] & 0x3FFF
                    if (
                        face.packet_kind == 12
                        and face.environment_parameters
                    )
                    else -1
                )
                imported_environment = int(
                    material.get("v8_environment_material_id", -1)
                )
                imported_texture = int(
                    material.get("v8_texture_index", -1)
                )
                require(
                    imported_environment == environment_id,
                    f"{obj.name} face {face_index} environment ID changed",
                )
                require(
                    imported_texture
                    == (-1 if face.texture is None else face.texture),
                    f"{obj.name} face {face_index} texture index changed",
                )
                should_hide = face_index in paired_environment
                require(
                    bool(polygon.hide) == should_hide,
                    f"{obj.name} face {face_index} has incorrect paired-pass "
                    "visibility",
                )
                if should_hide:
                    hidden_count += 1
                    require(
                        material.get("v8_preview_pass")
                        == "PAIRED_ENVIRONMENT",
                        f"{obj.name} face {face_index} lost its paired native "
                        "environment pass",
                    )
                    require(
                        material.surface_render_method == "BLENDED",
                        f"{obj.name} face {face_index} paired pass is not "
                        "transparent",
                    )
                else:
                    require(
                        material.get("v8_preview_pass") == "SURFACE",
                        f"{obj.name} face {face_index} is incorrectly marked "
                        "as a paired pass",
                    )
                paired_id = int(
                    material.get(
                        "v8_paired_environment_material_id", -1
                    )
                )
                require(
                    paired_id
                    == paired_surfaces.get(face_index, -1),
                    f"{obj.name} face {face_index} lost its native surface/"
                    "environment pairing",
                )
                if face.texture is not None and environment_id < 0:
                    require(
                        material.surface_render_method == "DITHERED",
                        f"{obj.name} face {face_index} ordinary texture uses "
                        "the wheel-breaking blended workaround",
                    )
                    require(
                        material_uses_emission(material),
                        f"{obj.name} face {face_index} indexed texture is "
                        "still affected by Blender studio lighting",
                    )
                if face.normal_indices:
                    require(
                        polygon.use_smooth,
                        f"{obj.name} face {face_index} did not receive native "
                        "loop normals",
                    )
            require(
                hidden_count
                == int(
                    obj.get(
                        "v8_hidden_paired_environment_faces", -1
                    )
                ),
                f"{obj.name} paired-pass metadata count is incorrect",
            )

        result["textured_faces"] += sum(
            1
            for face in group.faces
            if face.texture is not None
            and not (
                face.packet_kind == 12
                and face.environment_parameters
            )
        )
        result["environment_faces"] += sum(
            1
            for face in group.faces
            if (
                face.packet_kind == 12
                and face.environment_parameters
            )
        )
        result["normal_bearing_faces"] += sum(
            1 for face in group.faces if face.normal_indices
        )

    images = {
        int(image["v8_texture_index"]): image
        for image in bpy.data.images
        if (
            image.get("v8_role") == "authored_texture"
            and image.get("v8_vehicle_collection") == collection.name
            and image.get("v8_bank") == bank_name
        )
    }
    require(
        set(images) == set(range(len(source_bank.textures))),
        f"{collection.name} {bank_name} bank did not create every texture "
        "as an editable Blender image",
    )
    for texture_index, texture in enumerate(source_bank.textures):
        image = images[texture_index]
        require(
            tuple(image.size) == (texture.width, texture.height),
            f"{image.name} dimensions changed",
        )
        require(
            image.packed_file is not None and image.use_fake_user,
            f"{image.name} is not self-contained for save/reopen",
        )
        if texture.depth == 2:
            result["direct_color_textures"] += 1
            require(
                image.get("v8_storage")
                == "Direct 16-bit BGR555 pixels",
                f"{image.name} direct-color storage is not human-readable",
            )
            require(
                tuple(
                    int(value)
                    for value in image[
                        "v8_direct_pixels_bgr555"
                    ]
                )
                == texture.direct_pixels_bgr555,
                f"{image.name} direct-color pixels changed during import",
            )
    return result


def damage_states(collection: bpy.types.Collection) -> tuple[str, ...]:
    present = {
        state
        for obj in collection.all_objects
        if obj.type == "MESH"
        for state in (authored_scene._slot_damage_state(obj),)
        if state is not None
    }
    require(
        any(
            obj.type == "MESH"
            and obj.get("v8_role") == "authored_slot"
            and obj.get("v8_part_role")
            not in authored_scene.ADVANCED_MODEL_ROLES
            for child in collection.children
            if child.get("v8_bank") == "body"
            for obj in child.all_objects
        ),
        f"{collection.name} has no authored body mesh",
    )
    # Some retail vehicles put the intact geometry directly on the native
    # vehicle-root object instead of a key-identified body child.  That root
    # has no damage-state tag, but it is the engine's ordinary/default state.
    present.add("INTACT")
    return tuple(state for state in BODY_STATE_ORDER if state in present)


def preview_wheel_indices(
    collection: bpy.types.Collection,
) -> set[int]:
    return {
        int(obj["v8_preview_wheel"])
        for obj in collection.all_objects
        if obj.get("v8_role")
        == authored_scene.ROLE_TRANSFORMATION_PREVIEW
    }


def expected_wheel_indices(
    collection: bpy.types.Collection,
) -> set[int]:
    return {
        int(obj["v8_attachment_key"]) - 0x8000
        for obj in collection.all_objects
        if (
            obj.get("v8_part_role") == "wheel_anchor"
            and 0x8000
            <= int(obj.get("v8_attachment_key", -1))
            <= 0x8005
        )
    }


def setup_renderer() -> tuple[bpy.types.Object, bpy.types.Object]:
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE_NEXT"
    scene.render.resolution_x = 128
    scene.render.resolution_y = 96
    scene.render.resolution_percentage = 100
    scene.render.film_transparent = True
    scene.render.image_settings.file_format = "PNG"

    camera_data = bpy.data.cameras.new("Retail Matrix Smoke Camera")
    camera = bpy.data.objects.new(
        "Retail Matrix Smoke Camera", camera_data
    )
    scene.collection.objects.link(camera)
    scene.camera = camera
    camera_data.lens = 52.0

    light_data = bpy.data.lights.new(
        "Retail Matrix Smoke Light", type="AREA"
    )
    light_data.energy = 900.0
    light_data.shape = "DISK"
    light_data.size = 8.0
    light = bpy.data.objects.new(
        "Retail Matrix Smoke Light", light_data
    )
    scene.collection.objects.link(light)
    return camera, light


def set_render_visibility(
    collection: bpy.types.Collection,
    camera: bpy.types.Object,
    light: bpy.types.Object,
) -> list[bpy.types.Object]:
    transform_objects = {
        obj
        for child in collection.children
        if child.get("v8_bank") == "transformation"
        for obj in child.all_objects
    }
    visible = []
    for obj in bpy.context.scene.objects:
        if obj in {camera, light}:
            obj.hide_render = False
            continue
        obj.hide_render = bool(obj.hide_get() or obj in transform_objects)
        if (
            obj.type == "MESH"
            and not obj.hide_render
            and len(obj.data.polygons) > 0
        ):
            visible.append(obj)
    return visible


def frame_camera(
    camera: bpy.types.Object,
    light: bpy.types.Object,
    visible: list[bpy.types.Object],
) -> None:
    coordinates = [
        obj.matrix_world @ vertex.co
        for obj in visible
        for vertex in obj.data.vertices
    ]
    require(coordinates, "variant has no renderable vertices")
    minimum = Vector(
        tuple(min(value[axis] for value in coordinates) for axis in range(3))
    )
    maximum = Vector(
        tuple(max(value[axis] for value in coordinates) for axis in range(3))
    )
    center = (minimum + maximum) * 0.5
    radius = max((maximum - minimum).length * 0.5, 0.25)
    direction = Vector((1.35, -1.8, 1.0)).normalized()
    half_angle = math.radians(camera.data.angle_y) * 0.5
    distance = radius / max(math.tan(half_angle), 0.1) * 0.75
    camera.location = center + direction * distance
    camera.rotation_euler = (
        center - camera.location
    ).to_track_quat("-Z", "Y").to_euler()
    camera.data.clip_start = max(0.001, distance / 1000.0)
    camera.data.clip_end = max(1000.0, distance * 20.0)
    light.location = center + Vector((0.0, -radius, radius * 3.0))
    light.rotation_euler = (
        center - light.location
    ).to_track_quat("-Z", "Y").to_euler()
    light.data.size = max(radius * 2.0, 1.0)


def render_digest(
    collection: bpy.types.Collection,
    camera: bpy.types.Object,
    light: bpy.types.Object,
) -> tuple[str, int, int]:
    visible = set_render_visibility(collection, camera, light)
    frame_camera(camera, light, visible)
    bpy.context.view_layer.update()
    bpy.ops.render.render()
    render = bpy.data.images.get("Render Result")
    require(render is not None, "Blender did not produce a render result")
    render_path = (
        Path(tempfile.gettempdir())
        / f"v8_vehicle_smoke_render_{os.getpid()}.png"
    )
    render.save_render(str(render_path))
    render_bytes = render_path.read_bytes()
    check_image = bpy.data.images.load(
        str(render_path), check_existing=False
    )
    pixels = array("f", [0.0]) * (
        check_image.size[0] * check_image.size[1] * 4
    )
    check_image.pixels.foreach_get(pixels)
    bpy.data.images.remove(check_image)
    render_path.unlink()
    packed = bytes(
        max(0, min(255, round(component * 255.0)))
        for component in pixels
    )
    foreground = sum(
        1 for alpha in packed[3::4] if alpha >= 16
    )
    colors = {
        packed[offset : offset + 3]
        for offset in range(0, len(packed), 4)
        if packed[offset + 3] >= 16
    }
    if foreground < 24:
        failure_path = OUTPUT.parent / "failed_variant_render.png"
        failure_path.write_bytes(render_bytes)
        print(
            "RENDER FAILURE",
            collection.name,
            "foreground",
            foreground,
            "visible",
            [
                (
                    obj.name,
                    tuple(round(value, 4) for value in obj.dimensions),
                    tuple(
                        round(value, 4)
                        for value in obj.matrix_world.translation
                    ),
                )
                for obj in visible
            ],
        )
    require(
        foreground >= 24,
        f"{collection.name} variant rendered fewer than 24 foreground pixels",
    )
    require(
        len(colors) >= 2,
        f"{collection.name} variant render is a flat/empty color",
    )
    return (
        hashlib.sha256(packed).hexdigest().upper(),
        foreground,
        len(colors),
    )


def remove_renderer(
    camera: bpy.types.Object, light: bpy.types.Object
) -> None:
    bpy.data.objects.remove(camera, do_unlink=True)
    bpy.data.objects.remove(light, do_unlink=True)


def variant_smoke(
    collection: bpy.types.Collection,
) -> list[dict[str, object]]:
    states = damage_states(collection)
    modes = (
        V82_MODE_ORDER
        if collection.get("v8_game") == "V8_2"
        else ("STANDARD",)
    )
    camera, light = setup_renderer()
    variants = []
    expected_wheels = expected_wheel_indices(collection)
    for state in states:
        for mode in modes:
            authored_scene.apply_authoring_visibility(
                collection,
                preview_state=state,
                show_attachment_guides=False,
                show_advanced=False,
                show_transform_library=False,
            )
            authored_scene.update_transformation_preview(
                collection, mode
            )
            bpy.context.view_layer.update()
            if collection.get("v8_game") == "V8_2":
                previews = [
                    obj
                    for obj in collection.all_objects
                    if obj.get("v8_role")
                    == authored_scene.ROLE_TRANSFORMATION_PREVIEW
                ]
                require(
                    preview_wheel_indices(collection) == expected_wheels,
                    f"{collection.name} {mode} did not assemble every "
                    "native wheel anchor",
                )
                require(
                    len({obj.data.as_pointer() for obj in previews})
                    == len(previews),
                    f"{collection.name} {mode} preview uses instanced meshes",
                )
                require(
                    all(
                        material.surface_render_method != "BLENDED"
                        for obj in previews
                        for material in obj.data.materials
                        if (
                            material is not None
                            and int(
                                material.get(
                                    "v8_environment_material_id", -1
                                )
                            )
                            < 0
                        )
                    ),
                    f"{collection.name} {mode} wheel preview inherited the "
                    "global blend workaround",
                )
            digest, foreground, colors = render_digest(
                collection, camera, light
            )
            variants.append(
                {
                    "body_state": state,
                    "wheel_mode": mode,
                    "render_sha256": digest,
                    "foreground_pixels": foreground,
                    "unique_foreground_colors": colors,
                }
            )

    if collection.get("v8_game") == "V8_2":
        authored_scene.update_transformation_preview(collection, "NONE")
        require(
            not preview_wheel_indices(collection),
            f"{collection.name} Body Only mode still shows wheel previews",
        )
    remove_renderer(camera, light)
    return variants


def save_reopen_round_trip(
    source: project.VehicleProject,
    collection: bpy.types.Collection,
    expected_package: registry.CompiledVehiclePackage,
) -> None:
    authored_scene.apply_authoring_visibility(
        collection,
        preview_state="INTACT",
        show_attachment_guides=False,
        show_advanced=False,
        show_transform_library=False,
    )
    authored_scene.update_transformation_preview(
        collection,
        "STANDARD" if source.game == "V8_2" else "NONE",
    )
    before_save = authored_scene.scene_to_project(collection)
    require(
        project.to_dict(before_save) == project.to_dict(source),
        f"{source.display_name} changed after cycling variants",
    )
    before_package = registry.compile_package((before_save,))
    require(
        before_package.archive == expected_package.archive
        and before_package.registry == expected_package.registry,
        f"{source.display_name} native package changed after cycling variants",
    )

    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "retail_vehicle_smoke.blend"
        bpy.ops.wm.save_as_mainfile(filepath=str(path))
        bpy.ops.wm.open_mainfile(filepath=str(path))
        reopened = next(
            item
            for item in bpy.data.collections
            if item.get("v8_role") == authored_scene.ROLE_VEHICLE
        )
        rebuilt = authored_scene.scene_to_project(reopened)
        require(
            project.to_dict(rebuilt) == project.to_dict(source),
            f"{source.display_name} changed after .blend save/reopen",
        )
        reopened_package = registry.compile_package((rebuilt,))
        require(
            reopened_package.archive == expected_package.archive
            and reopened_package.registry == expected_package.registry,
            f"{source.display_name} package is not byte-exact after "
            "save/reopen",
        )


def run_vehicle(game: str, vehicle_index: int) -> dict[str, object]:
    clear_scene()
    source = build_vehicle(game, vehicle_index)
    expected_package = registry.compile_package((source,))
    decoded = registry.decompile_package(
        expected_package.archive, expected_package.registry
    )[0]
    require(
        project.to_dict(decoded) == project.to_dict(source),
        f"{source.display_name} source-free native compile/decompile changed",
    )
    collection = authored_scene.project_to_scene(
        bpy.context, decoded
    )
    body_metrics = audit_bank(collection, "body", decoded)
    transform_metrics = (
        {}
        if decoded.transformation_bank is None
        else audit_bank(
            collection,
            "transformation",
            decoded.transformation_bank,
        )
    )
    variants = variant_smoke(collection)
    save_reopen_round_trip(
        decoded, collection, expected_package
    )
    return {
        "game": game,
        "retail_vehicle_index": vehicle_index,
        "display_name": source.display_name,
        "body_groups": len(source.groups),
        "body_slots": len(source.slots),
        "body_textures": len(source.textures),
        "body_metrics": body_metrics,
        "transformation_metrics": transform_metrics,
        "body_states": list(
            dict.fromkeys(
                variant["body_state"] for variant in variants
            )
        ),
        "wheel_modes": list(
            dict.fromkeys(
                variant["wheel_mode"] for variant in variants
            )
        ),
        "variants": variants,
        "custom_exp_sha256": hashlib.sha256(
            expected_package.archive
        ).hexdigest().upper(),
        "vehicles_v8r_sha256": hashlib.sha256(
            expected_package.registry
        ).hexdigest().upper(),
        "native_round_trip": "PASS",
        "blend_save_reopen": "PASS",
    }


def write_manifest(
    results: list[dict[str, object]], *, result: str = "PASS"
) -> None:
    manifest = {
        "schema": 1,
        "addon_root": str(ADDONS),
        "blender_version": bpy.app.version_string,
        "scope": {
            "V8_retail_vehicles": stats.PROFILES["V8"].record_count,
            "V8_2_retail_vehicles": stats.PROFILES[
                "V8_2"
            ].record_count,
            "body_states": list(BODY_STATE_ORDER),
            "V8_2_wheel_modes": list(V82_MODE_ORDER),
        },
        "summary": {
            "vehicles_passed": len(results),
            "variant_renders_passed": sum(
                len(result["variants"]) for result in results
            ),
            "faces_audited": sum(
                int(result["body_metrics"]["faces"])
                + int(
                    result["transformation_metrics"].get(
                        "faces", 0
                    )
                )
                for result in results
            ),
            "paired_environment_faces_audited": sum(
                int(
                    result["body_metrics"][
                        "paired_environment_faces"
                    ]
                )
                + int(
                    result["transformation_metrics"].get(
                        "paired_environment_faces", 0
                    )
                )
                for result in results
            ),
            "direct_color_textures_audited": sum(
                int(
                    result["body_metrics"][
                        "direct_color_textures"
                    ]
                )
                + int(
                    result["transformation_metrics"].get(
                        "direct_color_textures", 0
                    )
                )
                for result in results
            ),
            "result": result,
        },
        "vehicles": results,
    }
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )


def main() -> None:
    require(
        Path(vigilante8_vehicle_tools.__file__).resolve().is_relative_to(
            ADDONS
        ),
        "smoke test loaded the add-on from outside the requested root",
    )
    for required in (V8_COMMON, V82_COMMON, V8_EXE, V82_EXE):
        require(required.is_file(), f"required retail input is missing: {required}")
    requested_game = os.environ.get("V8_SMOKE_GAME")
    requested_index = os.environ.get("V8_SMOKE_INDEX")
    requested_result = os.environ.get("V8_SMOKE_RESULT")
    if requested_game is not None:
        require(
            requested_game in {"V8", "V8_2"},
            f"invalid V8_SMOKE_GAME: {requested_game}",
        )
        require(
            requested_index is not None and requested_result is not None,
            "child smoke run requires index and result path",
        )
        vigilante8_vehicle_tools.register()
        result = run_vehicle(requested_game, int(requested_index))
        Path(requested_result).write_text(
            json.dumps(result, indent=2) + "\n", encoding="utf-8"
        )
        print(
            "PASS",
            requested_game,
            requested_index,
            f"{len(result['variants'])} variants",
        )
        return

    jobs = tuple(
        (game, vehicle_index)
        for game, count in (
        ("V8", stats.PROFILES["V8"].record_count),
        ("V8_2", stats.PROFILES["V8_2"].record_count),
        )
        for vehicle_index in range(count)
    )
    write_manifest([], result="IN_PROGRESS")
    results: list[dict[str, object]] = []
    with tempfile.TemporaryDirectory() as directory:
        result_directory = Path(directory)
        for game, vehicle_index in jobs:
            result_path = (
                result_directory
                / f"{game.lower()}_{vehicle_index:02d}.json"
            )
            environment = os.environ.copy()
            environment["V8_SMOKE_GAME"] = game
            environment["V8_SMOKE_INDEX"] = str(vehicle_index)
            environment["V8_SMOKE_RESULT"] = str(result_path)
            subprocess.run(
                [
                    bpy.app.binary_path,
                    "--background",
                    "--factory-startup",
                    "--python",
                    str(Path(__file__).resolve()),
                ],
                check=True,
                env=environment,
            )
            result = json.loads(
                result_path.read_text(encoding="utf-8")
            )
            results.append(result)
            print(
                "PASS",
                game,
                vehicle_index,
                f"{len(result['variants'])} variants",
            )
    write_manifest(results)
    print(
        "All-retail vehicle/variant Blender smoke: PASS "
        f"({len(results)} vehicles, "
        f"{sum(len(item['variants']) for item in results)} variants)"
    )


if __name__ == "__main__":
    main()
