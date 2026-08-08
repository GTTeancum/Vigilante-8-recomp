#!/usr/bin/env python3
"""Audit native material semantics in imported V8 vehicle packages.

This is intentionally independent of the renderer.  It decodes the exact
CUSTOM.EXP/VEHICLES.V8R pair used by the loose build and reports every
semi-transparent face, including its native packet type, resolved texture,
palette transparency, vertex colour, and UV range.  The report makes it
possible to distinguish authored glass, reflection, and shadow-card packets
without guessing from a screenshot.
"""

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

from vigilante8_vehicle_tools import registry  # noqa: E402


@dataclass(frozen=True)
class FaceAudit:
    vehicle: str
    bank: str
    group: int
    face: int
    packet_index: int | None
    packet_kind: int | None
    packet_flags: int
    color: tuple[int, int, int]
    texture: int | None
    texture_size: tuple[int, int] | None
    palette_zero_entries: int | None
    palette_stp_entries: int | None
    uv_min: tuple[int, int]
    uv_max: tuple[int, int]
    material_parameter: int
    texture_flags: int
    environment_parameters: tuple[int, ...]


def palette_counts(texture) -> tuple[int, int]:
    palette = (
        texture.direct_pixels_bgr555
        if texture.depth == 2
        else texture.palette_bgr555
    )
    return (
        sum(pixel & 0x7FFF == 0 for pixel in palette),
        sum(bool(pixel & 0x8000) for pixel in palette),
    )


def audit_bank(vehicle: str, bank_name: str, bank) -> list[FaceAudit]:
    result: list[FaceAudit] = []
    for group_index, group in enumerate(bank.groups):
        for face_index, face in enumerate(group.faces):
            # Native bit 0x10 is the retail semitransparency selector used by
            # imported environment faces.  Other packet families encode their
            # blend mode in the same native type byte, so retain every
            # explicitly semitransparent face in the diagnostic report.
            if not (face.packet_flags & 0x10):
                continue
            texture = (
                bank.textures[face.texture]
                if face.texture is not None
                and 0 <= face.texture < len(bank.textures)
                else None
            )
            zeros, stp = (
                palette_counts(texture)
                if texture is not None
                else (None, None)
            )
            us = [pair[0] for pair in face.uv]
            vs = [pair[1] for pair in face.uv]
            result.append(
                FaceAudit(
                    vehicle=vehicle,
                    bank=bank_name,
                    group=group_index,
                    face=face_index,
                    packet_index=face.packet_index,
                    packet_kind=face.packet_kind,
                    packet_flags=face.packet_flags,
                    color=face.color,
                    texture=face.texture,
                    texture_size=(
                        (texture.width, texture.height)
                        if texture is not None
                        else None
                    ),
                    palette_zero_entries=zeros,
                    palette_stp_entries=stp,
                    uv_min=(min(us), min(vs)),
                    uv_max=(max(us), max(vs)),
                    material_parameter=face.material_parameter,
                    texture_flags=face.texture_flags,
                    environment_parameters=face.environment_parameters,
                )
            )
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--package",
        type=Path,
        default=REPO / "V8_2_LOOSE",
    )
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    package = args.package.resolve()
    archive = (package / "CUSTOM.EXP").read_bytes()
    roster = (package / "VEHICLES.V8R").read_bytes()
    vehicles = registry.decompile_package(archive, roster)
    faces: list[FaceAudit] = []
    for vehicle in vehicles:
        faces += audit_bank(vehicle.stable_id, "gameplay", vehicle)
        if vehicle.selector_preview_bank is not None:
            faces += audit_bank(
                vehicle.stable_id,
                "selector",
                vehicle.selector_preview_bank,
            )

    signatures = Counter(
        (
            face.bank,
            face.packet_kind,
            face.packet_flags,
            face.texture_size,
            face.palette_zero_entries,
            face.palette_stp_entries,
            face.color,
            face.material_parameter,
            face.texture_flags,
            face.environment_parameters,
        )
        for face in faces
    )
    report = {
        "schema": 1,
        "package": str(package),
        "vehicles": len(vehicles),
        "semitransparent_faces": len(faces),
        "signatures": [
            {
                "count": count,
                "bank": signature[0],
                "packet_kind": signature[1],
                "packet_flags": signature[2],
                "texture_size": signature[3],
                "palette_zero_entries": signature[4],
                "palette_stp_entries": signature[5],
                "color": signature[6],
                "material_parameter": signature[7],
                "texture_flags": signature[8],
                "environment_parameters": signature[9],
            }
            for signature, count in signatures.most_common()
        ],
        "faces": [asdict(face) for face in faces],
    }
    text = json.dumps(report, indent=2) + "\n"
    if args.output is not None:
        output = args.output.resolve()
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(text, encoding="utf-8")
    print(
        "[ImportedMaterialAudit] "
        f"vehicles={len(vehicles)} semitransparent_faces={len(faces)} "
        f"signatures={len(signatures)}"
    )
    for signature in report["signatures"][:20]:
        print(
            "  "
            f"count={signature['count']} bank={signature['bank']} "
            f"kind={signature['packet_kind']} "
            f"flags=0x{signature['packet_flags']:02X} "
            f"size={signature['texture_size']} "
            f"zero={signature['palette_zero_entries']} "
            f"stp={signature['palette_stp_entries']} "
            f"color={signature['color']} "
            f"env={signature['environment_parameters']}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
