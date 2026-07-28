"""Open the decoded V8:2 vehicle in the default clean authoring view."""

from __future__ import annotations

import os
import math
from pathlib import Path
import sys

import bpy
from mathutils import Quaternion


ROOT = Path(__file__).resolve().parents[3]
ADDONS = Path(
    os.environ.get("V8_ADDON_ROOT", ROOT / "tools" / "blender_addons")
).resolve()
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

import vigilante8_vehicle_tools
from vigilante8_vehicle_tools import authored_scene, registry


PACKAGE = ROOT / "artifacts" / "dual_game_default_roundtrip" / "source_free_v82"
OUTPUT = (
    ROOT
    / "artifacts"
    / "vehicle_plugin_proofs"
    / "v82_clean_authoring_view.blend"
)


def configure_window() -> None:
    window = bpy.context.window_manager.windows[0]
    visible_meshes = [
        obj
        for obj in bpy.context.scene.objects
        if obj.type == "MESH" and not obj.hide_get()
    ]
    bpy.ops.object.select_all(action="DESELECT")
    for obj in visible_meshes:
        obj.select_set(True)
    if visible_meshes:
        bpy.context.view_layer.objects.active = visible_meshes[0]

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
            region_3d.view_distance *= 0.78
        elif area.type == "PROPERTIES":
            area.spaces.active.context = "SCENE"

    for obj in visible_meshes:
        obj.select_set(False)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUTPUT.resolve()))


def main() -> None:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    vigilante8_vehicle_tools.register()
    vehicle = registry.decompile_package(
        (PACKAGE / "CUSTOM.EXP").read_bytes(),
        (PACKAGE / "VEHICLES.V8R").read_bytes(),
    )[0]
    authored_scene.project_to_scene(bpy.context, vehicle)
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    configure_window()


if __name__ == "__main__":
    main()
