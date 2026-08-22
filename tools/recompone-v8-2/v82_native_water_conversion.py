#!/usr/bin/env python3
"""Reusable original-V8 surface to native V8:2 water conversion helpers."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import struct
import sys


REPO = Path(__file__).resolve().parents[2]
ADDONS = REPO / "tools" / "blender_addons"
sys.path.insert(0, str(ADDONS))

from vigilante8_vehicle_tools import conversion, iff, registry  # noqa: E402
from vigilante8_vehicle_tools.project import ObjectBank, Texture  # noqa: E402


@dataclass(frozen=True)
class NativeWaterSource:
    """All authored data needed to replace a modeled V8 water surface."""

    bank_index: int
    bank: ObjectBank
    plane_y: int
    rectangle: tuple[int, int, int, int, int, int, int]


def decode_v8_arena_bank(node: iff.IffChunk) -> ObjectBank:
    """Decode an arena bank while allowing its engine-global texture selector."""

    decode_face = registry._decode_face

    def decode_terrain_face(packet, packet_index, native_group, model):
        texture_count = model.texture_count
        model.texture_count = 0x4000
        try:
            return decode_face(packet, packet_index, native_group, model)
        finally:
            model.texture_count = texture_count

    registry._decode_face = decode_terrain_face
    try:
        return registry._decode_bank(node, "V8")
    finally:
        registry._decode_face = decode_face


def find_native_water_source(document: iff.IffDocument) -> NativeWaterSource:
    """Find the V8 data contract that has a direct V8:2 XWAT equivalent.

    Detection is structural.  It never consults an arena name, object name,
    archive slot, or runtime identity.
    """

    banks = [decode_v8_arena_bank(form) for form in document.forms(b"XOBF")]
    surface_indices = [
        index for index, bank in enumerate(banks)
        if conversion.is_v8_alpha_coverage_surface(bank)
    ]
    if len(surface_indices) != 1:
        raise ValueError(
            "arena must contain exactly one V8 alpha-coverage water surface"
        )
    bank_index = surface_indices[0]

    plane_values: list[int] = []
    for obj in document.forms(b"OBJ "):
        for child in obj.children:
            if child.tag != b"HEAD" or len(child.payload) < 34:
                continue
            authored_bank = struct.unpack_from(">h", child.payload, 26)[0]
            if authored_bank == bank_index:
                plane_values.append(
                    struct.unpack_from(">i", child.payload, 12)[0] - 0x100000
                )
    if len(plane_values) != 1:
        raise ValueError(
            "water surface must have exactly one owning object transform"
        )

    rectangles = []
    for node in document.walk():
        if node.tag != b"RECT" or len(node.payload) != 14:
            continue
        record = struct.unpack(">7h", node.payload)
        if record[5] == 0x0043 and record[6] == -1:
            rectangles.append(record)
    if len(rectangles) != 1:
        raise ValueError(
            "water surface must have exactly one V8 water rectangle"
        )
    return NativeWaterSource(
        bank_index=bank_index,
        bank=banks[bank_index],
        plane_y=plane_values[0],
        rectangle=rectangles[0],
    )


def convert_water_rectangle(payload: bytes) -> bytes:
    """Translate an original-V8 drowning region to V8:2's native contract.

    Original V8 identifies its drowning volume with ``0x0043/-1`` and
    exclusive upper endpoints.  V8:2 has native global-water rendering and
    physics but no equivalent region-driven drowning lifecycle (stock Bayou
    only floats the vehicle).  Preserve the source semantic tag as the generic
    imported-water extension marker while translating the endpoints to the
    target parser's inclusive convention.
    """

    if len(payload) != 14:
        return payload
    x0, z0, x1, z1, height, attr_a, selector = struct.unpack(">7h", payload)
    if attr_a != 0x0043 or selector != -1:
        return payload
    if x1 <= x0 or z1 <= z0:
        raise ValueError("V8 water rectangle has invalid exclusive endpoints")
    return struct.pack(
        ">7h", x0, z0, x1 - 1, z1 - 1, height, attr_a, selector
    )


def xwat_payload(document: iff.IffDocument) -> bytes:
    chunks = [node.payload for node in document.walk() if node.tag == b"XWAT"]
    if len(chunks) != 1:
        raise ValueError("native water template must contain exactly one XWAT")
    return chunks[0]


def encode_native_xwat(texture: Texture, template: bytes) -> bytes:
    """Encode a 32x32 V8 phase as Bayou's standard 64x64 CI4 XWAT."""

    if (
        texture.depth != 0
        or (texture.width, texture.height) != (32, 32)
        or len(texture.indices) != 32 * 32
        or not 1 <= len(texture.palette_bgr555) <= 16
        or any(index >= len(texture.palette_bgr555) for index in texture.indices)
    ):
        raise ValueError("native XWAT source must be one valid 32x32 CI4 phase")
    if len(template) < 64 or struct.unpack_from("<II", template, 0) != (0x10, 0x08):
        raise ValueError("Bayou XWAT template is not a standard CI4 TIM")
    clut_size, clut_x, clut_y, clut_width, clut_height = struct.unpack_from(
        "<IHHHH", template, 8
    )
    image_offset = 8 + clut_size
    image_size, image_x, image_y, width_words, height = struct.unpack_from(
        "<IHHHH", template, image_offset
    )
    if (
        clut_height != 1
        or clut_width != 16
        or clut_size != 44
        or width_words != 16
        or height != 64
        or image_size != 2060
        or image_offset + image_size != len(template)
    ):
        raise ValueError("Bayou XWAT template does not have the 64x64 CI4 contract")

    expanded = bytearray()
    for source_y in range(32):
        row = texture.indices[source_y * 32:(source_y + 1) * 32]
        doubled = bytes(index for value in row for index in (value, value))
        expanded += doubled
        expanded += doubled
    packed = bytes(
        expanded[index] | (expanded[index + 1] << 4)
        for index in range(0, len(expanded), 2)
    )
    palette = tuple(texture.palette_bgr555) + (0,) * (
        clut_width - len(texture.palette_bgr555)
    )
    result = bytearray(struct.pack("<II", 0x10, 0x08))
    result += struct.pack("<IHHHH", 44, clut_x, clut_y, 16, 1)
    result += struct.pack("<16H", *palette)
    result += struct.pack(
        "<IHHHH", 2060, image_x, image_y, 16, 64
    )
    result += packed
    if len(result) != len(template):
        raise AssertionError("encoded XWAT size differs from Bayou's contract")
    return bytes(result)


def native_water_init_argument(plane_y: int) -> int:
    """Return func_80017F34 A0 for an authored V8 world-space plane."""

    argument = 0x2FF800 - plane_y
    if not 0 <= argument <= 0xFFFFFFFF:
        raise ValueError("water plane cannot be represented by V8:2 initializer")
    return argument
