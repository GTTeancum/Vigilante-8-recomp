"""Import, save/reopen, and export all converted guests in Blender."""

from __future__ import annotations

from dataclasses import replace
import hashlib
import json
from pathlib import Path
import sys

import bpy


ROOT = Path(__file__).resolve().parents[3]
ADDONS = ROOT / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

import vigilante8_vehicle_tools as addon  # noqa: E402
from vigilante8_vehicle_tools import (  # noqa: E402
    authored_scene,
    project,
    registry,
)


OUTPUT = ROOT / "artifacts" / "v8_to_v82_guest_roster"
SOURCE = OUTPUT / "pre_blender"
FINAL = OUTPUT / "final"
BLENDS = OUTPUT / "blender"
EXPORTED = OUTPUT / "blender_exported"


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def first_difference(expected, actual, path: str = "project") -> str:
    if type(expected) is not type(actual):
        return (
            f"{path}: type {type(expected).__name__} != "
            f"{type(actual).__name__}"
        )
    if isinstance(expected, dict):
        if expected.keys() != actual.keys():
            return (
                f"{path}: keys {sorted(expected.keys())} != "
                f"{sorted(actual.keys())}"
            )
        for key in expected:
            if expected[key] != actual[key]:
                return first_difference(
                    expected[key],
                    actual[key],
                    f"{path}.{key}",
                )
    elif isinstance(expected, list):
        if len(expected) != len(actual):
            return f"{path}: length {len(expected)} != {len(actual)}"
        for index, (expected_item, actual_item) in enumerate(
            zip(expected, actual)
        ):
            if expected_item != actual_item:
                return first_difference(
                    expected_item,
                    actual_item,
                    f"{path}[{index}]",
                )
    return f"{path}: {expected!r} != {actual!r}"


def set_authoring_view() -> None:
    for screen in bpy.data.screens:
        for area in screen.areas:
            if area.type != "VIEW_3D":
                continue
            area.spaces.active.shading.type = "MATERIAL"
            area.spaces.active.region_3d.view_perspective = "PERSP"


def authored_collection() -> bpy.types.Collection:
    matches = [
        collection
        for collection in bpy.data.collections
        if collection.get("v8_role") == authored_scene.ROLE_VEHICLE
    ]
    if len(matches) != 1:
        raise AssertionError(
            f"expected one authored vehicle collection, found {len(matches)}"
        )
    return matches[0]


def record_for(vehicle: project.VehicleProject) -> dict:
    blend = BLENDS / f"{vehicle.stable_id}.blend"
    return {
        "stable_id": vehicle.stable_id,
        "blend": str(blend.relative_to(ROOT)),
        "blend_sha256": digest(blend.read_bytes()),
        "body_slots": len(vehicle.slots),
        "body_groups": len(vehicle.groups),
        "body_textures": len(vehicle.textures),
        "transform_slots": (
            len(vehicle.transformation_bank.slots)
            if vehicle.transformation_bank is not None
            else 0
        ),
        "transform_groups": (
            len(vehicle.transformation_bank.groups)
            if vehicle.transformation_bank is not None
            else 0
        ),
        "controller_class": vehicle.controller_class,
        "supports_transformations": vehicle.supports_transformations,
    }


def finalize(originals: tuple[project.VehicleProject, ...]) -> None:
    exported = tuple(
        project.VehicleProject.from_dict(
            json.loads(
                (EXPORTED / f"{original.stable_id}.json").read_text(
                    encoding="utf-8"
                )
            )
        )
        for original in originals
    )
    package = registry.compile_package(exported)
    reparsed = registry.decompile_package(package.archive, package.registry)
    if [project.to_dict(item) for item in reparsed] != [
        project.to_dict(item) for item in exported
    ]:
        raise AssertionError(
            "Blender-exported package changed on semantic decode"
        )
    FINAL.mkdir(parents=True, exist_ok=True)
    (FINAL / "CUSTOM.EXP").write_bytes(package.archive)
    (FINAL / "VEHICLES.V8R").write_bytes(package.registry)
    proof = {
        "blender_version": bpy.app.version_string,
        "material_preview_default": True,
        "perspective_default": True,
        "vehicles": [record_for(vehicle) for vehicle in exported],
        "final_package": {
            "CUSTOM.EXP": digest(package.archive),
            "VEHICLES.V8R": digest(package.registry),
        },
        "byte_exact_to_pre_blender": (
            package.archive == (SOURCE / "CUSTOM.EXP").read_bytes()
            and package.registry == (SOURCE / "VEHICLES.V8R").read_bytes()
        ),
    }
    (OUTPUT / "blender_roundtrip.json").write_text(
        json.dumps(proof, indent=2) + "\n",
        encoding="utf-8",
    )
    print(
        f"Blender {bpy.app.version_string}: finalized {len(exported)} guests; "
        f"CUSTOM.EXP={digest(package.archive)} "
        f"VEHICLES.V8R={digest(package.registry)}"
    )


def main() -> None:
    addon.register()
    originals = tuple(registry.decompile_package(
        (SOURCE / "CUSTOM.EXP").read_bytes(),
        (SOURCE / "VEHICLES.V8R").read_bytes(),
    ))
    separator = sys.argv.index("--") if "--" in sys.argv else len(sys.argv)
    arguments = sys.argv[separator + 1 :]
    if arguments == ["finalize"]:
        finalize(originals)
        return
    selected_indices = (
        tuple(int(argument) for argument in arguments)
        if arguments
        else tuple(range(len(originals)))
    )
    BLENDS.mkdir(parents=True, exist_ok=True)
    EXPORTED.mkdir(parents=True, exist_ok=True)
    exported = []

    for index in selected_indices:
        original = originals[index]
        bpy.ops.wm.read_factory_settings(use_empty=True)
        if not hasattr(bpy.context.scene, "v8_vehicle_settings"):
            addon.register()
        collection = authored_scene.project_to_scene(bpy.context, original)
        authored_scene.apply_authoring_visibility(
            collection,
            preview_state="INTACT",
            show_attachment_guides=False,
            show_advanced=False,
            show_transform_library=False,
        )
        set_authoring_view()
        blend = BLENDS / f"{original.stable_id}.blend"
        bpy.ops.wm.save_as_mainfile(filepath=str(blend))

        bpy.ops.wm.open_mainfile(filepath=str(blend))
        reopened = authored_collection()
        rebuilt = authored_scene.scene_to_project(reopened)
        # SND payloads are package data, not Blender-authored scene data.
        # Carry the source bank through the visual round trip just as the
        # compiler carries the other non-scene registry metadata.
        rebuilt = replace(
            rebuilt,
            sounds=original.sounds,
            special_behavior_type=original.special_behavior_type,
        )
        original_dict = project.to_dict(original)
        rebuilt_dict = project.to_dict(rebuilt)
        if rebuilt_dict != original_dict:
            raise AssertionError(
                f"{original.stable_id} changed during Blender save/reopen/export: "
                f"{first_difference(original_dict, rebuilt_dict)}"
            )
        exported.append(rebuilt)
        (EXPORTED / f"{rebuilt.stable_id}.json").write_text(
            json.dumps(project.to_dict(rebuilt), indent=2) + "\n",
            encoding="utf-8",
        )

    if len(selected_indices) == len(originals):
        finalize(originals)
        return
    print(
        f"Blender {bpy.app.version_string}: round-tripped indices "
        f"{','.join(str(index) for index in selected_indices)}"
    )


if __name__ == "__main__":
    main()
