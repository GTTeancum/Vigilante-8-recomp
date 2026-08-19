#!/usr/bin/env python3
"""Convert the working PS1 Dreamland arena to V8:2's native EXP dialect."""

from __future__ import annotations

import argparse
from io import BytesIO
from pathlib import Path
import struct
import sys

from PIL import Image, ImageOps


REPO = Path(__file__).resolve().parents[2]
ADDONS = REPO / "tools" / "blender_addons"
sys.path.insert(0, str(REPO / "tools"))
sys.path.insert(0, str(ADDONS))

from v8_n64_level import V8N64Rom, root_children
import build_v8_dreamland_shell_assets as shell_assets
from vigilante8_vehicle_tools import compiler, conversion, iff, registry, xobf


V8_TO_V82_OBJECT_KIND = {
    b"I_Cannon": 11,
    b"I_MisslL": 10,
    b"I_RocktL": 12,
    b"I_Special": 6,
    b"I_Surprise": 5,
    b"PU_RadarJammer": 2,
    b"PU_WeaponUpgrade": 3,
    b"PU_Shield": 4,
}


class _Target:
    game = "V8_2"

    @staticmethod
    def validate() -> None:
        pass


def _convert_xobf(node: iff.IffChunk) -> iff.IffChunk:
    # Terrain banks may use 0x3fff as an engine-global texture selector.  The
    # strict vehicle decoder normally rejects that value because authored
    # vehicles may only address textures they own.  Relax that one validation
    # while retaining the semantic packet decoder for every other field.
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
        bank = registry._decode_bank(node, "V8")
    finally:
        registry._decode_face = decode_face
    source_chunks = {child.tag: child.payload for child in node.children}
    source_model = xobf.Model(source_chunks[b"BIN "], dialect="V8")
    bank = conversion.v8_bank_to_v82(bank)
    target = _Target()
    model = bytearray(compiler.compile_model(target, bank))
    # Semantic conversion keeps every slot index. Preserve the source's exact
    # hierarchy links as well: independent location roots are not one sibling
    # chain, even though the generic vehicle compiler links top-level roots.
    for slot in source_model.slots():
        struct.pack_into(
            "<HH",
            model,
            0x1C + slot.index * 0x1C + 0x18,
            slot.next_sibling,
            slot.first_child,
        )
    children = [
        iff.IffChunk(tag=b"BIN ", payload=bytes(model))
    ]
    animation = compiler.compile_animation(target, bank)
    if animation is not None:
        children.append(iff.IffChunk(tag=b"ANM ", payload=animation))
    return iff.IffChunk(tag=b"FORM", form_type=b"XOBF", children=children)


def _decode_xbmp(payload: bytes) -> tuple[int, int, tuple[int, ...], bytes]:
    if len(payload) < 0x20 or struct.unpack_from("<I", payload, 0)[0] != 0x10:
        raise ValueError("Dreamland XBMP is not a native PS1 TIM payload")
    flags = struct.unpack_from("<I", payload, 4)[0]
    if (flags & 3) != 1:
        raise ValueError("Dreamland terrain atlas is not CI8")
    image_offset = struct.unpack_from("<I", payload, 8)[0]
    clut_width, clut_height = struct.unpack_from("<hh", payload, 0x10)
    if clut_height != 1 or clut_width <= 0 or clut_width > 256:
        raise ValueError("Dreamland terrain atlas has an invalid CI8 palette")
    width_words, height = struct.unpack_from("<hh", payload, image_offset + 0x10)
    width = width_words * 2
    pixels = payload[image_offset + 0x14:image_offset + 0x14 + width * height]
    if len(pixels) != width * height:
        raise ValueError("Dreamland terrain atlas is truncated")
    palette = tuple(
        struct.unpack_from("<H", payload, 0x14 + index * 2)[0]
        for index in range(clut_width)
    )
    return width, height, palette, pixels


