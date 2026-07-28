from __future__ import annotations

from pathlib import Path
import sys
import unittest


TESTS = Path(__file__).resolve().parent
ADDONS = TESTS.parent
ROOT = TESTS.parents[2]
for path in (TESTS, ADDONS):
    if str(path) not in sys.path:
        sys.path.insert(0, str(path))

from build_v8_to_v82_guest_roster import build_projects  # noqa: E402
from vigilante8_vehicle_tools import (  # noqa: E402
    iff,
    project,
    registry,
    stats,
)


EXPECTED_IDS = (
    "guest.v8.chassey_blue",
    "guest.v8.slick_clyde",
    "guest.v8.sheila",
)


class V8ToV82GuestConversionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.vehicles = build_projects()

    def test_three_entries_own_six_exclusive_native_banks(self) -> None:
        package = registry.compile_package(self.vehicles)
        game, entries = registry.parse_registry(package.registry)
        forms = tuple(iff.parse(package.archive).forms(b"XOBF"))

        self.assertEqual("V8_2", game)
        self.assertEqual(EXPECTED_IDS, tuple(v.stable_id for v in self.vehicles))
        self.assertEqual(6, len(forms))
        self.assertEqual(
            ((0, 1), (2, 3), (4, 5)),
            tuple(
                (entry.archive_index, entry.transformation_archive_index)
                for entry in entries
            ),
        )
        referenced = {
            index
            for entry in entries
            for index in (
                entry.archive_index,
                entry.transformation_archive_index,
            )
        }
        self.assertEqual(set(range(6)), referenced)

        decoded = registry.decompile_package(package.archive, package.registry)
        rebuilt = registry.compile_package(decoded)
        self.assertEqual(package.archive, rebuilt.archive)
        self.assertEqual(package.registry, rebuilt.registry)

    def test_v8_body_anchors_and_all_sequel_mounts_are_preserved(self) -> None:
        for vehicle in self.vehicles:
            direct_keys = {
                slot.key
                for slot in vehicle.slots
                if slot.parent == vehicle.body_kind and slot.key is not None
            }
            self.assertEqual(
                {0x8000, 0x8001, 0x8002, 0x8003},
                direct_keys & set(range(0x8000, 0x8006)),
                vehicle.stable_id,
            )
            self.assertEqual(
                set(range(0x8010, 0x8017)),
                direct_keys & set(range(0x8010, 0x8017)),
                vehicle.stable_id,
            )

    def test_every_transformation_selector_is_an_owned_valid_root(self) -> None:
        for vehicle in self.vehicles:
            bank = vehicle.transformation_bank
            self.assertIsNotNone(bank)
            roots = {
                index
                for index, slot in enumerate(bank.slots)
                if slot.parent is None
            }
            self.assertEqual((0, 0, 0, 0, 0, 0), vehicle.transform_modes[0])
            for mode in vehicle.transform_modes[1:]:
                self.assertEqual(6, len(mode))
                for root in mode:
                    self.assertIn(root, roots)
                    self.assertEqual(-21846, bank.slots[root].flags)
                    self.assertIsNotNone(bank.slots[root].collision)
            vehicle.validate()

    def test_shared_handling_fields_come_from_each_v8_record(self) -> None:
        source = stats.StatsFile(
            (ROOT / "PS1 game" / "SLUS_005.10").read_bytes(),
            "V8",
        )
        direct_fields = (
            "front_spring_stiffness",
            "front_suspension_damping",
            "rear_spring_stiffness",
            "rear_suspension_damping",
            "low_speed_steering_response",
            "steering_speed_falloff",
            "maximum_drive_force",
            "health",
            "speed_drag_coefficient",
            "pitch_angular_response",
            "yaw_angular_response",
            "roll_angular_response",
            "mass",
        )
        for index, vehicle in enumerate(self.vehicles):
            source_values = source.record(index).values()
            for field in direct_fields:
                target = (
                    "middle_spring_stiffness"
                    if field == "rear_spring_stiffness"
                    else (
                        "front_damping_rear_stiffness"
                        if field == "front_suspension_damping"
                        else (
                            "middle_suspension_damping"
                            if field == "rear_suspension_damping"
                            else field
                        )
                    )
                )
                self.assertEqual(
                    source_values[field],
                    vehicle.stats[target],
                    f"{vehicle.stable_id}: {field}",
                )


if __name__ == "__main__":
    unittest.main()
