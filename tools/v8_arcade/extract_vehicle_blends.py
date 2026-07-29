#!/usr/bin/env python3
"""Create complete per-vehicle Blender files from V8 Arcade ISOP bundles.

Run with Blender 4.5 or newer:

    blender --background --factory-startup --python extract_vehicle_blends.py \
      -- Manta.ib --out extracted_blends

Every discovered Model3 allocation is retained.  Authored Scene/Node references
are reconstructed as named collections, the final compact scene is labeled as
the PS1 classic model, and models not reached through the currently recovered
node types are placed in an auxiliary collection instead of being discarded.
Xbox 360 tiled BC textures referenced by material parameter blocks are decoded,
saved as PNG, and packed into the .blend.
"""

from __future__ import annotations

import argparse
import json
import math
import struct
import sys
import zlib
from pathlib import Path

import bpy
from mathutils import Matrix

sys.path.insert(0, str(Path(__file__).resolve().parent))
from extract_vehicle_lods import Model3, SubMesh, decode_bundle, find_models
from inspect_vehicle_scenes import matrix_be, scene_roots, u32le, valid_node
from inspect_vehicle_textures import xbox360_texture_candidates


USAGE_NAMES = {
    0: "POSITION",
    1: "BLENDWEIGHT",
    2: "BLENDINDICES",
    3: "NORMAL",
    4: "PSIZE",
    5: "TEXCOORD",
    6: "TANGENT",
    7: "BINORMAL",
    8: "TESSFACTOR",
    9: "POSITIONT",
    10: "COLOR",
    11: "FOG",
    12: "DEPTH",
    13: "SAMPLE",
}

FORMAT_NAMES = {
    6: "RGBA8",
    18: "BC1",
    19: "BC2",
    20: "BC3",
    49: "BC5",
    58: "BC2_ALPHA",
    59: "BC3_ALPHA",
    60: "CTX1",
}

# These are the reduced native body Model3 objects shown by the original
# perspective comparison as the per-vehicle numbered candidates.  Their
# immediate Nodes are members of additional serialized part arrays that sit
# outside the public Scene table.  Recovering the owning arrays is essential:
# exporting only the seed body drops the matching reduced doors/panels.
XBOX360_DAMAGE_BODY_SEED_MODELS = {
    "GrooVan": 0x1401F8,
    "Incarcerator": 0x0E82E4,
    "Jefferson": 0x0A8F40,
    "Leprechaun": 0x0B0AE4,
    "Mammoth": 0x0F1168,
    "Manta": 0x0A0C18,
    "Piranha": 0x0F2DBC,
    "Stag": 0x0B7958,
}


def u32be(data: bytes, offset: int) -> int:
    return struct.unpack_from(">I", data, offset)[0]


def shader_semantics(data: bytes, technique: int) -> list[dict[str, int | str]]:
    """Read semantics from the Xbox compiled vertex-shader container."""

    try:
        vector = u32le(data, technique)
        blob = u32le(data, vector)
        if u32be(data, blob) & 0xFFFFFF00 != 0x102A1100:
            return []
        shader = blob + u32be(data, blob + 24)
        first = u32be(data, shader + 24)
        count = u32be(data, shader + 28)
        array = shader + 36
        if count > 32 or array + (first + count) * 4 > len(data):
            return []
        result = []
        for index in range(count):
            value = u32be(data, array + (first + index) * 4)
            usage = (value >> 12) & 0xF
            result.append(
                {
                    "fetch_address": value & 0xFFF,
                    "usage": usage,
                    "usage_index": (value >> 16) & 0xF,
                    "name": USAGE_NAMES.get(usage, f"USAGE_{usage}"),
                }
            )
        return result
    except (IndexError, struct.error):
        return []


def shader_constants(
    data: bytes, technique: int, stage: str = "pixel"
) -> list[dict[str, int | str]]:
    """Read the embedded D3DX constant table from a compiled Xbox shader."""

    try:
        vector_offset = technique + (4 if stage == "pixel" else 0)
        vector = u32le(data, vector_offset)
        blob = u32le(data, vector)
        if u32be(data, blob) & 0xFFFFFF00 != 0x102A1100:
            return []
        table = blob + 0x28
        if u32be(data, table) != 0x1C:
            return []
        count = u32be(data, table + 12)
        records = table + u32be(data, table + 16)
        if count > 256 or records + count * 20 > len(data):
            return []

        result = []
        for index in range(count):
            record = records + index * 20
            name_offset = table + u32be(data, record)
            name_end = data.find(b"\0", name_offset, min(len(data), name_offset + 256))
            if name_end < 0:
                return []
            name = data[name_offset:name_end].decode("ascii", errors="replace")
            result.append(
                {
                    "name": name,
                    "register_set": struct.unpack_from(">H", data, record + 4)[0],
                    "register_index": struct.unpack_from(">H", data, record + 6)[0],
                    "register_count": struct.unpack_from(">H", data, record + 8)[0],
                    "type_info_offset": table + u32be(data, record + 12),
                    "default_value_offset": (
                        table + u32be(data, record + 16)
                        if u32be(data, record + 16)
                        else 0
                    ),
                }
            )
        return result
    except (IndexError, struct.error):
        return []


def is_material_parameter(name: str) -> bool:
    lowered = name.lower()
    if name.startswith("_"):
        return False
    if lowered in {
        "world",
        "worldview",
        "worldviewproj",
        "view",
        "iview",
        "piview",
        "shadow",
    }:
        return False
    if lowered.startswith("light_"):
        return False
    return any(
        token in lowered
        for token in (
            "samp",
            "color",
            "colour",
            "gloss",
            "noisi",
            "rough",
            "spec",
            "opacity",
            "alpha",
        )
    )


def material_parameter_values(
    data: bytes, technique: int, parameters_offset: int
) -> list[dict[str, object]]:
    """Decode the packed material-local values in shader declaration order."""

    values = []
    cursor = parameters_offset
    for constant in shader_constants(data, technique, "pixel"):
        name = str(constant["name"])
        if not is_material_parameter(name):
            continue
        register_set = int(constant["register_set"])
        type_info = int(constant["type_info_offset"])
        try:
            if register_set == 3:  # D3DXRS_SAMPLER
                size = 4
                value: object = u32le(data, cursor)
            else:
                rows = struct.unpack_from(">H", data, type_info + 4)[0]
                columns = struct.unpack_from(">H", data, type_info + 6)[0]
                elements = struct.unpack_from(">H", data, type_info + 8)[0] or 1
                component_count = max(1, rows) * max(1, columns) * elements
                size = component_count * 4
                value = struct.unpack_from(f">{component_count}f", data, cursor)
        except (IndexError, struct.error):
            break
        values.append(
            {
                **constant,
                "parameter_offset": cursor,
                "byte_size": size,
                "value": value,
            }
        )
        cursor += size
    return values


