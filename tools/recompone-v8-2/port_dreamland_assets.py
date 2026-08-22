#!/usr/bin/env python3
"""Convert the working PS1 Dreamland arena to V8:2's native EXP dialect."""

from __future__ import annotations

import argparse
from io import BytesIO
from pathlib import Path
import struct
import sys
import tempfile

from PIL import Image, ImageOps


REPO = Path(__file__).resolve().parents[2]
ADDONS = REPO / "tools" / "blender_addons"
sys.path.insert(0, str(REPO / "tools"))
sys.path.insert(0, str(ADDONS))

from v8_n64_level import (
    V8N64Rom,
    convert_arena as convert_n64_arena,
    overlay_exports,
    psx_executable_exports,
    root_children,
)
import build_v8_dreamland_shell_assets as shell_assets
import v82_native_selector_table as selector_table
import v82_arena_registry as arena_registry
import v82_native_water_conversion as native_water
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


def _decode_xobf(node: iff.IffChunk):
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
    return bank


def _convert_xobf(node: iff.IffChunk) -> iff.IffChunk:
    bank = _decode_xobf(node)
    bank = conversion.v8_bank_to_v82(bank)
    target = _Target()
    model = bytearray(compiler.compile_model(target, bank))
    # V8:2 owns top-level objects as one sibling chain.  Original V8 leaves
    # independent roots unlinked and its arena loader enumerates them by
    # record; copying those raw links into a V8:2 bank leaves lifecycle code
    # with the wrong ownership graph.  Let the shared semantic compiler rebuild
    # sibling/child links from decoded parents, exactly as the retail V8:2
    # conversions do.
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


def _convert_object(
    node: iff.IffChunk,
    bank_map: dict[int, int | None],
) -> iff.IffChunk | None:
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
        source_bank = struct.unpack_from(">h", payload, 26)[0]
        if source_bank >= 0:
            if source_bank not in bank_map:
                raise ValueError(f"object references unknown source bank {source_bank}")
            target_bank = bank_map[source_bank]
            if target_bank is None:
                # A modeled original-V8 water plane has no object equivalent
                # after conversion to V8:2's global XWAT facility.
                continue
            struct.pack_into(">h", payload, 26, target_bank)
        name = bytes(payload[34:]).split(b"\0", 1)[0]
        kind = V8_TO_V82_OBJECT_KIND.get(name)
        if kind is not None:
            struct.pack_into(">H", payload, 28, kind)
        converted.append(iff.IffChunk(tag=b"HEAD", payload=bytes(payload)))
    if not any(child.tag == b"HEAD" for child in converted):
        return None
    return iff.IffChunk(tag=b"FORM", form_type=b"OBJ ", children=converted)


def _native_v82_item_bank(source: Path) -> iff.IffChunk:
    forms = list(iff.parse(source.read_bytes()).forms(b"XOBF"))
    if len(forms) < 2:
        raise ValueError("V8:2 donor arena has no shared item XOBF bank")
    return forms[1]


def _native_xbgm_prefix(source_payload: bytes, template_exp: Path) -> bytes:
    """Derive V8:2's four-byte XBGM placement prefix from a native peer.

    Original V8 XBGM chunks begin directly with the TIM-like texture record.
    V8:2 prepends one big-endian signed placement word which SHELL/LOAD consumes
    before passing the remaining record to the shared texture decoder.  Match
    a native arena with the same texture-record layout so this remains a data
    conversion rule rather than an arena-specific runtime workaround.
    """

    if len(source_payload) < 20 or struct.unpack_from("<I", source_payload, 0)[0] != 0x10:
        raise ValueError("V8 XBGM does not begin with a texture record")
    template_chunks = [
        node.payload
        for node in iff.parse(template_exp.read_bytes()).walk()
        if node.tag == b"XBGM"
    ]
    if len(template_chunks) != 1:
        raise ValueError("V8:2 backdrop template must contain exactly one XBGM")
    template = template_chunks[0]
    if len(template) < 24 or struct.unpack_from("<I", template, 4)[0] != 0x10:
        raise ValueError("V8:2 XBGM template has no native placement prefix")
    # Compare the texture flags, image-block offset, CLUT origin, and CLUT
    # dimensions. Pixel data and the authored placement value may differ.
    if template[8:24] != source_payload[4:20]:
        raise ValueError(
            "V8:2 XBGM template does not match the source texture layout"
        )
    return template[:4]


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


def _selector_preview_tim(source: Path, template: bytes) -> bytes:
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
    indexed = Image.frombytes(
        "P", (texture.width, texture.height), bytes(texture.indices)
    )
    # Retail V8:2's original-V8 previews preserve the complete wide source
    # composition inside a 220x74 native TIM and use an authored transparent
    # silhouette. Do the same here: never crop the source, and derive the
    # silhouette from the selected stock preview template.
    resized = indexed.resize(
        (220, 74),
        resample=Image.Resampling.NEAREST,
    )
    masked_indices, masked_palette = selector_table.apply_ci8_template_mask(
        resized.tobytes(),
        220,
        74,
        texture.palette_bgr555,
        template,
    )
    return selector_table.encode_ci8_tim(
        masked_indices,
        220,
        74,
        masked_palette,
        template,
    )


