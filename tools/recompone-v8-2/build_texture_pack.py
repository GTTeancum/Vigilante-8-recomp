#!/usr/bin/env python3
"""Build the V8:2 loose-files DDS texture replacement pack.

The runtime identifies a texture by the decoded PS1 texels used by an
authored polygon, not by its transient VRAM address.  This tool derives the
same regions directly from each XOBF bank, runs those regions through the
official Real-ESRGAN executable, restores the original STP mask, and writes
ordinary uncompressed DDS files. The Enhanced renderer packs those files into
GPU memory at launch; no proprietary or prebuilt atlas is shipped.

Ordinary world textures are 2x. Runtime-captured shell/loading UI textures are
4x. Each level's authored XBMP terrain atlas is
kept as one human-editable DDS and is 4x, because terrain occupies most of the
screen and benefits materially from the extra source density. Vehicle banks
are also 4x so gameplay and selector models receive the same close-up detail.
"""

from __future__ import annotations

import argparse
import base64
from collections import defaultdict
from dataclasses import dataclass, field
import json
from pathlib import Path
import shutil
import subprocess
import sys

from PIL import Image, ImageFilter, ImageStat


ROOT = Path(__file__).resolve().parents[2]
TOOLS = ROOT / "tools"
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

from blender_addons.vigilante8_vehicle_tools.xobf import (  # noqa: E402
    FormatError,
    Model,
    TextureSlot,
)
from terrain_object_identity_audit import collect_bins  # noqa: E402
from terrain_runtime_decode import collect  # noqa: E402


FNV_OFFSET = 14695981039346656037
FNV_PRIME = 1099511628211
TEXTURE_SCALE = 2
TERRAIN_SCALE = 4
VEHICLE_SCALE = 4
ROUTE_SCALE = 4
UI_SCALE = 4
VEHICLE_ARCHIVES = (
    "V8_2_LOOSE/CUSTOM.EXP",
    "V8_2_LOOSE/SHARED/COMMON.EXP",
    "V8_2_LOOSE/SHARED/HOTRODS.EXP",
    "V8_2_LOOSE/SHELL/VEHICLES.EXP",
)


@dataclass
class Region:
    key: int
    image: Image.Image
    backing_key: int
    backing_box: tuple[int, int, int, int]
    sources: set[str] = field(default_factory=set)


@dataclass
class TerrainAtlas:
    name: str
    image_key: int
    image: Image.Image
    width: int
    height: int
    image_x: int
    image_y: int
    depth: int
    index_hash: int
    indices: bytes
    palette: tuple[int, ...]


@dataclass
class RouteTexture:
    name: str
    image_key: int
    image: Image.Image
    width: int
    height: int
    source: str


def expand5(value: int) -> int:
    return (value << 3) | (value >> 2)


def decode_texture(texture: TextureSlot) -> Image.Image:
    """Match TextureReplacementAtlas.Decode, including its STP byte."""

    if texture.direct_pixels_bgr555:
        source = texture.direct_pixels_bgr555
    else:
        source = tuple(texture.palette[index] for index in texture.indices)
    rgba = bytearray(texture.width * texture.height * 4)
    for index, pixel in enumerate(source):
        output = index * 4
        rgba[output] = expand5(pixel & 0x1F)
        rgba[output + 1] = expand5((pixel >> 5) & 0x1F)
        rgba[output + 2] = expand5((pixel >> 10) & 0x1F)
        rgba[output + 3] = 255 if pixel & 0x8000 else 0
    return Image.frombytes("RGBA", (texture.width, texture.height), bytes(rgba))


def runtime_hash(image: Image.Image) -> int:
    width, height = image.size
    value = FNV_OFFSET
    for byte in (width & 0xFF, width >> 8, height & 0xFF, height >> 8):
        value ^= byte
        value = (value * FNV_PRIME) & 0xFFFFFFFFFFFFFFFF
    for byte in image.tobytes():
        value ^= byte
        value = (value * FNV_PRIME) & 0xFFFFFFFFFFFFFFFF
    return value


def byte_hash(payload: bytes, width: int, height: int) -> int:
    """Use the runtime FNV convention for non-RGBA source identity."""

    value = FNV_OFFSET
    for byte in (width & 0xFF, width >> 8, height & 0xFF, height >> 8):
        value ^= byte
        value = (value * FNV_PRIME) & 0xFFFFFFFFFFFFFFFF
    for byte in payload:
        value ^= byte
        value = (value * FNV_PRIME) & 0xFFFFFFFFFFFFFFFF
    return value


