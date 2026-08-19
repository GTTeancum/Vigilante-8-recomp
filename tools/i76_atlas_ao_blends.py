#!/usr/bin/env python3
"""Build one 4x Real-ESRGAN atlas with baked AO for selected I76 blends."""

from __future__ import annotations

import argparse
import json
import math
import re
import shutil
import subprocess
import sys
from array import array
from dataclasses import dataclass
from pathlib import Path

import bpy
from mathutils import Vector


ROOT = Path(__file__).resolve().parents[1]
REAL_ESRGAN = ROOT / "build" / "realesrgan" / "bin" / "realesrgan-ncnn-vulkan.exe"
TEXTURE_FIDELITY = ROOT / "tools" / "i76_texture_upscale.py"
IMAGE_OPS = ROOT / "tools" / "i76_atlas_image_ops.py"
SCALE = 4
TILE_PADDING = 8
SWATCH_SIZE = 32
AO_STRENGTH = 0.34
ATLAS_MAX_WIDTH = 4096


TARGETS = (
    "I76_picard_piranha",
    "NP_ransom_marshal",
    "NP_dover_rampage",
    "I76_skeeters_van",
    "NP_leopard_xlc",
)


@dataclass(frozen=True)
class Tile:
    key: str
    source_name: str
    width: int
    height: int
    x: int
    y_bottom: int
    kind: str


def safe_stem(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "_", value.lower()).strip("_") or "image"


def copy_custom_properties(source: bpy.types.ID, destination: bpy.types.ID) -> None:
    for key in source.keys():
        destination[key] = source[key]


def find_material_image(material: bpy.types.Material) -> bpy.types.Image | None:
    if not material or not material.use_nodes or material.node_tree is None:
        return None
    for node in material.node_tree.nodes:
        if node.type == "TEX_IMAGE" and node.image is not None:
            return node.image
    return None


def material_color(material: bpy.types.Material | None) -> tuple[int, int, int, int]:
    color = (0.8, 0.8, 0.8, 1.0)
    if material is not None:
        color = tuple(material.diffuse_color)
        if material.use_nodes and material.node_tree is not None:
            for node in material.node_tree.nodes:
                if node.type == "BSDF_PRINCIPLED":
                    try:
                        base = node.inputs["Base Color"].default_value
                        alpha = node.inputs["Alpha"].default_value
                        color = (base[0], base[1], base[2], alpha)
                    except Exception:
                        pass
                    break
    return tuple(max(0, min(255, round(component * 255))) for component in color)


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


def used_images_and_swatch_keys() -> tuple[dict[str, bpy.types.Image], set[str]]:
    images: dict[str, bpy.types.Image] = {}
    swatches: set[str] = set()
    for obj in mesh_objects():
        for polygon in obj.data.polygons:
            material = (
                obj.material_slots[polygon.material_index].material
                if polygon.material_index < len(obj.material_slots)
                else None
            )
            image = find_material_image(material) if material is not None else None
            if image is not None:
                images[image.name] = image
            else:
                swatches.add("solid_%02x%02x%02x%02x" % material_color(material))
    return images, swatches


def export_source_textures(images: dict[str, bpy.types.Image], source_dir: Path) -> dict[str, dict[str, object]]:
    source_dir.mkdir(parents=True, exist_ok=True)
    records: dict[str, dict[str, object]] = {}
    for index, image in enumerate(sorted(images.values(), key=lambda item: item.name)):
        filename = f"{index:03d}_{safe_stem(image.name)}.png"
        destination = source_dir / filename
        image.filepath_raw = str(destination)
        image.file_format = "PNG"
        image.save()
        records[image.name] = {
            "filename": filename,
            "width": int(image.size[0]),
            "height": int(image.size[1]),
            "colorspace": image.colorspace_settings.name,
            "alpha_mode": image.alpha_mode,
            "properties": {key: image[key] for key in image.keys()},
        }
    return records


def run_realesrgan(source_dir: Path, neural_dir: Path, fidelity_dir: Path) -> None:
    if not REAL_ESRGAN.is_file():
        raise RuntimeError(f"missing Real-ESRGAN executable: {REAL_ESRGAN}")
    python_exe = shutil.which("python")
    if python_exe is None:
        raise RuntimeError("system Python was not found")
    neural_dir.mkdir(parents=True, exist_ok=True)
    fidelity_dir.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        [
            str(REAL_ESRGAN),
            "-i",
            str(source_dir),
            "-o",
            str(neural_dir),
            "-n",
            "realesr-animevideov3",
            "-s",
            str(SCALE),
            "-t",
            "64",
            "-f",
            "png",
        ],
        cwd=REAL_ESRGAN.parent,
        check=True,
    )
    subprocess.run(
        [
            python_exe,
            str(TEXTURE_FIDELITY),
            "--source",
            str(source_dir),
            "--esrgan",
            str(neural_dir),
            "--out",
            str(fidelity_dir),
            "--scale",
            str(SCALE),
            "--edge-threshold",
            "35",
            "--edge-strength",
            "0.85",
        ],
        cwd=ROOT,
        check=True,
    )


