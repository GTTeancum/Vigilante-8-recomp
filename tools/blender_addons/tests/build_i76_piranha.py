#!/usr/bin/env python3
"""Build a textured Interstate '76 Piranha scene in Blender."""

from __future__ import annotations

import re
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


ROOT = Path(__file__).resolve().parents[3]
TOOLS = ROOT / "tools"
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

from i76_vehicle_import import (  # noqa: E402
    AssetStore,
    decode_vqm,
    load_geo_pack,
    normalized_name,
    parse_geometry_chunk,
    parse_hardpoints,
    parse_vgeo_bank,
    parse_vehicle_weapons,
    parse_wheel_locations,
    resolve_material_vqm,
)


ASSETS = ROOT / "I76_ZFS"
OUTPUT = ROOT / "artifacts" / "i76_piranha_blender"
BLEND = OUTPUT / "interstate76_piranha_textured.blend"
RENDER = OUTPUT / "interstate76_piranha_textured.png"


def native_matrix(values: tuple[float, ...]) -> Matrix:
    # I76 uses Y-up. Map native X/Y/Z to Blender X/Z/Y, matching the
    # established Battlezone importer for this engine family.
    native = Matrix(
        (
            (values[0], values[1], values[2], values[9]),
            (values[3], values[4], values[5], values[10]),
            (values[6], values[7], values[8], values[11]),
            (0.0, 0.0, 0.0, 1.0),
        )
    )
    basis = Matrix(
        (
            (1.0, 0.0, 0.0, 0.0),
            (0.0, 0.0, 1.0, 0.0),
            (0.0, 1.0, 0.0, 0.0),
            (0.0, 0.0, 0.0, 1.0),
        )
    )
    return basis @ native @ basis.inverted()


def image_from_vqm(
    name: str,
    payload: bytes,
    cache: dict[str, bpy.types.Image],
    transparent_palette_index: int | None = None,
) -> bpy.types.Image:
    key = normalized_name(name) + (
        f"_alpha{transparent_palette_index}"
        if transparent_palette_index is not None
        else "_opaque"
    )
    if key in cache:
        return cache[key]
    width, height, rgba, codebook = decode_vqm(
        ASSETS,
        payload,
        transparent_palette_index=transparent_palette_index,
    )
    image_name = (
        f"{name} [index {transparent_palette_index} transparent]"
        if transparent_palette_index is not None
        else name
    )
    image = bpy.data.images.new(
        image_name, width=width, height=height, alpha=True
    )
    image.colorspace_settings.name = "sRGB"
    image.alpha_mode = "STRAIGHT"
    # Blender stores the first image row at the bottom. VQM rows are top-down.
    pixels: list[float] = []
    for y in range(height - 1, -1, -1):
        row = rgba[y * width * 4 : (y + 1) * width * 4]
        pixels.extend(value / 255.0 for value in row)
    image.pixels.foreach_set(pixels)
    image.update()
    image["i76_source_format"] = "VQM"
    image["i76_codebook"] = codebook
    image["i76_palette"] = "p02.act"
    if transparent_palette_index is not None:
        image["i76_transparent_palette_index"] = transparent_palette_index
    cache_directory = OUTPUT / "_texture_cache"
    cache_directory.mkdir(parents=True, exist_ok=True)
    image.filepath_raw = str(cache_directory / f"{key}.png")
    image.file_format = "PNG"
    image.save()
    image.pack()
    cache[key] = image
    return image