def little_u32(payload: bytes, offset: int) -> int:
    return int.from_bytes(payload[offset:offset + 4], "little")


def little_i16(payload: bytes, offset: int) -> int:
    return int.from_bytes(payload[offset:offset + 2], "little", signed=True)


def terrain_name(exp_path: Path) -> str:
    relative = exp_path.relative_to(ROOT / "V8_2_LOOSE")
    return relative.with_suffix("").as_posix().lower()


def decode_terrain_atlas(exp_path: Path) -> TerrainAtlas | None:
    """Decode the one authored XBMP atlas used by terrain materials.

    Terrain packets do not own hundreds of independent textures.  Their TINF
    records select overlapping windows from this single indexed source and
    choose distance-lighting CLUT rows at draw time.  Keeping XBMP as one DDS
    is therefore both the faithful representation and the useful mod surface.
    """

    chunks = collect(exp_path.read_bytes(), b"XBMP", b"TERR")
    if not chunks:
        return None
    if len(chunks) != 1:
        raise ValueError(f"{exp_path} has {len(chunks)} terrain XBMP chunks")
    payload = chunks[0][4]
    if len(payload) < 0x220:
        raise ValueError(f"{exp_path} terrain XBMP is truncated")
    flags = little_u32(payload, 4)
    depth = flags & 3
    if depth not in (0, 1, 2):
        raise ValueError(f"{exp_path} terrain XBMP depth is {depth}")
    image_offset = little_u32(payload, 8)
    image_x = little_i16(payload, image_offset + 0x0C)
    image_y = little_i16(payload, image_offset + 0x0E)
    width_words = little_i16(payload, image_offset + 0x10)
    height = little_i16(payload, image_offset + 0x12)
    pixels_per_word = 4 if depth == 0 else 2 if depth == 1 else 1
    width = width_words * pixels_per_word
    if width <= 0 or height <= 0:
        raise ValueError(f"{exp_path} terrain XBMP has invalid dimensions")
    palette = tuple(
        int.from_bytes(payload[0x14 + index * 2:0x16 + index * 2], "little")
        for index in range(256)
    )
    pixel_offset = image_offset + 0x14
    if depth == 1:
        indices = payload[pixel_offset:pixel_offset + width * height]
    elif depth == 0:
        packed = payload[pixel_offset:pixel_offset + width_words * height * 2]
        expanded = bytearray(width * height)
        output = 0
        for value in packed:
            expanded[output] = value & 0x0F
            expanded[output + 1] = value >> 4
            output += 2
        indices = bytes(expanded)
    else:
        direct = payload[pixel_offset:pixel_offset + width * height * 2]
        indices = direct
    expected = width * height * (2 if depth == 2 else 1)
    if len(indices) != expected:
        raise ValueError(f"{exp_path} terrain XBMP pixels are truncated")
    rgba = bytearray(width * height * 4)
    for index in range(width * height):
        pixel = (
            int.from_bytes(indices[index * 2:index * 2 + 2], "little")
            if depth == 2 else palette[indices[index]]
        )
        output = index * 4
        rgba[output] = expand5(pixel & 0x1F)
        rgba[output + 1] = expand5((pixel >> 5) & 0x1F)
        rgba[output + 2] = expand5((pixel >> 10) & 0x1F)
        rgba[output + 3] = 255 if pixel & 0x8000 else 0
    image = Image.frombytes("RGBA", (width, height), bytes(rgba))
    return TerrainAtlas(
        terrain_name(exp_path), runtime_hash(image), image,
        width, height, image_x, image_y, depth,
        byte_hash(indices, width, height), indices, palette,
    )


def collect_terrain_atlases(exp_paths: list[Path]) -> list[TerrainAtlas]:
    atlases: list[TerrainAtlas] = []
    for path in exp_paths:
        atlas = decode_terrain_atlas(path)
        if atlas is not None:
            atlases.append(atlas)
    return atlases


