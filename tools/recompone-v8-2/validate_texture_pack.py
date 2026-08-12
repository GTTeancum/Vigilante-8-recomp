#!/usr/bin/env python3
"""Validate a V8:2 format-3 loose DDS texture replacement pack."""

from __future__ import annotations

import argparse
import base64
import json
from pathlib import Path
import struct


def dds_size(path: Path) -> tuple[int, int]:
    with path.open("rb") as stream:
        data = stream.read(128)
    if len(data) < 128 or data[:4] != b"DDS " or struct.unpack_from("<I", data, 4)[0] != 124:
        raise ValueError(f"invalid DDS header: {path}")
    height, width, pitch = struct.unpack_from("<III", data, 12)
    four_cc, bits = struct.unpack_from("<II", data, 84)
    if width <= 0 or height <= 0 or four_cc != 0 or bits != 32:
        raise ValueError(f"DDS must be uncompressed 32-bit RGBA: {path}")
    if pitch < width * 4 or path.stat().st_size < 128 + pitch * height:
        raise ValueError(f"truncated DDS pixels: {path}")
    return width, height


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pack", type=Path)
    args = parser.parse_args()
    pack = args.pack.resolve()
    manifest_path = pack / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    if (manifest.get("format") != 3 or
            manifest.get("scale") != 2 or
            manifest.get("terrainScale") != 4 or
            manifest.get("vehicleScale") != 4 or
            manifest.get("routeScale") != 4 or
            manifest.get("uiScale", 4) != 4):
        raise ValueError(
            "manifest must declare format 3, 2x world textures, 4x terrain, "
            "4x vehicles, 4x route textures, and 4x loading UI"
        )

    pack_prefix = str(pack) + str(Path("/"))
    sizes: dict[str, tuple[int, int]] = {}
    keys: set[str] = set()
    referenced: set[Path] = set()

    def resolve_dds(relative_text: str) -> Path:
        relative = relative_text.replace("/", str(Path("/")))
        path = (pack / relative).resolve()
        if (not str(path).startswith(pack_prefix) or
                path.suffix.lower() != ".dds"):
            raise ValueError(
                f"entry escapes pack or is not DDS: {relative_text}"
            )
        return path

    def size_of(relative_text: str) -> tuple[int, int]:
        size = sizes.get(relative_text)
        if size is None:
            size = dds_size(resolve_dds(relative_text))
            sizes[relative_text] = size
        return size

    for entry in manifest.get("entries", []):
        key = entry["key"].lower()
        if len(key) != 16 or int(key, 16) < 0 or key in keys:
            raise ValueError(f"invalid or duplicate content key: {key}")
        keys.add(key)
        path = resolve_dds(entry["image"])
        referenced.add(path)
        size = size_of(entry["image"])
        x, y = int(entry["x"]), int(entry["y"])
        width, height = int(entry["width"]), int(entry["height"])
        if x < 0 or y < 0 or width <= 0 or height <= 0 or x + width > size[0] or y + height > size[1]:
            raise ValueError(f"crop outside DDS for key {key}")

    vehicle_prefixes = (
        "V8_2_LOOSE/CUSTOM.EXP",
        "V8_2_LOOSE/SHARED/COMMON.EXP",
        "V8_2_LOOSE/SHARED/HOTRODS.EXP",
        "V8_2_LOOSE/SHELL/VEHICLES.EXP",
    )
    entries_by_key = {
        str(entry["key"]).lower(): entry
        for entry in manifest.get("entries", [])
    }
    vehicle_images: set[str] = set()
    for key, source_list in manifest.get("sources", {}).items():
        if not any(
                str(source).startswith(vehicle_prefixes)
                for source in source_list):
            continue
        entry = entries_by_key.get(str(key).lower())
        if entry is None:
            raise ValueError(f"vehicle source has no mapped entry: {key}")
        for field in ("x", "y", "width", "height"):
            if int(entry[field]) % 4 != 0:
                raise ValueError(
                    f"vehicle entry {key} field {field} is not at 4x scale"
                )
        vehicle_images.add(str(entry["image"]))
    if not vehicle_images:
        raise ValueError("pack contains no 4x vehicle textures")

    terrain_names: set[str] = set()
    for entry in manifest.get("terrainAtlases", []):
        name = str(entry["name"])
        if not name or name in terrain_names:
            raise ValueError(f"invalid or duplicate terrain name: {name}")
        terrain_names.add(name)
        if int(entry["depth"]) not in (0, 1, 2):
            raise ValueError(f"invalid terrain depth: {name}")
        palette = entry.get("palette", [])
        if len(palette) != 256 or any(
                len(value) != 4 or int(value, 16) < 0 for value in palette):
            raise ValueError(f"invalid terrain palette: {name}")
        index_hash = str(entry["indexHash"]).lower()
        if len(index_hash) != 16 or int(index_hash, 16) < 0:
            raise ValueError(f"invalid terrain index hash: {name}")
        source_width = int(entry["width"])
        source_height = int(entry["height"])
        indices = base64.b64decode(entry.get("indices", ""), validate=True)
        bytes_per_pixel = 2 if int(entry["depth"]) == 2 else 1
        if len(indices) != source_width * source_height * bytes_per_pixel:
            raise ValueError(f"invalid terrain source-index payload: {name}")
        path = resolve_dds(entry["image"])
        referenced.add(path)
        size = size_of(entry["image"])
        expected = (source_width * 4, source_height * 4)
        if size != expected:
            raise ValueError(
                f"terrain {name} is {size[0]}x{size[1]}; "
                f"required 4x size is {expected[0]}x{expected[1]}"
            )

    if not terrain_names:
        raise ValueError("pack contains no per-level terrain atlases")

    route_names: set[str] = set()
    route_images: set[str] = set()
    for route in manifest.get("routeTextures", []):
        name = str(route["name"])
        if not name or name in route_names:
            raise ValueError(f"invalid or duplicate route texture name: {name}")
        route_names.add(name)
        key = str(route["key"]).lower()
        if len(key) != 16 or int(key, 16) < 0:
            raise ValueError(f"invalid route texture key: {name}")
        source_width = int(route["width"])
        source_height = int(route["height"])
        path = resolve_dds(route["image"])
        referenced.add(path)
        size = size_of(route["image"])
        expected = (source_width * 4, source_height * 4)
        if size != expected:
            raise ValueError(
                f"route {name} is {size[0]}x{size[1]}; "
                f"required 4x size is {expected[0]}x{expected[1]}"
            )
        route_images.add(str(route["image"]))

    if not route_names:
        raise ValueError("pack contains no 4x XRTP route textures")

    for key, source_list in manifest.get("sources", {}).items():
        if not any(":XRTP" in str(source) for source in source_list):
            continue
        entry = entries_by_key.get(str(key).lower())
        if entry is None:
            raise ValueError(f"route source has no mapped entry: {key}")
        if str(entry["image"]) not in route_images:
            raise ValueError(f"route source {key} does not use a route DDS")
        for field in ("x", "y", "width", "height"):
            if int(entry[field]) % 4 != 0:
                raise ValueError(
                    f"route entry {key} field {field} is not at 4x scale"
                )

    runtime_ui_images: set[str] = set()
    for key, source_list in manifest.get("sources", {}).items():
        if not any(str(source).startswith("runtime:") for source in source_list):
            continue
        entry = entries_by_key.get(str(key).lower())
        if entry is None:
            raise ValueError(f"runtime UI source has no mapped entry: {key}")
        if str(entry["image"]).startswith("images/terrain/") or str(entry["image"]).startswith("images/route/"):
            continue
        for field in ("x", "y", "width", "height"):
            if int(entry[field]) % 4 != 0:
                raise ValueError(
                    f"runtime UI entry {key} field {field} is not at 4x scale"
                )
        runtime_ui_images.add(str(entry["image"]))

    shipped_files = set(
        path.resolve() for path in (pack / "images").rglob("*")
        if path.is_file()
    )
    non_dds = sorted(
        path for path in shipped_files if path.suffix.lower() != ".dds"
    )
    if non_dds:
        raise ValueError(f"non-DDS files in images/: {non_dds[:5]}")
    unused = sorted(shipped_files - referenced)
    if unused:
        raise ValueError(f"unreferenced DDS files: {unused[:5]}")
    maximum_width = max((size[0] for size in sizes.values()), default=0)
    maximum_height = max((size[1] for size in sizes.values()), default=0)
    print(
        f"PASS: {len(sizes)} DDS files, {len(keys)} mapped regions, "
        f"{len(terrain_names)} per-level 4x terrain atlases, "
        f"{len(vehicle_images)} 4x vehicle DDS files, "
        f"{len(route_names)} 4x route DDS files, "
        f"{len(runtime_ui_images)} 4x runtime UI DDS files, "
        f"maximum={maximum_width}x{maximum_height}, "
        f"generator={manifest.get('generator', 'unspecified')}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
