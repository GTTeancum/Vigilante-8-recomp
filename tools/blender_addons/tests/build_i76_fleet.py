#!/usr/bin/env python3
"""Build packed, conservatively upscaled Interstate '76 Blender vehicles."""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

import bpy
from mathutils import Matrix


ROOT = Path(__file__).resolve().parents[3]
TOOLS = ROOT / "tools"
SCRIPT_DIRECTORY = Path(__file__).resolve().parent
for import_path in (TOOLS, SCRIPT_DIRECTORY):
    if str(import_path) not in sys.path:
        sys.path.insert(0, str(import_path))

import build_i76_piranha as blender_import  # noqa: E402
from i76_vehicle_import import (  # noqa: E402
    AssetStore,
    normalized_name,
    parse_geo,
    parse_geometry_chunk,
    parse_hardpoints,
    parse_vehicle_config,
    parse_vehicle_name,
    parse_vgeo_bank,
    parse_wheel_locations,
)


ASSETS = ROOT / "I76_ZFS"
OUTPUT = ROOT / "I76"
FILE_PREFIX = "I76"
EXPECTED_COUNT = 32
INCLUDED_VDFS: set[str] | None = None
UPSCALE_TEXTURES = True
UPSCALE_FACTOR = 2
EXCLUDED_ARTICULATED_VDFS = {"vxcargo.vdf", "vxtanker.vdf"}
REALESRGAN_DIRECTORY = ROOT / "build" / "i76_realesrgan"
REALESRGAN_EXECUTABLE = REALESRGAN_DIRECTORY / "realesrgan-ncnn-vulkan.exe"
TEXTURE_FIDELITY_SCRIPT = ROOT / "tools" / "i76_texture_upscale.py"


def common_prefix_length(left: str, right: str) -> int:
    count = 0
    for left_character, right_character in zip(left, right):
        if left_character != right_character:
            break
        count += 1
    return count


def selected_vdf_paths() -> list[Path]:
    paths = sorted(ASSETS.glob("v*.vdf"))
    if INCLUDED_VDFS is not None:
        return [
            path
            for path in paths
            if path.name.lower() in INCLUDED_VDFS
        ]
    return [
        path
        for path in paths
        if path.name.lower() not in EXCLUDED_ARTICULATED_VDFS
    ]


def select_default_configs() -> dict[str, tuple[Path, object]]:
    candidates: dict[str, list[tuple[Path, object]]] = {}
    for config_path in sorted(ASSETS.glob("*.vcf")):
        config = parse_vehicle_config(config_path)
        candidates.setdefault(config.vehicle_definition.lower(), []).append(
            (config_path, config)
        )

    selected: dict[str, tuple[Path, object]] = {}
    for vdf_path in selected_vdf_paths():
        key = vdf_path.name.lower()
        matches = candidates.get(key, [])
        if not matches:
            raise RuntimeError(f"{vdf_path.name}: no VCF configuration found")

        def preference(item: tuple[Path, object]) -> tuple[int, int, int, int]:
            stem = item[0].stem.lower()
            base = stem.rstrip("0123456789")
            return (
                int(stem.startswith("v")),
                int(stem.endswith("1")),
                common_prefix_length(base, vdf_path.stem.lower()),
                -len(stem),
            )

        selected[key] = max(matches, key=preference)
    if len(selected) != EXPECTED_COUNT:
        raise RuntimeError(
            f"expected {EXPECTED_COUNT} vehicles, found {len(selected)}"
        )
    return selected


def appearance_number(appearance_definition: str) -> int:
    match = re.search(r"(\d)$", Path(appearance_definition).stem)
    if match is None:
        raise RuntimeError(
            f"cannot identify appearance number: {appearance_definition}"
        )
    return int(match.group(1))


def slug(value: str) -> str:
    source_name_corrections = {
        "phaedra palamino": "phaedra palomino",
        "picard jackrabit": "picard jackrabbit",
    }
    corrected = source_name_corrections.get(value.lower(), value.lower())
    return re.sub(r"[^a-z0-9]+", "_", corrected).strip("_")


def load_named_geo(store: AssetStore, name: str):
    entry = store.find(name + ".geo")
    if entry is None:
        raise RuntimeError(f"missing GEO: {name}")
    return parse_geo(store.payload(entry))


