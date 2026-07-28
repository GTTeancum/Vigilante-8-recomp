from __future__ import annotations

from pathlib import Path
import struct
import sys
import unittest


ADDONS = Path(__file__).resolve().parents[1]
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

from vigilante8_vehicle_tools import (
    archive,
    compiler,
    iff,
    project,
    registry,
    scene_codec,
    stats,
    xobf,
)


ROOT = Path(__file__).resolve().parents[3]


class IffRoundTripTests(unittest.TestCase):
    ASSETS = (
        ROOT / "PS1 game" / "COMMON.EXP",
        ROOT / "PS1 game" / "SHELL" / "VEHICLES.EXP",
        ROOT / "V8_2_WORK" / "disc" / "SHARED" / "COMMON.EXP",
        ROOT / "V8_2_WORK" / "disc" / "SHELL" / "VEHICLES.EXP",
        ROOT / "V8_2_WORK" / "disc" / "SHARED" / "HOTRODS.EXP",
    )

    def test_retail_exp_round_trips_byte_exactly(self) -> None:
        tested = 0
        for path in self.ASSETS:
            if not path.is_file():
                continue
            source = path.read_bytes()
            self.assertEqual(source, iff.parse(source).to_bytes(), str(path))
            tested += 1
        self.assertGreater(tested, 0, "no local retail vehicle assets were available")

    def test_archive_facade_commits_native_model_edits(self) -> None:
        for path in self.ASSETS:
            if not path.is_file():
                continue
            source = path.read_bytes()
            vehicles = archive.VehicleArchive(source)
            entry = vehicles.entry(0)
            model = entry.model()
            group = model.group(0)
            if not group.vertices:
                continue
            vertices = [
                (vertex.x, vertex.y, vertex.z, vertex.pad)
                for vertex in group.vertices
            ]
            vertices[0] = (
                vertices[0][0] + 1,
                vertices[0][1],
                vertices[0][2],
                vertices[0][3],
            )
            model.patch_group_vertices(0, vertices)
            entry.commit_model(model)
            rebuilt = archive.VehicleArchive(vehicles.data)
            self.assertEqual(
                vertices[0][0],
                rebuilt.entry(0).model().group(0).vertices[0].x,
            )
            self.assertEqual(
                source[: entry.bin_chunk.source_offset],
                vehicles.data[: entry.bin_chunk.source_offset],
            )
            return
        self.fail("no editable archive entry was found")


class SceneCodecTests(unittest.TestCase):
    def test_vertex_and_slot_coordinates_round_trip(self) -> None:
        native_vertex = xobf.Vertex(123, -456, 789, 7)
        blender_vertex = scene_codec.native_vertex_to_blender(native_vertex, 8)
        self.assertGreater(
            blender_vertex[2], 0.0,
            "negative native Y must be positive Blender Z (up)",
        )
        self.assertEqual(
            (123, -456, 789, 7),
            scene_codec.blender_vertex_to_native(blender_vertex, 8, 7),
        )
        native_position = (0x12345, -0x23456, 0x34567)
        self.assertGreater(
            scene_codec.native_position_to_blender(native_position)[2],
            0.0,
        )
        self.assertEqual(
            native_position,
            scene_codec.blender_position_to_native(
                scene_codec.native_position_to_blender(native_position)
            ),
        )
        native_rotation = (1024, -512, 256)
        self.assertEqual(
            native_rotation,
            scene_codec.blender_rotation_to_native(
                scene_codec.native_rotation_to_blender(native_rotation)
            ),
        )

    def test_embedded_source_and_triangulation(self) -> None:
        source = bytes(range(256))
        self.assertEqual(
            source,
            scene_codec.decode_source(scene_codec.encode_source(source)),
        )
        self.assertEqual(
            ((0, 1, 2), (0, 2, 3)),
            scene_codec.triangulate_faces(((0, 1, 2, 3),)),
        )


