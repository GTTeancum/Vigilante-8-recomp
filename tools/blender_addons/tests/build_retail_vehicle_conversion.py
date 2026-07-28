"""Convert a real retail V8:2 menu vehicle into independent native packages.

The left package is a canonical decoded conversion.  The right package applies
an obvious green palette/geometry edit before native compilation.  Neither
package retains the retail archive or any opaque packet bytes.
"""

from __future__ import annotations

import argparse
from dataclasses import replace
import json
import math
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[3]
ADDONS = ROOT / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

from vigilante8_vehicle_tools import authored_scene, iff, project, registry


def wheel_group() -> project.RenderGroup:
    vertices = []
    faces = []
    segments = 12
    for x in (-18, 18):
        vertices.extend(
            (
                x,
                round(math.cos(index * math.tau / segments) * 42),
                round(math.sin(index * math.tau / segments) * 42),
            )
            for index in range(segments)
        )
    vertices.extend(((-18, 0, 0), (18, 0, 0)))
    left_center = segments * 2
    right_center = left_center + 1
    for index in range(segments):
        nxt = (index + 1) % segments
        a, b = index, nxt
        c, d = segments + nxt, segments + index
        faces.extend(
            (
                project.Face((a, b, c), (18, 18, 20)),
                project.Face((a, c, d), (18, 18, 20)),
                project.Face((left_center, b, a), (104, 108, 116)),
                project.Face((right_center, d, c), (104, 108, 116)),
            )
        )
    return project.RenderGroup(
        name="independent_native_wheel",
        scale_shift=8,
        vertices=tuple(vertices),
        faces=tuple(faces),
    )


def attachment_position(key: int) -> tuple[int, int, int]:
    if key in (0x8004, 0x8005):
        return (-16000 if key == 0x8004 else 16000, 4000, 0)
    if 0x8010 <= key <= 0x8016:
        spread = key - 0x8013
        return (spread * 3500, -9000, 26000)
    if key == 0x801F:
        return (0, -12000, 8000)
    if key == 0x8100:
        return (0, -15000, -18000)
    return (0, -8000, 0)


def complete_slots(
    source: tuple[project.Slot, ...], wheel_group_index: int
) -> tuple[tuple[project.Slot, ...], int]:
    slots = list(source)
    for index in (0, 1):
        if index < len(slots) and slots[index].render_group is not None:
            slots[index] = replace(slots[index], render_group=None)
    required = (
        *(0x8000 + index for index in range(6)),
        *(0x8010 + index for index in range(7)),
        0x801F,
        0x8100,
        0x8101,
    )
    present = {slot.key for slot in slots if slot.key is not None}
    for key in required:
        if key not in present:
            slots.append(
                project.Slot(
                    name=f"independent_attachment_{key:04x}",
                    render_group=None,
                    collision=None,
                    key=key,
                    position=attachment_position(key),
                    parent=0,
                )
            )
    wheel_kind = len(slots)
    slots.append(
        project.Slot(
            name="independent_normal_wheel",
            render_group=wheel_group_index,
            collision=None,
        )
    )
    return tuple(slots), wheel_kind