def primary_wheel_placement(wdf_path: Path, side: str):
    placements = parse_geometry_chunk(wdf_path, b"WGEO")
    matches = [
        placement
        for placement in placements
        if len(placement.name) >= 7
        and placement.name[2:4] == "11"
        and placement.name[4].upper() == side
        and placement.name[5:7].upper() == "WL"
    ]
    if len(matches) != 1:
        raise RuntimeError(
            f"{wdf_path.name}: expected one primary {side} wheel, "
            f"found {len(matches)}"
        )
    return matches[0]


def clear_texture_cache() -> None:
    cache_directory = OUTPUT / "_texture_cache"
    if not cache_directory.exists():
        return
    for cache_file in cache_directory.glob("*.png"):
        cache_file.unlink()
    cache_directory.rmdir()


def copy_custom_properties(
    source: bpy.types.ID,
    destination: bpy.types.ID,
) -> None:
    for key in source.keys():
        destination[key] = source[key]


def upscale_vehicle_textures(
    images: dict[str, bpy.types.Image],
    vehicle_slug: str,
) -> int:
    if not UPSCALE_TEXTURES or not images:
        return 0
    if not REALESRGAN_EXECUTABLE.is_file():
        raise RuntimeError(f"missing Real-ESRGAN: {REALESRGAN_EXECUTABLE}")
    python_executable = shutil.which("python")
    if python_executable is None:
        raise RuntimeError("system Python was not found")

    source_directory = OUTPUT / "_texture_cache"
    build_directory = ROOT / "build"
    build_directory.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(
        prefix=f"i76_{vehicle_slug}_",
        dir=build_directory,
    ) as temporary:
        temporary_path = Path(temporary)
        neural_directory = temporary_path / "neural"
        fidelity_directory = temporary_path / "fidelity"
        neural_directory.mkdir()
        subprocess.run(
            [
                str(REALESRGAN_EXECUTABLE),
                "-i",
                str(source_directory),
                "-o",
                str(neural_directory),
                "-n",
                "realesr-animevideov3",
                "-s",
                str(UPSCALE_FACTOR),
                "-t",
                "64",
                "-f",
                "png",
            ],
            cwd=REALESRGAN_DIRECTORY,
            check=True,
        )
        subprocess.run(
            [
                python_executable,
                str(TEXTURE_FIDELITY_SCRIPT),
                "--source",
                str(source_directory),
                "--esrgan",
                str(neural_directory),
                "--out",
                str(fidelity_directory),
                "--scale",
                str(UPSCALE_FACTOR),
                "--edge-threshold",
                "35",
                "--edge-strength",
                "0.85",
            ],
            cwd=ROOT,
            check=True,
        )

        old_images = list(images.values())
        for old_image in old_images:
            filename = Path(old_image.filepath_raw).name
            replacement_path = fidelity_directory / filename
            if not replacement_path.is_file():
                raise RuntimeError(f"missing upscaled image: {replacement_path}")
            original_size = tuple(old_image.size)
            new_image = bpy.data.images.load(
                str(replacement_path),
                check_existing=False,
            )
            expected_size = tuple(
                dimension * UPSCALE_FACTOR for dimension in original_size
            )
            if tuple(new_image.size) != expected_size:
                raise RuntimeError(
                    f"{filename}: got {tuple(new_image.size)}, "
                    f"expected {expected_size}"
                )
            new_image.name = (
                f"{old_image.name} [Real-ESRGAN {UPSCALE_FACTOR}x]"
            )
            new_image.colorspace_settings.name = (
                old_image.colorspace_settings.name
            )
            new_image.alpha_mode = "STRAIGHT"
            copy_custom_properties(old_image, new_image)
            new_image["i76_upscaler"] = (
                "realesr-animevideov3 + source-edge preservation"
            )
            new_image["i76_upscale_factor"] = UPSCALE_FACTOR
            new_image["i76_original_width"] = original_size[0]
            new_image["i76_original_height"] = original_size[1]
            new_image.pack()

            for material in bpy.data.materials:
                if not material.use_nodes:
                    continue
                for node in material.node_tree.nodes:
                    if node.type == "TEX_IMAGE" and node.image == old_image:
                        node.image = new_image
            bpy.data.images.remove(old_image)
    return len(images)


