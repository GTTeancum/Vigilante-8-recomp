"""Build a temporary V8:2 guest-roster package with the I76 Marshal over Beezwax.

Run with Blender:

    blender -b I76/NP_ransom_marshal.blend --python tools/i76_marshal_v82_test_package.py

The source .blend is read-only for this script.  Outputs are written under
artifacts/i76_marshal_v82_test and then deployed over the loose guest-roster
mod with timestamped backups.
"""

from __future__ import annotations

from dataclasses import replace
from collections import Counter
import hashlib
import json
import math
import shutil
import sys
from datetime import datetime
from pathlib import Path

import bpy
from mathutils import Vector


ROOT = Path(__file__).resolve().parents[1]
ADDONS = ROOT / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

from vigilante8_vehicle_tools import project, registry, scene_codec  # noqa: E402
from vigilante8_vehicle_tools.authored_scene import new_project  # noqa: E402


SOURCE_BLEND = ROOT / "I76" / "NP_ransom_marshal.blend"
GUEST_OUTPUT = ROOT / "artifacts" / "v8_to_v82_guest_roster"
SOURCE_PROJECTS = GUEST_OUTPUT / "source_projects"
ROUNDTRIP = GUEST_OUTPUT / "blender_roundtrip.json"
OUTPUT = ROOT / "artifacts" / "i76_marshal_v82_test"
FINAL = OUTPUT / "final"
DEPLOY = ROOT / "V8_2_LOOSE" / "mods" / "v8_to_v82_guest_roster"
TARGET_STABLE_ID = "guest.v8.beezwax"
ATLAS_SIZE = 256
MODEL_SCALE = 0.36
BOTTOM_Z = -0.22
SCALE_SHIFT = 8
SLOT_POSITION_SCALE = 128


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def first_difference(expected, actual, path: str = "project") -> str:
    if type(expected) is not type(actual):
        return (
            f"{path}: type {type(expected).__name__} != "
            f"{type(actual).__name__}"
        )
    if isinstance(expected, dict):
        if expected.keys() != actual.keys():
            return (
                f"{path}: keys {sorted(expected.keys())} != "
                f"{sorted(actual.keys())}"
            )
        for key in expected:
            if expected[key] != actual[key]:
                return first_difference(expected[key], actual[key], f"{path}.{key}")
    elif isinstance(expected, list):
        if len(expected) != len(actual):
            return f"{path}: length {len(expected)} != {len(actual)}"
        for index, (expected_item, actual_item) in enumerate(zip(expected, actual)):
            if expected_item != actual_item:
                return first_difference(
                    expected_item, actual_item, f"{path}[{index}]"
                )
    return f"{path}: {expected!r} != {actual!r}"


def bgr555_to_rgb(color: int) -> tuple[int, int, int]:
    return (
        round((color & 0x1F) * 255 / 31),
        round(((color >> 5) & 0x1F) * 255 / 31),
        round(((color >> 10) & 0x1F) * 255 / 31),
    )


def image_pixels_top_down(image: bpy.types.Image) -> list[float]:
    texture = image.copy()
    texture.scale(ATLAS_SIZE, ATLAS_SIZE)
    pixels = [0.0] * (ATLAS_SIZE * ATLAS_SIZE * 4)
    texture.pixels.foreach_get(pixels)
    top_down: list[float] = []
    for output_row in range(ATLAS_SIZE):
        source_row = ATLAS_SIZE - 1 - output_row
        start = source_row * ATLAS_SIZE * 4
        top_down.extend(pixels[start:start + ATLAS_SIZE * 4])
    bpy.data.images.remove(texture)
    return top_down


def marshal_atlas_pixels() -> list[float]:
    images = [
        image
        for image in bpy.data.images
        if "NP_ransom_marshal Atlas" in image.name
    ]
    if len(images) != 1:
        raise RuntimeError(f"expected one Marshal atlas image, found {len(images)}")
    return image_pixels_top_down(images[0])


def marshal_texture() -> project.Texture:
    pixels = marshal_atlas_pixels()
    colors = []
    for offset in range(0, len(pixels), 4):
        red, green, blue, alpha = (
            max(0.0, min(1.0, float(value)))
            for value in pixels[offset:offset + 4]
        )
        if alpha < 0.5:
            red = green = blue = 0.08
        color = (
            round(red * 31.0)
            | (round(green * 31.0) << 5)
            | (round(blue * 31.0) << 10)
        )
        if color == 0:
            color = 0x0421
        colors.append(color)
    counts = Counter(colors)
    palette = [0]
    palette.extend(color for color, _count in counts.most_common(255))
    palette.extend([0] * (256 - len(palette)))
    exact = {color: index for index, color in enumerate(palette) if index != 0}

    def distance(first: int, second: int) -> int:
        return sum(
            (
                ((first >> shift) & 0x1F)
                - ((second >> shift) & 0x1F)
            )
            ** 2
            for shift in (0, 5, 10)
        )

    usable_indices = range(1, 256)
    indices = bytes(
        exact[color]
        if color in exact
        else min(
            usable_indices,
            key=lambda index: distance(color, palette[index]),
        )
        for color in colors
    )
    return project.Texture(
        name="texture_000",
        width=ATLAS_SIZE,
        height=ATLAS_SIZE,
        depth=1,
        palette_bgr555=tuple(palette),
        indices=indices,
        compressed=False,
    )


