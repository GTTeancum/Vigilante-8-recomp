"""Blender operators and panels for native Vigilante 8 vehicle data."""

from __future__ import annotations

import json
import hashlib
import math
from pathlib import Path
from typing import Iterable

import bpy
from bpy.props import (
    BoolProperty,
    CollectionProperty,
    EnumProperty,
    IntProperty,
    StringProperty,
)
from bpy.types import Operator, Panel, PropertyGroup
from bpy_extras.io_utils import ExportHelper, ImportHelper

from . import archive, authored_scene, project, registry, scene_codec, stats, xobf


ADDON_PREFIX = "V8 Native"
TRANSFORM_MODE_NAMES = ("Standard", "Hover", "Float", "Ski")
WHEEL_NAMES = ("Front L", "Front R", "Middle L", "Middle R", "Rear L", "Rear R")


def _unique_name(base: str, names: Iterable[str]) -> str:
    occupied = set(names)
    if base not in occupied:
        return base
    suffix = 1
    while f"{base}.{suffix:03d}" in occupied:
        suffix += 1
    return f"{base}.{suffix:03d}"


def _store_source(label: str, data: bytes) -> str:
    name = _unique_name(f".{ADDON_PREFIX} {label}", bpy.data.texts.keys())
    block = bpy.data.texts.new(name)
    encoded = scene_codec.encode_source(data)
    block.write("\n".join(encoded[index : index + 76] for index in range(0, len(encoded), 76)))
    block["v8_embedded_native_source"] = True
    block["v8_source_bytes"] = len(data)
    return block.name


def _load_source(name: str) -> bytes:
    block = bpy.data.texts.get(name)
    if block is None:
        raise ValueError(f"embedded source text {name!r} is missing")
    return scene_codec.decode_source(block.as_string())


def _active_vehicle_collection(context) -> bpy.types.Collection | None:
    settings = context.scene.v8_vehicle_settings
    if settings.vehicle_collection:
        collection = bpy.data.collections.get(settings.vehicle_collection)
        if collection is not None and collection.get("v8_role") in {
            "vehicle_entry",
            authored_scene.ROLE_VEHICLE,
        }:
            return collection
    obj = context.active_object
    if obj is not None:
        for collection in obj.users_collection:
            if collection.get("v8_role") in {
                "vehicle_entry",
                authored_scene.ROLE_VEHICLE,
            }:
                return collection
    return None


def _link_collection(parent: bpy.types.Collection, child: bpy.types.Collection) -> None:
    if child.name not in parent.children:
        parent.children.link(child)


def _texture_pixels(texture: xobf.TextureSlot) -> list[float]:
    source = scene_codec.texture_rgba(texture)
    row_size = texture.width * 4
    result = []
    for row in range(texture.height - 1, -1, -1):
        result.extend(source[row * row_size : (row + 1) * row_size])
    return result


def _pixel_hash(values: Iterable[float]) -> str:
    packed = bytes(
        max(0, min(255, round(float(component) * 255.0)))
        for component in values
    )
    return hashlib.sha256(packed).hexdigest()


def _image_native_pixels(image: bpy.types.Image) -> list[float]:
    """Return Blender image pixels in the native top-to-bottom row order."""

    source = list(image.pixels)
    row_size = image.size[0] * 4
    result = []
    for row in range(image.size[1] - 1, -1, -1):
        result.extend(source[row * row_size : (row + 1) * row_size])
    return result


def _make_texture_assets(
    entry_name: str,
    model: xobf.Model,
    source_text: str,
    entry_index: int,
) -> tuple[dict[int, bpy.types.Image], dict[int, bpy.types.Material]]:
    images: dict[int, bpy.types.Image] = {}
    materials: dict[int, bpy.types.Material] = {}
    for texture in model.textures():
        if not texture.supported:
            continue
        image = bpy.data.images.new(
            f"{entry_name} Texture {texture.index:03d}",
            width=texture.width,
            height=texture.height,
            alpha=True,
        )
        image.colorspace_settings.name = "Non-Color"
        display_pixels = _texture_pixels(texture)
        image.pixels.foreach_set(display_pixels)
        image.update()
        image.pack()
        image["v8_texture_index"] = texture.index
        image["v8_depth"] = texture.depth
        image["v8_compressed"] = texture.compressed
        image["v8_source_text"] = source_text
        image["v8_entry_index"] = entry_index
        image["v8_vehicle_collection"] = entry_name
        image["v8_original_pixel_hash"] = _pixel_hash(display_pixels)
        images[texture.index] = image

        material = bpy.data.materials.new(f"{entry_name} Texture {texture.index:03d}")
        material.use_nodes = True
        material.diffuse_color = (1.0, 1.0, 1.0, 1.0)
        material["v8_texture_index"] = texture.index
        nodes = material.node_tree.nodes
        links = material.node_tree.links
        principled = nodes.get("Principled BSDF")
        image_node = nodes.new("ShaderNodeTexImage")
        image_node.image = image
        image_node.interpolation = "Closest"
        if principled is not None:
            links.new(image_node.outputs["Color"], principled.inputs["Base Color"])
            links.new(image_node.outputs["Alpha"], principled.inputs["Alpha"])
        materials[texture.index] = material
    return images, materials


def _flat_material(
    entry_name: str,
    color: tuple[int, int, int],
    cache: dict[tuple[int, int, int], bpy.types.Material],
) -> bpy.types.Material:
    if color in cache:
        return cache[color]
    material = bpy.data.materials.new(
        f"{entry_name} RGB {color[0]:02X}{color[1]:02X}{color[2]:02X}"
    )
    rgba = tuple(component / 255.0 for component in color) + (1.0,)
    material.diffuse_color = rgba
    material["v8_flat_color"] = color
    cache[color] = material
    return material


