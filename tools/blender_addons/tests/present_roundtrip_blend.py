"""Prepare a saved retail round-trip scene for interactive visual inspection."""

from __future__ import annotations

import math

import bpy


def frame_scene() -> None:
    window = bpy.context.window_manager.windows[0]
    area = next(area for area in window.screen.areas if area.type == "VIEW_3D")
    region = next(region for region in area.regions if region.type == "WINDOW")
    space = area.spaces.active
    space.shading.type = "MATERIAL"
    space.overlay.show_relationship_lines = False
    space.overlay.show_extras = False
    meshes = [
        obj
        for obj in bpy.context.scene.objects
        if obj.type == "MESH" and obj.get("v8_role") == "render_group"
    ]
    for obj in bpy.context.scene.objects:
        obj.select_set(obj in meshes)
    if not meshes:
        raise RuntimeError("round-trip scene has no imported retail meshes")
    bpy.context.view_layer.objects.active = meshes[0]
    with bpy.context.temp_override(window=window, area=area, region=region):
        bpy.ops.view3d.view_selected(use_all_regions=False)
        bpy.ops.view3d.view_axis(type="FRONT", align_active=False)
        bpy.ops.view3d.view_orbit(type="ORBITUP", angle=math.radians(18))
        bpy.ops.view3d.view_orbit(type="ORBITLEFT", angle=math.radians(25))
        if space.region_3d.view_perspective != "PERSP":
            bpy.ops.view3d.view_persportho()
        space.region_3d.view_distance *= 0.72
    for obj in meshes:
        obj.select_set(False)
    meshes[0].select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]


bpy.app.timers.register(frame_scene, first_interval=1.0)
