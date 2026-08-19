#!/usr/bin/env python3
"""Add clean projected window guide curves to the I76 Marshal blend."""

from __future__ import annotations

import argparse
import json
import math
import shutil
import subprocess
import sys
from pathlib import Path

import bpy
from mathutils import Vector


ROOT = Path(__file__).resolve().parents[1]
BLEND = ROOT / "I76" / "NP_ransom_marshal.blend"
ARTIFACT_ROOT = ROOT / "artifacts" / "i76_atlas_ao"
SHOT_ROOT = ARTIFACT_ROOT / "marshal_window_guides"
IMAGE_OPS = ROOT / "tools" / "i76_atlas_image_ops.py"

GUIDE_PREFIX = "I76_Marshal_WindowGuide"
WIRE_PREFIX = "I76_Marshal_BlackWire"
GUIDE_COLLECTION = "I76 Marshal Window Guides"
WIRE_COLLECTION = "I76 Marshal Wire Overlay"

ATLAS_SIZE = 4096.0
TILES = {
    "rm11tp_1": {"x": 8.0, "y_bottom": 8.0, "w": 1024.0, "h": 1024.0},
    "rm11bkt1": {"x": 8.0, "y_bottom": 1040.0, "w": 1024.0, "h": 512.0},
    "rm11mdl1": {"x": 1040.0, "y_bottom": 1560.0, "w": 1024.0, "h": 512.0},
}

# Normalized texture-space controls traced from each window paint component.
# Coordinates are [0, 1] in the source texture crop.  Each guide uses four
# corner anchors plus one midpoint per side, then expands those controls into a
# clean 20-point border before projection back onto the mesh.
GUIDE_SPECS = [
    {
        "name": "front_windshield",
        "object": "RM11BDYT",
        "tile": "rm11tp_1",
        "normalized_controls": [
            (0.10100, 0.05800),
            (0.49700, 0.10800),
            (0.90700, 0.05500),
            (0.93000, 0.19800),
            (0.95300, 0.29800),
            (0.49700, 0.34100),
            (0.05500, 0.29400),
            (0.07800, 0.17600),
        ],
    },
    {
        "name": "rear_windshield",
        "object": "RM11BDYB",
        "tile": "rm11bkt1",
        "normalized_controls": [
            (0.09400, 0.11100),
            (0.48600, 0.07800),
            (0.88600, 0.11700),
            (0.86000, 0.21400),
            (0.83500, 0.31200),
            (0.49200, 0.29900),
            (0.14500, 0.30800),
            (0.12000, 0.20900),
        ],
    },
    {
        "name": "left_side_window_main",
        "object": "RM11BDYM",
        "tile": "rm11mdl1",
        "side": -1,
        "side_yz_controls": [
            (0.197, 1.240),
            (-0.263, 1.250),
            (-0.791, 1.050),
            (-0.791, 0.990),
            (-0.791, 0.930),
            (-0.263, 0.920),
            (0.197, 0.920),
            (0.197, 1.080),
        ],
        "normalized_controls": [
            (0.09500, 0.68700),
            (0.28700, 0.66900),
            (0.63800, 0.68700),
            (0.81900, 0.83100),
            (1.00000, 0.97600),
            (0.54700, 0.97600),
            (0.09500, 0.97600),
            (0.09500, 0.83100),
        ],
    },
    {
        "name": "left_side_window_quarter",
        "object": "RM11BDYM",
        "tile": "rm11mdl1",
        "side": -1,
        "side_yz_controls": [
            (0.767, 0.970),
            (0.482, 1.120),
            (0.197, 1.240),
            (0.197, 1.080),
            (0.197, 0.920),
            (0.482, 0.920),
            (0.767, 0.920),
            (0.767, 0.935),
        ],
        "normalized_controls": [
            (0.84706, 0.67717),
            (0.92353, 0.67323),
            (1.00000, 0.66929),
            (1.00000, 0.77165),
            (0.99216, 0.88976),
            (0.92353, 0.92126),
            (0.85490, 0.93701),
            (0.84706, 0.81102),
        ],
    },
    {
        "name": "right_side_window_main",
        "object": "RM11BDYM",
        "tile": "rm11mdl1",
        "side": 1,
        "side_yz_controls": [
            (0.197, 1.240),
            (-0.263, 1.250),
            (-0.791, 1.050),
            (-0.791, 0.990),
            (-0.791, 0.930),
            (-0.263, 0.920),
            (0.197, 0.920),
            (0.197, 1.080),
        ],
        "normalized_controls": [
            (0.09500, 0.68700),
            (0.28700, 0.66900),
            (0.63800, 0.68700),
            (0.81900, 0.83100),
            (1.00000, 0.97600),
            (0.54700, 0.97600),
            (0.09500, 0.97600),
            (0.09500, 0.83100),
        ],
    },
    {
        "name": "right_side_window_quarter",
        "object": "RM11BDYM",
        "tile": "rm11mdl1",
        "side": 1,
        "side_yz_controls": [
            (0.767, 0.970),
            (0.482, 1.120),
            (0.197, 1.240),
            (0.197, 1.080),
            (0.197, 0.920),
            (0.482, 0.920),
            (0.767, 0.920),
            (0.767, 0.935),
        ],
        "normalized_controls": [
            (0.84706, 0.67717),
            (0.92353, 0.67323),
            (1.00000, 0.66929),
            (1.00000, 0.77165),
            (0.99216, 0.88976),
            (0.92353, 0.92126),
            (0.85490, 0.93701),
            (0.84706, 0.81102),
        ],
    },
]


