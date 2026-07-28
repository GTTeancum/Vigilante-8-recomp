#!/usr/bin/env python3
"""Build three independent V8 vehicles as new V8:2 roster entries."""

from __future__ import annotations

from dataclasses import replace
import hashlib
import json
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[3]
ADDONS = ROOT / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

from vigilante8_vehicle_tools import (  # noqa: E402
    conversion,
    iff,
    project,
    registry,
    stats,
)


V8_COMMON = (
    ROOT / "artifacts" / "dual_game_default_roundtrip"
    / "V8_COMMON_ORIGINAL.EXP"
)
V82_COMMON = (
    ROOT / "artifacts" / "dual_game_default_roundtrip"
    / "V82_COMMON_ORIGINAL.EXP"
)
V8_EXE = ROOT / "PS1 game" / "SLUS_005.10"
V82_EXE = ROOT / "V8_2_LOOSE" / "SLUS_008.68"
OUTPUT = ROOT / "artifacts" / "v8_to_v82_guest_roster"

VEHICLES = (
    (0, "guest.v8.chassey_blue", "Chassey Blue — V8 Guest"),
    (1, "guest.v8.slick_clyde", "Slick Clyde — V8 Guest"),
    (2, "guest.v8.sheila", "Sheila — V8 Guest"),
)


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def decode_bank(path: Path, game: str, index: int) -> project.ObjectBank:
    form = tuple(iff.parse(path.read_bytes()).forms(b"XOBF"))[index]
    model_form = iff.IffChunk(
        tag=b"FORM",
        form_type=b"XOBF",
        children=[
            child
            for child in form.children
            if child.tag in {b"BIN ", b"ANM "}
        ],
    )
    return registry._decode_bank(model_form, game)


def build_projects() -> tuple[project.VehicleProject, ...]:
    v8_stats = stats.StatsFile(V8_EXE.read_bytes(), "V8")
    v82_stats = stats.StatsFile(V82_EXE.read_bytes(), "V8_2")
    v8_wheel_library = decode_bank(V8_COMMON, "V8", 13)
    v82_transform_library = decode_bank(V82_COMMON, "V8_2", 18)
    native_modes = v82_stats.transform_modes()
    standard_wheel_roots = {
        record.get(field)
        for record in v82_stats.records()
        for field in ("wheel_kind_front", "wheel_kind_rear")
    }
    terrain_roots = {
        kind
        for mode in native_modes[1:]
        for kind in mode
    }

    result = []
    for source_index, stable_id, display_name in VEHICLES:
        source_values = v8_stats.record(source_index).values()
        source_values.pop("vehicle_type")

        body = conversion.v8_bank_to_v82(
            decode_bank(V8_COMMON, "V8", source_index)
        )
        body = conversion.add_v82_flamethrower_mount(body, 0)

        matched_front_wheel = conversion.closest_wheel_root(
            v8_wheel_library,
            source_values["wheel_kind_front"],
            v82_transform_library,
            standard_wheel_roots,
        )
        matched_rear_wheel = conversion.closest_wheel_root(
            v8_wheel_library,
            source_values["wheel_kind_rear"],
            v82_transform_library,
            standard_wheel_roots,
        )
        transform, transform_map = conversion.extract_roots(
            v82_transform_library,
            terrain_roots
            | {matched_front_wheel, matched_rear_wheel},
        )
        front_wheel = transform_map[matched_front_wheel]
        rear_wheel = transform_map[matched_rear_wheel]
        converted_stats = conversion.v8_stats_to_v82(
            source_values,
            front_wheel_kind=front_wheel,
            rear_wheel_kind=rear_wheel,
        )
        mapped_modes = tuple(
            tuple(
                0
                if mode_index == 0
                else transform_map[kind]
                for kind in mode
            )
            for mode_index, mode in enumerate(native_modes)
        )

        vehicle = project.VehicleProject(
            schema_version=project.SCHEMA_VERSION,
            stable_id=stable_id,
            display_name=display_name,
            game="V8_2",
            groups=body.groups,
            slots=body.slots,
            collisions=body.collisions,
            textures=body.textures,
            animations=body.animations,
            stats=converted_stats,
            body_kind=0,
            transformation_bank=transform,
            transform_modes=mapped_modes,
            powerups=v82_stats.powerup_values(),
        )
        vehicle.validate()
        result.append(vehicle)
    return tuple(result)