def semantic_layout(
    semantics: list[dict[str, int | str]], stride: int
) -> list[dict[str, int | str]]:
    """Return the observed on-disk vertex struct layout.

    The shader semantic table is in fetch/result-register order, not memory
    order. The old exporter incorrectly treated it as sequential, mapping
    NORMAL.xyz at +0x0C into the UV layer. All vehicle streams use the float
    layouts below; fields are ordered POSITION, NORMAL, auxiliary TEXCOORDs,
    primary TEXCOORD0, then tangents.
    """

    by_key = {
        (str(semantic["name"]), int(semantic["usage_index"])): semantic
        for semantic in semantics
    }

    def field(
        name: str, usage_index: int, offset: int, size: int, encoding: str
    ) -> dict[str, int | str]:
        semantic = by_key.get(
            (name, usage_index),
            {
                "fetch_address": -1,
                "usage": {"POSITION": 0, "NORMAL": 3, "TEXCOORD": 5, "TANGENT": 6}.get(
                    name, -1
                ),
                "usage_index": usage_index,
                "name": name,
            },
        )
        return {
            **semantic,
            "offset": offset,
            "size": size,
            "encoding": encoding,
        }

    keys = set(by_key)
    if stride == 12:
        return [field("POSITION", 0, 0, 12, "float3")]
    if stride == 20:
        return [
            field("POSITION", 0, 0, 12, "float3"),
            field("TEXCOORD", 0, 12, 8, "float2"),
        ]
    if stride == 24 and ("NORMAL", 0) in keys:
        return [
            field("POSITION", 0, 0, 12, "float3"),
            field("NORMAL", 0, 12, 12, "float3"),
        ]
    if stride == 24:
        return [
            field("POSITION", 0, 0, 12, "float3"),
            field("TEXCOORD", 0, 12, 12, "float3"),
        ]
    if stride == 32:
        return [
            field("POSITION", 0, 0, 12, "float3"),
            field("NORMAL", 0, 12, 12, "float3"),
            field("TEXCOORD", 0, 24, 8, "float2"),
        ]
    if stride == 44:
        return [
            field("POSITION", 0, 0, 12, "float3"),
            field("NORMAL", 0, 12, 12, "float3"),
            field("TEXCOORD", 1, 24, 12, "float3"),
            field("TEXCOORD", 0, 36, 8, "float2"),
        ]
    if stride == 48:
        return [
            field("POSITION", 0, 0, 12, "float3"),
            field("NORMAL", 0, 12, 12, "float3"),
            field("TEXCOORD", 0, 24, 8, "float2"),
            field("TANGENT", 0, 32, 16, "float4"),
        ]
    if stride == 60:
        return [
            field("POSITION", 0, 0, 12, "float3"),
            field("NORMAL", 0, 12, 12, "float3"),
            field("TEXCOORD", 1, 24, 12, "float3"),
            field("TEXCOORD", 0, 36, 8, "float2"),
            field("TANGENT", 0, 44, 16, "float4"),
        ]
    if stride == 92:
        return [
            field("POSITION", 0, 0, 12, "float3"),
            field("NORMAL", 0, 12, 12, "float3"),
            field("TEXCOORD", 3, 24, 12, "float3"),
            field("TEXCOORD", 0, 36, 8, "float2"),
            field("TEXCOORD", 1, 44, 8, "float2"),
            field("TEXCOORD", 2, 52, 8, "float2"),
            field("TANGENT", 0, 60, 16, "float4"),
            field("TANGENT", 1, 76, 16, "float4"),
        ]
    return [field("POSITION", 0, 0, 12, "float3")]


def signed_bits(value: int, bits: int) -> int:
    sign = 1 << (bits - 1)
    return value - (1 << bits) if value & sign else value


def decode_attribute(data: bytes, offset: int, encoding: str):
    if encoding == "float2":
        return struct.unpack_from(">2f", data, offset)
    if encoding == "float3":
        return struct.unpack_from(">3f", data, offset)
    if encoding == "float4":
        return struct.unpack_from(">4f", data, offset)
    if encoding == "half2":
        return struct.unpack_from(">2e", data, offset)
    if encoding == "half4":
        return struct.unpack_from(">4e", data, offset)
    if encoding == "rgba8":
        return tuple(component / 255.0 for component in data[offset : offset + 4])
    if encoding.startswith("packed"):
        value = u32be(data, offset)
        # Common Xenos signed 10_11_11 / 2_10_10_10 usage.
        x = signed_bits(value & 0x3FF, 10) / 511.0
        y = signed_bits((value >> 10) & 0x7FF, 11) / 1023.0
        z = signed_bits((value >> 21) & 0x7FF, 11) / 1023.0
        if encoding == "packed4":
            return (x, y, z, 1.0)
        return (x, y, z)
    return tuple(data[offset : offset + 4])


def tiled_offset_2d(x: int, y: int, pitch: int, log_bpb: int) -> int:
    pitch = (pitch + 31) & ~31
    macro = ((x >> 5) + (y >> 5) * (pitch >> 5)) << (log_bpb + 7)
    micro = ((x & 7) + ((y & 0xE) << 2)) << log_bpb
    offset = macro + ((micro & ~0xF) << 1) + (micro & 0xF) + ((y & 1) << 4)
    return (
        ((offset & ~0x1FF) << 3)
        + ((y & 16) << 7)
        + ((offset & 0x1C0) << 2)
        + (((((y & 8) >> 2) + (x >> 3)) & 3) << 6)
        + (offset & 0x3F)
    )


def swap_block(block: bytes, endian: int) -> bytes:
    if endian == 0:
        return block
    if endian == 1:
        return b"".join(block[index : index + 2][::-1] for index in range(0, len(block), 2))
    if endian == 2:
        return b"".join(block[index : index + 4][::-1] for index in range(0, len(block), 4))
    return b"".join(
        block[index + 2 : index + 4] + block[index : index + 2]
        for index in range(0, len(block), 4)
    )


def rgb565(value: int) -> tuple[int, int, int]:
    return (
        ((value >> 11) & 31) * 255 // 31,
        ((value >> 5) & 63) * 255 // 63,
        (value & 31) * 255 // 31,
    )


