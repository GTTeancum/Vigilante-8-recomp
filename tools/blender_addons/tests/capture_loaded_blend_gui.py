"""Capture a deterministic perspective/material-preview Blender GUI proof."""

from __future__ import annotations

from pathlib import Path
import sys

import bpy
from mathutils import Quaternion, Vector


def output_path() -> Path:
    arguments = sys.argv
    values = arguments[arguments.index("--") + 1 :] if "--" in arguments else []
    if len(values) != 1:
        raise ValueError("expected one screenshot output path after --")
    result = Path(values[0]).resolve()
    result.parent.mkdir(parents=True, exist_ok=True)
    return result


OUTPUT = output_path()


def prepare_view() -> None:
    window = bpy.context.window_manager.windows[0]
    area = next(item for item in window.screen.areas if item.type == "VIEW_3D")
    space = area.spaces.active
    space.shading.type = "MATERIAL"
    space.overlay.show_extras = False
    space.overlay.show_relationship_lines = False
    region_3d = space.region_3d
    # Fixed three-quarter user perspective used by every mode.  Keeping this
    # independent of selection prevents hidden transformation-library objects
    # from changing the proof framing.
    region_3d.view_perspective = "PERSP"
    region_3d.view_rotation = Quaternion(
        (0.46412283182144165, 0.28735819458961487,
         -0.4410620331764221, -0.7123757600784302)
    )
    region_3d.view_location = Vector(
        (0.06697729229927063, 0.0020751953125, 0.25458526611328125)
    )
    region_3d.view_distance = 2.35
    bpy.ops.object.select_all(action="DESELECT")
    for item in window.screen.areas:
        if item.type == "PROPERTIES":
            item.spaces.active.context = "SCENE"


def capture() -> None:
    bpy.ops.screen.screenshot(filepath=str(OUTPUT))
    print(f"Captured Blender GUI proof: {OUTPUT}")


def finish() -> None:
    bpy.ops.wm.quit_blender()


bpy.app.timers.register(prepare_view, first_interval=1.0)
bpy.app.timers.register(capture, first_interval=12.0)
bpy.app.timers.register(finish, first_interval=13.0)
