"""Generate reproducible binary-location and custom-record vehicle-stat proof."""

from __future__ import annotations

import hashlib
import json
from dataclasses import replace
from pathlib import Path
import sys

from capstone import Cs, CS_ARCH_MIPS, CS_MODE_LITTLE_ENDIAN, CS_MODE_MIPS32


ROOT = Path(__file__).resolve().parents[3]
ADDONS = ROOT / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

from vigilante8_vehicle_tools import registry, stats


RETAIL_EXECUTABLES = {
    "V8": ROOT / "PS1 game" / "SLUS_005.10",
    "V8_2": ROOT / "V8_2_WORK" / "disc" / "SLUS_008.68",
}

CUSTOM_PACKAGES = {
    "V8": (
        ROOT
        / "artifacts"
        / "dual_game_default_roundtrip"
        / "source_free_v8"
    ),
    "V8_2": (
        ROOT
        / "artifacts"
        / "dual_game_default_roundtrip"
        / "source_free_v82"
    ),
}

OUTPUT = ROOT / "artifacts" / "vehicle_plugin_proofs"


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def table_code_references(
    data: bytes,
    load_address: int,
    text_size: int,
    table_address: int,
    record_size: int,
    record_count: int,
) -> list[dict]:
    """Find MIPS LUI plus ADDIU/ORI constructions inside the table."""

    decoder = Cs(
        CS_ARCH_MIPS,
        CS_MODE_MIPS32 | CS_MODE_LITTLE_ENDIAN,
    )
    decoder.skipdata = True
    instructions = tuple(
        decoder.disasm(
            data[0x800 : 0x800 + text_size],
            load_address,
        )
    )
    table_end = table_address + record_size * record_count
    references = []
    for index, upper in enumerate(instructions):
        if upper.mnemonic != "lui":
            continue
        upper_parts = [part.strip() for part in upper.op_str.split(",")]
        if len(upper_parts) != 2:
            continue
        register = upper_parts[0]
        upper_immediate = int(upper_parts[1], 0) & 0xFFFF
        for lower in instructions[index + 1 : index + 6]:
            if lower.address > upper.address + 20:
                break
            if lower.mnemonic not in {"addiu", "ori"}:
                continue
            lower_parts = [part.strip() for part in lower.op_str.split(",")]
            if (
                len(lower_parts) != 3
                or lower_parts[0] != register
                or lower_parts[1] != register
            ):
                continue
            lower_immediate = int(lower_parts[2], 0) & 0xFFFF
            signed_lower = (
                lower_immediate
                if lower.mnemonic == "ori" or lower_immediate < 0x8000
                else lower_immediate - 0x10000
            )
            address = ((upper_immediate << 16) + signed_lower) & 0xFFFFFFFF
            if table_address <= address < table_end:
                references.append(
                    {
                        "lui_instruction_address": f"0x{upper.address:08X}",
                        "lui_instruction": f"{upper.mnemonic} {upper.op_str}",
                        "lower_instruction_address": f"0x{lower.address:08X}",
                        "lower_instruction": f"{lower.mnemonic} {lower.op_str}",
                        "constructed_record_address": f"0x{address:08X}",
                        "record_index": (address - table_address) // record_size,
                    }
                )
    return references


def retail_proof(game: str, path: Path) -> dict:
    data = path.read_bytes()
    parsed = stats.StatsFile(data)
    if parsed.profile.game != game:
        raise AssertionError(f"{path} detected as {parsed.profile.game}, not {game}")
    records = tuple(record.raw for record in parsed.records())
    vehicle_types = tuple(record.get("vehicle_type") for record in parsed.records())
    if vehicle_types != tuple(range(parsed.profile.record_count)):
        raise AssertionError(f"{game} vehicle-type sequence is not contiguous")
    health_field = parsed.field("health")
    before_health = parsed.record(0).get("health")
    mutated = stats.StatsFile(data, game)
    mutated.record(0).set("health", before_health + 1)
    executable_differences = [
        index
        for index, (before, after) in enumerate(
            zip(data, mutated.data, strict=True)
        )
        if before != after
    ]
    expected_health_offsets = list(
        range(
            parsed.table_offset + health_field.offset,
            parsed.table_offset + health_field.offset + health_field.size,
        )
    )
    if not executable_differences or any(
        offset not in expected_health_offsets
        for offset in executable_differences
    ):
        raise AssertionError(
            f"{game} retail health edit changed bytes outside its native field"
        )
    reparsed_mutation = stats.StatsFile(mutated.data, game)
    if reparsed_mutation.record(0).get("health") != before_health + 1:
        raise AssertionError(f"{game} retail health mutation did not decode")
    return {
        "game": game,
        "executable": str(path.resolve()),
        "executable_sha256": sha256(data),
        "load_address": f"0x{parsed.load_address:08X}",
        "table_address": f"0x{parsed.profile.table_address:08X}",
        "table_file_offset": f"0x{parsed.table_offset:X}",
        "record_size": f"0x{parsed.profile.record_size:X}",
        "record_count": parsed.profile.record_count,
        "table_sha256": sha256(b"".join(records)),
        "vehicle_type_sequence": list(vehicle_types),
        "first_record_hex": records[0].hex(" ").upper(),
        "last_record_hex": records[-1].hex(" ").upper(),
        "first_record_fields": parsed.record(0).values(),
        "field_mutation_proof": {
            "field": "health",
            "record_index": 0,
            "field_offset_in_record": f"0x{health_field.offset:X}",
            "before": before_health,
            "after": before_health + 1,
            "changed_executable_file_offsets": [
                f"0x{offset:X}" for offset in executable_differences
            ],
            "all_changed_bytes_inside_health_field": True,
            "mutated_executable_decoded_to_requested_value": True,
        },
        "code_references": table_code_references(
            data,
            parsed.load_address,
            parsed.text_size,
            parsed.profile.table_address,
            parsed.profile.record_size,
            parsed.profile.record_count,
        ),
    }


