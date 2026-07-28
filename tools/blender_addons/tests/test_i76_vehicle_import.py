#!/usr/bin/env python3
"""Regression checks for the Interstate '76 Piranha import path."""

from __future__ import annotations

import hashlib
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools"))

from i76_vehicle_import import (  # noqa: E402
    AssetStore,
    decode_vqm,
    load_geo_pack,
    normalized_name,
    parse_geometry_chunk,
    parse_hardpoints,
    parse_vgeo_bank,
    parse_vehicle_config,
    parse_vehicle_name,
    parse_vehicle_weapons,
    parse_wheel_locations,
    resolve_material_vqm,
)


class I76PiranhaImportTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.assets = ROOT / "I76_ZFS"
        cls.store = AssetStore(cls.assets)

    def test_intact_lod_geometry_is_complete(self) -> None:
        placements = parse_vgeo_bank(self.assets / "vppirnha.vdf", 0)
        geos = load_geo_pack(self.store, "vppirnag")
        selected = [geos[normalized_name(item.name)] for item in placements]
        self.assertEqual(14, len(placements))
        self.assertEqual(361, sum(len(geo.vertices) for geo in selected))
        self.assertEqual(277, sum(len(geo.faces) for geo in selected))

    def test_material_resolution_covers_rendered_faces(self) -> None:
        placements = parse_vgeo_bank(self.assets / "vppirnha.vdf", 0)
        geos = load_geo_pack(self.store, "vppirnag")
        resolved = 0
        unresolved: list[str] = []
        for placement in placements:
            geo = geos[normalized_name(placement.name)]
            for face in geo.faces:
                if not face.material:
                    continue
                if resolve_material_vqm(self.store, face.material, "PP", 1):
                    resolved += 1
                else:
                    unresolved.append(face.material)
        self.assertEqual(263, resolved)
        self.assertEqual(["ZRETC_3", "ZRETC_3"], unresolved)

    def test_orange_piranha_texture_decodes_exactly(self) -> None:
        payload = self.store.named_payload("pp11ftf1.vqm")
        self.assertIsNotNone(payload)
        width, height, rgba, codebook = decode_vqm(self.assets, payload)
        self.assertEqual((180, 71), (width, height))
        self.assertEqual("ORNGEVCL.CBK", codebook)
        self.assertEqual(
            "317DCB8D0B825300B3219A5FCE20A147785D147D0BBEDC98132AF135C4F030A7",
            hashlib.sha256(rgba).hexdigest().upper(),
        )

    def test_stock_wheels_and_weapon_hardpoints_are_recovered(self) -> None:
        vdf = self.assets / "vppirnha.vdf"
        wheels = parse_wheel_locations(vdf)
        self.assertEqual(4, len(wheels))
        self.assertEqual(
            [
                (0.739, 0.328, 1.360),
                (-0.739, 0.328, 1.360),
                (0.739, 0.328, -1.298),
                (-0.739, 0.328, -1.298),
            ],
            [
                tuple(round(value, 3) for value in item.matrix[9:12])
                for item in wheels
            ],
        )
        wheel_geometry = parse_geometry_chunk(
            self.assets / "wauto_1a.wdf", b"WGEO"
        )
        self.assertEqual(16, len(wheel_geometry))
        self.assertIn("WA11RWL1", {item.name for item in wheel_geometry})
        self.assertIn("WA11LWL1", {item.name for item in wheel_geometry})

        hardpoints = parse_hardpoints(vdf)
        self.assertEqual(
            ["PP1_GDB1", "PP1_GIB1", "PP1_GPF1", "PP1_GPF2"],
            [item.name for item in hardpoints],
        )
        self.assertEqual(
            [
                "goilslck.gdf",
                "gfmedium.gdf",
                "gmmedium.gdf",
                "gcmedium.gdf",
            ],
            parse_vehicle_weapons(self.assets / "vppirna1.vcf"),
        )

    def test_vehicle_config_and_display_name_are_recovered(self) -> None:
        config = parse_vehicle_config(self.assets / "vppirna1.vcf")
        self.assertEqual("Picard Piranha", parse_vehicle_name(self.assets / "vppirnha.vdf"))
        self.assertEqual("vppirnha.vdf", config.vehicle_definition)
        self.assertEqual("piranha1.vtf", config.appearance_definition)
        self.assertEqual(
            ("wauto_1a.wdf", None, "wauto_1a.wdf"),
            config.wheel_definitions,
        )
        self.assertEqual(
            (
                "goilslck.gdf",
                "gfmedium.gdf",
                "gmmedium.gdf",
                "gcmedium.gdf",
            ),
            config.weapons,
        )

    def test_cutout_faces_and_palette_key_are_preserved(self) -> None:
        target_faces = [
            load_geo_pack(self.store, "vppirnag")[
                normalized_name("PP11HLGT")
            ].faces[0],
            load_geo_pack(self.store, "wauto_1g")[
                normalized_name("WA11RWL1")
            ].faces[1],
            load_geo_pack(self.store, "gmmedimg")[
                normalized_name("GMMP11GN")
            ].faces[12],
        ]
        self.assertEqual(
            [("HL1", (4, 5, 0)), ("WA11LS01", (5, 5, 0)), ("GMLPM_", (5, 5, 0))],
            [(face.material, face.render_flags) for face in target_faces],
        )

        payload = self.store.named_payload("HL1.vqm")
        self.assertIsNotNone(payload)
        width, height, rgba, _codebook = decode_vqm(
            self.assets, payload, transparent_palette_index=255
        )
        alpha = rgba[3::4]
        self.assertEqual(0, alpha[0])
        self.assertEqual(0, alpha[(width - 1)])
        self.assertEqual(0, alpha[(height - 1) * width])
        self.assertEqual(0, alpha[-1])
        self.assertGreater(alpha.count(0), 0)
        self.assertGreater(alpha.count(255), 0)


if __name__ == "__main__":
    unittest.main()
