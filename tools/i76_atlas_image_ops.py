#!/usr/bin/env python3
"""Pillow-side image operations for the I76 Blender atlas batch."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw


def build_atlas(manifest_path: Path) -> None:
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    width, height = manifest["atlas_size"]
    atlas = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    fidelity_dir = Path(manifest["fidelity_dir"])
    for tile in manifest["tiles"]:
        paste_y = height - tile["yBottom"] - tile["height"]
        if tile["kind"] == "image":
            record = manifest["records"][tile["sourceName"]]
            with Image.open(fidelity_dir / record["filename"]) as source:
                image = source.convert("RGBA")
                expected = (tile["width"], tile["height"])
                if image.size != expected:
                    raise RuntimeError(
                        f"{record['filename']}: got {image.size}, expected {expected}"
                    )
                atlas.alpha_composite(image, (tile["x"], paste_y))
        else:
            color = tuple(int(tile["key"][index : index + 2], 16) for index in (6, 8, 10, 12))
            atlas.alpha_composite(
                Image.new("RGBA", (tile["width"], tile["height"]), color),
                (tile["x"], paste_y),
            )
    output = Path(manifest["output"])
    output.parent.mkdir(parents=True, exist_ok=True)
    atlas.save(output)


def multiply_ao(base_path: Path, ao_path: Path, output_path: Path, strength: float) -> None:
    with Image.open(base_path) as base_file, Image.open(ao_path) as ao_file:
        base = base_file.convert("RGBA")
        ao = ao_file.convert("L")
        pixels = bytearray(base.tobytes())
        ao_pixels = ao.tobytes()
        for index, value in enumerate(ao_pixels):
            factor = (1.0 - strength) + strength * (value / 255.0)
            offset = index * 4
            pixels[offset + 0] = min(255, round(pixels[offset + 0] * factor))
            pixels[offset + 1] = min(255, round(pixels[offset + 1] * factor))
            pixels[offset + 2] = min(255, round(pixels[offset + 2] * factor))
        Image.frombytes("RGBA", base.size, bytes(pixels)).save(output_path)


def contact_sheet(paths: list[Path], output: Path) -> None:
    thumbs = []
    for path in paths:
        image = Image.open(path).convert("RGB")
        image.thumbnail((520, 368), Image.Resampling.LANCZOS)
        thumbs.append((path, image.copy()))
    columns = 2
    cell_w, cell_h = 560, 430
    rows = math.ceil(len(thumbs) / columns)
    sheet = Image.new("RGB", (columns * cell_w, rows * cell_h), (28, 28, 28))
    draw = ImageDraw.Draw(sheet)
    for index, (path, image) in enumerate(thumbs):
        x = (index % columns) * cell_w
        y = (index // columns) * cell_h
        sheet.paste(image, (x + (cell_w - image.width) // 2, y + 18))
        draw.text((x + 20, y + cell_h - 36), path.stem, fill=(230, 230, 230))
    output.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(output)


def shade_boost(manifest_path: Path) -> None:
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    width, height = manifest["atlas_size"]
    mask = Image.new("L", (width, height), 255)
    for polygon in manifest["polygons"]:
        value = max(0, min(255, round(255 * polygon["factor"])))
        points = [(round(x), round(y)) for x, y in polygon["points"]]
        if len(points) < 3:
            continue
        layer = Image.new("L", (width, height), 255)
        ImageDraw.Draw(layer).polygon(points, fill=value)
        mask = ImageChops.darker(mask, layer)

    with Image.open(manifest["base"]) as base_file:
        base = base_file.convert("RGBA")
    if base.size != (width, height):
        raise RuntimeError(f"base atlas size {base.size} != {(width, height)}")

    ao_path = manifest.get("ao")
    if ao_path and Path(ao_path).is_file():
        with Image.open(ao_path) as ao_file:
            ao = ao_file.convert("L")
        if ao.size != (width, height):
            raise RuntimeError(f"AO atlas size {ao.size} != {(width, height)}")
        ao_strength = float(manifest.get("ao_strength", 0.34))
        ao_pixels = ao.tobytes()
    else:
        ao_strength = 0.0
        ao_pixels = bytes([255]) * (width * height)

    pixels = bytearray(base.tobytes())
    mask_pixels = mask.tobytes()
    for index, value in enumerate(mask_pixels):
        procedural = value / 255.0
        ao_factor = (1.0 - ao_strength) + ao_strength * (ao_pixels[index] / 255.0)
        factor = min(procedural, ao_factor)
        offset = index * 4
        pixels[offset + 0] = min(255, round(pixels[offset + 0] * factor))
        pixels[offset + 1] = min(255, round(pixels[offset + 1] * factor))
        pixels[offset + 2] = min(255, round(pixels[offset + 2] * factor))

    output = Path(manifest["output"])
    output.parent.mkdir(parents=True, exist_ok=True)
    Image.frombytes("RGBA", base.size, bytes(pixels)).save(output)
    mask_output = manifest.get("mask_output")
    if mask_output:
        mask.save(mask_output)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    build = subparsers.add_parser("build-atlas")
    build.add_argument("manifest", type=Path)
    multiply = subparsers.add_parser("multiply-ao")
    multiply.add_argument("--base", required=True, type=Path)
    multiply.add_argument("--ao", required=True, type=Path)
    multiply.add_argument("--out", required=True, type=Path)
    multiply.add_argument("--strength", required=True, type=float)
    sheet = subparsers.add_parser("contact-sheet")
    sheet.add_argument("--out", required=True, type=Path)
    sheet.add_argument("images", nargs="+", type=Path)
    boost = subparsers.add_parser("shade-boost")
    boost.add_argument("manifest", type=Path)
    args = parser.parse_args()
    if args.command == "build-atlas":
        build_atlas(args.manifest)
    elif args.command == "multiply-ao":
        multiply_ao(args.base, args.ao, args.out, args.strength)
    elif args.command == "contact-sheet":
        contact_sheet(args.images, args.out)
    elif args.command == "shade-boost":
        shade_boost(args.manifest)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
