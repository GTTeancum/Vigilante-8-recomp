#!/usr/bin/env python3
"""Programmatically audit the complete V8-to-V8:2 guest package."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import struct
import sys


ROOT = Path(__file__).resolve().parents[3]
TESTS = Path(__file__).resolve().parent
ADDONS = ROOT / "tools" / "blender_addons"
for path in (TESTS, ADDONS):
    if str(path) not in sys.path:
        sys.path.insert(0, str(path))

from build_v8_to_v82_guest_roster import (  # noqa: E402
    VEHICLES,
    V8_COMMON,
    V8_SELECTOR_VEHICLES,
    decode_bank,
)
from vigilante8_vehicle_tools import iff, project, registry, xobf  # noqa: E402


OUTPUT = ROOT / "artifacts" / "v8_to_v82_guest_roster"
FINAL = OUTPUT / "final"
EXPECTED_IDS = tuple(record[1] for record in VEHICLES)
RETAIL_ENVIRONMENT_ROLES = {
    (0x00, (0x3FFF, 0x8080, 0, 0)),
    (0x10, (0x7FFE, 0x8080, 0, 0)),
}


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def render_mode(native_packet_type: int) -> int:
    """Reproduce the identical retail V8/V8:2 packet-mode rewrite."""

    return (
        ((native_packet_type & 0x0F) << 2)
        | (0x40 if native_packet_type & 0x80 else 0)
        | (0x02 if native_packet_type & 0x10 else 0)
        | (0x80 if native_packet_type & 0x40 else 0)
    )


def environment_faces(
    bank: project.ObjectBank,
) -> tuple[project.Face, ...]:
    return tuple(
        face
        for group in bank.groups
        for face in group.faces
        if face.packet_kind == 12
    )


def validate_environment_translation(
    stable_id: str,
    label: str,
    source: project.ObjectBank,
    converted: project.ObjectBank,
) -> dict[str, int]:
    source_faces = environment_faces(source)
    converted_faces = environment_faces(converted)
    require(
        len(source_faces) == len(converted_faces),
        f"{stable_id} {label} changed environment face count",
    )

    opaque = 0
    gloss = 0
    for index, (before, after) in enumerate(
        zip(source_faces, converted_faces)
    ):
        before_type = before.packet_kind | before.packet_flags
        after_type = after.packet_kind | after.packet_flags
        expected_mode = render_mode(before_type)
        if before.packet_flags & 0x20:
            expected_mode |= 0x02
        require(
            expected_mode == render_mode(after_type),
            f"{stable_id} {label} face {index} did not map the V8 "
            "environment role to V8:2",
        )
        require(
            after.packet_flags & 0x20 == 0,
            f"{stable_id} {label} face {index} retained ignored V8 bit 0x20",
        )

        source_selector = before.environment_parameters[0]
        if (source_selector & 0x3FFF) == 0x3FFF:
            translucent = bool(before.packet_flags & 0x30)
            expected_flags = before.packet_flags & ~0x20
            if before.packet_flags & 0x20:
                expected_flags |= 0x10
            expected = (
                expected_flags,
                (
                    0x7FFE if translucent else 0x3FFF,
                    0x8080,
                    0,
                    0,
                ),
            )
            actual = (
                after.packet_flags,
                after.environment_parameters,
            )
            require(
                actual == expected,
                f"{stable_id} {label} face {index} is not a retail "
                "V8:2 environment role",
            )
            if expected[0] == 0x10:
                gloss += 1
            else:
                opaque += 1
        else:
            require(
                after.environment_parameters
                == (source_selector, 0x8080, 0, 0),
                f"{stable_id} {label} face {index} changed a local "
                "environment selector",
            )

    return {
        "faces": len(converted_faces),
        "opaque_arena_reflection": opaque,
        "translucent_gloss": gloss,
    }


def validate_native_texture_boundaries(forms: tuple[iff.IffChunk, ...]) -> int:
    texture_count = 0
    for bank_index, form in enumerate(forms):
        chunk = next(child for child in form.children if child.tag == b"BIN ")
        model = xobf.Model(chunk.payload, "V8_2")
        source = model.data
        table = model.texture_table_offset
        targets = tuple(
            table + struct.unpack_from("<I", source, table + index * 4)[0]
            for index in range(model.texture_count)
        )
        for texture_index, texture in enumerate(model.textures()):
            require(
                texture.supported,
                f"bank {bank_index} texture {texture_index} is unsupported",
            )
            if texture.depth == 2:
                image_offset = texture.offset
            else:
                image_offset = (
                    texture.offset
                    + struct.unpack_from("<I", source, texture.offset + 8)[0]
                )
                palette_end = (
                    texture.offset + 0x14 + len(texture.palette) * 2
                )
                require(
                    image_offset == palette_end - 8,
                    f"bank {bank_index} texture {texture_index} does not "
                    "use the retail CLUT/image overlap",
                )
            pixel_offset = image_offset + 0x14
            encoded_size = (
                xobf._compressed_input_size(
                    source[pixel_offset:],
                    len(texture.packed_pixels),
                )
                if texture.compressed
                else len(texture.packed_pixels)
            )
            declared_size = struct.unpack_from(
                "<I", source, image_offset + 8
            )[0]
            require(
                declared_size == encoded_size + 13,
                f"bank {bank_index} texture {texture_index} has an invalid "
                "native image-block size",
            )
            logical_end = pixel_offset + encoded_size
            runtime_end = (image_offset + declared_size + 11) & ~3
            expected_end = (
                targets[texture_index + 1]
                if texture_index + 1 < len(targets)
                else len(source)
            )
            require(
                runtime_end == expected_end,
                f"bank {bank_index} texture {texture_index} advances to "
                "the wrong native record boundary",
            )
            require(
                1 <= runtime_end - logical_end <= 4
                and source[logical_end:runtime_end]
                == b"\0" * (runtime_end - logical_end),
                f"bank {bank_index} texture {texture_index} lacks retail "
                "zero termination/alignment",
            )
            texture_count += 1
    return texture_count


def main() -> None:
    archive_path = FINAL / "CUSTOM.EXP"
    registry_path = FINAL / "VEHICLES.V8R"
    archive = archive_path.read_bytes()
    registry_data = registry_path.read_bytes()

    game, entries = registry.parse_registry(registry_data)
    forms = tuple(iff.parse(archive).forms(b"XOBF"))
    vehicles = registry.decompile_package(archive, registry_data)
    require(game == "V8_2", "registry is not a sequel package")
    require(
        tuple(entry.stable_id for entry in entries) == EXPECTED_IDS,
        "stable roster identities changed",
    )
    require(
        len(entries) == len(EXPECTED_IDS),
        "registry does not contain the complete V8-exclusive roster",
    )
    require(
        len(forms) == sum(
            2 + (vehicle.transformation_bank is not None)
            for vehicle in vehicles
        ),
        "archive does not contain the authored owned banks per vehicle",
    )

    bank_triples = tuple(
        (
            entry.archive_index,
            entry.transformation_archive_index,
            entry.selector_preview_archive_index,
        )
        for entry in entries
    )
    expected_triples_list = []
    next_bank = 0
    for vehicle in vehicles:
        body_bank = next_bank
        next_bank += 1
        transform_bank = (
            next_bank
            if vehicle.transformation_bank is not None
            else None
        )
        if transform_bank is not None:
            next_bank += 1
        selector_bank = next_bank
        next_bank += 1
        expected_triples_list.append(
            (body_bank, transform_bank, selector_bank)
        )
    expected_triples = tuple(expected_triples_list)
    require(
        bank_triples == expected_triples,
        "body/transformation/selector banks are not independently owned",
    )
    require(
        {
            bank
            for triple in bank_triples
            for bank in triple
            if bank is not None
        }
        == set(range(len(forms))),
        "archive has shared or unreferenced banks",
    )
    native_texture_count = validate_native_texture_boundaries(forms)

    rebuilt = registry.compile_package(vehicles)
    require(rebuilt.archive == archive, "native archive is not deterministic")
    require(
        rebuilt.registry == registry_data,
        "native registry is not deterministic",
    )

    vehicle_records = []
    for index, vehicle in enumerate(vehicles):
        vehicle.validate()
        require(
            vehicle.selector_preview_bank is not None,
            f"{vehicle.stable_id} has no owned selector bank",
        )
        require(
            vehicle.transformation_bank is not None,
            f"{vehicle.stable_id} has no authored wheel/contact bank",
        )

        direct_keys = {
            slot.key
            for slot in vehicle.slots
            if slot.parent == vehicle.body_kind and slot.key is not None
        }
        expected_wheels = {0x8000, 0x8001, 0x8002, 0x8003}
        require(
            direct_keys & set(range(0x8000, 0x8006))
            == expected_wheels,
            f"{vehicle.stable_id} does not expose four native contact anchors",
        )
        require(
            direct_keys & set(range(0x8010, 0x8017))
            == set(range(0x8010, 0x8017)),
            f"{vehicle.stable_id} does not expose all sequel weapon mounts",
        )
        require(
            (
                len(vehicle.transform_modes) == 4
                and all(len(mode) == 6 for mode in vehicle.transform_modes)
            )
            if vehicle.supports_transformations
            else vehicle.transform_modes == (),
            f"{vehicle.stable_id} transform table contradicts capability",
        )
        if not vehicle.supports_transformations:
            contact_bank = vehicle.transformation_bank
            require(
                len(contact_bank.groups) == 1
                and contact_bank.groups[0].faces == ()
                and contact_bank.groups[0].controls == ()
                and contact_bank.textures == ()
                and contact_bank.animations == (),
                f"{vehicle.stable_id} non-transformable contact bank renders geometry",
            )
            contact_roots = {
                index
                for index, slot in enumerate(contact_bank.slots)
                if slot.parent is None
            }
            require(
                vehicle.stats["wheel_kind_front"] in contact_roots
                and vehicle.stats["wheel_kind_rear"] in contact_roots
                and all(
                    contact_bank.slots[root].render_group == 0
                    and contact_bank.slots[root].collision is not None
                    for root in contact_roots
                ),
                f"{vehicle.stable_id} contact roots are not native collision-only objects",
            )

        source_index = VEHICLES[index][0]
        source_body = decode_bank(V8_COMMON, "V8", source_index)
        source_selector = decode_bank(
            V8_SELECTOR_VEHICLES,
            "V8",
            source_index,
        )
        body_environment = validate_environment_translation(
            vehicle.stable_id,
            "body",
            source_body,
            vehicle,
        )
        selector_environment = validate_environment_translation(
            vehicle.stable_id,
            "selector",
            source_selector,
            vehicle.selector_preview_bank,
        )

        source_project = (
            OUTPUT / "source_projects" / f"{vehicle.stable_id}.json"
        )
        blend = OUTPUT / "blender" / f"{vehicle.stable_id}.blend"
        require(source_project.is_file(), f"{vehicle.stable_id} JSON missing")
        require(blend.is_file(), f"{vehicle.stable_id} Blender file missing")
        vehicle_records.append(
            {
                "stable_id": vehicle.stable_id,
                "body_bank": bank_triples[index][0],
                "transformation_bank": bank_triples[index][1],
                "selector_bank": bank_triples[index][2],
                "controller_class": vehicle.controller_class,
                "supports_transformations": (
                    vehicle.supports_transformations
                ),
                "body_environment": body_environment,
                "selector_environment": selector_environment,
                "source_project_sha256": digest(source_project),
                "blend_sha256": digest(blend),
            }
        )

    beezwax = next(
        record
        for record in vehicle_records
        if record["stable_id"] == "guest.v8.beezwax"
    )
    require(
        beezwax["body_environment"]
        == {
            "faces": 26,
            "opaque_arena_reflection": 2,
            "translucent_gloss": 24,
        },
        "Beezwax body glass/reflection structure changed",
    )

    blender_proof = json.loads(
        (OUTPUT / "blender_roundtrip.json").read_text(encoding="utf-8")
    )
    require(
        blender_proof["byte_exact_to_pre_blender"] is True,
        "Blender export is not exact to the generated native package",
    )
    require(
        blender_proof["final_package"]["CUSTOM.EXP"]
        == digest(archive_path)
        and blender_proof["final_package"]["VEHICLES.V8R"]
        == digest(registry_path),
        "Blender proof hashes do not match the delivered package",
    )

    report = {
        "schema": "v8-to-v82-roster-validation-v3",
        "result": "PASS",
        "source_game": "V8",
        "target_game": "V8_2",
        "vehicle_count": len(vehicles),
        "owned_bank_count": len(forms),
        "native_texture_count": native_texture_count,
        "shared_banks": 0,
        "unreferenced_banks": 0,
        "blender_version": blender_proof["blender_version"],
        "blender_byte_exact_export": True,
        "retail_environment_roles": [
            {
                "packet_flags": flags,
                "environment_parameters": list(parameters),
            }
            for flags, parameters in sorted(RETAIL_ENVIRONMENT_ROLES)
        ],
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
        "vehicles": vehicle_records,
    }
    report_path = OUTPUT / "validation.json"
    report_path.write_text(
        json.dumps(report, indent=2) + "\n",
        encoding="utf-8",
    )
    print(
        f"PASS: {len(vehicles)} vehicles, {len(forms)} exclusive native "
        "banks, deterministic "
        f"compile, {native_texture_count} retail-compatible texture records, "
        "byte-exact Blender round trip, and source-to-V8:2 environment-role "
        "parity"
    )


if __name__ == "__main__":
    main()
