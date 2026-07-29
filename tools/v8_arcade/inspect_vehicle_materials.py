#!/usr/bin/env python3
"""Inventory V8 Arcade material parameters, samplers, and skin signatures."""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from extract_vehicle_blends import material_parameter_values
from extract_vehicle_lods import Model3, decode_bundle, find_models
from inspect_vehicle_scenes import scene_roots, u32le, valid_node
from inspect_vehicle_textures import xbox360_texture_candidates


def model_geometry_hash(data: bytes, model: Model3) -> str:
    digest = hashlib.sha256()
    for submesh in model.submeshes:
        vertex_end = (
            submesh.vertex_data_offset
            + submesh.vertex_count * submesh.vertex_stride
        )
        index_end = submesh.index_data_offset + submesh.triangle_count * 6
        digest.update(data[submesh.vertex_data_offset:vertex_end])
        digest.update(data[submesh.index_data_offset:index_end])
    return digest.hexdigest().upper()


def scene_models(
    data: bytes,
    root: int,
    image_limit: int,
    model_by_offset: dict[int, Model3],
) -> list[Model3]:
    result = []
    active: set[int] = set()

    def visit(offset: int) -> None:
        model = model_by_offset.get(offset)
        if model is not None:
            result.append(model)
            return
        if offset in active or not valid_node(data, offset, image_limit):
            return
        active.add(offset)
        children = u32le(data, offset + 48)
        if children:
            for index in range(u32le(data, children - 4)):
                visit(u32le(data, children + index * 4))
        active.remove(offset)

    visit(root)
    return result


def json_value(value: object) -> object:
    if isinstance(value, tuple):
        return [float(component) for component in value]
    return value


def inspect(path: Path) -> dict[str, object]:
    data, container = decode_bundle(path)
    image_limit = u32le(data, 8)
    models = find_models(data)
    model_by_offset = {model.offset: model for model in models}
    texture_records = xbox360_texture_candidates(data, image_limit, image_limit)
    texture_by_offset = {int(record["offset"]): record for record in texture_records}
    materials: dict[tuple[int, int], dict[str, object]] = {}

    for model in models:
        for submesh in model.submeshes:
            key = (submesh.technique_offset, submesh.parameters_offset)
            if key in materials:
                continue
            parameters = material_parameter_values(
                data, submesh.technique_offset, submesh.parameters_offset
            )
            decoded = []
            for parameter in parameters:
                record = {
                    "name": parameter["name"],
                    "register_set": parameter["register_set"],
                    "register_index": parameter["register_index"],
                    "parameter_offset": parameter["parameter_offset"],
                    "value": json_value(parameter["value"]),
                }
                if int(parameter["register_set"]) == 3:
                    texture = texture_by_offset.get(int(parameter["value"]))
                    if texture is not None:
                        record["texture"] = {
                            "offset": texture["offset"],
                            "base_address": texture["base_address"],
                            "width": texture["width"],
                            "height": texture["height"],
                            "format": texture["format"],
                        }
                decoded.append(record)
            fingerprint_source = json.dumps(decoded, sort_keys=True).encode("utf-8")
            materials[key] = {
                "technique_offset": submesh.technique_offset,
                "parameters_offset": submesh.parameters_offset,
                "fingerprint": hashlib.sha256(fingerprint_source).hexdigest().upper()[:16],
                "parameters": decoded,
            }

    scenes = []
    for scene in scene_roots(data, image_limit):
        members = scene_models(
            data,
            int(scene["root_offset"]),
            image_limit,
            model_by_offset,
        )
        scene_materials = []
        for model in members:
            for submesh in model.submeshes:
                material = materials[
                    (submesh.technique_offset, submesh.parameters_offset)
                ]
                if material["fingerprint"] not in {
                    item["fingerprint"] for item in scene_materials
                }:
                    scene_materials.append(material)
        scenes.append(
            {
                **scene,
                "model_offsets": [model.offset for model in members],
                "geometry_hashes": [
                    model_geometry_hash(data, model) for model in members
                ],
                "material_fingerprints": [
                    material["fingerprint"] for material in scene_materials
                ],
                "materials": scene_materials,
            }
        )

    return {
        "source": str(path.resolve()),
        "container": container,
        "scene_count": len(scenes),
        "material_count": len(materials),
        "scenes": scenes,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path)
    parser.add_argument("--out", type=Path)
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None
    args = parser.parse_args(argv)
    results = [inspect(path) for path in args.inputs]
    text = json.dumps(results, indent=2) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
