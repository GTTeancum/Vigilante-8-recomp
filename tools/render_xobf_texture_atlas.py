#!/usr/bin/env python3
"""Render every native XOBF texture in an EXP archive to a labeled atlas."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageDraw

from blender_addons.vigilante8_vehicle_tools.xobf import Model, TextureSlot
from terrain_object_identity_audit import collect_bins


def psx_color(value: int) -> tuple[int, int, int, int]:
    """Decode native PS1 BGR555/STP into an inspection-friendly RGBA value."""

    if value == 0:
        return 0, 0, 0, 0
    red = (value & 0x1F) * 255 // 31
    green = ((value >> 5) & 0x1F) * 255 // 31
    blue = ((value >> 10) & 0x1F) * 255 // 31
    # STP is a render-mode flag rather than conventional alpha.  Keep those
    # texels visible in the atlas; a magenta corner marker identifies textures
    # that use it without changing their decoded RGB.
    return red, green, blue, 255


def texture_image(texture: TextureSlot) -> tuple[Image.Image, bool]:
    image = Image.new("RGBA", (texture.width, texture.height))
    stp = False
    pixels = []
    if texture.direct_pixels_bgr555:
        source = texture.direct_pixels_bgr555
    else:
        source = tuple(texture.palette[index] for index in texture.indices)
    for value in source:
        stp |= bool(value & 0x8000)
        pixels.append(psx_color(value))
    image.putdata(pixels)
    return image, stp


def render_atlas(exp_path: Path, output: Path, scale: int) -> None:
    banks = collect_bins(exp_path)
    models = [Model(payload, "V8") for payload in banks]
    textures = [
        (bank_index, texture)
        for bank_index, model in enumerate(models)
        for texture in model.textures()
    ]
    if not textures:
        raise ValueError(f"{exp_path} contains no XOBF textures")

    columns = 8
    cell_width = 272
    cell_height = 184
    rows = (len(textures) + columns - 1) // columns
    atlas = Image.new(
        "RGB", (columns * cell_width, rows * cell_height), (31, 31, 35)
    )
    draw = ImageDraw.Draw(atlas)
    for atlas_index, (bank_index, texture) in enumerate(textures):
        column = atlas_index % columns
        row = atlas_index // columns
        left = column * cell_width
        top = row * cell_height
        draw.rectangle(
            (left + 4, top + 24, left + cell_width - 5, top + cell_height - 5),
            fill=(76, 76, 82),
        )
        decoded, has_stp = texture_image(texture)
        decoded = decoded.resize(
            (decoded.width * scale, decoded.height * scale),
            Image.Resampling.NEAREST,
        )
        # Composite transparent texels over a checkerboard.
        checker = Image.new("RGBA", decoded.size, (88, 88, 94, 255))
        checker_draw = ImageDraw.Draw(checker)
        for y in range(0, decoded.height, 8):
            for x in range(0, decoded.width, 8):
                if ((x // 8) + (y // 8)) & 1:
                    checker_draw.rectangle(
                        (x, y, x + 7, y + 7), fill=(116, 116, 122, 255)
                    )
        checker.alpha_composite(decoded)
        available_width = cell_width - 16
        available_height = cell_height - 36
        if checker.width > available_width or checker.height > available_height:
            ratio = min(
                available_width / checker.width,
                available_height / checker.height,
            )
            checker = checker.resize(
                (
                    max(1, round(checker.width * ratio)),
                    max(1, round(checker.height * ratio)),
                ),
                Image.Resampling.NEAREST,
            )
        atlas.paste(checker.convert("RGB"), (left + 8, top + 30))
        label = (
            f"B{bank_index}:T{texture.index:03d} "
            f"{texture.width}x{texture.height} "
            f"{'CI4' if texture.depth == 0 else 'CI8'}"
        )
        draw.text((left + 6, top + 6), label, fill=(235, 235, 235))
        if has_stp:
            draw.rectangle((left + cell_width - 18, top + 6,
                            left + cell_width - 7, top + 17),
                           fill=(255, 0, 255))

    output.parent.mkdir(parents=True, exist_ok=True)
    atlas.save(output)
    print(f"{len(textures)} textures across {len(banks)} banks -> {output}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exp", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--scale", type=int, default=2)
    args = parser.parse_args()
    if args.scale < 1:
        parser.error("--scale must be at least 1")
    render_atlas(args.exp, args.output, args.scale)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