def tile_pixel_to_uv(tile_name: str, point: tuple[float, float]) -> Vector:
    tile = TILES[tile_name]
    x, y = point
    u = (tile["x"] + x) / ATLAS_SIZE
    v = (tile["y_bottom"] + tile["h"] - y) / ATLAS_SIZE
    return Vector((u, v))


def normalized_to_tile_uv(tile_name: str, point: tuple[float, float]) -> Vector:
    tile = TILES[tile_name]
    x = point[0] * tile["w"]
    y = point[1] * tile["h"]
    return tile_pixel_to_uv(tile_name, (x, y))


def quadratic_point(
    start: tuple[float, float],
    control: tuple[float, float],
    end: tuple[float, float],
    t: float,
) -> tuple[float, float]:
    inv = 1.0 - t
    return (
        inv * inv * start[0] + 2.0 * inv * t * control[0] + t * t * end[0],
        inv * inv * start[1] + 2.0 * inv * t * control[1] + t * t * end[1],
    )


def normalized_border_points(
    controls: list[tuple[float, float]],
    samples_per_side: int = 5,
) -> list[tuple[float, float]]:
    """Expand corner/midpoint controls into a clean 20-point closed border."""
    if len(controls) != 8:
        raise ValueError("expected controls ordered corner, side-midpoint, corner...")
    result = []
    for side in range(4):
        start = controls[(side * 2) % 8]
        midpoint = controls[(side * 2 + 1) % 8]
        end = controls[(side * 2 + 2) % 8]
        for sample in range(samples_per_side):
            t = sample / float(samples_per_side)
            result.append(quadratic_point(start, midpoint, end, t))
    return result


def barycentric_2d(point: Vector, a: Vector, b: Vector, c: Vector) -> tuple[float, float, float] | None:
    v0 = b - a
    v1 = c - a
    v2 = point - a
    dot00 = v0.dot(v0)
    dot01 = v0.dot(v1)
    dot02 = v0.dot(v2)
    dot11 = v1.dot(v1)
    dot12 = v1.dot(v2)
    denom = dot00 * dot11 - dot01 * dot01
    if abs(denom) < 1.0e-12:
        return None
    inv = 1.0 / denom
    v = (dot11 * dot02 - dot01 * dot12) * inv
    w = (dot00 * dot12 - dot01 * dot02) * inv
    u = 1.0 - v - w
    if u >= -0.001 and v >= -0.001 and w >= -0.001:
        return u, v, w
    return None


def closest_point_on_segment(point: Vector, a: Vector, b: Vector) -> tuple[Vector, float]:
    ab = b - a
    denom = ab.dot(ab)
    if denom <= 1.0e-12:
        return a, 0.0
    t = max(0.0, min(1.0, (point - a).dot(ab) / denom))
    return a.lerp(b, t), t


def closest_barycentric_2d(point: Vector, a: Vector, b: Vector, c: Vector) -> tuple[float, float, float, float]:
    inside = barycentric_2d(point, a, b, c)
    if inside is not None:
        return inside[0], inside[1], inside[2], 0.0

    candidates = []
    closest, t = closest_point_on_segment(point, a, b)
    candidates.append(((point - closest).length_squared, 1.0 - t, t, 0.0))
    closest, t = closest_point_on_segment(point, b, c)
    candidates.append(((point - closest).length_squared, 0.0, 1.0 - t, t))
    closest, t = closest_point_on_segment(point, c, a)
    candidates.append(((point - closest).length_squared, t, 0.0, 1.0 - t))
    candidates.sort(key=lambda item: item[0])
    distance, u, v, w = candidates[0]
    return u, v, w, distance


