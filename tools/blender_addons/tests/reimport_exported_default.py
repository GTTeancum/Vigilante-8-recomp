"""Reimport a Blender-exported COMMON.EXP entry and save the proof scene."""

from __future__ import annotations

from pathlib import Path
import sys

import bpy


ROOT = Path(__file__).resolve().parents[3]
ADDONS = ROOT / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

import vigilante8_vehicle_tools
from vigilante8_vehicle_tools import blender_ui


values = sys.argv[sys.argv.index("--") + 1 :]
source = Path(values[0]).resolve()
blend = Path(values[1]).resolve()
bpy.ops.wm.read_factory_settings(use_empty=True)
vigilante8_vehicle_tools.register()
blender_ui.import_archive_into_scene(
    bpy.context, str(source), 0, False, True
)
bpy.context.scene["roundtrip_stage"] = "Blender export reimported"
bpy.context.scene["native_export"] = str(source)
blend.parent.mkdir(parents=True, exist_ok=True)
bpy.ops.wm.save_as_mainfile(filepath=str(blend))
