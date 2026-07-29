#!/usr/bin/env python3
"""Export the isolated highest PS1 vehicle LOD Blender files.

No Xbox 360 distance LOD is emitted: the lower native body family is the
destroyed/wreck state, as shown by its missing panels and damage-only material.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import bpy

sys.path.insert(0, str(Path(__file__).resolve().parent))
from extract_vehicle_blends import (
    build_model_mesh,
    collect_scene_instances,
    infer_part_names,
    link_object,
    load_images,
    prepare_scene,
    referenced_texture_offsets,
    scene_roots,
)
from extract_vehicle_lods import Model3, decode_bundle, find_models, u32le
from inspect_vehicle_textures import xbox360_texture_candidates


def export_instances(
    source_path: Path,
    output_root: Path,
    data: bytes,
    image_limit: int,
    all_models: list[Model3],
    texture_by_offset: dict[int, dict[str, object]],
    instances: list[dict[str, object]],
    collection_name: str,
    version_kind: str,
    stem: str,
    source_metadata: dict[str, object],
) -> dict[str, object]:
    infer_part_names(instances)
    member_models = list(
        {
            instance["model"].offset: instance["model"]
            for instance in instances
        }.values()
    )
    triangle_count = sum(
        instance["model"].triangle_count for instance in instances
    )
    scene = prepare_scene(
        stem,
        source_path,
        image_limit,
        len(all_models),
        version_kind,
    )
    scene["model_instance_count"] = len(instances)
    scene["unique_model_count"] = len(member_models)
    scene["triangle_count"] = triangle_count
    for name, value in source_metadata.items():
        scene[name] = value

    texture_offsets = referenced_texture_offsets(
        data, member_models, texture_by_offset, image_limit
    )
    images = load_images(
        source_path.stem,
        data,
        texture_offsets,
        texture_by_offset,
        output_root / "textures" / source_path.stem,
    )
    material_cache = {}
    meshes = {
        model.offset: build_model_mesh(
            source_path.stem,
            data,
            model,
            set(texture_by_offset),
            images,
            texture_by_offset,
            material_cache,
        )
        for model in member_models
    }

    collection = bpy.data.collections.new(collection_name)
    scene.collection.children.link(collection)
    object_records = []
    for instance in instances:
        model = instance["model"]
        assert isinstance(model, Model3)
        name = f"{instance['part_name']}__Model3_{model.offset:08X}"
        obj = link_object(name, meshes[model.offset], collection, instance["matrix"])
        obj["part_name"] = str(instance["part_name"])
        obj["part_name_confidence"] = str(instance["part_name_confidence"])
        obj["part_name_basis"] = str(instance["part_name_basis"])
        obj["source_node_path"] = str(instance["path"])
        obj["source_model_offset"] = model.offset
        object_records.append(
            {
                "name": name,
                "part_name": instance["part_name"],
                "model_offset": model.offset,
                "vertex_count": model.vertex_count,
                "triangle_count": model.triangle_count,
            }
        )

    blend_path = output_root / source_path.stem / f"{stem}.blend"
    blend_path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path), compress=True)
    return {
        "file": str(blend_path.resolve()),
        "version_kind": version_kind,
        "collection": collection_name,
        "model_instance_count": len(instances),
        "unique_model_count": len(member_models),
        "triangle_count": triangle_count,
        "texture_count": len(texture_offsets),
        "objects": object_records,
        **source_metadata,
    }


def native_and_classic_scenes(
    data: bytes,
    image_limit: int,
    model_by_offset: dict[int, Model3],
) -> tuple[list[dict[str, object]], dict[str, object] | None]:
    native = []
    classic = None
    scenes = scene_roots(data, image_limit)
    for record in scenes:
        instances, unknown = collect_scene_instances(
            data, int(record["root_offset"]), image_limit, model_by_offset
        )
        triangles = sum(
            instance["model"].triangle_count for instance in instances
        )
        expanded = {
            **record,
            "instances": instances,
            "unknown": unknown,
            "triangle_count": triangles,
        }
        if (
            int(record["index"]) == len(scenes) - 1
            and 0 < triangles <= 1000
        ):
            classic = expanded
        else:
            native.append(expanded)
    return native, classic


def export_bundle(path: Path, output_root: Path) -> list[dict[str, object]]:
    data, _ = decode_bundle(path)
    image_limit = u32le(data, 8)
    models = find_models(data)
    model_by_offset = {model.offset: model for model in models}
    texture_records = xbox360_texture_candidates(data, image_limit, image_limit)
    texture_by_offset = {
        int(record["offset"]): record for record in texture_records
    }
    _native_scenes, classic_scene = native_and_classic_scenes(
        data, image_limit, model_by_offset
    )

    records = []
    if classic_scene is not None:
        records.append(
            export_instances(
                path,
                output_root,
                data,
                image_limit,
                models,
                texture_by_offset,
                list(classic_scene["instances"]),
                "PS1_HIGHEST_LOD",
                "ps1_highest_lod_only",
                f"{path.stem}_PS1_highest_LOD",
                {
                    "selection_basis": (
                        "highest and only authored PS1 classic scene"
                    ),
                    "source_scene_index": int(classic_scene["index"]),
                    "source_root_offset": int(classic_scene["root_offset"]),
                },
            )
        )

    manifest_path = output_root / path.stem / "manifest.json"
    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text(
        json.dumps(
            {
                "vehicle": path.stem,
                "source": str(path.resolve()),
                "deliverable": "isolated highest PS1 LOD only",
                "xbox360_distance_lod_status": (
                    "not emitted; no intact lower native distance LOD was "
                    "recovered"
                ),
                "files": records,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    for record in records:
        print(
            f"{Path(record['file']).name}: "
            f"{record['model_instance_count']} instances, "
            f"{record['triangle_count']} triangles"
        )
    return records


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path)
    parser.add_argument("--out", required=True, type=Path)
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None
    args = parser.parse_args(argv)
    output_root = args.out.resolve()
    output_root.mkdir(parents=True, exist_ok=True)
    all_records = []
    for path in args.inputs:
        all_records.extend(export_bundle(path, output_root))
    (output_root / "manifest.json").write_text(
        json.dumps(
            {
                "deliverable": "isolated highest PS1 LOD only",
                "xbox360_distance_lod_status": (
                    "not emitted; no intact lower native distance LOD was "
                    "recovered"
                ),
                "file_count": len(all_records),
                "files": all_records,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