def decode_tim_image(payload: bytes, context: str) -> Image.Image:
    """Decode the paletted/direct PSX TIM payload carried by XRTP records."""

    if len(payload) < 20 or little_u32(payload, 0) != 0x10:
        raise ValueError(f"{context} is not a TIM payload")
    flags = little_u32(payload, 4)
    depth = flags & 3
    has_clut = bool(flags & 8)
    offset = 8
    palette: tuple[int, ...] = ()
    if has_clut:
        if offset + 12 > len(payload):
            raise ValueError(f"{context} TIM CLUT is truncated")
        clut_size = little_u32(payload, offset)
        if clut_size < 12 or offset + clut_size > len(payload):
            raise ValueError(f"{context} TIM CLUT size is invalid")
        color_count = int.from_bytes(payload[offset + 8:offset + 10], "little")
        palette_count = int.from_bytes(payload[offset + 10:offset + 12], "little")
        total_colors = color_count * palette_count
        if total_colors <= 0 or 12 + total_colors * 2 > clut_size:
            raise ValueError(f"{context} TIM CLUT color count is invalid")
        palette = tuple(
            int.from_bytes(
                payload[offset + 12 + index * 2:offset + 14 + index * 2],
                "little",
            )
            for index in range(total_colors)
        )
        offset += clut_size
    if offset + 12 > len(payload):
        raise ValueError(f"{context} TIM image block is truncated")
    image_size = little_u32(payload, offset)
    if image_size < 12 or offset + image_size > len(payload):
        raise ValueError(f"{context} TIM image size is invalid")
    width_words = int.from_bytes(payload[offset + 8:offset + 10], "little")
    height = int.from_bytes(payload[offset + 10:offset + 12], "little")
    pixels = payload[offset + 12:offset + image_size]
    pixels_per_word = 4 if depth == 0 else 2 if depth == 1 else 1
    width = width_words * pixels_per_word
    expected = width_words * height * 2
    if width <= 0 or height <= 0 or len(pixels) != expected:
        raise ValueError(f"{context} TIM pixels are truncated")
    if depth in (0, 1) and not palette:
        raise ValueError(f"{context} indexed TIM has no CLUT")
    rgba = bytearray(width * height * 4)
    for y in range(height):
        row = y * width_words * 2
        for x in range(width):
            if depth == 0:
                packed = pixels[row + (x >> 1)]
                palette_index = (
                    packed & 0x0F if (x & 1) == 0 else packed >> 4
                )
                pixel = palette[palette_index]
            elif depth == 1:
                palette_index = pixels[row + x]
                pixel = palette[palette_index]
            else:
                pixel = int.from_bytes(
                    pixels[row + x * 2:row + x * 2 + 2],
                    "little",
                )
            output = (y * width + x) * 4
            rgba[output] = expand5(pixel & 0x1F)
            rgba[output + 1] = expand5((pixel >> 5) & 0x1F)
            rgba[output + 2] = expand5((pixel >> 10) & 0x1F)
            rgba[output + 3] = 255 if pixel & 0x8000 else 0
    return Image.frombytes("RGBA", (width, height), bytes(rgba))


def collect_route_textures(exp_paths: list[Path]) -> list[RouteTexture]:
    """Decode authored XRTP TIM route/road images.

    These are not part of the base terrain XBMP atlas.  Route packets are
    source-owned by the road-strip renderer and can draw above terrain, so they
    need their own loose DDS source instead of falling back to the stock VRAM
    texels.
    """

    routes: list[RouteTexture] = []
    for path in exp_paths:
        relative = path.relative_to(ROOT).as_posix()
        for index, (_off, size, _body, _parent, payload) in enumerate(
                collect(path.read_bytes(), b"XRTP")):
            if size <= 12:
                continue
            image = decode_tim_image(
                payload[12:],
                f"{relative}:XRTP{index}",
            )
            texture_id = int.from_bytes(payload[8:10], "big", signed=True)
            name = (
                f"{terrain_name(path)}_xrtp{index:02d}_tex{texture_id}"
            )
            routes.append(RouteTexture(
                name,
                runtime_hash(image),
                image,
                image.width,
                image.height,
                f"{relative}:XRTP{index}:tex{texture_id}",
            ))
    return routes


def add_region(
    regions: dict[int, Region], image: Image.Image, source: str,
    backing_key: int, backing_box: tuple[int, int, int, int],
) -> None:
    if image.width < 2 or image.height < 2:
        return
    key = runtime_hash(image)
    current = regions.get(key)
    if current is None:
        regions[key] = Region(
            key, image.copy(), backing_key, backing_box, {source}
        )
        return
    if current.image.size != image.size or current.image.tobytes() != image.tobytes():
        raise RuntimeError(f"64-bit texture hash collision at {key:016x}")
    current.sources.add(source)