def reverse_triangle_with_uv(
    indices: tuple[int, int, int],
    uv: tuple[tuple[int, int], tuple[int, int], tuple[int, int]],
) -> tuple[tuple[int, int, int], tuple[tuple[int, int], tuple[int, int], tuple[int, int]]]:
    return (indices[0], indices[2], indices[1]), (uv[0], uv[2], uv[1])


def native_normal(
    vertices: tuple[tuple[int, int, int], ...],
    face_vertices: tuple[int, int, int],
) -> tuple[int, int, int, int]:
    a, b, c = (vertices[index] for index in face_vertices)
    ab = tuple(b[axis] - a[axis] for axis in range(3))
    ac = tuple(c[axis] - a[axis] for axis in range(3))
    normal = (
        ab[1] * ac[2] - ab[2] * ac[1],
        ab[2] * ac[0] - ab[0] * ac[2],
        ab[0] * ac[1] - ab[1] * ac[0],
    )
    magnitude = math.sqrt(sum(component * component for component in normal))
    if magnitude == 0.0:
        return 0, -4096, 0, 0
    return tuple(
        max(-32768, min(32767, round(component * 4096.0 / magnitude)))
        for component in normal
    ) + (0,)


def uv_pair(layer, loop_index: int) -> tuple[int, int]:
    uv = layer.data[loop_index].uv if layer is not None else (0.0, 0.0)
    u = max(0.0, min(1.0, float(uv[0])))
    v = max(0.0, min(1.0, float(uv[1])))
    return round(u * 255), round((1.0 - v) * 255)


def object_vertices(
    objects: list[bpy.types.Object],
) -> tuple[list[Vector], Vector, float]:
    points: list[Vector] = []
    for obj in objects:
        points.extend(obj.matrix_world @ vertex.co for vertex in obj.data.vertices)
    if not points:
        raise RuntimeError("Marshal body/glass meshes have no vertices")
    minimum = Vector((min(point[0] for point in points),
                      min(point[1] for point in points),
                      min(point[2] for point in points)))
    maximum = Vector((max(point[0] for point in points),
                      max(point[1] for point in points),
                      max(point[2] for point in points)))
    center = Vector((
        (minimum.x + maximum.x) * 0.5,
        (minimum.y + maximum.y) * 0.5,
        minimum.z,
    ))
    return points, center, max((point - center).length for point in points)


def native_vertex(point: Vector, center: Vector) -> tuple[int, int, int]:
    local = Vector((
        (point.x - center.x) * MODEL_SCALE,
        (point.y - center.y) * MODEL_SCALE,
        (point.z - center.z) * MODEL_SCALE + BOTTOM_Z,
    ))
    return scene_codec.blender_vertex_to_native(local, SCALE_SHIFT)[:3]


