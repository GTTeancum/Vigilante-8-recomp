#!/usr/bin/env python3
"""Build the thirteen V8-exclusive vehicles as new V8:2 roster entries."""

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
    ROOT / "PS1 game" / "COMMON.EXP"
)
V8_SELECTOR_VEHICLES = (
    ROOT / "PS1 game" / "SHELL" / "VEHICLES.EXP"
)
V82_COMMON = (
    ROOT / "V8_2_LOOSE" / "SHARED" / "COMMON.EXP"
)
V8_EXE = ROOT / "PS1 game" / "SLUS_005.10"
V82_EXE = ROOT / "V8_2_LOOSE" / "SLUS_008.68"
OUTPUT = ROOT / "artifacts" / "v8_to_v82_guest_roster"

VEHICLES = (
    (0, "guest.v8.chassey_blue", "Chassey Blue", "'67 Rattler"),
    (1, "guest.v8.slick_clyde", "Slick Clyde", "'70 Clydesdale"),
    (2, "guest.v8.sheila", "Sheila", "'74 Strider"),
    (3, "guest.v8.john_torque", "John Torque", "'69 Jefferson"),
    (4, "guest.v8.dave", "Dave", "'70 Van"),
    (5, "guest.v8.convoy", "Convoy", "'72 Moth Truck"),
    (6, "guest.v8.loki", "Loki", "'73 Glenn 4x4"),
    (7, "guest.v8.houston_3", "Houston 3", "'75 Palamino"),
    (8, "guest.v8.boogie", "Boogie", "'76 Leprechaun"),
    (9, "guest.v8.beezwax", "Beezwax", "'70 Stag Pickup"),
    (10, "guest.v8.molo", "Molo", "'66 School Bus"),
    (11, "guest.v8.sid_burn", "Sid Burn", "'69 Manta"),
    (12, "guest.v8.y_the_alien", '"Y" the Alien', "'64 Luxo Saucer"),
)

SELECTOR_ASSETS = (
    *(
        ROOT / "V8_2_LOOSE" / "SHELL" / f"SELECTOR_{index:02}.PPM"
        for index in range(len(VEHICLES))
    ),
)