def collect_regions(
    exp_paths: list[Path],
) -> tuple[dict[int, Region], dict[int, Image.Image], list[str]]:
    regions: dict[int, Region] = {}
    backing_images: dict[int, Image.Image] = {}
    warnings: list[str] = []
    for exp_path in exp_paths:
        relative = exp_path.relative_to(ROOT).as_posix()
        for bank_index, payload in enumerate(collect_bins(exp_path)):
            try:
                model = Model(payload, "V8_2")
                textures = tuple(model.textures())
            except (FormatError, ValueError, IndexError) as error:
                warnings.append(f"{relative}: bank {bank_index}: {error}")
                continue
            decoded = [decode_texture(texture) for texture in textures]
            # Include the complete slot. Some full-screen and sprite paths use
            # the allocation directly rather than a model packet.
            for texture_index, image in enumerate(decoded):
                backing_key = runtime_hash(image)
                if backing_key not in backing_images:
                    backing_images[backing_key] = image.copy()
                add_region(
                    regions,
                    image,
                    f"{relative}:B{bank_index}:T{texture_index}:full",
                    backing_key,
                    (0, 0, image.width, image.height),
                )
            try:
                groups = tuple(model.groups())
            except (FormatError, ValueError, IndexError) as error:
                warnings.append(f"{relative}: bank {bank_index} groups: {error}")
                continue
            for group in groups:
                for packet_index, packet in enumerate(group.packets):
                    if packet.texture_slot is None or len(packet.uv) < 3:
                        continue
                    texture_index = packet.texture_slot & 0x3FFF
                    if texture_index >= len(decoded):
                        warnings.append(
                            f"{relative}:B{bank_index}:G{group.index}:P{packet_index} "
                            f"references texture {texture_index}/{len(decoded)}"
                        )
                        continue
                    image = decoded[texture_index]
                    min_u = min(uv[0] for uv in packet.uv)
                    min_v = min(uv[1] for uv in packet.uv)
                    max_u = max(uv[0] for uv in packet.uv)
                    max_v = max(uv[1] for uv in packet.uv)
                    if min_u >= image.width or min_v >= image.height:
                        continue
                    max_u = min(max_u, image.width - 1)
                    max_v = min(max_v, image.height - 1)
                    if max_u < min_u or max_v < min_v:
                        continue
                    crop = image.crop((min_u, min_v, max_u + 1, max_v + 1))
                    backing_key = runtime_hash(image)
                    add_region(
                        regions,
                        crop,
                        f"{relative}:B{bank_index}:G{group.index}:P{packet_index}",
                        backing_key,
                        (min_u, min_v, max_u + 1, max_v + 1),
                    )
    return regions, backing_images, warnings


def collect_runtime_dumps(
    directories: list[Path],
    regions: dict[int, Region],
    backing_images: dict[int, Image.Image],
) -> set[int]:
    added = 0
    keys: set[int] = set()
    for directory in directories:
        for path in sorted(directory.glob("*.rgba")):
            try:
                key_text, dimensions = path.stem.split("_", 1)
                width_text, height_text = dimensions.split("x", 1)
                key = int(key_text, 16)
                width, height = int(width_text), int(height_text)
            except ValueError as error:
                raise ValueError(f"invalid runtime dump name: {path.name}") from error
            rgba = path.read_bytes()
            expected = width * height * 4
            if len(rgba) != expected:
                raise ValueError(
                    f"runtime dump {path.name} has {len(rgba)} bytes; "
                    f"expected {expected}"
                )
            image = Image.frombytes("RGBA", (width, height), rgba)
            actual = runtime_hash(image)
            if actual != key:
                raise ValueError(
                    f"runtime dump {path.name} hashes to {actual:016x}"
                )
            keys.add(key)
            if key in regions:
                regions[key].sources.add(f"runtime:{path.parent.name}/{path.name}")
                continue
            backing_images[key] = image.copy()
            regions[key] = Region(
                key,
                image,
                key,
                (0, 0, width, height),
                {f"runtime:{path.parent.name}/{path.name}"},
            )
            added += 1
    return keys


def collect_vehicle_backing_keys(regions: dict[int, Region]) -> set[int]:
    """Return complete source images belonging to stock/imported vehicles.

    V8:2 stores gameplay vehicles in the two SHARED banks, selector models in
    SHELL/VEHICLES, and converted V8 vehicles in CUSTOM. Region provenance is
    retained while parsing, so this category is derived from authored asset
    ownership rather than filename guesses made after the DDS pack is built.
    """

    return {
        region.backing_key
        for region in regions.values()
        if any(
            source.startswith(VEHICLE_ARCHIVES)
            for source in region.sources
        )
    }


