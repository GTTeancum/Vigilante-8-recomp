#!/usr/bin/env python3
"""Recover non-Scene vehicle assemblies that contain selected Model3 objects.

Vehicle bundles contain additional arrays of transformed Node roots behind
wrapper objects that the authored Scene walker does not understand yet.  This
tool starts from a known Model3, finds its immediate Node, then finds and
decodes every serialized pointer array that contains that Node.
"""

from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from extract_vehicle_blends import collect_scene_instances, valid_node
from extract_vehicle_lods import decode_bundle, find_models, u32le


MAX_ARRAY_COUNT = 128


def all_u32_references(data: bytes, value: int) -> list[int]:
    needle = struct.pack("<I", value)
    result = []
    cursor = 0
    while True:
        offset = data.find(needle, cursor)
        if offset < 0:
            return result
        if offset % 4 == 0:
            result.append(offset)
        cursor = offset + 1


def immediate_model_nodes(
    data: bytes, image_limit: int, model_offset: int
) -> list[int]:
    roots = []
    for reference in all_u32_references(data, model_offset):
        root = reference - 56
        if (
            root >= 0
            and u32le(data, root + 48) == reference
            and valid_node(data, root, image_limit)
        ):
            roots.append(root)
    return sorted(set(roots))


def containing_arrays(
    data: bytes,
    image_limit: int,
    value: int,
    model_by_offset: dict,
) -> list[dict[str, object]]:
    arrays = {}
    for reference in all_u32_references(data, value):
        for index in range(MAX_ARRAY_COUNT):
            start = reference - index * 4
            if start < 4:
                break
            count = u32le(data, start - 4)
            if not (index < count <= MAX_ARRAY_COUNT):
                continue
            if start + count * 4 > image_limit:
                continue
            pointers = [u32le(data, start + item * 4) for item in range(count)]
            if pointers[index] != value:
                continue
            if any(
                pointer < 0x20 or pointer >= image_limit or pointer % 4
                for pointer in pointers
            ):
                continue

            instances = []
            unknown = []
            for item, pointer in enumerate(pointers):
                item_instances, item_unknown = collect_scene_instances(
                    data, pointer, image_limit, model_by_offset
                )
                for instance in item_instances:
                    instances.append(
                        {
                            "array_index": item,
                            "path": str(instance["path"]),
                            "model_offset": instance["model"].offset,
                            "vertex_count": instance["model"].vertex_count,
                            "triangle_count": instance["model"].triangle_count,
                            "world_matrix": [
                                list(row) for row in instance["matrix"]
                            ],
                        }
                    )
                unknown.extend(
                    {
                        "array_index": item,
                        "path": str(record["path"]),
                        "offset": int(record["offset"]),
                    }
                    for record in item_unknown
                )
            arrays[start] = {
                "array_offset": start,
                "count_offset": start - 4,
                "count": count,
                "seed_index": index,
                "root_offsets": pointers,
                "model_instance_count": len(instances),
                "unique_model_count": len(
                    {instance["model_offset"] for instance in instances}
                ),
                "vertex_count": sum(
                    instance["vertex_count"] for instance in instances
                ),
                "triangle_count": sum(
                    instance["triangle_count"] for instance in instances
                ),
                "instances": instances,
                "unknown_objects": unknown,
            }
    return [arrays[offset] for offset in sorted(arrays)]


def inspect(path: Path, seeds: list[int]) -> dict[str, object]:
    data, container = decode_bundle(path)
    image_limit = u32le(data, 8)
    models = find_models(data)
    model_by_offset = {model.offset: model for model in models}
    records = []
    for seed in seeds:
        nodes = immediate_model_nodes(data, image_limit, seed)
        records.append(
            {
                "seed_model_offset": seed,
                "seed_model_nodes": nodes,
                "arrays": [
                    array
                    for node in nodes
                    for array in containing_arrays(
                        data, image_limit, node, model_by_offset
                    )
                ],
            }
        )
    return {
        "source": str(path.resolve()),
        "container": container,
        "discovered_model_count": len(models),
        "seeds": records,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("--model", action="append", required=True, type=lambda v: int(v, 0))
    parser.add_argument("--out", type=Path)
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None
    args = parser.parse_args(argv)
    result = inspect(args.input, args.model)
    text = json.dumps(result, indent=2) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
