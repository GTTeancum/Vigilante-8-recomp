#!/usr/bin/env python3
"""Compare source, retail-target, and converted vehicle packet contracts."""

from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import asdict, dataclass
import json
from pathlib import Path
import sys


REPO = Path(__file__).resolve().parents[2]
ADDONS = REPO / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

from vigilante8_vehicle_tools import iff, registry  # noqa: E402


@dataclass(frozen=True)
class EnvironmentFace:
    archive: str
    dialect: str
    form: int
    group: int
    face: int
    packet_kind: int | None
    packet_flags: int
    environment_parameters: tuple[int, ...]
    texture: int | None
    color: tuple[int, int, int]
    preceding_same_vertices: bool
    preceding_packet_kind: int | None
    preceding_packet_flags: int | None
    preceding_texture: int | None


def decode_forms(path: Path, dialect: str):
    result = []
    for form in iff.parse(path.read_bytes()).forms(b"XOBF"):
        model = iff.IffChunk(
            tag=b"FORM",
            form_type=b"XOBF",
            children=[
                child for child in form.children
                if child.tag in {b"BIN ", b"ANM "}
            ],
        )
        result.append(registry._decode_bank(model, dialect))
    return result


def audit_archive(path: Path, dialect: str) -> dict[str, object]:
    banks = decode_forms(path, dialect)
    packet_signatures: Counter[tuple[object, ...]] = Counter()
    environment: list[EnvironmentFace] = []
    for form_index, bank in enumerate(banks):
        for group_index, group in enumerate(bank.groups):
            for face_index, face in enumerate(group.faces):
                packet_signatures[
                    (
                        face.packet_kind,
                        face.packet_flags,
                        len(face.environment_parameters),
                        face.environment_parameters,
                        face.texture is not None,
                    )
                ] += 1
                if not face.environment_parameters:
                    continue
                previous = group.faces[face_index - 1] if face_index else None
                same_vertices = (
                    previous is not None
                    and set(previous.vertices) == set(face.vertices)
                )
                environment.append(EnvironmentFace(
                    archive=str(path.resolve()),
                    dialect=dialect,
                    form=form_index,
                    group=group_index,
                    face=face_index,
                    packet_kind=face.packet_kind,
                    packet_flags=face.packet_flags,
                    environment_parameters=face.environment_parameters,
                    texture=face.texture,
                    color=face.color,
                    preceding_same_vertices=same_vertices,
                    preceding_packet_kind=(
                        previous.packet_kind if previous is not None else None
                    ),
                    preceding_packet_flags=(
                        previous.packet_flags if previous is not None else None
                    ),
                    preceding_texture=(
                        previous.texture if previous is not None else None
                    ),
                ))
    return {
        "path": str(path.resolve()),
        "dialect": dialect,
        "forms": len(banks),
        "groups": sum(len(bank.groups) for bank in banks),
        "faces": sum(
            len(group.faces) for bank in banks for group in bank.groups
        ),
        "environment_faces": len(environment),
        "packet_signatures": [
            {
                "count": count,
                "packet_kind": signature[0],
                "packet_flags": signature[1],
                "environment_parameter_count": signature[2],
                "environment_parameters": signature[3],
                "textured": signature[4],
            }
            for signature, count in packet_signatures.most_common()
        ],
        "environment": [asdict(face) for face in environment],
    }


def audit_converted_package(package: Path) -> dict[str, object]:
    vehicles = registry.decompile_package(
        (package / "CUSTOM.EXP").read_bytes(),
        (package / "VEHICLES.V8R").read_bytes(),
    )
    environment: list[dict[str, object]] = []
    signatures: Counter[tuple[object, ...]] = Counter()
    face_count = 0
    for vehicle_index, vehicle in enumerate(vehicles):
        for group_index, group in enumerate(vehicle.groups):
            face_count += len(group.faces)
            for face_index, face in enumerate(group.faces):
                signatures[
                    (
                        face.packet_kind,
                        face.packet_flags,
                        len(face.environment_parameters),
                        face.environment_parameters,
                        face.texture is not None,
                    )
                ] += 1
                if face.environment_parameters:
                    environment.append({
                        "vehicle": vehicle.stable_id,
                        "vehicle_index": vehicle_index,
                        "group": group_index,
                        "face": face_index,
                        "packet_kind": face.packet_kind,
                        "packet_flags": face.packet_flags,
                        "environment_parameters": face.environment_parameters,
                        "texture": face.texture,
                    })
    return {
        "path": str(package.resolve()),
        "dialect": "V8_2",
        "vehicles": len(vehicles),
        "faces": face_count,
        "environment_faces": len(environment),
        "packet_signatures": [
            {
                "count": count,
                "packet_kind": signature[0],
                "packet_flags": signature[1],
                "environment_parameter_count": signature[2],
                "environment_parameters": signature[3],
                "textured": signature[4],
            }
            for signature, count in signatures.most_common()
        ],
        "environment": environment,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--output",
        type=Path,
        default=REPO / "artifacts" / "v82-native-conversion" /
        "vehicle-packet-contracts.json",
    )
    args = parser.parse_args()
    report = {
        "schema": 1,
        "source_v8_common": audit_archive(
            REPO / "PS1 game" / "COMMON.EXP", "V8"
        ),
        "retail_v82_common": audit_archive(
            REPO / "V8_2_LOOSE" / "SHARED" / "COMMON.EXP", "V8_2"
        ),
        "source_v8_selector": audit_archive(
            REPO / "PS1 game" / "SHELL" / "VEHICLES.EXP", "V8"
        ),
        "retail_v82_selector": audit_archive(
            REPO / "V8_2_LOOSE" / "SHELL" / "VEHICLES.EXP", "V8_2"
        ),
        "converted_v8_package": audit_converted_package(
            REPO / "V8_2_LOOSE" / "mods" / "v8_to_v82_guest_roster"
        ),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    for name, section in report.items():
        if name == "schema":
            continue
        print(
            f"[{name}] faces={section['faces']} "
            f"environment={section['environment_faces']}"
        )
        for signature in section["packet_signatures"]:
            if signature["environment_parameter_count"]:
                print(
                    "  "
                    f"count={signature['count']} "
                    f"kind={signature['packet_kind']} "
                    f"flags=0x{signature['packet_flags']:02X} "
                    f"env={tuple(signature['environment_parameters'])} "
                    f"textured={int(signature['textured'])}"
                )
    print(f"[VehicleContractAudit] wrote {args.output.resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
