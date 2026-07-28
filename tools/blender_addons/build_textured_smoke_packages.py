#!/usr/bin/env python3
"""Build reproducible, independently authored textured runtime fixtures."""

from __future__ import annotations

from dataclasses import replace
from pathlib import Path
import sys


ADDONS = Path(__file__).resolve().parent
ROOT = ADDONS.parents[1]
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

from vigilante8_vehicle_tools import authored_scene, project, registry


def textured_vehicle(game: str) -> project.VehicleProject:
    vehicle = authored_scene.new_project(game)
    stable_id = (
        "test.textured.v82" if game == "V8_2" else "test.textured.v8"
    )
    display_name = (
        "Textured Guest V8:2" if game == "V8_2" else "Textured Guest V8"
    )
    texture = project.Texture(
        name="independent_checker",
        width=64,
        height=64,
        depth=0,
        palette_bgr555=(
            0x0000,
            0x001F,
            0x03E0,
            0x7C00,
            0x03FF,
            0x7C1F,
            0x7FE0,
            0x7FFF,
            0x4210,
            0x2108,
            0x1084,
            0x6318,
            0x56B5,
            0x294A,
            0x14A5,
            0x2529,
        ),
        indices=bytes(
            ((x // 8) + (y // 8) * 3) & 0x0F
            for y in range(64)
            for x in range(64)
        ),
        compressed=game == "V8_2",
    )
    group = vehicle.groups[0]
    face = replace(
        group.faces[0],
        texture=0,
        uv=((0, 0), (63, 0), (31, 63)),
    )
    group = replace(group, faces=(face, *group.faces[1:]))
    return replace(
        vehicle,
        stable_id=stable_id,
        display_name=display_name,
        groups=(group, *vehicle.groups[1:]),
        textures=(texture,),
    )


def main() -> int:
    for game, directory in (
        ("V8", ROOT / "artifacts" / "v8_textured_guest"),
        ("V8_2", ROOT / "artifacts" / "v82_textured_guest"),
    ):
        package = registry.compile_package((textured_vehicle(game),))
        directory.mkdir(parents=True, exist_ok=True)
        (directory / "CUSTOM.EXP").write_bytes(package.archive)
        (directory / "VEHICLES.V8R").write_bytes(package.registry)
        print(
            f"{game}: {directory} archive={len(package.archive)} "
            f"registry={len(package.registry)}"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