def system_python() -> str:
    python_exe = shutil.which("python")
    if python_exe is None:
        raise RuntimeError("system Python was not found")
    return python_exe


def pack_tiles(
    records: dict[str, dict[str, object]],
    swatches: set[str],
) -> tuple[list[Tile], int, int]:
    pending: list[tuple[str, str, int, int, str]] = []
    for image_name, record in records.items():
        pending.append(
            (
                image_name,
                image_name,
                int(record["width"]) * SCALE,
                int(record["height"]) * SCALE,
                "image",
            )
        )
    for key in sorted(swatches):
        pending.append((key, key, SWATCH_SIZE, SWATCH_SIZE, "swatch"))
    pending.sort(key=lambda item: (-item[3], -item[2], item[0]))

    tiles: list[Tile] = []
    x = TILE_PADDING
    y = TILE_PADDING
    row_height = 0
    atlas_width = 0
    for key, source_name, width, height, kind in pending:
        if x > TILE_PADDING and x + width + TILE_PADDING > ATLAS_MAX_WIDTH:
            x = TILE_PADDING
            y += row_height + TILE_PADDING
            row_height = 0
        tiles.append(Tile(key, source_name, width, height, x, y, kind))
        atlas_width = max(atlas_width, x + width + TILE_PADDING)
        x += width + TILE_PADDING
        row_height = max(row_height, height)
    atlas_height = y + row_height + TILE_PADDING
    atlas_width = next_power_of_two(max(64, atlas_width))
    atlas_height = next_power_of_two(max(64, atlas_height))
    return tiles, atlas_width, atlas_height


def next_power_of_two(value: int) -> int:
    return 1 << (value - 1).bit_length()


def build_atlas_png(
    records: dict[str, dict[str, object]],
    swatches: set[str],
    fidelity_dir: Path,
    atlas_path: Path,
) -> tuple[dict[str, Tile], int, int]:
    tiles, width, height = pack_tiles(records, swatches)
    tile_map: dict[str, Tile] = {}
    manifest_path = atlas_path.with_suffix(".build.json")
    for tile in tiles:
        tile_map[tile.key] = tile
    atlas_path.parent.mkdir(parents=True, exist_ok=True)
    manifest = {
        "output": str(atlas_path),
        "fidelity_dir": str(fidelity_dir),
        "atlas_size": [width, height],
        "records": records,
        "tiles": [
            {
                "key": tile.key,
                "sourceName": tile.source_name,
                "kind": tile.kind,
                "x": tile.x,
                "yBottom": tile.y_bottom,
                "width": tile.width,
                "height": tile.height,
            }
            for tile in tiles
        ],
    }
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    subprocess.run(
        [system_python(), str(IMAGE_OPS), "build-atlas", str(manifest_path)],
        cwd=ROOT,
        check=True,
    )
    return tile_map, width, height


def make_atlas_material(image: bpy.types.Image) -> bpy.types.Material:
    material = bpy.data.materials.new("I76 4x Atlas + AO")
    material.use_nodes = True
    material.diffuse_color = (1, 1, 1, 1)
    material.blend_method = "CLIP"
    material.alpha_threshold = 0.5
    material.use_screen_refraction = False
    material.show_transparent_back = True
    nodes = material.node_tree.nodes
    for node in nodes:
        nodes.remove(node)
    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (360, 0)
    bsdf = nodes.new("ShaderNodeBsdfPrincipled")
    bsdf.location = (120, 0)
    texture = nodes.new("ShaderNodeTexImage")
    texture.name = "I76 Atlas Texture"
    texture.location = (-160, 0)
    texture.image = image
    material.node_tree.links.new(texture.outputs["Color"], bsdf.inputs["Base Color"])
    material.node_tree.links.new(texture.outputs["Alpha"], bsdf.inputs["Alpha"])
    material.node_tree.links.new(bsdf.outputs["BSDF"], output.inputs["Surface"])
    nodes.active = texture
    texture.select = True
    return material