def polygon_uv_world(obj: bpy.types.Object, poly: bpy.types.MeshPolygon) -> list[tuple[Vector, Vector]]:
    uv_layer = obj.data.uv_layers.active
    result = []
    for loop_index in poly.loop_indices:
        loop = obj.data.loops[loop_index]
        uv = uv_layer.data[loop_index].uv
        world = obj.matrix_world @ obj.data.vertices[loop.vertex_index].co
        result.append((Vector((uv.x, uv.y)), world))
    return result


def point_from_uv(
    obj: bpy.types.Object,
    uv_point: Vector,
    side: int | None = None,
) -> tuple[Vector, Vector] | None:
    candidates: list[tuple[float, Vector, Vector]] = []
    fallback: list[tuple[float, Vector, Vector]] = []
    for poly in obj.data.polygons:
        coords = polygon_uv_world(obj, poly)
        if len(coords) < 3:
            continue
        triangles = [(0, i, i + 1) for i in range(1, len(coords) - 1)]
        for a_i, b_i, c_i in triangles:
            uv_a, world_a = coords[a_i]
            uv_b, world_b = coords[b_i]
            uv_c, world_c = coords[c_i]
            bary = barycentric_2d(uv_point, uv_a, uv_b, uv_c)
            if bary is None:
                u, v, w, distance = closest_barycentric_2d(uv_point, uv_a, uv_b, uv_c)
            else:
                u, v, w = bary
                distance = 0.0
            world = world_a * u + world_b * v + world_c * w
            if side is not None and world.x * side < -0.01:
                continue
            normal = (obj.matrix_world.to_3x3() @ poly.normal).normalized()
            # Prefer a match close to the requested side and close to the UV point.
            score = 0.0 if side is None else abs(world.x - side * 0.8)
            if bary is None:
                fallback.append((distance * 100.0 + score, world, normal))
            else:
                candidates.append((score, world, normal))
    if candidates:
        candidates.sort(key=lambda item: item[0])
        _score, world, normal = candidates[0]
        return world, normal
    if fallback:
        fallback.sort(key=lambda item: item[0])
        _score, world, normal = fallback[0]
        return world, normal
    return None


def point_from_side_yz(
    obj: bpy.types.Object,
    yz_point: tuple[float, float],
    side: int,
) -> tuple[Vector, Vector] | None:
    y, z = yz_point
    origin_world = Vector((side * 2.0, y, z))
    direction_world = Vector((-side, 0.0, 0.0))
    inv = obj.matrix_world.inverted()
    origin_local = inv @ origin_world
    direction_local = (inv.to_3x3() @ direction_world).normalized()
    hit, location, normal, _face_index = obj.ray_cast(origin_local, direction_local, distance=4.0)
    if hit:
        world = obj.matrix_world @ location
        world_normal = (obj.matrix_world.to_3x3() @ normal).normalized()
        return world, world_normal
    # The side glass paint rides over very coarse triangles.  If the exact ray
    # misses an open seam, keep the construction point on the side plane.
    return Vector((side * 0.86, y, z)), Vector((side, 0.0, 0.0))


def sampled_loop(points: list[Vector], samples_per_edge: int = 12) -> list[Vector]:
    result = []
    for index, start in enumerate(points):
        end = points[(index + 1) % len(points)]
        for sample in range(samples_per_edge):
            t = sample / float(samples_per_edge)
            result.append(start.lerp(end, t))
    result.append(points[0])
    return result


def material(name: str, color: tuple[float, float, float, float]) -> bpy.types.Material:
    mat = bpy.data.materials.get(name)
    if mat is None:
        mat = bpy.data.materials.new(name)
    mat.diffuse_color = color
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    if bsdf is not None:
        bsdf.inputs["Base Color"].default_value = color
        bsdf.inputs["Alpha"].default_value = color[3]
    return mat


def remove_prefixed(prefix: str) -> None:
    for obj in list(bpy.data.objects):
        if obj.name.startswith(prefix):
            bpy.data.objects.remove(obj, do_unlink=True)


def collection(name: str) -> bpy.types.Collection:
    coll = bpy.data.collections.get(name)
    if coll is None:
        coll = bpy.data.collections.new(name)
        bpy.context.scene.collection.children.link(coll)
    return coll


