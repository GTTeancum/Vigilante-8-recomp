#!/usr/bin/env python3
"""Inspect the serialized Scene/Node graph in V8 Arcade vehicle bundles.

This module intentionally uses only the Python standard library so it can be
run either by CPython or Blender's bundled Python.  It complements
``extract_vehicle_lods.py``: the older scanner finds every Model3 allocation,
while this tool follows the authored Scene and Node references that say which
models belong together.
"""

from __future__ import annotations

import argparse
import json
import math
import struct
import sys
from dataclasses import asdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from extract_vehicle_lods import Model3, decode_bundle, find_models


SCENE_TABLE_COUNT_OFFSET = 0xC0
SCENE_TABLE_OFFSET = 0xC4
SCENE_TABLE_ENTRY_SIZE = 16
NODE_SIZE = 52
MAX_SCENES = 64
MAX_CHILDREN = 4096


def u32le(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def matrix_be(data: bytes, offset: int) -> tuple[float, ...]:
    return struct.unpack_from(">12f", data, offset)


def valid_node(data: bytes, offset: int, image_limit: int) -> bool:
    if offset < 0x20 or offset + NODE_SIZE > image_limit or offset % 4:
        return False
    values = matrix_be(data, offset)
    if not all(math.isfinite(value) and abs(value) < 1_000_000 for value in values):
        return False
    # The first three rows are an affine 3x4 transform.  Reflected Node has a
    # 48-byte matrix at +0 and a children pointer at +48.
    children = u32le(data, offset + 48)
    if children == 0:
        return True
    if children < 4 or children >= image_limit or children % 4:
        return False
    count = u32le(data, children - 4)
    if count > MAX_CHILDREN or children + count * 4 > image_limit:
        return False
    return True


def scene_roots(data: bytes, image_limit: int) -> list[dict[str, object]]:
    count = u32le(data, SCENE_TABLE_COUNT_OFFSET)
    if not (1 <= count <= MAX_SCENES):
        raise RuntimeError(f"invalid scene table count {count}")
    end = SCENE_TABLE_OFFSET + count * SCENE_TABLE_ENTRY_SIZE
    if end > image_limit:
        raise RuntimeError("scene table extends past the ISOP memory image")

    scenes = []
    for index in range(count):
        entry = SCENE_TABLE_OFFSET + index * SCENE_TABLE_ENTRY_SIZE
        scene = u32le(data, entry)
        ambient = struct.unpack_from(">3f", data, entry + 4)
        if scene < 0x20 or scene + 4 > image_limit or scene % 4:
            raise RuntimeError(f"scene {index} has invalid object offset {scene:#x}")
        root = u32le(data, scene)
        scenes.append(
            {
                "index": index,
                "entry_offset": entry,
                "scene_offset": scene,
                "root_offset": root,
                "ambient_rgb": ambient,
            }
        )
    return scenes


def walk_scene(
    data: bytes,
    root: int,
    image_limit: int,
    model_by_offset: dict[int, Model3],
) -> dict[str, object]:
    nodes: list[dict[str, object]] = []
    models: list[dict[str, object]] = []
    unknown: list[dict[str, object]] = []
    active: set[int] = set()

    def visit(offset: int, path: str) -> None:
        model = model_by_offset.get(offset)
        if model is not None:
            models.append(
                {
                    "path": path,
                    "offset": offset,
                    "vertex_count": model.vertex_count,
                    "triangle_count": model.triangle_count,
                    "submesh_count": len(model.submeshes),
                    "bounds": model.bounds,
                }
            )
            return

        if not valid_node(data, offset, image_limit):
            unknown.append({"path": path, "offset": offset})
            return
        if offset in active:
            unknown.append({"path": path, "offset": offset, "cycle": True})
            return

        active.add(offset)
        children_offset = u32le(data, offset + 48)
        child_offsets: list[int] = []
        if children_offset:
            child_count = u32le(data, children_offset - 4)
            child_offsets = [
                u32le(data, children_offset + child_index * 4)
                for child_index in range(child_count)
            ]
        nodes.append(
            {
                "path": path,
                "offset": offset,
                "matrix": matrix_be(data, offset),
                "children_offset": children_offset,
                "children": child_offsets,
            }
        )
        for child_index, child in enumerate(child_offsets):
            visit(child, f"{path}/{child_index:03d}")
        active.remove(offset)

    visit(root, "root")
    return {
        "nodes": nodes,
        "models": models,
        "unknown_objects": unknown,
        "model_count": len(models),
        "unique_model_count": len({record["offset"] for record in models}),
        "vertex_count": sum(record["vertex_count"] for record in models),
        "triangle_count": sum(record["triangle_count"] for record in models),
    }


def inspect(path: Path) -> dict[str, object]:
    data, container = decode_bundle(path)
    image_limit = u32le(data, 8)
    models = find_models(data)
    model_by_offset = {model.offset: model for model in models}
    scenes = scene_roots(data, image_limit)
    for scene in scenes:
        scene.update(walk_scene(data, scene["root_offset"], image_limit, model_by_offset))

    referenced = {
        model["offset"] for scene in scenes for model in scene["models"]
    }
    return {
        "source": str(path.resolve()),
        "container": container,
        "memory_image_size": image_limit,
        "discovered_model_count": len(models),
        "referenced_model_count": len(referenced),
        "unreferenced_models": [
            {
                "offset": model.offset,
                "vertex_count": model.vertex_count,
                "triangle_count": model.triangle_count,
            }
            for model in models
            if model.offset not in referenced
        ],
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