def make_material(
    face_material: str,
    face_color: tuple[int, int, int],
    render_flags: tuple[int, int, int],
    store: AssetStore,
    vehicle_prefix: str,
    images: dict[str, bpy.types.Image],
    materials: dict[
        tuple[str, tuple[int, int, int], bool], bpy.types.Material
    ],
    appearance: int = 1,
) -> bpy.types.Material:
    resolved = resolve_material_vqm(
        store, face_material, vehicle_prefix, appearance
    )
    is_transparent = resolved is not None and render_flags[1] == 5
    identity = (
        normalized_name(resolved[0]) if resolved else normalized_name(face_material),
        face_color if resolved is None else (0, 0, 0),
        is_transparent,
    )
    if identity in materials:
        return materials[identity]

    label = face_material or "Native Face Color"
    material = bpy.data.materials.new(
        f"I76 {label} ({face_color[0]},{face_color[1]},{face_color[2]})"
        if resolved is None
        else f"I76 {label}{' — Index 255 Cutout' if is_transparent else ''}"
    )
    material.use_nodes = True
    material.diffuse_color = (
        face_color[0] / 255.0,
        face_color[1] / 255.0,
        face_color[2] / 255.0,
        1.0,
    )
    material["i76_material_key"] = face_material
    nodes = material.node_tree.nodes
    nodes.clear()
    output = nodes.new("ShaderNodeOutputMaterial")
    emission = nodes.new("ShaderNodeEmission")
    if resolved is not None:
        vqm_name, payload = resolved
        image = image_from_vqm(
            vqm_name,
            payload,
            images,
            255 if is_transparent else None,
        )
        texture = nodes.new("ShaderNodeTexImage")
        texture.image = image
        texture.interpolation = "Closest"
        texture.extension = "EXTEND"
        material.node_tree.links.new(texture.outputs["Color"], emission.inputs["Color"])
        material["i76_vqm"] = vqm_name
        if is_transparent:
            transparent = nodes.new("ShaderNodeBsdfTransparent")
            mix = nodes.new("ShaderNodeMixShader")
            material.node_tree.links.new(
                texture.outputs["Alpha"], mix.inputs["Fac"]
            )
            material.node_tree.links.new(
                transparent.outputs["BSDF"], mix.inputs[1]
            )
            material.node_tree.links.new(
                emission.outputs["Emission"], mix.inputs[2]
            )
            material.node_tree.links.new(
                mix.outputs["Shader"], output.inputs["Surface"]
            )
            material.surface_render_method = "BLENDED"
            material["i76_transparent_palette_index"] = 255
        else:
            material.node_tree.links.new(
                emission.outputs["Emission"], output.inputs["Surface"]
            )
    else:
        emission.inputs["Color"].default_value = material.diffuse_color
        material.node_tree.links.new(
            emission.outputs["Emission"], output.inputs["Surface"]
        )
    materials[identity] = material
    return material


def create_object(
    placement,
    geo,
    store: AssetStore,
    vehicle_prefix: str,
    collection: bpy.types.Collection,
    images: dict[str, bpy.types.Image],
    materials: dict[
        tuple[str, tuple[int, int, int], bool], bpy.types.Material
    ],
    object_name: str | None = None,
    appearance: int = 1,
) -> bpy.types.Object:
    vertices = [(x, z, y) for x, y, z in geo.vertices]
    faces = [tuple(corner.vertex for corner in face.corners) for face in geo.faces]
    mesh = bpy.data.meshes.new(f"{placement.name} Mesh")
    mesh.from_pydata(vertices, (), faces)
    mesh.update()
    obj = bpy.data.objects.new(object_name or placement.name, mesh)
    collection.objects.link(obj)
    obj.matrix_local = native_matrix(placement.matrix)
    obj["i76_geo_type"] = placement.geo_type
    obj["i76_geo_flags"] = placement.geo_flags
    obj["i76_parent_name"] = placement.parent
    obj["i76_source_geo"] = geo.name

    uv_layer = mesh.uv_layers.new(name="I76 UV")
    slot_by_material: dict[str, int] = {}
    for polygon, face in zip(mesh.polygons, geo.faces):
        material = make_material(
            face.material,
            face.color,
            face.render_flags,
            store,
            vehicle_prefix,
            images,
            materials,
            appearance,
        )
        if material.name not in slot_by_material:
            slot_by_material[material.name] = len(mesh.materials)
            mesh.materials.append(material)
        polygon.material_index = slot_by_material[material.name]
        polygon.use_smooth = False
        for loop_index, corner in zip(
            range(polygon.loop_start, polygon.loop_start + polygon.loop_total),
            face.corners,
        ):
            uv_layer.data[loop_index].uv = (corner.u, 1.0 - corner.v)
    return obj


