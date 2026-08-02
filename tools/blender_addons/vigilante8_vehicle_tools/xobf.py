"""Native Vigilante 8 XOBF/BIN model structures.

The current writer is deliberately surgical: parsed retail data stays in its
original byte layout, and edits patch validated fields in place. This already
supports lossless transforms and mesh-coordinate edits while the topology
repacker is developed and validated.
"""

from __future__ import annotations

from dataclasses import dataclass
import struct
from typing import Iterable, Iterator, Sequence

from .binary import (
    FormatError,
    i16le,
    i32le,
    put_i16le,
    put_i32le,
    put_u8,
    put_u16le,
    put_u32le,
    require_range,
    u8,
    u16le,
    u32le,
)


PACKET_SIZES_V8 = (
    12, 28, 20, 28, 12, 20, 12, 20, 16, 24, 12, 24, 20, 20, 0, 20
)
PACKET_SIZES_V82 = (
    12, 32, 20, 32, 12, 24, 12, 24, 16, 28, 12, 28, 24, 24, 0, 24
)
PACKET_DIALECTS = {
    "V8": PACKET_SIZES_V8,
    "V8_2": PACKET_SIZES_V82,
}
# The BIN packets all address three geometry vertices.  Kind 10 is a
# variable-length renderer control record rather than a triangle.  Kinds 13
# and 15 are genuine textured triangles; their high texture bits select native
# render modes and must not cause them to be dropped from editable geometry.
QUAD_PACKET_KINDS: frozenset[int] = frozenset()
NON_VERTEX_PACKET_KINDS = frozenset((10,))
TEXTURE_SLOT_OFFSETS_V8 = {
    1: 0x12,
    3: 0x12,
    5: 0x12,
    7: 0x12,
    9: 0x16,
    11: 0x16,
    13: 0x12,
    15: 0x12,
}
# V8:2 widened the per-vertex texture payloads of these packet families.  The
# third UV pair is still immediately followed by the texture index, so both
# move four bytes later than in V8.
TEXTURE_SLOT_OFFSETS_V82 = {
    kind: offset + 4 for kind, offset in TEXTURE_SLOT_OFFSETS_V8.items()
}


@dataclass(frozen=True)
class Vertex:
    x: int
    y: int
    z: int
    pad: int


@dataclass(frozen=True)
class PolygonPacket:
    offset: int
    raw: bytes
    kind: int
    vertex_indices: tuple[int, ...]
    color: tuple[int, int, int]
    texture_slot: int | None
    uv: tuple[tuple[int, int], ...]

    @property
    def is_quad(self) -> bool:
        return self.kind in QUAD_PACKET_KINDS


@dataclass(frozen=True)
class RenderGroup:
    index: int
    descriptor_offset: int
    vertex_count: int
    vertex_offset: int
    normal_count: int
    normal_offset: int
    polygon_count: int
    polygon_offset: int
    scale_shift: int
    texture_slot_count: int
    render_extent: int
    vertices: tuple[Vertex, ...]
    normals: tuple[Vertex, ...]
    packets: tuple[PolygonPacket, ...]

    @property
    def scale(self) -> float:
        return 1.0 / float(1 << self.scale_shift)


@dataclass(frozen=True)
class Slot:
    index: int
    offset: int
    render_key: int
    obstacle_index: int
    x: int
    y: int
    z: int
    rot_y: int
    rot_x: int
    rot_z: int
    flags: int
    next_sibling: int
    first_child: int
    group_mask: int = 0x07FF

    @property
    def render_group(self) -> int | None:
        raw = self.render_key & 0xFFFF
        # Classes 8 and 9 are effect/sound opcodes.  Their low bits are effect
        # parameters, not model indices, even when they happen to fall inside
        # the bank's model count.  Renderable descriptors use an eleven-bit
        # owned group index; V8:2 0x8002C344 masks key0 with 0x07ff before
        # resolving it, and 0x07ff is the no-model sentinel.
        if (
            raw >> 12 in {8, 9} or
            0x8000 <= raw <= 0x8005 or
            0x8010 <= raw <= 0x8016 or
            raw == 0x801F or
            0x8040 <= raw <= 0x8043 or
            0x8100 <= raw <= 0x8102
        ):
            return None
        group = raw & self.group_mask
        return None if group == self.group_mask else group

    @property
    def render_flags(self) -> int:
        return (
            0
            if self.render_group is None
            else (self.render_key & (0xFFFF ^ self.group_mask))
        )


@dataclass(frozen=True)
class TextureSlot:
    index: int
    offset: int
    depth: int
    width: int
    height: int
    palette: tuple[int, ...]
    indices: bytes
    direct_pixels_bgr555: tuple[int, ...]
    packed_pixels: bytes
    compressed: bool
    supported: bool
    palette_origin: tuple[int, int] = (0, 0)
    image_origin: tuple[int, int] = (0, 0)


@dataclass(frozen=True)
class CollisionAabb:
    flags: int
    minimum_x: int
    override_y: int
    minimum_z: int
    maximum_x: int
    maximum_y: int
    maximum_z: int


@dataclass(frozen=True)
class CollisionPlane:
    normal_x: int
    normal_y: int
    normal_z: int
    pad: int
    offset: int


@dataclass(frozen=True)
class CollisionConvex:
    planes: tuple[CollisionPlane, ...]


@dataclass(frozen=True)
class CollisionStream:
    index: int
    offset: int
    end_offset: int
    shapes: tuple[CollisionAabb | CollisionConvex, ...]


@dataclass(frozen=True)
class AnimationFrame:
    offset: int
    size: int
    frame_delta: int
    flags: int
    rotation_yxz: tuple[int, int, int] | None
    rotation_parameter: int
    translation_absolute: tuple[int, int, int] | None
    translation_delta: tuple[int, int, int] | None
    texture_bindings: tuple[tuple[int, int], ...]
    scale: tuple[int, int, int, int] | None
    mesh_data: bytes | None
    raw: bytes

    @property
    def is_jump(self) -> bool:
        return self.flags < 0


