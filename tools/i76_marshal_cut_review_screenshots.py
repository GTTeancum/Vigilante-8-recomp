#!/usr/bin/env python3
"""Render temporary review overlays for the Marshal window cuts."""

from __future__ import annotations

import json
import math
import sys
from collections import defaultdict
from pathlib import Path

import bpy
from mathutils import Vector


ROOT = Path(__file__).resolve().parents[1]
BLEND = ROOT / "I76" / "NP_ransom_marshal.blend"
OUT = ROOT / "artifacts" / "i76_atlas_ao" / "marshal_cut_review"


REGIONS = [
    {
        "name": "right_side_glass_candidates",
        "object": "RM11BDYM",
        "predicate": lambda p, n: p.x > 0.60 and -0.86 <= p.y <= 0.86 and 0.84 <= p.z <= 1.32 and n.x > 0.40,
    },
    {
        "name": "left_side_glass_candidates",
        "object": "RM11BDYM",
        "predicate": lambda p, n: p.x < -0.60 and -0.86 <= p.y <= 0.86 and 0.84 <= p.z <= 1.32 and n.x < -0.40,
    },
    {
        "name": "front_windshield_candidates",
        "object": "RM11BDYT",
        "predicate": lambda p, n: -0.90 <= p.x <= 0.90 and 0.55 <= p.y <= 1.55 and 0.82 <= p.z <= 1.34,
    },
    {
        "name": "rear_windshield_candidates",
        "object": "RM11BDYB",
        "predicate": lambda p, n: -0.90 <= p.x <= 0.90 and -1.95 <= p.y <= -0.62 and 0.82 <= p.z <= 1.34,
    },
]


def material(name: str, color: tuple[float, float, float, float]) -> bpy.types.Material:
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = color
    mat.use_nodes = True
    mat.blend_method = "BLEND"
    mat.use_screen_refraction = False
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = color
        bsdf.inputs["Alpha"].default_value = color[3]
    return mat


def make_collection() -> bpy.types.Collection:
    coll = bpy.data.collections.new("I76 Marshal Cut Review TEMP")
    bpy.context.scene.collection.children.link(coll)
    return coll


def link_only(obj: bpy.types.Object, coll: bpy.types.Collection) -> None:
    for existing in list(obj.users_collection):
        existing.objects.unlink(obj)
    coll.objects.link(obj)


def face_center_normal(obj: bpy.types.Object, poly: bpy.types.MeshPolygon) -> tuple[Vector, Vector]:
    center = obj.matrix_world @ poly.center
    normal = (obj.matrix_world.to_3x3() @ poly.normal).normalized()
    return center, normal


def atlas_image() -> bpy.types.Image | None:
    for image in bpy.data.images:
        if image.get("i76_atlas"):
            return image
    for image in bpy.data.images:
        if "atlas" in image.name.lower():
            return image
    return None


def sample_image(image: bpy.types.Image, uv: Vector) -> tuple[float, float, float, float]:
    width, height = image.size
    x = max(0, min(width - 1, int(uv.x * (width - 1))))
    y = max(0, min(height - 1, int(uv.y * (height - 1))))
    index = (y * width + x) * 4
    pixels = image.pixels
    return pixels[index], pixels[index + 1], pixels[index + 2], pixels[index + 3]


def polygon_uv_center(obj: bpy.types.Object, poly: bpy.types.MeshPolygon) -> Vector | None:
    uv_layer = obj.data.uv_layers.active
    if uv_layer is None:
        return None
    uv = Vector((0.0, 0.0))
    count = 0
    for loop_index in poly.loop_indices:
        item = uv_layer.data[loop_index].uv
        uv += Vector((item.x, item.y))
        count += 1
    return uv / max(count, 1)


def is_glass_textured(obj: bpy.types.Object, poly: bpy.types.MeshPolygon, image: bpy.types.Image | None) -> bool:
    if image is None:
        return True
    uv = polygon_uv_center(obj, poly)
    if uv is None:
        return True
    r, g, b, a = sample_image(image, uv)
    return a > 0.1 and b > 0.12 and b > r + 0.045 and b >= g - 0.02 and r < 0.45