def _parent_map(slots: tuple[xobf.Slot, ...]) -> dict[int, int]:
    parents: dict[int, int] = {}
    total = len(slots)
    for parent in slots:
        child = parent.first_child
        visited = set()
        while 0 <= child < total and child not in visited:
            visited.add(child)
            parents.setdefault(child, parent.index)
            child = slots[child].next_sibling
    return parents


def _retail_part_name(
    slot: xobf.Slot,
    dialect: str,
    parent_index: int | None = None,
    body_root: int = 0,
) -> tuple[str, str]:
    """Return a binary-established part name and its evidence level."""

    key = slot.render_key & 0xFFFF
    direct_body_child = parent_index == body_root
    wheel_names = (
        ("Front Left Wheel Joint", "Front Right Wheel Joint",
         "Rear Left Wheel Joint", "Rear Right Wheel Joint")
        if dialect == "V8"
        else (
            "Front Left Wheel Joint", "Front Right Wheel Joint",
            "Middle Left Wheel Joint", "Middle Right Wheel Joint",
            "Rear Left Wheel Joint", "Rear Right Wheel Joint",
        )
    )
    if direct_body_child and 0x8000 <= key < 0x8000 + len(wheel_names):
        return wheel_names[key - 0x8000], "HIGH"
    if direct_body_child and 0x8010 <= key <= 0x8016:
        return f"Weapon Mount {key - 0x8010 + 1}", "HIGH"
    if direct_body_child and key == 0x801F:
        return "Weapon Mount 8", "HIGH"
    if direct_body_child and key == 0x8040:
        return "Left Headlight Emitter", "HIGH"
    if direct_body_child and key == 0x8041:
        return "Right Headlight Emitter", "HIGH"
    if direct_body_child and key == 0x8042:
        return "Left Tail-Light Emitter", "HIGH"
    if direct_body_child and key == 0x8043:
        return "Right Tail-Light Emitter", "HIGH"
    if direct_body_child and key == 0x8101:
        return "Destruction Effect Anchor", "HIGH"
    if direct_body_child and key == 0x8100:
        return "Engine Attachment 0x8100 — Role Unresolved", "UNKNOWN"
    if (
        0x8000 <= key <= 0x8005
        or 0x8010 <= key <= 0x8016
        or key in {0x801F, 0x8040, 0x8041, 0x8042, 0x8043, 0x8100, 0x8101}
    ):
        return f"Nested Native Key 0x{key:04X}", "UNKNOWN"
    if slot.index == 0:
        return "Vehicle Root", "HIGH"
    if slot.render_group is not None and slot.obstacle_index >= 0:
        return f"Body Part {slot.index:03d} (Render + Collision)", "MED"
    if slot.render_group is not None:
        return f"Render Part {slot.index:03d}", "MED"
    if slot.obstacle_index >= 0:
        return f"Collision Part {slot.index:03d}", "MED"
    return f"Unknown Part {slot.index:03d}", "UNKNOWN"


def _import_slots(
    collection: bpy.types.Collection, model: xobf.Model
) -> dict[int, bpy.types.Object]:
    slots = model.slots()
    parents = _parent_map(slots)
    objects: dict[int, bpy.types.Object] = {}
    for slot in slots:
        part_name, confidence = _retail_part_name(
            slot, model.dialect, parents.get(slot.index)
        )
        obj = bpy.data.objects.new(part_name, None)
        collection.objects.link(obj)
        obj.empty_display_type = "ARROWS"
        obj.empty_display_size = 0.2
        obj.rotation_mode = "XYZ"
        obj.location = scene_codec.native_position_to_blender((slot.x, slot.y, slot.z))
        obj.rotation_euler = scene_codec.native_rotation_to_blender(
            (slot.rot_y, slot.rot_x, slot.rot_z)
        )
        obj["v8_role"] = "slot"
        obj["v8_slot_index"] = slot.index
        obj["v8_render_key"] = slot.render_key
        obj["v8_obstacle_index"] = slot.obstacle_index
        obj["v8_flags"] = slot.flags
        obj["v8_next_sibling"] = slot.next_sibling
        obj["v8_first_child"] = slot.first_child
        obj["v8_part_name"] = part_name
        obj["v8_part_name_confidence"] = confidence
        objects[slot.index] = obj
    for child_index, parent_index in parents.items():
        objects[child_index].parent = objects[parent_index]
    return objects


def _texture_for_packet(
    group: xobf.RenderGroup, packet: xobf.PolygonPacket, model: xobf.Model
) -> int | None:
    if packet.texture_slot is None:
        return None
    texture_index = group.texture_base + packet.texture_slot
    return texture_index if 0 <= texture_index < model.texture_count else None


