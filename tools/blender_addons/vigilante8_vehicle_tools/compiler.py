"""From-scratch native compiler for strict vehicle projects."""

from __future__ import annotations

import struct
from typing import Iterable, Sequence

from . import iff, project, xobf


def _align(buffer: bytearray, alignment: int = 4) -> None:
    while len(buffer) % alignment:
        buffer.append(0)


def _i16(value: int, context: str) -> int:
    if value < -32768 or value > 32767:
        raise ValueError(f"{context} exceeds signed 16-bit")
    return value


def _u16(value: int, context: str) -> int:
    if value < 0 or value > 0xFFFF:
        raise ValueError(f"{context} exceeds unsigned 16-bit")
    return value


def _native_slot_key(slot: project.Slot) -> int:
    """Return the signed representation stored in the native i16 key field."""

    if slot.key is not None:
        value = _u16(slot.key, f"slot {slot.name!r} attachment key")
        return value if value <= 0x7FFF else value - 0x10000
    value = (
        -1
        if slot.render_group is None
        else slot.render_group | slot.render_flags
    )
    return value if value <= 0x7FFF else value - 0x10000


def _face_normal(
    vertices: Sequence[tuple[int, int, int]], face: project.Face
) -> tuple[int, int, int, int]:
    a, b, c = (vertices[index] for index in face.vertices)
    ab = tuple(b[axis] - a[axis] for axis in range(3))
    ac = tuple(c[axis] - a[axis] for axis in range(3))
    normal = (
        ab[1] * ac[2] - ab[2] * ac[1],
        ab[2] * ac[0] - ab[0] * ac[2],
        ab[0] * ac[1] - ab[1] * ac[0],
    )
    magnitude = sum(component * component for component in normal) ** 0.5
    if magnitude == 0:
        return 0, -4096, 0, 0
    return tuple(
        max(-32768, min(32767, round(component * 4096.0 / magnitude)))
        for component in normal
    ) + (0,)


def _packet_type(kind: int, flags: int) -> int:
    if kind < 0 or kind > 15 or kind == 14:
        raise ValueError(f"unsupported native packet kind {kind}")
    if flags < 0 or flags & ~0xF0:
        raise ValueError("native packet flags must use only the high nibble")
    return kind | flags


def _texture_word(face: project.Face) -> int:
    if face.texture is None:
        texture = 0
    else:
        texture = face.texture
    if texture < 0 or texture > 0x3FFF:
        raise ValueError("native texture index must fit fourteen bits")
    return texture | face.texture_flags


def _textured_tail(
    dialect: str,
    face: project.Face,
) -> bytes:
    if dialect == "V8_2":
        return (
            bytes(face.uv[0])
            + b"\0\0"
            + bytes(face.uv[1])
            + b"\0\0"
            + bytes(face.uv[2])
            + struct.pack("<H", _texture_word(face))
        )
    return (
        bytes(face.uv[0])
        + bytes(face.uv[1])
        + bytes(face.uv[2])
        + struct.pack("<H", _texture_word(face))
    )


def _normal_indices(
    face: project.Face, count: int, context: str
) -> tuple[int, ...]:
    if len(face.normal_indices) != count:
        raise ValueError(f"{context} requires {count} native normal indices")
    return tuple(_u16(index, context) for index in face.normal_indices)


def _compile_face(dialect: str, face: project.Face) -> bytes:
    kind = (
        (0 if face.texture is None else 5)
        if face.packet_kind is None
        else face.packet_kind
    )
    header = struct.pack(
        "<BBBBHHH",
        *face.color,
        _packet_type(kind, face.packet_flags),
        *face.vertices,
    )
    parameter = _u16(face.material_parameter, "packet material parameter")

    if kind in {0, 6}:
        return header + struct.pack("<H", parameter)
    if kind == 4:
        return header + struct.pack(
            "<H", _normal_indices(face, 1, "flat normal")[0]
        )
    if kind in {5, 7}:
        normal = _normal_indices(face, 1, "flat textured normal")[0]
        return header + struct.pack("<H", normal) + _textured_tail(dialect, face)
    if kind == 8:
        return header + struct.pack(
            "<HHH", *_normal_indices(face, 3, "Gouraud normals")
        )
    if kind in {9, 11}:
        return (
            header
            + struct.pack(
                "<HHH", *_normal_indices(face, 3, "textured Gouraud normals")
            )
            + _textured_tail(dialect, face)
        )
    if kind == 2:
        if len(face.gouraud_colors) != 2:
            raise ValueError("kind-2 packet requires two trailing Gouraud colors")
        return (
            header
            + struct.pack("<H", parameter)
            + b"".join(bytes(color) + b"\0" for color in face.gouraud_colors)
        )
    if kind in {1, 3}:
        if len(face.gouraud_colors) != 2:
            raise ValueError(
                f"kind-{kind} packet requires two trailing Gouraud colors"
            )
        return (
            header
            + struct.pack("<H", parameter)
            + _textured_tail(dialect, face)
            + b"".join(bytes(color) + b"\0" for color in face.gouraud_colors)
        )
    if kind == 12:
        normals = struct.pack(
            "<HHH", *_normal_indices(face, 3, "environment normals")
        )
        if dialect == "V8_2":
            if len(face.environment_parameters) != 4:
                raise ValueError(
                    "V8:2 kind-12 packet requires four environment parameters"
                )
            environment = struct.pack(
                "<HHHH", *face.environment_parameters
            )
        else:
            if len(face.environment_parameters) != 2:
                raise ValueError(
                    "V8 kind-12 packet requires two environment parameters"
                )
            environment = struct.pack(
                "<HH", *face.environment_parameters
            )
        return header + normals + environment
    if kind in {13, 15}:
        return (
            header
            + struct.pack("<H", parameter)
            + _textured_tail(dialect, face)
        )
    raise ValueError(
        f"native triangle packet kind {kind} is not represented by Face"
    )


