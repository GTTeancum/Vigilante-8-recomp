"""Open an original/native-round-trip comparison in Blender's GUI."""

from __future__ import annotations

import argparse
import math
import os
from pathlib import Path
import sys

import bpy


ROOT = Path(__file__).resolve().parents[3]
ADDONS = Path(
    os.environ.get("V8_ADDON_ROOT", ROOT / "tools" / "blender_addons")
).resolve()
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

import vigilante8_vehicle_tools
from vigilante8_vehicle_tools import authored_scene, registry


def parse_args() -> argparse.Namespace:
    arguments = sys.argv
    arguments = (
        arguments[arguments.index("--") + 1 :] if "--" in arguments else []
    )
    parser = argparse.ArgumentParser()
    parser.add_argument("original", type=Path)
    parser.add_argument("round_trip", type=Path)
    parser.add_argument("blend", type=Path)
    parser.add_argument("screenshot", type=Path)
    return parser.parse_args(arguments)


def load_project(directory: Path):
    return registry.decompile_package(
        (directory / "CUSTOM.EXP").read_bytes(),
        (directory / "VEHICLES.V8R").read_bytes(),
    )[0]


def move_collection(collection: bpy.types.Collection, x: float) -> None:
    for obj in collection.all_objects:
        if obj is None:
            continue
        if obj.parent is None:
            obj.location.x += x
        if (
            obj.type == "MESH"
            and not (
                obj.get("v8_role")
                in {"authored_slot", "authored_render_group"}
                and "v8_group_index" in obj
            )
        ):
            obj.hide_viewport = True
            obj.hide_render = True
    for child in collection.children:
        if child.get("v8_bank") == "transformation":
            child.hide_viewport = True
            child.hide_render = True


def make_label(text: str, x: float) -> bpy.types.Object:
    curve = bpy.data.curves.new(f"{text}.font", "FONT")
    curve.body = text
    curve.align_x = "CENTER"
    curve.align_y = "CENTER"
    curve.size = 0.12
    curve.extrude = 0.005
    obj = bpy.data.objects.new(text, curve)
    obj.location = (x, 0.05, 0.70)
    obj.rotation_euler.x = math.radians(90)
    bpy.context.scene.collection.objects.link(obj)
    return obj


def configure_scene(args: argparse.Namespace) -> None:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    vigilante8_vehicle_tools.register()

    original = authored_scene.project_to_scene(
        bpy.context, load_project(args.original.resolve())
    )
    original.name = "ORIGINAL — native game vehicle"
    move_collection(original, -0.75)

    rebuilt = authored_scene.project_to_scene(
        bpy.context, load_project(args.round_trip.resolve())
    )
    rebuilt.name = "BLENDER EXPORT + NATIVE REIMPORT — edited"
    move_collection(rebuilt, 0.75)
    bpy.context.scene.v8_vehicle_settings.vehicle_collection = rebuilt.name

    make_label("ORIGINAL GAME", -0.75)
    make_label("BLENDER EXPORT + REIMPORT", 0.75)

    bpy.context.scene["comparison_result"] = (
        "Independent native source and visibly edited Blender export"
    )
    bpy.context.scene["comparison_left"] = str(args.original.resolve())
    bpy.context.scene["comparison_right"] = str(args.round_trip.resolve())
    args.blend.resolve().parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(args.blend.resolve()))


def schedule_screenshot(path: Path) -> None:
    output = path.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)

    def frame_view() -> float:
        window = bpy.context.window_manager.windows[0]
        area = next(
            area for area in window.screen.areas if area.type == "VIEW_3D"
        )
        region = next(
            region for region in area.regions if region.type == "WINDOW"
        )
        space = area.spaces.active
        space.shading.type = "MATERIAL"
        space.overlay.show_extras = False
        space.overlay.show_relationship_lines = False
        for obj in bpy.context.scene.objects:
            obj.select_set(
                obj.type == "FONT"
                or (
                    obj.type == "MESH"
                    and obj.get("v8_role")
                    in {"authored_slot", "authored_render_group"}
                    and "v8_group_index" in obj
                )
            )
        with bpy.context.temp_override(
            window=window, area=area, region=region
        ):
            bpy.ops.view3d.view_selected(use_all_regions=False)
            bpy.ops.view3d.view_axis(type="FRONT", align_active=False)
            bpy.ops.view3d.view_orbit(
                type="ORBITUP", angle=math.radians(18)
            )
            bpy.ops.view3d.view_orbit(
                type="ORBITLEFT", angle=math.radians(24)
            )
            if space.region_3d.view_perspective != "PERSP":
                bpy.ops.view3d.view_persportho()
            space.region_3d.view_distance *= 0.78
            bpy.ops.object.select_all(action="DESELECT")
            selected_collection = bpy.data.collections[
                bpy.context.scene.v8_vehicle_settings.vehicle_collection
            ]
            active = next(
                obj for obj in bpy.context.scene.objects
                if obj.type == "MESH"
                and obj in selected_collection.all_objects
                and "v8_group_index" in obj
            )
            active.select_set(True)
            bpy.context.view_layer.objects.active = active
        return None

    bpy.app.timers.register(frame_view, first_interval=1.0)


def main() -> None:
    args = parse_args()
    configure_scene(args)
    schedule_screenshot(args.screenshot)


if __name__ == "__main__":
    main()