def _import_group(
    collection: bpy.types.Collection,
    entry_name: str,
    model: xobf.Model,
    group: xobf.RenderGroup,
    slot_objects: dict[int, bpy.types.Object],
    texture_images: dict[int, bpy.types.Image],
    texture_materials: dict[int, bpy.types.Material],
    flat_materials: dict[tuple[int, int, int], bpy.types.Material],
) -> bpy.types.Object:
    packet_indices = [
        index
        for index, packet in enumerate(group.packets)
        if packet.kind not in xobf.NON_VERTEX_PACKET_KINDS
        and len(packet.vertex_indices) == 3
    ]
    faces = [group.packets[index].vertex_indices for index in packet_indices]
    vertices = [
        scene_codec.native_vertex_to_blender(vertex, group.scale_shift)
        for vertex in group.vertices
    ]
    referencing_slots = [
        slot_objects[slot.index]
        for slot in model.slots()
        if slot.render_group == group.index
    ]
    part_label = (
        str(referencing_slots[0].get("v8_part_name", referencing_slots[0].name))
        if referencing_slots
        else f"Unassigned Render Group {group.index:03d}"
    )
    mesh = bpy.data.meshes.new(
        f"{entry_name} — {part_label} — Group {group.index:03d}"
    )
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(mesh.name, mesh)
    collection.objects.link(obj)
    obj["v8_role"] = "render_group"
    obj["v8_group_index"] = group.index
    obj["v8_scale_shift"] = group.scale_shift
    obj["v8_original_vertex_count"] = group.vertex_count
    obj["v8_original_face_count"] = len(faces)
    obj["v8_packet_indices"] = json.dumps(packet_indices)
    obj["v8_part_name"] = part_label
    obj["v8_part_name_confidence"] = (
        str(referencing_slots[0].get("v8_part_name_confidence", "UNKNOWN"))
        if referencing_slots
        else "UNKNOWN"
    )

    color_channel_names = tuple(f"V8 Color {channel}" for channel in ("R", "G", "B"))
    for name in color_channel_names:
        mesh.attributes.new(name=name, type="INT", domain="FACE")
    mesh.uv_layers.new(name="V8 UV")
    material_lookup: dict[str, int] = {}

    for face_index, packet_index in enumerate(packet_indices):
        packet = group.packets[packet_index]
        for name, value in zip(color_channel_names, packet.color):
            mesh.attributes[name].data[face_index].value = value
        texture_index = _texture_for_packet(group, packet, model)
        material = (
            texture_materials.get(texture_index)
            if texture_index is not None
            else None
        )
        if material is None:
            material = _flat_material(entry_name, packet.color, flat_materials)
        if material.name not in material_lookup:
            material_lookup[material.name] = len(mesh.materials)
            mesh.materials.append(material)
        mesh.polygons[face_index].material_index = material_lookup[material.name]

        if (
            texture_index is not None
            and texture_index in texture_images
            and len(packet.uv) == 3
        ):
            texture = model.texture(texture_index)
            denominator_u = max(1, texture.width - 1)
            denominator_v = max(1, texture.height - 1)
            polygon = mesh.polygons[face_index]
            for loop_index, uv in zip(polygon.loop_indices, packet.uv):
                mesh.uv_layers["V8 UV"].data[loop_index].uv = (
                    uv[0] / denominator_u,
                    1.0 - uv[1] / denominator_v,
                )

    if referencing_slots:
        obj.parent = referencing_slots[0]
    return obj


def _insert_transform_keys(
    obj: bpy.types.Object,
    bank: xobf.AnimationBank,
    slot_index: int,
) -> None:
    frames = bank.frames(slot_index)
    if not frames:
        return
    action = bpy.data.actions.new(f"{obj.name} Native Animation")
    obj.animation_data_create()
    obj.animation_data.action = action
    timeline = 0
    location = tuple(obj.location)
    rotation = tuple(obj.rotation_euler)
    scale = tuple(obj.scale)
    mapping = []
    for frame_index, frame in enumerate(frames):
        timeline += max(0, frame.frame_delta)
        mapping.append(
            {
                "native_index": frame_index,
                "frame": timeline,
                "flags": frame.flags,
            }
        )
        if frame.flags < 0:
            continue
        if frame.translation_absolute is not None:
            location = scene_codec.native_position_to_blender(
                frame.translation_absolute
            )
            obj.location = location
            obj.keyframe_insert("location", frame=timeline, group="Native Transform")
        elif frame.translation_delta is not None:
            delta = scene_codec.native_position_to_blender(frame.translation_delta)
            location = tuple(location[axis] + delta[axis] for axis in range(3))
            obj.location = location
            obj.keyframe_insert("location", frame=timeline, group="Native Transform")
        if frame.rotation_yxz is not None:
            rotation = scene_codec.native_rotation_to_blender(frame.rotation_yxz)
            obj.rotation_euler = rotation
            obj.keyframe_insert(
                "rotation_euler", frame=timeline, group="Native Transform"
            )
        if frame.scale is not None:
            scale = tuple(component / 4096.0 for component in frame.scale[:3])
            obj.scale = scale
            obj.keyframe_insert("scale", frame=timeline, group="Native Transform")
    obj["v8_animation_frames"] = json.dumps(mapping)


def _import_entry(
    root: bpy.types.Collection,
    source_text: str,
    source_path: Path,
    vehicles: archive.VehicleArchive,
    entry_index: int,
    import_animation: bool,
) -> bpy.types.Collection:
    entry = vehicles.entry(entry_index)
    model = entry.model()
    name = f"{source_path.stem} Vehicle {entry_index:02d}"
    collection = bpy.data.collections.new(
        _unique_name(name, bpy.data.collections.keys())
    )
    _link_collection(root, collection)
    collection["v8_role"] = "vehicle_entry"
    collection["v8_source_text"] = source_text
    collection["v8_source_path"] = str(source_path)
    collection["v8_entry_index"] = entry_index
    collection["v8_dialect"] = model.dialect
    collection["v8_archive_entry_count"] = len(tuple(vehicles.entries()))

    slot_objects = _import_slots(collection, model)
    images, texture_materials = _make_texture_assets(
        collection.name, model, source_text, entry_index
    )
    flat_materials: dict[tuple[int, int, int], bpy.types.Material] = {}
    for group in model.groups():
        _import_group(
            collection,
            collection.name,
            model,
            group,
            slot_objects,
            images,
            texture_materials,
            flat_materials,
        )
    if import_animation:
        bank = entry.animation(model.slot_count)
        if bank is not None:
            for slot_index, obj in slot_objects.items():
                _insert_transform_keys(obj, bank, slot_index)
    return collection


