from __future__ import annotations

from pathlib import Path
import sys
import unittest


TESTS = Path(__file__).resolve().parent
ADDONS = TESTS.parent
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

from vigilante8_vehicle_tools import conversion, project  # noqa: E402


def texture(index: int, *, coverage: bool) -> project.Texture:
    palette = (0, 0x8001) if coverage else (0x8001, 0x8421)
    indices = bytes((0, 1, 0, 1)) if coverage else bytes((0, 1, 1, 0))
    return project.Texture(
        name=f"texture_{index:03d}",
        width=2,
        height=2,
        depth=0,
        palette_bgr555=palette,
        indices=indices,
    )


class V8AnimatedSurfaceConversionTests(unittest.TestCase):
    def make_bank(self) -> project.ObjectBank:
        base = project.Face(
            vertices=(0, 1, 2),
            color=(128, 128, 128),
            texture=0,
            native_texture_slot=0,
            packet_kind=15,
            packet_flags=0x10,
        )
        coverage = project.Face(
            vertices=(0, 1, 2),
            color=(128, 128, 128),
            texture=2,
            native_texture_slot=2,
            packet_kind=15,
            packet_flags=0x10,
        )
        return project.ObjectBank(
            groups=(project.RenderGroup(
                name="group_000",
                scale_shift=8,
                vertices=((-1024, 0, -1024), (1024, 0, -1024), (0, 0, 1024)),
                faces=(base, coverage),
                texture_slot_count=4,
                render_extent=1449,
            ),),
            slots=(project.Slot(
                name="slot_000",
                render_group=0,
                collision=None,
            ),),
            collisions=(),
            textures=(
                texture(0, coverage=False),
                texture(1, coverage=False),
                texture(2, coverage=True),
                texture(3, coverage=True),
            ),
            animations=(project.SlotAnimation(
                slot=0,
                loop=True,
                frames=(project.AnimationFrame(
                    frame_delta=0,
                    texture_bindings=(
                        project.TextureBinding(target=0, texture=0),
                        project.TextureBinding(target=2, texture=2),
                    ),
                ),),
            ),),
        )

    def test_structural_detector_and_native_conversion(self) -> None:
        source = self.make_bank()
        self.assertTrue(conversion.is_v8_alpha_coverage_surface(source))
        converted = conversion.v8_alpha_coverage_surface_to_v82(source)
        group = converted.groups[0]
        self.assertEqual(source.textures, converted.textures)
        self.assertEqual(2, len(group.faces))
        self.assertEqual(4, group.texture_slot_count)
        self.assertEqual(7, group.scale_shift)
        self.assertEqual(724, group.render_extent)
        self.assertEqual(
            ((-512, 0, -512), (512, 0, -512), (0, 0, 512)),
            group.vertices,
        )
        self.assertTrue(all(face.color == (38, 38, 38) for face in group.faces))
        self.assertTrue(all(face.texture_flags == 0x4000 for face in group.faces))
        self.assertTrue(
            all(face.material_parameter == 0xBFF7 for face in group.faces)
        )
        self.assertEqual(source.animations, converted.animations)
        self.assertEqual(
            (
                project.TextureBinding(target=0, texture=0),
                project.TextureBinding(target=2, texture=2),
            ),
            converted.animations[0].frames[0].texture_bindings,
        )

    def test_near_match_is_not_rewritten(self) -> None:
        source = self.make_bank()
        broken_texture = project.Texture(
            name="texture_002",
            width=2,
            height=2,
            depth=0,
            palette_bgr555=(0x8001, 0x8421),
            indices=bytes((0, 1, 0, 1)),
        )
        near_match = project.ObjectBank(
            groups=source.groups,
            slots=source.slots,
            collisions=source.collisions,
            textures=source.textures[:2] + (broken_texture,) + source.textures[3:],
            animations=source.animations,
        )
        self.assertFalse(conversion.is_v8_alpha_coverage_surface(near_match))
        self.assertIs(
            near_match,
            conversion.v8_alpha_coverage_surface_to_v82(near_match),
        )


if __name__ == "__main__":
    unittest.main()
