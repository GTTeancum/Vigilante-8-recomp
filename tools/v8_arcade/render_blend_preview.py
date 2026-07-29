#!/usr/bin/env python3
"""Render a perspective validation image from an extracted vehicle .blend."""

from __future__ import annotations

import argparse
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector


def look_at(obj: bpy.types.Object, target: Vector) -> None:
    obj.rotation_euler = (target - obj.location).to_track_quat("-Z", "Y").to_euler()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("blend", type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--collection", default="VEHICLE_ASSEMBLY")
    parser.add_argument(
        "--model-offset",
        type=lambda value: int(value, 0),
        help="render only mesh objects carrying this source Model3 offset",
    )
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None
    args = parser.parse_args(argv)

    bpy.ops.wm.open_mainfile(filepath=str(args.blend.resolve()))
    target_collection = bpy.data.collections.get(args.collection)
    if target_collection is not None:
        target_objects = set(target_collection.all_objects)
        for collection in bpy.data.collections:
            collection.hide_viewport = False
            collection.hide_render = False
        for obj in bpy.context.scene.objects:
            if obj.type != "MESH":
                continue
            enabled = obj in target_objects
            obj.hide_set(not enabled)
            obj.hide_viewport = not enabled
            obj.hide_render = not enabled
    if args.model_offset is not None:
        for obj in bpy.context.scene.objects:
            if obj.type != "MESH":
                continue
            enabled = int(obj.get("source_model_offset", -1)) == args.model_offset
            obj.hide_viewport = not enabled
            obj.hide_render = not enabled

    objects = [
        obj
        for obj in bpy.context.scene.objects
        if obj.type == "MESH" and not obj.hide_render and obj.visible_get()
    ]
    if not objects:
        raise RuntimeError(f"no visible meshes in collection {args.collection}")
    corners = [
        obj.matrix_world @ Vector(corner) for obj in objects for corner in obj.bound_box
    ]
    minimum = Vector(tuple(min(point[axis] for point in corners) for axis in range(3)))
    maximum = Vector(tuple(max(point[axis] for point in corners) for axis in range(3)))
    center = (minimum + maximum) * 0.5
    radius = max((maximum - minimum).length * 0.5, 1.0)

    camera_data = bpy.data.cameras.new("PreviewCamera")
    camera = bpy.data.objects.new("PreviewCamera", camera_data)
    bpy.context.scene.collection.objects.link(camera)
    camera.location = center + Vector((radius * 1.25, -radius * 1.7, radius * 0.85))
    camera_data.lens = 62
    look_at(camera, center)
    bpy.context.scene.camera = camera

    scene = bpy.context.scene
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.display.shading.light = "STUDIO"
    scene.display.shading.color_type = "TEXTURE"
    scene.display.shading.show_shadows = False
    scene.display.shading.show_cavity = False
    scene.display.shading.show_specular_highlight = False
    scene.render.resolution_x = 960
    scene.render.resolution_y = 640
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = True
    scene.render.filepath = str(args.out.resolve())
    scene.render.image_settings.color_mode = "RGBA"
    bpy.ops.render.render(write_still=True)
    print(f"Rendered {args.collection} -> {args.out.resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
