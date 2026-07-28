#!/usr/bin/env python3
"""Audit the delivered three-entry V8-to-V8:2 package and its proofs."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[3]
ADDONS = ROOT / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

from vigilante8_vehicle_tools import iff, project, registry  # noqa: E402


OUTPUT = ROOT / "artifacts" / "v8_to_v82_guest_roster"
FINAL = OUTPUT / "final"
EXPECTED_IDS = (
    "guest.v8.chassey_blue",
    "guest.v8.slick_clyde",
    "guest.v8.sheila",
)
EXPECTED_WEAPONS = list(range(1, 8))
EXPECTED_POWERUPS = [
    "radar-jammer",
    "repair-wrench",
    "shield",
    "transform-1",
    "transform-2",
    "transform-3",
    "weapon-upgrade",
]
RUNS = {
    "guest.v8.chassey_blue": {
        "weapons": (
            OUTPUT / "runtime" / "isolated" / "chassey_weapons"
            / "summary.json"
        ),
        "powerups": (
            OUTPUT / "runtime" / "final_validation" / "chassey_powerups"
            / "summary.json"
        ),
    },
    "guest.v8.slick_clyde": {
        "weapons": (
            OUTPUT / "runtime" / "isolated" / "slick_weapons"
            / "summary.json"
        ),
        "powerups": (
            OUTPUT / "runtime" / "final_validation"
            / "slick_powerups_route66" / "summary.json"
        ),
    },
    "guest.v8.sheila": {
        "weapons": (
            OUTPUT / "runtime" / "isolated" / "sheila_weapons"
            / "summary.json"
        ),
        "powerups": (
            OUTPUT / "runtime" / "final_validation"
            / "sheila_powerups_route66" / "summary.json"
        ),
    },
}


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def main() -> None:
    archive_path = FINAL / "CUSTOM.EXP"
    registry_path = FINAL / "VEHICLES.V8R"
    archive = archive_path.read_bytes()
    registry_data = registry_path.read_bytes()

    game, entries = registry.parse_registry(registry_data)
    forms = tuple(iff.parse(archive).forms(b"XOBF"))
    vehicles = registry.decompile_package(archive, registry_data)
    require(game == "V8_2", "registry is not a sequel package")
    require(len(entries) == 3, "registry does not contain exactly three entries")
    require(len(forms) == 6, "archive does not contain three owned bank pairs")
    require(
        tuple(entry.stable_id for entry in entries) == EXPECTED_IDS,
        "stable roster identities changed",
    )
    bank_pairs = tuple(
        (entry.archive_index, entry.transformation_archive_index)
        for entry in entries
    )
    require(
        bank_pairs == ((0, 1), (2, 3), (4, 5)),
        "entries do not own exclusive body/transformation bank pairs",
    )
    require(
        {
            bank
            for pair in bank_pairs
            for bank in pair
            if bank is not None
        }
        == set(range(6)),
        "archive has shared or unreferenced support/donor banks",
    )
    rebuilt = registry.compile_package(vehicles)
    require(rebuilt.archive == archive, "native archive is not deterministic")
    require(
        rebuilt.registry == registry_data,
        "native registry is not deterministic",
    )

    vehicle_records = []
    for custom_offset, vehicle in enumerate(vehicles):
        vehicle.validate()
        direct_keys = {
            slot.key
            for slot in vehicle.slots
            if slot.parent == vehicle.body_kind and slot.key is not None
        }
        require(
            direct_keys & set(range(0x8000, 0x8006))
            == {0x8000, 0x8001, 0x8002, 0x8003},
            f"{vehicle.stable_id} does not preserve its four V8 wheel anchors",
        )
        require(
            direct_keys & set(range(0x8010, 0x8017))
            == set(range(0x8010, 0x8017)),
            f"{vehicle.stable_id} does not expose all sequel weapon mounts",
        )
        require(
            vehicle.transformation_bank is not None,
            f"{vehicle.stable_id} has no owned transformation bank",
        )
        require(
            len(vehicle.transform_modes) == 4
            and all(len(mode) == 6 for mode in vehicle.transform_modes),
            f"{vehicle.stable_id} lacks the complete four-by-six mode table",
        )

        runtime = {}
        for profile, summary_path in RUNS[vehicle.stable_id].items():
            summary = json.loads(summary_path.read_text(encoding="utf-8"))
            require(
                summary["coverageProfile"] == profile,
                f"{vehicle.stable_id} {profile} summary profile is wrong",
            )
            require(
                summary["totals"]["runs"] == 1
                and summary["totals"]["passed"] == 1
                and summary["totals"]["failed"] == 0,
                f"{vehicle.stable_id} {profile} run did not pass",
            )
            run = summary["runs"][0]
            require(
                run["last_frame"] >= 2700
                and run["collision_stream_rejections"] == 0,
                f"{vehicle.stable_id} {profile} run is incomplete",
            )
            if profile == "weapons":
                require(
                    run["weapon_armed"] == EXPECTED_WEAPONS
                    and run["weapon_fired"] == EXPECTED_WEAPONS,
                    f"{vehicle.stable_id} did not exercise all weapons",
                )
            else:
                require(
                    run["powerups"] == EXPECTED_POWERUPS,
                    f"{vehicle.stable_id} did not exercise all powerups",
                )
                require(
                    len(run["transformation_captures"]) == 3,
                    f"{vehicle.stable_id} lacks transformation screenshots",
                )
                require(
                    all(Path(path).is_file() for path in run["transformation_captures"]),
                    f"{vehicle.stable_id} transformation screenshot is missing",
                )
            runtime[profile] = {
                "summary": str(summary_path.relative_to(ROOT)),
                "frames": run["last_frame"],
                "arena": run["actual_overlay"],
                "collision_stream_rejections": (
                    run["collision_stream_rejections"]
                ),
                "weapons_armed": run["weapon_armed"],
                "weapons_fired": run["weapon_fired"],
                "powerups": run["powerups"],
                "transformation_captures": [
                    str(Path(path).relative_to(ROOT))
                    for path in run["transformation_captures"]
                ],
            }

        source_project = (
            OUTPUT / "source_projects" / f"{vehicle.stable_id}.json"
        )
        blend = OUTPUT / "blender" / f"{vehicle.stable_id}.blend"
        require(source_project.is_file(), f"{vehicle.stable_id} JSON is missing")
        require(blend.is_file(), f"{vehicle.stable_id} Blender file is missing")
        vehicle_records.append(
            {
                "stable_id": vehicle.stable_id,
                "display_name": vehicle.display_name,
                "assigned_runtime_type": 64 + custom_offset,
                "retail_replacement": False,
                "body_bank": bank_pairs[custom_offset][0],
                "transformation_bank": bank_pairs[custom_offset][1],
                "source_project": str(source_project.relative_to(ROOT)),
                "blend": str(blend.relative_to(ROOT)),
                "blend_sha256": digest(blend),
                "body_slots": len(vehicle.slots),
                "body_groups": len(vehicle.groups),
                "body_textures": len(vehicle.textures),
                "body_animations": len(vehicle.animations),
                "transformation_slots": len(
                    vehicle.transformation_bank.slots
                ),
                "transformation_groups": len(
                    vehicle.transformation_bank.groups
                ),
                "wheel_anchor_keys": [
                    "0x8000",
                    "0x8001",
                    "0x8002",
                    "0x8003",
                ],
                "weapon_mount_keys": [
                    f"0x{key:04X}" for key in range(0x8010, 0x8017)
                ],
                "runtime": runtime,
            }
        )

    blender_proof = json.loads(
        (OUTPUT / "blender_roundtrip.json").read_text(encoding="utf-8")
    )
    require(
        blender_proof["byte_exact_to_pre_blender"] is True,
        "Blender export is not exact to the generated native package",
    )
    require(
        blender_proof["final_package"]["CUSTOM.EXP"] == digest(archive_path)
        and blender_proof["final_package"]["VEHICLES.V8R"]
        == digest(registry_path),
        "Blender proof hashes do not match the delivered package",
    )

    proof_sheet = (
        OUTPUT / "proofs"
        / "three_v8_guests_standard_hover_float_ski.png"
    )
    require(proof_sheet.is_file(), "combined in-game proof sheet is missing")
    report = {
        "schema": "v8-to-v82-three-guest-validation-v1",
        "result": "PASS",
        "source_game": "V8",
        "target_game": "V8_2",
        "vehicle_count": 3,
        "retail_vehicle_count": 18,
        "first_custom_runtime_type": 64,
        "native_bank_count": 6,
        "retail_replacements": 0,
        "runtime_dependencies_on_retail_vehicle_entries": 0,
        "shared_owned_banks": 0,
        "unreferenced_support_or_donor_banks": 0,
        "opaque_payloads": 0,
        "embedded_source_archives": 0,
        "blender_version": blender_proof["blender_version"],
        "blender_byte_exact_export": True,
        "blender_addon": {
            "version": "0.3.1",
            "path": "artifacts\\vigilante8_vehicle_tools-0.3.1.zip",
            "sha256": digest(
                ROOT / "artifacts"
                / "vigilante8_vehicle_tools-0.3.1.zip"
            ),
        },
        "native_package": {
            "CUSTOM.EXP": {
                "path": str(archive_path.relative_to(ROOT)),
                "sha256": digest(archive_path),
            },
            "VEHICLES.V8R": {
                "path": str(registry_path.relative_to(ROOT)),
                "sha256": digest(registry_path),
            },
        },
        "proof_sheet": {
            "path": str(proof_sheet.relative_to(ROOT)),
            "sha256": digest(proof_sheet),
            "rows": ["Chassey Blue", "Slick Clyde", "Sheila"],
            "columns": ["Standard", "Hover", "Float", "Ski"],
        },
        "vehicles": vehicle_records,
    }
    report_path = OUTPUT / "validation.json"
    report_path.write_text(
        json.dumps(report, indent=2) + "\n",
        encoding="utf-8",
    )
    print(
        "PASS: 3 independent entries, 6 exclusive banks, exact Blender "
        "export, 3 powerup/transform runs, and 3 seven-weapon runs"
    )


if __name__ == "__main__":
    main()
