"""Generated append-only vehicle registry shared with the ported engines."""

from __future__ import annotations

from dataclasses import dataclass
import struct
from typing import Iterable

from . import compiler, iff, project, stats, xobf


MAGIC = b"V8VR"
LEGACY_VERSION = 2
PREVIEWLESS_VERSION = 3
VERSION = 4
GAME_IDS = {"V8": 1, "V8_2": 2}
HEADER_FORMAT = "<4sHBBHHII"
LEGACY_ENTRY_FORMAT = "<IIHHIHHHHIII"
PREVIEWLESS_ENTRY_FORMAT = "<IIHHIHHHHIIIHH"
ENTRY_FORMAT = "<IIHHIHHHHIIIHHHH"
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)
LEGACY_ENTRY_SIZE = struct.calcsize(LEGACY_ENTRY_FORMAT)
PREVIEWLESS_ENTRY_SIZE = struct.calcsize(PREVIEWLESS_ENTRY_FORMAT)
ENTRY_SIZE = struct.calcsize(ENTRY_FORMAT)
NO_ARCHIVE_INDEX = 0xFFFF


@dataclass(frozen=True)
class RegistryEntry:
    stable_id: str
    display_name: str
    archive_index: int
    transformation_archive_index: int | None
    selector_preview_archive_index: int | None
    flags: int
    stats_record: bytes
    body_kind: int
    selector_preview_body_kind: int
    selection_order: int
    rear_suspension_damping: int | None
    transform_modes: tuple[tuple[int, ...], ...]
    powerups: dict[str, int]


@dataclass(frozen=True)
class CompiledVehiclePackage:
    game: str
    archive: bytes
    registry: bytes


def compile_stats_record(
    vehicle: project.VehicleProject, assigned_vehicle_type: int
) -> bytes:
    profile = stats.PROFILES[vehicle.game]
    output = bytearray(profile.record_size)
    for field in profile.fields:
        value = (
            assigned_vehicle_type
            if field.name == "vehicle_type"
            else vehicle.stats[field.name]
        )
        try:
            struct.pack_into("<" + field.format, output, field.offset, value)
        except struct.error as error:
            raise ValueError(
                f"{vehicle.stable_id} stat {field.name}={value} is outside "
                "its native range"
            ) from error
    return bytes(output)


def compile_registry(vehicles: Iterable[project.VehicleProject]) -> bytes:
    projects = tuple(vehicles)
    if not projects:
        raise ValueError("a registry requires at least one vehicle")
    if len(projects) > project.MAX_CUSTOM_VEHICLES:
        raise ValueError(
            f"a registry supports at most {project.MAX_CUSTOM_VEHICLES} "
            "one-byte custom vehicle identities"
        )
    games = {vehicle.game for vehicle in projects}
    if len(games) != 1:
        raise ValueError("a registry cannot mix V8 and V8:2 projects")
    game = projects[0].game
    stable_ids = [vehicle.stable_id for vehicle in projects]
    if len(set(stable_ids)) != len(stable_ids):
        raise ValueError("stable vehicle IDs must be unique")

    entry_table_offset = HEADER_SIZE
    output = bytearray(HEADER_SIZE + len(projects) * ENTRY_SIZE)
    archive_indices = []
    next_archive_index = 0
    for vehicle in projects:
        body_index = next_archive_index
        next_archive_index += 1
        transform_index = NO_ARCHIVE_INDEX
        if vehicle.transformation_bank is not None:
            transform_index = next_archive_index
            next_archive_index += 1
        preview_index = NO_ARCHIVE_INDEX
        if vehicle.selector_preview_bank is not None:
            preview_index = next_archive_index
            next_archive_index += 1
        archive_indices.append((body_index, transform_index, preview_index))
    stat_offsets = []
    stat_records = []
    # Types are resolved to final global IDs by the engine after the retail
    # roster. The stored local type is deterministic and is overwritten during
    # registration, avoiding identity == archive-index assumptions.
    for local_index, vehicle in enumerate(projects):
        record = compile_stats_record(vehicle, local_index)
        stat_offsets.append(len(output))
        stat_records.append(record)
        output += record
    transform_offsets = []
    powerup_offsets = []
    for vehicle in projects:
        if vehicle.game == "V8_2":
            transform_offsets.append(len(output))
            for mode in vehicle.transform_modes:
                output += struct.pack("<6H", *mode)
            powerup_offsets.append(len(output))
            output += struct.pack(
                "<5I",
                *(vehicle.powerups[name] for name in project.V82_POWERUP_FIELDS),
            )
        else:
            transform_offsets.append(0)
            powerup_offsets.append(0)
    string_table_offset = len(output)
    string_offsets: dict[str, int] = {}

    def add_string(value: str) -> int:
        if value in string_offsets:
            return string_offsets[value]
        encoded = value.encode("utf-8")
        if b"\0" in encoded:
            raise ValueError("registry strings cannot contain NUL")
        offset = len(output)
        output.extend(encoded + b"\0")
        string_offsets[value] = offset
        return offset

    entries = []
    for index, vehicle in enumerate(projects):
        stable_offset = add_string(vehicle.stable_id)
        display_offset = add_string(vehicle.display_name)
        (
            body_archive_index,
            transform_archive_index,
            selector_preview_archive_index,
        ) = archive_indices[index]
        entries.append(
            (
                stable_offset,
                display_offset,
                body_archive_index,
                transform_archive_index,
                0,
                len(stat_records[index]),
                vehicle.body_kind,
                index,
                0,
                stat_offsets[index],
                transform_offsets[index],
                powerup_offsets[index],
                (
                    vehicle.stats["rear_suspension_damping"]
                    if game == "V8_2"
                    else 0
                ),
                0,
                selector_preview_archive_index,
                vehicle.selector_preview_body_kind,
            )
        )
    struct.pack_into(
        HEADER_FORMAT,
        output,
        0,
        MAGIC,
        VERSION,
        GAME_IDS[game],
        0,
        len(projects),
        ENTRY_SIZE,
        entry_table_offset,
        string_table_offset,
    )
    for index, entry in enumerate(entries):
        struct.pack_into(
            ENTRY_FORMAT, output, entry_table_offset + index * ENTRY_SIZE, *entry
        )
    return bytes(output)