def import_archive_into_scene(
    context,
    filepath: str,
    entry_index: int,
    import_all: bool,
    import_animation: bool,
) -> tuple[bpy.types.Collection, int]:
    path = Path(filepath)
    source = path.read_bytes()
    vehicles = archive.VehicleArchive(source)
    entries = tuple(vehicles.entries())
    if not import_all and not 0 <= entry_index < len(entries):
        raise IndexError(
            f"archive has {len(entries)} vehicle entries; {entry_index} is invalid"
        )
    source_text = _store_source(path.name, source)
    root = bpy.data.collections.new(
        _unique_name(f"{path.stem} Native Vehicles", bpy.data.collections.keys())
    )
    context.scene.collection.children.link(root)
    indices = range(len(entries)) if import_all else (entry_index,)
    imported = []
    for index in indices:
        imported.append(
            _import_entry(
                root,
                source_text,
                path,
                vehicles,
                index,
                import_animation,
            )
        )
    context.scene.v8_vehicle_settings.vehicle_collection = imported[-1].name
    return root, len(imported)


def _objects_by_role(
    collection: bpy.types.Collection, role: str
) -> list[bpy.types.Object]:
    return [obj for obj in collection.all_objects if obj.get("v8_role") == role]


def _material_texture_index(
    mesh: bpy.types.Mesh, polygon: bpy.types.MeshPolygon
) -> int | None:
    if polygon.material_index >= len(mesh.materials):
        return None
    material = mesh.materials[polygon.material_index]
    if material is None or "v8_texture_index" not in material:
        return None
    return int(material["v8_texture_index"])


def _polygon_color(
    mesh: bpy.types.Mesh, polygon: bpy.types.MeshPolygon
) -> tuple[int, int, int]:
    channels = tuple(
        mesh.attributes.get(f"V8 Color {channel}") for channel in ("R", "G", "B")
    )
    if all(channel is not None for channel in channels):
        return tuple(
            max(0, min(255, int(channel.data[polygon.index].value)))
            for channel in channels
        )
    attribute = mesh.color_attributes.get("V8 Face Color")
    if attribute is not None and polygon.index < len(attribute.data):
        color = attribute.data[polygon.index].color
        return tuple(max(0, min(255, round(color[index] * 255.0))) for index in range(3))
    if polygon.material_index < len(mesh.materials):
        material = mesh.materials[polygon.material_index]
        if material is not None:
            return tuple(
                max(0, min(255, round(material.diffuse_color[index] * 255.0)))
                for index in range(3)
            )
    return 127, 127, 127


def _polygon_uvs(
    mesh: bpy.types.Mesh,
    polygon: bpy.types.MeshPolygon,
    texture: xobf.TextureSlot | None,
) -> tuple[tuple[int, int], ...] | None:
    layer = mesh.uv_layers.active
    if layer is None or texture is None or not texture.supported:
        return None
    denominator_u = max(1, texture.width - 1)
    denominator_v = max(1, texture.height - 1)
    return tuple(
        (
            max(0, min(255, round(layer.data[loop].uv[0] * denominator_u))),
            max(
                0,
                min(255, round((1.0 - layer.data[loop].uv[1]) * denominator_v)),
            ),
        )
        for loop in polygon.loop_indices
    )


def _export_group(
    model: xobf.Model, obj: bpy.types.Object
) -> None:
    if obj.type != "MESH":
        return
    group_index = int(obj["v8_group_index"])
    source_group = model.group(group_index)
    scale_shift = int(obj.get("v8_scale_shift", source_group.scale_shift))
    mesh = obj.data
    vertices = [
        scene_codec.blender_vertex_to_native(vertex.co, scale_shift)
        for vertex in mesh.vertices
    ]
    source_packet_indices = json.loads(obj.get("v8_packet_indices", "[]"))
    source_faces = [
        source_group.packets[index].vertex_indices for index in source_packet_indices
    ]
    current_faces = [tuple(polygon.vertices) for polygon in mesh.polygons]
    topology_matches = (
        len(vertices) == source_group.vertex_count
        and current_faces == source_faces
        and len(source_packet_indices) == len(mesh.polygons)
    )

    if topology_matches:
        model.patch_group_vertices(group_index, vertices)
        colors = []
        textures = []
        face_uvs = []
        for polygon, packet_index in zip(mesh.polygons, source_packet_indices):
            colors.append(_polygon_color(mesh, polygon))
            absolute_texture = _material_texture_index(mesh, polygon)
            raw_texture = (
                None
                if absolute_texture is None
                else absolute_texture - source_group.texture_base
            )
            textures.append(raw_texture)
            packet = source_group.packets[packet_index]
            texture = (
                model.texture(absolute_texture)
                if absolute_texture is not None
                and 0 <= absolute_texture < model.texture_count
                else None
            )
            face_uvs.append(
                _polygon_uvs(mesh, polygon, texture)
                if packet.texture_slot is not None
                else None
            )
        try:
            model.patch_group_packets(
                group_index,
                source_packet_indices,
                colors=colors,
                texture_slots=textures,
                uvs=face_uvs,
            )
            return
        except ValueError:
            # Assigning a texture to a formerly untextured packet requires the
            # same native kind-5 rebuild used for changed topology.
            pass

    mesh.calc_loop_triangles()
    faces = []
    colors = []
    texture_slots = []
    uvs = []
    for triangle in mesh.loop_triangles:
        polygon = mesh.polygons[triangle.polygon_index]
        faces.append(tuple(triangle.vertices))
        colors.append(_polygon_color(mesh, polygon))
        absolute_texture = _material_texture_index(mesh, polygon)
        texture_slots.append(
            None
            if absolute_texture is None
            else absolute_texture - source_group.texture_base
        )
        texture = (
            model.texture(absolute_texture)
            if absolute_texture is not None
            and 0 <= absolute_texture < model.texture_count
            else None
        )
        polygon_uv = _polygon_uvs(mesh, polygon, texture)
        if polygon_uv is None:
            uvs.append(((0, 0), (0, 0), (0, 0)))
        else:
            loop_to_uv = {
                loop: uv for loop, uv in zip(polygon.loop_indices, polygon_uv)
            }
            uvs.append(tuple(loop_to_uv[loop] for loop in triangle.loops))
    model.replace_group_geometry(
        group_index,
        vertices,
        faces,
        colors=colors,
        texture_slots=texture_slots,
        uvs=uvs,
        scale_shift=scale_shift,
    )


