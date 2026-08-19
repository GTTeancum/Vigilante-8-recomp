#!/usr/bin/env python3
"""Strengthen lower-body shading on already atlased I76 blends."""

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
IMAGE_OPS = ROOT / "tools" / "i76_atlas_image_ops.py"
ARTIFACT_ROOT = ROOT / "artifacts" / "i76_atlas_ao"

TARGETS = (
    "I76_picard_piranha",
    "NP_ransom_marshal",
    "NP_dover_rampage",
    "I76_skeeters_van",
    "NP_leopard_xlc",
)


def system_python() -> str:
    python_exe = shutil.which("python")
    if python_exe is None:
        raise RuntimeError("system Python was not found")
    return python_exe


def mesh_objects() -> list[bpy.types.Object]:
    return [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]


def is_ao_excluded_object(obj: bpy.types.Object) -> bool:
    name = obj.name.lower()
    mesh_name = obj.data.name.lower() if obj.data is not None else ""
    source_name = str(obj.get("source_name", "")).lower()
    combined = " ".join((name, mesh_name, source_name))
    if any(token in combined for token in ("wheel", "weapon", "gun")):
        return True
    if any(token in combined for token in ("blgt", "hlgt", "tlgt", "lght", "light")):
        return True
    return False


def clamp(value: float, low: float = 0.0, high: float = 1.0) -> float:
    return max(low, min(high, value))


def look_at(obj: bpy.types.Object, target: Vector) -> None:
    obj.rotation_euler = (target - obj.location).to_track_quat("-Z", "Y").to_euler()


def render_preview(output: Path, view_name: str) -> None:
    objects = [obj for obj in mesh_objects() if obj.visible_get()]
    corners = [obj.matrix_world @ Vector(corner) for obj in objects for corner in obj.bound_box]
    minimum = Vector(tuple(min(point[axis] for point in corners) for axis in range(3)))
    maximum = Vector(tuple(max(point[axis] for point in corners) for axis in range(3)))
    center = (minimum + maximum) * 0.5
    radius = max((maximum - minimum).length * 0.5, 1.0)

    camera_data = bpy.data.cameras.new(f"{view_name} Camera")
    camera = bpy.data.objects.new(f"{view_name} Camera", camera_data)
    bpy.context.scene.collection.objects.link(camera)
    if view_name == "front":
        camera.location = center + Vector((0, -radius * 2.0, radius * 0.45))
    else:
        camera.location = center + Vector((radius * 1.25, -radius * 1.65, radius * 0.65))
    camera_data.lens = 70
    look_at(camera, center)
    bpy.context.scene.camera = camera

    scene = bpy.context.scene
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.display.shading.light = "STUDIO"
    scene.display.shading.color_type = "TEXTURE"
    scene.display.shading.show_shadows = False
    scene.display.shading.show_cavity = False
    scene.display.shading.show_specular_highlight = False
    scene.render.resolution_x = 1200
    scene.render.resolution_y = 850
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.render.film_transparent = False
    scene.render.filepath = str(output)
    bpy.ops.render.render(write_still=True)
    bpy.data.objects.remove(camera, do_unlink=True)
    bpy.data.cameras.remove(camera_data)


def atlas_image() -> bpy.types.Image:
    images = [image for image in bpy.data.images if image.get("i76_atlas")]
    if len(images) != 1:
        raise RuntimeError(f"expected one atlas image, found {len(images)}")
    return images[0]


def body_bounds(objects: list[bpy.types.Object]) -> tuple[float, float]:
    z_values = [
        (obj.matrix_world @ Vector(corner)).z
        for obj in objects
        for corner in obj.bound_box
    ]
    return min(z_values), max(z_values)


def polygon_factor(obj: bpy.types.Object, polygon: bpy.types.MeshPolygon, min_z: float, max_z: float) -> float:
    center = obj.matrix_world @ polygon.center
    normal = (obj.matrix_world.to_3x3() @ polygon.normal).normalized()
    height = clamp((center.z - min_z) / max(max_z - min_z, 0.001))

    lower = clamp(1.0 - height)
    underside = clamp(-normal.z)
    vertical = 1.0 - abs(normal.z)

    # These cars are sub-500-poly meshes with flat source textures, so a
    # purely physical AO bake is too timid.  This is an art-directed diffuse
    # lighting pass: side panels get a base falloff, lower body panels get a
    # stronger grime/contact falloff, and downward-facing surfaces get heavy
    # underside shade.  A fixed upper-front light adds faceted depth without
    # touching wheels, weapons, or separate light meshes.
    key_light = Vector((-0.35, -0.55, 0.76)).normalized()
    half_lambert = clamp(normal.dot(key_light) * 0.5 + 0.5)
    directional_factor = 0.68 + 0.32 * half_lambert

    side_depth = 0.18 * vertical
    lower_side_depth = 0.36 * (lower ** 1.20) * vertical
    lower_contact_depth = 0.18 * (lower ** 1.65)
    underside_depth = 0.46 * underside
    darken = side_depth + lower_side_depth + lower_contact_depth + underside_depth

    if normal.z > 0.35 and height > 0.62:
        darken *= 0.22
    return clamp(min(directional_factor, 1.0 - darken), 0.38, 1.0)