def decompress_v82_texture(data: bytes, output_size: int) -> bytes:
    """Decode V8:2's 2 KiB-window texture compression.

    This is a direct expression of SLUS_008.68 function 0x80021064: control
    bits are consumed MSB first; a set bit is a literal, while a clear bit is
    an absolute 11-bit window position plus a 5-bit length.
    """

    output = bytearray()
    window = bytearray(0x800)
    source = 0
    control = 0
    mask = 0
    while len(output) < output_size:
        if mask == 0:
            require_range(data, source, 1)
            control = data[source]
            source += 1
            mask = 0x80
        if control & mask:
            require_range(data, source, 1)
            value = data[source]
            source += 1
            output.append(value)
            window[(len(output) - 1) & 0x7FF] = value
        else:
            require_range(data, source, 2)
            token = u16le(data, source)
            source += 2
            window_position = token >> 5
            run_length = (token & 0x1F) + 2
            for _index in range(run_length):
                value = window[window_position & 0x7FF]
                window_position += 1
                output.append(value)
                window[(len(output) - 1) & 0x7FF] = value
                if len(output) == output_size:
                    break
        mask >>= 1
    return bytes(output)


def compress_v82_texture(data: bytes) -> bytes:
    """Encode the sequel's native 2 KiB-window texture stream.

    The encoder is deterministic and uses the longest available match at each
    position. Absolute window positions make this compatible with the retail
    decoder without introducing an external compression dependency.
    """

    output = bytearray()
    source = 0
    while source < len(data):
        control_offset = len(output)
        output.append(0)
        control = 0
        payload = bytearray()
        for bit in range(8):
            if source >= len(data):
                break
            best_position = 0
            best_length = 0
            maximum = min(33, len(data) - source)
            # Search prior output within the 2 KiB window.  Emit only
            # non-overlapping references.  Although the decoder copies
            # forward, retail texture streams do not require a self-extending
            # run, and constraining the encoder keeps every referenced byte
            # inside the already-produced native window.
            if maximum >= 2:
                needle = data[source : source + 2]
                window_start = max(0, source - 0x800)
                search_end = source
                attempts = 0
                position = data.rfind(needle, window_start, search_end)
                while position >= window_start and attempts < 64:
                    length = 2
                    available = source - position
                    while (
                        length < maximum
                        and length < available
                        and data[position + length] == data[source + length]
                    ):
                        length += 1
                    if length > best_length:
                        best_position = position & 0x7FF
                        best_length = length
                        if length == maximum:
                            break
                    search_end = position
                    position = data.rfind(needle, window_start, search_end)
                    attempts += 1
            if best_length >= 2:
                token = (best_position << 5) | (best_length - 2)
                payload += struct.pack("<H", token)
                source += best_length
            else:
                control |= 0x80 >> bit
                value = data[source]
                payload.append(value)
                source += 1
        output[control_offset] = control
        output += payload
    return bytes(output)


def _compressed_input_size(data: bytes, output_size: int) -> int:
    produced = 0
    source = 0
    mask = 0
    control = 0
    while produced < output_size:
        if mask == 0:
            require_range(data, source, 1)
            control = data[source]
            source += 1
            mask = 0x80
        if control & mask:
            require_range(data, source, 1)
            source += 1
            produced += 1
        else:
            require_range(data, source, 2)
            token = u16le(data, source)
            source += 2
            produced += min((token & 0x1F) + 2, output_size - produced)
        mask >>= 1
    return source


