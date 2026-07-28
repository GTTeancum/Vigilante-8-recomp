"""Import, save/reopen, and export the three converted guests in Blender."""

from __future__ import annotations

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


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


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


def main() -> None:
    addon.register()
    originals = registry.decompile_package(
        (SOURCE / "CUSTOM.EXP").read_bytes(),
        (SOURCE / "VEHICLES.V8R").read_bytes(),
    )
    BLENDS.mkdir(parents=True, exist_ok=True)
    exported = []
    records = []

    for original in originals:
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
        if project.to_dict(rebuilt) != project.to_dict(original):
            raise AssertionError(
                f"{original.stable_id} changed during Blender save/reopen/export"
            )
        exported.append(rebuilt)
        records.append(
            {
                "stable_id": rebuilt.stable_id,
                "blend": str(blend.relative_to(ROOT)),
                "blend_sha256": digest(blend.read_bytes()),
                "body_slots": len(rebuilt.slots),
                "body_groups": len(rebuilt.groups),
                "body_textures": len(rebuilt.textures),
                "transform_slots": len(rebuilt.transformation_bank.slots),
                "transform_groups": len(rebuilt.transformation_bank.groups),
            }
        )

    package = registry.compile_package(exported)
    reparsed = registry.decompile_package(package.archive, package.registry)
    if [project.to_dict(item) for item in reparsed] != [
        project.to_dict(item) for item in exported
    ]:
        raise AssertionError(
            "Blender-exported three-entry package changed on semantic decode"
        )
    FINAL.mkdir(parents=True, exist_ok=True)
    (FINAL / "CUSTOM.EXP").write_bytes(package.archive)
    (FINAL / "VEHICLES.V8R").write_bytes(package.registry)
    proof = {
        "blender_version": bpy.app.version_string,
        "material_preview_default": True,
        "perspective_default": True,
        "vehicles": records,
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
        f"Blender {bpy.app.version_string}: exported {len(exported)} guests; "
        f"CUSTOM.EXP={digest(package.archive)} "
        f"VEHICLES.V8R={digest(package.registry)}"
    )


if __name__ == "__main__":
    main()