def _compile_control(control: project.RenderControl) -> bytes:
    if control.packet_kind != 10:
        raise ValueError("only native kind-10 control packets are defined")
    return (
        struct.pack(
            "<BBBBHHHH",
            *control.color,
            _packet_type(control.packet_kind, control.packet_flags),
            *control.vertex_indices,
            len(control.entries),
        )
        + b"".join(struct.pack("<HHHH", *entry) for entry in control.entries)
    )


def _compile_packets(
    dialect: str, group: project.RenderGroup
) -> tuple[bytes, tuple[tuple[int, int, int, int], ...]]:
    normals = list(group.normals)
    faces = []
    for sequential_index, source_face in enumerate(group.faces):
        face = source_face
        if face.packet_kind is None and face.texture is not None:
            normal_index = len(normals)
            normals.append(_face_normal(group.vertices, face))
            face = project.Face(
                vertices=face.vertices,
                color=face.color,
                texture=face.texture,
                uv=face.uv,
                packet_index=face.packet_index,
                packet_kind=5,
                packet_flags=face.packet_flags,
                normal_indices=(normal_index,),
                material_parameter=face.material_parameter,
                texture_flags=face.texture_flags,
                gouraud_colors=face.gouraud_colors,
                environment_parameters=face.environment_parameters,
                post_parameter=face.post_parameter,
            )
        index = (
            sequential_index
            if face.packet_index is None
            else face.packet_index
        )
        faces.append((index, _compile_face(dialect, face)))
    controls = [
        (control.packet_index, _compile_control(control))
        for control in group.controls
    ]
    ordered = sorted(faces + controls, key=lambda item: item[0])
    if [index for index, _data in ordered] != list(range(len(ordered))):
        raise ValueError(
            f"group {group.name!r} packet indices must be contiguous"
        )
    return b"".join(data for _index, data in ordered), tuple(normals)


def _compile_group(dialect: str, group: project.RenderGroup) -> bytes:
    packets, normals = _compile_packets(dialect, group)
    descriptor_size = 0x1C
    packet_offset = descriptor_size
    vertex_offset = packet_offset + len(packets)
    vertices = b"".join(
        struct.pack("<hhhh", vertex[0], vertex[1], vertex[2], 0)
        for vertex in group.vertices
    )
    normal_offset = vertex_offset + len(vertices)
    normal_data = b"".join(struct.pack("<hhhh", *normal) for normal in normals)
    descriptor = struct.pack(
        "<IIIIHhIB3x",
        len(group.vertices),
        vertex_offset,
        len(normals),
        normal_offset,
        len(group.faces) + len(group.controls),
        0,
        packet_offset,
        group.scale_shift,
    )
    return descriptor + packets + vertices + normal_data


def _compile_collision_stream(stream: project.CollisionStream) -> bytes:
    data = bytearray()
    for shape in stream.shapes:
        if isinstance(shape, project.CollisionAabb):
            data += struct.pack(
                "<hhiiiiii",
                1,
                _i16(shape.flags, "AABB flags"),
                shape.minimum[0],
                shape.override_y,
                shape.minimum[2],
                shape.maximum[0],
                shape.maximum[1],
                shape.maximum[2],
            )
        elif isinstance(shape, project.CollisionConvex):
            data += struct.pack("<hH", 2, len(shape.planes))
            for plane in shape.planes:
                data += struct.pack(
                    "<hhhhI",
                    _i16(plane.normal[0], "plane normal"),
                    _i16(plane.normal[1], "plane normal"),
                    _i16(plane.normal[2], "plane normal"),
                    _i16(plane.pad, "plane pad"),
                    plane.offset & 0xFFFFFFFF,
                )
        else:
            raise TypeError(shape)
    data += struct.pack("<h", 0)
    _align(data)
    return bytes(data)