def parse_registry(data: bytes) -> tuple[str, tuple[RegistryEntry, ...]]:
    if len(data) < HEADER_SIZE:
        raise ValueError("vehicle registry is truncated")
    (
        magic,
        version,
        game_id,
        reserved,
        count,
        entry_size,
        table_offset,
        string_offset,
    ) = struct.unpack_from(HEADER_FORMAT, data)
    if (
        magic != MAGIC
        or version not in {LEGACY_VERSION, PREVIEWLESS_VERSION, VERSION}
        or reserved != 0
    ):
        raise ValueError("vehicle registry header is invalid")
    expected_entry_size = {
        LEGACY_VERSION: LEGACY_ENTRY_SIZE,
        PREVIEWLESS_VERSION: PREVIEWLESS_ENTRY_SIZE,
        VERSION: ENTRY_SIZE,
    }[version]
    if entry_size != expected_entry_size:
        raise ValueError("vehicle registry entry size is unsupported")
    games = {value: name for name, value in GAME_IDS.items()}
    if game_id not in games:
        raise ValueError("vehicle registry game ID is invalid")
    if table_offset + count * entry_size > len(data):
        raise ValueError("vehicle registry table is truncated")
    if string_offset > len(data):
        raise ValueError("vehicle registry string table is invalid")

    def read_string(offset: int) -> str:
        if offset < string_offset or offset >= len(data):
            raise ValueError("vehicle registry string offset is invalid")
        end = data.find(b"\0", offset)
        if end < 0:
            raise ValueError("vehicle registry string is unterminated")
        return data[offset:end].decode("utf-8")

    entries = []
    for index in range(count):
        entry_format = {
            LEGACY_VERSION: LEGACY_ENTRY_FORMAT,
            PREVIEWLESS_VERSION: PREVIEWLESS_ENTRY_FORMAT,
            VERSION: ENTRY_FORMAT,
        }[version]
        entry_values = struct.unpack_from(
            entry_format, data, table_offset + index * entry_size
        )
        (
            stable_offset,
            display_offset,
            archive_index,
            transform_archive_index,
            flags,
            stat_size,
            body_kind,
            selection_order,
            entry_reserved,
            stat_offset,
            transform_offset,
            powerup_offset,
        ) = entry_values[:12]
        rear_suspension_damping = (
            None if version == LEGACY_VERSION else entry_values[12]
        )
        extension_reserved = 0 if version == LEGACY_VERSION else entry_values[13]
        selector_preview_archive_index = (
            NO_ARCHIVE_INDEX if version != VERSION else entry_values[14]
        )
        selector_preview_body_kind = (
            0 if version != VERSION else entry_values[15]
        )
        if entry_reserved != 0:
            raise ValueError("vehicle registry entry reserved field is nonzero")
        if extension_reserved != 0:
            raise ValueError("vehicle registry extension reserved field is nonzero")
        if stat_offset + stat_size > len(data):
            raise ValueError("vehicle registry entry is invalid")
        game = games[game_id]
        transform_modes: tuple[tuple[int, ...], ...] = ()
        powerups: dict[str, int] = {}
        if game == "V8_2":
            if rear_suspension_damping is None:
                rear_suspension_damping = struct.unpack_from(
                    "<H", data, stat_offset + 0x0C
                )[0]
            if rear_suspension_damping > 0x7FFF:
                raise ValueError(
                    "V8:2 rear suspension damping must be 0..32767"
                )
            transform_size = (
                project.V82_TRANSFORM_MODE_COUNT
                * project.V82_TRANSFORM_WHEEL_COUNT
                * 2
            )
            powerup_size = len(project.V82_POWERUP_FIELDS) * 4
            if (
                transform_archive_index == NO_ARCHIVE_INDEX
                or transform_offset == 0
                or transform_offset + transform_size > len(data)
                or powerup_offset == 0
                or powerup_offset + powerup_size > len(data)
            ):
                raise ValueError("V8:2 registry transformation data is invalid")
            flat_modes = struct.unpack_from("<24H", data, transform_offset)
            transform_modes = tuple(
                tuple(
                    flat_modes[
                        mode * project.V82_TRANSFORM_WHEEL_COUNT :
                        (mode + 1) * project.V82_TRANSFORM_WHEEL_COUNT
                    ]
                )
                for mode in range(project.V82_TRANSFORM_MODE_COUNT)
            )
            powerup_values = struct.unpack_from("<5I", data, powerup_offset)
            powerups = dict(zip(project.V82_POWERUP_FIELDS, powerup_values))
        elif (
            transform_archive_index != NO_ARCHIVE_INDEX
            or transform_offset != 0
            or powerup_offset != 0
            or rear_suspension_damping not in {None, 0}
        ):
            raise ValueError("V8 registry contains V8:2-only data")
        entries.append(
            RegistryEntry(
                stable_id=read_string(stable_offset),
                display_name=read_string(display_offset),
                archive_index=archive_index,
                transformation_archive_index=(
                    None
                    if transform_archive_index == NO_ARCHIVE_INDEX
                    else transform_archive_index
                ),
                selector_preview_archive_index=(
                    None
                    if selector_preview_archive_index == NO_ARCHIVE_INDEX
                    else selector_preview_archive_index
                ),
                flags=flags,
                stats_record=data[stat_offset : stat_offset + stat_size],
                body_kind=body_kind,
                selector_preview_body_kind=selector_preview_body_kind,
                selection_order=selection_order,
                rear_suspension_damping=rear_suspension_damping,
                transform_modes=transform_modes,
                powerups=powerups,
            )
        )
    return games[game_id], tuple(entries)


