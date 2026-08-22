from __future__ import annotations

from dataclasses import replace
from pathlib import Path
import sys
import struct
import unittest


TOOLS = Path(__file__).resolve().parent
if str(TOOLS) not in sys.path:
    sys.path.insert(0, str(TOOLS))

import v82_arena_registry as registry  # noqa: E402


class V82ArenaRegistryTests(unittest.TestCase):
    def setUp(self) -> None:
        self.entry = registry.ArenaEntry(
            stable_id="test.arena",
            name="Aircraft Graveyard",
            subtitle="Arizona/Nevada",
            path="LEVELS\\TEST\\ARENA.EXP",
            marker_x=100,
            marker_y=120,
            preview_index=18,
        )

    def test_native_selector_field_limits_round_trip(self) -> None:
        data = registry.build_registry([self.entry])
        self.assertEqual([self.entry], registry.parse_registry(data))
        self.assertEqual(18, len(self.entry.name))
        self.assertEqual(14, len(self.entry.subtitle))

    def test_overlong_display_name_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "18-character"):
            registry.build_registry([replace(self.entry, name="N" * 19)])

    def test_full_18_character_subtitle_is_accepted(self) -> None:
        data = registry.build_registry([
            replace(self.entry, subtitle="Super Dreamland 64")
        ])
        self.assertEqual(
            "Super Dreamland 64", registry.parse_registry(data)[0].subtitle
        )

    def test_overlong_subtitle_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "18-character"):
            registry.build_registry([replace(self.entry, subtitle="S" * 19)])

    def test_non_ascii_native_fields_are_rejected(self) -> None:
        for field, value in (
            ("name", "México"),
            ("subtitle", "México"),
            ("path", "LEVELS\\MÉXICO\\ARENA.EXP"),
        ):
            with self.subTest(field=field):
                with self.assertRaisesRegex(ValueError, "not ASCII"):
                    registry.build_registry([
                        replace(self.entry, **{field: value})
                    ])

    @staticmethod
    def dll(primary: str) -> bytes:
        name = primary.encode("ascii") + b"\0"
        image = bytearray(0x80)
        struct.pack_into("<II", image, 0, 0x80, 0x10)
        struct.pack_into("<II", image, 0x10, 0x20, 0x40)
        image[0x20:0x20 + len(name)] = name
        return bytes(image)

    def test_primary_export_matches_path_stem_exactly(self) -> None:
        entry = replace(self.entry, path="Levels\\Test\\Arena.exp")
        self.assertEqual(
            "Arena", registry.validate_primary_export(entry, self.dll("Arena"))
        )

    def test_primary_export_case_mismatch_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "does not exactly match"):
            registry.validate_primary_export(
                self.entry, self.dll("Arena")
            )


if __name__ == "__main__":
    unittest.main()