def remap_meshes(atlas_material: bpy.types.Material, tile_map: dict[str, Tile], atlas_width: int, atlas_height: int) -> None:
    swatch_tiles = {key: tile for key, tile in tile_map.items() if tile.kind == "swatch"}
    for obj in mesh_objects():
        mesh = obj.data
        uv_layer = mesh.uv_layers.active or mesh.uv_layers.new(name="I76 Atlas UV")
        uv_layer.name = "I76 Atlas UV"
        original_materials = [slot.material for slot in obj.material_slots]

        for polygon in mesh.polygons:
            material = (
                original_materials[polygon.material_index]
                if polygon.material_index < len(original_materials)
                else None
            )
            image = find_material_image(material) if material is not None else None
            if image is not None:
                tile = tile_map[image.name]
                for loop_index in polygon.loop_indices:
                    uv = uv_layer.data[loop_index].uv
                    source_u = max(0.0, min(1.0, uv.x))
                    source_v = max(0.0, min(1.0, uv.y))
                    uv.x = (tile.x + source_u * tile.width) / atlas_width
                    uv.y = (tile.y_bottom + source_v * tile.height) / atlas_height
            else:
                key = "solid_%02x%02x%02x%02x" % material_color(material)
                tile = swatch_tiles[key]
                margin = 4
                coords = (
                    (tile.x + margin, tile.y_bottom + margin),
                    (tile.x + tile.width - margin, tile.y_bottom + margin),
                    (tile.x + tile.width - margin, tile.y_bottom + tile.height - margin),
                    (tile.x + margin, tile.y_bottom + tile.height - margin),
                )
                for offset, loop_index in enumerate(polygon.loop_indices):
                    u, v = coords[offset % len(coords)]
                    uv_layer.data[loop_index].uv = (u / atlas_width, v / atlas_height)

        obj.data.materials.clear()
        obj.data.materials.append(atlas_material)
        for polygon in obj.data.polygons:
            polygon.material_index = 0


def bake_ao_and_multiply(
    atlas_image: bpy.types.Image,
    atlas_material: bpy.types.Material,
    atlas_path: Path,
    ao_path: Path,
    baked_path: Path,
) -> None:
    ao_image = bpy.data.images.new(
        "I76 Atlas AO Bake",
        width=atlas_image.size[0],
        height=atlas_image.size[1],
        alpha=False,
        float_buffer=False,
    )
    white = array("f", [1.0]) * (ao_image.size[0] * ao_image.size[1] * 4)
    ao_image.pixels.foreach_set(white)
    ao_image.update()
    nodes = atlas_material.node_tree.nodes
    ao_node = nodes.new("ShaderNodeTexImage")
    ao_node.name = "I76 Atlas AO Bake Target"
    ao_node.image = ao_image
    for node in nodes:
        node.select = False
    ao_node.select = True
    nodes.active = ao_node

    all_meshes = mesh_objects()
    excluded_meshes = [obj for obj in all_meshes if is_ao_excluded_object(obj)]
    visibility_state = {
        obj: (obj.hide_render, obj.hide_viewport, obj.hide_get())
        for obj in excluded_meshes
    }
    for obj in excluded_meshes:
        obj.hide_render = True
        obj.hide_viewport = True
        obj.hide_set(True)

    bpy.ops.object.select_all(action="DESELECT")
    visible_meshes = [obj for obj in all_meshes if not is_ao_excluded_object(obj)]
    if not visible_meshes:
        raise RuntimeError("no body meshes left after AO exclusions")
    for obj in visible_meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = visible_meshes[0]

    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 32
    scene.cycles.use_denoising = False
    scene.cycles.device = "CPU"
    if scene.world is None:
        scene.world = bpy.data.worlds.new("I76 AO Bake World")
    scene.world.color = (1, 1, 1)
    scene.render.bake.margin = 12
    try:
        bpy.ops.object.bake(type="AO", target="IMAGE_TEXTURES", use_clear=False, margin=12)
    finally:
        for obj, (hide_render, hide_viewport, hidden) in visibility_state.items():
            obj.hide_render = hide_render
            obj.hide_viewport = hide_viewport
            obj.hide_set(hidden)

    ao_image.filepath_raw = str(ao_path)
    ao_image.file_format = "PNG"
    ao_image.save()

    subprocess.run(
        [
            system_python(),
            str(IMAGE_OPS),
            "multiply-ao",
            "--base",
            str(atlas_path),
            "--ao",
            str(ao_path),
            "--out",
            str(baked_path),
            "--strength",
            str(AO_STRENGTH),
        ],
        cwd=ROOT,
        check=True,
    )

    atlas_image.filepath = str(baked_path)
    atlas_image.reload()
    atlas_image.pack()
    nodes.remove(ao_node)
    bpy.data.images.remove(ao_image)
    for node in nodes:
        node.select = False
        if node.type == "TEX_IMAGE" and node.image == atlas_image:
            node.select = True
            nodes.active = node


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
    elif view_name == "rear":
        camera.location = center + Vector((0, radius * 2.0, radius * 0.45))
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


def build_contact_sheet(paths: list[Path], output: Path) -> None:
    subprocess.run(
        [system_python(), str(IMAGE_OPS), "contact-sheet", "--out", str(output), *map(str, paths)],
        cwd=ROOT,
        check=True,
    )