def _compact_xbmp(payload: bytes) -> bytes:
    """Deduplicate a V8 CI8 palette using the retail V8:2 layout."""

    if len(payload) < 0x220 or struct.unpack_from("<I", payload, 0)[0] != 0x10:
        raise ValueError("Dreamland XBMP is not a native PS1 TIM payload")
    flags = struct.unpack_from("<I", payload, 4)[0]
    if (flags & 3) != 1:
        raise ValueError("Dreamland terrain atlas is not CI8")

    clut_block_size = struct.unpack_from("<I", payload, 8)[0]
    clut_width, clut_height = struct.unpack_from("<hh", payload, 0x10)
    if clut_height != 1 or clut_width <= 0 or clut_width > 256:
        raise ValueError("Dreamland terrain atlas has an invalid CI8 palette")
    if clut_block_size != 12 + clut_width * 2:
        raise ValueError("Dreamland terrain palette block has an invalid size")

    image_block_offset = 8 + clut_block_size
    if image_block_offset + 12 > len(payload):
        raise ValueError("Dreamland terrain image block is truncated")
    image_block_size = struct.unpack_from("<I", payload, image_block_offset)[0]
    if image_block_offset + image_block_size != len(payload):
        raise ValueError("Dreamland terrain image block has an invalid size")
    pixel_offset = image_block_offset + 12
    pixels = payload[pixel_offset:]

    colors = [
        payload[0x14 + index * 2:0x16 + index * 2]
        for index in range(clut_width)
    ]
    unique: list[bytes] = []
    color_to_index: dict[bytes, int] = {}
    remap: list[int] = []
    for color in colors:
        if color not in color_to_index:
            color_to_index[color] = len(unique)
            unique.append(color)
        remap.append(color_to_index[color])
    if len(unique) > 256:
        raise ValueError("Dreamland terrain palette cannot be represented as CI8")
    if any(index >= len(remap) for index in pixels):
        raise ValueError("Dreamland terrain atlas references a missing palette entry")

    compact_clut_size = 12 + len(unique) * 2
    result = bytearray()
    result += payload[0:8]
    result += struct.pack("<I", compact_clut_size)
    result += payload[0x0C:0x10]
    result += struct.pack("<hh", len(unique), 1)
    result += b"".join(unique)
    result += payload[image_block_offset:pixel_offset]
    result += bytes(remap[index] for index in pixels)
    return bytes(result)


def _expand5(value: int) -> int:
    return (value << 3) | (value >> 2)