def _curve_value(
    obj: bpy.types.Object,
    data_path: str,
    component: int,
    frame: float,
    fallback: float,
) -> float:
    action = obj.animation_data.action if obj.animation_data else None
    if action is None:
        return fallback
    curve = next(
        (
            item
            for item in action.fcurves
            if item.data_path == data_path and item.array_index == component
        ),
        None,
    )
    return fallback if curve is None else curve.evaluate(frame)


def _sample_vector(
    obj: bpy.types.Object,
    data_path: str,
    frame: float,
    fallback,
) -> tuple[float, float, float]:
    return tuple(
        _curve_value(obj, data_path, component, frame, fallback[component])
        for component in range(3)
    )


def _export_animation(
    entry: archive.VehicleEntry,
    collection: bpy.types.Collection,
    model: xobf.Model,
) -> None:
    bank = entry.animation(model.slot_count)
    if bank is None:
        return
    for obj in _objects_by_role(collection, "slot"):
        mapping_text = obj.get("v8_animation_frames")
        if not mapping_text or not obj.animation_data or obj.animation_data.action is None:
            continue
        slot_index = int(obj["v8_slot_index"])
        mapping = json.loads(mapping_text)
        previous_location = tuple(obj.location)
        native_frames = bank.frames(slot_index)
        for item in mapping:
            frame_index = int(item["native_index"])
            if frame_index >= len(native_frames):
                continue
            native_frame = native_frames[frame_index]
            if native_frame.flags < 0:
                continue
            timeline = float(item["frame"])
            location = _sample_vector(
                obj, "location", timeline, tuple(obj.location)
            )
            rotation = _sample_vector(
                obj, "rotation_euler", timeline, tuple(obj.rotation_euler)
            )
            scale = _sample_vector(obj, "scale", timeline, tuple(obj.scale))
            kwargs = {}
            if native_frame.translation_absolute is not None:
                kwargs["translation_absolute"] = (
                    scene_codec.blender_position_to_native(location)
                )
            if native_frame.translation_delta is not None:
                delta = tuple(
                    location[axis] - previous_location[axis] for axis in range(3)
                )
                kwargs["translation_delta"] = (
                    scene_codec.blender_position_to_native(delta)
                )
            if native_frame.rotation_yxz is not None:
                kwargs["rotation_yxz"] = scene_codec.blender_rotation_to_native(
                    rotation
                )
            if native_frame.scale is not None:
                kwargs["scale"] = tuple(
                    max(-32768, min(32767, round(value * 4096.0)))
                    for value in scale
                ) + (native_frame.scale[3],)
            bank.patch_frame(slot_index, frame_index, **kwargs)
            previous_location = location
    entry.commit_animation(bank)


def _export_textures(
    model: xobf.Model, collection: bpy.types.Collection
) -> None:
    for image in bpy.data.images:
        if image.get("v8_vehicle_collection") != collection.name:
            continue
        if "v8_texture_index" not in image:
            continue
        display_pixels = list(image.pixels)
        if _pixel_hash(display_pixels) == image.get("v8_original_pixel_hash"):
            continue
        texture_index = int(image["v8_texture_index"])
        texture = model.texture(texture_index)
        if tuple(image.size) != (texture.width, texture.height):
            raise ValueError(
                f"texture {texture_index} must remain "
                f"{texture.width}x{texture.height}"
            )
        native_pixels = _image_native_pixels(image)
        palette, indices = scene_codec.rgba_to_native_palette(
            native_pixels, len(texture.palette)
        )
        model.replace_texture(texture_index, palette, indices)


def _apply_collection_to_archive(
    vehicles: archive.VehicleArchive,
    collection: bpy.types.Collection,
) -> None:
    entry_index = int(collection["v8_entry_index"])
    entry = vehicles.entry(entry_index)
    model = entry.model(collection.get("v8_dialect"))
    _export_textures(model, collection)

    native_slots = model.slots()
    for obj in _objects_by_role(collection, "slot"):
        index = int(obj["v8_slot_index"])
        model.patch_slot(
            index,
            render_key=int(obj.get("v8_render_key", native_slots[index].render_key)),
            obstacle_index=int(
                obj.get("v8_obstacle_index", native_slots[index].obstacle_index)
            ),
            position=scene_codec.blender_position_to_native(obj.location),
            rotation_yxz=scene_codec.blender_rotation_to_native(obj.rotation_euler),
            flags=int(obj.get("v8_flags", native_slots[index].flags)),
            next_sibling=int(
                obj.get("v8_next_sibling", native_slots[index].next_sibling)
            ),
            first_child=int(obj.get("v8_first_child", native_slots[index].first_child)),
        )
    for obj in sorted(
        _objects_by_role(collection, "render_group"),
        key=lambda item: int(item["v8_group_index"]),
    ):
        _export_group(model, obj)
    entry.commit_model(model)
    _export_animation(entry, collection, model)


def export_collection_to_archive(
    collection: bpy.types.Collection, filepath: str
) -> None:
    source = _load_source(collection["v8_source_text"])
    vehicles = archive.VehicleArchive(source)
    source_text = collection["v8_source_text"]
    related = sorted(
        (
            candidate
            for candidate in bpy.data.collections
            if candidate.get("v8_role") == "vehicle_entry"
            and candidate.get("v8_source_text") == source_text
        ),
        key=lambda candidate: int(candidate["v8_entry_index"]),
    )
    for candidate in related:
        _apply_collection_to_archive(vehicles, candidate)
    Path(filepath).write_bytes(vehicles.data)