def build_vehicle(
    store: AssetStore,
    ordinal: int,
    vdf_path: Path,
    config_path: Path,
    config,
) -> dict[str, object]:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    blender_import.OUTPUT = OUTPUT
    blender_import.ASSETS = ASSETS

    vehicle_name = parse_vehicle_name(vdf_path)
    appearance = appearance_number(config.appearance_definition)
    placements = parse_vgeo_bank(vdf_path, 0)
    if not placements:
        raise RuntimeError(f"{vdf_path.name}: intact geometry bank is empty")
    vehicle_prefix = placements[0].name[:2]

    master = bpy.data.collections.new(f"Interstate 76 - {vehicle_name}")
    body_collection = bpy.data.collections.new("Body - Intact Exterior")
    wheel_collection = bpy.data.collections.new("Wheels - Default Configuration")
    weapon_collection = bpy.data.collections.new("Weapons - Default Configuration")
    hardpoint_collection = bpy.data.collections.new("Weapon Hardpoints")
    bpy.context.scene.collection.children.link(master)
    for collection in (
        body_collection,
        wheel_collection,
        weapon_collection,
        hardpoint_collection,
    ):
        master.children.link(collection)

    root = bpy.data.objects.new(f"{vehicle_name} - Assembly Root", None)
    root.empty_display_type = "PLAIN_AXES"
    root.empty_display_size = 0.35
    master.objects.link(root)

    images: dict[str, bpy.types.Image] = {}
    materials: dict[
        tuple[str, tuple[int, int, int], bool], bpy.types.Material
    ] = {}
    body_objects: dict[str, bpy.types.Object] = {}

    for placement in placements:
        geo = load_named_geo(store, placement.name)
        body_objects[normalized_name(placement.name)] = (
            blender_import.create_object(
                placement,
                geo,
                store,
                vehicle_prefix,
                body_collection,
                images,
                materials,
                appearance=appearance,
            )
        )
    for placement in placements:
        obj = body_objects[normalized_name(placement.name)]
        parent = body_objects.get(normalized_name(placement.parent))
        obj.parent = parent if parent is not None else root
        obj.matrix_parent_inverse = Matrix.Identity(4)
        obj.matrix_local = blender_import.native_matrix(placement.matrix)

    wheel_objects: list[bpy.types.Object] = []
    wheel_locations = parse_wheel_locations(vdf_path)
    front_wdf, middle_wdf, rear_wdf = config.wheel_definitions
    for wheel_index, location in enumerate(wheel_locations):
        if wheel_index < 2:
            wheel_definition = front_wdf
        elif middle_wdf is not None and wheel_index < 4:
            wheel_definition = middle_wdf
        else:
            wheel_definition = rear_wdf
        if wheel_definition is None:
            continue
        wheel_path = ASSETS / wheel_definition
        if not wheel_path.is_file():
            raise RuntimeError(
                f"{vdf_path.name}: missing wheel definition {wheel_definition}"
            )
        side = "R" if location.matrix[9] > 0.0 else "L"
        placement = primary_wheel_placement(wheel_path, side)
        geo = load_named_geo(store, placement.name)
        axle = "Front" if wheel_index < 2 else "Rear"
        side_label = "Right" if side == "R" else "Left"
        obj = blender_import.create_object(
            placement,
            geo,
            store,
            placement.name[:2],
            wheel_collection,
            images,
            materials,
            f"{axle} {side_label} Wheel - {placement.name}",
            appearance=1,
        )
        obj.parent = root
        obj.matrix_parent_inverse = Matrix.Identity(4)
        obj.matrix_local = (
            blender_import.native_matrix(location.matrix)
            @ blender_import.native_matrix(placement.matrix)
        )
        obj["i76_wloc_index"] = wheel_index
        obj["i76_wdf"] = wheel_definition
        wheel_objects.append(obj)

    hardpoints = sorted(parse_hardpoints(vdf_path), key=lambda item: item.index)
    weapon_objects: list[bpy.types.Object] = []
    for hardpoint_offset, hardpoint in enumerate(hardpoints):
        weapon_file = (
            config.weapons[hardpoint_offset]
            if hardpoint_offset < len(config.weapons)
            else ""
        )
        locator = bpy.data.objects.new(
            f"Hardpoint {hardpoint.index + 1} - {hardpoint.name}", None
        )
        locator.empty_display_type = "ARROWS"
        locator.empty_display_size = 0.22
        hardpoint_collection.objects.link(locator)
        locator.parent = root
        locator.matrix_parent_inverse = Matrix.Identity(4)
        locator.matrix_local = blender_import.native_matrix(hardpoint.matrix)
        locator["i76_hloc_name"] = hardpoint.name
        locator["i76_mount_class"] = hardpoint.mount_class
        locator["i76_weapon_class"] = hardpoint.weapon_class
        locator["i76_stock_weapon"] = weapon_file
        if not weapon_file:
            continue

        weapon_path = ASSETS / weapon_file
        if not weapon_path.is_file():
            raise RuntimeError(f"missing weapon definition: {weapon_file}")
        mount_match = re.search(r"_G([A-Z])", hardpoint.name.upper())
        mount_group = mount_match.group(1) if mount_match else ""
        selected = [
            placement
            for placement in parse_geometry_chunk(weapon_path, b"GGEO")
            if len(placement.name) >= 6
            and placement.name[3].upper() == mount_group
            and placement.name[4:6] == "11"
        ]
        if not selected:
            selected = [
                placement
                for placement in parse_geometry_chunk(weapon_path, b"GGEO")
                if len(placement.name) >= 6
                and placement.name[4:6] == "11"
            ]
        selected_objects: dict[str, bpy.types.Object] = {}
        for placement in selected:
            geo = load_named_geo(store, placement.name)
            obj = blender_import.create_object(
                placement,
                geo,
                store,
                placement.name[:2],
                weapon_collection,
                images,
                materials,
                f"Weapon {hardpoint.index + 1} - "
                f"{Path(weapon_file).stem} - {placement.name}",
                appearance=1,
            )
            obj["i76_gdf"] = weapon_file
            obj["i76_hloc_name"] = hardpoint.name
            selected_objects[normalized_name(placement.name)] = obj
            weapon_objects.append(obj)
        for placement in selected:
            obj = selected_objects[normalized_name(placement.name)]
            parent = selected_objects.get(normalized_name(placement.parent))
            obj.parent = parent if parent is not None else locator
            obj.matrix_parent_inverse = Matrix.Identity(4)
            obj.matrix_local = blender_import.native_matrix(placement.matrix)

    bpy.context.view_layer.update()
    upscaled_image_count = upscale_vehicle_textures(
        images,
        slug(vehicle_name),
    )
    scene = bpy.context.scene
    visible_meshes = (
        list(body_objects.values()) + wheel_objects + weapon_objects
    )
    scene["i76_vehicle"] = vehicle_name
    scene["i76_vdf"] = vdf_path.name
    scene["i76_default_config"] = config_path.name
    scene["i76_appearance"] = config.appearance_definition
    scene["i76_wheel_definitions"] = ", ".join(
        value or "null" for value in config.wheel_definitions
    )
    scene["i76_stock_weapons"] = ", ".join(config.weapons)
    scene["i76_mesh_object_count"] = len(visible_meshes)
    scene["i76_body_mesh_count"] = len(body_objects)
    scene["i76_wheel_mesh_count"] = len(wheel_objects)
    scene["i76_weapon_mesh_count"] = len(weapon_objects)
    scene["i76_packed_image_count"] = len(images)
    scene["i76_source_release"] = FILE_PREFIX
    scene["i76_texture_upscale_factor"] = (
        UPSCALE_FACTOR if UPSCALE_TEXTURES else 1
    )
    scene["i76_upscaled_texture_count"] = upscaled_image_count
    scene.view_settings.view_transform = "Standard"
    scene.view_settings.exposure = 0.0
    scene.view_settings.gamma = 1.0

    blend_path = OUTPUT / f"{FILE_PREFIX}_{slug(vehicle_name)}.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    backup_path = Path(str(blend_path) + "1")
    if backup_path.exists():
        backup_path.unlink()

    if any(image.packed_file is None for image in bpy.data.images):
        raise RuntimeError(f"{blend_path.name}: an image was not packed")
    if any(obj.type in {"CAMERA", "LIGHT"} for obj in bpy.data.objects):
        raise RuntimeError(f"{blend_path.name}: contains presentation objects")
    clear_texture_cache()
    print(
        f"I76_FLEET_BUILT {ordinal:02d}/{EXPECTED_COUNT} {vehicle_name} "
        f"body={len(body_objects)} wheels={len(wheel_objects)} "
        f"weapons={len(weapon_objects)} images={len(images)} "
        f"blend={blend_path.name}"
    )
    return {
        "ordinal": ordinal,
        "vehicle": vehicle_name,
        "blend": blend_path.name,
        "vdf": vdf_path.name,
        "config": config_path.name,
        "appearance": config.appearance_definition,
        "body_meshes": len(body_objects),
        "wheel_meshes": len(wheel_objects),
        "weapon_meshes": len(weapon_objects),
        "packed_images": len(images),
    }


