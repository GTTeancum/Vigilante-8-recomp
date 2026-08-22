#!/usr/bin/env python3
"""Regression tests for generic N64 rooted-prop terrain conversion."""

from __future__ import annotations

from pathlib import Path
import sys
import unittest


REPO = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO / "tools"))
sys.path.insert(0, str(REPO / "tools" / "blender_addons"))

import analyze_rooted_prop_terrain_intersections as intersections  # noqa: E402
import v8_n64_level as n64  # noqa: E402
from vigilante8_vehicle_tools import iff  # noqa: E402


ROM = REPO / "Vigilante 8 (U) (!).n64"
RUNTIME = REPO / "PS1 game" / "SLUS_005.10"
BASELINE = REPO / "PS1 game" / "TERRAIN" / "DREAMLND.EXP"


def _callbacks(rom: n64.V8N64Rom) -> frozenset[str]:
    return frozenset(
        item.name
        for item in (
            *n64.overlay_exports(rom.decoded("DREAMLND.DLL")),
            *n64.psx_executable_exports(RUNTIME.read_bytes()),
        )
    )


def _head_bank_slots(exp: bytes) -> tuple[tuple[int, int], ...]:
    result = []
    for _offset, tag, payload, parent in n64.iter_chunks(exp):
        if tag == b"HEAD" and parent == b"OBJ " and len(payload) >= 30:
            result.append(
                (
                    n64.be16(payload, 26, signed=True),
                    n64.be16(payload, 28, signed=True),
                )
            )
    return tuple(result)


@unittest.skipUnless(ROM.is_file() and RUNTIME.is_file(), "retail inputs unavailable")
class RootedPropConversionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.rom = n64.V8N64Rom(ROM)
        cls.source = cls.rom.decoded("DREAMLND.EXP")
        cls.baseline, cls.baseline_report = n64.convert_arena(
            cls.source, "DREAMLND"
        )
        cls.converted, cls.report = n64.convert_arena(
            cls.source,
            "DREAMLND",
            object_callback_names=_callbacks(cls.rom),
        )

    def test_export_tables_supply_arena_and_resident_callbacks(self) -> None:
        arena = {item.name for item in n64.overlay_exports(
            self.rom.decoded("DREAMLND.DLL")
        )}
        resident = {item.name for item in n64.psx_executable_exports(
            RUNTIME.read_bytes()
        )}
        self.assertIn("DreamLnd", arena)
        self.assertIn("Butterfly", arena)
        self.assertIn("PU_Health", resident)
        self.assertIn("I_Special", resident)
        self.assertNotIn("Carrot", arena | resident)
        self.assertNotIn("Cabbage", arena | resident)

    def test_opt_in_path_preserves_the_exact_prior_conversion(self) -> None:
        self.assertEqual(self.baseline, BASELINE.read_bytes())
        self.assertTrue(all(model.rooted_variants == 0 for model in self.baseline_report.xobf))

    def test_rooted_adaptation_stays_inside_native_v8_capacity(self) -> None:
        for before, after in zip(self.baseline_report.xobf, self.report.xobf):
            self.assertEqual(after.slots, before.slots)
            self.assertLessEqual(after.groups, 0x100)
        self.assertGreater(self.report.xobf[0].rooted_variants, 0)
        self.assertGreater(self.report.xobf[0].terrain_clipped_faces, 0)
        # The resident pickup bank is callback-owned and must remain byte-for-byte
        # structurally unchanged by the static rooted-prop pass.
        self.assertEqual(self.report.xobf[1], self.baseline_report.xobf[1])
        self.assertEqual(
            _head_bank_slots(self.converted), _head_bank_slots(self.baseline)
        )

    def test_rooted_props_have_no_terrain_crossing_faces(self) -> None:
        document = iff.parse(self.converted)
        models = intersections._models(document, "V8")
        zmap, zones = intersections._terrain_payloads(document)
        checked = 0
        for head in intersections._heads(document):
            if head.name not in {"Carrot", "Cabbage"}:
                continue
            counts, _groups, _minimum, _maximum = intersections._instance_counts(
                models[head.bank], head, zmap, zones, 256.0
            )
            self.assertEqual(counts["crossing"], 0, head.name)
            self.assertEqual(counts["below"], 0, head.name)
            checked += 1
        self.assertEqual(checked, 31)

    def test_no_identity_literal_drives_conversion_logic(self) -> None:
        source = Path(n64.__file__).read_text(encoding="utf-8")
        start = source.index("def convert_xobf_bin(")
        end = source.index("\ndef convert_aimp(", start)
        implementation = source[start:end]
        self.assertNotIn("Carrot", implementation)
        self.assertNotIn("Cabbage", implementation)
        self.assertNotIn("DREAMLND", implementation)


if __name__ == "__main__":
    unittest.main()