def selected_polygons(obj: bpy.types.Object, predicate, image: bpy.types.Image | None) -> list[bpy.types.MeshPolygon]:
    return [
        poly
        for poly in obj.data.polygons
        if predicate(*face_center_normal(obj, poly)) and is_glass_textured(obj, poly, image)
    ]


def create_face_overlay(
    source: bpy.types.Object,
    polygons: list[bpy.types.MeshPolygon],
    name: str,
    mat: bpy.types.Material,
    coll: bpy.types.Collection,
) -> bpy.types.Object | None:
    if not polygons:
        return None

    verts: list[Vector] = []
    faces: list[list[int]] = []
    for poly in polygons:
        center, normal = face_center_normal(source, poly)
        face_indices = []
        for vertex_index in poly.vertices:
            world = source.matrix_world @ source.data.vertices[vertex_index].co
            face_indices.append(len(verts))
            verts.append(world + normal * 0.006)
        faces.append(face_indices)

    mesh = bpy.data.meshes.new(name + "Mesh")
    mesh.from_pydata([tuple(v) for v in verts], [], faces)
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    obj.data.materials.append(mat)
    coll.objects.link(obj)
    return obj


def create_boundary_curves(
    source: bpy.types.Object,
    polygons: list[bpy.types.MeshPolygon],
    name: str,
    mat: bpy.types.Material,
    coll: bpy.types.Collection,
) -> bpy.types.Object | None:
    if not polygons:
        return None
    selected = {poly.index for poly in polygons}
    edge_to_polys: dict[tuple[int, int], list[int]] = defaultdict(list)
    for poly in source.data.polygons:
        verts = list(poly.vertices)
        for index, start in enumerate(verts):
            end = verts[(index + 1) % len(verts)]
            edge_to_polys[tuple(sorted((start, end)))].append(poly.index)

    curve = bpy.data.curves.new(name, "CURVE")
    curve.dimensions = "3D"
    curve.bevel_depth = 0.004
    curve.bevel_resolution = 0
    count = 0
    for (a, b), owners in edge_to_polys.items():
        selected_owners = [owner for owner in owners if owner in selected]
        if len(selected_owners) == 1:
            pa = source.matrix_world @ source.data.vertices[a].co
            pb = source.matrix_world @ source.data.vertices[b].co
            normal = Vector((0.0, 0.0, 0.0))
            for owner in selected_owners:
                normal += (source.matrix_world.to_3x3() @ source.data.polygons[owner].normal).normalized()
            normal.normalize()
            spline = curve.splines.new("POLY")
            spline.points.add(1)
            for point, value in zip(spline.points, (pa + normal * 0.014, pb + normal * 0.014)):
                point.co = (value.x, value.y, value.z, 1.0)
            count += 1
    if count == 0:
        bpy.data.curves.remove(curve)
        return None
    obj = bpy.data.objects.new(name, curve)
    obj.data.materials.append(mat)
    coll.objects.link(obj)
    return obj


def make_wire_overlay(mat: bpy.types.Material, coll: bpy.types.Collection) -> list[bpy.types.Object]:
    objects = []
    for source in bpy.context.scene.objects:
        if source.type != "MESH" or not source.name.startswith("RM11BDY"):
            continue
        duplicate = source.copy()
        duplicate.data = source.data.copy()
        duplicate.name = "I76_Marshal_CutReview_Wire_" + source.name
        duplicate.data.materials.clear()
        duplicate.data.materials.append(mat)
        bpy.context.scene.collection.objects.link(duplicate)
        link_only(duplicate, coll)
        mod = duplicate.modifiers.new("Black cut review wire", "WIREFRAME")
        mod.thickness = 0.0025
        mod.use_even_offset = True
        mod.use_replace = True
        objects.append(duplicate)
    return objects