def upscale_regions(
    regions: dict[int, Region], work: Path, executable: Path,
    terrain_keys: set[int], vehicle_keys: set[int], route_keys: set[int],
    ui_keys: set[int],
    reuse_esrgan: bool = False,
) -> dict[int, Image.Image]:
    source_dir = work / "source"
    output_dir = work / "esrgan"
    if work.exists() and not reuse_esrgan:
        shutil.rmtree(work)
    source_dir.mkdir(parents=True, exist_ok=True)
    output_dir.mkdir(parents=True, exist_ok=True)

    for key, region in regions.items():
        # Real-ESRGAN expects conventional opacity. Its RGB result is used,
        # while the PS1 STP flag is restored independently below.
        region.image.convert("RGB").save(source_dir / f"{key:016x}.png")

    if not reuse_esrgan:
        command = [
            str(executable),
            "-i", str(source_dir),
            "-o", str(output_dir),
            # The compact native 2x model is the appropriate Real-ESRGAN
            # variant for stylized game textures and fits integrated Vulkan
            # GPUs. The general x4plus network exhausts this machine before
            # its first tile.
            "-n", "realesr-animevideov3-x2",
            "-s", "2",
            "-t", "128",
            "-j", "1:1:1",
            "-f", "png",
        ]
        print("Running:", " ".join(command), flush=True)
        subprocess.run(command, check=True, cwd=executable.parent)
        # Terrain, vehicles, routes, and runtime-captured loading UI receive
        # two learned x2 passes (net 4x).
        # Terrain keys are complete authored XBMP atlases, not duplicated
        # 40x40 crops; vehicle keys are complete source images owned by the
        # SHARED, SHELL/VEHICLES, and CUSTOM banks.
        second_pass = work / "four_x_second_pass"
        second_pass.mkdir(parents=True, exist_ok=True)
        for key in sorted(terrain_keys | vehicle_keys | route_keys | ui_keys):
            source = output_dir / f"{key:016x}.png"
            command = [
                str(executable),
                "-i", str(source),
                "-o", str(second_pass),
                "-n", "realesr-animevideov3-x2",
                "-s", "2",
                "-t", "128",
                "-j", "1:1:1",
                "-f", "png",
            ]
            category = (
                "terrain" if key in terrain_keys else
                "vehicle" if key in vehicle_keys else
                "route" if key in route_keys else
                "ui"
            )
            print(f"Running {category} 4x:", " ".join(command), flush=True)
            subprocess.run(command, check=True, cwd=executable.parent)
            generated = second_pass / source.name
            if not generated.exists():
                raise FileNotFoundError(
                    f"Real-ESRGAN did not produce 4x {generated.name}"
                )
            shutil.copy2(generated, source)

    upscaled: dict[int, Image.Image] = {}
    corrected_count = 0
    fallback_count = 0
    for key, region in regions.items():
        path = output_dir / f"{key:016x}.png"
        if not path.exists():
            raise FileNotFoundError(f"Real-ESRGAN did not produce {path.name}")
        result = Image.open(path).convert("RGBA")
        scale = (
            TERRAIN_SCALE if key in terrain_keys else
            VEHICLE_SCALE if key in vehicle_keys else
            ROUTE_SCALE if key in route_keys else
            UI_SCALE if key in ui_keys else
            TEXTURE_SCALE
        )
        expected = (region.image.width * scale, region.image.height * scale)
        if result.size != expected:
            result = result.resize(expected, Image.Resampling.LANCZOS)
        neural = result.convert("RGB")
        source_up = region.image.convert("RGB").resize(
            expected, Image.Resampling.LANCZOS
        )
        # Real-ESRGAN removes compression noise well, but its compact 2x game
        # model can over-smooth low-contrast roads and terrain. Reintroduce a
        # controlled amount of source-authored frequency, then deconvolve the
        # combined image. This is deliberately source-faithful: no synthetic
        # pattern or texture is introduced.
        corrected = Image.blend(neural, source_up, 0.35).filter(
            ImageFilter.UnsharpMask(radius=0.85, percent=145, threshold=2)
        )

        def edge_energy(image: Image.Image) -> float:
            edges = image.convert("L").filter(ImageFilter.FIND_EDGES)
            if edges.width > 2 and edges.height > 2:
                edges = edges.crop((1, 1, edges.width - 1, edges.height - 1))
            return float(ImageStat.Stat(edges).mean[0])

        neural_energy = edge_energy(neural)
        source_energy = edge_energy(source_up)
        corrected_energy = edge_energy(corrected)
        required = max(neural_energy * 1.08, source_energy * 0.92)
        if corrected_energy + 0.01 < required:
            fallback = source_up.filter(
                ImageFilter.UnsharpMask(radius=0.75, percent=175, threshold=1)
            )
            if edge_energy(fallback) > corrected_energy:
                corrected = fallback
                fallback_count += 1
        else:
            corrected_count += 1
        result = corrected.convert("RGBA")
        # Alpha is an STP flag in the PS1 renderer, not conventional opacity.
        # Never let the neural model invent or blur that bit.
        stp = region.image.getchannel("A").resize(expected, Image.Resampling.NEAREST)
        result.putalpha(stp)
        upscaled[key] = result
    print(
        f"Texture correction: neural+source={corrected_count} "
        f"source-detail-fallback={fallback_count}",
        flush=True,
    )
    return upscaled


