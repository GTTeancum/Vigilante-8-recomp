#!/usr/bin/env python3
"""Regression tests for the reusable Bayou-native water conversion."""

from __future__ import annotations

import importlib.util
from pathlib import Path
import struct
import sys
import unittest


MODULE_PATH = Path(__file__).with_name("v82_native_water_conversion.py")
SPEC = importlib.util.spec_from_file_location("v82_native_water_conversion", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
WATER = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = WATER
SPEC.loader.exec_module(WATER)


class NativeWaterConversionTests(unittest.TestCase):
    def test_rectangle_becomes_inclusive_imported_drowning_region(self) -> None:
        source = struct.pack(">7h", 852, 1324, 913, 1384, 0, 0x43, -1)
        self.assertEqual(
            struct.unpack(">7h", WATER.convert_water_rectangle(source)),
            (852, 1324, 912, 1383, 0, 0x43, -1),
        )

    def test_nonwater_rectangle_is_unchanged(self) -> None:
        source = struct.pack(">7h", 1, 2, 3, 4, 0, 0x42, -1)
        self.assertEqual(WATER.convert_water_rectangle(source), source)

    def test_xwat_is_exact_two_by_nearest_neighbor(self) -> None:
        source_doc = WATER.iff.parse(
            (WATER.REPO / "PS1 game" / "TERRAIN" / "DREAMLND.EXP").read_bytes()
        )
        source = WATER.find_native_water_source(source_doc)
        template_doc = WATER.iff.parse(
            (WATER.REPO / "V8_2_LOOSE" / "LEVELS" / "BAYOU.EXP").read_bytes()
        )
        payload = WATER.encode_native_xwat(
            source.bank.textures[0], WATER.xwat_payload(template_doc)
        )
        self.assertEqual(len(payload), 2112)
        self.assertEqual(struct.unpack_from("<II", payload), (0x10, 0x08))
        packed = payload[64:]
        indices = bytes(
            nibble
            for value in packed
            for nibble in (value & 0xF, value >> 4)
        )
        authored = source.bank.textures[0].indices
        for y in range(64):
            for x in range(64):
                self.assertEqual(indices[y * 64 + x], authored[(y // 2) * 32 + x // 2])

    def test_source_plane_produces_native_initializer_argument(self) -> None:
        source_doc = WATER.iff.parse(
            (WATER.REPO / "PS1 game" / "TERRAIN" / "DREAMLND.EXP").read_bytes()
        )
        source = WATER.find_native_water_source(source_doc)
        # Original-V8 HEAD transforms carry a serialized 0x100000 Y bias.
        self.assertEqual(source.plane_y, 0x002D5400)
        self.assertEqual(WATER.native_water_init_argument(source.plane_y), 0x0002A400)


if __name__ == "__main__":
    unittest.main()