class V8NumericField(PropertyGroup):
    key: StringProperty()
    label: StringProperty()
    description: StringProperty()
    section: StringProperty()
    record: IntProperty(default=-1)
    row: IntProperty(default=-1)
    column: IntProperty(default=-1)
    advanced: BoolProperty(default=False)
    value: IntProperty(min=-2147483648, max=2147483647)


class V8VehicleSettings(PropertyGroup):
    vehicle_collection: StringProperty(name="Vehicle collection")
    stats_source_text: StringProperty()
    stats_source_path: StringProperty()
    stats_game: StringProperty()
    stat_record_index: IntProperty(name="Vehicle", min=0, max=255)
    show_advanced: BoolProperty(name="Show advanced fields", default=False)
    new_game: EnumProperty(
        name="Game",
        items=(
            ("V8_2", "V8: 2nd Offense", "Lead engine with transformations and powerups"),
            ("V8", "Vigilante 8", "Original-game compatible vehicle"),
        ),
        default="V8_2",
    )
    fields: CollectionProperty(type=V8NumericField)


def _add_numeric_field(
    settings: V8VehicleSettings,
    *,
    key: str,
    label: str,
    description: str,
    section: str,
    value: int,
    record: int = -1,
    row: int = -1,
    column: int = -1,
    advanced: bool = False,
) -> None:
    item = settings.fields.add()
    item.key = key
    item.label = label
    item.description = description
    item.section = section
    item.value = value
    item.record = record
    item.row = row
    item.column = column
    item.advanced = advanced


def load_stats_into_scene(context, filepath: str) -> stats.StatsFile:
    path = Path(filepath)
    parsed = stats.StatsFile(path.read_bytes())
    settings = context.scene.v8_vehicle_settings
    settings.fields.clear()
    settings.stats_source_text = _store_source(path.name, parsed.data)
    settings.stats_source_path = str(path)
    settings.stats_game = parsed.profile.game
    settings.stat_record_index = 0
    for record in parsed.records():
        for field in parsed.profile.fields:
            _add_numeric_field(
                settings,
                key=field.name,
                label=field.label,
                description=field.description,
                section="stats",
                value=record.get(field.name),
                record=record.index,
                advanced=field.advanced,
            )
    if parsed.profile.game == "V8_2":
        for setting in stats.V82_POWERUP_SETTINGS:
            _add_numeric_field(
                settings,
                key=setting.name,
                label=setting.label,
                description=setting.description,
                section="powerup",
                value=parsed.powerup_values()[setting.name],
            )
        for mode, values in enumerate(parsed.transform_modes()):
            for wheel, value in enumerate(values):
                _add_numeric_field(
                    settings,
                    key=f"transform_{mode}_{wheel}",
                    label=WHEEL_NAMES[wheel],
                    description=(
                        f"Native object kind for {TRANSFORM_MODE_NAMES[mode]} "
                        f"{WHEEL_NAMES[wheel]}."
                    ),
                    section="transform",
                    value=value,
                    row=mode,
                    column=wheel,
                )
    return parsed


def export_stats_from_scene(context, filepath: str) -> None:
    settings = context.scene.v8_vehicle_settings
    if not settings.stats_source_text:
        raise ValueError("load a V8 or V8:2 executable first")
    parsed = stats.StatsFile(
        _load_source(settings.stats_source_text), settings.stats_game
    )
    for item in settings.fields:
        if item.section == "stats":
            parsed.record(item.record).set(item.key, item.value)
        elif item.section == "powerup":
            parsed.set_powerup(item.key, item.value)
        elif item.section == "transform":
            parsed.set_transform_wheel_kind(item.row, item.column, item.value)
    Path(filepath).write_bytes(parsed.data)


class V8_OT_import_vehicle_archive(Operator, ImportHelper):
    bl_idname = "v8.import_vehicle_archive"
    bl_label = "Import V8 Native Vehicle Archive"
    bl_description = "Import V8/V8:2 EXP vehicle geometry, hierarchy, textures, and ANM"
    bl_options = {"REGISTER", "UNDO"}

    filename_ext = ".EXP"
    filter_glob: StringProperty(default="*.EXP;*.exp", options={"HIDDEN"})
    entry_index: IntProperty(name="Vehicle entry", default=0, min=0)
    import_all: BoolProperty(name="Import all entries", default=False)
    import_animation: BoolProperty(name="Import ANM actions", default=True)

    def execute(self, context):
        try:
            _root, count = import_archive_into_scene(
                context,
                self.filepath,
                self.entry_index,
                self.import_all,
                self.import_animation,
            )
        except Exception as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Imported {count} native vehicle entr{'y' if count == 1 else 'ies'}")
        return {"FINISHED"}


class V8_OT_export_vehicle_archive(Operator, ExportHelper):
    bl_idname = "v8.export_vehicle_archive"
    bl_label = "Export V8 Native Vehicle Archive"
    bl_description = "Patch the embedded retail source and write a native EXP archive"

    filename_ext = ".EXP"
    filter_glob: StringProperty(default="*.EXP;*.exp", options={"HIDDEN"})

    def invoke(self, context, event):
        collection = _active_vehicle_collection(context)
        if collection is not None:
            source = Path(collection.get("v8_source_path", "VEHICLES.EXP"))
            self.filepath = str(source.with_name(f"{source.stem}_edited{source.suffix}"))
        return ExportHelper.invoke(self, context, event)

    def execute(self, context):
        collection = _active_vehicle_collection(context)
        if collection is None:
            self.report({"ERROR"}, "Select an imported native vehicle collection")
            return {"CANCELLED"}
        try:
            export_collection_to_archive(collection, self.filepath)
        except Exception as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Wrote native archive {self.filepath}")
        return {"FINISHED"}