def look_at(obj: bpy.types.Object, target: Vector) -> None:
    obj.rotation_euler = (target - obj.location).to_track_quat("-Z", "Y").to_euler()


def bounds_center_radius() -> tuple[Vector, float]:
    corners = [
        obj.matrix_world @ Vector(corner)
        for obj in bpy.context.scene.objects
        if obj.type == "MESH" and obj.visible_get()
        for corner in obj.bound_box
    ]
    minimum = Vector(tuple(min(point[axis] for point in corners) for axis in range(3)))
    maximum = Vector(tuple(max(point[axis] for point in corners) for axis in range(3)))
    center = (minimum + maximum) * 0.5
    radius = max((maximum - minimum).length * 0.5, 1.0)
    return center, radius


def render(path: Path, view: str) -> None:
    center, radius = bounds_center_radius()
    camera_data = bpy.data.cameras.new("Marshal Cut Review Camera")
    camera = bpy.data.objects.new("Marshal Cut Review Camera", camera_data)
    bpy.context.scene.collection.objects.link(camera)
    camera.data.type = "ORTHO"

    if view == "right_side":
        camera.location = center + Vector((radius * 2.0, 0.0, radius * 0.03))
        camera.data.ortho_scale = radius * 1.08
        look_at(camera, center + Vector((0.0, 0.0, radius * 0.03)))
    elif view == "top":
        camera.location = center + Vector((0.0, 0.0, radius * 2.0))
        camera.data.ortho_scale = radius * 1.02
        look_at(camera, center)
    else:
        camera.location = center + Vector((-radius * 1.05, -radius * 1.75, radius * 0.72))
        camera.data.type = "PERSP"
        camera.data.lens = 88
        look_at(camera, center + Vector((0.0, -radius * 0.25, radius * 0.08)))

    scene = bpy.context.scene
    scene.camera = camera
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.display.shading.light = "STUDIO"
    scene.display.shading.color_type = "TEXTURE"
    scene.display.shading.show_shadows = False
    scene.display.shading.show_cavity = False
    scene.render.resolution_x = 1400
    scene.render.resolution_y = 950
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    path.parent.mkdir(parents=True, exist_ok=True)
    scene.render.filepath = str(path)
    bpy.ops.render.render(write_still=True)
    bpy.data.objects.remove(camera, do_unlink=True)
    bpy.data.cameras.remove(camera_data)


def main() -> int:
    bpy.ops.wm.open_mainfile(filepath=str(BLEND))
    for obj in bpy.data.objects:
        if obj.type == "CURVE" and obj.name.startswith("I76_Marshal_WindowGuide"):
            obj.hide_render = True
            obj.hide_viewport = True

    coll = make_collection()
    fill_mat = material("I76 Cut Review Candidate Glass Blue", (0.0, 0.55, 1.0, 0.38))
    edge_mat = material("I76 Cut Review Boundary Yellow", (1.0, 0.85, 0.0, 1.0))
    wire_mat = material("I76 Cut Review Wire Black", (0.0, 0.0, 0.0, 1.0))
    make_wire_overlay(wire_mat, coll)

    summary = []
    image = atlas_image()
    for region in REGIONS:
        source = bpy.data.objects[region["object"]]
        polys = selected_polygons(source, region["predicate"], image)
        create_face_overlay(source, polys, "I76_CutReview_Fill_" + region["name"], fill_mat, coll)
        create_boundary_curves(source, polys, "I76_CutReview_Boundary_" + region["name"], edge_mat, coll)
        summary.append({"region": region["name"], "object": source.name, "faces": len(polys)})

    OUT.mkdir(parents=True, exist_ok=True)
    render(OUT / "marshal_cut_understanding_side.png", "right_side")
    render(OUT / "marshal_cut_understanding_top.png", "top")
    render(OUT / "marshal_cut_understanding_rear.png", "rear")
    (OUT / "marshal_cut_review_summary.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(summary, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
