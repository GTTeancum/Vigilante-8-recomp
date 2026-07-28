"""Build a recognizable from-scratch native V8:2 vehicle proof package."""

from __future__ import annotations

import argparse
from dataclasses import replace
import math
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[3]
ADDONS = ROOT / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

from vigilante8_vehicle_tools import authored_scene, project, registry


class Mesh:
    def __init__(self) -> None:
        self.vertices: list[tuple[int, int, int]] = []
        self.faces: list[project.Face] = []

    def add_vertices(self, values) -> tuple[int, ...]:
        indices = tuple(range(len(self.vertices), len(self.vertices) + len(values)))
        self.vertices.extend(tuple(map(int, value)) for value in values)
        return indices

    def triangle(
        self,
        indices: tuple[int, int, int],
        color: tuple[int, int, int],
        texture: int | None = None,
        uv=((0, 0), (31, 0), (31, 31)),
    ) -> None:
        self.faces.append(project.Face(indices, color, texture, uv))

    def quad(
        self,
        indices: tuple[int, int, int, int],
        color: tuple[int, int, int],
        texture: int | None = None,
    ) -> None:
        a, b, c, d = indices
        self.triangle((a, b, c), color, texture, ((0, 0), (31, 0), (31, 31)))
        self.triangle((a, c, d), color, texture, ((0, 0), (31, 31), (0, 31)))

    def box(
        self,
        minimum: tuple[int, int, int],
        maximum: tuple[int, int, int],
        color: tuple[int, int, int],
        texture: int | None = None,
    ) -> None:
        x0, y0, z0 = minimum
        x1, y1, z1 = maximum
        v = self.add_vertices(
            (
                (x0, y0, z0),
                (x1, y0, z0),
                (x1, y1, z0),
                (x0, y1, z0),
                (x0, y0, z1),
                (x1, y0, z1),
                (x1, y1, z1),
                (x0, y1, z1),
            )
        )
        self.quad((v[0], v[1], v[2], v[3]), color, texture)
        self.quad((v[4], v[7], v[6], v[5]), color, texture)
        self.quad((v[0], v[4], v[5], v[1]), color, texture)
        self.quad((v[3], v[2], v[6], v[7]), color, texture)
        self.quad((v[1], v[5], v[6], v[2]), color, texture)
        self.quad((v[0], v[3], v[7], v[4]), color, texture)

    def cabin(self) -> None:
        lower = ((-92, -62, -85), (92, -62, -85), (92, -62, 105), (-92, -62, 105))
        upper = ((-70, -132, -48), (70, -132, -48), (70, -132, 64), (-70, -132, 64))
        v = self.add_vertices(lower + upper)
        glass = (28, 62, 88)
        red = (176, 24, 22)
        self.quad((v[0], v[1], v[5], v[4]), glass)
        self.quad((v[3], v[7], v[6], v[2]), glass)
        self.quad((v[1], v[2], v[6], v[5]), glass)
        self.quad((v[0], v[4], v[7], v[3]), glass)
        self.quad((v[4], v[5], v[6], v[7]), red, 0)

    def wheel(self, center: tuple[int, int, int]) -> None:
        cx, cy, cz = center
        segments = 10
        half_width = 22
        rings = []
        for x in (cx - half_width, cx + half_width):
            rings.append(
                self.add_vertices(
                    tuple(
                        (
                            x,
                            round(cy + math.cos(index * math.tau / segments) * 45),
                            round(cz + math.sin(index * math.tau / segments) * 45),
                        )
                        for index in range(segments)
                    )
                )
            )
        tire = (18, 18, 20)
        hub = (116, 120, 126)
        for index in range(segments):
            nxt = (index + 1) % segments
            self.quad(
                (
                    rings[0][index],
                    rings[0][nxt],
                    rings[1][nxt],
                    rings[1][index],
                ),
                tire,
            )
        left_center, right_center = self.add_vertices(
            ((cx - half_width, cy, cz), (cx + half_width, cy, cz))
        )
        for index in range(segments):
            nxt = (index + 1) % segments
            self.triangle((left_center, rings[0][nxt], rings[0][index]), hub)
            self.triangle((right_center, rings[1][index], rings[1][nxt]), hub)


def rgb555(red: int, green: int, blue: int) -> int:
    return (red >> 3) | ((green >> 3) << 5) | ((blue >> 3) << 10)


def build() -> project.VehicleProject:
    mesh = Mesh()
    mesh.box((-116, -58, -205), (116, 30, 205), (164, 20, 20), 0)
    mesh.box((-104, -76, 82), (104, -58, 214), (192, 28, 24), 0)
    mesh.box((-104, -70, -214), (104, -55, -108), (132, 12, 18), 0)
    mesh.cabin()
    mesh.box((-96, -30, 205), (-34, 4, 218), (244, 226, 124))
    mesh.box((34, -30, 205), (96, 4, 218), (244, 226, 124))
    mesh.box((-108, -34, -220), (-55, 2, -207), (192, 12, 12))
    mesh.box((55, -34, -220), (108, 2, -207), (192, 12, 12))
    mesh.box((-126, 18, 198), (126, 34, 218), (88, 92, 100))
    mesh.box((-126, 18, -218), (126, 34, -198), (88, 92, 100))
    for x in (-122, 122):
        for z in (-132, 132):
            mesh.wheel((x, 20, z))

    palette = (
        rgb555(124, 8, 12),
        rgb555(180, 18, 20),
        rgb555(228, 44, 32),
        rgb555(248, 106, 54),
    ) + (rgb555(32, 32, 36),) * 12
    texture = project.Texture(
        name="red_racing_stripe",
        width=32,
        height=32,
        depth=0,
        palette_bgr555=palette,
        indices=bytes(
            3 if abs(column - 15) < 3 else 1 + ((row // 8) & 1)
            for row in range(32)
            for column in range(32)
        ),
        compressed=True,
    )
    base = authored_scene.new_project(
        "V8_2", "proof.original_lowpoly_coupe"
    )
    vehicle = replace(
        base,
        display_name="Original Low-Poly Coupe",
        groups=(
            project.RenderGroup(
                name="complete_coupe_body",
                scale_shift=8,
                vertices=tuple(mesh.vertices),
                faces=tuple(mesh.faces),
            ),
        ),
        textures=(texture,),
    )
    vehicle.validate()
    return vehicle


def main() -> None:
    arguments = sys.argv
    arguments = (
        arguments[arguments.index("--") + 1 :] if "--" in arguments else []
    )
    parser = argparse.ArgumentParser()
    parser.add_argument("output", type=Path)
    output = parser.parse_args(arguments).output.resolve()
    package = registry.compile_package((build(),))
    output.mkdir(parents=True, exist_ok=True)
    (output / "CUSTOM.EXP").write_bytes(package.archive)
    (output / "VEHICLES.V8R").write_bytes(package.registry)
    print(
        f"ORIGINAL CAR PACKAGE BUILT {output} "
        f"archive={len(package.archive)} registry={len(package.registry)}"
    )


if __name__ == "__main__":
    main()