def render_clean_preview(objects: list[bpy.types.Object]) -> None:
    bounds = []
    for obj in objects:
        bounds.extend(obj.matrix_world @ Vector(corner) for corner in obj.bound_box)
    center = sum(bounds, Vector()) / len(bounds)
    radius = max((point - center).length for point in bounds)

    camera_data = bpy.data.cameras.new("Temporary Preview Camera")
    camera = bpy.data.objects.new("Temporary Preview Camera", camera_data)
    bpy.context.scene.collection.objects.link(camera)
    camera.location = center + Vector(
        (radius * 2.0, radius * 3.0, radius * 1.2)
    )
    direction = center + Vector((0.0, 0.0, radius * 0.03)) - camera.location
    camera.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
    camera_data.lens = 58
    scene = bpy.context.scene
    scene.camera = camera
    scene.render.engine = "BLENDER_EEVEE_NEXT"
    scene.render.resolution_x = 1400
    scene.render.resolution_y = 900
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.render.filepath = str(RENDER)
    scene.render.film_transparent = False
    if scene.world is None:
        scene.world = bpy.data.worlds.new("World")
    scene.world.color = (0.12, 0.12, 0.12)
    bpy.ops.render.render(write_still=True)
    scene.camera = None
    bpy.data.objects.remove(camera, do_unlink=True)
    bpy.data.cameras.remove(camera_data)