def custom_proof(game: str, directory: Path) -> dict:
    archive = (directory / "CUSTOM.EXP").read_bytes()
    registry_data = (directory / "VEHICLES.V8R").read_bytes()
    parsed_game, entries = registry.parse_registry(registry_data)
    if parsed_game != game or len(entries) != 1:
        raise AssertionError(f"{directory} is not the expected one-entry {game} package")
    projects = registry.decompile_package(archive, registry_data)
    project = projects[0]
    expected_record = registry.compile_stats_record(project, 0)
    if entries[0].stats_record != expected_record:
        raise AssertionError(f"{game} decoded fields do not reproduce the registry record")
    rebuilt = registry.compile_package(projects)
    if rebuilt.archive != archive or rebuilt.registry != registry_data:
        raise AssertionError(f"{game} package does not round-trip byte-exactly")
    record_offset = registry_data.find(entries[0].stats_record)
    health_field = stats.PROFILES[game].fields[
        next(
            index
            for index, field in enumerate(stats.PROFILES[game].fields)
            if field.name == "health"
        )
    ]
    mutated_stats = dict(project.stats)
    mutated_stats["health"] += 1
    mutated_project = replace(project, stats=mutated_stats)
    mutated = registry.compile_package((mutated_project,))
    registry_differences = [
        index
        for index, (before, after) in enumerate(
            zip(registry_data, mutated.registry, strict=True)
        )
        if before != after
    ]
    expected_health_offsets = list(
        range(
            record_offset + health_field.offset,
            record_offset + health_field.offset + health_field.size,
        )
    )
    if not registry_differences or any(
        offset not in expected_health_offsets
        for offset in registry_differences
    ):
        raise AssertionError(
            f"{game} health edit changed bytes outside its native field"
        )
    mutated_decoded = registry.decompile_package(
        mutated.archive,
        mutated.registry,
    )[0]
    if mutated_decoded.stats["health"] != mutated_stats["health"]:
        raise AssertionError(f"{game} health mutation did not decode")
    return {
        "game": game,
        "package_directory": str(directory.resolve()),
        "custom_exp_sha256": sha256(archive),
        "vehicles_v8r_sha256": sha256(registry_data),
        "stable_id": project.stable_id,
        "display_name": project.display_name,
        "stats_record_offset": f"0x{record_offset:X}",
        "stats_record_size": f"0x{len(entries[0].stats_record):X}",
        "stats_record_hex": entries[0].stats_record.hex(" ").upper(),
        "decoded_fields": dict(project.stats),
        "record_recompiled_byte_exact": True,
        "package_recompiled_byte_exact": True,
        "field_mutation_proof": {
            "field": "health",
            "field_offset_in_record": f"0x{health_field.offset:X}",
            "before": project.stats["health"],
            "after": mutated_stats["health"],
            "changed_registry_file_offsets": [
                f"0x{offset:X}" for offset in registry_differences
            ],
            "all_changed_bytes_inside_health_field": True,
            "mutated_record_decoded_to_requested_value": True,
            "custom_exp_unchanged": mutated.archive == archive,
        },
    }


def main() -> None:
    proof = {
        "retail_executables": [
            retail_proof(game, path)
            for game, path in RETAIL_EXECUTABLES.items()
        ],
        "custom_packages": [
            custom_proof(game, directory)
            for game, directory in CUSTOM_PACKAGES.items()
        ],
    }
    OUTPUT.mkdir(parents=True, exist_ok=True)
    output_path = OUTPUT / "stats_binary_proof.json"
    output_path.write_text(
        json.dumps(proof, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    print(json.dumps(proof, indent=2, sort_keys=True))
    print(f"PROOF_JSON={output_path.resolve()}")


if __name__ == "__main__":
    main()