def _decode_parents(slots: tuple[xobf.Slot, ...]) -> tuple[int | None, ...]:
    parents: list[int | None] = [None] * len(slots)
    for parent, slot in enumerate(slots):
        child = slot.first_child
        visited: set[int] = set()
        while child != 0xFFFF:
            if child >= len(slots) or child in visited:
                raise ValueError("native slot hierarchy is invalid")
            visited.add(child)
            if parents[child] is not None:
                raise ValueError("native slot has more than one parent")
            parents[child] = parent
            child = slots[child].next_sibling
    return tuple(parents)


def _decode_animations(
    payload: bytes | None, slot_count: int
) -> tuple[project.SlotAnimation, ...]:
    if payload is None:
        return ()
    bank = xobf.AnimationBank(payload, slot_count)
    animations = []
    for slot in range(slot_count):
        native_frames = list(bank.frames(slot))
        if not native_frames:
            continue
        loop = native_frames[-1].is_jump
        if loop:
            native_frames.pop()
        if any(frame.is_jump for frame in native_frames):
            raise ValueError("native animation has a nonterminal jump frame")
        frames = []
        for frame in native_frames:
            mesh_vectors: tuple[tuple[int, int, int, int], ...] = ()
            if frame.mesh_data is not None:
                count = struct.unpack_from("<I", frame.mesh_data)[0]
                if 4 + count * 8 != len(frame.mesh_data):
                    raise ValueError("native animation mesh vectors are invalid")
                mesh_vectors = tuple(
                    struct.unpack_from("<hhhh", frame.mesh_data, 4 + index * 8)
                    for index in range(count)
                )
            frames.append(
                project.AnimationFrame(
                    frame_delta=frame.frame_delta,
                    rotation_yxz=frame.rotation_yxz,
                    rotation_parameter=frame.rotation_parameter,
                    translation_absolute=frame.translation_absolute,
                    translation_delta=frame.translation_delta,
                    texture_bindings=tuple(
                        project.TextureBinding(
                            target=target,
                            texture=texture,
                        )
                        for target, texture in frame.texture_bindings
                    ),
                    scale=frame.scale,
                    mesh_vectors=mesh_vectors,
                )
            )
        animations.append(
            project.SlotAnimation(
                slot=slot, loop=loop, frames=tuple(frames)
            )
        )
    return tuple(animations)