def marshal_group() -> tuple[project.RenderGroup, project.CollisionStream, dict]:
    body = bpy.data.objects.get("RM11BDYM")
    glass = bpy.data.objects.get("GLASS")
    if body is None or glass is None:
        raise RuntimeError("expected RM11BDYM body mesh and GLASS mesh")
    if body.type != "MESH" or glass.type != "MESH":
        raise RuntimeError("RM11BDYM and GLASS must both be mesh objects")

    source_objects = [body, glass]
    _points, center, radius = object_vertices(source_objects)
    vertices: list[tuple[int, int, int]] = []
    faces: list[project.Face] = []
    normals: list[tuple[int, int, int, int]] = []
    mesh_stats: dict[str, int] = {}

    for obj in source_objects:
        mesh = obj.data
        mesh.calc_loop_triangles()
        uv_layer = mesh.uv_layers.active
        base_vertex = len(vertices)
        vertices.extend(
            native_vertex(obj.matrix_world @ vertex.co, center)
            for vertex in mesh.vertices
        )
        mesh_stats[obj.name] = len(mesh.loop_triangles)

        for triangle in mesh.loop_triangles:
            indices = tuple(base_vertex + int(index) for index in triangle.vertices)
            if obj.name == "GLASS":
                uv = tuple(uv_pair(uv_layer, loop) for loop in triangle.loops)
                indices, _uv = reverse_triangle_with_uv(indices, uv)
                normal_index = len(normals)
                normals.append(native_normal(tuple(vertices), indices))
                normal_triplet = (normal_index, normal_index, normal_index)
                faces.append(
                    project.Face(
                        vertices=indices,
                        color=(80, 96, 104),
                        packet_kind=12,
                        packet_flags=0,
                        normal_indices=normal_triplet,
                        environment_parameters=(0x3FFF, 0x8080, 0, 0),
                    )
                )
                continue

            uv = tuple(uv_pair(uv_layer, loop) for loop in triangle.loops)
            indices, uv = reverse_triangle_with_uv(indices, uv)
            faces.append(
                project.Face(
                    vertices=indices,
                    color=(128, 128, 128),
                    texture=0,
                    native_texture_slot=0,
                    uv=uv,
                    packet_kind=13,
                )
            )

    indexed_faces = tuple(
        replace(face, packet_index=index) for index, face in enumerate(faces)
    )
    group = project.RenderGroup(
        name="group_000",
        scale_shift=SCALE_SHIFT,
        vertices=tuple(vertices),
        faces=indexed_faces,
        normals=tuple(normals),
        texture_slot_count=0,
        render_extent=max(1, round(radius * MODEL_SCALE * (1 << SCALE_SHIFT))),
    )
    xs = [vertex[0] for vertex in vertices]
    ys = [vertex[1] for vertex in vertices]
    zs = [vertex[2] for vertex in vertices]
    collision = project.CollisionStream(
        "collision_000",
        (
            project.CollisionAabb(
                (min(xs), min(ys), min(zs)),
                (max(xs), max(ys), max(zs)),
                min(ys),
            ),
        ),
    )
    stats = {
        "vertices": len(vertices),
        "faces": len(faces),
        "glass_source_triangles": mesh_stats.get("GLASS", 0),
        "body_source_triangles": mesh_stats.get("RM11BDYM", 0),
        "render_extent": group.render_extent,
        "bbox": [min(xs), max(xs), min(ys), max(ys), min(zs), max(zs)],
    }
    return group, collision, stats


def marshal_slots(stats: dict) -> tuple[project.Slot, ...]:
    min_x, max_x, min_y, _max_y, min_z, max_z = stats["bbox"]
    wheel_x = round(max(abs(min_x), abs(max_x)) * SLOT_POSITION_SCALE * 0.92)
    wheel_y = round((min_y + 72) * SLOT_POSITION_SCALE)
    front_z = round(max_z * SLOT_POSITION_SCALE * 0.72)
    rear_z = round(min_z * SLOT_POSITION_SCALE * 0.72)
    slots: list[project.Slot] = [
        project.Slot("slot_000", 0, 0),
        project.Slot(
            "slot_001",
            None,
            None,
            key=0x8000,
            position=(-wheel_x, wheel_y, front_z),
            parent=0,
        ),
        project.Slot(
            "slot_002",
            None,
            None,
            key=0x8001,
            position=(wheel_x, wheel_y, front_z),
            parent=0,
        ),
        project.Slot(
            "slot_003",
            None,
            None,
            key=0x8002,
            position=(-wheel_x, wheel_y, rear_z),
            parent=0,
        ),
        project.Slot(
            "slot_004",
            None,
            None,
            key=0x8003,
            position=(wheel_x, wheel_y, rear_z),
            parent=0,
        ),
        project.Slot(
            "slot_005",
            None,
            None,
            key=0x8100,
            position=(0, -65536, -65536),
            parent=0,
        ),
        project.Slot("slot_006", None, None),
    ]
    for index in range(7):
        slots.append(
            project.Slot(
                f"slot_{index + 7:03d}",
                None,
                None,
                key=0x8010 + index,
                parent=0,
            )
        )
    slots.append(
        project.Slot("slot_014", None, None, key=0x801F, parent=0)
    )
    slots.append(
        project.Slot(
            "slot_015",
            None,
            None,
            key=0x8101,
            parent=0,
        )
    )
    stats["wheel_anchor_positions"] = {
        "front_left": slots[1].position,
        "front_right": slots[2].position,
        "rear_left": slots[3].position,
        "rear_right": slots[4].position,
    }
    return tuple(slots)


def ordered_source_projects() -> list[project.VehicleProject]:
    proof = json.loads(ROUNDTRIP.read_text(encoding="utf-8"))
    result = []
    for entry in proof["vehicles"]:
        stable_id = entry["stable_id"]
        path = SOURCE_PROJECTS / f"{stable_id}.json"
        result.append(
            project.VehicleProject.from_dict(
                json.loads(path.read_text(encoding="utf-8"))
            )
        )
    return result


