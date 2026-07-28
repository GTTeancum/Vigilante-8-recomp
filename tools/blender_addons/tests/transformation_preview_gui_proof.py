"""Save concise Standard/Hover/Float/Ski Blender preview scenes."""

from __future__ import annotations

from pathlib import Path
import sys

import bpy


ROOT = Path(__file__).resolve().parents[3]
ADDONS = ROOT / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

import vigilante8_vehicle_tools


SOURCE = (
    ROOT
    / "artifacts"
    / "vehicle_plugin_proofs"
    / "v82_clean_authoring_view.blend"
)
OUTPUT = ROOT / "artifacts" / "vehicle_plugin_proofs"


def main() -> None:
    vigilante8_vehicle_tools.register()
    bpy.ops.wm.open_mainfile(filepath=str(SOURCE))
    settings = bpy.context.scene.v8_vehicle_settings
    for mode in ("STANDARD", "HOVER", "FLOAT", "SKI"):
        settings.transformation_preview_mode = mode
        bpy.context.view_layer.update()
        bpy.ops.wm.save_as_mainfile(
            filepath=str(OUTPUT / f"v82_{mode.lower()}_preview.blend")
        )


if __name__ == "__main__":
    main()