def write_source_pngs(images: dict[int, Image.Image], work: Path) -> Path:
    """Prepare conventional RGB inputs for an external Real-ESRGAN runner."""
    source_directory = work / "source"
    source_directory.mkdir(parents=True, exist_ok=True)
    for key, image in images.items():
        image.convert("RGB").save(source_directory / f"{key:016x}.png")
    print(f"Prepared {len(images)} source PNGs in {source_directory}", flush=True)
    return source_directory


def write_loose_pack(
    images: dict[int, Image.Image], destination: Path,
    sources: dict[int, Region], terrain_atlases: list[TerrainAtlas],
    route_textures: list[RouteTexture], vehicle_keys: set[int],
    ui_keys: set[int], warnings: list[str], generator: str,
) -> None:
    """Write ordinary editable DDS files plus a small region catalog.

    Runtime regions can be crops of a shared source image, so the catalog maps
    each content key to a DDS and crop instead of duplicating tens of thousands
    of nearly identical files. The renderer builds its GPU atlas in memory;
    no binary texture container is shipped.
    """
    destination.mkdir(parents=True, exist_ok=True)
    image_directory = destination / "images"
    image_directory.mkdir(parents=True, exist_ok=True)
    terrain_keys = {atlas.image_key for atlas in terrain_atlases}
    route_names = {
        route.image_key: route.name
        for route in route_textures
    }
    referenced_images = {
        region.backing_key for region in sources.values()
        if region.backing_key not in terrain_keys
    }
    for stale in image_directory.glob("*.dds"):
        stale.unlink()
    for key, image in sorted(images.items()):
        if key not in referenced_images or key in route_names:
            continue
        # Pillow emits a conventional uncompressed 32-bit DDS here. Keeping
        # the shipping files uncompressed avoids BC block artifacts and lets
        # the dependency-free runtime decode them directly.
        image.save(image_directory / f"{key:016x}.dds")

    terrain_directory = image_directory / "terrain"
    if terrain_directory.exists():
        for stale in terrain_directory.rglob("*.dds"):
            stale.unlink()
    terrain_entries: list[dict[str, object]] = []
    for atlas in terrain_atlases:
        relative_name = Path(atlas.name).with_suffix(".dds")
        relative = Path("images") / "terrain" / relative_name
        output_path = destination / relative
        output_path.parent.mkdir(parents=True, exist_ok=True)
        images[atlas.image_key].save(output_path)
        terrain_entries.append({
            "name": atlas.name,
            "image": relative.as_posix(),
            "width": atlas.width,
            "height": atlas.height,
            "imageX": atlas.image_x,
            "imageY": atlas.image_y,
            "depth": atlas.depth,
            "indexHash": f"{atlas.index_hash:016x}",
            "indices": base64.b64encode(atlas.indices).decode("ascii"),
            "palette": [f"{pixel:04x}" for pixel in atlas.palette],
        })

    route_directory = image_directory / "route"
    if route_directory.exists():
        for stale in route_directory.rglob("*.dds"):
            stale.unlink()
    route_entries: list[dict[str, object]] = []
    for route in route_textures:
        relative_name = Path(route.name).with_suffix(".dds")
        relative = Path("images") / "route" / relative_name
        output_path = destination / relative
        output_path.parent.mkdir(parents=True, exist_ok=True)
        images[route.image_key].save(output_path)
        route_entries.append({
            "name": route.name,
            "image": relative.as_posix(),
            "width": route.width,
            "height": route.height,
            "key": f"{route.image_key:016x}",
            "source": route.source,
        })

    entries: list[dict[str, int | str]] = []
    for key, region in sorted(sources.items()):
        left, top, right, bottom = region.backing_box
        scale = (
            ROUTE_SCALE
            if region.backing_key in route_names
            else
            VEHICLE_SCALE
            if region.backing_key in vehicle_keys
            else
            UI_SCALE
            if region.backing_key in ui_keys
            else TEXTURE_SCALE
        )
        image = (
            f"images/route/{Path(route_names[region.backing_key]).with_suffix('.dds').as_posix()}"
            if region.backing_key in route_names
            else f"images/{region.backing_key:016x}.dds"
        )
        entries.append({
            "key": f"{key:016x}",
            "image": image,
            "x": left * scale,
            "y": top * scale,
            "width": (right - left) * scale,
            "height": (bottom - top) * scale,
        })
    manifest = {
        "format": 3,
        "generator": generator,
        "scale": TEXTURE_SCALE,
        "terrainScale": TERRAIN_SCALE,
        "vehicleScale": VEHICLE_SCALE,
        "routeScale": ROUTE_SCALE,
        "uiScale": UI_SCALE,
        "entries": entries,
        "terrainAtlases": terrain_entries,
        "routeTextures": route_entries,
        "sources": {
            f"{key:016x}": sorted(region.sources)
            for key, region in sorted(sources.items())
        },
        "warnings": warnings,
    }
    (destination / "manifest.json").write_text(
        json.dumps(manifest, indent=2), encoding="utf-8"
    )
    print(
        f"Wrote {len(referenced_images)} loose DDS textures + "
        f"{len(terrain_entries)} level terrain atlases / "
        f"{len(route_entries)} route textures / "
        f"{len(entries)} runtime "
        f"regions to {destination}",
        flush=True,
    )