def green_color(color: tuple[int, int, int]) -> tuple[int, int, int]:
    luminance = sum(color) // 3
    return (
        min(255, 20 + luminance // 4),
        min(255, 100 + luminance),
        min(255, 30 + luminance // 3),
    )


def green_palette_color(color: int) -> int:
    if color & 0x7FFF == 0:
        return color
    red = color & 31
    green = (color >> 5) & 31
    blue = (color >> 10) & 31
    luminance = (red + green + blue) // 3
    out_red = min(31, 3 + luminance // 4)
    out_green = min(31, 13 + luminance)
    out_blue = min(31, 4 + luminance // 3)
    return (
        (color & 0x8000)
        | out_red
        | (out_green << 5)
        | (out_blue << 10)
    )


def edited_vehicle(vehicle: project.VehicleProject) -> project.VehicleProject:
    groups = []
    for group_index, group in enumerate(vehicle.groups):
        groups.append(
            replace(
                group,
                vertices=tuple(
                    (
                        round(vertex[0] * (1.14 if group_index == 0 else 1.0)),
                        vertex[1] - (18 if group_index == 0 and vertex[1] < -40 else 0),
                        vertex[2],
                    )
                    for vertex in group.vertices
                ),
                faces=tuple(
                    replace(face, color=green_color(face.color))
                    for face in group.faces
                ),
            )
        )
    textures = tuple(
        replace(
            texture,
            name=f"edited_green_{texture.name}",
            palette_bgr555=tuple(
                green_palette_color(color)
                for color in texture.palette_bgr555
            ),
        )
        for texture in vehicle.textures
    )
    result = replace(
        vehicle,
        stable_id="proof.retail_v82_vehicle0.green_export",
        display_name="Retail V8:2 Vehicle 00 — Green Export",
        groups=tuple(groups),
        textures=textures,
    )
    result.validate()
    return result


def canonical_retail_vehicle(source: Path) -> project.VehicleProject:
    forms = tuple(iff.parse(source.read_bytes()).forms(b"XOBF"))
    if not forms:
        raise ValueError(f"{source} contains no XOBF forms")
    body = registry._decode_bank(forms[0], "V8_2")
    textures = tuple(
        replace(
            texture,
            palette_bgr555=(
                texture.palette_bgr555
                + (0,) * (
                    (16 if texture.depth == 0 else 256)
                    - len(texture.palette_bgr555)
                )
            ),
        )
        for texture in body.textures
    )
    wheel = wheel_group()
    groups = body.groups + (wheel,)
    slots, wheel_kind = complete_slots(body.slots, len(groups) - 1)

    base = authored_scene.new_project(
        "V8_2", "proof.retail_v82_vehicle0.canonical"
    )
    stat_values = dict(base.stats)
    stat_values["wheel_kind_front"] = wheel_kind
    stat_values["wheel_kind_rear"] = wheel_kind
    transform = replace(
        base.transformation_bank,
        groups=(wheel,),
    )
    result = replace(
        base,
        display_name="Retail V8:2 Vehicle 00 — Canonical Import",
        groups=groups,
        slots=slots,
        collisions=body.collisions,
        textures=textures,
        animations=body.animations,
        stats=stat_values,
        transformation_bank=transform,
    )
    result.validate()
    return result


def write_package(
    directory: Path, vehicle: project.VehicleProject
) -> registry.CompiledVehiclePackage:
    package = registry.compile_package((vehicle,))
    directory.mkdir(parents=True, exist_ok=True)
    (directory / "CUSTOM.EXP").write_bytes(package.archive)
    (directory / "VEHICLES.V8R").write_bytes(package.registry)
    return package


def main() -> None:
    arguments = sys.argv
    arguments = (
        arguments[arguments.index("--") + 1 :] if "--" in arguments else []
    )
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("original_output", type=Path)
    parser.add_argument("edited_output", type=Path)
    args = parser.parse_args(arguments)

    source = args.source.resolve()
    canonical = canonical_retail_vehicle(source)
    edited = edited_vehicle(canonical)
    original_package = write_package(args.original_output.resolve(), canonical)
    edited_package = write_package(args.edited_output.resolve(), edited)
    manifest = {
        "source": str(source),
        "source_archive_sha256": __import__("hashlib").sha256(
            source.read_bytes()).hexdigest().upper(),
        "source_form_index": 0,
        "decoded_groups": len(canonical.groups) - 1,
        "decoded_faces": sum(
            len(group.faces) for group in canonical.groups[:-1]),
        "decoded_textures": len(canonical.textures),
        "retained_source_bytes": 0,
        "canonical_archive_bytes": len(original_package.archive),
        "edited_archive_bytes": len(edited_package.archive),
    }
    (args.edited_output.resolve() / "conversion-proof.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(manifest, indent=2))


if __name__ == "__main__":
    main()