def link_to_collection(obj: bpy.types.Object, coll: bpy.types.Collection) -> None:
    for existing in obj.users_collection:
        existing.objects.unlink(obj)
    coll.objects.link(obj)


def create_curve(name: str, points: list[Vector], mat: bpy.types.Material) -> bpy.types.Object:
    curve = bpy.data.curves.new(name, "CURVE")
    curve.dimensions = "3D"
    curve.resolution_u = 2
    curve.bevel_depth = 0.008
    curve.bevel_resolution = 1
    poly = curve.splines.new("POLY")
    poly.points.add(len(points) - 1)
    for item, point in zip(poly.points, points):
        item.co = (point.x, point.y, point.z, 1.0)
    poly.use_cyclic_u = True
    obj = bpy.data.objects.new(name, curve)
    obj.data.materials.append(mat)
    obj.show_in_front = False
    return obj


def body_mesh_objects() -> list[bpy.types.Object]:
    return [
        obj
        for obj in bpy.context.scene.objects
        if obj.type == "MESH" and obj.name.startswith("RM11BDY")
    ]


def create_wire_overlay(mat: bpy.types.Material) -> list[bpy.types.Object]:
    coll = collection(WIRE_COLLECTION)
    created = []
    for source in body_mesh_objects():
        duplicate = source.copy()
        duplicate.data = source.data.copy()
        duplicate.name = f"{WIRE_PREFIX}_{source.name}"
        duplicate.data.materials.clear()
        duplicate.data.materials.append(mat)
        bpy.context.scene.collection.objects.link(duplicate)
        link_to_collection(duplicate, coll)
        mod = duplicate.modifiers.new("Black review wire", "WIREFRAME")
        mod.thickness = 0.004
        mod.use_even_offset = True
        mod.use_replace = True
        duplicate.show_in_front = False
        created.append(duplicate)
    return created


def look_at(obj: bpy.types.Object, target: Vector) -> None:
    obj.rotation_euler = (target - obj.location).to_track_quat("-Z", "Y").to_euler()


def render_preview(output: Path, view: str = "front_quarter") -> None:
    objects = [obj for obj in bpy.context.scene.objects if obj.visible_get()]
    corners = [obj.matrix_world @ Vector(corner) for obj in objects if obj.type == "MESH" for corner in obj.bound_box]
    minimum = Vector(tuple(min(point[axis] for point in corners) for axis in range(3)))
    maximum = Vector(tuple(max(point[axis] for point in corners) for axis in range(3)))
    center = (minimum + maximum) * 0.5
    radius = max((maximum - minimum).length * 0.5, 1.0)

    camera_data = bpy.data.cameras.new("Marshal Window Guide Camera")
    camera = bpy.data.objects.new("Marshal Window Guide Camera", camera_data)
    bpy.context.scene.collection.objects.link(camera)
    if view == "right_side":
        camera.location = center + Vector((radius * 2.0, 0.0, radius * 0.02))
        camera.data.type = "ORTHO"
        camera.data.ortho_scale = radius * 1.1
        look_at(camera, center + Vector((0.0, 0.0, radius * 0.02)))
    elif view == "rear_quarter":
        camera.location = center + Vector((-radius * 1.12, -radius * 1.72, radius * 0.72))
        camera.data.lens = 88
        look_at(camera, center + Vector((0.0, -radius * 0.36, radius * 0.10)))
    else:
        camera.location = center + Vector((radius * 1.08, -radius * 1.58, radius * 0.72))
        camera.data.lens = 80
        look_at(camera, center + Vector((0.0, 0.0, radius * 0.08)))
    bpy.context.scene.camera = camera

    scene = bpy.context.scene
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.display.shading.light = "STUDIO"
    scene.display.shading.color_type = "TEXTURE"
    scene.display.shading.show_shadows = False
    scene.display.shading.show_cavity = False
    scene.display.shading.show_specular_highlight = False
    scene.render.resolution_x = 1400
    scene.render.resolution_y = 950
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.render.film_transparent = False
    output.parent.mkdir(parents=True, exist_ok=True)
    scene.render.filepath = str(output)
    bpy.ops.render.render(write_still=True)
    bpy.data.objects.remove(camera, do_unlink=True)
    bpy.data.cameras.remove(camera_data)