def default_exp_paths(loose_root: Path) -> list[Path]:
    return sorted(
        path for path in loose_root.rglob("*.EXP")
        if "mods" not in {part.lower() for part in path.parts}
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--loose-root", type=Path, default=ROOT / "V8_2_LOOSE",
    )
    parser.add_argument(
        "--exp", type=Path, action="append",
        help="limit input to one or more EXP archives (repeatable)",
    )
    parser.add_argument("--realesrgan", type=Path)
    parser.add_argument(
        "--work", type=Path, default=ROOT / "build" / "v82_texture_pack_work",
    )
    parser.add_argument("--output", type=Path)
    parser.add_argument(
        "--inventory-only", action="store_true",
        help="Parse and report regions without running Real-ESRGAN",
    )
    parser.add_argument(
        "--prepare-only", action="store_true",
        help="write build-time source PNGs, then stop before upscaling",
    )
    parser.add_argument(
        "--category-list-only", action="store_true",
        help="write deterministic terrain/vehicle key lists, then stop",
    )
    parser.add_argument(
        "--reuse-esrgan", action="store_true",
        help="reuse PNG outputs already present below --work",
    )
    parser.add_argument(
        "--generator",
        default=(
            "Real-ESRGAN x2plus: world 2x, terrain and vehicles "
            "and routes and loading UI two-pass 4x"
        ),
        help="human-readable generator recorded in manifest.json",
    )
    parser.add_argument(
        "--match-dump", type=Path,
        help="Report how many runtime-dumped hashes exist in the offline set",
    )
    parser.add_argument(
        "--runtime-dump", type=Path, action="append",
        help="include exact decoded runtime regions (repeatable)",
    )
    args = parser.parse_args()
    loose_root = args.loose_root.resolve()
    exp_paths = (
        [path.resolve() for path in args.exp]
        if args.exp else default_exp_paths(loose_root)
    )
    if not exp_paths:
        parser.error(f"no EXP archives found below {loose_root}")
    regions, backing_images, warnings = collect_regions(exp_paths)
    vehicle_keys = collect_vehicle_backing_keys(regions)
    terrain_atlases = collect_terrain_atlases(exp_paths)
    for atlas in terrain_atlases:
        existing = backing_images.get(atlas.image_key)
        if existing is not None and existing.tobytes() != atlas.image.tobytes():
            raise RuntimeError(
                f"64-bit terrain atlas collision at {atlas.image_key:016x}"
            )
        backing_images[atlas.image_key] = atlas.image.copy()
    route_textures = collect_route_textures(exp_paths)
    route_keys = {route.image_key for route in route_textures}
    for route in route_textures:
        existing = backing_images.get(route.image_key)
        if existing is not None and existing.tobytes() != route.image.tobytes():
            raise RuntimeError(
                f"64-bit route texture collision at {route.image_key:016x}"
            )
        backing_images[route.image_key] = route.image.copy()
        add_region(
            regions,
            route.image,
            route.source,
            route.image_key,
            (0, 0, route.width, route.height),
        )
        if route.width % 2 == 0 and route.height % 2 == 0:
            half_width = route.width // 2
            half_height = route.height // 2
            for quadrant_y in range(2):
                for quadrant_x in range(2):
                    left = quadrant_x * half_width
                    top = quadrant_y * half_height
                    add_region(
                        regions,
                        route.image.crop((
                            left, top,
                            left + half_width,
                            top + half_height,
                        )),
                        f"{route.source}:quadrant{quadrant_y * 2 + quadrant_x}",
                        route.image_key,
                        (left, top, left + half_width, top + half_height),
                    )
    ui_keys = collect_runtime_dumps(
        [path.resolve() for path in (args.runtime_dump or [])],
        regions,
        backing_images,
    )
    pixels = sum(region.image.width * region.image.height for region in regions.values())
    source_pixels = sum(image.width * image.height for image in backing_images.values())
    print(
        f"Parsed {len(exp_paths)} archives: {len(backing_images)} unique source "
        f"textures, {len(regions)} authored runtime regions, "
        f"{len(terrain_atlases)} authored terrain atlases, "
        f"{len(route_textures)} authored route textures, "
        f"{len(vehicle_keys)} vehicle source images, "
        f"{source_pixels:,} source texels, {pixels:,} region texels, "
        f"{len(ui_keys)} runtime loading UI textures, {len(warnings)} warnings",
        flush=True,
    )
    if args.match_dump:
        dumped = {
            int(path.stem.split("_", 1)[0], 16)
            for path in args.match_dump.glob("*.rgba")
        }
        matched = dumped & regions.keys()
        print(
            f"Runtime dump match: {len(matched)}/{len(dumped)} "
            f"({(100.0 * len(matched) / max(1, len(dumped))):.1f}%)",
            flush=True,
        )
    if args.inventory_only:
        for warning in warnings[:100]:
            print("warning:", warning)
        return 0
    if args.category_list_only:
        work = args.work.resolve()
        work.mkdir(parents=True, exist_ok=True)
        (work / "vehicle_keys.txt").write_text(
            "".join(f"{key:016x}\n" for key in sorted(vehicle_keys)),
            encoding="utf-8",
        )
        (work / "terrain_keys.txt").write_text(
            "".join(
                f"{atlas.image_key:016x}\n"
                for atlas in sorted(terrain_atlases, key=lambda item: item.name)
            ),
            encoding="utf-8",
        )
        (work / "route_keys.txt").write_text(
            "".join(f"{key:016x}\n" for key in sorted(route_keys)),
            encoding="utf-8",
        )
        (work / "ui_keys.txt").write_text(
            "".join(f"{key:016x}\n" for key in sorted(ui_keys)),
            encoding="utf-8",
        )
        print(
            f"Wrote {len(vehicle_keys)} vehicle keys and "
            f"{len(terrain_atlases)} terrain keys and "
            f"{len(route_keys)} route keys and "
            f"{len(ui_keys)} runtime loading UI keys to {work}",
            flush=True,
        )
        return 0
    neural_sources = {
        key: Region(
            key, image, key, (0, 0, image.width, image.height), {"full texture"}
        )
        for key, image in backing_images.items()
    }
    if args.prepare_only:
        write_source_pngs(
            {key: region.image for key, region in neural_sources.items()},
            args.work.resolve(),
        )
        return 0
    if args.realesrgan is None and not args.reuse_esrgan:
        parser.error(
            "--realesrgan is required unless --inventory-only or "
            "--prepare-only or --reuse-esrgan is used"
        )
    executable = args.realesrgan.resolve() if args.realesrgan else Path()
    if not args.reuse_esrgan and not executable.exists():
        parser.error(f"Real-ESRGAN executable does not exist: {executable}")
    output = (
        args.output or loose_root / "mods" / "enhanced_textures_2x"
    ).resolve()
    upscaled = upscale_regions(
        neural_sources, args.work.resolve(), executable,
        {atlas.image_key for atlas in terrain_atlases}, vehicle_keys,
        route_keys,
        ui_keys,
        args.reuse_esrgan
    )
    write_loose_pack(
        upscaled, output, regions, terrain_atlases, route_textures,
        vehicle_keys, ui_keys, warnings, args.generator
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