def marshal_project(beezwax: project.VehicleProject) -> tuple[project.VehicleProject, dict]:
    group, collision, stats = marshal_group()
    texture = marshal_texture()
    base = new_project("V8_2", TARGET_STABLE_ID)
    slots = marshal_slots(stats)
    selector_preview = project.ObjectBank(
        groups=(group,),
        slots=slots,
        collisions=(collision,),
        textures=(texture,),
        animations=(),
    )

    vehicle = replace(
        base,
        stable_id=TARGET_STABLE_ID,
        display_name="I76 Ransom Marshal Test",
        groups=(group,),
        slots=slots,
        collisions=(collision,),
        textures=(texture,),
        stats=beezwax.stats,
        body_kind=0,
        transformation_bank=beezwax.transformation_bank,
        transform_modes=beezwax.transform_modes,
        powerups=beezwax.powerups,
        selector_preview_bank=selector_preview,
        selector_preview_body_kind=0,
    )
    vehicle.validate()
    texture_memory = project.bank_memory_usage(vehicle)
    stats.update(
        {
            "texture": {
                "width": texture.width,
                "height": texture.height,
                "depth": texture.depth,
                "palette": len(texture.palette_bgr555),
                "native_bytes": texture_memory.native_texture_bytes,
            },
            "glass_native_faces": stats["glass_source_triangles"],
        }
    )
    return vehicle, stats


def write_package(vehicles: list[project.VehicleProject], stats: dict) -> dict:
    package = registry.compile_package(vehicles)
    decoded = registry.decompile_package(package.archive, package.registry)
    expected = [project.to_dict(vehicle) for vehicle in vehicles]
    actual = [project.to_dict(vehicle) for vehicle in decoded]
    if actual != expected:
        raise RuntimeError(
            "compiled Marshal test package changed on decode: "
            + first_difference(expected, actual)
        )

    FINAL.mkdir(parents=True, exist_ok=True)
    archive_path = FINAL / "CUSTOM.EXP"
    registry_path = FINAL / "VEHICLES.V8R"
    archive_path.write_bytes(package.archive)
    registry_path.write_bytes(package.registry)
    selector_output = FINAL / "SHELL"
    selector_output.mkdir(parents=True, exist_ok=True)
    for selector in (ROOT / "V8_2_LOOSE" / "SHELL").glob("SELECTOR_*.PPM"):
        shutil.copy2(selector, selector_output / selector.name)

    return {
        "built_at": datetime.now().astimezone().isoformat(),
        "target_stable_id": TARGET_STABLE_ID,
        "source_blend": str(SOURCE_BLEND.relative_to(ROOT)),
        "final": {
            "CUSTOM.EXP": {
                "path": str(archive_path.relative_to(ROOT)),
                "bytes": len(package.archive),
                "sha256": digest(package.archive),
            },
            "VEHICLES.V8R": {
                "path": str(registry_path.relative_to(ROOT)),
                "bytes": len(package.registry),
                "sha256": digest(package.registry),
            },
        },
        "marshal": stats,
    }


def backup_and_deploy(manifest: dict) -> None:
    stamp = datetime.now().astimezone().strftime("%Y%m%d_%H%M%S")
    backup_dir = OUTPUT / "backups" / stamp
    backup_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(SOURCE_BLEND, backup_dir / SOURCE_BLEND.name)
    for name in ("CUSTOM.EXP", "VEHICLES.V8R"):
        source = DEPLOY / name
        if source.exists():
            shutil.copy2(source, backup_dir / name)
        shutil.copy2(FINAL / name, DEPLOY / name)
    manifest["deployed_to"] = str(DEPLOY.relative_to(ROOT))
    manifest["backup_dir"] = str(backup_dir.relative_to(ROOT))


def main() -> None:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    projects = ordered_source_projects()
    beezwax = next(
        vehicle for vehicle in projects if vehicle.stable_id == TARGET_STABLE_ID
    )
    replacement, stats = marshal_project(beezwax)
    vehicles = [
        replacement if vehicle.stable_id == TARGET_STABLE_ID else vehicle
        for vehicle in projects
    ]
    manifest = write_package(vehicles, stats)
    backup_and_deploy(manifest)
    (OUTPUT / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n",
        encoding="utf-8",
    )
    print(
        "I76_MARSHAL_V82_TEST_OK "
        f"vehicles={len(vehicles)} "
        f"body_tris={stats['body_source_triangles']} "
        f"glass_tris={stats['glass_source_triangles']} "
        f"native_faces={stats['faces']} "
        f"backup={manifest['backup_dir']}"
    )


if __name__ == "__main__":
    main()