def _pack_texture_pixels(texture: project.Texture) -> bytes:
    if texture.depth == 0:
        packed = bytearray((len(texture.indices) + 1) // 2)
        for pixel, index in enumerate(texture.indices):
            packed[pixel >> 1] |= index << (4 if pixel & 1 else 0)
        return bytes(packed)
    if texture.depth == 2:
        return b"".join(
            struct.pack("<H", color)
            for color in texture.direct_pixels_bgr555
        )
    return texture.indices


def _compile_texture(texture: project.Texture) -> bytes:
    packed = _pack_texture_pixels(texture)
    encoded = (
        xobf.compress_v82_texture(packed) if texture.compressed else packed
    )
    if texture.depth == 2:
        # Direct-color records have no CLUT. The image RECT occupies
        # record+0x0c and compressed/uncompressed 16-bit pixels begin at
        # record+0x14, exactly as the retail V8:2 loader consumes them.
        flags = texture.depth | (0x10 if texture.compressed else 0)
        data = bytearray(
            struct.pack(
                "<IIIhhhh",
                0x10,
                flags,
                0x10,
                texture.image_origin[0],
                texture.image_origin[1],
                texture.width,
                texture.height,
            )
        )
        data += encoded
        _align(data)
        return bytes(data)

    flags = 0x08 | texture.depth | (0x10 if texture.compressed else 0)
    data = bytearray(
        struct.pack(
            "<IIIhhhh",
            0x10,
            flags,
            0,
            texture.palette_origin[0],
            texture.palette_origin[1],
            len(texture.palette_bgr555),
            1,
        )
    )
    data += b"".join(
        struct.pack("<H", color) for color in texture.palette_bgr555
    )
    _align(data)
    image_offset = len(data)
    pixels_per_word = 4 if texture.depth == 0 else 2
    words = texture.width // pixels_per_word
    if words * pixels_per_word != texture.width:
        raise ValueError(
            f"texture {texture.name!r} width is not aligned to its PS1 depth"
        )
    data += b"\0" * 12
    data += struct.pack(
        "<hhhh",
        texture.image_origin[0],
        texture.image_origin[1],
        words,
        texture.height,
    )
    data += encoded
    struct.pack_into("<I", data, 8, image_offset)
    _align(data)
    return bytes(data)


def _slot_links(slots: Sequence[project.Slot]) -> tuple[tuple[int, int], ...]:
    children: dict[int | None, list[int]] = {}
    for index, slot in enumerate(slots):
        # Top-level roots are siblings too. Location banks in particular are
        # one native root chain; omitting those links makes only the first
        # wheel object reachable even though every slot reparses in isolation.
        children.setdefault(slot.parent, []).append(index)
    result = []
    for index in range(len(slots)):
        siblings = children.get(slots[index].parent, [])
        sibling_position = siblings.index(index) if index in siblings else -1
        next_sibling = (
            siblings[sibling_position + 1]
            if sibling_position >= 0 and sibling_position + 1 < len(siblings)
            else 0xFFFF
        )
        own_children = children.get(index, [])
        first_child = own_children[0] if own_children else 0xFFFF
        result.append((next_sibling, first_child))
    return tuple(result)


def compile_model(
    vehicle: project.VehicleProject, bank: project.ObjectBank | None = None
) -> bytes:
    """Compile a complete BIN payload without reading any source asset."""

    vehicle.validate()
    source = vehicle if bank is None else bank
    slot_links = _slot_links(source.slots)
    output = bytearray(0x1C + len(source.slots) * 0x1C)
    for index, slot in enumerate(source.slots):
        next_sibling, first_child = slot_links[index]
        struct.pack_into(
            "<hhiiihhhhHH",
            output,
            0x1C + index * 0x1C,
            _native_slot_key(slot),
            -1 if slot.collision is None else slot.collision,
            *slot.position,
            slot.rotation_yxz[0],
            slot.rotation_yxz[1],
            slot.rotation_yxz[2],
            slot.flags,
            next_sibling,
            first_child,
        )
    _align(output)

    group_table = len(output)
    output += b"\0" * (len(source.groups) * 4)
    for index, group in enumerate(source.groups):
        _align(output)
        target = len(output)
        struct.pack_into("<I", output, group_table + index * 4, target - group_table)
        output += _compile_group(vehicle.game, group)

    _align(output)
    collision_table = len(output)
    output += b"\0" * (len(source.collisions) * 4)
    for index, stream in enumerate(source.collisions):
        _align(output)
        target = len(output)
        struct.pack_into(
            "<I", output, collision_table + index * 4, target - collision_table
        )
        output += _compile_collision_stream(stream)

    _align(output)
    texture_table = len(output)
    output += b"\0" * (len(source.textures) * 4)
    for index, texture in enumerate(source.textures):
        _align(output)
        target = len(output)
        struct.pack_into(
            "<I", output, texture_table + index * 4, target - texture_table
        )
        output += _compile_texture(texture)

    struct.pack_into(
        "<IIIIIII",
        output,
        0,
        len(source.groups),
        group_table,
        len(source.collisions),
        collision_table,
        len(source.textures),
        texture_table,
        len(source.slots),
    )
    return bytes(output)


def _compile_animation_frame(frame: project.AnimationFrame) -> bytes:
    flags = 0
    if frame.rotation_yxz is not None:
        flags |= 0x01
    if frame.translation_absolute is not None:
        flags |= 0x02
    if frame.translation_delta is not None:
        flags |= 0x08
    if frame.texture_bindings:
        flags |= 0x10
    if frame.scale is not None:
        flags |= 0x20
    if frame.mesh_vectors:
        flags |= 0x40
    data = bytearray(struct.pack("<hh", _i16(frame.frame_delta, "frame delta"), flags))
    if frame.rotation_yxz is not None:
        data += struct.pack(
            "<hhhh",
            *(_i16(value, "rotation") for value in frame.rotation_yxz),
            _i16(frame.rotation_parameter, "rotation parameter"),
        )
    if frame.translation_absolute is not None:
        data += struct.pack("<iii", *frame.translation_absolute)
    if frame.translation_delta is not None:
        data += struct.pack(
            "<hhhh",
            *(_i16(value, "translation delta") for value in frame.translation_delta),
            0,
        )
    if frame.texture_bindings:
        for index, binding in enumerate(frame.texture_bindings):
            target = _u16(
                binding.target, "animation texture target"
            ) & 0x7FFF
            if index == len(frame.texture_bindings) - 1:
                target |= 0x8000
            data += struct.pack(
                "<HH",
                target,
                _u16(binding.texture, "animation texture index"),
            )
    if frame.scale is not None:
        data += struct.pack(
            "<hhhh", *(_i16(value, "scale") for value in frame.scale)
        )
    if frame.mesh_vectors:
        data += struct.pack("<I", len(frame.mesh_vectors))
        for vector in frame.mesh_vectors:
            data += struct.pack(
                "<hhhh", *(_i16(value, "mesh vector") for value in vector)
            )
    return bytes(data)


def compile_animation(
    vehicle: project.VehicleProject, bank: project.ObjectBank | None = None
) -> bytes | None:
    source = vehicle if bank is None else bank
    if not source.animations:
        return None
    streams = {animation.slot: animation for animation in source.animations}
    output = bytearray(4 + len(source.slots) * 4)
    maximum_duration = 0
    for slot_index in range(len(source.slots)):
        animation = streams.get(slot_index)
        if animation is None:
            continue
        _align(output)
        stream_start = len(output)
        struct.pack_into("<I", output, 4 + slot_index * 4, stream_start)
        duration = 0
        for frame in animation.frames:
            output += _compile_animation_frame(frame)
            duration += max(0, frame.frame_delta)
        if animation.loop:
            jump_start = len(output)
            jump = stream_start - jump_start
            output += struct.pack(
                "<hh",
                _i16(max(1, duration), "animation loop duration"),
                _i16(jump, "animation loop jump"),
            )
        maximum_duration = max(maximum_duration, duration)
    struct.pack_into("<I", output, 0, maximum_duration)
    return bytes(output)


def compile_xobf(
    vehicle: project.VehicleProject, bank: project.ObjectBank | None = None
) -> iff.IffChunk:
    children = [
        iff.IffChunk(tag=b"BIN ", payload=compile_model(vehicle, bank))
    ]
    animation = compile_animation(vehicle, bank)
    if animation is not None:
        children.append(iff.IffChunk(tag=b"ANM ", payload=animation))
    return iff.IffChunk(tag=b"FORM", form_type=b"XOBF", children=children)


def compile_archive(vehicles: Iterable[project.VehicleProject]) -> bytes:
    projects = tuple(vehicles)
    if not projects:
        raise ValueError("an archive requires at least one vehicle")
    games = {vehicle.game for vehicle in projects}
    if len(games) != 1:
        raise ValueError("one native archive cannot mix V8 and V8:2 packet dialects")
    chunks = []
    for vehicle in projects:
        chunks.append(compile_xobf(vehicle))
        if vehicle.transformation_bank is not None:
            chunks.append(compile_xobf(vehicle, vehicle.transformation_bank))
        if vehicle.selector_preview_bank is not None:
            chunks.append(compile_xobf(vehicle, vehicle.selector_preview_bank))
    document = iff.IffDocument(chunks=chunks)
    return document.to_bytes()