class V8_OT_load_stats(Operator, ImportHelper):
    bl_idname = "v8.load_vehicle_stats"
    bl_label = "Load V8 Executable"
    bl_description = "Load the native vehicle stat table from a V8/V8:2 PS-X EXE"

    filename_ext = ""
    filter_glob: StringProperty(default="SLUS*;SCUS*;*.EXE;*.exe;*.*", options={"HIDDEN"})

    def execute(self, context):
        try:
            parsed = load_stats_into_scene(context, self.filepath)
        except Exception as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report(
            {"INFO"},
            f"Loaded {parsed.profile.record_count} {parsed.profile.game} vehicle records",
        )
        return {"FINISHED"}


class V8_OT_export_stats(Operator, ExportHelper):
    bl_idname = "v8.export_vehicle_stats"
    bl_label = "Export Patched Executable"
    bl_description = "Write edited stats, powerups, and transformations to a PS-X EXE"

    filename_ext = ""
    filter_glob: StringProperty(default="SLUS*;SCUS*;*.EXE;*.exe;*.*", options={"HIDDEN"})

    def invoke(self, context, event):
        settings = context.scene.v8_vehicle_settings
        if settings.stats_source_path:
            source = Path(settings.stats_source_path)
            self.filepath = str(source.with_name(f"{source.name}_edited"))
        return ExportHelper.invoke(self, context, event)

    def execute(self, context):
        try:
            export_stats_from_scene(context, self.filepath)
        except Exception as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Wrote patched executable {self.filepath}")
        return {"FINISHED"}


class V8_OT_new_authored_vehicle(Operator):
    bl_idname = "v8.new_authored_vehicle"
    bl_label = "New Original Vehicle"
    bl_description = "Create a complete source-free native vehicle project"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        try:
            vehicle = authored_scene.new_project(
                context.scene.v8_vehicle_settings.new_game
            )
            collection = authored_scene.project_to_scene(context, vehicle)
        except Exception as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report(
            {"INFO"},
            f"Created independent {vehicle.game} vehicle {collection.name}",
        )
        return {"FINISHED"}


class V8_OT_import_authored_project(Operator, ImportHelper):
    bl_idname = "v8.import_authored_project"
    bl_label = "Import Vehicle Project"
    bl_description = "Import a strict decoded vehicle JSON project"
    bl_options = {"REGISTER", "UNDO"}

    filename_ext = ".json"
    filter_glob: StringProperty(default="*.json", options={"HIDDEN"})

    def execute(self, context):
        try:
            value = json.loads(Path(self.filepath).read_text(encoding="utf-8"))
            vehicle = project.VehicleProject.from_dict(value)
            authored_scene.project_to_scene(context, vehicle)
        except Exception as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Imported {vehicle.game} project {vehicle.stable_id}")
        return {"FINISHED"}


class V8_OT_export_authored_project(Operator, ExportHelper):
    bl_idname = "v8.export_authored_project"
    bl_label = "Export Vehicle Project"
    bl_description = "Export all named decoded fields without source or passthrough data"

    filename_ext = ".json"
    filter_glob: StringProperty(default="*.json", options={"HIDDEN"})

    def invoke(self, context, event):
        collection = _active_vehicle_collection(context)
        if collection is not None and collection.get("v8_role") == authored_scene.ROLE_VEHICLE:
            self.filepath = str(collection.get("v8_stable_id", "vehicle")) + ".json"
        return ExportHelper.invoke(self, context, event)

    def execute(self, context):
        collection = _active_vehicle_collection(context)
        if collection is None or collection.get("v8_role") != authored_scene.ROLE_VEHICLE:
            self.report({"ERROR"}, "Select an authored vehicle collection")
            return {"CANCELLED"}
        try:
            vehicle = authored_scene.scene_to_project(collection)
            Path(self.filepath).write_text(
                json.dumps(project.to_dict(vehicle), indent=2) + "\n",
                encoding="utf-8",
            )
        except Exception as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Wrote strict project {self.filepath}")
        return {"FINISHED"}


class V8_OT_compile_authored_package(Operator, ExportHelper):
    bl_idname = "v8.compile_authored_package"
    bl_label = "Compile Native Vehicle Package"
    bl_description = "Build CUSTOM.EXP and append-only VEHICLES.V8R from authored entries"

    filename_ext = ".V8R"
    filter_glob: StringProperty(default="*.V8R;*.v8r", options={"HIDDEN"})
    export_all: BoolProperty(
        name="All authored vehicles for this game",
        default=True,
    )

    def invoke(self, context, event):
        self.filepath = "VEHICLES.V8R"
        return ExportHelper.invoke(self, context, event)

    def execute(self, context):
        active = _active_vehicle_collection(context)
        if active is None or active.get("v8_role") != authored_scene.ROLE_VEHICLE:
            self.report({"ERROR"}, "Select an authored vehicle collection")
            return {"CANCELLED"}
        try:
            if self.export_all:
                collections = sorted(
                    (
                        collection
                        for collection in bpy.data.collections
                        if collection.get("v8_role") == authored_scene.ROLE_VEHICLE
                        and collection.get("v8_game") == active.get("v8_game")
                    ),
                    key=lambda collection: str(collection.get("v8_stable_id", "")),
                )
            else:
                collections = [active]
            vehicles = tuple(
                authored_scene.scene_to_project(collection)
                for collection in collections
            )
            package = registry.compile_package(vehicles)
            registry_path = Path(self.filepath)
            registry_path.write_bytes(package.registry)
            (registry_path.parent / "CUSTOM.EXP").write_bytes(package.archive)
        except Exception as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report(
            {"INFO"},
            f"Compiled {len(vehicles)} independent {package.game} vehicle(s)",
        )
        return {"FINISHED"}