def bc1_colors(block: bytes, force_four: bool = False):
    c0, c1, selectors = struct.unpack_from("<HHI", block)
    a, b = rgb565(c0), rgb565(c1)
    if c0 > c1 or force_four:
        colors = [
            (*a, 255),
            (*b, 255),
            (*(tuple((2 * a[i] + b[i]) // 3 for i in range(3))), 255),
            (*(tuple((a[i] + 2 * b[i]) // 3 for i in range(3))), 255),
        ]
    else:
        colors = [
            (*a, 255),
            (*b, 255),
            (*(tuple((a[i] + b[i]) // 2 for i in range(3))), 255),
            (0, 0, 0, 0),
        ]
    return [colors[(selectors >> (2 * index)) & 3] for index in range(16)]


def bc4_values(block: bytes) -> list[int]:
    a0, a1 = block[0], block[1]
    bits = int.from_bytes(block[2:8], "little")
    values = [a0, a1]
    if a0 > a1:
        values.extend(((7 - i) * a0 + i * a1) // 7 for i in range(1, 7))
    else:
        values.extend(((5 - i) * a0 + i * a1) // 5 for i in range(1, 5))
        values.extend((0, 255))
    return [values[(bits >> (3 * index)) & 7] for index in range(16)]


def decode_block(block: bytes, texture_format: int):
    if texture_format == 18:
        return bc1_colors(block)
    if texture_format == 19:
        alpha_bits = int.from_bytes(block[:8], "little")
        colors = bc1_colors(block[8:16], True)
        return [
            (*colors[index][:3], ((alpha_bits >> (4 * index)) & 15) * 17)
            for index in range(16)
        ]
    if texture_format == 20:
        alpha = bc4_values(block[:8])
        colors = bc1_colors(block[8:16], True)
        return [(*colors[index][:3], alpha[index]) for index in range(16)]
    if texture_format == 49:
        red, green = bc4_values(block[:8]), bc4_values(block[8:16])
        result = []
        for r, g in zip(red, green):
            x, y = r / 127.5 - 1.0, g / 127.5 - 1.0
            z = math.sqrt(max(0.0, 1.0 - x * x - y * y))
            result.append((r, g, round((z * 0.5 + 0.5) * 255), 255))
        return result
    if texture_format in (58, 59):
        alpha = (
            [((int.from_bytes(block[:8], "little") >> (4 * i)) & 15) * 17 for i in range(16)]
            if texture_format == 58
            else bc4_values(block[:8])
        )
        return [(value, value, value, 255) for value in alpha]
    if texture_format == 60:
        x0, y0, x1, y1 = block[:4]
        selectors = int.from_bytes(block[4:8], "little")
        palette = [
            (x0, y0),
            (x1, y1),
            ((2 * x0 + x1) // 3, (2 * y0 + y1) // 3),
            ((x0 + 2 * x1) // 3, (y0 + 2 * y1) // 3),
        ]
        result = []
        for index in range(16):
            r, g = palette[(selectors >> (2 * index)) & 3]
            x, y = r / 127.5 - 1.0, g / 127.5 - 1.0
            z = math.sqrt(max(0.0, 1.0 - x * x - y * y))
            result.append((r, g, round((z * 0.5 + 0.5) * 255), 255))
        return result
    raise ValueError(f"unsupported texture format {texture_format}")


def decode_texture(data: bytes, record: dict[str, object]) -> bytes:
    width, height = int(record["width"]), int(record["height"])
    texture_format = int(record["format"])
    if texture_format == 6:
        block_width = block_height = 1
        block_bytes = 4
    else:
        block_width = block_height = 4
        block_bytes = 8 if texture_format in (18, 58, 59, 60) else 16
    width_blocks = (width + block_width - 1) // block_width
    height_blocks = (height + block_height - 1) // block_height
    pitch_blocks = max(width_blocks, int(record["pitch_pixels"]) // block_width)
    log_bpb = int(math.log2(block_bytes))
    base = int(record["base_file_offset"])
    rgba = bytearray(width * height * 4)
    for by in range(height_blocks):
        for bx in range(width_blocks):
            source = base + tiled_offset_2d(bx, by, pitch_blocks, log_bpb)
            block = swap_block(data[source : source + block_bytes], int(record["endian"]))
            if len(block) != block_bytes:
                continue
            if texture_format == 6:
                pixels = [tuple(block)]
            else:
                pixels = decode_block(block, texture_format)
            for py in range(block_height):
                y = by * block_height + py
                if y >= height:
                    continue
                for px in range(block_width):
                    x = bx * block_width + px
                    if x >= width:
                        continue
                    pixel = pixels[py * block_width + px]
                    target = (y * width + x) * 4
                    rgba[target : target + 4] = bytes(pixel)
    return bytes(rgba)


def png_chunk(kind: bytes, payload: bytes) -> bytes:
    return (
        struct.pack(">I", len(payload))
        + kind
        + payload
        + struct.pack(">I", zlib.crc32(kind + payload) & 0xFFFFFFFF)
    )


def write_png(path: Path, width: int, height: int, rgba: bytes) -> None:
    rows = b"".join(
        b"\0" + rgba[y * width * 4 : (y + 1) * width * 4] for y in range(height)
    )
    payload = (
        b"\x89PNG\r\n\x1a\n"
        + png_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0))
        + png_chunk(b"IDAT", zlib.compress(rows, 6))
        + png_chunk(b"IEND", b"")
    )
    path.write_bytes(payload)


def node_matrix(values: tuple[float, ...]) -> Matrix:
    # Isopod's affine Matrix stores a row-major 3x3 basis followed by XYZ
    # translation, rather than three interleaved float4 rows.
    return Matrix(
        (
            (values[0], values[1], values[2], values[9]),
            (values[3], values[4], values[5], values[10]),
            (values[6], values[7], values[8], values[11]),
            (0.0, 0.0, 0.0, 1.0),
        )
    )


def make_material(
    vehicle: str,
    submesh: SubMesh,
    texture_bindings: list[tuple[str, int]],
    parameter_values: list[dict[str, object]],
    images: dict[int, bpy.types.Image],
    texture_records: dict[int, dict[str, object]],
) -> bpy.types.Material:
    material = bpy.data.materials.new(
        f"{vehicle}_material_{submesh.parameters_offset:08X}"
    )
    material.use_nodes = True
    material["source_parameter_offset"] = submesh.parameters_offset
    material["source_technique_offset"] = submesh.technique_offset
    material["source_texture_offsets"] = [
        texture_offset for _, texture_offset in texture_bindings
    ]
    material["source_sampler_bindings"] = [
        f"{name}=0x{texture_offset:08X}"
        for name, texture_offset in texture_bindings
    ]
    material["decoded_material_parameters"] = [
        f"{parameter['name']}={parameter['value']}"
        for parameter in parameter_values
    ]
    material["shader_reconstruction"] = (
        "D3DX sampler names recovered from the compiled Xbox pixel shader; "
        "inspection material is opaque and connects only diffuse_samp RGB. "
        "Alpha, paintColor0, and all other recovered resources remain packed "
        "and labeled but unconnected."
    )
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    principled = nodes.get("Principled BSDF")
    if principled is not None:
        principled.inputs["Metallic"].default_value = 0.0
        principled.inputs["Roughness"].default_value = 1.0
        principled.inputs["Alpha"].default_value = 1.0
        if "Specular IOR Level" in principled.inputs:
            principled.inputs["Specular IOR Level"].default_value = 0.0
        if "Coat Weight" in principled.inputs:
            principled.inputs["Coat Weight"].default_value = 0.0
    parameter_by_name = {
        str(parameter["name"]): parameter["value"] for parameter in parameter_values
    }
    diffuse_colour = parameter_by_name.get("diffuse_colour")
    if principled is not None and isinstance(diffuse_colour, tuple):
        colour = tuple(float(component) for component in diffuse_colour)
        principled.inputs["Base Color"].default_value = (
            colour[0],
            colour[1] if len(colour) > 1 else colour[0],
            colour[2] if len(colour) > 2 else colour[0],
            colour[3] if len(colour) > 3 else 1.0,
        )
    x = -700
    connected_base = False
    for index, (sampler_name, texture_offset) in enumerate(texture_bindings):
        image = images.get(texture_offset)
        if image is None:
            continue
        node = nodes.new("ShaderNodeTexImage")
        node.name = f"Xbox360Texture_{texture_offset:08X}"
        node.label = f"{sampler_name} — resource 0x{texture_offset:08X}"
        node.image = image
        node.location = (x, -240 * index)
        node.extension = "REPEAT"
        lowered = sampler_name.lower()
        if (
            principled is not None
            and any(token in lowered for token in ("diffuse", "albedo", "base"))
        ):
            links.new(node.outputs["Color"], principled.inputs["Base Color"])
            connected_base = True
        elif "normal" in lowered:
            image.colorspace_settings.name = "Non-Color"
            node.label += " — inspection material: unconnected"
        elif any(token in lowered for token in ("gloss", "rough", "spec")):
            image.colorspace_settings.name = "Non-Color"
            node.label += " — inspection material: unconnected"

    if not connected_base and texture_bindings and principled is not None:
        # Preserve a useful preview for stripped/unknown sampler names without
        # claiming that the fallback is an authoritative shader binding.
        sampler_name, texture_offset = texture_bindings[0]
        fallback = nodes.get(f"Xbox360Texture_{texture_offset:08X}")
        if fallback is not None and "normal" not in sampler_name.lower():
            links.new(fallback.outputs["Color"], principled.inputs["Base Color"])
            material["base_color_binding_confidence"] = "LOW_FALLBACK_FIRST_SAMPLER"
    else:
        material["base_color_binding_confidence"] = "HIGH_DIFFUSE_SAMPLER_NAME"
    return material


def build_model_mesh(
    vehicle: str,
    data: bytes,
    model: Model3,
    texture_offsets: set[int],
    images: dict[int, bpy.types.Image],
    texture_records: dict[int, dict[str, object]],
    material_cache: dict[
        tuple[int, int, tuple[tuple[str, int], ...]], bpy.types.Material
    ],
) -> bpy.types.Mesh:
    vertices = []
    faces = []
    polygon_materials = []
    vertex_uvs: list[tuple[float, float]] = []
    vertex_normals: list[tuple[float, float, float]] = []
    nonfinite_uv_sentinel_count = 0
    materials = []
    vertex_base = 0
    for submesh_index, submesh in enumerate(model.submeshes):
        semantics = shader_semantics(data, submesh.technique_offset)
        layout = semantic_layout(semantics, submesh.vertex_stride)
        uv_field = next((field for field in layout if field["name"] == "TEXCOORD"), None)
        normal_field = next((field for field in layout if field["name"] == "NORMAL"), None)
        for vertex_index in range(submesh.vertex_count):
            source = submesh.vertex_data_offset + vertex_index * submesh.vertex_stride
            vertices.append(struct.unpack_from(">3f", data, source))
            if uv_field:
                value = decode_attribute(
                    data, source + int(uv_field["offset"]), str(uv_field["encoding"])
                )
                u, v = float(value[0]), float(value[1])
                if not math.isfinite(u) or not math.isfinite(v):
                    # Untextured classic-model submeshes deliberately fill the
                    # otherwise present TEXCOORD slot with 0x7FFFFFFF or
                    # 0xFFC00000. The compiled material has no sampler, so the
                    # value is not consumed by the game. Blender cannot safely
                    # retain NaNs in a UV layer; use a neutral coordinate and
                    # preserve the replacement count as source metadata.
                    u, v = 0.0, 0.0
                    nonfinite_uv_sentinel_count += 1
                vertex_uvs.append((u, 1.0 - v))
            else:
                vertex_uvs.append((0.0, 0.0))
            if normal_field:
                value = decode_attribute(
                    data,
                    source + int(normal_field["offset"]),
                    str(normal_field["encoding"]),
                )
                vertex_normals.append(tuple(float(component) for component in value[:3]))
            else:
                vertex_normals.append((0.0, 0.0, 1.0))

        raw_indices = struct.unpack_from(
            f">{submesh.triangle_count * 3}H", data, submesh.index_data_offset
        )
        for triangle in range(submesh.triangle_count):
            start = triangle * 3
            faces.append(
                tuple(vertex_base + raw_indices[start + corner] for corner in range(3))
            )
            polygon_materials.append(submesh_index)
        vertex_base += submesh.vertex_count

        refs: list[int] = []
        for scan in range(
            submesh.parameters_offset,
            min(submesh.parameters_offset + 160, u32le(data, 8)) - 3,
            4,
        ):
            pointer = u32le(data, scan)
            if pointer in texture_offsets and pointer not in refs:
                refs.append(pointer)
        parameter_values = material_parameter_values(
            data, submesh.technique_offset, submesh.parameters_offset
        )
        texture_bindings = [
            (str(parameter["name"]), int(parameter["value"]))
            for parameter in parameter_values
            if int(parameter["register_set"]) == 3
            and int(parameter["value"]) in texture_offsets
        ]
        if len(texture_bindings) != len(refs):
            bound_offsets = {texture_offset for _, texture_offset in texture_bindings}
            texture_bindings.extend(
                (f"unresolved_sampler_{index}", texture_offset)
                for index, texture_offset in enumerate(refs)
                if texture_offset not in bound_offsets
            )
        key = (
            submesh.technique_offset,
            submesh.parameters_offset,
            tuple(texture_bindings),
        )
        material = material_cache.get(key)
        if material is None:
            material = make_material(
                vehicle,
                submesh,
                texture_bindings,
                parameter_values,
                images,
                texture_records,
            )
            material_cache[key] = material
        materials.append(material)

    mesh = bpy.data.meshes.new(f"{vehicle}_Model3_{model.offset:08X}")
    mesh.from_pydata(vertices, [], faces)
    mesh.materials.clear()
    for material in materials:
        mesh.materials.append(material)
    for polygon, material_index in zip(mesh.polygons, polygon_materials):
        polygon.material_index = material_index
        polygon.use_smooth = True
    uv_layer = mesh.uv_layers.new(name="TEXCOORD_0")
    for loop in mesh.loops:
        uv_layer.data[loop.index].uv = vertex_uvs[loop.vertex_index]
    normal_attribute = mesh.attributes.new("NORMAL_0", "FLOAT_VECTOR", "POINT")
    for index, normal in enumerate(vertex_normals):
        normal_attribute.data[index].vector = normal
    mesh["source_model_offset"] = model.offset
    mesh["source_bounds"] = model.bounds
    mesh["vertex_count"] = model.vertex_count
    mesh["triangle_count"] = model.triangle_count
    mesh["nonfinite_uv_sentinel_count"] = nonfinite_uv_sentinel_count
    mesh["vertex_layouts"] = [
        f"stride={submesh.vertex_stride} "
        + ",".join(
            f"{field['name']}{field['usage_index']}@{field['offset']}:{field['encoding']}"
            for field in semantic_layout(
                shader_semantics(data, submesh.technique_offset),
                submesh.vertex_stride,
            )
        )
        for submesh in model.submeshes
    ]
    mesh.update()
    return mesh


def link_object(
    name: str,
    mesh: bpy.types.Mesh,
    collection: bpy.types.Collection,
    matrix: Matrix | None = None,
) -> bpy.types.Object:
    obj = bpy.data.objects.new(name, mesh)
    collection.objects.link(obj)
    if matrix is not None:
        obj.matrix_world = matrix
    return obj


def collect_scene_instances(
    data: bytes,
    root_offset: int,
    image_limit: int,
    model_by_offset: dict[int, Model3],
) -> tuple[list[dict[str, object]], list[dict[str, object]]]:
    instances: list[dict[str, object]] = []
    unknown: list[dict[str, object]] = []
    active: set[int] = set()

    def visit(
        offset: int, parent_matrix: Matrix, path_name: str
    ) -> None:
        model = model_by_offset.get(offset)
        if model is not None:
            instances.append(
                {
                    "path": path_name,
                    "model": model,
                    "matrix": parent_matrix.copy(),
                }
            )
            return
        if offset in active or not valid_node(data, offset, image_limit):
            unknown.append(
                {
                    "path": path_name,
                    "offset": offset,
                    "matrix": parent_matrix.copy(),
                }
            )
            return
        active.add(offset)
        world = parent_matrix @ node_matrix(matrix_be(data, offset))
        children = u32le(data, offset + 48)
        if children:
            for index in range(u32le(data, children - 4)):
                visit(
                    u32le(data, children + index * 4),
                    world,
                    f"{path_name}_{index:03d}",
                )
        active.remove(offset)

    visit(root_offset, Matrix.Identity(4), "part")
    return instances, unknown


def aligned_u32_references(data: bytes, value: int) -> list[int]:
    needle = struct.pack("<I", value)
    references = []
    cursor = 0
    while True:
        offset = data.find(needle, cursor)
        if offset < 0:
            return references
        if offset % 4 == 0:
            references.append(offset)
        cursor = offset + 1


def immediate_model_nodes(
    data: bytes, image_limit: int, model_offset: int
) -> list[int]:
    roots = []
    for reference in aligned_u32_references(data, model_offset):
        root = reference - 56
        if (
            root >= 0
            and u32le(data, root + 48) == reference
            and valid_node(data, root, image_limit)
        ):
            roots.append(root)
    return sorted(set(roots))


def recover_damage_assemblies(
    data: bytes,
    image_limit: int,
    model_by_offset: dict[int, Model3],
    seed_model_offset: int,
) -> list[dict[str, object]]:
    """Recover serialized secondary part arrays containing a reduced body."""

    assemblies: dict[int, dict[str, object]] = {}
    for seed_node in immediate_model_nodes(data, image_limit, seed_model_offset):
        for reference in aligned_u32_references(data, seed_node):
            for seed_index in range(128):
                array_offset = reference - seed_index * 4
                if array_offset < 4:
                    break
                count = u32le(data, array_offset - 4)
                if not (seed_index < count <= 128):
                    continue
                if array_offset + count * 4 > image_limit:
                    continue
                roots = [
                    u32le(data, array_offset + index * 4)
                    for index in range(count)
                ]
                if roots[seed_index] != seed_node:
                    continue
                if any(
                    root < 0x20 or root >= image_limit or root % 4
                    for root in roots
                ):
                    continue

                instances = []
                unknown = []
                for array_index, root in enumerate(roots):
                    child_instances, child_unknown = collect_scene_instances(
                        data, root, image_limit, model_by_offset
                    )
                    for instance in child_instances:
                        instance = dict(instance)
                        instance["path"] = (
                            f"damage_{array_index:03d}_{instance['path']}"
                        )
                        instances.append(instance)
                    for record in child_unknown:
                        record = dict(record)
                        record["path"] = (
                            f"damage_{array_index:03d}_{record['path']}"
                        )
                        unknown.append(record)
                if any(
                    instance["model"].offset == seed_model_offset
                    for instance in instances
                ):
                    assemblies[array_offset] = {
                        "array_offset": array_offset,
                        "count": count,
                        "seed_model_offset": seed_model_offset,
                        "instances": instances,
                        "unknown": unknown,
                    }
    return [assemblies[offset] for offset in sorted(assemblies)]


def infer_part_names(instances: list[dict[str, object]]) -> None:
    """Assign useful geometry/placement-based names with confidence metadata."""

    if not instances:
        return
    offset_counts: dict[int, int] = {}
    for instance in instances:
        model = instance["model"]
        assert isinstance(model, Model3)
        offset_counts[model.offset] = offset_counts.get(model.offset, 0) + 1
    body = max(
        instances,
        key=lambda instance: (
            instance["model"].triangle_count,
            instance["model"].vertex_count,
        ),
    )
    used: dict[str, int] = {}

    for instance in instances:
        model = instance["model"]
        matrix = instance["matrix"]
        assert isinstance(model, Model3)
        assert isinstance(matrix, Matrix)
        x, y, z = matrix.translation
        min_x, min_y, min_z, max_x, max_y, max_z = model.bounds
        dx, dy, dz = max_x - min_x, max_y - min_y, max_z - min_z
        side = "left" if x > 0.0 else "right"
        end = "front" if y < 0.0 else "rear"
        confidence = "LOW_INFERRED"
        basis = "unclassified scene placement"

        if instance is body:
            part_name = "body_shell"
            confidence = "HIGH_INFERRED"
            basis = "largest triangle assembly in authored scene"
        elif offset_counts[model.offset] >= 4 and abs(x) > 0.35 and abs(y) > 0.65:
            part_name = f"wheel_{end}_{side}"
            confidence = "HIGH_INFERRED"
            basis = "same circular Model3 instanced at all four axle corners"
        elif abs(x) > 0.45 and abs(y) < 0.75 and dy > 0.65:
            part_name = f"door_{side}"
            confidence = "MEDIUM_INFERRED"
            basis = "large mirrored side panel centered between axles"
        elif abs(x) > 0.45 and abs(y) > 0.75 and model.triangle_count > 80:
            part_name = f"fender_{end}_{side}"
            confidence = "MEDIUM_INFERRED"
            basis = "mirrored body panel adjacent to axle"
        elif abs(x) > 0.35 and dx < 0.45 and dy < 0.45 and z > 0.1:
            part_name = f"side_mirror_{side}"
            confidence = "MEDIUM_INFERRED"
            basis = "small mirrored elevated side object"
        elif abs(x) < 0.3 and y < -1.85:
            part_name = "front_bumper"
            confidence = "MEDIUM_INFERRED"
            basis = "wide centered leading-edge panel"
        elif abs(x) < 0.3 and y > 1.85:
            part_name = "rear_bumper"
            confidence = "MEDIUM_INFERRED"
            basis = "wide centered trailing-edge panel"
        elif abs(x) < 0.3 and y < -0.9 and dx > 0.65 and dz < 0.55:
            part_name = "hood"
            confidence = "MEDIUM_INFERRED"
            basis = "wide centered shallow panel over the front compartment"
        elif abs(x) < 0.3 and y > 1.35 and dx > 0.65 and dz < 0.55:
            part_name = "trunk_or_hatch"
            confidence = "MEDIUM_INFERRED"
            basis = "wide centered shallow panel over the rear compartment"
        elif abs(x) < 0.3 and z > 0.25 and y < 0.1 and dz < 0.35:
            part_name = "windshield"
            confidence = "MEDIUM_INFERRED"
            basis = "wide elevated shallow front-facing panel"
        elif abs(x) < 0.3 and z > 0.25 and y >= 0.1 and dz < 0.35:
            part_name = "rear_window"
            confidence = "MEDIUM_INFERRED"
            basis = "wide elevated shallow rear-facing panel"
        else:
            part_name = f"accessory_{end}"

        duplicate = used.get(part_name, 0)
        used[part_name] = duplicate + 1
        if duplicate:
            part_name = f"{part_name}_{duplicate + 1:02d}"
        instance["part_name"] = part_name
        instance["part_name_confidence"] = confidence
        instance["part_name_basis"] = basis


def referenced_texture_offsets(
    data: bytes,
    models: list[Model3],
    texture_by_offset: dict[int, dict[str, object]],
    image_limit: int,
) -> set[int]:
    result: set[int] = set()
    for model in models:
        for submesh in model.submeshes:
            for parameter in material_parameter_values(
                data, submesh.technique_offset, submesh.parameters_offset
            ):
                if (
                    int(parameter["register_set"]) == 3
                    and int(parameter["value"]) in texture_by_offset
                ):
                    result.add(int(parameter["value"]))
            # Retain unknown pointers as a fallback for techniques whose local
            # parameter names are not recognized yet.
            for scan in range(
                submesh.parameters_offset,
                min(submesh.parameters_offset + 160, image_limit) - 3,
                4,
            ):
                pointer = u32le(data, scan)
                if pointer in texture_by_offset:
                    result.add(pointer)
    return result


def load_images(
    vehicle: str,
    data: bytes,
    texture_offsets: set[int],
    texture_by_offset: dict[int, dict[str, object]],
    texture_dir: Path,
) -> dict[int, bpy.types.Image]:
    texture_dir.mkdir(parents=True, exist_ok=True)
    images: dict[int, bpy.types.Image] = {}
    decoded_by_key: dict[tuple[int, int, int, int], bpy.types.Image] = {}
    for texture_offset in sorted(texture_offsets):
        record = texture_by_offset[texture_offset]
        key = (
            int(record["base_address"]),
            int(record["width"]),
            int(record["height"]),
            int(record["format"]),
        )
        image = decoded_by_key.get(key)
        if image is None:
            name = (
                f"{vehicle}_{int(record['base_address']):08X}_"
                f"{int(record['width'])}x{int(record['height'])}_"
                f"{FORMAT_NAMES.get(int(record['format']), record['format'])}"
            )
            png_path = texture_dir / f"{name}.png"
            if not png_path.exists():
                rgba = decode_texture(data, record)
                write_png(
                    png_path,
                    int(record["width"]),
                    int(record["height"]),
                    rgba,
                )
            image = bpy.data.images.load(str(png_path), check_existing=False)
            image.name = name
            image.pack()
            image["source_texture_offset"] = texture_offset
            image["source_base_address"] = int(record["base_address"])
            image["source_format"] = FORMAT_NAMES.get(
                int(record["format"]), str(record["format"])
            )
            image["source_fetch_dwords"] = " ".join(
                f"{int(value):08X}" for value in record["fetch_dwords"]
            )
            decoded_by_key[key] = image
        images[texture_offset] = image
    return images


def prepare_scene(
    scene_name: str,
    source_path: Path,
    image_limit: int,
    source_model_count: int,
    version_kind: str,
) -> bpy.types.Scene:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.name = scene_name
    scene["source_bundle"] = str(source_path.resolve())
    scene["isop_physical_data_offset"] = image_limit
    scene["discovered_model_count"] = source_model_count
    scene["version_kind"] = version_kind
    if scene.world is None:
        scene.world = bpy.data.worlds.new(f"{scene_name}_World")
    scene.world.color = (0.05, 0.05, 0.05)
    return scene


def export_scene_version(
    path: Path,
    output_root: Path,
    data: bytes,
    image_limit: int,
    models: list[Model3],
    model_by_offset: dict[int, Model3],
    texture_by_offset: dict[int, dict[str, object]],
    scene_record: dict[str, object],
    scene_count: int,
    lod1_assembly: dict[str, object] | None,
) -> dict[str, object]:
    vehicle = path.stem
    index = int(scene_record["index"])
    instances, unknown = collect_scene_instances(
        data,
        int(scene_record["root_offset"]),
        image_limit,
        model_by_offset,
    )
    infer_part_names(instances)
    lod1_instances = (
        list(lod1_assembly["instances"]) if lod1_assembly is not None else []
    )
    lod1_unknown = (
        list(lod1_assembly["unknown"]) if lod1_assembly is not None else []
    )
    infer_part_names(lod1_instances)
    member_models = list(
        {
            instance["model"].offset: instance["model"]
            for instance in [*instances, *lod1_instances]
        }.values()
    )
    triangles = sum(
        instance["model"].triangle_count for instance in instances
    )
    is_classic = (
        index == scene_count - 1
        and bool(instances)
        and 0 < triangles <= 1000
    )
    colour = tuple(float(value) for value in scene_record["ambient_rgb"])
    colour_hex = "".join(
        f"{max(0, min(255, round(component * 255))):02X}"
        for component in colour
    )
    if is_classic:
        stem = f"{vehicle}_PS1_classic"
        version_kind = "ps1_classic"
    else:
        stem = f"{vehicle}_360_skin_{index + 1:02d}_{colour_hex}"
        version_kind = "xbox360_skin"

    scene = prepare_scene(
        stem,
        path,
        image_limit,
        len(models),
        version_kind,
    )
    scene["source_scene_index"] = index
    scene["source_scene_offset"] = int(scene_record["scene_offset"])
    scene["source_root_offset"] = int(scene_record["root_offset"])
    scene["skin_color_rgb"] = colour
    scene["scene_model_instance_count"] = len(instances)
    scene["scene_triangle_count"] = triangles
    scene["unresolved_scene_object_count"] = len(unknown)
    scene["damage_model_instance_count"] = len(lod1_instances)
    scene["damage_triangle_count"] = sum(
        instance["model"].triangle_count for instance in lod1_instances
    )
    if lod1_assembly is not None:
        scene["damage_source_array_offset"] = int(lod1_assembly["array_offset"])
        scene["damage_seed_model_offset"] = int(lod1_assembly["seed_model_offset"])

    texture_offsets = referenced_texture_offsets(
        data, member_models, texture_by_offset, image_limit
    )
    images = load_images(
        vehicle,
        data,
        texture_offsets,
        texture_by_offset,
        output_root / "textures" / vehicle,
    )
    material_cache = {}
    meshes = {
        model.offset: build_model_mesh(
            vehicle,
            data,
            model,
            set(texture_by_offset),
            images,
            texture_by_offset,
            material_cache,
        )
        for model in member_models
    }

    assembly = bpy.data.collections.new("VEHICLE_ASSEMBLY")
    scene.collection.children.link(assembly)
    object_records = []
    for instance in instances:
        model = instance["model"]
        assert isinstance(model, Model3)
        name = f"{instance['part_name']}__Model3_{model.offset:08X}"
        obj = link_object(name, meshes[model.offset], assembly, instance["matrix"])
        obj["part_name"] = str(instance["part_name"])
        obj["part_name_confidence"] = str(instance["part_name_confidence"])
        obj["part_name_basis"] = str(instance["part_name_basis"])
        obj["source_node_path"] = str(instance["path"])
        obj["source_model_offset"] = model.offset
        object_records.append(
            {
                "name": name,
                "part_name": instance["part_name"],
                "part_name_confidence": instance["part_name_confidence"],
                "source_node_path": instance["path"],
                "model_offset": model.offset,
                "vertex_count": model.vertex_count,
                "triangle_count": model.triangle_count,
            }
        )

    lod1_object_records = []
    if lod1_instances:
        lod1_collection = bpy.data.collections.new("XBOX360_DAMAGE_ASSEMBLY")
        scene.collection.children.link(lod1_collection)
        lod1_collection.hide_render = True
        lod1_collection.hide_viewport = True
        lod1_collection["source_array_offset"] = int(
            lod1_assembly["array_offset"]
        )
        lod1_collection["source_seed_model_offset"] = int(
            lod1_assembly["seed_model_offset"]
        )
        lod1_collection["classification"] = (
            "authored Xbox 360 destroyed/wreck assembly"
        )
        for instance in lod1_instances:
            model = instance["model"]
            assert isinstance(model, Model3)
            name = (
                f"damage_{instance['part_name']}__Model3_{model.offset:08X}"
            )
            obj = link_object(
                name, meshes[model.offset], lod1_collection, instance["matrix"]
            )
            obj["part_name"] = str(instance["part_name"])
            obj["part_name_confidence"] = str(
                instance["part_name_confidence"]
            )
            obj["part_name_basis"] = str(instance["part_name_basis"])
            obj["source_node_path"] = str(instance["path"])
            obj["source_model_offset"] = model.offset
            obj["damage_state_model"] = True
            lod1_object_records.append(
                {
                    "name": name,
                    "part_name": instance["part_name"],
                    "part_name_confidence": instance[
                        "part_name_confidence"
                    ],
                    "source_node_path": instance["path"],
                    "model_offset": model.offset,
                    "vertex_count": model.vertex_count,
                    "triangle_count": model.triangle_count,
                }
            )

        if lod1_unknown:
            lod1_unresolved = bpy.data.collections.new(
                "XBOX360_DAMAGE_UNRESOLVED_OBJECTS"
            )
            scene.collection.children.link(lod1_unresolved)
            lod1_unresolved.hide_render = True
            lod1_unresolved.hide_viewport = True
            for record in lod1_unknown:
                empty = bpy.data.objects.new(
                    (
                        f"damage_unresolved_{record['path']}"
                        f"__offset_{int(record['offset']):08X}"
                    ),
                    None,
                )
                lod1_unresolved.objects.link(empty)
                empty.matrix_world = record["matrix"]
                empty["source_object_offset"] = int(record["offset"])
                empty["source_node_path"] = str(record["path"])

    unresolved_collection = bpy.data.collections.new(
        "UNRESOLVED_NON_MODEL_SCENE_OBJECTS"
    )
    scene.collection.children.link(unresolved_collection)
    for record in unknown:
        empty = bpy.data.objects.new(
            f"unresolved_{record['path']}__offset_{int(record['offset']):08X}",
            None,
        )
        unresolved_collection.objects.link(empty)
        empty.matrix_world = record["matrix"]
        empty["source_object_offset"] = int(record["offset"])
        empty["source_node_path"] = str(record["path"])
    unresolved_collection.hide_render = True

    blend_path = output_root / vehicle / f"{stem}.blend"
    blend_path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path), compress=True)
    print(
        f"{stem}: {len(instances)} model instances, {triangles} triangles, "
        f"{len(texture_offsets)} textures -> {blend_path}"
    )
    return {
        "file": str(blend_path.resolve()),
        "version_kind": version_kind,
        "scene_index": index,
        "skin_color_rgb": colour,
        "model_instance_count": len(instances),
        "unique_model_count": len(member_models),
        "triangle_count": triangles,
        "texture_count": len(texture_offsets),
        "unresolved_non_model_count": len(unknown),
        "objects": object_records,
        "damage_assembly": (
            {
                "source_array_offset": int(lod1_assembly["array_offset"]),
                "seed_model_offset": int(lod1_assembly["seed_model_offset"]),
                "model_instance_count": len(lod1_instances),
                "unique_model_count": len(
                    {
                        int(item["model_offset"])
                        for item in lod1_object_records
                    }
                ),
                "triangle_count": sum(
                    int(item["triangle_count"])
                    for item in lod1_object_records
                ),
                "unresolved_non_model_count": len(lod1_unknown),
                "objects": lod1_object_records,
            }
            if lod1_assembly is not None
            else None
        ),
    }


def export_auxiliary_models(
    path: Path,
    output_root: Path,
    data: bytes,
    image_limit: int,
    all_models: list[Model3],
    referenced_offsets: set[int],
    texture_by_offset: dict[int, dict[str, object]],
) -> dict[str, object] | None:
    auxiliary_models = [
        model for model in all_models if model.offset not in referenced_offsets
    ]
    if not auxiliary_models:
        return None
    vehicle = path.stem
    stem = f"{vehicle}_auxiliary_unresolved"
    scene = prepare_scene(
        stem,
        path,
        image_limit,
        len(all_models),
        "auxiliary_unresolved_models",
    )
    scene["note"] = (
        "Valid Model3 allocations not referenced by any authored skin/classic "
        "scene. Retained for damage, collision, shadow, or detachable-part research."
    )
    texture_offsets = referenced_texture_offsets(
        data, auxiliary_models, texture_by_offset, image_limit
    )
    images = load_images(
        vehicle,
        data,
        texture_offsets,
        texture_by_offset,
        output_root / "textures" / vehicle,
    )
    material_cache = {}
    collection = bpy.data.collections.new("AUXILIARY_UNRESOLVED_MODELS")
    scene.collection.children.link(collection)
    for index, model in enumerate(auxiliary_models):
        mesh = build_model_mesh(
            vehicle,
            data,
            model,
            set(texture_by_offset),
            images,
            texture_by_offset,
            material_cache,
        )
        obj = link_object(
            f"unresolved_model_{index + 1:03d}__Model3_{model.offset:08X}",
            mesh,
            collection,
        )
        obj["part_name"] = "unresolved_auxiliary_model"
        obj["part_name_confidence"] = "UNKNOWN"
        obj["source_model_offset"] = model.offset

    blend_path = output_root / vehicle / f"{stem}.blend"
    blend_path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path), compress=True)
    print(
        f"{stem}: {len(auxiliary_models)} models, {len(texture_offsets)} "
        f"textures -> {blend_path}"
    )
    return {
        "file": str(blend_path.resolve()),
        "version_kind": "auxiliary_unresolved_models",
        "unique_model_count": len(auxiliary_models),
        "texture_count": len(texture_offsets),
        "model_offsets": [model.offset for model in auxiliary_models],
    }


def export_bundle(path: Path, output_root: Path) -> list[Path]:
    data, _ = decode_bundle(path)
    vehicle = path.stem
    image_limit = u32le(data, 8)
    models = find_models(data)
    model_by_offset = {model.offset: model for model in models}
    texture_records = xbox360_texture_candidates(data, image_limit, image_limit)
    texture_by_offset = {int(record["offset"]): record for record in texture_records}
    scenes = scene_roots(data, image_limit)
    native_scene_records = []
    for scene_record in scenes:
        scene_instances, _ = collect_scene_instances(
            data,
            int(scene_record["root_offset"]),
            image_limit,
            model_by_offset,
        )
        scene_triangles = sum(
            instance["model"].triangle_count for instance in scene_instances
        )
        is_classic = (
            int(scene_record["index"]) == len(scenes) - 1
            and 0 < scene_triangles <= 1000
        )
        if not is_classic:
            native_scene_records.append(scene_record)

    lod1_assemblies = []
    seed_model_offset = XBOX360_DAMAGE_BODY_SEED_MODELS.get(vehicle)
    if seed_model_offset is not None:
        lod1_assemblies = recover_damage_assemblies(
            data, image_limit, model_by_offset, seed_model_offset
        )
        for assembly in lod1_assemblies:
            preceding = [
                record
                for record in native_scene_records
                if int(record["root_offset"])
                < int(assembly["array_offset"])
            ]
            assembly["owner_scene_index"] = (
                int(preceding[-1]["index"]) if preceding else 0
            )

    records = []
    referenced_offsets: set[int] = set()
    for scene_record in scenes:
        scene_index = int(scene_record["index"])
        eligible_lod1 = [
            assembly
            for assembly in lod1_assemblies
            if int(assembly["owner_scene_index"]) <= scene_index
        ]
        lod1_assembly = eligible_lod1[-1] if eligible_lod1 else None
        if scene_record not in native_scene_records:
            lod1_assembly = None
        record = export_scene_version(
            path,
            output_root,
            data,
            image_limit,
            models,
            model_by_offset,
            texture_by_offset,
            scene_record,
            len(scenes),
            lod1_assembly,
        )
        records.append(record)
        referenced_offsets.update(
            int(item["model_offset"]) for item in record["objects"]
        )
        if record["damage_assembly"] is not None:
            referenced_offsets.update(
                int(item["model_offset"])
                for item in record["damage_assembly"]["objects"]
            )
    auxiliary = export_auxiliary_models(
        path,
        output_root,
        data,
        image_limit,
        models,
        referenced_offsets,
        texture_by_offset,
    )
    if auxiliary is not None:
        records.append(auxiliary)
    manifest_path = output_root / vehicle / "manifest.json"
    manifest_path.write_text(
        json.dumps(
            {
                "vehicle": vehicle,
                "source": str(path.resolve()),
                "discovered_model_count": len(models),
                "authored_scene_count": len(scenes),
                "interpretation": (
                    "Native scenes 0-4 are full-resolution selectable skin "
                    "variants; their geometry counts match and paintColor0 "
                    "matches the scene-table RGB. Serialized secondary part "
                    "arrays containing the destroyed native body are exported "
                    "as XBOX360_DAMAGE_ASSEMBLY in each applicable skin. "
                    "The compact final scene is the PS1 classic where present."
                ),
                "damage_assembly_count": len(lod1_assemblies),
                "versions": records,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    return [Path(record["file"]) for record in records]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path)
    parser.add_argument("--out", required=True, type=Path)
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None
    args = parser.parse_args(argv)
    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    for path in args.inputs:
        export_bundle(path, output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
