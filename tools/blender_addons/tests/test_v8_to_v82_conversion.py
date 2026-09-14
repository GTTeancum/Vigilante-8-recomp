from __future__ import annotations

import hashlib
import struct
from pathlib import Path
import sys
import unittest


TESTS = Path(__file__).resolve().parent
ADDONS = TESTS.parent
ROOT = TESTS.parents[2]
for path in (TESTS, ADDONS):
    if str(path) not in sys.path:
        sys.path.insert(0, str(path))

from build_v8_to_v82_guest_roster import (  # noqa: E402
    VEHICLES,
    V8_COMMON,
    V8_SELECTOR_VEHICLES,
    build_projects,
    decode_bank,
    decode_sounds,
)
from vigilante8_vehicle_tools import (  # noqa: E402
    conversion,
    iff,
    project,
    registry,
    stats,
    xobf,
)


EXPECTED_IDS = (
    "guest.v8.chassey_blue",
    "guest.v8.slick_clyde",
    "guest.v8.sheila",
    "guest.v8.john_torque",
    "guest.v8.dave",
    "guest.v8.convoy",
    "guest.v8.loki",
    "guest.v8.houston_3",
    "guest.v8.boogie",
    "guest.v8.beezwax",
    "guest.v8.molo",
    "guest.v8.sid_burn",
    "guest.v8.y_the_alien",
)


