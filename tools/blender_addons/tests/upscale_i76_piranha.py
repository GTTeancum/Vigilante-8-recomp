#!/usr/bin/env python3
"""Extract, replace, and validate the Piranha's packed Blender textures."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

import bpy


ROOT = Path(__file__).resolve().parents[3]
UPSCALE_FACTOR = 2
SOURCE_BLEND = ROOT / "I76" / "22_picard_piranha.blend"
OUTPUT_DIRECTORY = ROOT / "I76" / "Upscaled"
OUTPUT_BLEND = OUTPUT_DIRECTORY / "picard_piranha_upscaled_2x.blend"
WORK_DIRECTORY = ROOT / "build" / "i76_piranha_upscale"
SOURCE_TEXTURES = WORK_DIRECTORY / "source"
UPSCALED_TEXTURES = WORK_DIRECTORY / "upscaled_2x"
FIDELITY_TEXTURES = WORK_DIRECTORY / "fidelity_2x"
TEXTURE_MANIFEST = WORK_DIRECTORY / "textures.json"


def safe_stem(name: str) -> str:
    return re.sub(r"[^a-z0-9]+", "_", name.lower()).strip("_")


def extract_textures() -> None:
    SOURCE_TEXTURES.mkdir(parents=True, exist_ok=True)
    records: list[dict[str, object]] = []
    for index, image in enumerate(sorted(bpy.data.images, key=lambda item: item.name)):
        filename = f"{index:02d}_{safe_stem(image.name)}.png"
        output_path = SOURCE_TEXTURES / filename
        image.filepath_raw = str(output_path)
        image.file_format = "PNG"
        image.save()
        records.append(
            {
                "image_name": image.name,
                "filename": filename,
                "width": image.size[0],
                "height": image.size[1],
                "transparent_palette_index": image.get(
                    "i76_transparent_palette_index"
                ),
            }
        )
    TEXTURE_MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    TEXTURE_MANIFEST.write_text(
        json.dumps(records, indent=2) + "\n", encoding="utf-8"
    )
    print(
        f"I76_PIRANHA_TEXTURES_EXTRACTED count={len(records)} "
        f"directory={SOURCE_TEXTURES}"
    )


def copy_custom_properties(
    source: bpy.types.ID, destination: bpy.types.ID
) -> None:
    for key in source.keys():
        destination[key] = source[key]


def build_upscaled_blend() -> None:
    bpy.ops.wm.open_mainfile(filepath=str(SOURCE_BLEND))
    records = json.loads(TEXTURE_MANIFEST.read_text(encoding="utf-8"))
    source_by_name = {image.name: image for image in bpy.data.images}
    replacement_count = 0
    for record in records:
        old_image = source_by_name[record["image_name"]]
        upscaled_path = FIDELITY_TEXTURES / record["filename"]
        if not upscaled_path.is_file():
            raise RuntimeError(f"missing upscaled texture: {upscaled_path}")
        new_image = bpy.data.images.load(str(upscaled_path), check_existing=False)
        expected_size = (
            record["width"] * UPSCALE_FACTOR,
            record["height"] * UPSCALE_FACTOR,
        )
        if tuple(new_image.size) != expected_size:
            raise RuntimeError(
                f"{record['filename']}: got {tuple(new_image.size)}, "
                f"expected {expected_size}"
            )
        new_image.name = (
            f"{record['image_name']} [Real-ESRGAN {UPSCALE_FACTOR}x]"
        )
        new_image.colorspace_settings.name = old_image.colorspace_settings.name
        new_image.alpha_mode = "STRAIGHT"
        copy_custom_properties(old_image, new_image)
        new_image["i76_upscaler"] = (
            "realesr-animevideov3 + source-edge preservation"
        )
        new_image["i76_upscale_factor"] = UPSCALE_FACTOR
        new_image["i76_original_width"] = record["width"]
        new_image["i76_original_height"] = record["height"]
        new_image.pack()

        for material in bpy.data.materials:
            if not material.use_nodes:
                continue
            for node in material.node_tree.nodes:
                if node.type == "TEX_IMAGE" and node.image == old_image:
                    node.image = new_image
                    replacement_count += 1
        bpy.data.images.remove(old_image)

    scene = bpy.context.scene
    scene["i76_upscaled_from"] = SOURCE_BLEND.name
    scene["i76_texture_upscaler"] = (
        "realesr-animevideov3 + source-edge preservation"
    )
    scene["i76_texture_upscale_factor"] = UPSCALE_FACTOR
    scene["i76_upscaled_texture_count"] = len(records)
    OUTPUT_DIRECTORY.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUTPUT_BLEND))
    backup_path = Path(str(OUTPUT_BLEND) + "1")
    if backup_path.exists():
        backup_path.unlink()
    print(
        f"I76_PIRANHA_UPSCALED_BLEND_OK textures={len(records)} "
        f"node_replacements={replacement_count} blend={OUTPUT_BLEND}"
    )


def validate_upscaled_blend() -> None:
    bpy.ops.wm.open_mainfile(filepath=str(OUTPUT_BLEND))
    records = json.loads(TEXTURE_MANIFEST.read_text(encoding="utf-8"))
    images = list(bpy.data.images)
    meshes = [obj for obj in bpy.data.objects if obj.type == "MESH"]
    if len(images) != len(records):
        raise RuntimeError(f"expected {len(records)} images, found {len(images)}")
    if len(meshes) != 21:
        raise RuntimeError(f"expected 21 meshes, found {len(meshes)}")
    if any(image.packed_file is None for image in images):
        raise RuntimeError("an upscaled image is not packed")
    if any(obj.type in {"CAMERA", "LIGHT"} for obj in bpy.data.objects):
        raise RuntimeError("presentation object found")

    by_original_name = {
        image.name.removesuffix(
            f" [Real-ESRGAN {UPSCALE_FACTOR}x]"
        ): image
        for image in images
    }
    cutout_count = 0
    for record in records:
        image = by_original_name[record["image_name"]]
        expected_size = (
            record["width"] * UPSCALE_FACTOR,
            record["height"] * UPSCALE_FACTOR,
        )
        if tuple(image.size) != expected_size:
            raise RuntimeError(
                f"{image.name}: got {tuple(image.size)}, expected {expected_size}"
            )
        if record["transparent_palette_index"] is not None:
            alpha = list(image.pixels)[3::4]
            if min(alpha) > 0.05 or max(alpha) < 0.95:
                raise RuntimeError(f"{image.name}: cutout alpha was lost")
            cutout_count += 1
    print(
        f"I76_PIRANHA_UPSCALED_VALIDATION_OK meshes={len(meshes)} "
        f"packed_images={len(images)} cutouts={cutout_count} "
        f"blend={OUTPUT_BLEND}"
    )


if __name__ == "__main__":
    arguments = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []
    if "--extract" in arguments:
        extract_textures()
    elif "--build" in arguments:
        build_upscaled_blend()
    elif "--validate" in arguments:
        validate_upscaled_blend()
    else:
        raise SystemExit("use --extract, --build, or --validate")
