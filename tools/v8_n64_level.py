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


@dataclass(frozen=True)
class OverlayExport:
    name: str
    callback_offset: int


def overlay_exports(data: bytes, *, byteorder: str = "big") -> tuple[OverlayExport, ...]:
    """Decode the ordinary V8 overlay export directory.

    Both N64 and PS1 arena overlays use the same image-size/export-table
    header and zero-name terminator; only byte order differs.  Object names
    absent from this directory resolve to the engine's static/default
    callback, which is the reusable source-level distinction needed by arena
    conversion.  No arena or object identities participate in the decision.
    """

    if byteorder not in {"big", "little"}:
        raise ValueError(f"unsupported overlay byte order {byteorder!r}")
    if len(data) < 16:
        raise FormatError("overlay is truncated")

    def word(offset: int) -> int:
        return int.from_bytes(data[offset:offset + 4], byteorder)

    image_size = word(0)
    table = word(4)
    if image_size < 8 or image_size > len(data):
        raise FormatError(f"invalid overlay image size 0x{image_size:X}")
    if table < 8 or table + 8 > image_size or table & 3:
        raise FormatError(f"invalid overlay export table 0x{table:X}")

    result: list[OverlayExport] = []
    cursor = table
    while cursor + 8 <= image_size:
        name_offset = word(cursor)
        callback_offset = word(cursor + 4)
        cursor += 8
        if name_offset == 0:
            if callback_offset != 0:
                raise FormatError("overlay export terminator has a callback")
            return tuple(result)
        if not (table + 8 <= name_offset < image_size):
            raise FormatError(
                f"overlay export name 0x{name_offset:X} is outside its image"
            )
        if not (0 <= callback_offset < image_size):
            raise FormatError(
                f"overlay callback 0x{callback_offset:X} is outside its image"
            )
        end = data.find(b"\0", name_offset, image_size)
        if end < 0:
            raise FormatError("overlay export name is unterminated")
        try:
            name = data[name_offset:end].decode("ascii")
        except UnicodeDecodeError as error:
            raise FormatError("overlay export name is not ASCII") from error
        if not name:
            raise FormatError("overlay export name is empty")
        result.append(OverlayExport(name, callback_offset))
    raise FormatError("overlay export table has no terminator")


def psx_executable_exports(data: bytes) -> tuple[OverlayExport, ...]:
    """Decode the resident callback table at the start of a PS-X EXE image."""

    if len(data) < 0x808 or data[:8] != b"PS-X EXE":
        raise FormatError("target runtime is not a PS-X EXE")
    load_address = int.from_bytes(data[0x18:0x1C], "little")
    image_size = int.from_bytes(data[0x1C:0x20], "little")
    if load_address == 0 or image_size == 0 or 0x800 + image_size > len(data):
        raise FormatError("PS-X EXE has an invalid load image")

    def image_offset(address: int) -> int:
        relative = address - load_address
        if not 0 <= relative < image_size:
            raise FormatError(
                f"resident export address 0x{address:08X} is outside the image"
            )
        return 0x800 + relative

    result: list[OverlayExport] = []
    cursor = 0x800
    for _ in range(0x1000):
        name_address, callback_address = struct.unpack_from("<II", data, cursor)
        cursor += 8
        if name_address == 0:
            return tuple(result)
        name_offset = image_offset(name_address)
        callback_offset = image_offset(callback_address) - 0x800
        end = data.find(b"\0", name_offset, 0x800 + image_size)
        if end < 0:
            raise FormatError("resident export name is unterminated")
        try:
            name = data[name_offset:end].decode("ascii")
        except UnicodeDecodeError as error:
            raise FormatError("resident export name is not ASCII") from error
        if not name:
            raise FormatError("resident export name is empty")
        result.append(OverlayExport(name, callback_offset))
    raise FormatError("resident export table has no terminator")


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
    data: bytes,
    offset: int,
    index: int,
    end: int,
    *,
    archive_storage_is_tmem_order: bool = True,
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
            data[palette_end:pixel_end],
            row_bytes,
            row_stride,
            height,
            undo_odd_row_half_swap=archive_storage_is_tmem_order,
        ),
    )


def _decode_n64_texture_rows(
    packed: bytes,
    row_bytes: int,
    row_stride: int,
    height: int,
    *,
    undo_odd_row_half_swap: bool = True,
) -> bytes:
    """Return logical image rows and remove archive padding.

    Runtime evidence proves two storage paths. Standalone XOBF/XRTP images are
    uploaded directly with ``LoadBlock DXT=0``, so their archive bytes already
    carry the odd-row TMEM half swap and must be undone. The large XBMP terrain
    atlas is stored linearly; the game extracts and swaps its 32x32 tiles into
    a separate runtime bank. Callers identify which archive contract applies.
    """

    output = bytearray()
    for row in range(height):
        source = packed[row * row_stride:(row + 1) * row_stride]
        if undo_odd_row_half_swap and row & 1:
            source = b"".join(
                source[offset + 4:offset + 8]
                + source[offset:offset + 4]
                for offset in range(0, row_stride, 8)
            )
        output.extend(source[:row_bytes])
    return bytes(output)


