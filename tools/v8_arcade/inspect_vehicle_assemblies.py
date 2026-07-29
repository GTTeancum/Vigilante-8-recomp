#!/usr/bin/env python3
"""Find standalone Node assemblies that are not listed in the ISOP scene table."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from mathutils import Matrix

sys.path.insert(0, str(Path(__file__).resolve().parent))
from extract_vehicle_blends import collect_scene_instances, valid_node
from extract_vehicle_lods import decode_bundle, find_models, u32le


def inspect(path: Path) -> dict[str, object]:
    data, container = decode_bundle(path)
    image_limit = u32le(data, 8)
    models = find_models(data)
    model_by_offset = {model.offset: model for model in models}
    assemblies = []
    seen_roots = set()
    for model in models:
        # Standalone assemblies serialize a 48-byte Matrix, an array pointer,
        # a count immediately before the array, then the first child Model3
        # after 0-8 bytes of alignment padding.
        roots = []
        for children in range(max(4, model.offset - 0x80), model.offset, 4):
            if u32le(data, children) != model.offset:
                continue
            root = children - 56
            if (
                root >= 0
                and u32le(data, root + 48) == children
                and valid_node(data, root, image_limit)
            ):
                roots.append(root)
        if not roots:
            continue
        root = roots[0]
        if root in seen_roots:
            continue
        children = u32le(data, root + 48)
        instances, unknown = collect_scene_instances(
            data, root, image_limit, model_by_offset
        )
        if not instances:
            continue
        seen_roots.add(root)
        assemblies.append(
            {
                "root_offset": root,
                "child_count": u32le(data, children - 4),
                "model_instance_count": len(instances),
                "unique_model_offsets": sorted(
                    {instance["model"].offset for instance in instances}
                ),
                "triangle_count": sum(
                    instance["model"].triangle_count for instance in instances
                ),
                "unknown_object_count": len(unknown),
                "unknown_offsets": sorted({int(record["offset"]) for record in unknown}),
            }
        )
    return {
        "source": str(path.resolve()),
        "container": container,
        "discovered_model_count": len(models),
        "standalone_assemblies": assemblies,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path)
    parser.add_argument("--out", required=True, type=Path)
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None
    args = parser.parse_args(argv)
    records = [inspect(path) for path in args.inputs]
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(records, indent=2) + "\n", encoding="utf-8")
    for record in records:
        print(
            f"{Path(record['source']).name}: "
            f"{len(record['standalone_assemblies'])} standalone assemblies"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
