#!/usr/bin/env python3
"""Decode and semantically convert Vigilante 8 N64 arenas.

The N64 port retained the original EA-IFF level organization but changed the
platform-facing model, texture, animation, and navigation encodings.  This
module converts those structures into the native little-endian XOBF and PSX
image records consumed by the original PlayStation game.  It never embeds ROM
regions or compressed source payloads in its output.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import math
import os
import struct
from typing import Iterable, Iterator


class FormatError(ValueError):
    pass


def align(value: int, boundary: int = 4) -> int:
    return (value + boundary - 1) & -boundary


def be16(data: bytes, offset: int, *, signed: bool = False) -> int:
    return int.from_bytes(data[offset:offset + 2], "big", signed=signed)


def be32(data: bytes, offset: int, *, signed: bool = False) -> int:
    return int.from_bytes(data[offset:offset + 4], "big", signed=signed)


def iff_chunk(tag: bytes, payload: bytes) -> bytes:
    if len(tag) != 4:
        raise ValueError(tag)
    result = tag + struct.pack(">I", len(payload)) + payload
    return result + (b"\0" if len(payload) & 1 else b"")


def iff_form(form_type: bytes, children: Iterable[bytes]) -> bytes:
    return iff_chunk(b"FORM", form_type + b"".join(children))


@dataclass(frozen=True)
class Chunk:
    tag: bytes
    payload: bytes
    form_type: bytes | None = None

    @property
    def is_form(self) -> bool:
        return self.form_type is not None

    def encode(self) -> bytes:
        if self.is_form:
            return iff_form(self.form_type or b"????", [self.payload])
        return iff_chunk(self.tag, self.payload)


def iter_chunks(
    data: bytes, start: int = 0, end: int | None = None, parent: bytes = b""
) -> Iterator[tuple[int, bytes, bytes, bytes]]:
    if end is None:
        end = len(data)
    cursor = start
    while cursor + 8 <= end:
        tag = data[cursor:cursor + 4]
        size = be32(data, cursor + 4)
        body = cursor + 8
        if body + size > end:
            raise FormatError(
                f"{tag!r} at 0x{cursor:X} crosses parent boundary 0x{end:X}"
            )
        if tag == b"FORM":
            if size < 4:
                raise FormatError("FORM is missing its type")
            form_type = data[body:body + 4]
            payload = data[body + 4:body + size]
            yield cursor, form_type, payload, parent
            yield from iter_chunks(data, body + 4, body + size, form_type)
        else:
            yield cursor, tag, data[body:body + size], parent
        cursor = body + ((size + 1) & ~1)


def form_children(data: bytes, expected_type: bytes | None = None) -> list[Chunk]:
    if data[:4] != b"FORM" or len(data) < 12:
        raise FormatError("data is not an IFF FORM")
    form_type = data[8:12]
    if expected_type is not None and form_type != expected_type:
        raise FormatError(
            f"expected FORM {expected_type!r}, got FORM {form_type!r}"
        )
    end = 8 + be32(data, 4)
    result: list[Chunk] = []
    cursor = 12
    while cursor + 8 <= end:
        tag = data[cursor:cursor + 4]
        size = be32(data, cursor + 4)
        body = cursor + 8
        if body + size > end:
            raise FormatError(f"truncated root chunk {tag!r}")
        if tag == b"FORM":
            result.append(
                Chunk(tag=b"FORM", form_type=data[body:body + 4],
                      payload=data[body + 4:body + size])
            )
        else:
            result.append(Chunk(tag=tag, payload=data[body:body + size]))
        cursor = body + ((size + 1) & ~1)
    return result


def root_children(data: bytes) -> list[Chunk]:
    return form_children(data, b"TERR")


class V8N64Rom:
    """Normalized, read-only view of a retail V8 N64 ROM."""

    def __init__(self, path: str | Path):
        self.path = Path(path)
        source = self.path.read_bytes()
        if len(source) < 0x40:
            raise FormatError("ROM is too small")
        magic = source[:4]
        if magic == b"\x80\x37\x12\x40":
            normalized = source
        elif magic == b"\x37\x80\x40\x12":
            normalized = bytearray(source)
            for offset in range(0, len(normalized), 2):
                normalized[offset], normalized[offset + 1] = (
                    normalized[offset + 1], normalized[offset]
                )
            normalized = bytes(normalized)
        elif magic == b"\x40\x12\x37\x80":
            normalized = bytearray(source)
            for offset in range(0, len(normalized), 4):
                normalized[offset:offset + 4] = normalized[offset:offset + 4][::-1]
            normalized = bytes(normalized)
        else:
            raise FormatError(f"unsupported N64 ROM byte order {magic.hex()}")
        if normalized[0x3B:0x3F] != b"NV8E":
            raise FormatError(
                f"expected US Vigilante 8 game code NV8E, got "
                f"{normalized[0x3B:0x3F]!r}"
            )
        self.data = normalized

    def locate(self, name: str) -> tuple[int, int]:
        encoded = name.upper().encode("ascii")
        if len(encoded) > 12:
            raise ValueError("N64 archive names are limited to twelve bytes")
        needle = encoded.ljust(12, b" ")
        cursor = 0
        while True:
            cursor = self.data.find(needle, cursor)
            if cursor < 0:
                raise FileNotFoundError(name)
            if cursor + 20 <= len(self.data):
                address = be32(self.data, cursor + 12)
                size = be32(self.data, cursor + 16)
                rom_offset = address & 0x0FFFFFFF
                if (
                    address & 0xF0000000 == 0xB0000000
                    and size > 0
                    and rom_offset + size <= len(self.data)
                ):
                    return rom_offset, size
            cursor += 1

    def compressed(self, name: str) -> bytes:
        offset, size = self.locate(name)
        return self.data[offset:offset + size]

    def decoded(self, name: str) -> bytes:
        return decode_lzss(self.compressed(name))


def decode_lzss(data: bytes) -> bytes:
    """Decode the exact 2 KiB-window stream used by the N64 game."""

    if data[:4] != b"LZSS" or len(data) < 9:
        raise FormatError("asset is not an LZSS stream")
    output_size = int.from_bytes(data[4:8], "little")
    source = 8
    output = bytearray()
    window = bytearray(0x800)
    write_position = 0
    control = 0
    mask = 0
    copy_position = 0
    copy_remaining = 0
    while len(output) < output_size:
        if copy_remaining == 0:
            mask >>= 1
            if mask == 0:
                if source >= len(data):
                    raise FormatError("truncated LZSS control byte")
                mask = 0x80
                control = data[source]
                source += 1
            if control & mask:
                if source >= len(data):
                    raise FormatError("truncated LZSS literal")
                value = data[source]
                source += 1
            else:
                if source + 2 > len(data):
                    raise FormatError("truncated LZSS reference")
                token = int.from_bytes(data[source:source + 2], "little")
                source += 2
                copy_position = token >> 5
                copy_remaining = (token & 0x1F) + 2
        if copy_remaining:
            value = window[copy_position]
            copy_position = (copy_position + 1) & 0x7FF
            copy_remaining -= 1
        output.append(value)
        window[write_position] = value
        write_position = (write_position + 1) & 0x7FF
    if source != len(data):
        raise FormatError(
            f"LZSS decoded at 0x{source:X}, stream has {len(data) - source} trailing bytes"
        )
    return bytes(output)


def relocate_overlay_image(data: bytes, base: int) -> bytes:
    """Apply the native V8 overlay relocation stream to an image copy.

    V8 stores the unrelocated image first, followed at ``image_size`` by a
    word-oriented relocation list terminated by ``0xffffffff``.  The low two
    bits select the MIPS relocation form and the remaining bits identify the
    byte offset (the target itself is always word aligned):

    ``0`` absolute 32-bit, ``1`` high 16-bit, ``2`` low 16-bit, ``3`` J26.

    The routine is intentionally useful for analysis only; it returns a
    relocated image without retaining the relocation stream.
    """

    if len(data) < 8:
        raise FormatError("overlay is truncated")
    image_size = be32(data, 0)
    if image_size < 8 or image_size > len(data):
        raise FormatError(f"invalid overlay image size 0x{image_size:X}")
    image = bytearray(data[:image_size])
    cursor = image_size
    relocated_words: set[int] = set()

    def relocate_absolute(offset: int) -> None:
        if offset in relocated_words:
            return
        if offset + 4 > len(image):
            raise FormatError(
                f"overlay absolute relocation 0x{offset:X} is outside the image"
            )
        value = (be32(image, offset) + base) & 0xFFFFFFFF
        struct.pack_into(">I", image, offset, value)
        relocated_words.add(offset)

    while cursor + 4 <= len(data):
        relocation = be32(data, cursor)
        cursor += 4
        if relocation == 0xFFFFFFFF:
            return bytes(image)
        kind = relocation & 3
        offset = relocation & ~3
        if offset + 4 > len(image):
            raise FormatError(
                f"overlay relocation 0x{relocation:X} is outside the image"
            )
        word = be32(image, offset)
        if kind == 0:
            relocate_absolute(offset)
            continue
        elif kind == 1:
            if cursor + 4 > len(data):
                raise FormatError("truncated overlay HI16 relocation addend")
            # HI16 and LO16 entries are followed by the image-relative
            # symbol/addend they reference.  It is relocation metadata, not
            # another absolute-word relocation (and may therefore repeat).
            addend_offset = be32(data, cursor)
            cursor += 4
            relocate_absolute(addend_offset)
            # The compiler emits the standard carry-adjusted %hi value.
            word = (word & 0xFFFF0000) | (
                ((word & 0xFFFF) + ((base + 0x8000) >> 16)) & 0xFFFF
            )
        elif kind == 2:
            word = (word & 0xFFFF0000) | (
                ((word & 0xFFFF) + base) & 0xFFFF
            )
        else:
            word = (word & 0xFC000000) | (
                ((word & 0x03FFFFFF) + (base >> 2)) & 0x03FFFFFF
            )
        struct.pack_into(">I", image, offset, word)
    raise FormatError("overlay relocation stream has no terminator")


@dataclass(frozen=True)
class N64Texture:
    index: int
    source_offset: int
    format: int
    size: int
    palette_count: int
    width: int
    height: int
    palette_rgba5551: tuple[int, ...]
    pixels: bytes


def rgba5551_to_psx(
    value: int, *, alpha_zero_transparent: bool = False
) -> int:
    red = (value >> 11) & 0x1F
    green = (value >> 6) & 0x1F
    blue = (value >> 1) & 0x1F
    alpha = value & 1
    # Opaque N64 combiners ignore the palette alpha bit, and Dreamland's
    # terrain atlas legitimately stores visible colors with alpha zero.
    # Translucent passes receive a distinct PSX texture variant where those
    # entries become native transparent color word zero.
    if alpha_zero_transparent and alpha == 0:
        return 0
    if alpha == 0 and red == 0 and green == 0 and blue == 0:
        return 0
    # V8's PSX texture loader uses STP as its "occupied palette entry" marker
    # while generating the 17 lighting CLUTs.  Retail terrain palettes set it
    # on every visible color, even though the matching N64 RGBA5551 palettes
    # leave alpha clear.  Omitting STP therefore collapses most colors to the
    # ambient fallback during level load.
    return red | (green << 5) | (blue << 10) | 0x8000


def parse_n64_texture(
    data: bytes, offset: int, index: int, end: int
) -> N64Texture:
    if offset + 8 > end:
        raise FormatError(f"texture {index} has an invalid header")
    image_format = data[offset]
    image_size = data[offset + 1]
    palette_count = be16(data, offset + 2)
    width = be16(data, offset + 4)
    height = be16(data, offset + 6)
    if (
        image_format not in (0, 2)
        or image_size not in (0, 1, 2, 3)
        or width <= 0
        or height <= 0
        or palette_count > 256
        or (image_format == 2 and image_size == 0 and palette_count == 0)
    ):
        raise FormatError(
            f"texture {index} has invalid format={image_format} size={image_size} "
            f"{palette_count=} {width=} {height=}"
        )
    palette_end = offset + align(8 + palette_count * 2, 8)
    bits_per_texel = (4, 8, 16, 32)[image_size]
    row_bytes = (width * bits_per_texel + 7) // 8
    # RDP texture images are laid out in 64-bit TMEM scanline units.  The
    # archive retains that per-row padding.  Treating the image as one tightly
    # packed width*height bitstream makes row zero look right but advances
    # every later row into the alignment bytes.
    row_stride = align(row_bytes, 8)
    pixel_size = row_stride * height
    pixel_end = palette_end + pixel_size
    if pixel_end > end:
        raise FormatError(f"texture {index} pixel data is truncated")
    palette = tuple(
        be16(data, offset + 8 + color * 2) for color in range(palette_count)
    )
    return N64Texture(
        index=index,
        source_offset=offset,
        format=image_format,
        size=image_size,
        palette_count=palette_count,
        width=width,
        height=height,
        palette_rgba5551=palette,
        pixels=_decode_n64_texture_rows(
            data[palette_end:pixel_end], row_bytes, row_stride, height
        ),
    )


def _decode_n64_texture_rows(
    packed: bytes, row_bytes: int, row_stride: int, height: int
) -> bytes:
    """Remove archive scanline padding.

    The texture records contain source image rows, not a dump of RDP TMEM.
    Consequently odd rows remain in ordinary byte order.  Applying the TMEM
    LoadBlock odd-line word swap here corrupts every other scanline into
    four-byte bands (most visibly on Dreamland's doors, roofs, and foliage).
    """

    output = bytearray()
    for row in range(height):
        output.extend(
            packed[row * row_stride:row * row_stride + row_bytes]
        )
    return bytes(output)


def _encode_psx_indexed(
    width: int,
    height: int,
    palette: Iterable[int],
    pixels: bytes,
    *,
    ci8: bool,
    alpha_zero_transparent: bool = False,
) -> bytes:
    converted_palette = tuple(palette)
    count = 256 if ci8 else 16
    if len(converted_palette) > count:
        raise FormatError(
            f"{'CI8' if ci8 else 'CI4'} palette has "
            f"{len(converted_palette)} colors"
        )
    image_offset = 0x0C + count * 2
    pixel_offset = image_offset + 0x14
    result = bytearray(pixel_offset + len(pixels))
    struct.pack_into("<III", result, 0, 0x10, 0x09 if ci8 else 0x08, image_offset)
    struct.pack_into("<hhhh", result, 0x0C, 0, 480, count, 1)
    for index, color in enumerate(converted_palette):
        struct.pack_into(
            "<H",
            result,
            0x14 + index * 2,
            rgba5551_to_psx(
                color, alpha_zero_transparent=alpha_zero_transparent
            ),
        )
    words = (width + 1) // 2 if ci8 else (width + 3) // 4
    struct.pack_into("<hhhh", result, image_offset + 0x0C,
                     0, 0, words, height)
    struct.pack_into("<I", result, image_offset + 0x08,
                     0x0C + len(pixels))
    result[pixel_offset:pixel_offset + len(pixels)] = pixels
    return bytes(result)


def encode_psx_texture(
    texture: N64Texture,
    palette_bank: int = 0,
    *,
    alpha_zero_transparent: bool = False,
) -> bytes:
    if texture.format == 2 and texture.size == 0:
        first = palette_bank * 16
        palette = texture.palette_rgba5551[first:first + 16]
        if not palette:
            raise FormatError(
                f"texture {texture.index} has no CI4 palette bank {palette_bank}"
            )
        swapped = bytes(
            ((value & 0x0F) << 4) | (value >> 4)
            for value in texture.pixels
        )
        return _encode_psx_indexed(
            texture.width,
            texture.height,
            palette,
            swapped,
            ci8=False,
            alpha_zero_transparent=alpha_zero_transparent,
        )
    if texture.format == 2 and texture.size == 1:
        palette = texture.palette_rgba5551
        if not palette:
            # Palette-free N64 records are intensity sprites.  Their texel
            # byte is the luminance itself rather than an external CI index.
            palette = tuple(
                0
                if value == 0
                else (
                    ((value >> 3) << 11)
                    | ((value >> 3) << 6)
                    | ((value >> 3) << 1)
                    | 1
                )
                for value in range(256)
            )
        return _encode_psx_indexed(
            texture.width,
            texture.height,
            palette,
            texture.pixels,
            ci8=True,
            alpha_zero_transparent=alpha_zero_transparent,
        )
    if texture.format != 0 or texture.size not in (2, 3):
        raise FormatError(
            f"texture {texture.index} uses unsupported format/size "
            f"{texture.format}/{texture.size}"
        )
    rgba: list[tuple[int, int, int, int]] = []
    stride = 2 if texture.size == 2 else 4
    for offset in range(0, len(texture.pixels), stride):
        if texture.size == 2:
            value = be16(texture.pixels, offset)
            rgba.append(
                (
                    (value >> 11) & 0x1F,
                    (value >> 6) & 0x1F,
                    (value >> 1) & 0x1F,
                    value & 1,
                )
            )
        else:
            red, green, blue, alpha = texture.pixels[offset:offset + 4]
            rgba.append((red >> 3, green >> 3, blue >> 3, 1 if alpha else 0))
    palette_values: list[int] = []
    palette_lookup: dict[tuple[int, int, int, int], int] = {}
    indices = bytearray()
    for color in rgba:
        if color not in palette_lookup:
            if len(palette_values) == 256:
                raise FormatError(
                    f"RGBA texture {texture.index} exceeds the PSX 256-color limit"
                )
            palette_lookup[color] = len(palette_values)
            red, green, blue, alpha = color
            palette_values.append(
                (red << 11) | (green << 6) | (blue << 1) | alpha
            )
        indices.append(palette_lookup[color])
    return _encode_psx_indexed(
        texture.width,
        texture.height,
        palette_values,
        bytes(indices),
        ci8=True,
        alpha_zero_transparent=alpha_zero_transparent,
    )


def encode_psx_ci8(
    payload: bytes,
    *,
    output_width: int | None = None,
    output_height: int | None = None,
) -> bytes:
    if len(payload) < 0x208 or payload[0] != 2 or payload[1] != 1:
        raise FormatError("N64 CI8 image has an invalid header")
    width = be16(payload, 4)
    height = be16(payload, 6)
    expected = 8 + 512 + width * height
    if len(payload) < expected:
        raise FormatError("N64 CI8 image is truncated")
    palette = [be16(payload, 8 + index * 2) for index in range(256)]
    pixels = payload[0x208:0x208 + width * height]
    if output_width is not None or output_height is not None:
        scaled_width = output_width if output_width is not None else width
        scaled_height = output_height if output_height is not None else height
        if scaled_width <= 0 or scaled_height <= 0:
            raise FormatError("scaled CI8 image dimensions must be positive")
        # The retail PS1 terrain atlases are 3:2 larger than their N64
        # counterparts (480x192 versus 320x128).  Preserve the indexed source
        # palette and expand texels with integer nearest-neighbour sampling;
        # this keeps every 32x32 N64 material cell aligned to the PS1
        # renderer's fixed 48x48 UV grid without inventing blended colors.
        pixels = bytes(
            pixels[
                min(height - 1, destination_y * height // scaled_height)
                * width
                + min(width - 1, destination_x * width // scaled_width)
            ]
            for destination_y in range(scaled_height)
            for destination_x in range(scaled_width)
        )
        width = scaled_width
        height = scaled_height
    image_offset = 0x20C
    pixel_offset = image_offset + 0x14
    result = bytearray(pixel_offset + len(pixels))
    struct.pack_into("<III", result, 0, 0x10, 0x09, image_offset)
    struct.pack_into("<hhhh", result, 0x0C, 0, 480, 256, 1)
    for index, color in enumerate(palette):
        struct.pack_into("<H", result, 0x14 + index * 2, rgba5551_to_psx(color))
    struct.pack_into("<hhhh", result, image_offset + 0x0C,
                     0, 0, (width + 1) // 2, height)
    struct.pack_into("<I", result, image_offset + 0x08,
                     0x0C + len(pixels))
    result[pixel_offset:pixel_offset + len(pixels)] = pixels
    return bytes(result)


def convert_tinf(payload: bytes) -> bytes:
    """Convert N64 32-pixel terrain-material origins to PS1's 48-pixel grid.

    TINF contains 256 fixed 40-byte material records.  Each record consists
    of two 20-byte loader subrecords; the coordinate words at +2 and +4 in
    each subrecord use the same platform scale.  Comparing the complete
    matching Airgrave N64 and PS1 tables proves that the PS1 conversion is
    exactly ``coordinate * 3 / 2``; every other byte is identical.
    """

    if len(payload) % 20:
        raise FormatError("TINF is not an array of 20-byte records")
    result = bytearray(payload)
    for record in range(0, len(result), 20):
        for field in (2, 4):
            value = be16(result, record + field)
            scaled = value * 3
            if scaled & 1:
                raise FormatError(
                    f"TINF coordinate 0x{value:04X} is not exactly 3:2 scalable"
                )
            scaled //= 2
            if scaled > 0xFFFF:
                raise FormatError("scaled TINF coordinate exceeds u16")
            struct.pack_into(">H", result, record + field, scaled)
    return bytes(result)


@dataclass(frozen=True)
class Vertex:
    x: int
    y: int
    z: int
    s: int
    t: int
    color: tuple[int, int, int, int]
    source_offset: int


@dataclass(frozen=True)
class Face:
    vertices: tuple[Vertex, Vertex, Vertex]
    texture: int | None
    palette_bank: int
    color: tuple[int, int, int]
    uv: tuple[tuple[int, int], tuple[int, int], tuple[int, int]]
    lighting: bool = False
    dynamic_texture: bool = False
    double_sided: bool = False
    semi_transparent: bool = False


@dataclass
class RdpTile:
    palette_bank: int = 0
    clamp_s: bool = False
    mirror_s: bool = False
    mask_s: int = 0
    shift_s: int = 0
    clamp_t: bool = False
    mirror_t: bool = False
    mask_t: int = 0
    shift_t: int = 0
    upper_s: int = 0
    upper_t: int = 0
    lower_s: int = 0x3FC
    lower_t: int = 0x3FC


def _round_half_away(value: float) -> int:
    if value < 0:
        return math.ceil(value - 0.5)
    return math.floor(value + 0.5)


def _rdp_axis_coordinate(
    source: int,
    texture_scale: int,
    upper: int,
    lower: int,
    shift: int,
    mask: int,
    mirror: bool,
    clamp: bool,
) -> int:
    """Bake the RSP/RDP texture-coordinate state into a PSX UV byte.

    V8's vertices store S/T in signed 10.5 fixed point.  F3DEX2's texture
    scale is unsigned 0.16 with 0xffff used as exact unity by the game.  Tile
    upper/lower bounds are unsigned 10.2 fixed point.  The PSX packets have no
    corresponding shift, wrap, mirror, or tile-origin state, so conversion
    must resolve it here rather than clipping the raw vertex value.
    """

    scale = 1.0 if texture_scale == 0xFFFF else texture_scale / 65536.0
    texel = source * scale / 32.0
    if 0 < shift <= 10:
        texel /= 1 << shift
    elif shift > 10:
        texel *= 1 << (16 - shift)
    texel -= upper / 4.0
    coordinate = _round_half_away(texel)

    if mask:
        period = 1 << mask
        wrapped = coordinate % period
        if mirror and ((coordinate // period) & 1):
            wrapped = period - 1 - wrapped
        coordinate = wrapped
    if clamp:
        maximum = max(0, (lower - upper) >> 2)
        coordinate = max(0, min(maximum, coordinate))
    return max(0, min(255, coordinate))


def _rdp_uv(
    vertex: Vertex,
    texture_scale: tuple[int, int],
    tile: RdpTile,
) -> tuple[int, int]:
    return (
        _rdp_axis_coordinate(
            vertex.s,
            texture_scale[0],
            tile.upper_s,
            tile.lower_s,
            tile.shift_s,
            tile.mask_s,
            tile.mirror_s,
            tile.clamp_s,
        ),
        _rdp_axis_coordinate(
            vertex.t,
            texture_scale[1],
            tile.upper_t,
            tile.lower_t,
            tile.shift_t,
            tile.mask_t,
            tile.mirror_t,
            tile.clamp_t,
        ),
    )


def _texture_for_address(
    address: int, textures: list[N64Texture], texture_base: int
) -> int | None:
    if address >> 24 != 2:
        return None
    target = texture_base + (address & 0xFFFFFF)
    for texture in textures:
        palette = texture.source_offset + 8
        pixels = texture.source_offset + align(
            8 + texture.palette_count * 2, 8
        )
        if target in (palette, pixels):
            return texture.index
    return None


def _vertex_attributes_are_normals(vertices: tuple[Vertex, Vertex, Vertex]) -> bool:
    """Recognize the signed-byte unit vectors emitted while RSP lighting is on.

    Some secondary display lists inherit G_LIGHTING from their caller and do
    not repeat a local geometry-mode command.  Their vertex attributes remain
    authoritative: all three signed XYZ vectors have length approximately
    127, whereas unlit RGBA vertices do not.
    """

    for vertex in vertices:
        components = tuple(
            value - 256 if value >= 128 else value
            for value in vertex.color[:3]
        )
        magnitude_squared = sum(value * value for value in components)
        if not 14000 <= magnitude_squared <= 18000:
            return False
    return True


def parse_group_faces(
    data: bytes,
    group_offset: int,
    textures: list[N64Texture],
    texture_base: int,
) -> tuple[list[Vertex], list[Face], int, int, int]:
    scale_shift = data[group_offset + 0x14]
    auxiliary_count = data[group_offset + 0x15]
    native_extent = be16(data, group_offset + 0x16, signed=True)
    vertex_relative = be32(data, group_offset + 4)
    opaque_display = be32(data, group_offset + 8)
    alternate_display = be32(data, group_offset + 12)
    # The two descriptor pointers are separate opaque and alternate render
    # passes, not platform alternatives.  Some groups (for example Airgrave's
    # animated dish) own geometry in both.  Selecting only the first pointer
    # silently discarded translucent/emissive pieces.
    display_relatives = tuple(
        value for value in (opaque_display, alternate_display) if value
    )
    if not display_relatives or vertex_relative == 0:
        return [], [], scale_shift, auxiliary_count, native_extent
    vertex_base = group_offset + vertex_relative
    all_vertices: dict[int, Vertex] = {}
    faces: list[Face] = []
    for display_relative in display_relatives:
        cursor = group_offset + display_relative
        cache: dict[int, Vertex] = {}
        texture_index: int | None = None
        texture_enabled = False
        texture_scale = (0xFFFF, 0xFFFF)
        render_tile = 0
        tiles = [RdpTile() for _ in range(8)]
        dynamic_texture = False
        lighting_enabled = False
        double_sided = False
        primitive_color = (127, 127, 127)
        semi_transparent = False

        def triangle(indices: tuple[int, int, int]) -> None:
            if any(index not in cache for index in indices):
                raise FormatError(
                    f"group at 0x{group_offset:X} triangle references unloaded "
                    f"vertices {indices}"
                )
            vertices = tuple(cache[index] for index in indices)
            tile = tiles[render_tile]
            face_lighting = (
                lighting_enabled or _vertex_attributes_are_normals(vertices)
            )
            faces.append(
                Face(
                    vertices,
                    texture_index if texture_enabled else None,
                    tile.palette_bank,
                    primitive_color,
                    tuple(
                        _rdp_uv(vertex, texture_scale, tile)
                        for vertex in vertices
                    ),
                    face_lighting,
                    dynamic_texture if texture_enabled else False,
                    double_sided,
                    semi_transparent,
                )
            )

        for _command_index in range(0x10000):
            if cursor + 8 > len(data):
                raise FormatError(
                    f"group at 0x{group_offset:X} display list is truncated"
                )
            command = data[cursor:cursor + 8]
            opcode = command[0]
            word0 = be32(command, 0)
            word1 = be32(command, 4)
            cursor += 8
            if opcode == 0xDF:
                break
            if opcode == 0xFD:
                if word1 >> 24 == 3 and (word1 & 0xFFFFFF) >= 8:
                    # Segment three is engine-owned dynamic imagery, not the
                    # XOBF texture table.  Retail N64 water points at
                    # 0x03000008; the corresponding PS1 maps encode this as
                    # raw packet kind 13 with native dynamic texture word
                    # 0x4000.
                    texture_index = None
                    dynamic_texture = True
                else:
                    resolved = _texture_for_address(
                        word1, textures, texture_base
                    )
                    if resolved is not None:
                        texture_index = resolved
                        dynamic_texture = False
                continue
            if opcode == 0xD7:
                # F3DEX2 G_TEXTURE stores G_ON/G_OFF in word zero; word one
                # is the independent unsigned 0.16 S/T scale pair.
                texture_enabled = bool(word0 & 0x7F)
                render_tile = (word0 >> 8) & 7
                texture_scale = (word1 >> 16, word1 & 0xFFFF)
                continue
            if opcode == 0xFA:
                primitive_color = (command[4], command[5], command[6])
                continue
            if opcode == 0xE2:
                # V8's N64 translucent surface blender word ends in 0x49D8.
                # An alternate display list is not inherently translucent:
                # Airgrave and Dreamland also place opaque/emissive geometry
                # there, so derive the PSX flag from the authored RDP state.
                semi_transparent = (word1 & 0xFFFF) == 0x49D8
                continue
            if opcode == 0xD9:
                # F3DEX2 G_GEOMETRYMODE encodes a clear mask in word zero and
                # set bits in word one.  Water trench side/top groups clear
                # G_CULL_BACK (0x400); their PS1 counterparts duplicate the
                # reversed triangle because the PS1 has no equivalent
                # two-sided polygon bit.
                if not (word0 & 0x10000):
                    lighting_enabled = False
                if word1 & 0x10000:
                    lighting_enabled = True
                if not (word0 & 0x400):
                    double_sided = True
                if word1 & 0x400:
                    double_sided = False
                continue
            if opcode == 0xF5:
                tile = tiles[(word1 >> 24) & 7]
                tile.palette_bank = (word1 >> 20) & 0x0F
                tile.clamp_t = bool(word1 & (2 << 18))
                tile.mirror_t = bool(word1 & (1 << 18))
                tile.mask_t = (word1 >> 14) & 0x0F
                tile.shift_t = (word1 >> 10) & 0x0F
                tile.clamp_s = bool(word1 & (2 << 8))
                tile.mirror_s = bool(word1 & (1 << 8))
                tile.mask_s = (word1 >> 4) & 0x0F
                tile.shift_s = word1 & 0x0F
                continue
            if opcode == 0xF2:
                # G_SETTILESIZE uses inclusive unsigned 10.2 bounds.  Preserve
                # both the upper origin and lower clamp edge.
                tile = tiles[(word1 >> 24) & 7]
                tile.upper_s = (word0 >> 12) & 0x0FFF
                tile.upper_t = word0 & 0x0FFF
                tile.lower_s = (word1 >> 12) & 0x0FFF
                tile.lower_t = word1 & 0x0FFF
                continue
            if opcode == 0x01:
                count = (word0 >> 12) & 0xFF
                end_index = (word0 >> 1) & 0x7F
                first_index = end_index - count
                if count <= 0 or first_index < 0 or word1 >> 24 != 1:
                    raise FormatError(
                        f"group at 0x{group_offset:X} has invalid G_VTX "
                        f"{command.hex()}"
                    )
                source = vertex_base + (word1 & 0xFFFFFF)
                for item in range(count):
                    offset = source + item * 16
                    if offset + 16 > len(data):
                        raise FormatError("N64 vertex array is truncated")
                    vertex = Vertex(
                        x=be16(data, offset, signed=True),
                        y=be16(data, offset + 2, signed=True),
                        z=be16(data, offset + 4, signed=True),
                        s=be16(data, offset + 8, signed=True),
                        t=be16(data, offset + 10, signed=True),
                        color=tuple(data[offset + 12:offset + 16]),  # type: ignore[arg-type]
                        source_offset=offset,
                    )
                    cache[first_index + item] = vertex
                    all_vertices[offset] = vertex
                continue
            if opcode == 0x05:
                triangle((command[1] >> 1, command[2] >> 1, command[3] >> 1))
                continue
            if opcode == 0x06:
                triangle((command[1] >> 1, command[2] >> 1, command[3] >> 1))
                triangle((command[5] >> 1, command[6] >> 1, command[7] >> 1))
                continue
            if opcode not in {
                0xE3, 0xE6, 0xE7, 0xE8,
                0xF0, 0xF3, 0xFB, 0xFC,
            }:
                raise FormatError(
                    f"group at 0x{group_offset:X} has unsupported display "
                    f"opcode 0x{opcode:02X}"
                )
        else:
            raise FormatError(f"group at 0x{group_offset:X} has no G_ENDDL")
    return (
        list(all_vertices.values()),
        faces,
        scale_shift,
        auxiliary_count,
        native_extent,
    )


def _face_normal(a: Vertex, b: Vertex, c: Vertex) -> tuple[int, int, int]:
    ab = (b.x - a.x, b.y - a.y, b.z - a.z)
    ac = (c.x - a.x, c.y - a.y, c.z - a.z)
    normal = (
        ab[1] * ac[2] - ab[2] * ac[1],
        ab[2] * ac[0] - ab[0] * ac[2],
        ab[0] * ac[1] - ab[1] * ac[0],
    )
    magnitude = math.sqrt(sum(value * value for value in normal))
    if magnitude == 0:
        return 0, -4096, 0
    return tuple(
        max(-32768, min(32767, round(value * 4096 / magnitude)))
        for value in normal
    )  # type: ignore[return-value]


def _vertex_normal(vertex: Vertex) -> tuple[int, int, int]:
    """Convert an F3DEX2 signed-byte vertex normal to PS1 Q12."""

    components = tuple(
        value - 256 if value >= 128 else value
        for value in vertex.color[:3]
    )
    magnitude = math.sqrt(sum(value * value for value in components))
    if magnitude == 0:
        return 0, -4096, 0
    return tuple(
        max(-32768, min(32767, _round_half_away(value * 4096 / magnitude)))
        for value in components
    )  # type: ignore[return-value]


def _face_has_one_normal(face: Face) -> bool:
    """Match the source's flat-normal collapse after byte quantization."""

    source = [
        tuple(
            value - 256 if value >= 128 else value
            for value in vertex.color[:3]
        )
        for vertex in face.vertices
    ]
    return all(
        max(abs(a - b) for a, b in zip(source[0], item)) <= 2
        for item in source[1:]
    )


def encode_psx_group(
    vertices: list[Vertex],
    faces: list[Face],
    scale_shift: int,
    texture_base: int,
    auxiliary_count: int,
    native_extent: int,
) -> bytes:
    if not vertices or not faces:
        return struct.pack(
            "<IIIIHhIBBH",
            0, 0x1C,
            0, 0x1C,
            0, 0,
            0x1C, scale_shift, auxiliary_count, native_extent & 0xFFFF,
        )
    index_by_source = {vertex.source_offset: index for index, vertex in enumerate(vertices)}
    packets = bytearray()
    normals = bytearray()
    normal_indices: dict[tuple[int, int, int], int] = {}
    polygon_count = 0

    def normal_index(value: tuple[int, int, int]) -> int:
        index = normal_indices.get(value)
        if index is not None:
            return index
        index = len(normal_indices)
        normal_indices[value] = index
        normals.extend(struct.pack("<hhhh", value[0], value[1], value[2], 0))
        return index

    for face in faces:
        indices = tuple(index_by_source[vertex.source_offset] for vertex in face.vertices)
        color = face.color
        # F3DEX2 and the PS1 GTE use opposite front-face conventions for this
        # shared model space.  Matching Airgrave geometry and the identical
        # Dreamland/Casino/Hoover/Valley water meshes show that every
        # single-sided retail PS1 triangle is the N64 triangle in 2,1,0
        # order.  Keeping 0,1,2 makes exterior architecture and the top of
        # water planes back-face culled.
        psx_order = (2, 1, 0)
        if face.dynamic_texture:
            # This is the exact native representation used by the matching
            # Casino City, Hoover Dam, and Valley Farm PS1 water groups:
            # raw kind 13 with the native 0x10 render flag (type byte 29),
            # neutral modulation, and engine-global dynamic texture word
            # 0x4000.  Object_BuildFromBin moves the raw low nibble into the
            # runtime packet-family field.
            uvs = face.uv

            def dynamic_packet(order: tuple[int, int, int]) -> bytes:
                return (
                    struct.pack(
                        "<BBBBHHHH",
                        128, 128, 128, 29,
                        indices[order[0]], indices[order[1]], indices[order[2]],
                        0,
                    )
                    + bytes(uvs[order[0]])
                    + bytes(uvs[order[1]])
                    + bytes(uvs[order[2]])
                    + struct.pack("<H", 0x4000)
                )

            packets += dynamic_packet(psx_order)
            polygon_count += 1
            if face.double_sided:
                packets += dynamic_packet((0, 1, 2))
                polygon_count += 1
            continue
        if face.texture is None and face.semi_transparent:
            # N64 expresses the shared contact-water particle as a
            # primitive-colored translucent surface.  The matching retail
            # PS1 Airgrave bank proves its native representation: raw kind
            # 13 plus the render flag bit 0x10, neutral modulation, dynamic
            # material word 0x400a, and both windings.  A twelve-byte kind-0
            # packet carrying code 2 is invalid because raw kind 2 is a
            # twenty-byte record; it desynchronizes every packet that follows.
            def contact_water_packet(order: tuple[int, int, int]) -> bytes:
                return (
                    struct.pack(
                        "<BBBBHHHH",
                        128, 128, 128, 29,
                        indices[order[0]],
                        indices[order[1]],
                        indices[order[2]],
                        0,
                    )
                    + b"\0\0\0\0\0\0"
                    + struct.pack("<H", 0x400A)
                )

            packets += contact_water_packet(psx_order)
            packets += contact_water_packet((0, 1, 2))
            polygon_count += 2
            continue
        debug_untextured = os.environ.get("V8_N64_DEBUG_UNTEXTURED") == "1"
        orders = [psx_order]
        if face.double_sided:
            orders.append((0, 1, 2))

        vertex_normals = tuple(_vertex_normal(vertex) for vertex in face.vertices)
        flat_normal = _face_has_one_normal(face)
        lit = face.lighting and not debug_untextured

        for order in orders:
            ordered_indices = tuple(indices[item] for item in order)
            if face.texture is None or debug_untextured:
                if lit:
                    if flat_normal:
                        packets += struct.pack(
                            "<BBBBHHHH",
                            color[0], color[1], color[2],
                            4 | (0x10 if face.semi_transparent else 0),
                            ordered_indices[0],
                            ordered_indices[1],
                            ordered_indices[2],
                            normal_index(vertex_normals[order[0]]),
                        )
                    else:
                        ordered_normals = tuple(
                            normal_index(vertex_normals[item]) for item in order
                        )
                        packets += struct.pack(
                            "<BBBBHHHHHH",
                            color[0], color[1], color[2],
                            8 | (0x10 if face.semi_transparent else 0),
                            ordered_indices[0],
                            ordered_indices[1],
                            ordered_indices[2],
                            ordered_normals[0],
                            ordered_normals[1],
                            ordered_normals[2],
                        )
                else:
                    packets += struct.pack(
                        "<BBBBHHHH",
                        255 if debug_untextured and face.texture is not None
                        else color[0],
                        0 if debug_untextured and face.texture is not None
                        else color[1],
                        255 if debug_untextured and face.texture is not None
                        else color[2],
                        0 | (0x10 if face.semi_transparent else 0),
                        ordered_indices[0],
                        ordered_indices[1],
                        ordered_indices[2],
                        0,
                    )
                polygon_count += 1
                continue

            uvs = face.uv
            if lit and not flat_normal:
                ordered_normals = tuple(
                    normal_index(vertex_normals[item]) for item in order
                )
                packets += (
                    struct.pack(
                        "<BBBBHHHHHH",
                        color[0], color[1], color[2],
                        9 | (0x10 if face.semi_transparent else 0),
                        ordered_indices[0],
                        ordered_indices[1],
                        ordered_indices[2],
                        ordered_normals[0],
                        ordered_normals[1],
                        ordered_normals[2],
                    )
                    + bytes(uvs[order[0]])
                    + bytes(uvs[order[1]])
                    + bytes(uvs[order[2]])
                    + struct.pack("<H", face.texture)
                )
            else:
                # Kind five is the modulated flat-lit triangle.  Kind
                # thirteen is the unlit/raw-texture sibling.  The 0x10 flag
                # selects native semitransparency without changing the
                # on-disk packet stride.
                packet_kind = 5 if lit else 13
                packet_normal = (
                    normal_index(vertex_normals[order[0]]) if lit else 0
                )
                packets += (
                    struct.pack(
                        "<BBBBHHHH",
                        color[0], color[1], color[2],
                        packet_kind
                        | (0x10 if face.semi_transparent else 0),
                        ordered_indices[0],
                        ordered_indices[1],
                        ordered_indices[2],
                        packet_normal,
                    )
                    + bytes(uvs[order[0]])
                    + bytes(uvs[order[1]])
                    + bytes(uvs[order[2]])
                    + struct.pack("<H", face.texture)
                )
            polygon_count += 1
    encoded_vertices = b"".join(
        struct.pack("<hhhh", vertex.x, vertex.y, vertex.z, 0)
        for vertex in vertices
    )
    polygon_relative = 0x1C
    vertex_relative = polygon_relative + len(packets)
    normal_relative = vertex_relative + len(encoded_vertices)
    return (
        struct.pack(
            "<IIIIHhIBBH",
            len(vertices), vertex_relative,
            len(normal_indices), normal_relative,
            polygon_count, texture_base,
            polygon_relative, scale_shift, auxiliary_count,
            native_extent & 0xFFFF,
        )
        + packets + encoded_vertices + normals
    )


def convert_slot(record: bytes) -> bytes:
    if len(record) != 0x1C:
        raise ValueError(len(record))
    render_key = be16(record, 0)
    # N64's object builder masks render keys to eleven bits and uses 0x07ff
    # as its no-model sentinel. The PS1 builder uses signed structural keys:
    # retail AIRGRAVE pairs 0xf0NN with 0xffNN and 0xf7ff with 0xfffe.
    # Preserve those native PS1 encodings instead of merely byte-swapping the
    # N64 control values.
    if render_key == 0x07FF:
        render_key = 0xFFFF
    elif render_key == 0xF7FF:
        render_key = 0xFFFE
    elif render_key & 0xFF00 == 0xF000:
        render_key = 0xFF00 | (render_key & 0x00FF)
    return struct.pack(
        "<hhiiihhhhhh",
        render_key if render_key < 0x8000 else render_key - 0x10000,
        be16(record, 2, signed=True),
        be32(record, 4, signed=True),
        be32(record, 8, signed=True),
        be32(record, 12, signed=True),
        be16(record, 16, signed=True),
        be16(record, 18, signed=True),
        be16(record, 20, signed=True),
        be16(record, 22, signed=True),
        be16(record, 24, signed=True),
        be16(record, 26, signed=True),
    )


def convert_collision_stream(data: bytes, start: int, end: int) -> bytes:
    result = bytearray()
    cursor = start
    while cursor + 2 <= end:
        kind = be16(data, cursor, signed=True)
        if kind == 0:
            result += struct.pack("<h", 0)
            return bytes(result)
        if kind == 1:
            if cursor + 0x1C > end:
                raise FormatError("truncated N64 collision AABB")
            result += struct.pack(
                "<hhiiiiii",
                1,
                be16(data, cursor + 2, signed=True),
                *(be32(data, cursor + offset, signed=True)
                  for offset in range(4, 0x1C, 4)),
            )
            cursor += 0x1C
            continue
        if kind == 2:
            plane_count = be16(data, cursor + 2)
            if cursor + 4 + plane_count * 12 > end:
                raise FormatError("truncated N64 collision convex")
            result += struct.pack("<hh", 2, plane_count)
            cursor += 4
            for _ in range(plane_count):
                result += struct.pack(
                    "<hhhhI",
                    be16(data, cursor, signed=True),
                    be16(data, cursor + 2, signed=True),
                    be16(data, cursor + 4, signed=True),
                    be16(data, cursor + 6, signed=True),
                    be32(data, cursor + 8),
                )
                cursor += 12
            continue
        raise FormatError(f"unsupported N64 collision kind {kind}")
    raise FormatError("N64 collision stream has no terminator")


def _layout_table(
    output: bytearray, blobs: list[bytes], boundary: int = 4
) -> tuple[int, list[int]]:
    table = len(output)
    output.extend(b"\0" * (len(blobs) * 4))
    offsets: list[int] = []
    for blob in blobs:
        while len(output) < align(len(output), boundary):
            output.append(0)
        target = len(output)
        offsets.append(target)
        output.extend(blob)
    for index, target in enumerate(offsets):
        struct.pack_into("<I", output, table + index * 4, target - table)
    return table, offsets


@dataclass(frozen=True)
class XobfReport:
    groups: int
    faces: int
    textures: int
    collisions: int
    slots: int


def convert_xobf_bin(data: bytes) -> tuple[bytes, XobfReport]:
    if len(data) < 0x1C:
        raise FormatError("N64 XOBF is truncated")
    group_count = be32(data, 0)
    group_table = be32(data, 4)
    # XOBF stores the highest collision-stream index, not the number of
    # streams.  Both retail engines iterate inclusively.  Treating this as a
    # count drops the final stream and leaves the PS1 loader reading the first
    # texture-table word as a collision offset.
    collision_count_minus_one = be32(data, 8)
    collision_count = collision_count_minus_one + 1
    collision_table = be32(data, 12)
    texture_count = be32(data, 16)
    texture_table = be32(data, 20)
    slot_count = be32(data, 24)
    if 0x1C + slot_count * 0x1C > len(data):
        raise FormatError("N64 XOBF slot table is truncated")

    texture_targets = [
        texture_table + be32(data, texture_table + index * 4)
        for index in range(texture_count)
    ]
    textures = [
        parse_n64_texture(
            data,
            target,
            index,
            texture_targets[index + 1] if index + 1 < texture_count else len(data),
        )
        for index, target in enumerate(texture_targets)
    ]
    texture_base = texture_targets[0] if textures else 0

    group_targets = [
        group_table + be32(data, group_table + index * 4)
        for index in range(group_count)
    ]
    parsed_groups: list[
        tuple[list[Vertex], list[Face], int, int, int]
    ] = []
    face_count = 0
    for target in group_targets:
        (
            vertices,
            faces,
            scale_shift,
            auxiliary_count,
            native_extent,
        ) = parse_group_faces(data, target, textures, texture_base)
        face_count += len(faces)
        parsed_groups.append(
            (
                vertices,
                faces,
                scale_shift,
                auxiliary_count,
                native_extent,
            )
        )

    variant_indices = {
        (texture.index, 0, False): texture.index for texture in textures
    }
    texture_variants: list[tuple[N64Texture, int, bool]] = [
        (texture, 0, False) for texture in textures
    ]
    for _vertices, faces, _scale, _auxiliary_count, _extent in parsed_groups:
        for face in faces:
            if face.texture is None:
                continue
            texture = textures[face.texture]
            bank = (
                face.palette_bank
                if texture.format == 2
                and texture.size == 0
                and face.palette_bank * 16 < texture.palette_count
                else 0
            )
            key = (face.texture, bank, face.semi_transparent)
            if key not in variant_indices:
                variant_indices[key] = len(texture_variants)
                texture_variants.append(
                    (texture, bank, face.semi_transparent)
                )

    groups: list[bytes] = []
    for (
        vertices,
        faces,
        scale_shift,
        auxiliary_count,
        native_extent,
    ) in parsed_groups:
        remapped: list[Face] = []
        for face in faces:
            if face.texture is None:
                remapped.append(face)
                continue
            texture = textures[face.texture]
            bank = (
                face.palette_bank
                if texture.format == 2
                and texture.size == 0
                and face.palette_bank * 16 < texture.palette_count
                else 0
            )
            remapped.append(
                Face(
                    vertices=face.vertices,
                    texture=variant_indices[
                        (face.texture, bank, face.semi_transparent)
                    ],
                    palette_bank=0,
                    color=face.color,
                    uv=face.uv,
                    lighting=face.lighting,
                    dynamic_texture=face.dynamic_texture,
                    double_sided=face.double_sided,
                    semi_transparent=face.semi_transparent,
                )
            )
        groups.append(
            encode_psx_group(
                vertices,
                remapped,
                scale_shift,
                0,
                auxiliary_count,
                native_extent,
            )
        )

    collision_targets = [
        collision_table + be32(data, collision_table + index * 4)
        for index in range(collision_count)
    ]
    collision_limits = [
        value for value in (texture_table, group_table, len(data)) if value > 0
    ]
    collisions = []
    for index, target in enumerate(collision_targets):
        later = (
            collision_targets[index + 1]
            if index + 1 < collision_count
            else min(value for value in collision_limits if value > target)
        )
        collisions.append(convert_collision_stream(data, target, later))

    output = bytearray(b"\0" * 0x1C)
    for index in range(slot_count):
        start = 0x1C + index * 0x1C
        output += convert_slot(data[start:start + 0x1C])
    while len(output) < align(len(output), 4):
        output.append(0)
    group_table_out, _ = _layout_table(output, groups)
    while len(output) < align(len(output), 4):
        output.append(0)
    collision_table_out, _ = _layout_table(output, collisions)
    texture_blobs = [
        encode_psx_texture(
            texture,
            bank,
            alpha_zero_transparent=alpha_zero_transparent,
        )
        for texture, bank, alpha_zero_transparent in texture_variants
    ]
    while len(output) < align(len(output), 4):
        output.append(0)
    texture_table_out, _ = _layout_table(output, texture_blobs)
    struct.pack_into(
        "<IIIIIII", output, 0,
        group_count, group_table_out,
        collision_count_minus_one, collision_table_out,
        len(texture_blobs), texture_table_out,
        slot_count,
    )
    return bytes(output), XobfReport(
        group_count, face_count, len(texture_blobs), collision_count, slot_count
    )


def convert_aimp(payload: bytes) -> bytes:
    if len(payload) & 1:
        raise FormatError("AIMP is not halfword-aligned")
    return b"".join(
        payload[offset:offset + 2][::-1]
        for offset in range(0, len(payload), 2)
    )


def convert_xrtp(payload: bytes) -> bytes:
    if len(payload) <= 12:
        return payload
    texture = parse_n64_texture(payload, 12, 0, len(payload))
    return payload[:12] + encode_psx_texture(texture)


def convert_animation(payload: bytes, slot_count: int) -> bytes:
    """Convert the shared semantic ANM stream from BE fields to LE fields."""

    table_size = 4 + slot_count * 4
    if len(payload) < table_size:
        raise FormatError("N64 ANM table is truncated")
    offsets = [be32(payload, 4 + index * 4) for index in range(slot_count)]
    output = bytearray(payload)
    struct.pack_into("<I", output, 0, be32(payload, 0))
    for index, offset in enumerate(offsets):
        struct.pack_into("<I", output, 4 + index * 4, offset)
        if offset and not (table_size <= offset < len(payload)):
            raise FormatError(f"N64 ANM slot {index} has invalid offset 0x{offset:X}")
    for slot, start in enumerate(offsets):
        if start == 0:
            continue
        end = min((value for value in offsets if value > start), default=len(payload))
        cursor = start
        while cursor + 4 <= end:
            frame_delta = be16(payload, cursor, signed=True)
            flags = be16(payload, cursor + 2, signed=True)
            struct.pack_into("<hh", output, cursor, frame_delta, flags)
            cursor += 4
            if flags < 0:
                continue
            if flags & ~0x7B:
                raise FormatError(
                    f"N64 ANM slot {slot} uses unsupported flags 0x{flags:04X}"
                )
            if flags & 0x01:
                values = [be16(payload, cursor + item * 2, signed=True)
                          for item in range(4)]
                struct.pack_into("<hhhh", output, cursor, *values)
                cursor += 8
            if flags & 0x02:
                values = [be32(payload, cursor + item * 4, signed=True)
                          for item in range(3)]
                struct.pack_into("<iii", output, cursor, *values)
                cursor += 12
            if flags & 0x08:
                values = [be16(payload, cursor + item * 2, signed=True)
                          for item in range(4)]
                struct.pack_into("<hhhh", output, cursor, *values)
                cursor += 8
            if flags & 0x10:
                while True:
                    target = be16(payload, cursor)
                    texture = be16(payload, cursor + 2)
                    struct.pack_into("<HH", output, cursor, target, texture)
                    cursor += 4
                    if target & 0x8000:
                        break
            if flags & 0x20:
                values = [be16(payload, cursor + item * 2, signed=True)
                          for item in range(4)]
                struct.pack_into("<hhhh", output, cursor, *values)
                cursor += 8
            if flags & 0x40:
                count = be32(payload, cursor)
                struct.pack_into("<I", output, cursor, count)
                cursor += 4
                for _ in range(count):
                    values = [be16(payload, cursor + item * 2, signed=True)
                              for item in range(4)]
                    struct.pack_into("<hhhh", output, cursor, *values)
                    cursor += 8
            if cursor > end:
                raise FormatError(f"N64 ANM slot {slot} crosses its stream boundary")
    return bytes(output)


@dataclass(frozen=True)
class ArenaReport:
    name: str
    xobf: tuple[XobfReport, ...]
    zones: int
    objects: int
    chunks: tuple[str, ...]


def convert_arena(exp: bytes, name: str) -> tuple[bytes, ArenaReport]:
    children = root_children(exp)
    converted: list[bytes] = []
    reports: list[XobfReport] = []
    zone_count = 0
    object_count = sum(
        1 for _off, tag, _payload, _parent in iter_chunks(exp) if tag == b"OBJ "
    )
    converted_tags: list[str] = []
    for child in children:
        tag = child.form_type if child.is_form else child.tag
        if tag == b"TITL":
            # N64 shell metadata, not consumed by the PSX terrain loader.
            continue
        if tag == b"XLSC":
            # JPEG loading art is converted separately once the shell image
            # layout is emitted. It is not a runtime terrain dependency.
            continue
        if tag == b"HEAD":
            head = bytearray(child.payload)
            if len(head) != 0x1A:
                raise FormatError("unexpected TERR/HEAD size")
            head[0:2] = b"\x00\x30"
            converted.append(iff_chunk(b"HEAD", bytes(head)))
            converted_tags.append("HEAD")
            continue
        if child.is_form and tag == b"XOBF":
            nested = form_children(iff_form(b"XOBF", [child.payload]), b"XOBF")
            bin_chunk = next(item for item in nested if item.tag == b"BIN ")
            model, report = convert_xobf_bin(bin_chunk.payload)
            reports.append(report)
            out_children = [iff_chunk(b"BIN ", model)]
            anm = next((item for item in nested if item.tag == b"ANM "), None)
            if anm is not None:
                out_children.append(
                    iff_chunk(
                        b"ANM ",
                        convert_animation(anm.payload, report.slots),
                    )
                )
            converted.append(iff_form(b"XOBF", out_children))
            converted_tags.append("XOBF")
            continue
        if tag == b"XBMP" or tag == b"XBGM":
            source = (
                child.payload[4:]
                if tag == b"XBGM" and child.payload[:4] == b"\0\0\0\0"
                else child.payload
            )
            if tag == b"XBMP":
                source_width = be16(source, 4)
                source_height = be16(source, 6)
                converted_image = encode_psx_ci8(
                    source,
                    output_width=source_width * 3 // 2,
                    output_height=source_height * 3 // 2,
                )
            else:
                converted_image = encode_psx_ci8(source)
            converted.append(iff_chunk(tag, converted_image))
            converted_tags.append(tag.decode("ascii"))
            continue
        if tag == b"TINF":
            converted.append(iff_chunk(tag, convert_tinf(child.payload)))
            converted_tags.append("TINF")
            continue
        if tag == b"AIMP":
            converted.append(iff_chunk(tag, convert_aimp(child.payload)))
            converted_tags.append("AIMP")
            continue
        if tag == b"XRTP":
            converted.append(iff_chunk(tag, convert_xrtp(child.payload)))
            converted_tags.append("XRTP")
            continue
        if tag == b"ZONE":
            zone_count += 1
        # These chunks are platform-neutral big-endian semantic records.
        if tag in {
            b"TEXT", b"SUNA", b"COLS", b"ZONE", b"ZMAP",
            b"RECT", b"PLTX", b"JUNC", b"RSEG", b"BSP ", b"OBJ ",
        }:
            if child.is_form:
                converted.append(iff_form(tag, [child.payload]))
            else:
                converted.append(iff_chunk(tag, child.payload))
            converted_tags.append(tag.decode("ascii"))
            continue
        raise FormatError(f"unresolved root chunk {tag!r}")
    result = iff_form(b"TERR", converted)
    return result, ArenaReport(
        name=name,
        xobf=tuple(reports),
        zones=zone_count,
        objects=object_count,
        chunks=tuple(converted_tags),
    )