def export_shade_manifest(slug: str, work: Path, image: bpy.types.Image) -> Path:
    atlas_width, atlas_height = int(image.size[0]), int(image.size[1])
    body_meshes = [obj for obj in mesh_objects() if not is_ao_excluded_object(obj)]
    min_z, max_z = body_bounds(body_meshes)
    polygons: list[dict[str, object]] = []

    for obj in body_meshes:
        mesh = obj.data
        uv_layer = mesh.uv_layers.active
        if uv_layer is None:
            continue
        for polygon in mesh.polygons:
            factor = polygon_factor(obj, polygon, min_z, max_z)
            points = []
            for loop_index in polygon.loop_indices:
                uv = uv_layer.data[loop_index].uv
                points.append([
                    clamp(float(uv.x)) * atlas_width,
                    (1.0 - clamp(float(uv.y))) * atlas_height,
                ])
            polygons.append({"factor": factor, "points": points})

    manifest = {
        "atlas_size": [atlas_width, atlas_height],
        "base": str(work / f"{slug}_atlas_x4.png"),
        "ao": str(work / f"{slug}_atlas_ao.png"),
        "ao_strength": 0.42,
        "output": str(work / f"{slug}_atlas_x4_ao_boosted.png"),
        "mask_output": str(work / f"{slug}_body_shade_mask.png"),
        "polygons": polygons,
    }
    path = work / f"{slug}_shade_boost.json"
    path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return path


def process(target: str, screenshot_root: Path) -> dict[str, object]:
    blend = ROOT / "I76" / f"{target}.blend"
    work = ARTIFACT_ROOT / target
    bpy.ops.wm.open_mainfile(filepath=str(blend))
    image = atlas_image()
    manifest_path = export_shade_manifest(target, work, image)
    subprocess.run(
        [system_python(), str(IMAGE_OPS), "shade-boost", str(manifest_path)],
        cwd=ROOT,
        check=True,
    )
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    boosted_path = Path(manifest["output"])
    image.filepath = str(boosted_path)
    image.reload()
    image["i76_body_shading_boost"] = True
    image["i76_body_shading_boost_source"] = "AO + low-height/downward-facing body mask"
    image.pack()
    bpy.context.scene["i76_body_shading_boost"] = True
    bpy.context.scene["i76_body_shading_boost_source"] = "AO + low-height/downward-facing body mask"
    bpy.ops.object.select_all(action="DESELECT")
    bpy.ops.wm.save_as_mainfile(filepath=str(blend))
    backup = Path(str(blend) + "1")
    if backup.exists():
        backup.unlink()

    screenshot_root.mkdir(parents=True, exist_ok=True)
    shots = []
    for view in ("front", "front_quarter"):
        output = screenshot_root / f"{target}_{view}.png"
        render_preview(output, view)
        shots.append(str(output))
    return {
        "target": target,
        "blend": str(blend),
        "boosted_atlas": str(boosted_path),
        "mask": manifest["mask_output"],
        "screenshots": shots,
        "body_polygons": len(manifest["polygons"]),
    }


def build_contact_sheet(paths: list[Path], output: Path) -> None:
    subprocess.run(
        [system_python(), str(IMAGE_OPS), "contact-sheet", "--out", str(output), *map(str, paths)],
        cwd=ROOT,
        check=True,
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--targets", nargs="*", default=list(TARGETS))
    args = parser.parse_args(sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None)

    screenshot_root = ARTIFACT_ROOT / "screenshots_boosted"
    records = [process(target, screenshot_root) for target in args.targets]
    build_contact_sheet(
        [Path(record["screenshots"][0]) for record in records],
        screenshot_root / "i76_atlas_ao_boosted_front_contact_sheet.png",
    )
    build_contact_sheet(
        [Path(record["screenshots"][1]) for record in records],
        screenshot_root / "i76_atlas_ao_boosted_front_quarter_contact_sheet.png",
    )
    (ARTIFACT_ROOT / "shade_boost_summary.json").write_text(
        json.dumps(records, indent=2) + "\n", encoding="utf-8"
    )
    print(f"I76_BODY_SHADING_BOOST_DONE vehicles={len(records)} screenshots={screenshot_root}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