class NativeSlotIdentityTests(unittest.TestCase):
    @staticmethod
    def _slot(render_key: int) -> xobf.Slot:
        return xobf.Slot(
            index=0,
            offset=0,
            render_key=render_key,
            obstacle_index=-1,
            x=0,
            y=0,
            z=0,
            rot_y=0,
            rot_x=0,
            rot_z=0,
            flags=0,
            next_sibling=-1,
            first_child=-1,
        )

    def test_decoded_attachment_keys_are_not_render_groups(self) -> None:
        keys = (
            *range(0x8000, 0x8006),
            *range(0x8010, 0x8017),
            0x801F,
            *range(0x8040, 0x8044),
            *range(0x8100, 0x8103),
            0x8400,
            0x8500,
            0x8804,
            0x9109,
            0x9200,
        )
        for key in keys:
            with self.subTest(key=f"0x{key:04X}"):
                slot = self._slot(key)
                self.assertIsNone(slot.render_group)
                self.assertEqual(0, slot.render_flags)

    def test_high_bit_render_classes_keep_their_owned_group(self) -> None:
        for key, group, flags in (
            (0xB009, 0x009, 0xB000),
            (0xC000, 0, 0xC000),
            (0xC123, 0x123, 0xC000),
            (0xF001, 0x001, 0xF000),
        ):
            with self.subTest(key=f"0x{key:04X}"):
                slot = self._slot(key)
                self.assertEqual(group, slot.render_group)
                self.assertEqual(flags, slot.render_flags)

    def test_v8_uses_an_eight_bit_group_field(self) -> None:
        slot = self._slot(0xFF0A)
        object.__setattr__(slot, "group_mask", 0x00FF)
        self.assertEqual(0x0A, slot.render_group)
        self.assertEqual(0xFF00, slot.render_flags)


class HumanReadableStatsSchemaTests(unittest.TestCase):
    def test_every_native_byte_has_an_explicit_field(self) -> None:
        for game, profile in stats.PROFILES.items():
            covered = []
            for field in profile.fields:
                covered.extend(range(field.offset, field.offset + field.size))
            with self.subTest(game=game):
                self.assertEqual(list(range(profile.record_size)), sorted(covered))

    def test_every_editable_field_is_self_explanatory(self) -> None:
        for game, profile in stats.PROFILES.items():
            author_names = [
                field.author_name for field in profile.authoring_fields
            ]
            with self.subTest(game=game, check="unique author names"):
                self.assertEqual(len(author_names), len(set(author_names)))
            for field in profile.authoring_fields:
                if not field.editable:
                    continue
                with self.subTest(game=game, field=field.author_name):
                    self.assertNotIn("unresolved", field.author_name)
                    self.assertNotIn("parameter_", field.author_name)
                    self.assertGreaterEqual(len(field.description), 24)
                    self.assertTrue(field.unit)
                    self.assertTrue(field.lower_effect)
                    self.assertTrue(field.higher_effect)
                    self.assertLess(field.native_min, field.native_max)
                    self.assertLessEqual(
                        field.native_min, field.authoring_min
                    )
                    self.assertLessEqual(
                        field.authoring_min, field.authoring_max
                    )
                    self.assertLessEqual(
                        field.authoring_max, field.native_max
                    )
                    if field.recommended_min is not None:
                        self.assertLessEqual(
                            field.authoring_min, field.recommended_min
                        )
                        self.assertLessEqual(
                            field.recommended_min, field.recommended_max
                        )
                        self.assertLessEqual(
                            field.recommended_max, field.authoring_max
                        )

    def test_json_uses_author_names_and_accepts_legacy_names(self) -> None:
        for game in stats.PROFILES:
            vehicle = DonorFreeCompilerTests._project(game)
            encoded = project.to_dict(vehicle)
            self.assertEqual(
                set(encoded["stats"]),
                {
                    field.author_name
                    for field in stats.PROFILES[game].authoring_fields
                    if field.name != "vehicle_type"
                },
            )
            self.assertEqual(vehicle, project.VehicleProject.from_dict(encoded))

    def test_semantically_unsafe_negative_suspension_is_rejected(self) -> None:
        encoded = project.to_dict(DonorFreeCompilerTests._project("V8_2"))
        encoded["stats"]["first_axle_spring_stiffness"] = -1
        with self.assertRaisesRegex(ValueError, "authoring bound"):
            project.VehicleProject.from_dict(encoded)

    def test_powerup_fields_define_bounds_and_value_direction(self) -> None:
        for setting in stats.V82_POWERUP_SETTINGS:
            with self.subTest(field=setting.name):
                self.assertLess(setting.author_min, setting.author_max)
                self.assertTrue(setting.description)
                self.assertTrue(setting.unit)
                self.assertTrue(setting.lower_effect)
                self.assertTrue(setting.higher_effect)
                self.assertLessEqual(
                    setting.author_min, setting.retail_value
                )
                self.assertLessEqual(
                    setting.retail_value, setting.author_max
                )


