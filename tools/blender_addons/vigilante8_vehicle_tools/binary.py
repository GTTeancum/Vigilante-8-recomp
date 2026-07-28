"""Small, dependency-free binary helpers used by the Blender add-on.

The module deliberately avoids ``bpy`` so format tests can run under the
repository's ordinary Python interpreter.
"""

from __future__ import annotations

import struct


class FormatError(ValueError):
    """Raised when a retail asset violates a required format invariant."""


def require_range(data: bytes | bytearray | memoryview, offset: int, size: int) -> None:
    if offset < 0 or size < 0 or offset > len(data) - size:
        raise FormatError(
            f"range 0x{offset:X}+0x{size:X} exceeds buffer size 0x{len(data):X}"
        )


def u8(data: bytes | bytearray | memoryview, offset: int) -> int:
    require_range(data, offset, 1)
    return data[offset]


def i8(data: bytes | bytearray | memoryview, offset: int) -> int:
    require_range(data, offset, 1)
    return struct.unpack_from("<b", data, offset)[0]


def u16le(data: bytes | bytearray | memoryview, offset: int) -> int:
    require_range(data, offset, 2)
    return struct.unpack_from("<H", data, offset)[0]


def i16le(data: bytes | bytearray | memoryview, offset: int) -> int:
    require_range(data, offset, 2)
    return struct.unpack_from("<h", data, offset)[0]


def u32le(data: bytes | bytearray | memoryview, offset: int) -> int:
    require_range(data, offset, 4)
    return struct.unpack_from("<I", data, offset)[0]


def i32le(data: bytes | bytearray | memoryview, offset: int) -> int:
    require_range(data, offset, 4)
    return struct.unpack_from("<i", data, offset)[0]


def u32be(data: bytes | bytearray | memoryview, offset: int) -> int:
    require_range(data, offset, 4)
    return struct.unpack_from(">I", data, offset)[0]


def put_u8(data: bytearray, offset: int, value: int) -> None:
    require_range(data, offset, 1)
    struct.pack_into("<B", data, offset, value)


def put_i8(data: bytearray, offset: int, value: int) -> None:
    require_range(data, offset, 1)
    struct.pack_into("<b", data, offset, value)


def put_u16le(data: bytearray, offset: int, value: int) -> None:
    require_range(data, offset, 2)
    struct.pack_into("<H", data, offset, value)


def put_i16le(data: bytearray, offset: int, value: int) -> None:
    require_range(data, offset, 2)
    struct.pack_into("<h", data, offset, value)


def put_u32le(data: bytearray, offset: int, value: int) -> None:
    require_range(data, offset, 4)
    struct.pack_into("<I", data, offset, value)


def put_i32le(data: bytearray, offset: int, value: int) -> None:
    require_range(data, offset, 4)
    struct.pack_into("<i", data, offset, value)

