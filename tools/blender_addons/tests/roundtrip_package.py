"""Round-trip a native vehicle package through editable Blender scenes."""

from __future__ import annotations

import argparse
import hashlib
import os
from pathlib import Path
import struct
import sys

import bpy


ROOT = Path(__file__).resolve().parents[3]
ADDONS = Path(
    os.environ.get("V8_ADDON_ROOT", ROOT / "tools" / "blender_addons")
).resolve()
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

import vigilante8_vehicle_tools
from vigilante8_vehicle_tools import authored_scene, registry


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def parse_args() -> argparse.Namespace:
    arguments = sys.argv
    if "--" in arguments:
        arguments = arguments[arguments.index("--") + 1 :]
    else:
        arguments = []
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    return parser.parse_args(arguments)


def main() -> None:
    args = parse_args()
    source = args.source.resolve()
    output = args.output.resolve()
    archive_path = source / "CUSTOM.EXP"
    roster_path = source / "VEHICLES.V8R"
    original_archive = archive_path.read_bytes()
    original_roster = roster_path.read_bytes()

    loaded_from = Path(vigilante8_vehicle_tools.__file__).resolve()
    if not loaded_from.is_relative_to(ADDONS):
        raise RuntimeError(f"loaded add-on outside requested root: {loaded_from}")

    bpy.ops.wm.read_factory_settings(use_empty=True)
    vigilante8_vehicle_tools.register()
    projects = registry.decompile_package(original_archive, original_roster)
    rebuilt_projects = []
    for vehicle in projects:
        collection = authored_scene.project_to_scene(bpy.context, vehicle)
        if "v8_source_text" in collection:
            raise AssertionError("native import retained a source archive")
        rebuilt_projects.append(authored_scene.scene_to_project(collection))
    if any(text.get("v8_embedded_native_source") for text in bpy.data.texts):
        raise AssertionError("native import embedded source/blob text")

    rebuilt = registry.compile_package(rebuilt_projects)
    output.mkdir(parents=True, exist_ok=True)
    (output / "CUSTOM.EXP").write_bytes(rebuilt.archive)
    (output / "VEHICLES.V8R").write_bytes(rebuilt.registry)

    if rebuilt.archive != original_archive:
        raise AssertionError("CUSTOM.EXP changed during Blender round trip")
    source_registry_version = struct.unpack_from("<H", original_roster, 4)[0]
    if source_registry_version == registry.VERSION:
        if rebuilt.registry != original_roster:
            raise AssertionError(
                "VEHICLES.V8R changed during Blender round trip"
            )
        registry_result = "byte-exact"
    elif source_registry_version == registry.LEGACY_VERSION:
        if struct.unpack_from("<H", rebuilt.registry, 4)[0] != registry.VERSION:
            raise AssertionError(
                "legacy VEHICLES.V8R did not migrate to the current schema"
            )
        registry_result = (
            f"migrated-v{source_registry_version}-to-v{registry.VERSION}"
        )
    else:
        raise AssertionError(
            f"unexpected source registry version {source_registry_version}"
        )

    print(
        f"ROUND TRIP PASS game={rebuilt.game} vehicles={len(projects)} "
        f"CUSTOM.EXP={digest(rebuilt.archive)} "
        f"VEHICLES.V8R={digest(rebuilt.registry)} {registry_result}"
    )


if __name__ == "__main__":
    main()
