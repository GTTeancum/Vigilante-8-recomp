#!/usr/bin/env python3
"""Build append-only V8:2 location-selector tables from native TIM records.

The retail table contains one CI8 TIM per selectable location followed by one
VLC-compressed selector background.  Mod previews are inserted immediately
before that final background so the stock selector indexes them normally.
"""

from __future__ import annotations

import struct
from pathlib import Path
from typing import Sequence


def split_resource_table(data: bytes) -> list[bytes]:
    if len(data) < 12:
        raise ValueError("resource table is truncated")
    count = struct.unpack_from("<I", data, 0)[0]
    header_size = 4 + (count + 1) * 4
    if count < 2 or header_size > len(data):
        raise ValueError("resource table count is invalid")
    offsets = struct.unpack_from(f"<{count + 1}I", data, 4)
    if offsets[0] < header_size or offsets[-1] != len(data):
        raise ValueError("resource table offsets do not span the file")
    if any(left > right for left, right in zip(offsets, offsets[1:])):
        raise ValueError("resource table offsets are not monotonic")
    return [data[offsets[i]:offsets[i + 1]] for i in range(count)]


def build_resource_table(resources: Sequence[bytes]) -> bytes:
    if not resources:
        raise ValueError("resource table cannot be empty")
    count = len(resources)
    cursor = 4 + (count + 1) * 4
    offsets = []
    payload = bytearray()
    for resource in resources:
        while cursor & 3:
            payload.append(0)
            cursor += 1
        offsets.append(cursor)
        payload += resource
        cursor += len(resource)
    while cursor & 3:
        payload.append(0)
        cursor += 1
    offsets.append(cursor)
    return (
        struct.pack("<I", count)
        + struct.pack(f"<{count + 1}I", *offsets)
        + bytes(payload)
    )


def tim_dimensions(payload: bytes) -> tuple[int, int]:
    if len(payload) < 32 or struct.unpack_from("<I", payload, 0)[0] != 0x10:
        raise ValueError("selector resource is not a TIM")
    flags = struct.unpack_from("<I", payload, 4)[0]
    if flags != 9:
        raise ValueError(f"selector TIM is not CI8 (flags=0x{flags:X})")
    clut_size = struct.unpack_from("<I", payload, 8)[0]
    image_offset = 8 + clut_size
    if image_offset + 12 > len(payload):
        raise ValueError("selector TIM image block is truncated")
    width_words, height = struct.unpack_from("<HH", payload, image_offset + 8)
    return width_words * 2, height


def encode_ci8_tim(
    indices: bytes,
    width: int,
    height: int,
    palette_bgr555: Sequence[int],
    template: bytes,
) -> bytes:
    """Encode a CI8 preview while borrowing only native VRAM origins."""

    if width <= 0 or height <= 0 or width & 1:
        raise ValueError("CI8 dimensions must be positive with even width")
    if len(indices) != width * height:
        raise ValueError("CI8 index payload has the wrong size")
    if not palette_bgr555 or len(palette_bgr555) > 256:
        raise ValueError("CI8 palette must contain 1..256 colors")
    if indices and max(indices) >= len(palette_bgr555):
        raise ValueError("CI8 indices reference a missing palette entry")
    if tim_dimensions(template) != (220, 74):
        raise ValueError("selector template is not a native 220x74 preview")

    template_clut_size = struct.unpack_from("<I", template, 8)[0]
    template_image = 8 + template_clut_size
    clut_x, clut_y = struct.unpack_from("<HH", template, 12)
    image_x, image_y = struct.unpack_from("<HH", template, template_image + 4)

    palette = list(palette_bgr555)
    if len(palette) < 256:
        palette += [0] * (256 - len(palette))
    clut_size = 12 + 256 * 2
    image_size = 12 + len(indices)
    return b"".join(
        (
            struct.pack("<II", 0x10, 9),
            struct.pack("<IHHHH", clut_size, clut_x, clut_y, 256, 1),
            struct.pack("<256H", *palette),
            struct.pack(
                "<IHHHH", image_size, image_x, image_y, width // 2, height
            ),
            indices,
        )
    )


def apply_ci8_template_mask(
    indices: bytes,
    width: int,
    height: int,
    palette_bgr555: Sequence[int],
    template: bytes,
) -> tuple[bytes, tuple[int, ...]]:
    """Apply a stock preview's transparent silhouette to converted pixels."""

    if len(indices) != width * height:
        raise ValueError("CI8 source indices have the wrong size")
    if tim_dimensions(template) != (width, height):
        raise ValueError("selector mask template dimensions do not match")

    clut_size = struct.unpack_from("<I", template, 8)[0]
    clut_width = struct.unpack_from("<H", template, 16)[0]
    if clut_width <= 0 or clut_width > 256:
        raise ValueError("selector mask template has an invalid palette")
    template_palette = struct.unpack_from(
        f"<{clut_width}H", template, 20
    )
    image_offset = 8 + clut_size
    template_indices = template[
        image_offset + 12:image_offset + 12 + width * height
    ]
    if len(template_indices) != width * height:
        raise ValueError("selector mask template pixels are truncated")

    palette = list(palette_bgr555)
    if len(palette) > 256:
        raise ValueError("CI8 source palette exceeds 256 colors")
    palette += [0] * (256 - len(palette))
    used = set(indices)
    transparent_index = next(
        (index for index in range(256) if index not in used),
        None,
    )
    if transparent_index is None:
        raise ValueError(
            "CI8 source uses all 256 palette entries; no transparent mask "
            "entry is available"
        )
    palette[transparent_index] = 0

    masked = bytearray(indices)
    for offset, template_index in enumerate(template_indices):
        if template_palette[template_index] == 0:
            masked[offset] = transparent_index
    return bytes(masked), tuple(palette)


def append_location_preview(
    retail_table: bytes,
    preview_tim: bytes,
    *,
    expected_retail_locations: int = 18,
) -> bytes:
    resources = split_resource_table(retail_table)
    if len(resources) != expected_retail_locations + 1:
        raise ValueError(
            f"expected {expected_retail_locations} previews plus background, "
            f"found {len(resources)} resources"
        )
    for index, resource in enumerate(resources[:-1]):
        if tim_dimensions(resource) != (220, 74):
            raise ValueError(f"retail preview {index} is not 220x74")
    if tim_dimensions(preview_tim) != (220, 74):
        raise ValueError("mod preview is not 220x74")
    # The final resource is the native 640x421 VLC selector background.  It is
    # deliberately retained byte-for-byte and remains last after extension.
    return build_resource_table([*resources[:-1], preview_tim, resources[-1]])


def write_extended_table(
    retail_path: Path,
    output_path: Path,
    preview_tim: bytes,
) -> bytes:
    result = append_location_preview(retail_path.read_bytes(), preview_tim)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_bytes(result)
    return result