def process_blend(blend: Path, work_root: Path, screenshot_root: Path, backup_root: Path) -> dict[str, object]:
    slug = blend.stem
    work = work_root / slug
    source_dir = work / "source"
    neural_dir = work / "realesrgan_x4"
    fidelity_dir = work / "fidelity_x4"
    atlas_path = work / f"{slug}_atlas_x4.png"
    ao_path = work / f"{slug}_atlas_ao.png"
    baked_path = work / f"{slug}_atlas_x4_ao.png"
    manifest_path = work / "manifest.json"

    bpy.ops.wm.open_mainfile(filepath=str(blend))
    images, swatches = used_images_and_swatch_keys()
    records = export_source_textures(images, source_dir)
    run_realesrgan(source_dir, neural_dir, fidelity_dir)
    tile_map, atlas_width, atlas_height = build_atlas_png(records, swatches, fidelity_dir, atlas_path)

    atlas_image = bpy.data.images.load(str(atlas_path), check_existing=False)
    atlas_image.name = f"{slug} Atlas [Real-ESRGAN 4x + AO]"
    atlas_image.alpha_mode = "STRAIGHT"
    atlas_image["i76_atlas"] = True
    atlas_image["i76_upscaler"] = "Real-ESRGAN realesr-animevideov3"
    atlas_image["i76_upscale_factor"] = SCALE
    atlas_image["i76_ao_baked"] = True
    atlas_image["i76_ao_strength"] = AO_STRENGTH
    atlas_material = make_atlas_material(atlas_image)
    remap_meshes(atlas_material, tile_map, atlas_width, atlas_height)
    bake_ao_and_multiply(atlas_image, atlas_material, atlas_path, ao_path, baked_path)

    bpy.context.scene["i76_texture_atlas"] = bpy.data.images[atlas_image.name].name
    bpy.context.scene["i76_texture_atlas_size"] = f"{atlas_width}x{atlas_height}"
    bpy.context.scene["i76_texture_upscaler"] = "Real-ESRGAN realesr-animevideov3"
    bpy.context.scene["i76_texture_upscale_factor"] = SCALE
    bpy.context.scene["i76_ao_baked_into_atlas"] = True

    bpy.ops.object.select_all(action="DESELECT")
    backup_root.mkdir(parents=True, exist_ok=True)
    backup_path = backup_root / blend.name
    if not backup_path.exists():
        shutil.copy2(blend, backup_path)
    bpy.ops.wm.save_as_mainfile(filepath=str(blend))
    blender_backup = Path(str(blend) + "1")
    if blender_backup.exists():
        blender_backup.unlink()

    screenshot_root.mkdir(parents=True, exist_ok=True)
    screenshot_paths = []
    for view in ("front", "front_quarter"):
        shot = screenshot_root / f"{slug}_{view}.png"
        render_preview(shot, view)
        screenshot_paths.append(shot)

    manifest = {
        "blend": str(blend),
        "backup": str(backup_path),
        "atlas": str(baked_path),
        "atlas_size": [atlas_width, atlas_height],
        "source_images": len(records),
        "solid_swatches": len(swatches),
        "screenshots": [str(path) for path in screenshot_paths],
        "ao_excluded_objects": [
            obj.name for obj in mesh_objects() if is_ao_excluded_object(obj)
        ],
        "tiles": [
            {
                "key": tile.key,
                "kind": tile.kind,
                "x": tile.x,
                "yBottom": tile.y_bottom,
                "width": tile.width,
                "height": tile.height,
            }
            for tile in tile_map.values()
        ],
    }
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return manifest


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--targets", nargs="*", default=list(TARGETS))
    parser.add_argument("--work-root", type=Path, default=ROOT / "artifacts" / "i76_atlas_ao")
    args = parser.parse_args(sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None)

    work_root = args.work_root.resolve()
    screenshot_root = work_root / "screenshots"
    backup_root = work_root / "original_blend_backups"
    manifests = []
    for target in args.targets:
        blend = ROOT / "I76" / f"{target}.blend"
        if not blend.is_file():
            raise RuntimeError(f"missing blend: {blend}")
        manifests.append(process_blend(blend, work_root, screenshot_root, backup_root))
        print(f"I76_ATLAS_AO_OK {target}", flush=True)
    contact_sheet = screenshot_root / "i76_atlas_ao_contact_sheet.png"
    first_shots = [Path(item["screenshots"][0]) for item in manifests]
    build_contact_sheet(first_shots, contact_sheet)
    (work_root / "summary.json").write_text(
        json.dumps({"vehicles": manifests, "contact_sheet": str(contact_sheet)}, indent=2) + "\n",
        encoding="utf-8",
    )
    print(f"I76_ATLAS_AO_DONE vehicles={len(manifests)} contact_sheet={contact_sheet}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