def main() -> None:
    projects = build_projects()
    package = registry.compile_package(projects)
    decoded = registry.decompile_package(package.archive, package.registry)
    if [project.to_dict(item) for item in decoded] != [
        project.to_dict(item) for item in projects
    ]:
        raise AssertionError(
            "three-entry V8-to-V8:2 package changed on semantic decode"
        )

    source = OUTPUT / "source_projects"
    source.mkdir(parents=True, exist_ok=True)
    for vehicle in projects:
        (source / f"{vehicle.stable_id}.json").write_text(
            json.dumps(project.to_dict(vehicle), indent=2) + "\n",
            encoding="utf-8",
        )
    pre_blender = OUTPUT / "pre_blender"
    pre_blender.mkdir(parents=True, exist_ok=True)
    (pre_blender / "CUSTOM.EXP").write_bytes(package.archive)
    (pre_blender / "VEHICLES.V8R").write_bytes(package.registry)

    manifest = {
        "source_game": "V8",
        "target_game": "V8_2",
        "retail_replacements": 0,
        "entries": [],
        "source_files": {
            str(V8_COMMON.relative_to(ROOT)): digest(V8_COMMON.read_bytes()),
            str(V8_EXE.relative_to(ROOT)): digest(V8_EXE.read_bytes()),
            str(V82_COMMON.relative_to(ROOT)): digest(V82_COMMON.read_bytes()),
            str(V82_EXE.relative_to(ROOT)): digest(V82_EXE.read_bytes()),
        },
        "pre_blender_package": {
            "CUSTOM.EXP": digest(package.archive),
            "VEHICLES.V8R": digest(package.registry),
        },
    }
    for source_record, vehicle in zip(VEHICLES, projects):
        body_usage = project.bank_memory_usage(vehicle)
        transform_usage = project.bank_memory_usage(
            vehicle.transformation_bank
        )
        manifest["entries"].append(
            {
                "source_form_and_stat_index": source_record[0],
                "stable_id": vehicle.stable_id,
                "display_name": vehicle.display_name,
                "body_slots": len(vehicle.slots),
                "body_groups": len(vehicle.groups),
                "body_textures": len(vehicle.textures),
                "body_animations": len(vehicle.animations),
                "body_native_texture_bytes": (
                    body_usage.native_texture_bytes
                ),
                "transform_slots": len(
                    vehicle.transformation_bank.slots
                ),
                "transform_groups": len(
                    vehicle.transformation_bank.groups
                ),
                "transform_native_texture_bytes": (
                    transform_usage.native_texture_bytes
                ),
                "wheel_anchor_keys": sorted(
                    slot.key
                    for slot in vehicle.slots
                    if slot.parent == vehicle.body_kind
                    and slot.key is not None
                    and 0x8000 <= slot.key <= 0x8005
                ),
                "weapon_mount_keys": sorted(
                    slot.key
                    for slot in vehicle.slots
                    if slot.parent == vehicle.body_kind
                    and slot.key is not None
                    and 0x8010 <= slot.key <= 0x8016
                ),
            }
        )
    (OUTPUT / "build_manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n",
        encoding="utf-8",
    )
    print(
        f"built {len(projects)} V8 guest entries; "
        f"CUSTOM.EXP={digest(package.archive)} "
        f"VEHICLES.V8R={digest(package.registry)}"
    )


if __name__ == "__main__":
    main()