class V8_PT_vehicle_tools(Panel):
    bl_label = "Vigilante 8 Native Vehicles"
    bl_idname = "V8_PT_vehicle_tools"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"

    def draw(self, context):
        layout = self.layout
        settings = context.scene.v8_vehicle_settings
        layout.label(text="Independent native authoring", icon="OUTLINER_COLLECTION")
        row = layout.row(align=True)
        row.prop(settings, "new_game", text="")
        row.operator(V8_OT_new_authored_vehicle.bl_idname, text="New", icon="ADD")
        row = layout.row(align=True)
        row.operator(V8_OT_import_authored_project.bl_idname, icon="IMPORT")
        row.operator(V8_OT_export_authored_project.bl_idname, icon="EXPORT")
        layout.operator(
            V8_OT_compile_authored_package.bl_idname,
            icon="PACKAGE",
        )
        collection = _active_vehicle_collection(context)
        if collection is None:
            layout.label(text="No authored vehicle selected", icon="INFO")
            return
        layout.prop_search(
            settings,
            "vehicle_collection",
            bpy.data,
            "collections",
            text="Active",
        )
        box = layout.box()
        box.label(text=collection.name)
        if collection.get("v8_role") == authored_scene.ROLE_VEHICLE:
            box.prop(collection, '["v8_stable_id"]', text="Stable ID")
            box.prop(collection, '["v8_display_name"]', text="Display name")
            box.prop(collection, '["v8_body_kind"]', text="Body object")
            box.label(
                text=f"{collection.get('v8_game')} schema {collection.get('v8_schema_version')}"
            )
            box.label(text="No retail source, donor entry, or opaque payload.")
        else:
            box.label(text="Legacy research collection (not package-exportable)")


class V8_PT_stats(Panel):
    bl_label = "Vigilante 8 Vehicle Data"
    bl_idname = "V8_PT_stats"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"
    bl_parent_id = V8_PT_vehicle_tools.bl_idname

    def draw(self, context):
        layout = self.layout
        settings = context.scene.v8_vehicle_settings
        collection = _active_vehicle_collection(context)
        if collection is None or collection.get("v8_role") != authored_scene.ROLE_VEHICLE:
            layout.label(text="Select an authored vehicle", icon="INFO")
            return
        profile = stats.PROFILES[str(collection["v8_game"])]
        layout.prop(settings, "show_advanced")
        box = layout.box()
        for field in profile.authoring_fields:
            if field.name == "vehicle_type" or (
                field.advanced and not settings.show_advanced
            ):
                continue
            box.prop(collection, f'["v8_stat_{field.name}"]', text=field.label)


class V8_PT_powerups(Panel):
    bl_label = "V8:2 Powerups"
    bl_idname = "V8_PT_powerups"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"
    bl_parent_id = V8_PT_stats.bl_idname
    bl_options = {"DEFAULT_CLOSED"}

    @classmethod
    def poll(cls, context):
        collection = _active_vehicle_collection(context)
        return (
            collection is not None
            and collection.get("v8_role") == authored_scene.ROLE_VEHICLE
            and collection.get("v8_game") == "V8_2"
        )

    def draw(self, context):
        layout = self.layout
        collection = _active_vehicle_collection(context)
        labels = {setting.name: setting.label for setting in stats.V82_POWERUP_SETTINGS}
        for name in project.V82_POWERUP_FIELDS:
            layout.prop(
                collection,
                f'["v8_powerup_{name}"]',
                text=labels[name],
            )


class V8_PT_transformations(Panel):
    bl_label = "V8:2 Transformation Wheel Objects"
    bl_idname = "V8_PT_transformations"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"
    bl_parent_id = V8_PT_stats.bl_idname
    bl_options = {"DEFAULT_CLOSED"}

    @classmethod
    def poll(cls, context):
        return V8_PT_powerups.poll(context)

    def draw(self, context):
        layout = self.layout
        collection = _active_vehicle_collection(context)
        for mode, mode_name in enumerate(TRANSFORM_MODE_NAMES):
            box = layout.box()
            box.label(text=mode_name)
            for wheel in range(len(WHEEL_NAMES)):
                box.prop(
                    collection,
                    f'["v8_transform_{mode}_{wheel}"]',
                    text=WHEEL_NAMES[wheel],
                )


def _menu_import(self, _context):
    self.layout.operator(
        V8_OT_import_authored_project.bl_idname,
        text="Vigilante 8 Vehicle Project (.json)",
    )


def _menu_export(self, _context):
    self.layout.operator(
        V8_OT_export_authored_project.bl_idname,
        text="Vigilante 8 Vehicle Project (.json)",
    )


CLASSES = (
    V8NumericField,
    V8VehicleSettings,
    V8_OT_new_authored_vehicle,
    V8_OT_import_authored_project,
    V8_OT_export_authored_project,
    V8_OT_compile_authored_package,
    V8_PT_vehicle_tools,
    V8_PT_stats,
    V8_PT_powerups,
    V8_PT_transformations,
)


def register() -> None:
    for cls in CLASSES:
        bpy.utils.register_class(cls)
    bpy.types.Scene.v8_vehicle_settings = bpy.props.PointerProperty(
        type=V8VehicleSettings
    )
    bpy.types.TOPBAR_MT_file_import.append(_menu_import)
    bpy.types.TOPBAR_MT_file_export.append(_menu_export)


def unregister() -> None:
    bpy.types.TOPBAR_MT_file_export.remove(_menu_export)
    bpy.types.TOPBAR_MT_file_import.remove(_menu_import)
    del bpy.types.Scene.v8_vehicle_settings
    for cls in reversed(CLASSES):
        bpy.utils.unregister_class(cls)
