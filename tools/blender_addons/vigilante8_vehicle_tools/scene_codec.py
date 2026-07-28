"""Blender-independent coordinate and color conversion helpers.

Keeping these functions outside ``blender_ui`` lets the native conversion
rules be regression-tested without requiring a Blender installation.
"""

from __future__ import annotations

import base64
from collections import Counter
import math
from typing import Iterable, Sequence

from .xobf import TextureSlot, Vertex


PSX_TURN = 4096.0
PSX_ANGLE_RADIANS = math.tau / PSX_TURN
SLOT_POSITION_SCALE = 65536.0


def native_vertex_to_blender(
    vertex: Vertex | Sequence[int], scale_shift: int
) -> tuple[float, float, float]:
    """Map native X/Y/Z to Blender's X/Y/Z-up coordinate system."""

    x, y, z = (vertex.x, vertex.y, vertex.z) if isinstance(vertex, Vertex) else vertex[:3]
    scale = 1.0 / float(1 << scale_shift)
    return float(x) * scale, float(z) * scale, -float(y) * scale


def blender_vertex_to_native(
    coordinate: Sequence[float], scale_shift: int, pad: int = 0
) -> tuple[int, int, int, int]:
    if len(coordinate) < 3:
        raise ValueError("a Blender vertex needs three components")
    scale = float(1 << scale_shift)
    result = (
        round(float(coordinate[0]) * scale),
        round(-float(coordinate[2]) * scale),
        round(float(coordinate[1]) * scale),
        int(pad),
    )
    if any(value < -32768 or value > 32767 for value in result):
        raise ValueError("edited vertex exceeds the native signed 16-bit range")
    return result


def native_position_to_blender(
    position: Sequence[int],
) -> tuple[float, float, float]:
    if len(position) != 3:
        raise ValueError("native position needs three components")
    return (
        float(position[0]) / SLOT_POSITION_SCALE,
        float(position[2]) / SLOT_POSITION_SCALE,
        -float(position[1]) / SLOT_POSITION_SCALE,
    )


def blender_position_to_native(
    position: Sequence[float],
) -> tuple[int, int, int]:
    if len(position) != 3:
        raise ValueError("Blender position needs three components")
    result = (
        round(float(position[0]) * SLOT_POSITION_SCALE),
        round(-float(position[2]) * SLOT_POSITION_SCALE),
        round(float(position[1]) * SLOT_POSITION_SCALE),
    )
    if any(value < -0x80000000 or value > 0x7FFFFFFF for value in result):
        raise ValueError("edited slot position exceeds the native signed 32-bit range")
    return result


def native_rotation_to_blender(
    rotation_yxz: Sequence[int],
) -> tuple[float, float, float]:
    """Return Blender XYZ Euler components for a native Y/X/Z triplet."""

    if len(rotation_yxz) != 3:
        raise ValueError("native rotation needs Y, X, and Z components")
    rot_y, rot_x, rot_z = rotation_yxz
    return (
        float(rot_x) * PSX_ANGLE_RADIANS,
        float(rot_z) * PSX_ANGLE_RADIANS,
        -float(rot_y) * PSX_ANGLE_RADIANS,
    )


def blender_rotation_to_native(
    euler_xyz: Sequence[float],
) -> tuple[int, int, int]:
    if len(euler_xyz) != 3:
        raise ValueError("Blender rotation needs three Euler components")
    result = (
        round(-float(euler_xyz[2]) / PSX_ANGLE_RADIANS),
        round(float(euler_xyz[0]) / PSX_ANGLE_RADIANS),
        round(float(euler_xyz[1]) / PSX_ANGLE_RADIANS),
    )
    # Native angle fields are signed i16, and values outside one turn are
    # equivalent. Normalize before range validation so ordinary Blender Euler
    # edits never overflow solely because the curve crossed +/-2pi.
    return tuple(((value + 32768) & 0xFFFF) - 32768 for value in result)


def psx_bgr555_to_rgba(color: int) -> tuple[float, float, float, float]:
    red = (color & 0x1F) / 31.0
    green = ((color >> 5) & 0x1F) / 31.0
    blue = ((color >> 10) & 0x1F) / 31.0
    alpha = 0.0 if (color & 0x7FFF) == 0 else 1.0
    return red, green, blue, alpha


def texture_rgba(texture: TextureSlot) -> list[float]:
    if not texture.supported:
        raise ValueError("unsupported native texture cannot be displayed")
    result: list[float] = []
    if texture.depth == 2:
        for color in texture.direct_pixels_bgr555:
            result.extend(psx_bgr555_to_rgba(color))
        return result
    transparent = (0.0, 0.0, 0.0, 0.0)
    for index in texture.indices:
        result.extend(
            psx_bgr555_to_rgba(texture.palette[index])
            if index < len(texture.palette)
            else transparent
        )
    return result


def rgba_to_native_palette(
    rgba: Sequence[float], palette_size: int
) -> tuple[tuple[int, ...], bytes]:
    """Quantize RGBA floats to a fixed-size PSX BGR555 palette."""

    if len(rgba) % 4:
        raise ValueError("RGBA data must contain four components per pixel")
    if palette_size < 1 or palette_size > 256:
        raise ValueError("native palettes contain 1..256 entries")
    colors = []
    for offset in range(0, len(rgba), 4):
        red, green, blue, alpha = (
            max(0.0, min(1.0, float(value)))
            for value in rgba[offset : offset + 4]
        )
        if alpha < 0.5:
            color = 0
        else:
            color = (
                round(red * 31.0)
                | (round(green * 31.0) << 5)
                | (round(blue * 31.0) << 10)
            )
        colors.append(color)
    counts = Counter(colors)
    palette = [color for color, _count in counts.most_common(palette_size)]
    palette.extend([0] * (palette_size - len(palette)))

    def distance(first: int, second: int) -> int:
        if first == 0 or second == 0:
            return 0 if first == second else 0x10000
        return sum(
            (
                ((first >> shift) & 0x1F)
                - ((second >> shift) & 0x1F)
            )
            ** 2
            for shift in (0, 5, 10)
        )

    exact = {}
    for index, color in enumerate(palette):
        exact.setdefault(color, index)
    indices = bytes(
        exact[color]
        if color in exact
        else min(range(palette_size), key=lambda index: distance(color, palette[index]))
        for color in colors
    )
    return tuple(palette), indices


def encode_source(data: bytes) -> str:
    return base64.b64encode(data).decode("ascii")


def decode_source(text: str) -> bytes:
    try:
        return base64.b64decode("".join(text.split()), validate=True)
    except ValueError as error:
        raise ValueError("embedded native source data is not valid base64") from error


def triangulate_faces(
    faces: Iterable[Sequence[int]],
) -> tuple[tuple[int, int, int], ...]:
    """Fan-triangulate polygon index sequences deterministically."""

    result = []
    for face in faces:
        indices = tuple(int(value) for value in face)
        if len(indices) < 3:
            continue
        for corner in range(1, len(indices) - 1):
            result.append((indices[0], indices[corner], indices[corner + 1]))
    return tuple(result)