class Model:
    """A parsed BIN payload with validated in-place mutation methods."""

    HEADER_SIZE = 0x1C
    SLOT_SIZE = 0x1C

    def __init__(self, data: bytes, dialect: str | None = None):
        self._data = bytearray(data)
        if len(data) < self.HEADER_SIZE:
            raise FormatError("XOBF BIN is smaller than its 0x1c-byte header")
        self.group_count = u32le(data, 0x00)
        self.group_table_offset = u32le(data, 0x04)
        self.obstacle_count = u32le(data, 0x08)
        self.obstacle_table_offset = u32le(data, 0x0C)
        self.texture_count = u32le(data, 0x10)
        self.texture_table_offset = u32le(data, 0x14)
        self.slot_count = u32le(data, 0x18)
        self._validate_header()
        if dialect is None:
            self.dialect = self._detect_packet_dialect()
        else:
            normalized = dialect.upper().replace(":", "_")
            if normalized not in PACKET_DIALECTS:
                raise ValueError(f"unknown packet dialect {dialect!r}")
            self.dialect = normalized
        self.packet_sizes = PACKET_DIALECTS[self.dialect]

    @property
    def data(self) -> bytes:
        return bytes(self._data)

    def _validate_header(self) -> None:
        for label, count, table in (
            ("render group", self.group_count, self.group_table_offset),
            ("obstacle", self.obstacle_count, self.obstacle_table_offset),
            ("texture", self.texture_count, self.texture_table_offset),
        ):
            if count > 0x10000:
                raise FormatError(f"implausible {label} count {count}")
            require_range(self._data, table, count * 4)
        require_range(self._data, self.HEADER_SIZE, self.slot_count * self.SLOT_SIZE)

    def _relative_table_target(self, table: int, index: int, count: int) -> int:
        if index < 0 or index >= count:
            raise IndexError(index)
        relative = u32le(self._data, table + index * 4)
        target = table + relative
        require_range(self._data, target, 1)
        return target

    def _packet_stream_end(
        self,
        offset: int,
        count: int,
        sizes: Sequence[int],
        variable_entry_size: int,
    ) -> int | None:
        cursor = offset
        for _packet_index in range(count):
            if cursor + 4 > len(self._data):
                return None
            kind = u8(self._data, cursor + 3) & 0x0F
            size = sizes[kind]
            if kind == 10:
                if cursor + 12 > len(self._data):
                    return None
                size += (
                    u16le(self._data, cursor + 0x0A) * variable_entry_size
                )
            if size == 0 or cursor + size > len(self._data):
                return None
            cursor += size
        return cursor

    def _detect_packet_dialect(self) -> str:
        """Identify V8 versus V8:2 from recorded polygon stream boundaries.

        Both games use the same group descriptor.  Vertex and normal offsets
        therefore provide an independent end marker for the packet stream.
        V8:2 enlarged six packet kinds by four bytes; the dialect whose parsed
        stream reaches that marker across the most groups is authoritative.
        """

        scores = {name: 0 for name in PACKET_DIALECTS}
        for index in range(self.group_count):
            descriptor = self._relative_table_target(
                self.group_table_offset, index, self.group_count
            )
            require_range(self._data, descriptor, 0x19)
            polygon_count = u16le(self._data, descriptor + 0x10)
            polygon_relative = u32le(self._data, descriptor + 0x14)
            polygon_offset = descriptor + polygon_relative
            later_sections = [
                descriptor + relative
                for relative in (
                    u32le(self._data, descriptor + 0x04),
                    u32le(self._data, descriptor + 0x0C),
                )
                if relative > polygon_relative
            ]
            if not later_sections:
                continue
            expected_end = min(later_sections)
            for name, sizes in PACKET_DIALECTS.items():
                if (
                    self._packet_stream_end(
                        polygon_offset,
                        polygon_count,
                        sizes,
                        8 if name == "V8_2" else 4,
                    )
                    == expected_end
                ):
                    scores[name] += 1
        return max(PACKET_DIALECTS, key=lambda name: scores[name])

    def slots(self) -> tuple[Slot, ...]:
        result = []
        for index in range(self.slot_count):
            offset = self.HEADER_SIZE + index * self.SLOT_SIZE
            result.append(
                Slot(
                    index=index,
                    offset=offset,
                    render_key=i16le(self._data, offset + 0x00),
                    obstacle_index=i16le(self._data, offset + 0x02),
                    x=i32le(self._data, offset + 0x04),
                    y=i32le(self._data, offset + 0x08),
                    z=i32le(self._data, offset + 0x0C),
                    rot_y=i16le(self._data, offset + 0x10),
                    rot_x=i16le(self._data, offset + 0x12),
                    rot_z=i16le(self._data, offset + 0x14),
                    flags=i16le(self._data, offset + 0x16),
                    next_sibling=u16le(self._data, offset + 0x18),
                    first_child=u16le(self._data, offset + 0x1A),
                    group_mask=(
                        0x00FF if self.dialect == "V8" else 0x07FF
                    ),
                )
            )
        return tuple(result)

    def patch_slot(
        self,
        index: int,
        *,
        render_key: int | None = None,
        obstacle_index: int | None = None,
        position: Sequence[int] | None = None,
        rotation_yxz: Sequence[int] | None = None,
        flags: int | None = None,
        next_sibling: int | None = None,
        first_child: int | None = None,
    ) -> None:
        if index < 0 or index >= self.slot_count:
            raise IndexError(index)
        offset = self.HEADER_SIZE + index * self.SLOT_SIZE
        if render_key is not None:
            put_i16le(self._data, offset + 0x00, render_key)
        if obstacle_index is not None:
            put_i16le(self._data, offset + 0x02, obstacle_index)
        if position is not None:
            if len(position) != 3:
                raise ValueError("position must have three components")
            for component, field in zip(position, (0x04, 0x08, 0x0C)):
                put_i32le(self._data, offset + field, int(component))
        if rotation_yxz is not None:
            if len(rotation_yxz) != 3:
                raise ValueError("rotation_yxz must have three components")
            for component, field in zip(rotation_yxz, (0x10, 0x12, 0x14)):
                put_i16le(self._data, offset + field, int(component))
        if flags is not None:
            put_i16le(self._data, offset + 0x16, flags)
        if next_sibling is not None:
            put_u16le(self._data, offset + 0x18, next_sibling)
        if first_child is not None:
            put_u16le(self._data, offset + 0x1A, first_child)

    def _parse_vertices(self, offset: int, count: int) -> tuple[Vertex, ...]:
        require_range(self._data, offset, count * 8)
        return tuple(
            Vertex(
                i16le(self._data, offset + index * 8 + 0),
                i16le(self._data, offset + index * 8 + 2),
                i16le(self._data, offset + index * 8 + 4),
                i16le(self._data, offset + index * 8 + 6),
            )
            for index in range(count)
        )

    def _parse_packets(
        self, offset: int, count: int, vertex_count: int
    ) -> tuple[PolygonPacket, ...]:
        packets = []
        cursor = offset
        for packet_index in range(count):
            require_range(self._data, cursor, 4)
            kind = u8(self._data, cursor + 3) & 0x0F
            size = self.packet_sizes[kind]
            if kind == 10:
                require_range(self._data, cursor, 12)
                size += u16le(self._data, cursor + 0x0A) * (
                    8 if self.dialect == "V8_2" else 4
                )
            if size == 0:
                raise FormatError(
                    f"render packet {packet_index} at 0x{cursor:X} has "
                    f"unsupported zero-sized kind 0x{kind:X}"
                )
            require_range(self._data, cursor, size)
            vertex_total = 3
            indices = tuple(u16le(self._data, cursor + 4 + i * 2) for i in range(vertex_total))
            if (
                kind not in NON_VERTEX_PACKET_KINDS
                and any(index >= vertex_count for index in indices)
            ):
                raise FormatError(
                    f"packet {packet_index} at 0x{cursor:X} references vertex "
                    f"{max(indices)} but group has {vertex_count}"
                )
            texture_offsets = (
                TEXTURE_SLOT_OFFSETS_V82
                if self.dialect == "V8_2"
                else TEXTURE_SLOT_OFFSETS_V8
            )
            texture_offset = texture_offsets.get(kind)
            texture_slot = (
                u16le(self._data, cursor + texture_offset)
                if texture_offset is not None
                else None
            )
            if self.dialect == "V8_2":
                uv_offsets = (
                    (0x10, 0x14, 0x18)
                    if kind in (9, 11)
                    else (0x0C, 0x10, 0x14)
                )
            else:
                uv_offsets = (
                    (0x10, 0x12, 0x14)
                    if kind in (9, 11)
                    else (0x0C, 0x0E, 0x10)
                )
            uv = tuple(
                (u8(self._data, cursor + uv_offset), u8(self._data, cursor + uv_offset + 1))
                for uv_offset in uv_offsets
                if uv_offset + 2 <= size
            )
            packets.append(
                PolygonPacket(
                    offset=cursor,
                    raw=bytes(self._data[cursor : cursor + size]),
                    kind=kind,
                    vertex_indices=indices,
                    color=(
                        u8(self._data, cursor),
                        u8(self._data, cursor + 1),
                        u8(self._data, cursor + 2),
                    ),
                    texture_slot=texture_slot,
                    uv=uv,
                )
            )
            cursor += size
        return tuple(packets)

    def group(self, index: int) -> RenderGroup:
        descriptor = self._relative_table_target(
            self.group_table_offset, index, self.group_count
        )
        require_range(self._data, descriptor, 0x1C)
        vertex_count = u32le(self._data, descriptor + 0x00)
        vertex_offset = descriptor + u32le(self._data, descriptor + 0x04)
        normal_count = u32le(self._data, descriptor + 0x08)
        normal_offset = descriptor + u32le(self._data, descriptor + 0x0C)
        polygon_count = u32le(self._data, descriptor + 0x10)
        polygon_offset = descriptor + u32le(self._data, descriptor + 0x14)
        scale_shift = u8(self._data, descriptor + 0x18)
        texture_slot_count = u8(self._data, descriptor + 0x19)
        render_extent = u16le(self._data, descriptor + 0x1A)
        if scale_shift > 15:
            raise FormatError(
                f"group {index} has invalid render scale shift {scale_shift}"
            )
        return RenderGroup(
            index=index,
            descriptor_offset=descriptor,
            vertex_count=vertex_count,
            vertex_offset=vertex_offset,
            normal_count=normal_count,
            normal_offset=normal_offset,
            polygon_count=polygon_count,
            polygon_offset=polygon_offset,
            scale_shift=scale_shift,
            texture_slot_count=texture_slot_count,
            render_extent=render_extent,
            vertices=self._parse_vertices(vertex_offset, vertex_count),
            normals=self._parse_vertices(normal_offset, normal_count),
            packets=self._parse_packets(polygon_offset, polygon_count, vertex_count),
        )

    def groups(self) -> Iterator[RenderGroup]:
        for index in range(self.group_count):
            yield self.group(index)

    def patch_group_vertices(
        self, index: int, vertices: Sequence[Sequence[int]]
    ) -> None:
        group = self.group(index)
        if len(vertices) != group.vertex_count:
            raise ValueError(
                f"group {index} requires {group.vertex_count} vertices; "
                f"received {len(vertices)}"
            )
        for vertex_index, components in enumerate(vertices):
            if len(components) not in (3, 4):
                raise ValueError("each vertex needs x, y, z and optional pad")
            offset = group.vertex_offset + vertex_index * 8
            for component_index, component in enumerate(components[:3]):
                put_i16le(self._data, offset + component_index * 2, int(component))
            if len(components) == 4:
                put_i16le(self._data, offset + 6, int(components[3]))

    def patch_group_packets(
        self,
        index: int,
        packet_indices: Sequence[int],
        *,
        colors: Sequence[Sequence[int]] | None = None,
        texture_slots: Sequence[int | None] | None = None,
        uvs: Sequence[Sequence[Sequence[int]] | None] | None = None,
    ) -> None:
        """Patch editable fields while retaining each retail packet layout."""

        group = self.group(index)
        count = len(packet_indices)
        for label, values in (
            ("colors", colors),
            ("texture_slots", texture_slots),
            ("uvs", uvs),
        ):
            if values is not None and len(values) != count:
                raise ValueError(f"{label} must contain one item per packet index")
        texture_offsets = (
            TEXTURE_SLOT_OFFSETS_V82
            if self.dialect == "V8_2"
            else TEXTURE_SLOT_OFFSETS_V8
        )
        for item_index, packet_index in enumerate(packet_indices):
            if packet_index < 0 or packet_index >= len(group.packets):
                raise IndexError(packet_index)
            packet = group.packets[packet_index]
            if colors is not None:
                color = tuple(int(value) for value in colors[item_index])
                if len(color) != 3 or any(value < 0 or value > 255 for value in color):
                    raise ValueError("packet colors must be RGB values in 0..255")
                self._data[packet.offset : packet.offset + 3] = bytes(color)
            if texture_slots is not None:
                texture_slot = texture_slots[item_index]
                texture_offset = texture_offsets.get(packet.kind)
                if texture_slot is not None:
                    if texture_offset is None:
                        raise ValueError(
                            f"packet kind {packet.kind} cannot reference a texture"
                        )
                    if texture_slot < 0 or texture_slot > 0xFFFF:
                        raise ValueError("texture slots must fit unsigned 16-bit")
                    put_u16le(
                        self._data,
                        packet.offset + texture_offset,
                        int(texture_slot),
                    )
            if uvs is not None and uvs[item_index] is not None:
                face_uv = uvs[item_index]
                if len(packet.uv) != 3 or len(face_uv) != 3:
                    raise ValueError(
                        f"packet kind {packet.kind} does not expose three UV pairs"
                    )
                uv_offsets = (
                    (0x10, 0x14, 0x18)
                    if self.dialect == "V8_2" and packet.kind in (9, 11)
                    else (0x0C, 0x10, 0x14)
                    if self.dialect == "V8_2"
                    else (0x10, 0x12, 0x14)
                    if packet.kind in (9, 11)
                    else (0x0C, 0x0E, 0x10)
                )
                for uv, uv_offset in zip(face_uv, uv_offsets):
                    pair = tuple(int(value) for value in uv)
                    if len(pair) != 2 or any(
                        value < 0 or value > 255 for value in pair
                    ):
                        raise ValueError("UV components must fit unsigned 8-bit")
                    self._data[
                        packet.offset + uv_offset : packet.offset + uv_offset + 2
                    ] = bytes(pair)

    def patch_group_scale(self, index: int, scale_shift: int) -> None:
        if scale_shift < 0 or scale_shift > 15:
            raise ValueError("scale_shift must be in 0..15")
        group = self.group(index)
        put_u8(self._data, group.descriptor_offset + 0x18, scale_shift)

    def replace_group_geometry(
        self,
        index: int,
        vertices: Sequence[Sequence[int]],
        faces: Sequence[Sequence[int]],
        *,
        colors: Sequence[Sequence[int]] | None = None,
        texture_slots: Sequence[int | None] | None = None,
        uvs: Sequence[Sequence[Sequence[int]]] | None = None,
        scale_shift: int | None = None,
    ) -> None:
        """Repack one render group as native triangle packets.

        The surrounding BIN stays source-backed: slots, hierarchy, obstacle
        data, textures, other groups, and unknown bytes are retained.  All
        relative top-level pointers are rebased after the resized group.

        Faces with a texture slot are written as the retail flat-lit textured
        kind-5 packet, including the dialect-specific V8:2 UV layout.  Other
        faces use kind 0.  Existing texture records are referenced rather than
        converted to an interchange format.
        """

        original = self.group(index)
        if not vertices:
            raise ValueError("a render group requires at least one vertex")
        if len(vertices) > 0xFFFF:
            raise ValueError("native packets support at most 65535 vertices")
        if len(faces) > 0xFFFF:
            raise ValueError("a render group supports at most 65535 faces")
        if scale_shift is None:
            scale_shift = original.scale_shift
        if scale_shift < 0 or scale_shift > 15:
            raise ValueError("scale_shift must be in 0..15")
        if colors is not None and len(colors) != len(faces):
            raise ValueError("colors must contain one RGB triplet per face")
        if texture_slots is not None and len(texture_slots) != len(faces):
            raise ValueError("texture_slots must contain one item per face")
        if uvs is not None and len(uvs) != len(faces):
            raise ValueError("uvs must contain one triangle per face")

        encoded_vertices = bytearray()
        for vertex in vertices:
            if len(vertex) not in (3, 4):
                raise ValueError("each vertex needs x, y, z and optional pad")
            components = tuple(int(value) for value in vertex)
            if any(value < -32768 or value > 32767 for value in components):
                raise ValueError("native vertex components must fit signed 16-bit")
            encoded_vertices += struct.pack(
                "<hhhh",
                components[0],
                components[1],
                components[2],
                components[3] if len(components) == 4 else 0,
            )

        encoded_packets = bytearray()
        encoded_normals = bytearray()
        textured_faces = 0
        for face_index, face in enumerate(faces):
            if len(face) != 3:
                raise ValueError("native replacement faces must be triangles")
            indices = tuple(int(value) for value in face)
            if any(value < 0 or value >= len(vertices) for value in indices):
                raise ValueError(f"face {face_index} has an invalid vertex index")
            color = (127, 127, 127) if colors is None else tuple(
                int(value) for value in colors[face_index]
            )
            if len(color) != 3 or any(value < 0 or value > 255 for value in color):
                raise ValueError("face colors must be three values in 0..255")
            texture_slot = (
                None if texture_slots is None else texture_slots[face_index]
            )
            if texture_slot is None:
                encoded_packets += struct.pack(
                    "<BBBBHHHH",
                    color[0],
                    color[1],
                    color[2],
                    0,
                    indices[0],
                    indices[1],
                    indices[2],
                    0,
                )
                continue

            texture_slot = int(texture_slot)
            if texture_slot < 0 or texture_slot > 0xFFFF:
                raise ValueError("native texture slots must fit unsigned 16-bit")
            if uvs is None:
                face_uv = ((0, 0), (0, 0), (0, 0))
            else:
                face_uv = uvs[face_index]
            if len(face_uv) != 3:
                raise ValueError("each textured face requires three UV pairs")
            uv_values = []
            for uv in face_uv:
                if len(uv) != 2:
                    raise ValueError("each UV requires two components")
                pair = tuple(int(value) for value in uv)
                if any(value < 0 or value > 255 for value in pair):
                    raise ValueError("native UV components must fit unsigned 8-bit")
                uv_values.append(pair)

            # The source kind-5 packet carries one lighting normal index.
            # A normalized fixed-point face normal is generated for guest
            # topology, while original topology retains its source packets.
            a = tuple(int(value) for value in vertices[indices[0]][:3])
            b = tuple(int(value) for value in vertices[indices[1]][:3])
            c = tuple(int(value) for value in vertices[indices[2]][:3])
            ab = tuple(b[axis] - a[axis] for axis in range(3))
            ac = tuple(c[axis] - a[axis] for axis in range(3))
            normal = (
                ab[1] * ac[2] - ab[2] * ac[1],
                ab[2] * ac[0] - ab[0] * ac[2],
                ab[0] * ac[1] - ab[1] * ac[0],
            )
            magnitude = sum(component * component for component in normal) ** 0.5
            if magnitude == 0:
                fixed_normal = (0, -4096, 0)
            else:
                fixed_normal = tuple(
                    max(-32768, min(32767, round(component * 4096.0 / magnitude)))
                    for component in normal
                )
            normal_index = textured_faces
            encoded_normals += struct.pack(
                "<hhhh", fixed_normal[0], fixed_normal[1], fixed_normal[2], 0
            )
            textured_faces += 1

            header = struct.pack(
                "<BBBBHHHH",
                color[0],
                color[1],
                color[2],
                5,
                indices[0],
                indices[1],
                indices[2],
                normal_index,
            )
            if self.dialect == "V8_2":
                encoded_packets += (
                    header
                    + bytes(uv_values[0])
                    + b"\0\0"
                    + bytes(uv_values[1])
                    + b"\0\0"
                    + bytes(uv_values[2])
                    + struct.pack("<H", texture_slot)
                )
            else:
                encoded_packets += (
                    header
                    + bytes(uv_values[0])
                    + bytes(uv_values[1])
                    + bytes(uv_values[2])
                    + struct.pack("<H", texture_slot)
                )

        descriptor_size = 0x1C
        polygon_relative = descriptor_size
        vertex_relative = polygon_relative + len(encoded_packets)
        normal_relative = vertex_relative + len(encoded_vertices)
        replacement = bytearray(
            struct.pack(
                "<IIIIIIBBH",
                len(vertices),
                vertex_relative,
                textured_faces,
                normal_relative,
                len(faces),
                polygon_relative,
                scale_shift,
                original.texture_slot_count,
                original.render_extent,
            )
        )
        replacement += encoded_packets
        replacement += encoded_vertices
        replacement += encoded_normals

        packet_end = self._packet_stream_end(
            original.polygon_offset,
            original.polygon_count,
            self.packet_sizes,
            8 if self.dialect == "V8_2" else 4,
        )
        if packet_end is None:
            raise FormatError(f"group {index} packet stream cannot be bounded")
        span_start = original.descriptor_offset
        span_end = max(
            packet_end,
            original.vertex_offset + original.vertex_count * 8,
            original.normal_offset + original.normal_count * 8,
            span_start + descriptor_size,
        )

        table_specs = (
            (0x04, self.group_table_offset, self.group_count),
            (0x0C, self.obstacle_table_offset, self.obstacle_count),
            (0x14, self.texture_table_offset, self.texture_count),
        )
        captured = []
        for header_field, table, count in table_specs:
            targets = [
                table + u32le(self._data, table + entry * 4)
                for entry in range(count)
            ]
            captured.append((header_field, table, targets))

        other_targets = [
            target
            for _header, _table, targets in captured
            for target in targets
            if target != span_start
        ]
        if any(span_start <= target < span_end for target in other_targets):
            raise FormatError(
                f"group {index} overlaps another top-level BIN object"
            )

        delta = len(replacement) - (span_end - span_start)

        def translated(offset: int) -> int:
            if offset < span_start:
                return offset
            if offset >= span_end:
                return offset + delta
            if offset == span_start:
                return span_start
            raise FormatError("cannot translate a pointer into replaced group data")

        self._data[span_start:span_end] = replacement
        for header_field, old_table, old_targets in captured:
            new_table = translated(old_table)
            put_u32le(self._data, header_field, new_table)
            for entry, old_target in enumerate(old_targets):
                new_target = (
                    span_start
                    if header_field == 0x04 and entry == index
                    else translated(old_target)
                )
                put_u32le(
                    self._data,
                    new_table + entry * 4,
                    new_target - new_table,
                )

        self.group_table_offset = u32le(self._data, 0x04)
        self.obstacle_table_offset = u32le(self._data, 0x0C)
        self.texture_table_offset = u32le(self._data, 0x14)

    def texture(self, index: int) -> TextureSlot:
        offset = self._relative_table_target(
            self.texture_table_offset, index, self.texture_count
        )
        require_range(self._data, offset, 0x20)
        flags = u32le(self._data, offset + 0x04)
        image_offset = offset + u32le(self._data, offset + 0x08)
        depth = flags & 3
        compressed = bool(flags & 0x10)
        if depth == 2:
            # V8:2 depth-2 records are ordinary 16-bit direct-color images,
            # not opaque renderer controls.  The retail loader
            # (SLUS_008.68 0x80020FF8/0x800212E4) reads their image RECT
            # directly from record+0x0c, skips CLUT allocation because flag
            # 0x08 is clear, and consumes compressed pixels from record+0x14.
            image_rect_offset = offset + 0x0C
            require_range(self._data, image_rect_offset, 8)
            width = i16le(self._data, image_rect_offset + 0x04)
            height = i16le(self._data, image_rect_offset + 0x06)
            if width <= 0 or height <= 0:
                raise FormatError(
                    f"direct-color texture {index} has invalid dimensions "
                    f"{width}x{height}"
                )
            pixel_offset = offset + 0x14
            pixel_bytes = width * height * 2
            if compressed:
                packed = decompress_v82_texture(
                    bytes(self._data[pixel_offset:]), pixel_bytes
                )
            else:
                require_range(self._data, pixel_offset, pixel_bytes)
                packed = bytes(
                    self._data[pixel_offset : pixel_offset + pixel_bytes]
                )
            return TextureSlot(
                index=index,
                offset=offset,
                depth=depth,
                width=width,
                height=height,
                palette=(),
                indices=b"",
                direct_pixels_bgr555=tuple(
                    u16le(packed, pixel * 2)
                    for pixel in range(width * height)
                ),
                packed_pixels=packed,
                compressed=compressed,
                supported=True,
                image_origin=(
                    i16le(self._data, image_rect_offset + 0x00),
                    i16le(self._data, image_rect_offset + 0x02),
                ),
            )
        if depth not in (0, 1):
            return TextureSlot(
                index=index,
                offset=offset,
                depth=depth,
                width=0,
                height=0,
                palette=(),
                indices=b"",
                direct_pixels_bgr555=(),
                packed_pixels=b"",
                compressed=compressed,
                supported=False,
            )
        require_range(self._data, image_offset, 0x14)
        words = i16le(self._data, image_offset + 0x10)
        height = i16le(self._data, image_offset + 0x12)
        width = words * (4 if depth == 0 else 2)
        if width <= 0 or height <= 0:
            raise FormatError(f"texture {index} has invalid dimensions {width}x{height}")
        # The CLUT rectangle is embedded at source+0x0c and may be smaller
        # than the nominal 16/256-color capacity (several V8:2 effects use a
        # four-entry CLUT).  The retail loader uploads exactly this rectangle.
        palette_width = u16le(self._data, offset + 0x10)
        palette_height = u16le(self._data, offset + 0x12)
        palette_count = palette_width * palette_height
        if palette_count <= 0 or palette_count > 256:
            raise FormatError(
                f"texture {index} has invalid CLUT size "
                f"{palette_width}x{palette_height}"
            )
        palette_offset = offset + 0x14
        require_range(self._data, palette_offset, palette_count * 2)
        palette = tuple(
            u16le(self._data, palette_offset + color * 2)
            for color in range(palette_count)
        )
        pixel_offset = image_offset + 0x14
        pixel_bytes = (width * height + 1) // 2 if depth == 0 else width * height
        if compressed:
            packed = decompress_v82_texture(
                bytes(self._data[pixel_offset:]), pixel_bytes
            )
        else:
            require_range(self._data, pixel_offset, pixel_bytes)
            packed = bytes(self._data[pixel_offset : pixel_offset + pixel_bytes])
        if depth == 0:
            indices = bytes(
                (packed[pixel >> 1] >> (4 if pixel & 1 else 0)) & 0x0F
                for pixel in range(width * height)
            )
        else:
            indices = packed
        return TextureSlot(
            index=index,
            offset=offset,
            depth=depth,
            width=width,
            height=height,
            palette=palette,
            indices=indices,
            direct_pixels_bgr555=(),
            packed_pixels=packed,
            compressed=compressed,
            supported=True,
            palette_origin=(
                i16le(self._data, offset + 0x0C),
                i16le(self._data, offset + 0x0E),
            ),
            image_origin=(
                i16le(self._data, image_offset + 0x0C),
                i16le(self._data, image_offset + 0x0E),
            ),
        )

    def textures(self) -> Iterator[TextureSlot]:
        for index in range(self.texture_count):
            yield self.texture(index)

    def collision(self, index: int) -> CollisionStream:
        offset = self._relative_table_target(
            self.obstacle_table_offset, index, self.obstacle_count
        )
        if index + 1 < self.obstacle_count:
            end = self._relative_table_target(
                self.obstacle_table_offset, index + 1, self.obstacle_count
            )
        else:
            later = [
                candidate
                for candidate in (
                    self.texture_table_offset,
                    self.group_table_offset,
                    len(self._data),
                )
                if candidate > offset
            ]
            end = min(later) if later else len(self._data)
        shapes = []
        cursor = offset
        terminated = False
        while cursor + 2 <= end:
            kind = i16le(self._data, cursor)
            if kind == 0:
                terminated = True
                cursor += 2
                break
            if kind == 1:
                require_range(self._data, cursor, 0x1C)
                if cursor + 0x1C > end:
                    raise FormatError(f"collision stream {index} has a truncated AABB")
                shapes.append(
                    CollisionAabb(
                        flags=i16le(self._data, cursor + 0x02),
                        minimum_x=i32le(self._data, cursor + 0x04),
                        override_y=i32le(self._data, cursor + 0x08),
                        minimum_z=i32le(self._data, cursor + 0x0C),
                        maximum_x=i32le(self._data, cursor + 0x10),
                        maximum_y=i32le(self._data, cursor + 0x14),
                        maximum_z=i32le(self._data, cursor + 0x18),
                    )
                )
                cursor += 0x1C
                continue
            if kind == 2:
                require_range(self._data, cursor, 4)
                plane_count = u16le(self._data, cursor + 2)
                require_range(self._data, cursor + 4, plane_count * 12)
                if cursor + 4 + plane_count * 12 > end:
                    raise FormatError(
                        f"collision stream {index} has truncated convex planes"
                    )
                shapes.append(
                    CollisionConvex(
                        planes=tuple(
                            CollisionPlane(
                                normal_x=i16le(self._data, cursor + 4 + plane * 12),
                                normal_y=i16le(
                                    self._data, cursor + 6 + plane * 12
                                ),
                                normal_z=i16le(
                                    self._data, cursor + 8 + plane * 12
                                ),
                                pad=i16le(self._data, cursor + 10 + plane * 12),
                                offset=i32le(
                                    self._data, cursor + 12 + plane * 12
                                ),
                            )
                            for plane in range(plane_count)
                        )
                    )
                )
                cursor += 4 + plane_count * 12
                continue
            raise FormatError(
                f"collision stream {index} has unsupported kind {kind}"
            )
        if not terminated:
            raise FormatError(f"collision stream {index} has no terminator")
        return CollisionStream(index, offset, end, tuple(shapes))

    def collisions(self) -> Iterator[CollisionStream]:
        for index in range(self.obstacle_count):
            yield self.collision(index)

    def replace_texture(
        self,
        index: int,
        palette: Sequence[int],
        indices: bytes,
    ) -> None:
        """Replace a paletted texture while preserving its native record."""

        texture = self.texture(index)
        if not texture.supported:
            raise ValueError("unsupported native texture cannot be replaced")
        if texture.depth == 2:
            raise ValueError(
                "direct-color textures require 16-bit BGR555 pixel editing"
            )
        if len(palette) != len(texture.palette):
            raise ValueError(
                f"texture {index} requires exactly {len(texture.palette)} palette entries"
            )
        if len(indices) != texture.width * texture.height:
            raise ValueError(
                f"texture {index} requires {texture.width * texture.height} pixels"
            )
        if any(color < 0 or color > 0xFFFF for color in palette):
            raise ValueError("PSX palette entries must fit unsigned 16-bit")
        limit = 16 if texture.depth == 0 else 256
        if any(pixel >= len(palette) or pixel >= limit for pixel in indices):
            raise ValueError("texture pixel index exceeds its native palette")

        for color_index, color in enumerate(palette):
            put_u16le(
                self._data,
                texture.offset + 0x14 + color_index * 2,
                int(color),
            )
        if texture.depth == 0:
            packed = bytearray((len(indices) + 1) // 2)
            for pixel, value in enumerate(indices):
                packed[pixel >> 1] |= value << (4 if pixel & 1 else 0)
            packed_pixels = bytes(packed)
        else:
            packed_pixels = bytes(indices)

        image_offset = texture.offset + u32le(self._data, texture.offset + 0x08)
        pixel_offset = image_offset + 0x14
        if texture.compressed:
            consumed = _compressed_input_size(
                bytes(self._data[pixel_offset:]), len(packed_pixels)
            )
            encoded = compress_v82_texture(packed_pixels)
        else:
            consumed = len(packed_pixels)
            encoded = packed_pixels
        if self.dialect == "V8_2":
            old_declared_size = u32le(self._data, image_offset + 0x08)
            old_end = (image_offset + old_declared_size + 11) & ~3
            if old_end < pixel_offset + consumed:
                raise FormatError(
                    f"texture {index} has an invalid V8:2 image-block size"
                )
            new_declared_size = len(encoded) + 13
            new_end = (image_offset + new_declared_size + 11) & ~3
            replacement = encoded + b"\0" * (
                new_end - pixel_offset - len(encoded)
            )
        else:
            old_end = pixel_offset + consumed
            replacement = encoded

        table_specs = (
            (0x04, self.group_table_offset, self.group_count),
            (0x0C, self.obstacle_table_offset, self.obstacle_count),
            (0x14, self.texture_table_offset, self.texture_count),
        )
        captured = []
        for header_field, table, count in table_specs:
            targets = [
                table + u32le(self._data, table + entry * 4)
                for entry in range(count)
            ]
            captured.append((header_field, table, targets))
        if any(
            pixel_offset <= target < old_end
            for _header, _table, targets in captured
            for target in targets
        ):
            raise FormatError(f"texture {index} pixels overlap a top-level object")

        delta = len(replacement) - (old_end - pixel_offset)
        self._data[pixel_offset:old_end] = replacement
        if self.dialect == "V8_2":
            # V8:2 advances with align4(image + image[8] + 11).  Keep the
            # native encoded-size-plus-13 field synchronized when a
            # compressed stream changes length.
            put_u32le(self._data, image_offset + 0x08, new_declared_size)

        def translated(offset: int) -> int:
            return offset + delta if offset >= old_end else offset

        for header_field, old_table, old_targets in captured:
            new_table = translated(old_table)
            put_u32le(self._data, header_field, new_table)
            for entry, old_target in enumerate(old_targets):
                new_target = translated(old_target)
                put_u32le(
                    self._data,
                    new_table + entry * 4,
                    new_target - new_table,
                )
        self.group_table_offset = u32le(self._data, 0x04)
        self.obstacle_table_offset = u32le(self._data, 0x0C)
        self.texture_table_offset = u32le(self._data, 0x14)


class AnimationBank:
    """ANM offset table and editable native keyframe streams."""

    def __init__(self, data: bytes, slot_count: int):
        self._data = bytearray(data)
        self.slot_count = slot_count
        require_range(data, 0, min(len(data), 4))
        table_size = 4 + slot_count * 4
        require_range(data, 0, table_size)
        self.offsets = tuple(
            u32le(data, 4 + index * 4)
            for index in range(slot_count)
        )
        for index, offset in enumerate(self.offsets):
            if offset != 0 and (offset < table_size or offset >= len(data)):
                raise FormatError(
                    f"ANM slot {index} has invalid stream offset 0x{offset:X}"
                )

    @property
    def data(self) -> bytes:
        return bytes(self._data)

    def stream(self, slot_index: int) -> bytes | None:
        offset = self.offsets[slot_index]
        if offset == 0:
            return None
        later = sorted(value for value in self.offsets if value > offset)
        end = later[0] if later else len(self._data)
        return bytes(self._data[offset:end])

    def frames(self, slot_index: int) -> tuple[AnimationFrame, ...]:
        start = self.offsets[slot_index]
        if start == 0:
            return ()
        later = sorted(value for value in self.offsets if value > start)
        end = later[0] if later else len(self._data)
        frames = []
        cursor = start
        while cursor < end:
            if end - cursor < 4:
                if any(self._data[cursor:end]):
                    raise FormatError(
                        f"ANM slot {slot_index} has nonzero trailing bytes"
                    )
                break
            frame_start = cursor
            frame_delta = i16le(self._data, cursor)
            flags = i16le(self._data, cursor + 2)
            cursor += 4
            rotation = None
            rotation_parameter = 0
            absolute = None
            delta = None
            texture_bindings = []
            scale = None
            mesh_data = None
            if flags < 0:
                pass
            else:
                if flags & ~0x7B:
                    raise FormatError(
                        f"ANM slot {slot_index} frame at 0x{frame_start:X} "
                        f"has unsupported flags 0x{flags:04X}"
                    )
                if flags & 0x01:
                    require_range(self._data, cursor, 8)
                    rotation = (
                        i16le(self._data, cursor),
                        i16le(self._data, cursor + 2),
                        i16le(self._data, cursor + 4),
                    )
                    rotation_parameter = i16le(self._data, cursor + 6)
                    cursor += 8
                if flags & 0x02:
                    require_range(self._data, cursor, 12)
                    absolute = (
                        i32le(self._data, cursor),
                        i32le(self._data, cursor + 4),
                        i32le(self._data, cursor + 8),
                    )
                    cursor += 12
                if flags & 0x08:
                    require_range(self._data, cursor, 8)
                    delta = (
                        i16le(self._data, cursor),
                        i16le(self._data, cursor + 2),
                        i16le(self._data, cursor + 4),
                    )
                    cursor += 8
                if flags & 0x10:
                    while True:
                        require_range(self._data, cursor, 4)
                        target = u16le(self._data, cursor)
                        texture = u16le(self._data, cursor + 2)
                        texture_bindings.append(
                            (target & 0x7FFF, texture)
                        )
                        cursor += 4
                        if target & 0x8000:
                            break
                if flags & 0x20:
                    require_range(self._data, cursor, 8)
                    scale = (
                        i16le(self._data, cursor),
                        i16le(self._data, cursor + 2),
                        i16le(self._data, cursor + 4),
                        i16le(self._data, cursor + 6),
                    )
                    cursor += 8
                if flags & 0x40:
                    require_range(self._data, cursor, 4)
                    count = u32le(self._data, cursor)
                    mesh_size = 4 + count * 8
                    require_range(self._data, cursor, mesh_size)
                    mesh_data = bytes(self._data[cursor : cursor + mesh_size])
                    cursor += mesh_size
            if cursor > end:
                raise FormatError(
                    f"ANM slot {slot_index} frame crosses its stream boundary"
                )
            frames.append(
                AnimationFrame(
                    offset=frame_start,
                    size=cursor - frame_start,
                    frame_delta=frame_delta,
                    flags=flags,
                    rotation_yxz=rotation,
                    rotation_parameter=rotation_parameter,
                    translation_absolute=absolute,
                    translation_delta=delta,
                    texture_bindings=tuple(texture_bindings),
                    scale=scale,
                    mesh_data=mesh_data,
                    raw=bytes(self._data[frame_start:cursor]),
                )
            )
        return tuple(frames)

    def patch_frame(
        self,
        slot_index: int,
        frame_index: int,
        *,
        rotation_yxz: Sequence[int] | None = None,
        translation_absolute: Sequence[int] | None = None,
        translation_delta: Sequence[int] | None = None,
        scale: Sequence[int] | None = None,
    ) -> None:
        frame = self.frames(slot_index)[frame_index]
        cursor = frame.offset + 4

        def require_components(
            name: str,
            values: Sequence[int] | None,
            expected: int,
            present: bool,
        ) -> tuple[int, ...] | None:
            if values is None:
                return None
            if not present:
                raise ValueError(f"frame does not contain {name}")
            if len(values) != expected:
                raise ValueError(f"{name} requires {expected} components")
            return tuple(int(value) for value in values)

        rotation = require_components(
            "rotation_yxz", rotation_yxz, 3, frame.rotation_yxz is not None
        )
        absolute = require_components(
            "translation_absolute",
            translation_absolute,
            3,
            frame.translation_absolute is not None,
        )
        delta = require_components(
            "translation_delta",
            translation_delta,
            3,
            frame.translation_delta is not None,
        )
        scale_values = require_components(
            "scale", scale, 4, frame.scale is not None
        )

        if frame.flags < 0:
            if any(value is not None for value in (rotation, absolute, delta, scale_values)):
                raise ValueError("jump frames have no transform payload")
            return
        if frame.flags & 0x01:
            if rotation is not None:
                for component, field in zip(rotation, (0, 2, 4)):
                    put_i16le(self._data, cursor + field, component)
            cursor += 8
        if frame.flags & 0x02:
            if absolute is not None:
                for component, field in zip(absolute, (0, 4, 8)):
                    put_i32le(self._data, cursor + field, component)
            cursor += 12
        if frame.flags & 0x08:
            if delta is not None:
                for component, field in zip(delta, (0, 2, 4)):
                    put_i16le(self._data, cursor + field, component)
            cursor += 8
        if frame.flags & 0x10:
            while True:
                channel = u16le(self._data, cursor)
                cursor += 4
                if channel & 0x8000:
                    break
        if frame.flags & 0x20:
            if scale_values is not None:
                for component, field in zip(scale_values, (0, 2, 4, 6)):
                    put_i16le(self._data, cursor + field, component)