class V8ToV82GuestConversionTests(unittest.TestCase):
    def test_native_color_editor_can_copy_complete_group_block(self) -> None:
        package = registry.compile_package(self.vehicles)
        for form in iff.parse(package.archive).forms(b"XOBF"):
            data = next(child.payload for child in form.children if child.tag == b"BIN ")
            count, table = struct.unpack_from("<II", data)
            size = struct.unpack_from("<I", data, table + count * 4)[0]
            collision_table = struct.unpack_from("<I", data, 12)[0]
            self.assertEqual(size, collision_table - table)
            copied = data[table:table + size]
            self.assertEqual(len(copied), size)
            for index in range(count):
                relative = struct.unpack_from("<I", copied, index * 4)[0]
                self.assertGreaterEqual(relative, (count + 1) * 4)
                self.assertLess(relative + 0x1C, size + 1)

    def test_source_special_model_indices_and_geometry_are_preserved(self) -> None:
        for source_index, kinds in ((7,(58,60)),(12,(40,42))):
            source=conversion.v8_bank_to_v82(decode_bank(V8_COMMON,"V8",source_index))
            vehicle=self.vehicles[source_index]
            for kind in kinds:
                self.assertEqual(source.slots[kind],vehicle.slots[kind])
                self.assertEqual(source.groups[source.slots[kind].render_group],
                    vehicle.groups[vehicle.slots[kind].render_group])

    @classmethod
    def setUpClass(cls) -> None:
        cls.vehicles = build_projects()
        cls.source_banks = {
            stable_id: (
                decode_bank(V8_COMMON, "V8", source_index),
                decode_bank(V8_SELECTOR_VEHICLES, "V8", source_index),
            )
            for source_index, stable_id, _display_name, _vehicle_name in VEHICLES
        }

    def test_full_roster_owns_exclusive_native_banks(self) -> None:
        package = registry.compile_package(self.vehicles)
        game, entries = registry.parse_registry(package.registry)
        forms = tuple(iff.parse(package.archive).forms(b"XOBF"))

        self.assertEqual("V8_2", game)
        self.assertEqual(EXPECTED_IDS, tuple(v.stable_id for v in self.vehicles))
        self.assertEqual(
            sum(
                2 + (vehicle.transformation_bank is not None)
                for vehicle in self.vehicles
            ),
            len(forms),
        )
        expected_indices = []
        next_index = 0
        for vehicle in self.vehicles:
            body = next_index
            next_index += 1
            transform = (
                next_index
                if vehicle.transformation_bank is not None
                else None
            )
            if transform is not None:
                next_index += 1
            selector = next_index
            next_index += 1
            expected_indices.append((body, transform, selector))
        self.assertEqual(
            tuple(expected_indices),
            tuple(
                (
                    entry.archive_index,
                    entry.transformation_archive_index,
                    entry.selector_preview_archive_index,
                )
                for entry in entries
            ),
        )
        referenced = {
            index
            for entry in entries
            for index in (
                entry.archive_index,
                entry.transformation_archive_index,
                entry.selector_preview_archive_index,
            )
        }
        referenced.discard(None)
        self.assertEqual(set(range(len(forms))), referenced)
        behavior_types = {
            entry.stable_id: entry.special_behavior_type
            for entry in entries
        }
        self.assertTrue(all(value is None for value in behavior_types.values()))
        self.assertEqual({"guest.v8.houston_3":7,"guest.v8.y_the_alien":12},
            {e.stable_id:e.original_special_type for e in entries if e.original_special_type is not None})

        decoded = registry.decompile_package(package.archive, package.registry)
        rebuilt = registry.compile_package(decoded)
        self.assertEqual(package.archive, rebuilt.archive)
        self.assertEqual(package.registry, rebuilt.registry)

    def test_original_vehicle_sound_banks_are_converted_semantically(self) -> None:
        package = registry.compile_package(self.vehicles)
        _game, entries = registry.parse_registry(package.registry)
        forms = tuple(iff.parse(package.archive).forms(b"XOBF"))
        source_forms = tuple(iff.parse(V8_COMMON.read_bytes()).forms(b"XOBF"))
        for source_index, vehicle in enumerate(self.vehicles):
            expected = decode_sounds(V8_COMMON, source_index)
            self.assertEqual(expected, vehicle.sounds[:len(expected)], vehicle.stable_id)
            if vehicle.original_special_type is not None:
                common=registry._decode_sounds(iff.IffChunk(tag=b"FORM",form_type=b"XOBF",children=[
                    iff.IffChunk(tag=b"SND ",payload=(ROOT/"PS1 game/SOUNDS/MAIN.SND").read_bytes())]))
                self.assertEqual((common[0x41],common[0x2B]),vehicle.sounds[len(expected):])
                expected=vehicle.sounds
            body = forms[entries[source_index].archive_index]
            self.assertEqual(expected, registry._decode_sounds(body))
            if vehicle.original_special_type is not None:
                continue  # Extended table is checked sample-for-sample above.
            self.assertEqual(
                next(
                    child.payload
                    for child in source_forms[source_index].children
                    if child.tag == b"SND "
                ),
                next(
                    child.payload
                    for child in body.children
                    if child.tag == b"SND "
                ),
                vehicle.stable_id,
            )
            for support_index in (
                entries[source_index].transformation_archive_index,
                entries[source_index].selector_preview_archive_index,
            ):
                if support_index is not None:
                    self.assertEqual(
                        (), registry._decode_sounds(forms[support_index])
                    )

        y = self.vehicles[12]
        self.assertEqual(
            (743, 743, 1024),
            tuple(int(sound["pitch"]) for sound in y.sounds[:3]),
        )

    def test_v8_minus_two_arena_sentinel_preserves_native_class_f(self) -> None:
        source = project.ObjectBank(
            groups=(),
            slots=(
                project.Slot(
                    name="v8_minus_two",
                    render_group=None,
                    collision=None,
                    key=0xFFFE,
                ),
            ),
            collisions=(),
            textures=(),
            animations=(),
        )
        converted = conversion.v8_bank_to_v82(source)
        self.assertEqual(0xF7FF, converted.slots[0].key)
        self.assertIsNone(converted.slots[0].render_group)

    def test_v8_body_anchors_and_all_sequel_mounts_are_preserved(self) -> None:
        for vehicle in self.vehicles:
            direct_keys = {
                slot.key
                for slot in vehicle.slots
                if slot.parent == vehicle.body_kind and slot.key is not None
            }
            expected_wheels = set() if vehicle.controller_class == "flying" else {0x8000, 0x8001, 0x8002, 0x8003}
            self.assertEqual(
                expected_wheels,
                direct_keys & set(range(0x8000, 0x8006)),
                vehicle.stable_id,
            )
            self.assertEqual(
                set(range(0x8010, 0x8017)),
                direct_keys & set(range(0x8010, 0x8017)),
                vehicle.stable_id,
            )

    def test_y_retains_original_wheelless_construction(self) -> None:
        y = self.vehicles[12]
        self.assertIsNone(y.transformation_bank)
        for slots in (y.slots, y.selector_preview_bank.slots):
            self.assertFalse(any(slot.key in range(0x8000,0x8004)
                for slot in slots if slot.key is not None and slot.parent == 0))

    def test_clyde_suspension_travel_markers_use_native_v82_key(self) -> None:
        clyde = self.vehicles[1]
        for bank in (
            project.ObjectBank(
                groups=clyde.groups,
                slots=clyde.slots,
                collisions=clyde.collisions,
                textures=clyde.textures,
                animations=clyde.animations,
            ),
            clyde.selector_preview_bank,
        ):
            self.assertIsNotNone(bank)
            markers = []
            for index, anchor in enumerate(bank.slots):
                if (
                    anchor.parent != clyde.body_kind
                    or anchor.key not in range(0x8000, 0x8004)
                ):
                    continue
                children = [
                    slot
                    for slot in bank.slots
                    if slot.parent == index and slot.key == 0x8000
                ]
                self.assertEqual(1, len(children), anchor.key)
                markers.append(children[0].position[1])
            self.assertEqual([-3285, -3285, -3285, -3285], markers)

    def test_every_transformation_selector_is_an_owned_valid_root(self) -> None:
        for vehicle in self.vehicles:
            bank = vehicle.transformation_bank
            if not vehicle.supports_transformations:
                self.assertIsNone(bank)
                self.assertEqual((),vehicle.transform_modes)
                vehicle.validate()
                continue
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

    def test_animated_transform_material_slots_survive_conversion(self) -> None:
        """The terrain transform renderer indexes a native local slot table."""

        beezwax = next(
            vehicle
            for vehicle in self.vehicles
            if vehicle.stable_id == "guest.v8.beezwax"
        )
        source_groups = [
            group
            for group in beezwax.transformation_bank.groups
            if group.texture_slot_count == 2 and group.render_extent == 170
        ]
        self.assertEqual(1, len(source_groups))
        source = source_groups[0]
        self.assertEqual(
            {0, 1},
            {
                face.native_texture_slot
                for face in source.faces
                if face.packet_kind == 15
            },
        )

        package = registry.compile_package((beezwax,))
        _game, entries = registry.parse_registry(package.registry)
        form = tuple(iff.parse(package.archive).forms(b"XOBF"))[
            entries[0].transformation_archive_index
        ]
        model_data = next(
            child.payload for child in form.children if child.tag == b"BIN "
        )
        native = xobf.Model(model_data, dialect="V8_2")
        compiled_groups = [
            group
            for group in native.groups()
            if group.texture_slot_count == 2 and group.render_extent == 170
        ]
        self.assertEqual(1, len(compiled_groups))
        compiled = compiled_groups[0]
        self.assertEqual(
            {0, 1},
            {
                packet.texture_slot & 0x3FFF
                for packet in compiled.packets
                if packet.kind == 15
            },
        )

    @staticmethod
    def _retail_render_mode(native_packet_type: int) -> int:
        return (
            ((native_packet_type & 0x0F) << 2)
            | (0x40 if native_packet_type & 0x80 else 0)
            | (0x02 if native_packet_type & 0x10 else 0)
            | (0x80 if native_packet_type & 0x40 else 0)
        )

    def test_v8_environment_materials_preserve_authored_blend_role(
        self,
    ) -> None:
        for vehicle in self.vehicles:
            source_body, source_selector = self.source_banks[vehicle.stable_id]
            for label, source_bank, converted_bank in (
                ("body", source_body, vehicle),
                (
                    "selector",
                    source_selector,
                    vehicle.selector_preview_bank,
                ),
            ):
                self.assertIsNotNone(source_bank)
                self.assertIsNotNone(converted_bank)
                for source_group, converted_group in zip(
                    source_bank.groups, converted_bank.groups
                ):
                    source_faces = [
                        face
                        for face in source_group.faces
                        if face.packet_kind == 12
                    ]
                    converted_faces = [
                        face
                        for face in converted_group.faces
                        if face.packet_kind == 12
                    ]
                    if not source_faces:
                        continue
                    self.assertEqual(len(source_faces), len(converted_faces))
                    for source_face, converted_face in zip(
                        source_faces, converted_faces
                    ):
                        source_type = 12 | source_face.packet_flags
                        converted_type = 12 | converted_face.packet_flags
                        expected_mode = self._retail_render_mode(source_type)
                        self.assertEqual(
                            expected_mode,
                            self._retail_render_mode(converted_type),
                            f"{vehicle.stable_id}: {label}",
                        )
                        expected_flags = source_face.packet_flags & ~0x20
                        self.assertEqual(
                            converted_face.packet_flags,
                            expected_flags,
                            f"{vehicle.stable_id}: {label}",
                        )
                        source_environment = (
                            source_face.environment_parameters[0]
                        )
                        expected_environment = source_environment
                        if (source_environment & 0x3FFF) == 0x3FFF:
                            expected_environment = (
                                0x7FFE
                                if source_face.packet_flags & 0x10
                                else 0x3FFF
                            )
                        self.assertEqual(
                            converted_face.environment_parameters,
                            (
                                expected_environment,
                                0x8080,
                                0,
                                0,
                            ),
                            f"{vehicle.stable_id}: {label}",
                        )

    def test_beezwax_environment_faces_use_only_retail_v82_roles(
        self,
    ) -> None:
        source, _source_selector = self.source_banks["guest.v8.beezwax"]
        converted = next(
            vehicle
            for vehicle in self.vehicles
            if vehicle.stable_id == "guest.v8.beezwax"
        )
        source_opaque = [
            face
            for group in source.groups
            for face in group.faces
            if face.packet_kind == 12
            and not (face.packet_flags & 0x10)
        ]
        source_gloss = [
            face
            for group in source.groups
            for face in group.faces
            if face.packet_kind == 12 and face.packet_flags & 0x10
        ]
        converted_environment = [
            face
            for group in converted.groups
            for face in group.faces
            if face.packet_kind == 12
        ]
        self.assertEqual(18, len(source_opaque))
        self.assertEqual(8, len(source_gloss))
        self.assertEqual(26, len(converted_environment))
        self.assertEqual(
            {
                (0x00, (0x3FFF, 0x8080, 0, 0)),
                (0x10, (0x7FFE, 0x8080, 0, 0)),
            },
            {
                (face.packet_flags, face.environment_parameters)
                for face in converted_environment
            },
        )

    def test_compiled_beezwax_uses_stock_v82_environment_packets(
        self,
    ) -> None:
        package = registry.compile_package(self.vehicles)
        _game, entries = registry.parse_registry(package.registry)
        beezwax_index = EXPECTED_IDS.index("guest.v8.beezwax")
        archive_index = entries[beezwax_index].archive_index
        form = tuple(iff.parse(package.archive).forms(b"XOBF"))[archive_index]
        model_data = next(
            child.payload for child in form.children if child.tag == b"BIN "
        )
        model = xobf.Model(model_data, dialect="V8_2")
        packets = [
            (
                packet.raw[3],
                tuple(
                    int.from_bytes(packet.raw[offset : offset + 2], "little")
                    for offset in range(0x10, 0x18, 2)
                ),
            )
            for group in model.groups()
            for packet in group.packets
            if packet.kind == 12
        ]
        self.assertEqual(
            {
                (0x0C, (0x3FFF, 0x8080, 0, 0)),
                (0x1C, (0x7FFE, 0x8080, 0, 0)),
            },
            set(packets),
        )

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

    def test_compatible_v8_menu_stats_are_reused_exactly(self) -> None:
        source = stats.StatsFile(
            (ROOT / "PS1 game" / "SLUS_005.10").read_bytes(),
            "V8",
        )
        shared = (
            "rating_armor",
            "rating_speed",
            "rating_handling",
        )
        for index, vehicle in enumerate(self.vehicles):
            source_values = source.record(index).values()
            for field in shared:
                self.assertEqual(
                    source_values[field] * 10,
                    vehicle.stats[field],
                    f"{vehicle.stable_id}: {field}",
                )

            # Retail V8 leaves this byte zero and its selector presents only
            # three categories. V8:2 requires a fourth row, so this one value
            # is explicitly synthesized instead of being mislabeled as
            # original menu data.
            self.assertEqual(0, source_values["rating_special"])
            expected_special = (
                sum(source_values[field] for field in shared) * 10 + 1
            ) // 3
            self.assertEqual(
                expected_special,
                vehicle.stats["rating_special"],
                f"{vehicle.stable_id}: sequel-only special",
            )


if __name__ == "__main__":
    unittest.main()