def main() -> None:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    store = AssetStore(ASSETS)
    vdf_path = ASSETS / "vppirnha.vdf"
    placements = parse_vgeo_bank(vdf_path, 0)
    geos = load_geo_pack(store, "vppirnag")
    master = bpy.data.collections.new("Interstate 76 — Picard Piranha")
    body_collection = bpy.data.collections.new("Body — Intact Exterior")
    wheel_collection = bpy.data.collections.new("Wheels — 13in Stock")
    weapon_collection = bpy.data.collections.new("Weapons — Stock Loadout")
    hardpoint_collection = bpy.data.collections.new("Weapon Hardpoints")
    bpy.context.scene.collection.children.link(master)
    master.children.link(body_collection)
    master.children.link(wheel_collection)
    master.children.link(weapon_collection)
    master.children.link(hardpoint_collection)

    root = bpy.data.objects.new("Picard Piranha — Assembly Root", None)
    root.empty_display_type = "PLAIN_AXES"
    root.empty_display_size = 0.35
    master.objects.link(root)

    images: dict[str, bpy.types.Image] = {}
    materials: dict[
        tuple[str, tuple[int, int, int], bool], bpy.types.Material
    ] = {}
    objects: dict[str, bpy.types.Object] = {}

    for placement in placements:
        geo = geos.get(normalized_name(placement.name))
        if geo is None:
            raise RuntimeError(f"missing Piranha GEO: {placement.name}")
        objects[placement.name.lower()] = create_object(
            placement,
            geo,
            store,
            "PP",
            body_collection,
            images,
            materials,
        )
    for placement in placements:
        obj = objects[placement.name.lower()]
        parent = objects.get(placement.parent.lower())
        if parent is not None:
            obj.parent = parent
            obj.matrix_parent_inverse = Matrix.Identity(4)
            obj.matrix_local = native_matrix(placement.matrix)
        else:
            obj.parent = root
            obj.matrix_parent_inverse = Matrix.Identity(4)
            obj.matrix_local = native_matrix(placement.matrix)

    wheel_geos = load_geo_pack(store, "wauto_1g")
    wheel_placements = {
        placement.name.lower(): placement
        for placement in parse_geometry_chunk(ASSETS / "wauto_1a.wdf", b"WGEO")
    }
    wheel_objects: list[bpy.types.Object] = []
    for wheel_index, location in enumerate(parse_wheel_locations(vdf_path), 1):
        native_x = location.matrix[9]
        side = "R" if native_x > 0.0 else "L"
        placement_name = f"WA11{side}WL1"
        placement = wheel_placements[placement_name.lower()]
        geo = wheel_geos[normalized_name(placement_name)]
        axle = "Front" if wheel_index <= 2 else "Rear"
        side_label = "Right" if side == "R" else "Left"
        obj = create_object(
            placement,
            geo,
            store,
            "WA",
            wheel_collection,
            images,
            materials,
            f"{axle} {side_label} Wheel — {placement_name}",
        )
        obj.parent = root
        obj.matrix_parent_inverse = Matrix.Identity(4)
        obj.matrix_local = (
            native_matrix(location.matrix) @ native_matrix(placement.matrix)
        )
        obj["i76_wloc_index"] = wheel_index - 1
        obj["i76_wdf"] = "wauto_1a.wdf"
        wheel_objects.append(obj)

    hardpoints = sorted(parse_hardpoints(vdf_path), key=lambda item: item.index)
    weapon_files = parse_vehicle_weapons(ASSETS / "vppirna1.vcf")
    weapon_objects: list[bpy.types.Object] = []
    for hardpoint, weapon_file in zip(hardpoints, weapon_files):
        locator = bpy.data.objects.new(
            f"Hardpoint {hardpoint.index + 1} — {hardpoint.name}", None
        )
        locator.empty_display_type = "ARROWS"
        locator.empty_display_size = 0.22
        hardpoint_collection.objects.link(locator)
        locator.parent = root
        locator.matrix_parent_inverse = Matrix.Identity(4)
        locator.matrix_local = native_matrix(hardpoint.matrix)
        locator["i76_hloc_name"] = hardpoint.name
        locator["i76_mount_class"] = hardpoint.mount_class
        locator["i76_weapon_class"] = hardpoint.weapon_class
        locator["i76_stock_weapon"] = weapon_file

        mount_match = re.search(r"_G([A-Z])", hardpoint.name.upper())
        mount_group = mount_match.group(1) if mount_match else ""
        weapon_placements = parse_geometry_chunk(
            ASSETS / weapon_file, b"GGEO"
        )
        selected = [
            placement
            for placement in weapon_placements
            if len(placement.name) >= 6
            and placement.name[3].upper() == mount_group
            and placement.name[4:6] == "11"
        ]
        selected_objects: dict[str, bpy.types.Object] = {}
        for placement in selected:
            entry = store.find(placement.name + ".geo")
            if entry is None:
                raise RuntimeError(f"missing weapon GEO: {placement.name}")
            geo = load_geo_pack(store, entry.pack_stem)[
                normalized_name(placement.name)
            ]
            obj = create_object(
                placement,
                geo,
                store,
                placement.name[:2],
                weapon_collection,
                images,
                materials,
                f"Weapon {hardpoint.index + 1} — "
                f"{Path(weapon_file).stem} — {placement.name}",
            )
            obj["i76_gdf"] = weapon_file
            obj["i76_hloc_name"] = hardpoint.name
            selected_objects[placement.name.lower()] = obj
            weapon_objects.append(obj)
        for placement in selected:
            obj = selected_objects[placement.name.lower()]
            parent = selected_objects.get(placement.parent.lower())
            obj.parent = parent if parent is not None else locator
            obj.matrix_parent_inverse = Matrix.Identity(4)
            obj.matrix_local = native_matrix(placement.matrix)

    bpy.context.view_layer.update()
    visible = list(objects.values()) + wheel_objects + weapon_objects

    scene = bpy.context.scene
    scene["i76_vehicle"] = "Picard Piranha"
    scene["i76_appearance"] = "orange/black (piranha1.vtf)"
    scene["i76_vdf"] = "vppirnha.vdf"
    scene["i76_geometry_pack"] = "vppirnag.pak + vppirnag.pix"
    scene["i76_palette"] = "p02.act"
    scene["i76_wheel_definition"] = "wauto_1a.wdf (13in Stock)"
    scene["i76_stock_weapons"] = ", ".join(weapon_files)
    scene["i76_mesh_object_count"] = len(visible)
    scene["i76_textured_material_count"] = sum(
        1 for material in materials.values() if "i76_vqm" in material
    )
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.exposure = 0.0
    scene.view_settings.gamma = 1.0
    if bpy.context.screen is not None:
        for area in bpy.context.screen.areas:
            if area.type == "VIEW_3D":
                area.spaces.active.shading.type = "MATERIAL"

    render_clean_preview(visible)
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND))
    cache_directory = OUTPUT / "_texture_cache"
    for cache_file in cache_directory.glob("*.png"):
        cache_file.unlink()
    cache_directory.rmdir()
    print(
        f"I76_PIRANHA_OK body={len(objects)} wheels={len(wheel_objects)} "
        f"weapons={len(weapon_objects)} meshes={len(visible)} "
        f"materials={len(materials)} images={len(images)} "
        f"blend={BLEND} render={RENDER}"
    )


if __name__ == "__main__":
    main()
