"""Open a current V8:2 vehicle in Blender with semantic hierarchy and stats UI."""

from __future__ import annotations

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


PACKAGE = (
    ROOT
    / "artifacts"
    / "dual_game_default_roundtrip"
    / "source_free_v82"
)
OUTPUT = (
    ROOT
    / "artifacts"
    / "vehicle_plugin_proofs"
    / "v82_semantic_hierarchy_and_stats.blend"
)


def load_vehicle():
    return registry.decompile_package(
        (PACKAGE / "CUSTOM.EXP").read_bytes(),
        (PACKAGE / "VEHICLES.V8R").read_bytes(),
    )[0]


def configure_view() -> None:
    window = bpy.context.window_manager.windows[0]
    screen = window.screen
    proof_area = max(
        (area for area in screen.areas if area.type == "VIEW_3D"),
        key=lambda area: area.width * area.height,
    )
    for area in screen.areas:
        if area != proof_area and area.type == "PROPERTIES":
            area.type = "OUTLINER"
    proof_area.type = "PROPERTIES"
    proof_area.spaces.active.context = "SCENE"
    proof_area.spaces.active.search_filter = "Vehicle Stats"

    for area in screen.areas:
        if area.type == "PROPERTIES":
            area.spaces.active.context = "SCENE"
            area.spaces.active.search_filter = "Vehicle Stats"
        elif area.type == "OUTLINER":
            with bpy.context.temp_override(window=window, area=area):
                bpy.ops.outliner.show_hierarchy()
                for _level in range(4):
                    bpy.ops.outliner.show_one_level(open=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUTPUT.resolve()))


def main() -> None:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    vigilante8_vehicle_tools.register()
    collection = authored_scene.project_to_scene(bpy.context, load_vehicle())
    bpy.context.scene.v8_vehicle_settings.vehicle_collection = collection.name
    collection["proof_stats_storage"] = (
        "VEHICLES.V8R exact 0x30-byte native record"
    )
    collection["proof_retail_table"] = (
        "SLUS_008.68 address 0x80063A80, file offset 0x54280"
    )
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUTPUT.resolve()))
    bpy.app.timers.register(lambda: configure_view(), first_interval=1.0)


if __name__ == "__main__":
    main()