def build_guides() -> dict[str, object]:
    bpy.ops.wm.open_mainfile(filepath=str(BLEND))
    remove_prefixed(GUIDE_PREFIX)
    remove_prefixed("I76_Marshal_WindowAnchor")
    remove_prefixed(WIRE_PREFIX)

    guide_mat = material("I76 Window Guide Black", (0.0, 0.0, 0.0, 1.0))
    wire_mat = material("I76 Review Wire Black", (0.0, 0.0, 0.0, 1.0))
    guide_coll = collection(GUIDE_COLLECTION)

    guide_records = []
    for spec in GUIDE_SPECS:
        obj = bpy.data.objects[spec["object"]]
        world_points = []
        misses = 0
        if "side_yz_controls" in spec:
            normalized_points = normalized_border_points(spec["side_yz_controls"])
            projection_points = normalized_points
        else:
            normalized_points = normalized_border_points(spec["normalized_controls"])
            projection_points = [normalized_to_tile_uv(spec["tile"], point) for point in normalized_points]
        for projection_point in projection_points:
            if "side_yz_controls" in spec:
                projected = point_from_side_yz(obj, projection_point, spec["side"])
            else:
                projected = point_from_uv(obj, projection_point, spec.get("side"))
            if projected is None:
                misses += 1
                continue
            world, normal = projected
            world_points.append(world + normal * 0.018)
        if len(world_points) < 4:
            guide_records.append({"name": spec["name"], "status": "missed", "misses": misses})
            continue
        curve = create_curve(f"{GUIDE_PREFIX}_{spec['name']}", world_points, guide_mat)
        curve["i76_window_subdivision_guide"] = True
        curve["i76_window_guide_tile"] = spec["tile"]
        curve["i76_window_guide_source_object"] = spec["object"]
        curve["i76_window_guide_normalized_controls"] = json.dumps(spec["normalized_controls"])
        if "side_yz_controls" in spec:
            curve["i76_window_guide_side_yz_controls"] = json.dumps(spec["side_yz_controls"])
            curve["i76_window_guide_side_yz_points"] = json.dumps(normalized_points)
        else:
            curve["i76_window_guide_normalized_points"] = json.dumps(normalized_points)
        link_to_collection(curve, guide_coll)
        guide_records.append(
            {
                "name": curve.name,
                "source_object": spec["object"],
                "tile": spec["tile"],
                "points": len(world_points),
                "controls": len(spec["normalized_controls"]),
                "misses": misses,
            }
        )

    bpy.context.scene["i76_marshal_window_guides"] = True
    bpy.context.scene["i76_marshal_window_guides_source"] = (
        "normalized texture-section window corner/edge paths projected to body mesh surfaces"
    )
    bpy.ops.object.select_all(action="DESELECT")
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND))
    backup = Path(str(BLEND) + "1")
    if backup.exists():
        backup.unlink()

    wire_objects = create_wire_overlay(wire_mat)
    shot = SHOT_ROOT / "NP_ransom_marshal_window_guides_black_wire_overlay.png"
    rear_shot = SHOT_ROOT / "NP_ransom_marshal_window_guides_black_wire_overlay_rear.png"
    side_shot = SHOT_ROOT / "NP_ransom_marshal_window_guides_black_wire_overlay_side.png"
    render_preview(shot)
    render_preview(rear_shot, view="rear_quarter")
    render_preview(side_shot, view="right_side")
    for obj in wire_objects:
        bpy.data.objects.remove(obj, do_unlink=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND))
    if backup.exists():
        backup.unlink()

    summary = {
        "blend": str(BLEND),
        "screenshot": str(shot),
        "rear_screenshot": str(rear_shot),
        "side_screenshot": str(side_shot),
        "guides": guide_records,
    }
    SHOT_ROOT.mkdir(parents=True, exist_ok=True)
    (SHOT_ROOT / "marshal_window_guides_summary.json").write_text(
        json.dumps(summary, indent=2) + "\n",
        encoding="utf-8",
    )
    return summary


def validate() -> list[dict[str, object]]:
    bpy.ops.wm.open_mainfile(filepath=str(BLEND))
    guides = [obj for obj in bpy.data.objects if obj.get("i76_window_subdivision_guide")]
    images = [image for image in bpy.data.images if image.get("i76_atlas")]
    return [
        {
            "blend": str(BLEND),
            "guide_count": len(guides),
            "guide_names": [obj.name for obj in guides],
            "anchor_count": len([obj for obj in bpy.data.objects if obj.name.startswith("I76_Marshal_WindowAnchor")]),
            "atlas_images": len(images),
            "packed": [bool(image.packed_file) for image in images],
            "selected": len([obj for obj in bpy.context.scene.objects if obj.select_get()]),
        }
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--validate", action="store_true")
    args = parser.parse_args(sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None)
    result = validate() if args.validate else build_guides()
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