def _decode_face(
    packet: xobf.PolygonPacket,
    packet_index: int,
    native_group: xobf.RenderGroup,
    model: xobf.Model,
) -> project.Face:
    kind = packet.kind
    raw = packet.raw
    normal_indices: tuple[int, ...] = ()
    material_parameter = 0
    gouraud_colors: tuple[tuple[int, int, int], ...] = ()
    environment_parameters: tuple[int, ...] = ()
    post_parameter = 0

    if kind in {4, 5, 7}:
        normal_indices = (struct.unpack_from("<H", raw, 0x0A)[0],)
    elif kind in {8, 9, 11, 12}:
        normal_indices = struct.unpack_from("<HHH", raw, 0x0A)
    else:
        material_parameter = struct.unpack_from("<H", raw, 0x0A)[0]

    if kind == 2:
        gouraud_colors = (
            tuple(raw[0x0C:0x0F]),
            tuple(raw[0x10:0x13]),
        )
    elif kind in {1, 3}:
        color_offset = 0x18 if model.dialect == "V8_2" else 0x14
        gouraud_colors = (
            tuple(raw[color_offset : color_offset + 3]),
            tuple(raw[color_offset + 4 : color_offset + 7]),
        )

    if kind == 12:
        if model.dialect == "V8_2":
            environment_parameters = struct.unpack_from("<HHHH", raw, 0x10)
        else:
            environment_parameters = struct.unpack_from("<HH", raw, 0x10)

    textured_kinds = {1, 3, 5, 7, 9, 11, 13, 15}
    texture = None
    native_texture_slot = None
    texture_flags = 0
    uv = ((0, 0), (0, 0), (0, 0))
    if kind == 12:
        # Environment packets do not carry ordinary per-vertex UVs.  Their
        # first environment parameter selects either a bank texture directly
        # or one of the engine-global 0x3FFE/0x3FFF maps, while the renderer
        # derives UVs from the transformed vertex normals every frame.
        environment_texture = environment_parameters[0] & 0x3FFF
        if environment_texture < model.texture_count:
            texture = environment_texture
    if kind in textured_kinds:
        if packet.texture_slot is None:
            raise ValueError(
                f"native textured packet kind {kind} has no texture word"
            )
        texture_flags = packet.texture_slot & 0xC000
        native_texture_slot = packet.texture_slot & 0x3FFF
        texture = native_texture_slot
        if texture < 0 or texture >= model.texture_count:
            raise ValueError(
                "native textured triangle references an invalid texture"
            )
        if kind != 12:
            if len(packet.uv) != 3:
                raise ValueError("native textured triangle is incomplete")
            uv = packet.uv

    return project.Face(
        vertices=packet.vertex_indices,
        color=packet.color,
        texture=texture,
        native_texture_slot=native_texture_slot,
        uv=uv,
        packet_index=packet_index,
        packet_kind=kind,
        packet_flags=raw[3] & 0xF0,
        normal_indices=normal_indices,
        material_parameter=material_parameter,
        texture_flags=texture_flags,
        gouraud_colors=gouraud_colors,
        environment_parameters=environment_parameters,
        post_parameter=post_parameter,
    )


