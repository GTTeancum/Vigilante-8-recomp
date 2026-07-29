#!/usr/bin/env python3
"""Extract native and PS1 bonus Model3 geometry from V8 Arcade IB/IBZ files.

The Xbox 360 game stores models as native big-endian vertex data inside an
ISOP relocatable image.  Pointer fields are little-endian file offsets.  This
tool recognizes the reflected Model3/SubMesh layouts recovered from
default.xex and exports geometry without running the game.

OBJ output intentionally contains positions and triangle faces only.  Vertex
attributes after position vary with each shader/technique and remain raw in
the source bundle for later format-specific decoding.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
import sys
import zlib
from dataclasses import asdict, dataclass
from pathlib import Path


MODEL3_SIZE = 28
SUBMESH_SIZE = 44
MAX_SUBMESHES = 32
MAX_VERTICES = 100_000
MAX_TRIANGLES = 1_000_000
KNOWN_VEHICLES = {
    "FartyDog",
    "GrooVan",
    "Incarcerator",
    "Jefferson",
    "Leprechaun",
    "Mammoth",
    "Manta",
    "Piranha",
    "Saucer",
    "Stag",
}


@dataclass(frozen=True)
class SubMesh:
    offset: int
    bounds: tuple[float, float, float, float, float, float]
    flags: int
    technique_offset: int
    parameters_offset: int
    vertex_reference_offset: int
    vertex_data_offset: int
    vertex_count: int
    vertex_stride: int
    index_data_offset: int
    triangle_count: int


@dataclass(frozen=True)
class Model3:
    offset: int
    bounds: tuple[float, float, float, float, float, float]
    submeshes_offset: int
    submeshes: tuple[SubMesh, ...]

    @property
    def vertex_count(self) -> int:
        return sum(submesh.vertex_count for submesh in self.submeshes)

    @property
    def triangle_count(self) -> int:
        return sum(submesh.triangle_count for submesh in self.submeshes)


def u32le(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def u32be(data: bytes, offset: int) -> int:
    return struct.unpack_from(">I", data, offset)[0]


def bounds_be(data: bytes, offset: int) -> tuple[float, ...]:
    return struct.unpack_from(">6f", data, offset)


def valid_bounds(bounds: tuple[float, ...]) -> bool:
    return (
        all(math.isfinite(value) and abs(value) < 1_000_000 for value in bounds)
        and all(bounds[axis] <= bounds[axis + 3] for axis in range(3))
        and any(bounds[axis + 3] - bounds[axis] > 0.00001 for axis in range(3))
    )


def decode_bundle(path: Path) -> tuple[bytes, str]:
    source = path.read_bytes()
    if source[:4] == b"ISOP":
        return source, "ib"

    if len(source) >= 6 and source[4:6] in (
        b"\x78\x01",
        b"\x78\x5e",
        b"\x78\x9c",
        b"\x78\xda",
    ):
        expected_size = u32le(source, 0)
        data = zlib.decompress(source[4:])
        if len(data) != expected_size:
            raise RuntimeError(
                f"{path}: IBZ declares {expected_size} bytes but expands to {len(data)}"
            )
        if data[:4] != b"ISOP":
            raise RuntimeError(f"{path}: decompressed data does not begin with ISOP")
        return data, "ibz"

    raise RuntimeError(f"{path}: not an ISOP IB or size-prefixed zlib IBZ file")


def parse_submesh(
    data: bytes,
    offset: int,
    image_limit: int,
    known_vertex_strides: dict[int, int] | None = None,
) -> SubMesh | None:
    if offset < 0 or offset + SUBMESH_SIZE > image_limit:
        return None

    bounds = bounds_be(data, offset)
    if not valid_bounds(bounds):
        return None

    flags = u32be(data, offset + 24)
    technique_offset = u32le(data, offset + 28)
    parameters_offset = u32le(data, offset + 32)
    vertex_reference_offset = u32le(data, offset + 36)
    index_data_offset = u32le(data, offset + 40)
    pointer_values = (
        technique_offset,
        parameters_offset,
        vertex_reference_offset,
        index_data_offset,
    )
    if any(
        pointer < 0x20 or pointer >= image_limit or pointer % 4
        for pointer in pointer_values
    ):
        return None
    if vertex_reference_offset + 8 > image_limit or index_data_offset < 4:
        return None

    vertex_data_offset = u32le(data, vertex_reference_offset)
    vertex_count = u32le(data, vertex_reference_offset + 4)
    triangle_count = u32le(data, index_data_offset - 4)
    if not (0x20 <= vertex_data_offset < image_limit):
        return None
    if not (0 < vertex_count <= MAX_VERTICES):
        return None
    if not (0 < triangle_count <= MAX_TRIANGLES):
        return None

    # Serialized vertex arrays are immediately followed by a four-byte
    # triangle count and the big-endian uint16 triangle-list indices.
    vertex_bytes = index_data_offset - 4 - vertex_data_offset
    vertex_stride = 0
    if vertex_bytes > 0 and vertex_bytes % vertex_count == 0:
        vertex_stride = vertex_bytes // vertex_count
    if not (12 <= vertex_stride <= 256 and vertex_stride % 4 == 0):
        # Some scene/skin variants reuse an earlier VertexArray while owning a
        # new index allocation. In that case the index allocation no longer
        # follows the vertex bytes, so recover the stride from another
        # submesh referencing the same serialized VertexArray.
        vertex_stride = (known_vertex_strides or {}).get(vertex_reference_offset, 0)
    if vertex_stride < 12 or vertex_stride > 256 or vertex_stride % 4:
        return None
    if vertex_data_offset + vertex_count * vertex_stride > len(data):
        return None

    index_end = index_data_offset + triangle_count * 3 * 2
    if index_end > len(data):
        return None
    indices = struct.unpack_from(f">{triangle_count * 3}H", data, index_data_offset)
    if max(indices, default=0) >= vertex_count:
        return None

    return SubMesh(
        offset=offset,
        bounds=tuple(bounds),
        flags=flags,
        technique_offset=technique_offset,
        parameters_offset=parameters_offset,
        vertex_reference_offset=vertex_reference_offset,
        vertex_data_offset=vertex_data_offset,
        vertex_count=vertex_count,
        vertex_stride=vertex_stride,
        index_data_offset=index_data_offset,
        triangle_count=triangle_count,
    )


def find_models(data: bytes) -> list[Model3]:
    if len(data) < 0x24 or data[:4] != b"ISOP":
        raise RuntimeError("input is not an ISOP bundle")

    image_limit = u32le(data, 8)
    if image_limit < 0x24 or image_limit > len(data):
        raise RuntimeError(
            f"invalid ISOP memory-image limit {image_limit:#x} for {len(data):#x} bytes"
        )

    def scan(known_vertex_strides: dict[int, int]) -> list[Model3]:
        result: list[Model3] = []
        for offset in range(0x24, image_limit - MODEL3_SIZE + 1, 4):
            bounds = bounds_be(data, offset)
            if not valid_bounds(bounds):
                continue

            submeshes_offset = u32le(data, offset + 24)

            # In the serialized allocation, the four-byte element count
            # precedes the pointed-to array. Model3 itself is followed
            # immediately by that count and its inline SubMesh array.
            if submeshes_offset != offset + MODEL3_SIZE + 4:
                continue
            submesh_count = u32le(data, submeshes_offset - 4)
            if not (1 <= submesh_count <= MAX_SUBMESHES):
                continue
            if submeshes_offset + submesh_count * SUBMESH_SIZE > image_limit:
                continue

            submeshes: list[SubMesh] = []
            for index in range(submesh_count):
                submesh = parse_submesh(
                    data,
                    submeshes_offset + index * SUBMESH_SIZE,
                    image_limit,
                    known_vertex_strides,
                )
                if submesh is None:
                    break
                submeshes.append(submesh)
            if len(submeshes) != submesh_count:
                continue

            result.append(
                Model3(
                    offset=offset,
                    bounds=tuple(bounds),
                    submeshes_offset=submeshes_offset,
                    submeshes=tuple(submeshes),
                )
            )
        return result

    models = scan({})
    known_vertex_strides = {
        submesh.vertex_reference_offset: submesh.vertex_stride
        for model in models
        for submesh in model.submeshes
    }
    return scan(known_vertex_strides)


def export_obj(path: Path, data: bytes, model: Model3, source_name: str) -> None:
    lines = [
        f"# Vigilante 8 Arcade Model3 from {source_name}",
        f"# Model3 file offset: 0x{model.offset:08X}",
        f"# Bounds: {' '.join(f'{value:.9g}' for value in model.bounds)}",
        f"o model3_{model.offset:08x}",
    ]
    vertex_base = 1
    for index, submesh in enumerate(model.submeshes):
        lines.extend(
            (
                f"g submesh_{index:02d}_at_{submesh.offset:08x}",
                f"usemtl technique_{submesh.technique_offset:08x}",
                (
                    f"# vertices={submesh.vertex_count} "
                    f"stride={submesh.vertex_stride} "
                    f"triangles={submesh.triangle_count}"
                ),
            )
        )
        for vertex_index in range(submesh.vertex_count):
            vertex_offset = (
                submesh.vertex_data_offset + vertex_index * submesh.vertex_stride
            )
            x, y, z = struct.unpack_from(">3f", data, vertex_offset)
            lines.append(f"v {x:.9g} {y:.9g} {z:.9g}")

        raw_indices = struct.unpack_from(
            f">{submesh.triangle_count * 3}H",
            data,
            submesh.index_data_offset,
        )
        for triangle_index in range(submesh.triangle_count):
            first = triangle_index * 3
            a, b, c = raw_indices[first : first + 3]
            lines.append(
                f"f {a + vertex_base} {b + vertex_base} {c + vertex_base}"
            )
        vertex_base += submesh.vertex_count

    path.write_text("\n".join(lines) + "\n", encoding="ascii")


def extent(bounds: tuple[float, ...]) -> tuple[float, float, float]:
    return tuple(bounds[index + 3] - bounds[index] for index in range(3))


def bounds_near(
    left: tuple[float, ...], right: tuple[float, ...], epsilon: float
) -> bool:
    scale = max(*(abs(value) for value in left + right), 1.0)
    return max(abs(a - b) for a, b in zip(left, right)) <= epsilon * scale


def same_bounds_model_families(
    models: list[Model3], epsilon: float
) -> list[dict[str, object]]:
    """Cluster similar-bounds models and rank their polygon counts.

    These are candidates, not authoritative labels: damage states and material
    variants can also share bounds.  The offsets and complexity ranking make
    those cases straightforward to compare visually.
    """

    unassigned = set(range(len(models)))
    families: list[dict[str, object]] = []
    while unassigned:
        seed = min(unassigned)
        group = {seed}
        changed = True
        while changed:
            changed = False
            for candidate in tuple(unassigned - group):
                if any(
                    bounds_near(
                        models[candidate].bounds, models[member].bounds, epsilon
                    )
                    for member in group
                ):
                    group.add(candidate)
                    changed = True
        unassigned -= group
        if len(group) < 2:
            continue

        ranked = sorted(
            group,
            key=lambda index: (
                models[index].triangle_count,
                models[index].vertex_count,
            ),
            reverse=True,
        )
        families.append(
            {
                "family": len(families),
                "warning": "damage/material variants can share bounds; not an LOD label",
                "models": [
                    {
                        "detail_rank": rank,
                        "model_index": model_index,
                        "offset": models[model_index].offset,
                        "vertex_count": models[model_index].vertex_count,
                        "triangle_count": models[model_index].triangle_count,
                        "bounds": models[model_index].bounds,
                    }
                    for rank, model_index in enumerate(ranked)
                ],
            }
        )
    return families


def probable_ps1_bonus_models(
    models: list[Model3], image_limit: int
) -> list[dict[str, object]]:
    """Identify the compact PS1 bonus vehicle models used by eight bundles.

    The standard archives place their converted PS1 bonus geometry in the final
    seven percent of the relocatable image. Those models have vehicle-scale
    bounds but only a few hundred triangles. FartyDog and Saucer intentionally
    produce no matches.
    """

    if not models:
        return []
    maximum_diagonal = max(
        math.sqrt(sum(component * component for component in extent(model.bounds)))
        for model in models
    )
    matches = []
    for model_index, model in enumerate(models):
        diagonal = math.sqrt(
            sum(component * component for component in extent(model.bounds))
        )
        if model.offset < image_limit * 0.93:
            continue
        if diagonal < maximum_diagonal * 0.7:
            continue
        if model.triangle_count > 1_000:
            continue
        matches.append(
            {
                "model_index": model_index,
                "offset": model.offset,
                "vertex_count": model.vertex_count,
                "triangle_count": model.triangle_count,
                "submesh_count": len(model.submeshes),
                "bounds": model.bounds,
                "confidence": "HIGH",
                "classification": "PS1 bonus vehicle geometry",
            }
        )
    return matches


def probable_native_vehicle_models(models: list[Model3]) -> list[dict[str, object]]:
    """Return high-complexity, vehicle-scale Xbox 360 model candidates.

    This deliberately includes duplicate, damage, and auxiliary full-vehicle
    models until model references are traced. It does not claim that every
    returned model is a runtime distance LOD.
    """

    if not models:
        return []
    maximum_diagonal = max(
        math.sqrt(sum(component * component for component in extent(model.bounds)))
        for model in models
    )
    matches = []
    for model_index, model in enumerate(models):
        diagonal = math.sqrt(
            sum(component * component for component in extent(model.bounds))
        )
        if diagonal < maximum_diagonal * 0.7:
            continue
        if model.triangle_count < 1_000:
            continue
        matches.append(
            {
                "model_index": model_index,
                "offset": model.offset,
                "vertex_count": model.vertex_count,
                "triangle_count": model.triangle_count,
                "submesh_count": len(model.submeshes),
                "bounds": model.bounds,
                "confidence": "MEDIUM",
                "classification": "Xbox 360 vehicle-scale Model3 candidate",
            }
        )
    return matches


def source_paths(inputs: list[Path]) -> list[Path]:
    paths: list[Path] = []
    for candidate in inputs:
        if candidate.is_dir():
            paths.extend(candidate.rglob("*.ib"))
            paths.extend(candidate.rglob("*.ibz"))
        else:
            paths.append(candidate)
    return sorted({path.resolve() for path in paths})


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path, help="IB/IBZ files or directories")
    parser.add_argument("--out", required=True, type=Path, help="output directory")
    parser.add_argument(
        "--bounds-epsilon",
        type=float,
        default=0.02,
        help="relative tolerance for same-bounds model clustering (default: 0.02)",
    )
    parser.add_argument(
        "--manifest-only",
        action="store_true",
        help="inspect models without writing OBJ files",
    )
    parser.add_argument(
        "--vehicles-only",
        action="store_true",
        help="when scanning directories, restrict input to the ten vehicle bundles",
    )
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else None
    args = parser.parse_args(argv)
    if args.bounds_epsilon < 0:
        parser.error("--bounds-epsilon must be non-negative")

    paths = source_paths(args.inputs)
    if args.vehicles_only:
        paths = [path for path in paths if path.stem in KNOWN_VEHICLES]
    if not paths:
        raise RuntimeError("no IB or IBZ inputs found")

    output_root = args.out.resolve()
    output_root.mkdir(parents=True, exist_ok=True)
    run_manifest: list[dict[str, object]] = []
    for source_path in paths:
        data, container = decode_bundle(source_path)
        models = find_models(data)
        bundle_output = output_root / source_path.stem
        bundle_output.mkdir(parents=True, exist_ok=True)

        model_records = []
        for model_index, model in enumerate(models):
            obj_name = f"model_{model_index:03d}_{model.offset:08x}.obj"
            if not args.manifest_only:
                export_obj(bundle_output / obj_name, data, model, source_path.name)
            record = asdict(model)
            record.update(
                {
                    "index": model_index,
                    "obj": None if args.manifest_only else obj_name,
                    "vertex_count": model.vertex_count,
                    "triangle_count": model.triangle_count,
                    "extent": extent(model.bounds),
                }
            )
            model_records.append(record)

        bundle_manifest = {
            "source": str(source_path),
            "source_sha256": hashlib.sha256(source_path.read_bytes()).hexdigest().upper(),
            "container": container,
            "isop_hash": f"0x{u32le(data, 4):08X}",
            "memory_image_size": u32le(data, 8),
            "model3_layout": {
                "size": MODEL3_SIZE,
                "minext_offset": 0,
                "maxext_offset": 12,
                "submeshes_offset": 24,
            },
            "submesh_layout": {
                "size": SUBMESH_SIZE,
                "minext_offset": 0,
                "maxext_offset": 12,
                "flags_offset": 24,
                "technique_offset": 28,
                "parameters_offset": 32,
                "verts_offset": 36,
                "indices_offset": 40,
            },
            "models": model_records,
            "probable_ps1_bonus_models": probable_ps1_bonus_models(
                models, u32le(data, 8)
            ),
            "probable_native_vehicle_models": probable_native_vehicle_models(models),
            "same_bounds_model_families": same_bounds_model_families(
                models, args.bounds_epsilon
            ),
        }
        manifest_path = bundle_output / "manifest.json"
        manifest_path.write_text(
            json.dumps(bundle_manifest, indent=2) + "\n", encoding="utf-8"
        )
        run_manifest.append(
            {
                "source": str(source_path),
                "output": str(bundle_output),
                "model_count": len(models),
                "vertex_count": sum(model.vertex_count for model in models),
                "triangle_count": sum(model.triangle_count for model in models),
                "probable_ps1_bonus_model_count": len(
                    bundle_manifest["probable_ps1_bonus_models"]
                ),
                "probable_native_vehicle_model_count": len(
                    bundle_manifest["probable_native_vehicle_models"]
                ),
                "same_bounds_model_family_count": len(
                    bundle_manifest["same_bounds_model_families"]
                ),
            }
        )
        print(
            f"{source_path.name}: {len(models)} Model3 objects, "
            f"{run_manifest[-1]['triangle_count']} triangles, "
            f"{run_manifest[-1]['probable_ps1_bonus_model_count']} PS1 bonus models, "
            f"{run_manifest[-1]['probable_native_vehicle_model_count']} native "
            "vehicle candidates, "
            f"{run_manifest[-1]['same_bounds_model_family_count']} same-bounds families"
        )

    (output_root / "manifest.json").write_text(
        json.dumps(run_manifest, indent=2) + "\n", encoding="utf-8"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