# Original V8 normally stores wheel/contact points as 0x8000-series children
# in each vehicle bank.  Y's type-12 constructor instead sources these four
# points from executable-owned vehicle data.  The values below are recovered
# from that native constructor output and are conversion metadata: both target
# banks receive ordinary V8:2 anchors, and no runtime code recognizes Y.
V8_EXECUTABLE_CONTACT_ANCHORS = {
    12: (
        (-20138, 4343, 47752),
        (20231, 4343, 47752),
        (-20138, 4343, -48406),
        (20231, 4343, -48406),
    ),
}


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
    v8_wheel_library = conversion.v8_bank_to_v82(
        decode_bank(V8_COMMON, "V8", 13)
    )
    v82_transform_library = decode_bank(V82_COMMON, "V8_2", 18)
    native_modes = v82_stats.transform_modes()
    terrain_roots = {
        kind
        for mode in native_modes[1:]
        for kind in mode
    }

    result = []
    for source_index, stable_id, display_name, _vehicle_name in VEHICLES:
        flying = source_index == 12
        source_values = v8_stats.record(source_index).values()
        source_values.pop("vehicle_type")

        body = conversion.v8_bank_to_v82(
            decode_bank(V8_COMMON, "V8", source_index)
        )
        body = conversion.add_v82_flamethrower_mount(body, 0)
        selector_preview = conversion.v8_bank_to_v82(
            decode_bank(V8_SELECTOR_VEHICLES, "V8", source_index)
        )
        executable_anchors = V8_EXECUTABLE_CONTACT_ANCHORS.get(source_index)
        if executable_anchors is not None:
            body = conversion.add_v82_contact_anchors(
                body, 0, executable_anchors
            )
            selector_preview = conversion.add_v82_contact_anchors(
                selector_preview, 0, executable_anchors
            )

        wheel_bank, wheel_map = conversion.extract_roots(
            v8_wheel_library,
            {
                source_values["wheel_kind_front"],
                source_values["wheel_kind_rear"],
            },
        )
        if flying:
            transform = conversion.collision_only_contact_bank(wheel_bank)
            front_wheel = wheel_map[source_values["wheel_kind_front"]]
            rear_wheel = wheel_map[source_values["wheel_kind_rear"]]
            mapped_modes = ()
        else:
            terrain_bank, terrain_map = conversion.extract_roots(
                v82_transform_library,
                terrain_roots,
            )
            transform, bank_bases = conversion.merge_banks(
                (wheel_bank, terrain_bank)
            )
            wheel_base, terrain_base = bank_bases
            front_wheel = (
                wheel_base + wheel_map[source_values["wheel_kind_front"]]
            )
            rear_wheel = (
                wheel_base + wheel_map[source_values["wheel_kind_rear"]]
            )
            mapped_modes = tuple(
                tuple(
                    0
                    if mode_index == 0
                    else terrain_base + terrain_map[kind]
                    for kind in mode
                )
                for mode_index, mode in enumerate(native_modes)
            )
        converted_stats = conversion.v8_stats_to_v82(
            source_values,
            front_wheel_kind=front_wheel,
            rear_wheel_kind=rear_wheel,
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
            selector_preview_bank=selector_preview,
            selector_preview_body_kind=0,
            transform_modes=mapped_modes,
            powerups=v82_stats.powerup_values(),
            controller_class="flying" if flying else "ground",
            supports_transformations=not flying,
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
            "full-roster V8-to-V8:2 package changed on semantic decode"
        )

    source = OUTPUT / "source_projects"
    source.mkdir(parents=True, exist_ok=True)
    expected_projects = {
        f"{vehicle.stable_id}.json" for vehicle in projects
    }
    for stale in source.glob("guest.v8.*.json"):
        if stale.name not in expected_projects:
            stale.unlink()
    for vehicle in projects:
        (source / f"{vehicle.stable_id}.json").write_text(
            json.dumps(project.to_dict(vehicle), indent=2) + "\n",
            encoding="utf-8",
        )
    pre_blender = OUTPUT / "pre_blender"
    pre_blender.mkdir(parents=True, exist_ok=True)
    selector_output = pre_blender / "SHELL"
    selector_output.mkdir(parents=True, exist_ok=True)
    expected_selector_assets = {asset.name for asset in SELECTOR_ASSETS}
    for stale in selector_output.glob("SELECTOR_*"):
        if stale.name not in expected_selector_assets:
            stale.unlink()
    (pre_blender / "CUSTOM.EXP").write_bytes(package.archive)
    (pre_blender / "VEHICLES.V8R").write_bytes(package.registry)
    selector_assets = []
    selector_hashes: set[str] = set()
    for reference in SELECTOR_ASSETS:
        asset = reference.read_bytes()
        asset_hash = digest(asset)
        if asset_hash in selector_hashes:
            raise AssertionError(
                f"duplicate original V8 selector banner: {reference.name}"
            )
        selector_hashes.add(asset_hash)
        (selector_output / reference.name).write_bytes(asset)
        selector_assets.append(
            {
                "name": f"SHELL/{reference.name}",
                "source": str(reference.relative_to(ROOT)),
                "source_sha256": asset_hash,
                "format": "original CHARSEL1 VLC record",
                "sha256": asset_hash,
            }
        )

    manifest = {
        "source_game": "V8",
        "target_game": "V8_2",
        "retail_replacements": 0,
        "entries": [],
        "source_files": {
            str(V8_COMMON.relative_to(ROOT)): digest(V8_COMMON.read_bytes()),
            str(V8_SELECTOR_VEHICLES.relative_to(ROOT)): digest(
                V8_SELECTOR_VEHICLES.read_bytes()
            ),
            str(V8_EXE.relative_to(ROOT)): digest(V8_EXE.read_bytes()),
            str(V82_COMMON.relative_to(ROOT)): digest(V82_COMMON.read_bytes()),
            str(V82_EXE.relative_to(ROOT)): digest(V82_EXE.read_bytes()),
        },
        "pre_blender_package": {
            "CUSTOM.EXP": digest(package.archive),
            "VEHICLES.V8R": digest(package.registry),
        },
        "native_selector_assets": selector_assets,
    }
    for source_record, vehicle in zip(VEHICLES, projects):
        body_usage = project.bank_memory_usage(vehicle)
        transform_usage = (
            project.bank_memory_usage(vehicle.transformation_bank)
            if vehicle.transformation_bank is not None
            else None
        )
        manifest["entries"].append(
            {
                "source_form_and_stat_index": source_record[0],
                "stable_id": vehicle.stable_id,
                "display_name": vehicle.display_name,
                "vehicle_name": source_record[3],
                "menu_stats": {
                    "armor": vehicle.stats["rating_armor"],
                    "speed": vehicle.stats["rating_speed"],
                    "handling": vehicle.stats["rating_handling"],
                    "special": vehicle.stats["rating_special"],
                    "shared_v8_fields": "exact source values scaled by 10",
                    "special_provenance": (
                        "sequel-only compatibility value: rounded mean of "
                        "the three authored V8 menu ratings"
                    ),
                },
                "supports_hot_rod": False,
                "controller_class": vehicle.controller_class,
                "supports_transformations": (
                    vehicle.supports_transformations
                ),
                "body_slots": len(vehicle.slots),
                "body_groups": len(vehicle.groups),
                "body_textures": len(vehicle.textures),
                "body_animations": len(vehicle.animations),
                "body_native_texture_bytes": (
                    body_usage.native_texture_bytes
                ),
                "transform_slots": (
                    len(vehicle.transformation_bank.slots)
                    if vehicle.transformation_bank is not None
                    else 0
                ),
                "transform_groups": (
                    len(vehicle.transformation_bank.groups)
                    if vehicle.transformation_bank is not None
                    else 0
                ),
                "transform_native_texture_bytes": (
                    transform_usage.native_texture_bytes
                    if transform_usage is not None
                    else 0
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