def _tile_average(
    width: int,
    height: int,
    palette: tuple[int, ...],
    pixels: bytes,
    x: int,
    y: int,
) -> bytes:
    totals = [0, 0, 0, 0]
    count = 0
    for row in range(y, min(y + 48, height)):
        for column in range(x, min(x + 48, width)):
            color = palette[pixels[row * width + column]]
            totals[0] += _expand5(color & 0x1F)
            totals[1] += _expand5((color >> 5) & 0x1F)
            totals[2] += _expand5((color >> 10) & 0x1F)
            totals[3] += 255 if color & 0x8000 else 0
            count += 1
    if count == 0:
        return b"\0\0\0\0"
    return bytes(total // count for total in totals)


def _convert_tinf(payload: bytes, xbmp: bytes) -> bytes:
    """Pack V8's 40-byte TINF records into V8:2's 36-byte XTIN records."""

    if len(payload) != 256 * 0x28:
        raise ValueError("Dreamland TINF does not contain 256 records")
    width, height, palette, pixels = _decode_xbmp(xbmp)
    output = bytearray()
    for offset in range(0, len(payload), 0x28):
        source = payload[offset:offset + 0x28]
        x = struct.unpack_from(">H", source, 2)[0]
        y = struct.unpack_from(">H", source, 4)[0]
        if x % 48 or y % 48 or x // 48 > 15 or y // 48 > 15:
            raise ValueError(
                f"TINF origin ({x},{y}) cannot be packed into V8:2 XTIN"
            )
        output += source[0:2]
        output.append((y // 48) << 4 | (x // 48))
        output.append(source[7])
        output += source[8:36]
        output += _tile_average(width, height, palette, pixels, x, y)
    return bytes(output)


def _convert_cols(payload: bytes) -> bytes:
    """Expand V8's seven terrain colors to V8:2's eight-word table."""

    if len(payload) != 7 * 4:
        raise ValueError("Dreamland COLS does not contain seven RGBA words")
    # Nine of the ten retail V8 arena ports append an all-zero eighth word;
    # the first seven words remain byte-for-byte identical. Dreamland has no
    # authored eighth color, so follow that native conversion convention.
    return payload + b"\0\0\0\0"


def _convert_object(node: iff.IffChunk) -> iff.IffChunk:
    """Translate V8 object metadata that changed in V8:2.

    Retail V8:2 ports of the original arenas preserve the OBJ/HEAD layout but
    renumber shared weapon and non-health power-up kinds.  Leaving a V8 value
    in place makes the sequel's native pickup callback select a nonexistent
    model root before the first gameplay frame.
    """

    converted = []
    for child in node.children:
        if child.tag != b"HEAD" or len(child.payload) < 34:
            converted.append(child)
            continue
        payload = bytearray(child.payload)
        name = bytes(payload[34:]).split(b"\0", 1)[0]
        kind = V8_TO_V82_OBJECT_KIND.get(name)
        if kind is not None:
            struct.pack_into(">H", payload, 28, kind)
        converted.append(iff.IffChunk(tag=b"HEAD", payload=bytes(payload)))
    return iff.IffChunk(tag=b"FORM", form_type=b"OBJ ", children=converted)


def _native_v82_item_bank(source: Path) -> iff.IffChunk:
    forms = list(iff.parse(source.read_bytes()).forms(b"XOBF"))
    if len(forms) < 2:
        raise ValueError("V8:2 donor arena has no shared item XOBF bank")
    return forms[1]


def _n64_loading_chunk(rom_path: Path) -> bytes:
    rom = V8N64Rom(rom_path)
    chunks = [
        node.payload
        for node in root_children(rom.decoded("DREAMLND.EXP"))
        if node.tag == b"XLSC"
    ]
    if len(chunks) != 1 or chunks[0][4:6] != b"\xFF\xD8":
        raise ValueError("N64 Dreamland EXP does not contain one loading JPEG")
    return chunks[0]


def _upscale_preserving_source(
    image: Image.Image,
    output: Path,
    target_size: tuple[int, int],
) -> None:
    final = ImageOps.fit(
        image.convert("RGB"),
        target_size,
        method=Image.Resampling.LANCZOS,
        centering=(0.5, 0.5),
    )
    output.parent.mkdir(parents=True, exist_ok=True)
    final.save(output)


def _write_loading_card(
    n64_xlsc: bytes,
    output: Path,
) -> None:
    with Image.open(BytesIO(n64_xlsc[4:])) as source:
        if source.size != (320, 100):
            raise ValueError(
                f"unexpected N64 Dreamland loading image size {source.size}"
            )
        native = Image.new("RGB", (320, 112), (0, 0, 0))
        native.paste(source.convert("RGB"), (0, 6))
        native = native.crop((82, 0, 238, 112)).resize(
            (320, 112),
            Image.Resampling.LANCZOS,
        )
        pixels = native.load()
        width, height = native.size
        for y in range(height):
            for x in range(width):
                edge = min(x, width - 1 - x)
                factor = 0.18 + 0.82 * min(1.0, edge / 42.5)
                r, g, b = pixels[x, y]
                pixels[x, y] = (
                    int(r * factor),
                    int(g * factor),
                    int(b * factor),
                )
        output.parent.mkdir(parents=True, exist_ok=True)
        native.save(output.with_name("n64_dreamlnd_loading_card_native_320x112.ppm"))
        _upscale_preserving_source(native, output, (1280, 448))


def _write_selector_preview(
    source: Path,
    output: Path,
) -> None:
    form = iff.IffChunk(
        tag=b"FORM",
        form_type=b"XOBF",
        children=[iff.IffChunk(tag=b"BIN ", payload=source.read_bytes())],
    )
    bank = registry._decode_bank(form, "V8")
    dreamland, root_map = conversion.extract_roots(bank, {12})
    if root_map != {12: 0}:
        raise ValueError(f"Dreamland selector root did not isolate: {root_map}")
    if len(dreamland.textures) != 1:
        raise ValueError("Dreamland selector root does not have one texture")
    texture = dreamland.textures[0]
    rgb = bytearray()
    for index in texture.indices:
        color = texture.palette_bgr555[index]
        rgb.extend(
            (
                ((color & 0x1F) << 3) | ((color & 0x1F) >> 2),
                (((color >> 5) & 0x1F) << 3) | (((color >> 5) & 0x1F) >> 2),
                (((color >> 10) & 0x1F) << 3) | (((color >> 10) & 0x1F) >> 2),
            )
        )
    image = Image.frombytes("RGB", (texture.width, texture.height), bytes(rgb))
    output.parent.mkdir(parents=True, exist_ok=True)
    _upscale_preserving_source(image, output, (440, 115))


def convert(
    source: Path,
    item_bank_source: Path,
    loading_trigger: iff.IffChunk,
) -> bytes:
    document = iff.parse(source.read_bytes())
    roots = list(document.forms(b"TERR"))
    if len(roots) != 1:
        raise ValueError("Dreamland EXP must contain exactly one TERR form")
    root = roots[0]
    xbmps = [node.payload for node in root.children if node.tag == b"XBMP"]
    if len(xbmps) != 1:
        raise ValueError("Dreamland EXP must contain exactly one XBMP")

    converted: list[iff.IffChunk] = [
        iff.IffChunk(tag=b"TITL", payload=b"Super Dreamland 64"),
        loading_trigger,
    ]
    item_bank = _native_v82_item_bank(item_bank_source)
    xobf_index = 0
    for node in root.children:
        if node.is_form and node.form_type == b"XOBF":
            # The second V8 bank is the 15-slot shared weapon/power-up bank.
            # Every retail V8:2 port replaces it with the sequel's 24-slot
            # bank; packet conversion alone cannot manufacture its nine new
            # models. Reuse that native shared bank exactly as the retail
            # original-arena ports do.
            converted.append(
                item_bank if xobf_index == 1 else _convert_xobf(node)
            )
            xobf_index += 1
        elif node.is_form and node.form_type == b"OBJ ":
            converted.append(_convert_object(node))
        elif node.tag == b"TINF":
            converted.append(
                iff.IffChunk(tag=b"XTIN", payload=_convert_tinf(node.payload, xbmps[0]))
            )
        elif node.tag == b"COLS":
            converted.append(iff.IffChunk(tag=b"COLS", payload=_convert_cols(node.payload)))
        elif node.tag == b"XBMP":
            converted.append(iff.IffChunk(tag=b"XBMP", payload=_compact_xbmp(node.payload)))
        else:
            converted.append(node)
    root.children = converted
    return document.to_bytes()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--source",
        type=Path,
        default=REPO / "PS1 game" / "TERRAIN" / "DREAMLND.EXP",
    )
    parser.add_argument(
        "--item-bank-source",
        type=Path,
        default=REPO / "V8_2_LOOSE" / "LEVELS" / "V8" / "AIRGRAVE.EXP",
    )
    parser.add_argument(
        "--psxavenc",
        type=Path,
        default=REPO / "build" / "psxavenc" / "bin" / "psxavenc.exe",
    )
    parser.add_argument(
        "--upscaler",
        type=Path,
        default=REPO / "build" / "realesrgan" / "realesrgan-ncnn-vulkan.exe",
    )
    parser.add_argument("--upscale-model", default="realesrgan-x4plus-anime")
    parser.add_argument(
        "--rom",
        type=Path,
        default=REPO / "Vigilante 8 (U) (!).n64",
    )
    parser.add_argument(
        "--selector-source",
        type=Path,
        default=REPO / "PS1 game" / "SHELL" / "LOCATNS.BIN",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=(
            REPO / "V8_2_LOOSE" / "mods" / "v82_n64_super_dreamland" /
            "files" / "LEVELS" / "N64" / "DREAMLND.EXP"
        ),
    )
    parser.add_argument(
        "--selector-preview-output",
        type=Path,
        default=(
            REPO / "V8_2_LOOSE" / "mods" / "v82_n64_super_dreamland" /
            "ui" / "n64_dreamlnd_selector_preview.ppm"
        ),
    )
    parser.add_argument(
        "--loading-card-output",
        type=Path,
        default=(
            REPO / "V8_2_LOOSE" / "mods" / "v82_n64_super_dreamland" /
            "loading_cards" / "n64_dreamlnd_loading_card_4x.ppm"
        ),
    )
    args = parser.parse_args()
    n64_xlsc = _n64_loading_chunk(args.rom.resolve())
    native_preview = args.loading_card_output.resolve().with_name(
        "n64_dreamlnd_loading_card_native_320x112.ppm"
    )
    loading_payload, _ = shell_assets.build_loading_chunk(
        n64_xlsc,
        args.psxavenc.resolve(),
        native_preview,
    )
    result = convert(
        args.source.resolve(),
        args.item_bank_source.resolve(),
        iff.IffChunk(tag=b"XLSC", payload=loading_payload),
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(result)
    _write_selector_preview(
        args.selector_source.resolve(),
        args.selector_preview_output.resolve(),
    )
    _write_loading_card(
        n64_xlsc,
        args.loading_card_output.resolve(),
    )
    print(f"wrote {args.output.resolve()} ({len(result)} bytes)")
    print(
        f"wrote {args.selector_preview_output.resolve()} "
        f"({args.selector_preview_output.stat().st_size} bytes)"
    )
    print(
        f"wrote {args.loading_card_output.resolve()} "
        f"({args.loading_card_output.stat().st_size} bytes)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