class DonorFreeCompilerTests(unittest.TestCase):
    @staticmethod
    def _project(game: str) -> project.VehicleProject:
        profile = stats.PROFILES[game]
        stat_values = {
            field.name: field.default
            for field in profile.authoring_fields
            if field.name != "vehicle_type"
        }
        wheel_joint_count = 6 if game == "V8_2" else 4
        slots = [
            {
                "name": "root",
                "render_group": 0,
                "collision": 0,
            }
        ]
        wheel_names = (
            "front_left",
            "front_right",
            "middle_left",
            "middle_right",
            "rear_left",
            "rear_right",
        )
        wheel_positions = (
            (-49152, 0, 65536),
            (49152, 0, 65536),
            (-49152, 0, 0),
            (49152, 0, 0),
            (-49152, 0, -65536),
            (49152, 0, -65536),
        )
        if wheel_joint_count == 4:
            wheel_names = (
                "front_left",
                "front_right",
                "rear_left",
                "rear_right",
            )
            wheel_positions = (
                (-49152, 0, 65536),
                (49152, 0, 65536),
                (-49152, 0, -65536),
                (49152, 0, -65536),
            )
        for index, (name, position) in enumerate(
            zip(wheel_names, wheel_positions)
        ):
            slots.append(
                {
                    "name": f"{name}_wheel_joint",
                    "render_group": None,
                    "collision": None,
                    "key": 0x8000 + index,
                    "position": list(position),
                    "parent": 0,
                }
            )
        slots.append(
            {
                "name": "special_attachment",
                "render_group": None,
                "collision": None,
                "key": 0x8100,
                "parent": 0,
            }
        )
        normal_wheel_kind = len(slots)
        slots.append(
            {
                "name": "normal_wheel_object",
                "render_group": 0,
                "collision": None,
            }
        )
        for index in range(6 if game == "V8" else 7):
            slots.append(
                {
                    "name": f"weapon_mount_{index}",
                    "render_group": None,
                    "collision": None,
                    "key": 0x8010 + index,
                    "parent": 0,
                }
            )
        slots.append(
            {
                "name": "weapon_pickup_mount",
                "render_group": None,
                "collision": None,
                "key": 0x801F,
                "parent": 0,
            }
        )
        slots.append(
            {
                "name": "special_attachment_secondary",
                "render_group": None,
                "collision": None,
                "key": 0x8101,
                "parent": 0,
            }
        )
        stat_values["wheel_kind_front"] = normal_wheel_kind
        stat_values["wheel_kind_rear"] = normal_wheel_kind
        transformation = None
        transform_modes = []
        powerups = {}
        if game == "V8_2":
            stat_values["rear_suspension_damping"] = 137
            transformation = {
                "groups": [
                    {
                        "name": "transform_wheel",
                        "scale_shift": 8,
                        "vertices": [[-8, 0, -8], [8, 0, -8], [0, -16, 8]],
                        "faces": [{"vertices": [0, 1, 2]}],
                    }
                ],
                "slots": [
                    slot
                    for index in range(6)
                    for slot in (
                        {
                            "name": f"transform_wheel_{index}",
                            "render_group": 0,
                            "collision": 0,
                            "flags": -21846,
                        },
                        {
                            "name": f"transform_wheel_{index}_instance",
                            "render_group": 0,
                            "render_flags": 0xC000,
                            "collision": None,
                            "parent": index * 2,
                        },
                    )
                ],
                "collisions": [
                    {
                        "name": "transform_wheel_collision",
                        "shapes": [
                            {
                                "kind": "aabb",
                                "minimum": [-8192, -8192, -8192],
                                "maximum": [8192, 8192, 8192],
                                "override_y": -8192,
                            }
                        ],
                    }
                ],
                "textures": [],
                "animations": [],
            }
            transform_modes = [
                [0, 0, 0, 0, 0, 0],
                [0, 2, 4, 6, 8, 10],
                [0, 2, 4, 6, 8, 10],
                [0, 2, 4, 6, 8, 10],
            ]
            powerups = {
                "repair_amount": 500,
                "weapon_upgrade_duration": 900,
                "radar_jammer_duration": 900,
                "shield_duration": 900,
                "transformation_duration": 500,
            }
        return project.VehicleProject.from_dict(
            {
                "schema_version": project.SCHEMA_VERSION,
                "stable_id": f"test.original.{game.lower()}",
                "display_name": "Original Test Vehicle",
                "game": game,
                "groups": [
                    {
                        "name": "chassis",
                        "scale_shift": 8,
                        "vertices": [[-64, 0, -64], [64, 0, -64], [0, -32, 64]],
                        "faces": [
                            {
                                "vertices": [0, 1, 2],
                                "color": [100, 120, 140],
                                "texture": 0,
                                "uv": [[0, 0], [3, 0], [1, 3]],
                            }
                        ],
                    }
                ],
                "slots": slots,
                "collisions": [
                    {
                        "name": "chassis",
                        "shapes": [
                            {
                                "kind": "aabb",
                                "minimum": [-65536, -32768, -65536],
                                "maximum": [65536, 32768, 65536],
                                "override_y": -32768,
                            }
                        ],
                    }
                ],
                "textures": [
                    {
                        "name": "paint",
                        "width": 4,
                        "height": 4,
                        "depth": 0,
                        "palette_bgr555": [
                            0,
                            0x001F,
                            0x03E0,
                            0x7C00,
                        ]
                        + [0] * 12,
                        "indices": [0, 1, 2, 3] * 4,
                        "compressed": game == "V8_2",
                    }
                ],
                "animations": [
                    {
                        "slot": 0,
                        "loop": True,
                        "frames": [
                            {
                                "frame_delta": 1,
                                "rotation_yxz": [0, 0, 0],
                                "translation_absolute": [0, 0, 0],
                                "scale": [4096, 4096, 4096, 0],
                            }
                        ],
                    }
                ],
                "stats": stat_values,
                "body_kind": 0,
                "transformation_bank": transformation,
                "transform_modes": transform_modes,
                "powerups": powerups,
            }
        )

    def test_compiles_complete_native_vehicle_without_source_data(self) -> None:
        for game in ("V8", "V8_2"):
            authored = self._project(game)
            compiled = compiler.compile_archive((authored,))
            vehicles = archive.VehicleArchive(compiled)
            entry = vehicles.entry(0)
            model = entry.model(game)
            self.assertEqual(game, model.dialect)
            self.assertEqual(1, model.group_count)
            expected_slot_count = 18 if game == "V8_2" else 15
            self.assertEqual(expected_slot_count, model.slot_count)
            self.assertEqual(1, model.obstacle_count)
            self.assertEqual(1, model.texture_count)
            self.assertEqual((0, 1, 2), model.group(0).packets[0].vertex_indices)
            self.assertEqual(0, model.group(0).packets[0].texture_slot)
            self.assertEqual(bytes([0, 1, 2, 3] * 4), model.texture(0).indices)
            expected_keys = tuple(
                (0x8000 + index) - 0x10000
                for index in range(6 if game == "V8_2" else 4)
            ) + (-0x7F00,)
            joint_count = 6 if game == "V8_2" else 4
            self.assertEqual(
                expected_keys,
                tuple(
                    slot.render_key
                    for slot in (
                        model.slots()[1 : 1 + joint_count]
                        + (model.slots()[1 + joint_count],)
                    )
                ),
            )
            self.assertEqual(1, model.slots()[0].first_child)
            authored_slots = authored.slots
            child_indices = [
                index
                for index, slot in enumerate(authored_slots)
                if slot.parent == 0
            ]
            for position, slot_index in enumerate(child_indices):
                expected_next = (
                    child_indices[position + 1]
                    if position + 1 < len(child_indices)
                    else 0xFFFF
                )
                self.assertEqual(
                    expected_next,
                    model.slots()[slot_index].next_sibling,
                )
            collision = model.collision(0)
            self.assertEqual(1, len(collision.shapes))
            self.assertIsInstance(collision.shapes[0], xobf.CollisionAabb)
            animation = entry.animation(model.slot_count)
            self.assertIsNotNone(animation)
            self.assertGreaterEqual(len(animation.frames(0)), 2)
            self.assertEqual(compiled, vehicles.data)

    def test_schema_rejects_opaque_or_unknown_fields(self) -> None:
        value = {
            "schema_version": project.SCHEMA_VERSION,
            "stable_id": "test.invalid",
            "display_name": "Invalid",
            "game": "V8",
            "groups": [],
            "slots": [],
            "stats": {},
            "raw_blob": "forbidden",
        }
        with self.assertRaisesRegex(ValueError, "unsupported fields"):
            project.VehicleProject.from_dict(value)

    def test_native_memory_contract_rejects_oversized_textures(self) -> None:
        value = project.to_dict(self._project("V8"))
        value["textures"][0]["width"] = 257
        value["textures"][0]["indices"] = [0] * (257 * 4)
        with self.assertRaisesRegex(ValueError, "dimensions"):
            project.VehicleProject.from_dict(value)

        value = project.to_dict(self._project("V8"))
        texture = value["textures"][0]
        texture["width"] = 256
        texture["height"] = 256
        texture["indices"] = [0] * (256 * 256)
        value["textures"] = [dict(texture) for _ in range(5)]
        with self.assertRaisesRegex(ValueError, "texels|native texture"):
            project.VehicleProject.from_dict(value)

    def test_registry_enforces_one_byte_custom_identity_budget(self) -> None:
        authored = self._project("V8")
        with self.assertRaisesRegex(ValueError, "at most"):
            registry.compile_registry(
                (authored,) * (project.MAX_CUSTOM_VEHICLES + 1)
            )

    def test_append_registry_has_stable_identity_separate_from_archive_index(self) -> None:
        second = project.to_dict(self._project("V8"))
        second["stable_id"] = "test.original.second"
        second["display_name"] = "Second Original"
        projects = (
            self._project("V8"),
            project.VehicleProject.from_dict(second),
        )
        package = registry.compile_package(projects)
        game, entries = registry.parse_registry(package.registry)
        self.assertEqual("V8", game)
        self.assertEqual(2, len(entries))
        self.assertEqual("test.original.v8", entries[0].stable_id)
        self.assertEqual("test.original.second", entries[1].stable_id)
        self.assertEqual((0, 1), tuple(entry.archive_index for entry in entries))
        self.assertEqual(2, len(tuple(archive.VehicleArchive(package.archive).entries())))

    def test_v82_registry_owns_an_independent_transformation_bank(self) -> None:
        authored = self._project("V8_2")
        package = registry.compile_package((authored,))
        game, entries = registry.parse_registry(package.registry)
        self.assertEqual("V8_2", game)
        self.assertEqual(1, len(entries))
        self.assertEqual(0, entries[0].archive_index)
        self.assertEqual(1, entries[0].transformation_archive_index)
        self.assertEqual(authored.transform_modes, entries[0].transform_modes)
        self.assertEqual(dict(authored.powerups), entries[0].powerups)
        self.assertEqual(3, struct.unpack_from("<H", package.registry, 4)[0])
        self.assertEqual(40, struct.unpack_from("<H", package.registry, 10)[0])
        self.assertEqual(137, entries[0].rear_suspension_damping)
        self.assertEqual(
            137,
            struct.unpack_from(
                "<H",
                package.registry,
                registry.HEADER_SIZE + 36,
            )[0],
        )
        self.assertEqual(
            2, len(tuple(archive.VehicleArchive(package.archive).entries()))
        )

    def test_native_package_decompiles_without_source_or_passthrough(self) -> None:
        for game in ("V8", "V8_2"):
            authored = self._project(game)
            package = registry.compile_package((authored,))
            decoded = registry.decompile_package(
                package.archive, package.registry)
            self.assertEqual(1, len(decoded))
            self.assertEqual(authored.stable_id, decoded[0].stable_id)
            self.assertEqual(authored.display_name, decoded[0].display_name)
            self.assertEqual(authored.game, decoded[0].game)
            self.assertEqual(dict(authored.stats), dict(decoded[0].stats))
            self.assertEqual(
                authored.transform_modes, decoded[0].transform_modes)
            self.assertEqual(
                dict(authored.powerups), dict(decoded[0].powerups))
            rebuilt = registry.compile_package(decoded)
            self.assertEqual(package.archive, rebuilt.archive)
            self.assertEqual(package.registry, rebuilt.registry)

    def test_v8_preserves_native_clut_entries_beyond_addressable_nibbles(
        self,
    ) -> None:
        value = project.to_dict(self._project("V8"))
        texture = value["textures"][0]
        texture["palette_bgr555"] += [0x1927, 0xCDCD]
        authored = project.VehicleProject.from_dict(value)
        compiled = compiler.compile_archive((authored,))
        native = (
            archive.VehicleArchive(compiled)
            .entry(0)
            .model("V8")
            .texture(0)
        )
        self.assertEqual(18, len(native.palette))
        self.assertEqual((0x1927, 0xCDCD), native.palette[-2:])
        self.assertLessEqual(max(native.indices), 15)

        texture["indices"][0] = 16
        with self.assertRaisesRegex(ValueError, "palette index"):
            project.VehicleProject.from_dict(value)

    def test_v82_direct_color_texture_package_round_trip(self) -> None:
        value = project.to_dict(self._project("V8_2"))
        value["textures"][0] = {
            "name": "direct_bgr555",
            "width": 4,
            "height": 4,
            "depth": 2,
            "palette_bgr555": [],
            "indices": [],
            "compressed": True,
            "direct_pixels_bgr555": [
                0x8000 | (index & 31) | ((index & 31) << 5)
                for index in range(16)
            ],
        }
        authored = project.VehicleProject.from_dict(value)
        package = registry.compile_package((authored,))
        decoded = registry.decompile_package(
            package.archive, package.registry
        )[0]
        self.assertEqual(
            (
                authored.textures[0].width,
                authored.textures[0].height,
                authored.textures[0].depth,
                authored.textures[0].palette_bgr555,
                authored.textures[0].indices,
                authored.textures[0].compressed,
                authored.textures[0].direct_pixels_bgr555,
            ),
            (
                decoded.textures[0].width,
                decoded.textures[0].height,
                decoded.textures[0].depth,
                decoded.textures[0].palette_bgr555,
                decoded.textures[0].indices,
                decoded.textures[0].compressed,
                decoded.textures[0].direct_pixels_bgr555,
            ),
        )
        rebuilt = registry.compile_package((decoded,))
        self.assertEqual(package.archive, rebuilt.archive)
        self.assertEqual(package.registry, rebuilt.registry)