def build_fleet() -> None:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    clear_texture_cache()
    store = AssetStore(ASSETS)
    configs = select_default_configs()
    records: list[dict[str, object]] = []
    vdf_paths = selected_vdf_paths()
    for ordinal, vdf_path in enumerate(vdf_paths, 1):
        config_path, config = configs[vdf_path.name.lower()]
        records.append(
            build_vehicle(
                store,
                ordinal,
                vdf_path,
                config_path,
                config,
            )
        )
    manifest = {
        "format": "Interstate 76 Blender fleet",
        "vehicle_count": len(records),
        "textures": "packed into each blend",
        "excluded_articulated_assets": sorted(EXCLUDED_ARTICULATED_VDFS),
        "vehicles": records,
    }
    manifest_path = ROOT / "build" / f"{FILE_PREFIX.lower()}_fleet_manifest.json"
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    print(f"I76_FLEET_OK vehicles={len(records)} output={OUTPUT}")


def validate_fleet() -> None:
    blend_paths = sorted(OUTPUT.glob(f"{FILE_PREFIX}_*.blend"))
    if len(blend_paths) != EXPECTED_COUNT:
        raise RuntimeError(
            f"expected {EXPECTED_COUNT} {FILE_PREFIX} blend files, "
            f"found {len(blend_paths)}"
        )
    total_meshes = 0
    total_images = 0
    total_cutouts = 0
    for blend_path in blend_paths:
        bpy.ops.wm.open_mainfile(filepath=str(blend_path))
        meshes = [obj for obj in bpy.data.objects if obj.type == "MESH"]
        images = list(bpy.data.images)
        if len(meshes) != bpy.context.scene["i76_mesh_object_count"]:
            raise RuntimeError(f"{blend_path.name}: mesh count changed on reopen")
        if any(image.packed_file is None for image in images):
            raise RuntimeError(f"{blend_path.name}: unpacked image on reopen")
        if any(obj.type in {"CAMERA", "LIGHT"} for obj in bpy.data.objects):
            raise RuntimeError(f"{blend_path.name}: camera/light found on reopen")
        for image in images:
            if image.size[0] != image["i76_original_width"] * UPSCALE_FACTOR:
                raise RuntimeError(f"{blend_path.name}: image width is not 2x")
            if image.size[1] != image["i76_original_height"] * UPSCALE_FACTOR:
                raise RuntimeError(f"{blend_path.name}: image height is not 2x")
            if "i76_transparent_palette_index" in image:
                alpha = list(image.pixels)[3::4]
                if min(alpha) > 0.05 or max(alpha) < 0.95:
                    raise RuntimeError(
                        f"{blend_path.name}: cutout alpha was lost"
                    )
                total_cutouts += 1
        total_meshes += len(meshes)
        total_images += len(images)
        print(
            f"I76_FLEET_VALID {blend_path.name} "
            f"meshes={len(meshes)} packed_images={len(images)}"
        )
    print(
        f"I76_FLEET_VALIDATION_OK vehicles={len(blend_paths)} "
        f"meshes={total_meshes} packed_images={total_images} "
        f"cutouts={total_cutouts}"
    )


if __name__ == "__main__":
    arguments = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--assets", type=Path, default=ASSETS)
    parser.add_argument("--output", type=Path, default=OUTPUT)
    parser.add_argument("--prefix", default=FILE_PREFIX)
    parser.add_argument("--expected", type=int, default=EXPECTED_COUNT)
    parser.add_argument("--include-vdf", action="append")
    parser.add_argument("--validate", action="store_true")
    parser.add_argument("--no-upscale", action="store_true")
    options = parser.parse_args(arguments)
    ASSETS = options.assets.resolve()
    OUTPUT = options.output.resolve()
    FILE_PREFIX = options.prefix
    EXPECTED_COUNT = options.expected
    INCLUDED_VDFS = (
        {name.lower() for name in options.include_vdf}
        if options.include_vdf
        else None
    )
    UPSCALE_TEXTURES = not options.no_upscale
    if options.validate:
        validate_fleet()
    else:
        build_fleet()