def _decode_control(
    packet: xobf.PolygonPacket, packet_index: int
) -> project.RenderControl:
    if packet.kind != 10:
        raise ValueError(
            f"native non-triangle packet kind {packet.kind} is not decoded"
        )
    count = struct.unpack_from("<H", packet.raw, 0x0A)[0]
    entries = tuple(
        struct.unpack_from("<HHHH", packet.raw, 0x0C + index * 8)
        for index in range(count)
    )
    return project.RenderControl(
        packet_index=packet_index,
        packet_kind=packet.kind,
        packet_flags=packet.raw[3] & 0xF0,
        color=packet.color,
        vertex_indices=packet.vertex_indices,
        entries=entries,
    )


def _decode_bank(form: iff.IffChunk, game: str) -> project.ObjectBank:
    if form.tag != b"FORM" or form.form_type != b"XOBF":
        raise ValueError("vehicle archive contains a non-XOBF form")
    chunks: dict[bytes, bytes] = {}
    for child in form.children:
        if child.tag not in {b"BIN ", b"ANM "}:
            raise ValueError(
                f"XOBF contains unsupported chunk {child.tag!r}")
        if child.tag in chunks:
            raise ValueError(f"XOBF contains duplicate chunk {child.tag!r}")
        chunks[child.tag] = child.payload
    if b"BIN " not in chunks:
        raise ValueError("XOBF has no BIN chunk")

    model = xobf.Model(chunks[b"BIN "], dialect=game)
    native_groups = tuple(model.groups())
    groups = []
    for native_group in native_groups:
        faces = []
        controls = []
        for packet_index, packet in enumerate(native_group.packets):
            if packet.kind in xobf.NON_VERTEX_PACKET_KINDS:
                controls.append(_decode_control(packet, packet_index))
            else:
                faces.append(
                    _decode_face(
                        packet,
                        packet_index,
                        native_group,
                        model,
                    )
                )
        groups.append(
            project.RenderGroup(
                name=f"group_{native_group.index:03d}",
                scale_shift=native_group.scale_shift,
                vertices=tuple(
                    (vertex.x, vertex.y, vertex.z)
                    for vertex in native_group.vertices
                ),
                faces=tuple(faces),
                normals=tuple(
                    (normal.x, normal.y, normal.z, normal.pad)
                    for normal in native_group.normals
                ),
                controls=tuple(controls),
                texture_slot_count=native_group.texture_slot_count,
                render_extent=native_group.render_extent,
            )
        )

    native_slots = model.slots()
    parents = _decode_parents(native_slots)
    decoded_slots = []
    for slot in native_slots:
        raw_key = slot.render_key & 0xFFFF
        render_group = slot.render_group
        render_flags = slot.render_flags
        if render_group is not None and render_group >= model.group_count:
            render_group = None
            render_flags = 0
        decoded_slots.append(
            project.Slot(
            name=f"slot_{slot.index:03d}",
            render_group=render_group,
            render_flags=render_flags,
            collision=(
                None if slot.obstacle_index < 0 else slot.obstacle_index
            ),
            key=(
                None
                if render_group is not None or slot.render_key == -1
                else raw_key
            ),
            position=(slot.x, slot.y, slot.z),
            rotation_yxz=(slot.rot_y, slot.rot_x, slot.rot_z),
            flags=slot.flags,
            parent=parents[slot.index],
        )
        )
    slots = tuple(decoded_slots)

    collisions = []
    for native_stream in model.collisions():
        shapes = []
        for shape in native_stream.shapes:
            if isinstance(shape, xobf.CollisionAabb):
                shapes.append(
                    project.CollisionAabb(
                        minimum=(
                            shape.minimum_x,
                            shape.override_y,
                            shape.minimum_z,
                        ),
                        maximum=(
                            shape.maximum_x,
                            shape.maximum_y,
                            shape.maximum_z,
                        ),
                        override_y=shape.override_y,
                        flags=shape.flags,
                    )
                )
            else:
                shapes.append(
                    project.CollisionConvex(
                        planes=tuple(
                            project.CollisionPlane(
                                normal=(
                                    plane.normal_x,
                                    plane.normal_y,
                                    plane.normal_z,
                                ),
                                offset=plane.offset,
                                pad=plane.pad,
                            )
                            for plane in shape.planes
                        )
                    )
                )
        collisions.append(
            project.CollisionStream(
                name=f"collision_{native_stream.index:03d}",
                shapes=tuple(shapes),
            )
        )

    textures = []
    for native_texture in model.textures():
        if not native_texture.supported:
            raise ValueError(
                "authored package contains an unsupported renderer-only texture")
        textures.append(
            project.Texture(
                name=f"texture_{native_texture.index:03d}",
                width=native_texture.width,
                height=native_texture.height,
                depth=native_texture.depth,
                palette_bgr555=native_texture.palette,
                indices=native_texture.indices,
                compressed=native_texture.compressed,
                direct_pixels_bgr555=(
                    native_texture.direct_pixels_bgr555
                ),
                palette_origin=native_texture.palette_origin,
                image_origin=native_texture.image_origin,
            )
        )

    return project.ObjectBank(
        groups=tuple(groups),
        slots=slots,
        collisions=tuple(collisions),
        textures=tuple(textures),
        animations=_decode_animations(
            chunks.get(b"ANM "), model.slot_count),
    )