def convert(
    source: Path | bytes,
    item_bank_source: Path,
    loading_trigger: iff.IffChunk,
    backdrop_template: Path,
    water_template_source: Path,
) -> bytes:
    document = iff.parse(source if isinstance(source, bytes) else source.read_bytes())
    water = native_water.find_native_water_source(document)
    roots = list(document.forms(b"TERR"))
    if len(roots) != 1:
        raise ValueError("Dreamland EXP must contain exactly one TERR form")
    root = roots[0]
    source_xobfs = list(document.forms(b"XOBF"))
    # V8:2 owns water globally through XWAT; its original-V8 0x8043 arena
    # conversions do not keep a modeled water XOBF or an owning HEAD.  Build a
    # reusable structural remap so later source banks remain valid when the
    # detected water bank is removed.
    bank_map: dict[int, int | None] = {}
    target_bank = 0
    for source_bank in range(len(source_xobfs)):
        if source_bank == water.bank_index:
            bank_map[source_bank] = None
        else:
            bank_map[source_bank] = target_bank
            target_bank += 1
    xbmps = [node.payload for node in root.children if node.tag == b"XBMP"]
    if len(xbmps) != 1:
        raise ValueError("Dreamland EXP must contain exactly one XBMP")

    converted: list[iff.IffChunk] = [
        iff.IffChunk(tag=b"TITL", payload=b"Super Dreamland 64"),
        loading_trigger,
    ]
    item_bank = _native_v82_item_bank(item_bank_source)
    water_template_doc = iff.parse(water_template_source.read_bytes())
    xwat = iff.IffChunk(
        tag=b"XWAT",
        payload=native_water.encode_native_xwat(
            water.bank.textures[0], native_water.xwat_payload(water_template_doc)
        ),
    )
    source_backdrops = [node.payload for node in root.children if node.tag == b"XBGM"]
    if len(source_backdrops) != 1:
        raise ValueError("Dreamland EXP must contain exactly one XBGM")
    backdrop_prefix = _native_xbgm_prefix(
        source_backdrops[0], backdrop_template
    )
    xobf_index = 0
    for node in root.children:
        if node.is_form and node.form_type == b"XOBF":
            # The second V8 bank is the 15-slot shared weapon/power-up bank.
            # Every retail V8:2 port replaces it with the sequel's 24-slot
            # bank; packet conversion alone cannot manufacture its nine new
            # models. Reuse that native shared bank exactly as the retail
            # original-arena ports do.
            if xobf_index == water.bank_index:
                converted.append(xwat)
            else:
                converted.append(
                    item_bank if xobf_index == 1 else _convert_xobf(node)
                )
            xobf_index += 1
        elif node.is_form and node.form_type == b"OBJ ":
            converted_object = _convert_object(node, bank_map)
            if converted_object is not None:
                converted.append(converted_object)
        elif node.tag == b"TINF":
            converted.append(
                iff.IffChunk(tag=b"XTIN", payload=_convert_tinf(node.payload, xbmps[0]))
            )
        elif node.tag == b"COLS":
            converted.append(iff.IffChunk(tag=b"COLS", payload=_convert_cols(node.payload)))
        elif node.tag == b"XBMP":
            converted.append(iff.IffChunk(tag=b"XBMP", payload=_compact_xbmp(node.payload)))
        elif node.tag == b"XBGM":
            converted.append(
                iff.IffChunk(tag=b"XBGM", payload=backdrop_prefix + node.payload)
            )
        elif node.tag == b"RECT" and node.payload == struct.pack(
            ">7h", *water.rectangle
        ):
            converted.append(
                iff.IffChunk(
                    tag=b"RECT",
                    payload=native_water.convert_water_rectangle(node.payload),
                )
            )
        else:
            converted.append(node)
    root.children = converted
    return document.to_bytes()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--source",
        type=Path,
        help=(
            "optional preconverted V8 EXP; by default reconvert the N64 ROM "
            "through the generic rooted-prop pipeline"
        ),
    )
    parser.add_argument(
        "--runtime-executable",
        type=Path,
        default=REPO / "PS1 game" / "SLUS_005.10",
        help="target PS-X EXE supplying resident object callbacks",
    )
    parser.add_argument(
        "--item-bank-source",
        type=Path,
        default=REPO / "V8_2_LOOSE" / "LEVELS" / "V8" / "AIRGRAVE.EXP",
    )
    parser.add_argument(
        "--backdrop-template-source",
        type=Path,
        default=REPO / "V8_2_LOOSE" / "LEVELS" / "NUCLEAR.EXP",
    )
    parser.add_argument(
        "--water-template-source",
        type=Path,
        default=REPO / "V8_2_LOOSE" / "LEVELS" / "BAYOU.EXP",
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
        "--selector-table-source",
        type=Path,
        default=REPO / "V8_2_LOOSE" / "SHELL" / "LEVELSEL.TBL",
    )
    parser.add_argument(
        "--selector-table-output",
        type=Path,
        default=(
            REPO / "V8_2_LOOSE" / "mods" / "v82_n64_super_dreamland" /
            "files" / "SHELL" / "LEVELSEL.TBL"
        ),
    )
    parser.add_argument("--selector-template-index", type=int, default=8)
    parser.add_argument(
        "--arena-registry-output",
        type=Path,
        default=(
            REPO / "V8_2_LOOSE" / "mods" / "v82_n64_super_dreamland" /
            "ARENAS.V8R"
        ),
    )
    parser.add_argument(
        "--arena-dll",
        type=Path,
        default=(
            REPO / "V8_2_LOOSE" / "mods" / "v82_n64_super_dreamland" /
            "files" / "LEVELS" / "N64" / "DREAMLND.DLL"
        ),
    )
    args = parser.parse_args()
    rom = V8N64Rom(args.rom.resolve())
    source: Path | bytes
    if args.source is not None:
        source = args.source.resolve()
    else:
        callbacks = frozenset(
            item.name
            for item in (
                *overlay_exports(rom.decoded("DREAMLND.DLL")),
                *psx_executable_exports(
                    args.runtime_executable.resolve().read_bytes()
                ),
            )
        )
        source, conversion_report = convert_n64_arena(
            rom.decoded("DREAMLND.EXP"),
            "DREAMLND",
            object_callback_names=callbacks,
        )
        rooted = sum(item.rooted_variants for item in conversion_report.xobf)
        clipped = sum(
            item.terrain_clipped_faces for item in conversion_report.xobf
        )
        discarded = sum(
            item.terrain_discarded_faces for item in conversion_report.xobf
        )
        print(
            "reconverted N64 arena: "
            f"rooted-variants={rooted} terrain-clipped={clipped} "
            f"terrain-discarded={discarded}"
        )
    n64_xlsc = _n64_loading_chunk(args.rom.resolve())
    # The encoder requires a decoded intermediate, but conversion verification
    # is text-only.  Keep the PPM temporary so repeated reconversions do not
    # accumulate screenshot-like artifacts.
    with tempfile.TemporaryDirectory(prefix="v82-dreamland-loading-") as temp:
        native_preview = Path(temp) / "loading_native_320x96.ppm"
        loading_payload, _ = shell_assets.build_loading_chunk(
            n64_xlsc,
            args.psxavenc.resolve(),
            native_preview,
            target_size=(320, 96),
        )
    result = convert(
        source,
        args.item_bank_source.resolve(),
        iff.IffChunk(tag=b"XLSC", payload=loading_payload),
        args.backdrop_template_source.resolve(),
        args.water_template_source.resolve(),
    )
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(result)
    retail_selector = args.selector_table_source.resolve().read_bytes()
    resources = selector_table.split_resource_table(retail_selector)
    template_index = args.selector_template_index
    if not 0 <= template_index < len(resources) - 1:
        raise ValueError("selector template index is outside retail previews")
    preview_tim = _selector_preview_tim(
        args.selector_source.resolve(), resources[template_index]
    )
    selector_result = selector_table.write_extended_table(
        args.selector_table_source.resolve(),
        args.selector_table_output.resolve(),
        preview_tim,
    )
    arena_entry = arena_registry.ArenaEntry(
            stable_id="n64.super_dreamland_64",
            name="Super Dreamland 64",
            subtitle="Super Dreamland 64",
            # LOAD strips the extension and performs an exact, case-sensitive
            # lookup against the DLL's first export.  Preserve native path
            # casing just like Levels\Bayou.exp and Levels\V8\AirGrave.exp.
            path="Levels\\N64\\DreamLnd.exp",
            marker_x=84,
            marker_y=222,
            preview_index=18,
        )
    primary_export = arena_registry.validate_primary_export(
        arena_entry, args.arena_dll.resolve().read_bytes()
    )
    arena_result = arena_registry.write_registry(
        args.arena_registry_output.resolve(), [arena_entry],
    )
    print(f"wrote {args.output.resolve()} ({len(result)} bytes)")
    print(
        f"wrote {args.selector_table_output.resolve()} "
        f"({len(selector_result)} bytes, template={template_index}, "
        "preview=220x74, resources=20)"
    )
    print(
        f"wrote {args.arena_registry_output.resolve()} "
        f"({len(arena_result)} bytes, entries=1, "
        f"primary-export={primary_export})"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
