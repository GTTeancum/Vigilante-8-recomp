#!/usr/bin/env python3
"""Build the twelve V8-exclusive vehicles as new V8:2 roster entries."""

from __future__ import annotations

from dataclasses import replace
import hashlib
import json
from pathlib import Path
import struct
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
)

SELECTOR_REFERENCES = tuple(
    ROOT
    / "V8_2_LOOSE"
    / "SHELL"
    / f"SELECTOR_{index:02}.PPM"
    for index in range(len(VEHICLES))
)
# Runtime selector portraits are already the exact 260x422 native crop. Keep
# those durable assets beside the other SHELL data instead of depending on
# disposable captures under artifacts/.
SELECTOR_CROP = (0, 0, 260, 422)


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def crop_bmp_to_ppm(path: Path, crop: tuple[int, int, int, int]) -> bytes:
    """Extract the exact top-left native pixels from a 24-bit BMP capture."""
    data = path.read_bytes()
    if data[:2] != b"BM" or len(data) < 54:
        raise ValueError(f"selector reference is not a BMP: {path}")
    pixel_offset = struct.unpack_from("<I", data, 10)[0]
    dib_size = struct.unpack_from("<I", data, 14)[0]
    width = struct.unpack_from("<i", data, 18)[0]
    height = struct.unpack_from("<i", data, 22)[0]
    planes, bits = struct.unpack_from("<HH", data, 26)
    compression = struct.unpack_from("<I", data, 30)[0]
    if dib_size < 40 or width <= 0 or height == 0 or planes != 1:
        raise ValueError(f"unsupported selector BMP header: {path}")
    if bits != 24 or compression != 0:
        raise ValueError(f"selector BMP must be uncompressed 24-bit RGB: {path}")

    x, y, crop_width, crop_height = crop
    image_height = abs(height)
    if x < 0 or y < 0 or x + crop_width > width or y + crop_height > image_height:
        raise ValueError(f"selector crop is outside {width}x{image_height}: {path}")
    stride = (width * 3 + 3) & ~3
    rgb = bytearray()
    bottom_up = height > 0
    for output_y in range(y, y + crop_height):
        source_y = image_height - 1 - output_y if bottom_up else output_y
        row = pixel_offset + source_y * stride + x * 3
        for output_x in range(crop_width):
            blue, green, red = data[row + output_x * 3 : row + output_x * 3 + 3]
            rgb.extend((red, green, blue))
    return f"P6\n{crop_width} {crop_height}\n255\n".encode("ascii") + rgb


def crop_ppm_to_ppm(path: Path, crop: tuple[int, int, int, int]) -> bytes:
    """Extract exact top-left pixels from a binary P6 selector capture."""
    data = path.read_bytes()
    cursor = 0

    def token() -> bytes:
        nonlocal cursor
        while cursor < len(data):
            if data[cursor] == ord("#"):
                cursor = data.find(b"\n", cursor)
                if cursor < 0:
                    raise ValueError(f"truncated PPM comment: {path}")
            if cursor < len(data) and data[cursor] in b" \t\r\n":
                cursor += 1
                continue
            break
        start = cursor
        while cursor < len(data) and data[cursor] not in b" \t\r\n":
            cursor += 1
        return data[start:cursor]

    if token() != b"P6":
        raise ValueError(f"selector reference is not a binary P6 PPM: {path}")
    width = int(token())
    height = int(token())
    if int(token()) != 255:
        raise ValueError(f"selector PPM is not 8-bit RGB: {path}")
    if cursor >= len(data) or data[cursor] not in b" \t\r\n":
        raise ValueError(f"selector PPM has no header delimiter: {path}")
    if data[cursor : cursor + 2] == b"\r\n":
        cursor += 2
    else:
        cursor += 1
    pixels = data[cursor:]
    if len(pixels) != width * height * 3:
        raise ValueError(f"selector PPM payload is truncated: {path}")

    x, y, crop_width, crop_height = crop
    if x < 0 or y < 0 or x + crop_width > width or y + crop_height > height:
        raise ValueError(f"selector crop is outside {width}x{height}: {path}")
    rgb = bytearray()
    for output_y in range(y, y + crop_height):
        row = (output_y * width + x) * 3
        rgb.extend(pixels[row : row + crop_width * 3])
    return f"P6\n{crop_width} {crop_height}\n255\n".encode("ascii") + rgb


def crop_reference_to_ppm(
    path: Path, crop: tuple[int, int, int, int]
) -> bytes:
    data = path.read_bytes()[:2]
    if data == b"BM":
        return crop_bmp_to_ppm(path, crop)
    if data == b"P6":
        return crop_ppm_to_ppm(path, crop)
    raise ValueError(f"unsupported selector reference: {path}")


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
    for source_index, stable_id, display_name, _vehicle_name in VEHICLES:
        source_values = v8_stats.record(source_index).values()
        source_values.pop("vehicle_type")

        body = conversion.v8_bank_to_v82(
            decode_bank(V8_COMMON, "V8", source_index)
        )
        body = conversion.add_v82_flamethrower_mount(body, 0)
        selector_preview = conversion.v8_bank_to_v82(
            decode_bank(V8_SELECTOR_VEHICLES, "V8", source_index)
        )

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
            selector_preview_bank=selector_preview,
            selector_preview_body_kind=0,
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
    expected_banners = {
        f"SELECTOR_{index:02}.PPM"
        for index in range(len(SELECTOR_REFERENCES))
    }
    for stale in selector_output.glob("SELECTOR_*.PPM"):
        if stale.name not in expected_banners:
            stale.unlink()
    (pre_blender / "CUSTOM.EXP").write_bytes(package.archive)
    (pre_blender / "VEHICLES.V8R").write_bytes(package.registry)
    selector_assets = []
    for index, reference in enumerate(SELECTOR_REFERENCES):
        banner = crop_reference_to_ppm(reference, SELECTOR_CROP)
        name = f"SELECTOR_{index:02}.PPM"
        (selector_output / name).write_bytes(banner)
        selector_assets.append(
            {
                "name": f"SHELL/{name}",
                "source": str(reference.relative_to(ROOT)),
                "source_sha256": digest(reference.read_bytes()),
                "crop": {
                    "x": SELECTOR_CROP[0],
                    "y": SELECTOR_CROP[1],
                    "width": SELECTOR_CROP[2],
                    "height": SELECTOR_CROP[3],
                },
                "sha256": digest(banner),
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
        "native_selector_banners": selector_assets,
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