def decompile_package(
    archive_data: bytes, registry_data: bytes
) -> tuple[project.VehicleProject, ...]:
    """Decode a strict authored package without retaining source bytes."""

    game, entries = parse_registry(registry_data)
    forms = tuple(iff.parse(archive_data).forms(b"XOBF"))
    if not forms:
        raise ValueError("vehicle archive contains no XOBF banks")
    referenced: set[int] = set()
    vehicles = []
    profile = stats.PROFILES[game]
    for index, entry in enumerate(entries):
        if entry.flags != 0 or entry.selection_order != index:
            raise ValueError("vehicle registry contains unsupported entry metadata")
        if len(entry.stats_record) != profile.record_size:
            raise ValueError("vehicle registry stat record size is invalid")
        bank_indices = [entry.archive_index]
        if entry.transformation_archive_index is not None:
            bank_indices.append(entry.transformation_archive_index)
        if entry.selector_preview_archive_index is not None:
            bank_indices.append(entry.selector_preview_archive_index)
        if any(bank < 0 or bank >= len(forms) for bank in bank_indices):
            raise ValueError("vehicle registry references a missing XOBF bank")
        if any(bank in referenced for bank in bank_indices):
            raise ValueError("vehicle registry shares an owned XOBF bank")
        referenced.update(bank_indices)

        body = _decode_bank(forms[entry.archive_index], game)
        transformation = (
            None
            if entry.transformation_archive_index is None
            else _decode_bank(
                forms[entry.transformation_archive_index], game)
        )
        selector_preview = (
            None
            if entry.selector_preview_archive_index is None
            else _decode_bank(
                forms[entry.selector_preview_archive_index], game)
        )
        stat_values = {
            field.name: struct.unpack_from(
                "<" + field.format,
                entry.stats_record,
                field.offset,
            )[0]
            for field in profile.fields
            if field.name != "vehicle_type"
        }
        if game == "V8_2":
            stat_values["rear_suspension_damping"] = (
                entry.rear_suspension_damping
            )
        vehicle = project.VehicleProject(
            schema_version=project.SCHEMA_VERSION,
            stable_id=entry.stable_id,
            display_name=entry.display_name,
            game=game,
            groups=body.groups,
            slots=body.slots,
            collisions=body.collisions,
            textures=body.textures,
            animations=body.animations,
            stats=stat_values,
            body_kind=entry.body_kind,
            transformation_bank=transformation,
            selector_preview_bank=selector_preview,
            selector_preview_body_kind=entry.selector_preview_body_kind,
            transform_modes=entry.transform_modes,
            powerups=entry.powerups,
        )
        vehicle.validate()
        vehicles.append(vehicle)
    if referenced != set(range(len(forms))):
        raise ValueError(
            "vehicle archive contains unreferenced donor/support banks")
    return tuple(vehicles)


def compile_package(
    vehicles: Iterable[project.VehicleProject],
) -> CompiledVehiclePackage:
    projects = tuple(vehicles)
    if not projects:
        raise ValueError("a vehicle package requires at least one project")
    archive = compiler.compile_archive(projects)
    forms = tuple(iff.parse(archive).forms(b"XOBF"))
    for index, form in enumerate(forms):
        source_bytes = sum(
            len(child.payload)
            for child in form.children
            if child.tag in {b"BIN ", b"ANM "}
        )
        if source_bytes > project.MAX_NATIVE_BANK_SOURCE_BYTES:
            raise ValueError(
                f"native bank {index} uses {source_bytes} source bytes; "
                f"limit is {project.MAX_NATIVE_BANK_SOURCE_BYTES}"
            )
    return CompiledVehiclePackage(
        game=projects[0].game,
        archive=archive,
        registry=compile_registry(projects),
    )