def _encode_psx_indexed(
    width: int,
    height: int,
    palette: Iterable[int],
    indices: bytes,
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
    if len(indices) != width * height:
        raise FormatError(
            f"{'CI8' if ci8 else 'CI4'} texture has {len(indices)} indices "
            f"for {width}x{height}"
        )
    words = (width + 1) // 2 if ci8 else (width + 3) // 4
    row_stride = words * 2
    pixels = bytearray(row_stride * height)
    for row in range(height):
        source = row * width
        destination = row * row_stride
        if ci8:
            pixels[destination:destination + width] = (
                indices[source:source + width]
            )
            continue
        for column in range(width):
            value = indices[source + column]
            if value >= 16:
                raise FormatError(
                    f"CI4 texture index {value} exceeds its palette"
                )
            pixels[destination + column // 2] |= (
                value << (4 if column & 1 else 0)
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
        row_bytes = (texture.width + 1) // 2
        indices = bytes(
            (
                texture.pixels[
                    row * row_bytes + column // 2
                ] >> (0 if column & 1 else 4)
            ) & 0x0F
            for row in range(texture.height)
            for column in range(texture.width)
        )
        return _encode_psx_indexed(
            texture.width,
            texture.height,
            palette,
            indices,
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


def texture_has_nonblack_alpha_cutout(
    texture: N64Texture, palette_bank: int = 0
) -> bool:
    """Return whether authored texels require PSX color-zero transparency.

    N64 RGBA5551 can retain RGB in a texel whose one-bit alpha is clear.
    PSX indexed textures instead express a fully transparent texel with
    palette word zero.  Dreamland uses the former convention for foliage,
    fences, vegetable leaves, sprites, and other cutout materials.  Opaque
    terrain imagery also contains alpha-clear palette entries, so this test
    is deliberately based on *used* texels and requires both visible alpha
    values plus at least one non-black alpha-clear texel.
    """

    colors: list[int] = []
    if texture.format == 2 and texture.size == 0:
        first = palette_bank * 16
        palette = texture.palette_rgba5551[first:first + 16]
        if not palette:
            return False
        row_bytes = (texture.width + 1) // 2
        for row in range(texture.height):
            for column in range(texture.width):
                index = (
                    texture.pixels[row * row_bytes + column // 2]
                    >> (0 if column & 1 else 4)
                ) & 0x0F
                if index < len(palette):
                    colors.append(palette[index])
    elif (
        texture.format == 2
        and texture.size == 1
        and texture.palette_rgba5551
    ):
        colors = [
            texture.palette_rgba5551[index]
            for index in texture.pixels
            if index < len(texture.palette_rgba5551)
        ]
    elif texture.format == 0 and texture.size == 2:
        colors = [
            be16(texture.pixels, offset)
            for offset in range(0, len(texture.pixels), 2)
        ]
    elif texture.format == 0 and texture.size == 3:
        colors = [
            (
                ((texture.pixels[offset] >> 3) << 11)
                | ((texture.pixels[offset + 1] >> 3) << 6)
                | ((texture.pixels[offset + 2] >> 3) << 1)
                | (1 if texture.pixels[offset + 3] else 0)
            )
            for offset in range(0, len(texture.pixels), 4)
        ]
    if not colors:
        return False
    has_visible = any(color & 1 for color in colors)
    has_nonblack_transparent = any(
        not (color & 1) and (color & 0xFFFE) != 0
        for color in colors
    )
    return has_visible and has_nonblack_transparent


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
    row_bytes = width
    row_stride = align(row_bytes, 8)
    expected = 8 + 512 + row_stride * height
    if len(payload) < expected:
        raise FormatError("N64 CI8 image is truncated")
    palette = [be16(payload, 8 + index * 2) for index in range(256)]
    # Archive rows are logical image order. The N64 runtime's odd-row swap is
    # performed only when it repacks a tile for TMEM and must not be preserved
    # in a PS1 bitmap.
    pixels = _decode_n64_texture_rows(
        payload[0x208:0x208 + row_stride * height],
        row_bytes,
        row_stride,
        height,
        undo_odd_row_half_swap=False,
    )
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

    TINF contains 256 fixed 0x28-byte material records.  LOAD 80105550 reads
    the render origin from words +2 and +4, then copies the seven halfwords at
    +8..+20 into the 0x20-byte runtime material record.  Comparing matching
    Airgrave N64 and PS1 tables proves that only the render origin needs the
    exact ``coordinate * 3 / 2`` conversion; the remaining behavior words must
    stay source-authored.
    """

    if len(payload) % 0x28:
        raise FormatError("TINF is not an array of 0x28-byte records")
    result = bytearray(payload)
    for record in range(0, len(result), 0x28):
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
    combiner: tuple[int, int] = (0, 0)
    z_compare: bool = False
    z_update: bool = False
    z_buffer_geometry: bool | None = None


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


def _update_other_mode(current: int, word0: int, word1: int) -> int:
    """Apply one F3DEX2 G_SETOTHERMODE field update."""

    size = (word0 & 0xFF) + 1
    offset = max(0, 32 - ((word0 >> 8) & 0xFF) - size)
    mask = ((1 << size) - 1) << offset
    return (current & ~mask) | (word1 & mask)


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
        combiner = (0, 0)
        semi_transparent = False
        other_mode_l = 0
        geometry_mode = 0
        geometry_known = 0

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
                    combiner,
                    bool(other_mode_l & 0x0010),
                    bool(other_mode_l & 0x0020),
                    (
                        bool(geometry_mode & 0x00000001)
                        if geometry_known & 0x00000001
                        else None
                    ),
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
                    # XOBF texture table.  Preserve the distinction so the
                    # PS1 packet encoder can target an engine-global texture
                    # rather than an ordinary converted texture slot.
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
            if opcode == 0xFC:
                combiner = (word0 & 0x00FFFFFF, word1)
                continue
            if opcode == 0xE2:
                # V8's N64 translucent surface blender word ends in 0x49D8.
                # An alternate display list is not inherently translucent:
                # Airgrave and Dreamland also place opaque/emissive geometry
                # there, so derive the PSX flag from the authored RDP state.
                other_mode_l = _update_other_mode(
                    other_mode_l, word0, word1
                )
                semi_transparent = (other_mode_l & 0xFFFF) == 0x49D8
                continue
            if opcode == 0xD9:
                # F3DEX2 G_GEOMETRYMODE encodes a clear mask in word zero and
                # set bits in word one.  Water trench side/top groups clear
                # G_CULL_BACK (0x400); their PS1 counterparts duplicate the
                # reversed triangle because the PS1 has no equivalent
                # two-sided polygon bit.
                # F3DEX2 geometry-mode bits are distinct: 0x10000 enables
                # depth fog, while 0x20000 enables vertex lighting.  Treating
                # G_FOG as G_LIGHTING made almost all Dreamland world faces
                # consume their RGB shade attributes as signed normals and
                # produced a bright yellow/brown cast after PS1 conversion.
                clear_mask = word0 & 0x00FFFFFF
                geometry_mode = (geometry_mode & clear_mask) | word1
                geometry_known = (
                    (geometry_known & clear_mask)
                    | ((~clear_mask) & 0x00FFFFFF)
                    | word1
                )
                if not (word0 & 0x20000):
                    lighting_enabled = False
                if word1 & 0x20000:
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
                0xF0, 0xF3, 0xFB,
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

    def packet_type(kind: int, *, textured: bool, semi_transparent: bool) -> int:
        """Encode the on-disk discriminator consumed by FUN_8001a640."""

        # The loose XOBF stores the packet family in the low nibble.
        # FUN_8001a640 expands that nibble into bits 2..5 in-place before
        # FUN_8001b49c builds the cached renderer packets.  Bit 0x10 is the
        # native source semitransparency flag; texturing is determined by the
        # packet family itself.
        _ = textured
        return kind | (0x10 if semi_transparent else 0)

    def normal_index(value: tuple[int, int, int]) -> int:
        index = normal_indices.get(value)
        if index is not None:
            return index
        index = len(normal_indices)
        normal_indices[value] = index
        normals.extend(struct.pack("<hhhh", value[0], value[1], value[2], 0))
        return index

    def psx_lit_texture_base(face: Face) -> tuple[int, int, int]:
        """Translate the RDP's 0..255 shade modulation to PS1 GPU scale.

        Dreamland's ordinary lit models use an RDP ``TEXEL0 * SHADE``
        combiner.  Runtime traces prove that a neutral 127 PS1 GTE base emits
        the same 0..255 shade values as the N64 RSP, but the PS1 GPU then
        modulates texture texels with 128 as neutral.  Feeding the RSP-scale
        result to that path therefore doubles the source lighting.  A neutral
        base of 64 supplies the required 128/255 bridge before the native GTE
        and preserves the authored N64 primitive colour, which these two
        combiners do not consume, for every other material mode.
        """

        if face.combiner in (
            (0x127FFF, 0xFFFFF238),
            (0x127E24, 0xFFFFF3F9),
        ):
            return (64, 64, 64)
        return face.color

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
            # Preserve the N64 engine-global texture reference in the native
            # PS1 kind-13 source layout.  FUN_8001b49c treats texture id
            # 0xffff as the engine-owned descriptor at DAT_80065a28; the
            # upper 0x4000 bits on an ordinary texture id are only PS1
            # tpage/CLUT mode flags and do not select dynamic imagery.
            uvs = face.uv

            def dynamic_packet(order: tuple[int, int, int]) -> bytes:
                return (
                    struct.pack(
                        "<BBBBHHHH",
                        128, 128, 128,
                        packet_type(
                            13,
                            textured=True,
                            semi_transparent=face.semi_transparent,
                        ),
                        indices[order[0]], indices[order[1]], indices[order[2]],
                        0,
                    )
                    + bytes(uvs[order[0]])
                    + bytes(uvs[order[1]])
                    + bytes(uvs[order[2]])
                    + struct.pack("<H", 0xFFFF)
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
                        128, 128, 128,
                        packet_type(
                            13, textured=True, semi_transparent=True
                        ),
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
        lit_texture_color = psx_lit_texture_base(face) if lit else color

        for order in orders:
            ordered_indices = tuple(indices[item] for item in order)
            if face.texture is None or debug_untextured:
                if lit:
                    if flat_normal:
                        packets += struct.pack(
                            "<BBBBHHHH",
                            color[0], color[1], color[2],
                            packet_type(
                                4,
                                textured=False,
                                semi_transparent=face.semi_transparent,
                            ),
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
                            packet_type(
                                8,
                                textured=False,
                                semi_transparent=face.semi_transparent,
                            ),
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
                        packet_type(
                            0,
                            textured=False,
                            semi_transparent=face.semi_transparent,
                        ),
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
                        lit_texture_color[0],
                        lit_texture_color[1],
                        lit_texture_color[2],
                        packet_type(
                            9,
                            textured=True,
                            semi_transparent=face.semi_transparent,
                        ),
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
                        lit_texture_color[0],
                        lit_texture_color[1],
                        lit_texture_color[2],
                        packet_type(
                            packet_kind,
                            textured=True,
                            semi_transparent=face.semi_transparent,
                        ),
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
    rooted_variants: int = 0
    terrain_clipped_faces: int = 0
    terrain_discarded_faces: int = 0


@dataclass(frozen=True)
class RootedInstance:
    """One source OBJ placement eligible for static terrain adaptation."""

    key: int
    root_slot: int
    flags: int
    position: tuple[int, int, int]
    rotation_xyz: tuple[int, int, int]


@dataclass(frozen=True)
class _SourceSlot:
    render_key: int
    obstacle_index: int
    position: tuple[int, int, int]
    rotation_yxz_storage: tuple[int, int, int]
    flags: int
    next_sibling: int
    first_child: int


@dataclass(frozen=True)
class _Transform:
    rotation: tuple[tuple[float, float, float], ...]
    position: tuple[float, float, float]


@dataclass(frozen=True)
class _ClipVertex:
    position: tuple[float, float, float]
    st: tuple[float, float]
    color: tuple[float, float, float, float]
    uv: tuple[float, float]
    source_offset: int | None


def _matrix_yxz(
    rotation_xyz: tuple[int, int, int],
) -> tuple[tuple[float, float, float], ...]:
    """Float expression of the retail RotMatrixYXZ_gte coefficient order."""

    x, y, z = (value * math.tau / 4096.0 for value in rotation_xyz)
    sx, cx = math.sin(x), math.cos(x)
    sy, cy = math.sin(y), math.cos(y)
    sz, cz = math.sin(z), math.cos(z)
    return (
        (cy * cz + sz * sy * sx, cz * sy * sx - cy * sz, cx * sy),
        (sz * cx, cz * cx, -sx),
        (sz * cy * sx - sy * cz, cz * cy * sx + sy * sz, cx * cy),
    )


def _matrix_multiply(
    left: tuple[tuple[float, float, float], ...],
    right: tuple[tuple[float, float, float], ...],
) -> tuple[tuple[float, float, float], ...]:
    return tuple(
        tuple(
            sum(left[row][item] * right[item][column] for item in range(3))
            for column in range(3)
        )
        for row in range(3)
    )


def _matrix_rotate(
    matrix: tuple[tuple[float, float, float], ...],
    vector: tuple[float, float, float],
) -> tuple[float, float, float]:
    return tuple(
        sum(matrix[row][column] * vector[column] for column in range(3))
        for row in range(3)
    )  # type: ignore[return-value]


def _compose_transform(parent: _Transform, local: _Transform) -> _Transform:
    offset = _matrix_rotate(parent.rotation, local.position)
    return _Transform(
        _matrix_multiply(parent.rotation, local.rotation),
        tuple(parent.position[index] + offset[index] for index in range(3)),
    )


def _source_slots(data: bytes, count: int) -> list[_SourceSlot]:
    result: list[_SourceSlot] = []
    for index in range(count):
        offset = 0x1C + index * 0x1C
        result.append(
            _SourceSlot(
                render_key=be16(data, offset),
                obstacle_index=be16(data, offset + 2, signed=True),
                position=tuple(
                    be32(data, offset + item, signed=True)
                    for item in (4, 8, 12)
                ),
                rotation_yxz_storage=tuple(
                    be16(data, offset + item, signed=True)
                    for item in (16, 18, 20)
                ),
                flags=be16(data, offset + 22, signed=True),
                next_sibling=be16(data, offset + 24, signed=True),
                first_child=be16(data, offset + 26, signed=True),
            )
        )
    return result


def _render_group(render_key: int, group_count: int) -> int | None:
    raw = render_key & 0xFFFF
    if (
        raw >> 12 in {8, 9}
        or 0x8000 <= raw <= 0x8005
        or 0x8010 <= raw <= 0x8016
        or raw == 0x801F
        or 0x8040 <= raw <= 0x8043
        or 0x8100 <= raw <= 0x8102
    ):
        return None
    group = raw & 0x07FF
    return group if group < group_count else None


def _instance_slots(
    slots: list[_SourceSlot], instance: RootedInstance
) -> list[tuple[int, _Transform]]:
    if not 0 <= instance.root_slot < len(slots):
        return []
    root = _Transform(
        _matrix_yxz(instance.rotation_xyz),
        tuple(float(value) for value in instance.position),
    )
    result: list[tuple[int, _Transform]] = []
    seen: set[int] = set()

    def visit(index: int, parent: _Transform, include_sibling: bool) -> None:
        if index < 0 or index >= len(slots) or index in seen:
            return
        seen.add(index)
        slot = slots[index]
        transform = parent if index == instance.root_slot else _compose_transform(
            parent,
            _Transform(
                # The loader copies the stored Y/X/Z halfwords verbatim into
                # the SVECTOR consumed by RotMatrixYXZ_gte.
                _matrix_yxz(slot.rotation_yxz_storage),
                tuple(float(value) for value in slot.position),
            ),
        )
        result.append((index, transform))
        if slot.first_child >= 0:
            visit(slot.first_child, transform, True)
        if include_sibling and slot.next_sibling >= 0:
            visit(slot.next_sibling, parent, True)

    visit(instance.root_slot, root, False)
    return result


def _clip_point_distance(
    point: _ClipVertex,
    transforms: tuple[_Transform, ...],
    scale_shift: int,
    zmap: bytes | None,
    zones: list[bytes] | None,
) -> float:
    scale = 65536.0 / float(1 << scale_shift)
    local = tuple(value * scale for value in point.position)
    distances = []
    for transform in transforms:
        rotated = _matrix_rotate(transform.rotation, local)
        world = tuple(
            transform.position[index] + rotated[index] for index in range(3)
        )
        terrain_y = (
            0
            if zmap is None or zones is None
            else _terrain_height_from_source(
                round(world[0]), round(world[2]), zmap, zones
            )
        )
        # PS1 world Y increases downward; positive distance is buried. The
        # maximum forms one shared geometry envelope safe at every placement.
        distances.append(world[1] - terrain_y)
    return max(distances, default=-math.inf)


def _interpolate_clip_vertex(
    first: _ClipVertex, second: _ClipVertex, amount: float
) -> _ClipVertex:
    def values(left: tuple[float, ...], right: tuple[float, ...]) -> tuple[float, ...]:
        return tuple(
            left[index] + (right[index] - left[index]) * amount
            for index in range(len(left))
        )

    return _ClipVertex(
        values(first.position, second.position),  # type: ignore[arg-type]
        values(first.st, second.st),  # type: ignore[arg-type]
        values(first.color, second.color),  # type: ignore[arg-type]
        values(first.uv, second.uv),  # type: ignore[arg-type]
        None,
    )


def _terrain_intersection(
    first: _ClipVertex,
    second: _ClipVertex,
    first_distance: float,
    transforms: tuple[_Transform, ...],
    scale_shift: int,
    zmap: bytes | None,
    zones: list[bytes] | None,
) -> _ClipVertex:
    low = 0.0
    high = 1.0
    first_inside = first_distance <= 0.0
    for _ in range(32):
        middle = (low + high) * 0.5
        point = _interpolate_clip_vertex(first, second, middle)
        inside = _clip_point_distance(
            point, transforms, scale_shift, zmap, zones
        ) <= 0.0
        if inside == first_inside:
            low = middle
        else:
            high = middle
    return _interpolate_clip_vertex(first, second, (low + high) * 0.5)


def _clip_group_to_terrain(
    group: tuple[list[Vertex], list[Face], int, int, int],
    transforms: tuple[_Transform, ...],
    zmap: bytes | None,
    zones: list[bytes] | None,
    generated_offset: list[int],
) -> tuple[tuple[list[Vertex], list[Face], int, int, int], int, int]:
    vertices, faces, scale_shift, auxiliary_count, native_extent = group
    output_faces: list[Face] = []
    clipped_count = 0
    discarded_count = 0

    for face in faces:
        if not (face.z_compare and face.z_update):
            output_faces.append(face)
            continue
        polygon = [
            _ClipVertex(
                tuple(float(value) for value in vertex_position),
                (float(vertex.s), float(vertex.t)),
                tuple(float(value) for value in vertex.color),
                tuple(float(value) for value in uv),
                vertex.source_offset,
            )
            for vertex, vertex_position, uv in zip(
                face.vertices,
                ((item.x, item.y, item.z) for item in face.vertices),
                face.uv,
            )
        ]
        distances = [
            _clip_point_distance(
                point, transforms, scale_shift, zmap, zones
            )
            for point in polygon
        ]
        inside = [value <= 0.0 for value in distances]
        if all(inside):
            output_faces.append(face)
            continue
        if not any(inside):
            discarded_count += 1
            continue

        clipped_count += 1
        clipped: list[_ClipVertex] = []
        previous = polygon[-1]
        previous_distance = distances[-1]
        previous_inside = previous_distance <= 0.0
        for current, current_distance in zip(polygon, distances):
            current_inside = current_distance <= 0.0
            if current_inside != previous_inside:
                clipped.append(
                    _terrain_intersection(
                        previous,
                        current,
                        previous_distance,
                        transforms,
                        scale_shift,
                        zmap,
                        zones,
                    )
                )
            if current_inside:
                clipped.append(current)
            previous = current
            previous_distance = current_distance
            previous_inside = current_inside

        converted_vertices: list[Vertex] = []
        converted_uv: list[tuple[int, int]] = []
        for point in clipped:
            source_offset = point.source_offset
            if source_offset is None:
                source_offset = generated_offset[0]
                generated_offset[0] -= 1
            converted_vertices.append(
                Vertex(
                    *(
                        max(-32768, min(32767, _round_half_away(value)))
                        for value in point.position
                    ),
                    *(
                        max(-32768, min(32767, _round_half_away(value)))
                        for value in point.st
                    ),
                    tuple(
                        max(0, min(255, _round_half_away(value)))
                        for value in point.color
                    ),
                    source_offset,
                )
            )
            converted_uv.append(
                tuple(
                    max(0, min(255, _round_half_away(value)))
                    for value in point.uv
                )  # type: ignore[arg-type]
            )

        for index in range(1, len(converted_vertices) - 1):
            triangle_vertices = (
                converted_vertices[0],
                converted_vertices[index],
                converted_vertices[index + 1],
            )
            triangle_uv = (
                converted_uv[0],
                converted_uv[index],
                converted_uv[index + 1],
            )
            output_faces.append(
                Face(
                    vertices=triangle_vertices,
                    texture=face.texture,
                    palette_bank=face.palette_bank,
                    color=face.color,
                    uv=triangle_uv,
                    lighting=face.lighting,
                    dynamic_texture=face.dynamic_texture,
                    double_sided=face.double_sided,
                    semi_transparent=face.semi_transparent,
                    combiner=face.combiner,
                    z_compare=face.z_compare,
                    z_update=face.z_update,
                    z_buffer_geometry=face.z_buffer_geometry,
                )
            )

    if clipped_count == 0 and discarded_count == 0:
        return group, 0, 0
    output_vertices: list[Vertex] = []
    seen_vertices: set[int] = set()
    for face in output_faces:
        for vertex in face.vertices:
            if vertex.source_offset in seen_vertices:
                continue
            seen_vertices.add(vertex.source_offset)
            output_vertices.append(vertex)
    return (
        (
            output_vertices,
            output_faces,
            scale_shift,
            auxiliary_count,
            native_extent,
        ),
        clipped_count,
        discarded_count,
    )


def convert_xobf_bin(
    data: bytes,
    *,
    rooted_instances: Iterable[RootedInstance] = (),
    animated_slots: frozenset[int] = frozenset(),
    zmap: bytes | None = None,
    zones: list[bytes] | None = None,
) -> tuple[
    bytes,
    XobfReport,
    list[tuple[tuple[int, int, int, int], ...] | None],
]:
    if len(data) < 0x1C:
        raise FormatError("N64 XOBF is truncated")
    group_count = be32(data, 0)
    group_table = be32(data, 4)
    # XOBF +0x08 is the collision-stream count. The N64 source places one
    # additional boundary offset after those entries, pointing at the first
    # byte after the collision region. It is not another stream. The PS1
    # loader relocates exactly count entries with `index < count`.
    collision_count = be32(data, 8)
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

    rooted_instances = tuple(rooted_instances)
    if rooted_instances and (zmap is None or not zones):
        raise ValueError("rooted instances require ZMAP and ZONE terrain")
    source_slots = _source_slots(data, slot_count)
    slot_records = [
        bytearray(convert_slot(data[0x1C + index * 0x1C:0x38 + index * 0x1C]))
        for index in range(slot_count)
    ]
    rooted_variant_count = 0
    terrain_clipped_faces = 0
    terrain_discarded_faces = 0
    generated_offset = [-1]
    instances_by_root: dict[int, list[RootedInstance]] = {}
    for instance in rooted_instances:
        instances_by_root.setdefault(instance.root_slot, []).append(instance)
    patched_slots: dict[int, tuple[tuple[float, ...], int]] = {}

    for root_slot, instances in sorted(instances_by_root.items()):
        placement_nodes = [
            _instance_slots(source_slots, instance) for instance in instances
        ]
        if not placement_nodes or any(not nodes for nodes in placement_nodes):
            continue
        node_indices = tuple(index for index, _transform in placement_nodes[0])
        if any(
            tuple(index for index, _transform in nodes) != node_indices
            for nodes in placement_nodes[1:]
        ):
            raise FormatError(
                f"rooted template {root_slot} has placement-dependent topology"
            )
        if any(slot_index in animated_slots for slot_index in node_indices):
            # An animated subtree can leave its authored terrain relationship;
            # retain the source geometry instead of baking a static mask.
            continue
        template_changed = False
        for node_order, slot_index in enumerate(node_indices):
            transforms = tuple(
                nodes[node_order][1] for nodes in placement_nodes
            )
            group_index = _render_group(
                source_slots[slot_index].render_key, group_count
            )
            if group_index is None:
                continue
            context = tuple(
                round(value, 6)
                for transform in transforms
                for row in transform.rotation
                for value in row
            ) + tuple(
                round(value, 3)
                for transform in transforms
                for value in transform.position
            )
            existing = patched_slots.get(slot_index)
            if existing is not None:
                if existing[0] != context:
                    raise FormatError(
                        f"rooted slot {slot_index} has incompatible local planes"
                    )
                continue
            variant, clipped, discarded = _clip_group_to_terrain(
                parsed_groups[group_index],
                transforms,
                zmap,
                zones,
                generated_offset,
            )
            if not (clipped or discarded):
                continue
            if len(parsed_groups) >= 0x100:
                raise FormatError(
                    "terrain-adapted model exceeds V8's 8-bit group capacity"
                )
            variant_index = len(parsed_groups)
            parsed_groups.append(variant)
            raw_key = struct.unpack_from("<H", slot_records[slot_index], 0)[0]
            raw_key = (raw_key & 0xFF00) | variant_index
            struct.pack_into("<H", slot_records[slot_index], 0, raw_key)
            patched_slots[slot_index] = (context, variant_index)
            template_changed = True
            terrain_clipped_faces += clipped
            terrain_discarded_faces += discarded
        if template_changed:
            rooted_variant_count += 1

    output_group_count = len(parsed_groups)
    output_slot_count = len(slot_records)
    face_count = sum(len(group[1]) for group in parsed_groups)

    base_alpha_zero = {
        texture.index: texture_has_nonblack_alpha_cutout(texture)
        for texture in textures
    }
    variant_indices = {
        (texture.index, 0, base_alpha_zero[texture.index]): texture.index
        for texture in textures
    }
    texture_variants: list[tuple[N64Texture, int, bool]] = [
        (texture, 0, base_alpha_zero[texture.index])
        for texture in textures
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
            alpha_zero_transparent = (
                face.semi_transparent
                or texture_has_nonblack_alpha_cutout(texture, bank)
            )
            key = (face.texture, bank, alpha_zero_transparent)
            if key not in variant_indices:
                variant_indices[key] = len(texture_variants)
                texture_variants.append(
                    (texture, bank, alpha_zero_transparent)
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
            alpha_zero_transparent = (
                face.semi_transparent
                or texture_has_nonblack_alpha_cutout(texture, bank)
            )
            remapped.append(
                Face(
                    vertices=face.vertices,
                    texture=variant_indices[
                        (face.texture, bank, alpha_zero_transparent)
                    ],
                    palette_bank=0,
                    color=face.color,
                    uv=face.uv,
                    lighting=face.lighting,
                    dynamic_texture=face.dynamic_texture,
                    double_sided=face.double_sided,
                    semi_transparent=face.semi_transparent,
                    combiner=face.combiner,
                    z_compare=face.z_compare,
                    z_update=face.z_update,
                    z_buffer_geometry=face.z_buffer_geometry,
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
    for record in slot_records:
        output += record
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
        output_group_count, group_table_out,
        collision_count, collision_table_out,
        len(texture_blobs), texture_table_out,
        output_slot_count,
    )
    slot_vertex_defaults: list[
        tuple[tuple[int, int, int, int], ...] | None
    ] = []
    for record in slot_records:
        render_key = struct.unpack_from("<H", record, 0)[0] & 0x07FF
        if render_key >= output_group_count:
            slot_vertex_defaults.append(None)
            continue
        vertices = parsed_groups[render_key][0]
        slot_vertex_defaults.append(
            tuple((vertex.x, vertex.y, vertex.z, 0) for vertex in vertices)
        )
    return bytes(output), XobfReport(
        output_group_count,
        face_count,
        len(texture_blobs),
        collision_count,
        output_slot_count,
        rooted_variant_count,
        terrain_clipped_faces,
        terrain_discarded_faces,
    ), slot_vertex_defaults


def convert_aimp(payload: bytes) -> bytes:
    if len(payload) & 1:
        raise FormatError("AIMP is not halfword-aligned")
    return b"".join(
        payload[offset:offset + 2][::-1]
        for offset in range(0, len(payload), 2)
    )


def convert_xrtp(payload: bytes) -> bytes:
    if len(payload) < 12:
        return payload
    # XRTP's descriptor layout is shared, but its platform renderer flags are
    # not.  Dreamland's source 0x0192 road descriptor reaches the N64 RDP as
    # opaque Z-tested, Z-writing triangles; copying bit 0x0100 verbatim makes
    # the PS1 loader emit semitransparent primitives instead.  That false
    # blend both changes the road colour and prevents it from occluding water.
    descriptor = bytearray(payload[:12])
    flags = be16(descriptor, 10) & ~0x0100
    struct.pack_into(">H", descriptor, 10, flags)
    if len(payload) == 12:
        return bytes(descriptor)
    texture = parse_n64_texture(payload, 12, 0, len(payload))
    return bytes(descriptor) + encode_psx_texture(texture)


def convert_animation(
    payload: bytes,
    slot_count: int,
    slot_vertex_defaults: list[
        tuple[tuple[int, int, int, int], ...] | None
    ] | None = None,
    *,
    source_slot_count: int | None = None,
) -> bytes:
    """Convert N64 ANM streams and adapt partial vertex morph tables.

    N64 flag-0x40 keyframes may contain only the leading vertices that change;
    its renderer retains the static tail. The PS1 animation path replaces the
    mesh vertex pointer outright, so those keyframes must be expanded with the
    untouched source vertices. Stream-relative loop offsets are rebuilt after
    expansion.
    """

    if source_slot_count is None:
        source_slot_count = slot_count
    if not 0 <= source_slot_count <= slot_count:
        raise FormatError("animation source slot count exceeds output slots")
    source_table_size = 4 + source_slot_count * 4
    table_size = 4 + slot_count * 4
    if len(payload) < source_table_size:
        raise FormatError("N64 ANM table is truncated")
    offsets = [
        be32(payload, 4 + index * 4) for index in range(source_slot_count)
    ]
    for index, offset in enumerate(offsets):
        if offset and not (source_table_size <= offset < len(payload)):
            raise FormatError(f"N64 ANM slot {index} has invalid offset 0x{offset:X}")
    if slot_vertex_defaults is None:
        slot_vertex_defaults = [None] * slot_count
    if len(slot_vertex_defaults) != slot_count:
        raise FormatError("animation vertex-default table has the wrong size")

    unique_starts = sorted(set(offsets) - {0})
    streams: dict[int, bytes] = {}
    for start_index, start in enumerate(unique_starts):
        end = (
            unique_starts[start_index + 1]
            if start_index + 1 < len(unique_starts)
            else len(payload)
        )
        defaults = next(
            (
                slot_vertex_defaults[slot]
                for slot, offset in enumerate(offsets)
                if offset == start and slot_vertex_defaults[slot] is not None
            ),
            None,
        )
        cursor = start
        stream = bytearray()
        record_offsets: dict[int, int] = {}
        loop_patches: list[tuple[int, int, int]] = []
        while cursor + 4 <= end:
            record_start = cursor
            record_offsets[record_start] = len(stream)
            frame_delta = be16(payload, cursor, signed=True)
            flags = be16(payload, cursor + 2, signed=True)
            cursor += 4
            stream += struct.pack("<hh", frame_delta, 0 if flags < 0 else flags)
            if flags < 0:
                loop_patches.append(
                    (len(stream) - 2, record_start, record_start + flags)
                )
                continue
            if flags & ~0x7B:
                raise FormatError(
                    f"N64 ANM stream 0x{start:X} uses unsupported "
                    f"flags 0x{flags:04X}"
                )
            if flags & 0x01:
                values = [
                    be16(payload, cursor + item * 2, signed=True)
                    for item in range(4)
                ]
                stream += struct.pack("<hhhh", *values)
                cursor += 8
            if flags & 0x02:
                values = [
                    be32(payload, cursor + item * 4, signed=True)
                    for item in range(3)
                ]
                stream += struct.pack("<iii", *values)
                cursor += 12
            if flags & 0x08:
                values = [
                    be16(payload, cursor + item * 2, signed=True)
                    for item in range(4)
                ]
                stream += struct.pack("<hhhh", *values)
                cursor += 8
            if flags & 0x10:
                while True:
                    target = be16(payload, cursor)
                    texture = be16(payload, cursor + 2)
                    stream += struct.pack("<HH", target, texture)
                    cursor += 4
                    if target & 0x8000:
                        break
            if flags & 0x20:
                values = [
                    be16(payload, cursor + item * 2, signed=True)
                    for item in range(4)
                ]
                stream += struct.pack("<hhhh", *values)
                cursor += 8
            if flags & 0x40:
                count = be32(payload, cursor)
                cursor += 4
                values = [
                    tuple(
                        be16(
                            payload,
                            cursor + record * 8 + item * 2,
                            signed=True,
                        )
                        for item in range(4)
                    )
                    for record in range(count)
                ]
                cursor += count * 8
                if defaults is not None:
                    if count > len(defaults):
                        raise FormatError(
                            f"N64 ANM morph has {count} vertices but its "
                            f"model has only {len(defaults)}"
                        )
                    values.extend(defaults[count:])
                stream += struct.pack("<I", len(values))
                stream += b"".join(
                    struct.pack("<hhhh", *vertex) for vertex in values
                )
            if cursor > end:
                raise FormatError(
                    f"N64 ANM stream 0x{start:X} crosses its boundary"
                )

        for patch, source_record, source_target in loop_patches:
            if source_target not in record_offsets:
                raise FormatError(
                    f"N64 ANM loop at 0x{source_record:X} targets "
                    f"non-record 0x{source_target:X}"
                )
            relative = (
                record_offsets[source_target] - record_offsets[source_record]
            )
            if not -32768 <= relative <= 32767:
                raise FormatError("expanded ANM loop exceeds signed 16-bit range")
            struct.pack_into("<h", stream, patch, relative)
        streams[start] = bytes(stream)

    output = bytearray(b"\0" * table_size)
    struct.pack_into("<I", output, 0, be32(payload, 0))
    relocated: dict[int, int] = {}
    for start in unique_starts:
        while len(output) < align(len(output), 4):
            output.append(0)
        relocated[start] = len(output)
        output += streams[start]
    for slot, offset in enumerate(offsets):
        struct.pack_into(
            "<I", output, 4 + slot * 4,
            0 if offset == 0 else relocated[offset],
        )
    return bytes(output)


def _terrain_height_from_source(
    x: int, z: int, zmap: bytes, zones: list[bytes]
) -> int:
    """Evaluate the PS1 terrain sampler directly from semantic N64 chunks."""

    def sample(cell_x: int, cell_z: int) -> int:
        map_offset = (((cell_z >> 6) * 32) + (cell_x >> 6)) * 2
        if map_offset + 2 > len(zmap):
            return 0
        zone_index = be16(zmap, map_offset)
        if zone_index == 0:
            return 0
        if zone_index > len(zones):
            raise FormatError(
                f"ZMAP refers to missing ZONE {zone_index} "
                f"(available={len(zones)})"
            )
        zone = zones[zone_index - 1]
        offset = (((cell_x & 0x3F) * 64) + (cell_z & 0x3F)) * 4
        if offset + 4 > len(zone):
            raise FormatError("ZONE cell crosses its payload")
        # LOAD 801057f0 performs this exact source-to-runtime conversion.
        value = (
            (be16(zone, offset) - 0x0200)
            | ((zone[offset + 2] >> 3) << 11)
        )
        return value & 0x07FF

    fraction_x = x & 0xFFFF
    fraction_z = z & 0xFFFF
    cell_x = x >> 16
    cell_z = z >> 16
    h00 = sample(cell_x, cell_z)
    if fraction_x + fraction_z < 0x10000:
        h10 = sample(cell_x + 1, cell_z)
        h01 = sample(cell_x, cell_z + 1)
        accum = (
            h00 * 0x10000
            + fraction_x * (h10 - h00)
            + fraction_z * (h01 - h00)
        )
    else:
        h11 = sample(cell_x + 1, cell_z + 1)
        h10 = sample(cell_x + 1, cell_z)
        h01 = sample(cell_x, cell_z + 1)
        accum = (
            h11 * 0x10000
            + (0x10000 - fraction_x) * (h01 - h11)
            + (0x10000 - fraction_z) * (h10 - h11)
        )
    if accum < 0:
        accum += 0x1F
    return accum >> 5


def _dreamland_water_texture(
    atlas: N64Texture, tile_index: int
) -> N64Texture:
    """Extract one authored 32x32 Dreamland water frame from XBMP."""

    tile_width = 32
    tile_height = 32
    columns = atlas.width // tile_width
    rows = atlas.height // tile_height
    if (
        atlas.format != 2
        or atlas.size != 1
        or atlas.width % tile_width
        or atlas.height % tile_height
        or tile_index < 0
        or tile_index >= columns * rows
    ):
        raise FormatError(
            f"Dreamland water tile 0x{tile_index:X} is outside "
            f"{atlas.width}x{atlas.height} CI8 XBMP"
        )
    tile_x = (tile_index % columns) * tile_width
    tile_y = (tile_index // columns) * tile_height
    pixels = bytearray()
    for row in range(tile_height):
        source = (tile_y + row) * atlas.width + tile_x
        pixels += atlas.pixels[source:source + tile_width]
    return N64Texture(
        index=tile_index,
        source_offset=atlas.source_offset,
        format=atlas.format,
        size=atlas.size,
        palette_count=atlas.palette_count,
        width=tile_width,
        height=tile_height,
        palette_rgba5551=atlas.palette_rgba5551,
        pixels=bytes(pixels),
    )


def _encode_dreamland_water_ci4(
    frames: list[N64Texture],
    *,
    half_coverage: bool = False,
) -> list[bytes]:
    """Quantize each authored water frame independently to native PS1 CI4.

    A CI8 CLUT needs a contiguous 256-word VRAM row which Dreamland cannot
    reserve after its terrain/model textures are loaded. Per-frame weighted
    median cut retains each phase's own hue distribution and is materially
    closer to the N64 source than reducing all five phases into one palette.
    """

    def components(color: int) -> tuple[int, int, int]:
        return (color >> 11) & 31, (color >> 6) & 31, (color >> 1) & 31

    encoded: list[bytes] = []
    for texture in frames:
        if (
            texture.format != 2
            or texture.size != 1
            or len(texture.pixels) != texture.width * texture.height
        ):
            raise FormatError("Dreamland water frame is not tightly packed CI8")

        color_weights: dict[int, int] = {}
        for palette_index in texture.pixels:
            if palette_index >= len(texture.palette_rgba5551):
                raise FormatError(
                    f"Dreamland water palette index {palette_index} is missing"
                )
            color = texture.palette_rgba5551[palette_index]
            color_weights[color] = color_weights.get(color, 0) + 1

        color_limit = 15 if half_coverage else 16
        boxes: list[list[int]] = [sorted(color_weights)]
        while len(boxes) < color_limit:
            candidates = [
                (
                    max(
                        max(components(color)[channel] for color in box)
                        - min(components(color)[channel] for color in box)
                        for channel in range(3)
                    )
                    * sum(color_weights[color] for color in box),
                    index,
                )
                for index, box in enumerate(boxes)
                if len(box) > 1
            ]
            if not candidates:
                break
            _score, box_index = max(candidates)
            box = boxes.pop(box_index)
            ranges = [
                max(components(color)[channel] for color in box)
                - min(components(color)[channel] for color in box)
                for channel in range(3)
            ]
            channel = max(range(3), key=lambda item: (ranges[item], -item))
            ordered = sorted(
                box, key=lambda color: (components(color)[channel], color)
            )
            half = sum(color_weights[color] for color in ordered) / 2
            running = 0
            split = 1
            for split, color in enumerate(ordered, 1):
                running += color_weights[color]
                if running >= half:
                    break
            split = min(max(1, split), len(ordered) - 1)
            boxes.extend((ordered[:split], ordered[split:]))

        # PS1 semi-transparency has no variable source alpha. Dreamland uses
        # alpha 160/255 on N64, while PS1 ABR=0 is fixed at 1/2. A second pass
        # covering exactly half the texels raises the spatially averaged
        # source contribution to 5/8, matching 160/255 within 0.25%. Palette
        # entry zero is the native transparent texel for that coverage pass.
        palette: list[int] = [0] if half_coverage else []
        palette_rgb: list[tuple[int, int, int]] = []
        for box in boxes:
            total = sum(color_weights[color] for color in box)
            rgb = tuple(
                _round_half_away(
                    sum(
                        components(color)[channel] * color_weights[color]
                        for color in box
                    )
                    / total
                )
                for channel in range(3)
            )
            palette_rgb.append(rgb)  # type: ignore[arg-type]
            palette.append(
                (rgb[0] << 11) | (rgb[1] << 6) | (rgb[2] << 1) | 1
            )

        def nearest(color: int) -> int:
            red, green, blue = components(color)
            return min(
                range(len(palette_rgb)),
                key=lambda index: (
                    3 * (red - palette_rgb[index][0]) ** 2
                    + 4 * (green - palette_rgb[index][1]) ** 2
                    + 2 * (blue - palette_rgb[index][2]) ** 2,
                    index,
                ),
            )

        if half_coverage:
            indices = bytes(
                0 if (column + row) & 1 else
                nearest(texture.palette_rgba5551[
                    texture.pixels[row * texture.width + column]
                ]) + 1
                for row in range(texture.height)
                for column in range(texture.width)
            )
        else:
            indices = bytes(
                nearest(texture.palette_rgba5551[pixel])
                for pixel in texture.pixels
            )
        encoded.append(
            _encode_psx_indexed(
                texture.width,
                texture.height,
                palette,
                indices,
                ci8=False,
            )
        )
    return encoded


def _dreamland_water_group(
    width: int,
    height: int,
    texture_targets: tuple[int, ...],
) -> tuple[bytes, int]:
    """Build one native PS1 water patch from authored 32x32 repeats.

    Dreamland advances eight texels per terrain cell. Split patches at four
    cells so every quad uses the complete 0..31 N64 tile without requiring a
    duplicated 64x64 image in VRAM.
    """

    vertices: list[tuple[int, int, int]] = []
    quads: list[tuple[tuple[int, int, int, int], int, int]] = []
    for z0 in range(0, height, 4):
        z1 = min(z0 + 4, height)
        for x0 in range(0, width, 4):
            x1 = min(x0 + 4, width)
            first = len(vertices)
            vertices.extend(
                (
                    ((x0 * 2 - width) << 7, 0, (z0 * 2 - height) << 7),
                    ((x1 * 2 - width) << 7, 0, (z0 * 2 - height) << 7),
                    ((x1 * 2 - width) << 7, 0, (z1 * 2 - height) << 7),
                    ((x0 * 2 - width) << 7, 0, (z1 * 2 - height) << 7),
                )
            )
            quads.append(
                ((first, first + 1, first + 2, first + 3), x1 - x0, z1 - z0)
            )
    for vertex in vertices:
        if any(value < -32768 or value > 32767 for value in vertex):
            raise FormatError("Dreamland water patch exceeds scale-8 XOBF")

    packets = bytearray()
    polygon_count = 0
    for indices, quad_width, quad_height in quads:
        u1 = quad_width * 8 - 1
        v1 = quad_height * 8 - 1
        uv = ((0, 0), (u1, 0), (u1, v1), (0, v1))
        for triangle in (
            (indices[0], indices[3], indices[2]),
            (indices[2], indices[1], indices[0]),
        ):
            reverse_triangle = (
                triangle[2],
                triangle[1],
                triangle[0],
            )
            for texture_target in texture_targets:
                # Casino City's water stores every forward/reverse pair
                # adjacently under one texture descriptor. Preserve that
                # native ordering rather than separating all front and back
                # faces around the additional alpha-coverage pass.
                for oriented_triangle in (triangle, reverse_triangle):
                    packets += struct.pack(
                        "<BBBBHHHH",
                        128, 128, 128, 0x1F,
                        oriented_triangle[0],
                        oriented_triangle[1],
                        oriented_triangle[2],
                        0,
                    )
                    for vertex_index in oriented_triangle:
                        packets += bytes(uv[vertex_index - indices[0]])
                    # Object_PreTickRecurse replaces this descriptor through
                    # native ANM flag-0x10 animation. No upper texture bits
                    # select PS1 blend mode zero (average), matching shipped
                    # PS1 water.
                    packets += struct.pack("<H", texture_target)
                    polygon_count += 1

    encoded_vertices = b"".join(
        struct.pack("<hhhh", x, y, z, 0) for x, y, z in vertices
    )
    extent = math.ceil(
        max(math.sqrt(x * x + y * y + z * z) for x, y, z in vertices)
    )
    polygon_relative = 0x1C
    vertex_relative = polygon_relative + len(packets)
    normal_relative = vertex_relative + len(encoded_vertices)
    return (
        struct.pack(
            "<IIIIHhIBBH",
            len(vertices), vertex_relative,
            0, normal_relative,
            polygon_count, 0,
            # +0x19 is the number of consecutive texture descriptors cached
            # at group construction. The animated packets can resolve any of
            # the ten phase/pass slots, so declaring only slot zero makes the
            # renderer index beyond its descriptor pointer array.
            polygon_relative, 8, 10, extent,
        )
        + packets
        + encoded_vertices
    ), polygon_count


def _dreamland_water_tiles(
    x0: int, z0: int, x1: int, z1: int
) -> list[tuple[int, int, int, int]]:
    tiles: list[tuple[int, int, int, int]] = []
    z = z0
    while z < z1:
        next_z = min(z + 8, z1)
        x = x0
        while x < x1:
            next_x = min(x + 8, x1)
            tiles.append((x, z, next_x, next_z))
            x = next_x
        z = next_z
    return tiles


def _dreamland_water_xobf(
    rect: bytes, xbmp: bytes, zmap: bytes, zones: list[bytes]
) -> tuple[bytes, XobfReport, int, int, int]:
    """Generate Dreamland's RECT effect as independent native PS1 data."""

    if len(rect) != 14:
        raise FormatError(f"Dreamland RECT has unexpected size {len(rect)}")
    x0 = be16(rect, 0)
    z0 = be16(rect, 2)
    x1 = be16(rect, 4)
    z1 = be16(rect, 6)
    selector = be16(rect, 12, signed=True)
    if selector != -1 or x1 <= x0 or z1 <= z0:
        raise FormatError(
            "Dreamland RECT does not match the authored water surface"
        )

    center_x = (x0 + x1) << 15
    center_z = (z0 + z1) << 15
    terrain_y = _terrain_height_from_source(
        center_x, center_z, zmap, zones
    )
    # N64 80159900 writes Terrain_HeightAt(center) - 16 height samples.
    object_y = terrain_y - (16 << 11)

    atlas = parse_n64_texture(
        xbmp,
        0,
        0,
        len(xbmp),
        archive_storage_is_tmem_order=False,
    )
    phase_tiles = (0x23, 0x24, 0x25, 0x26, 0x27)
    source_frames = [
        _dreamland_water_texture(atlas, tile) for tile in phase_tiles
    ]
    textures = _encode_dreamland_water_ci4(source_frames)
    textures += _encode_dreamland_water_ci4(
        source_frames, half_coverage=True
    )

    tiles = _dreamland_water_tiles(x0, z0, x1, z1)
    dimensions: list[tuple[int, int]] = []
    for tile_x0, tile_z0, tile_x1, tile_z1 in tiles:
        size = (tile_x1 - tile_x0, tile_z1 - tile_z0)
        if size not in dimensions:
            dimensions.append(size)
    encoded_groups = [
        _dreamland_water_group(width, height, (0, 5))
        for width, height in dimensions
    ]
    groups = [group for group, _face_count in encoded_groups]
    water_face_count = sum(
        face_count for _group, face_count in encoded_groups
    )
    # Slot zero is a model-free hierarchy root at the exact RECT center.
    # Each water patch is a direct child/sibling with its own native culling
    # bounds. Making the first corner patch the root caused the complete
    # 61x60-cell surface to disappear when that one small patch left view.
    slots = bytearray(
        struct.pack(
            "<hhiiihhhhhh",
            -1, 0,
            0, 0, 0,
            0, 0, 0,
            -21846, -1, 1,
        )
    )
    for index, (tile_x0, tile_z0, tile_x1, tile_z1) in enumerate(tiles):
        tile_center_x = (tile_x0 + tile_x1) << 15
        tile_center_z = (tile_z0 + tile_z1) << 15
        group_index = dimensions.index(
            (tile_x1 - tile_x0, tile_z1 - tile_z0)
        )
        slot_index = index + 1
        next_sibling = slot_index + 1 if index + 1 < len(tiles) else -1
        slots += struct.pack(
            "<hhiiihhhhhh",
            group_index, 0,
            tile_center_x - center_x,
            0,
            tile_center_z - center_z,
            0, 0, 0,
            -21846, next_sibling, -1,
        )

    output = bytearray(b"\0" * 0x1C)
    output += slots
    while len(output) < align(len(output), 4):
        output.append(0)
    group_table, _ = _layout_table(output, groups)
    while len(output) < align(len(output), 4):
        output.append(0)
    collision_table, _ = _layout_table(output, [struct.pack("<h", 0)])
    while len(output) < align(len(output), 4):
        output.append(0)
    texture_table, _ = _layout_table(output, textures)
    struct.pack_into(
        "<IIIIIII", output, 0,
        len(groups), group_table,
        1, collision_table,
        len(textures), texture_table,
        len(tiles) + 1,
    )

    # Exact source phase order: 23,24,25,26,27,26,25,24, six ticks each.
    stream_offset = 4 + (len(tiles) + 1) * 4
    animation = bytearray(struct.pack("<I", 48))
    animation += struct.pack("<I", 0)
    animation += b"".join(
        struct.pack("<I", stream_offset) for _tile in tiles
    )
    for frame, texture in zip(
        range(0, 48, 6), (0, 1, 2, 3, 4, 3, 2, 1)
    ):
        animation += struct.pack(
            "<hhHHHH",
            frame, 0x10,
            0, texture,
            0x8005, texture + 5,
        )
    animation += struct.pack("<hh", 48, -96)
    xobf_children = [iff_chunk(b"BIN ", bytes(output))]
    xobf_children.append(iff_chunk(b"ANM ", bytes(animation)))
    xobf = iff_form(b"XOBF", xobf_children)
    return (
        xobf,
        XobfReport(
            len(groups),
            water_face_count,
            len(textures),
            1,
            len(tiles) + 1,
        ),
        center_x,
        object_y,
        center_z,
    )


def _dreamland_water_object(
    children: list[Chunk], center_x: int, object_y: int, center_z: int
) -> bytes:
    object_ids: list[int] = []
    for child in children:
        if not child.is_form or child.form_type != b"OBJ ":
            continue
        nested = form_children(iff_form(b"OBJ ", [child.payload]), b"OBJ ")
        head = next((item.payload for item in nested if item.tag == b"HEAD"), None)
        if head is not None and len(head) >= 34:
            object_ids.append(be16(head, 2))
    # 0xfffc..0xffff are source control/sentinel ids, not allocatable object
    # identities. Dreamland's ordinary authored ids occupy 0..306.
    ordinary_ids = [value for value in object_ids if value < 0xFF00]
    object_id = max(ordinary_ids, default=-1) + 1
    if object_id >= 0xFF00:
        raise FormatError("Dreamland has no free 16-bit object id")

    head = struct.pack(
        ">BBHIiiihhhhhhh",
        0, 0, object_id,
        # Exact persistent animated-water flags used by Casino City's native
        # PS1 water_1 objects.
        0x00018005,
        center_x, object_y + 0x100000, center_z,
        0, 0, 0,
        2, 0,
        0, 0,
    ) + b"DreamlandWater"
    return iff_form(b"OBJ ", [iff_chunk(b"HEAD", head)])


@dataclass(frozen=True)
class ArenaReport:
    name: str
    xobf: tuple[XobfReport, ...]
    zones: int
    objects: int
    chunks: tuple[str, ...]


def convert_arena(
    exp: bytes,
    name: str,
    *,
    object_callback_names: frozenset[str] | None = None,
) -> tuple[bytes, ArenaReport]:
    children = root_children(exp)
    converted: list[bytes] = []
    reports: list[XobfReport] = []
    zone_count = 0
    object_count = sum(
        1 for _off, tag, _payload, _parent in iter_chunks(exp) if tag == b"OBJ "
    )
    converted_tags: list[str] = []
    zmap = next((child.payload for child in children if child.tag == b"ZMAP"), None)
    terrain_zones = [
        child.payload for child in children if child.tag == b"ZONE"
    ]

    placements_by_bank: dict[int, list[RootedInstance]] = {}
    root_uses: dict[tuple[int, int], list[RootedInstance | None]] = {}
    if object_callback_names is not None:
        for child_index, child in enumerate(children):
            if not child.is_form or child.form_type != b"OBJ ":
                continue
            nested = form_children(iff_form(b"OBJ ", [child.payload]), b"OBJ ")
            head = next((item.payload for item in nested if item.tag == b"HEAD"), None)
            if head is None or len(head) < 34:
                continue
            object_name = head[34:].split(b"\0", 1)[0].decode(
                "ascii", "replace"
            )
            # Kind zero is the ordinary placed-object path. Other kinds are
            # engine-managed item/effect records and are not rooted scenery.
            if head[1] != 0:
                continue
            bank = be16(head, 26, signed=True)
            root_slot = be16(head, 28, signed=True)
            if bank < 0 or root_slot < 0:
                continue
            instance = RootedInstance(
                    key=child_index,
                    root_slot=root_slot,
                    flags=be32(head, 4),
                    position=(
                        be32(head, 8, signed=True),
                        be32(head, 12, signed=True) - 0x100000,
                        be32(head, 16, signed=True),
                    ),
                    rotation_xyz=tuple(
                        be16(head, offset, signed=True)
                        for offset in (20, 22, 24)
                    ),
                )
            root_uses.setdefault((bank, root_slot), []).append(
                None if object_name in object_callback_names else instance
            )
        for (bank, _root_slot), uses in root_uses.items():
            if all(instance is not None for instance in uses):
                placements_by_bank.setdefault(bank, []).extend(
                    instance for instance in uses if instance is not None
                )

    prepared_xobf: dict[int, bytes] = {}
    xobf_index = 0
    for child_index, child in enumerate(children):
        if not child.is_form or child.form_type != b"XOBF":
            continue
        nested = form_children(iff_form(b"XOBF", [child.payload]), b"XOBF")
        bin_chunk = next(item for item in nested if item.tag == b"BIN ")
        anm = next((item for item in nested if item.tag == b"ANM "), None)
        source_slot_count = be32(bin_chunk.payload, 24)
        animated_slots = frozenset()
        if anm is not None:
            source_table_size = 4 + source_slot_count * 4
            if len(anm.payload) < source_table_size:
                raise FormatError("N64 ANM slot table is truncated")
            animated_slots = frozenset(
                index
                for index in range(source_slot_count)
                if be32(anm.payload, 4 + index * 4) != 0
            )
        model, report, slot_vertex_defaults = convert_xobf_bin(
            bin_chunk.payload,
            rooted_instances=placements_by_bank.get(xobf_index, ()),
            animated_slots=animated_slots,
            zmap=zmap,
            zones=terrain_zones,
        )
        reports.append(report)
        out_children = [iff_chunk(b"BIN ", model)]
        if anm is not None:
            out_children.append(
                iff_chunk(
                    b"ANM ",
                    convert_animation(
                        anm.payload,
                        report.slots,
                        slot_vertex_defaults,
                        source_slot_count=source_slot_count,
                    ),
                )
            )
        prepared_xobf[child_index] = iff_form(b"XOBF", out_children)
        xobf_index += 1

    dreamland_water: tuple[bytes, XobfReport, int, int, int] | None = None
    if name.upper() == "DREAMLND":
        rects = [child.payload for child in children if child.tag == b"RECT"]
        xbmps = [child.payload for child in children if child.tag == b"XBMP"]
        zmaps = [child.payload for child in children if child.tag == b"ZMAP"]
        zones = [child.payload for child in children if child.tag == b"ZONE"]
        if len(rects) != 1 or len(xbmps) != 1 or len(zmaps) != 1 or not zones:
            raise FormatError(
                "Dreamland water requires exactly one RECT/XBMP/ZMAP and ZONE data"
            )
        dreamland_water = _dreamland_water_xobf(
            rects[0], xbmps[0], zmaps[0], zones
        )
    for child_index, child in enumerate(children):
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
            converted.append(prepared_xobf[child_index])
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
    if dreamland_water is not None:
        water_xobf, water_report, center_x, object_y, center_z = dreamland_water
        converted.append(water_xobf)
        converted_tags.append("XOBF")
        reports.append(water_report)
        converted.append(
            _dreamland_water_object(
                children, center_x, object_y, center_z
            )
        )
        converted_tags.append("OBJ ")
        object_count += 1
    result = iff_form(b"TERR", converted)
    return result, ArenaReport(
        name=name,
        xobf=tuple(reports),
        zones=zone_count,
        objects=object_count,
        chunks=tuple(converted_tags),
    )