class XobfTests(unittest.TestCase):
    def _models(self):
        for path in IffRoundTripTests.ASSETS:
            if not path.is_file():
                continue
            document = iff.parse(path.read_bytes())
            for form in document.forms(b"XOBF"):
                bin_chunks = [child for child in form.children if child.tag == b"BIN "]
                if bin_chunks:
                    yield path, form, xobf.Model(bin_chunks[0].payload)

    def test_all_retail_models_parse(self) -> None:
        model_count = 0
        group_count = 0
        dialects = set()
        for path, form, model in self._models():
            groups = tuple(model.groups())
            slots = model.slots()
            textures = tuple(model.textures())
            collisions = tuple(model.collisions())
            self.assertEqual(model.group_count, len(groups), str(path))
            self.assertEqual(model.slot_count, len(slots), str(path))
            self.assertEqual(model.texture_count, len(textures), str(path))
            self.assertEqual(model.obstacle_count, len(collisions), str(path))
            model_count += 1
            group_count += len(groups)
            dialects.add(model.dialect)
        self.assertGreaterEqual(model_count, 14)
        self.assertGreater(group_count, 0)
        self.assertEqual({"V8", "V8_2"}, dialects)

    def test_surgical_vertex_patch_changes_only_one_component(self) -> None:
        for _path, _form, model in self._models():
            group = next((group for group in model.groups() if group.vertices), None)
            if group is None:
                continue
            before = model.data
            vertices = [
                (vertex.x, vertex.y, vertex.z, vertex.pad)
                for vertex in group.vertices
            ]
            original = vertices[0]
            replacement_x = original[0] + 1 if original[0] < 32767 else original[0] - 1
            vertices[0] = (replacement_x, original[1], original[2], original[3])
            model.patch_group_vertices(group.index, vertices)
            after = model.data
            changed = [index for index, pair in enumerate(zip(before, after)) if pair[0] != pair[1]]
            self.assertTrue(changed)
            self.assertTrue(all(group.vertex_offset <= index < group.vertex_offset + 2 for index in changed))
            return
        self.fail("no model with vertices was found")

    def test_topology_repack_preserves_other_native_sections(self) -> None:
        for _path, _form, model in self._models():
            if model.group_count == 0:
                continue
            slots_before = model.slots()
            textures_before = tuple(model.textures())
            model.replace_group_geometry(
                0,
                ((0, 0, 0), (256, 0, 0), (0, 256, 0)),
                ((0, 1, 2),),
                colors=((255, 64, 32),),
            )
            reparsed = xobf.Model(model.data, model.dialect)
            group = reparsed.group(0)
            self.assertEqual(3, group.vertex_count)
            self.assertEqual(1, group.polygon_count)
            self.assertEqual((0, 1, 2), group.packets[0].vertex_indices)
            self.assertEqual((255, 64, 32), group.packets[0].color)
            self.assertEqual(slots_before, reparsed.slots())
            textures_after = tuple(reparsed.textures())
            self.assertEqual(len(textures_before), len(textures_after))
            for before, after in zip(textures_before, textures_after):
                self.assertEqual(
                    (
                        before.depth,
                        before.width,
                        before.height,
                        before.palette,
                        before.indices,
                        before.packed_pixels,
                        before.compressed,
                        before.supported,
                    ),
                    (
                        after.depth,
                        after.width,
                        after.height,
                        after.palette,
                        after.indices,
                        after.packed_pixels,
                        after.compressed,
                        after.supported,
                    ),
                )
            return
        self.fail("no model was available for topology repacking")

    def test_v82_textured_packets_use_widened_uv_layout(self) -> None:
        path = ROOT / "V8_2_WORK" / "disc" / "SHARED" / "HOTRODS.EXP"
        if not path.is_file():
            self.skipTest("V8:2 gameplay vehicle archive is unavailable")
        for form in iff.parse(path.read_bytes()).forms(b"XOBF"):
            chunk = next(
                (child for child in form.children if child.tag == b"BIN "),
                None,
            )
            if chunk is None:
                continue
            model = xobf.Model(chunk.payload)
            for group in model.groups():
                for packet in group.packets:
                    if packet.kind == 5:
                        self.assertEqual(
                            int.from_bytes(packet.raw[0x16:0x18], "little"),
                            packet.texture_slot,
                        )
                        self.assertEqual(
                            (
                                (packet.raw[0x0C], packet.raw[0x0D]),
                                (packet.raw[0x10], packet.raw[0x11]),
                                (packet.raw[0x14], packet.raw[0x15]),
                            ),
                            packet.uv,
                        )
                        return
        self.fail("no V8:2 kind-5 textured packet was found")

    def test_v82_environment_packets_decode_engine_material_ids(self) -> None:
        path = (
            ROOT
            / "artifacts"
            / "dual_game_default_roundtrip"
            / "V82_COMMON_ORIGINAL.EXP"
        )
        if not path.is_file():
            self.skipTest("V8:2 gameplay vehicle archive is unavailable")
        environment_ids = set()
        for form in iff.parse(path.read_bytes()).forms(b"XOBF"):
            model_form = iff.IffChunk(
                tag=b"FORM",
                form_type=b"XOBF",
                children=tuple(
                    child
                    for child in form.children
                    if child.tag in {b"BIN ", b"ANM "}
                ),
            )
            bank = registry._decode_bank(model_form, "V8_2")
            for group in bank.groups:
                for face in group.faces:
                    if face.packet_kind != 12:
                        continue
                    self.assertTrue(face.environment_parameters)
                    material_id = face.environment_parameters[0] & 0x3FFF
                    environment_ids.add(material_id)
                    if material_id in {0x3FFE, 0x3FFF}:
                        self.assertIsNone(face.texture)
                    elif material_id < len(bank.textures):
                        self.assertEqual(material_id, face.texture)
        self.assertTrue(
            {0x3FFE, 0x3FFF} <= environment_ids,
            "retail V8:2 gloss/reflection packet IDs were not found",
        )

    def test_v82_common_direct_color_textures_are_fully_decoded(
        self,
    ) -> None:
        path = (
            ROOT
            / "artifacts"
            / "dual_game_default_roundtrip"
            / "V82_COMMON_ORIGINAL.EXP"
        )
        if not path.is_file():
            self.skipTest("V8:2 gameplay vehicle archive is unavailable")
        decoded_forms = 0
        direct_textures = []
        for form in iff.parse(path.read_bytes()).forms(b"XOBF"):
            model_form = iff.IffChunk(
                tag=b"FORM",
                form_type=b"XOBF",
                children=tuple(
                    child
                    for child in form.children
                    if child.tag in {b"BIN ", b"ANM "}
                ),
            )
            bank = registry._decode_bank(model_form, "V8_2")
            decoded_forms += 1
            direct_textures.extend(
                texture
                for texture in bank.textures
                if texture.depth == 2
            )
        self.assertGreaterEqual(decoded_forms, 19)
        self.assertEqual(19, len(direct_textures))
        for texture in direct_textures:
            self.assertFalse(texture.palette_bgr555)
            self.assertFalse(texture.indices)
            self.assertEqual(
                texture.width * texture.height,
                len(texture.direct_pixels_bgr555),
            )

    def test_guest_textured_geometry_repack_is_native_in_both_dialects(self) -> None:
        tested = set()
        for _path, _form, model in self._models():
            if model.dialect in tested or model.texture_count == 0:
                continue
            model.replace_group_geometry(
                0,
                ((0, 0, 0), (32, 0, 0), (0, 32, 0)),
                ((0, 1, 2),),
                colors=((100, 120, 140),),
                texture_slots=(0,),
                uvs=(((1, 2), (30, 4), (5, 29)),),
            )
            rebuilt = xobf.Model(model.data, model.dialect)
            group = rebuilt.group(0)
            packet = group.packets[0]
            self.assertEqual(5, packet.kind)
            self.assertEqual(0, packet.texture_slot)
            self.assertEqual(((1, 2), (30, 4), (5, 29)), packet.uv)
            self.assertEqual(1, group.normal_count)
            tested.add(model.dialect)
        self.assertEqual({"V8", "V8_2"}, tested)

    def test_v82_texture_encoder_round_trips_retail_pixels(self) -> None:
        path = ROOT / "V8_2_WORK" / "disc" / "SHARED" / "HOTRODS.EXP"
        if not path.is_file():
            self.skipTest("V8:2 gameplay vehicle archive is unavailable")
        tested = 0
        for form in iff.parse(path.read_bytes()).forms(b"XOBF"):
            chunk = next(
                (child for child in form.children if child.tag == b"BIN "),
                None,
            )
            if chunk is None:
                continue
            model = xobf.Model(chunk.payload)
            for texture in model.textures():
                if not texture.supported:
                    continue
                encoded = xobf.compress_v82_texture(texture.packed_pixels)
                self.assertEqual(
                    texture.packed_pixels,
                    xobf.decompress_v82_texture(
                        encoded, len(texture.packed_pixels)
                    ),
                )
                tested += 1
        self.assertGreater(tested, 0)

    def test_texture_palette_and_indices_can_be_replaced(self) -> None:
        for _path, _form, model in self._models():
            texture = next(
                (texture for texture in model.textures() if texture.supported),
                None,
            )
            if texture is None:
                continue
            palette = list(texture.palette)
            palette[0] ^= 0x001F
            indices = bytearray(texture.indices)
            indices[0] = 0
            model.replace_texture(texture.index, palette, bytes(indices))
            rebuilt = xobf.Model(model.data, model.dialect)
            edited = rebuilt.texture(texture.index)
            self.assertEqual(tuple(palette), edited.palette)
            self.assertEqual(bytes(indices), edited.indices)
            return
        self.fail("no replaceable native texture was found")

    def test_all_retail_animation_streams_parse(self) -> None:
        bank_count = 0
        frame_count = 0
        for path in IffRoundTripTests.ASSETS:
            if not path.is_file():
                continue
            for form in iff.parse(path.read_bytes()).forms(b"XOBF"):
                bins = [child.payload for child in form.children if child.tag == b"BIN "]
                anms = [child.payload for child in form.children if child.tag == b"ANM "]
                if not bins or not anms:
                    continue
                model = xobf.Model(bins[0])
                bank = xobf.AnimationBank(anms[0], model.slot_count)
                for slot_index in range(model.slot_count):
                    frame_count += len(bank.frames(slot_index))
                bank_count += 1
        self.assertGreater(bank_count, 0)
        self.assertGreater(frame_count, 0)


class VehicleStatsTests(unittest.TestCase):
    EXECUTABLES = (
        ROOT / "PS1 game" / "SLUS_005.10",
        ROOT / "V8_2_WORK" / "disc" / "SLUS_008.68",
    )

    def test_retail_stats_tables_detect_and_parse(self) -> None:
        games = set()
        for path in self.EXECUTABLES:
            if not path.is_file():
                continue
            source = path.read_bytes()
            parsed = stats.StatsFile(source)
            self.assertEqual(source, parsed.data)
            self.assertEqual(
                list(range(parsed.profile.record_count)),
                [record.get("vehicle_type") for record in parsed.records()],
            )
            games.add(parsed.profile.game)
        self.assertEqual({"V8", "V8_2"}, games)

    def test_stat_patch_changes_only_the_native_field(self) -> None:
        for path in self.EXECUTABLES:
            if not path.is_file():
                continue
            source = path.read_bytes()
            parsed = stats.StatsFile(source)
            record = parsed.record(0)
            old_health = record.get("health")
            record.set("health", old_health + 1)
            changed = [
                index
                for index, pair in enumerate(zip(source, parsed.data))
                if pair[0] != pair[1]
            ]
            health_offset = record.offset + parsed.field("health").offset
            self.assertTrue(changed)
            self.assertTrue(
                all(health_offset <= index < health_offset + 2 for index in changed)
            )
            reparsed = stats.StatsFile(parsed.data, parsed.profile.game)
            self.assertEqual(old_health + 1, reparsed.record(0).get("health"))

    def test_v82_powerups_and_transformations_parse_and_patch(self) -> None:
        path = self.EXECUTABLES[1]
        if not path.is_file():
            self.skipTest("V8:2 retail executable is unavailable")
        source = path.read_bytes()
        parsed = stats.StatsFile(source)
        self.assertEqual(
            {
                "repair_amount": 500,
                "weapon_upgrade_duration": 900,
                "radar_jammer_duration": 900,
                "shield_duration": 900,
                "transformation_duration": 500,
            },
            parsed.powerup_values(),
        )
        modes = parsed.transform_modes()
        self.assertEqual((0, 0, 0, 0, 0, 0), modes[0])
        self.assertEqual((56, 72, 56, 72, 56, 72), modes[1])
        parsed.set_powerup("shield_duration", 901)
        parsed.set_transform_wheel_kind(3, 5, 76)
        reparsed = stats.StatsFile(parsed.data)
        self.assertEqual(901, reparsed.powerup_values()["shield_duration"])
        self.assertEqual(76, reparsed.transform_modes()[3][5])


if __name__ == "__main__":
    unittest.main()
