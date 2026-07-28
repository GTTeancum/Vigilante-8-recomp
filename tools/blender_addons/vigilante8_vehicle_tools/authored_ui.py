"""Production Blender UI for independent, donor-free vehicle projects."""

from __future__ import annotations

import json
from pathlib import Path
import textwrap

import bpy
from bpy.props import BoolProperty, EnumProperty, IntProperty, StringProperty
from bpy.types import Operator, Panel, PropertyGroup
from bpy_extras.io_utils import ExportHelper, ImportHelper

from . import authored_scene, project, registry, stats


TRANSFORM_MODE_NAMES = ("Standard", "Hover", "Float", "Ski")
WHEEL_NAMES = ("Front L", "Front R", "Middle L", "Middle R", "Rear L", "Rear R")


def active_vehicle(context) -> bpy.types.Collection | None:
    settings = context.scene.v8_vehicle_settings
    selected = bpy.data.collections.get(settings.vehicle_collection)
    if selected is not None and selected.get("v8_role") == authored_scene.ROLE_VEHICLE:
        return selected
    obj = context.active_object
    if obj is None:
        return None
    for collection in bpy.data.collections:
        if (
            collection.get("v8_role") == authored_scene.ROLE_VEHICLE
            and obj.name in collection.all_objects
        ):
            settings.vehicle_collection = collection.name
            return collection
    return None


def _update_authoring_visibility(settings, _context) -> None:
    collection = bpy.data.collections.get(settings.vehicle_collection)
    if (
        collection is None
        or collection.get("v8_role") != authored_scene.ROLE_VEHICLE
    ):
        return
    authored_scene.apply_authoring_visibility(
        collection,
        preview_state=settings.preview_state,
        show_attachment_guides=settings.show_attachment_guides,
        show_advanced=settings.show_advanced_engine_objects,
        show_transform_library=settings.show_transformation_library,
    )
    authored_scene.update_transformation_preview(
        collection, settings.transformation_preview_mode
    )


class V8VehicleSettings(PropertyGroup):
    vehicle_collection: StringProperty(name="Vehicle collection")
    show_advanced: BoolProperty(name="Show advanced fields", default=False)
    show_stat_explanations: BoolProperty(
        name="Show meanings, bounds, and value direction",
        default=True,
    )
    preview_state: EnumProperty(
        name="Visible body state",
        description="Show one physical damage state instead of overlapping variants",
        items=(
            ("INTACT", "Intact", "Normal undamaged vehicle"),
            ("DAMAGED", "Damaged", "First replacement-damage state"),
            (
                "HEAVILY_DAMAGED",
                "Heavily Damaged",
                "Second replacement-damage state",
            ),
            ("DESTROYED", "Destroyed", "Destruction replacement state"),
            ("ALL", "All States", "Diagnostic view of every native state"),
        ),
        default="INTACT",
        update=_update_authoring_visibility,
    )
    show_attachment_guides: BoolProperty(
        name="Attachment guides and labels",
        description="Show wheel, weapon, light, effect, and placement guides",
        default=False,
        update=_update_authoring_visibility,
    )
    show_advanced_engine_objects: BoolProperty(
        name="Advanced engine objects",
        description=(
            "Show collisions, animations, debris, upgrade models, LODs, "
            "and other engine-only records"
        ),
        default=False,
        update=_update_authoring_visibility,
    )
    show_transformation_library: BoolProperty(
        name="Raw transformation library",
        description=(
            "Show the independent V8:2 wheel-replacement object bank in its "
            "native library coordinates, not as a mounted vehicle preview"
        ),
        default=False,
        update=_update_authoring_visibility,
    )
    transformation_preview_mode: EnumProperty(
        name="Mounted wheel mode",
        description=(
            "Choose the wheel replacement assembled at the vehicle's real "
            "wheel anchors; preview geometry is not exported"
        ),
        items=(
            (
                "STANDARD",
                "Standard",
                "Show the normal wheel objects selected by the vehicle stats",
            ),
            (
                "HOVER",
                "Hover",
                "Show Hover replacements mounted at the vehicle wheel anchors",
            ),
            (
                "FLOAT",
                "Float",
                "Show Float replacements mounted at the vehicle wheel anchors",
            ),
            (
                "SKI",
                "Ski",
                "Show Ski replacements mounted at the vehicle wheel anchors",
            ),
            (
                "NONE",
                "Body Only",
                "Hide all mounted wheel replacement previews",
            ),
        ),
        default="STANDARD",
        update=_update_authoring_visibility,
    )
    new_game: EnumProperty(
        name="Game",
        items=(
            ("V8_2", "V8: 2nd Offense", "Lead engine with transformations and powerups"),
            ("V8", "Vigilante 8", "Original-game compatible subset"),
        ),
        default="V8_2",
    )


class V8_OT_new_authored_vehicle(Operator):
    bl_idname = "v8.new_authored_vehicle"
    bl_label = "New Original Vehicle"
    bl_description = "Create a complete source-free native vehicle project"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        try:
            vehicle = authored_scene.new_project(
                context.scene.v8_vehicle_settings.new_game
            )
            collection = authored_scene.project_to_scene(context, vehicle)
            collection["v8_collision_export"] = "AUTO_ALWAYS"
        except Exception as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Created independent {vehicle.game} vehicle")
        return {"FINISHED"}


class V8_OT_import_authored_project(Operator, ImportHelper):
    bl_idname = "v8.import_authored_project"
    bl_label = "Import Vehicle Project"
    bl_description = "Import a strict decoded vehicle JSON project"
    bl_options = {"REGISTER", "UNDO"}

    filename_ext = ".json"
    filter_glob: StringProperty(default="*.json", options={"HIDDEN"})

    def execute(self, context):
        try:
            vehicle = project.VehicleProject.from_dict(
                json.loads(Path(self.filepath).read_text(encoding="utf-8"))
            )
            authored_scene.project_to_scene(context, vehicle)
        except Exception as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Imported {vehicle.game} project {vehicle.stable_id}")
        return {"FINISHED"}


class V8_OT_import_native_package(Operator, ImportHelper):
    bl_idname = "v8.import_native_package"
    bl_label = "Import Native Vehicle Package"
    bl_description = (
        "Decode VEHICLES.V8R and CUSTOM.EXP into named editable fields"
    )
    bl_options = {"REGISTER", "UNDO"}

    filename_ext = ".V8R"
    filter_glob: StringProperty(
        default="*.V8R;*.v8r", options={"HIDDEN"})

    def execute(self, context):
        try:
            registry_path = Path(self.filepath)
            archive_path = registry_path.parent / "CUSTOM.EXP"
            if not archive_path.is_file():
                raise FileNotFoundError(
                    f"CUSTOM.EXP is missing beside {registry_path.name}")
            vehicles = registry.decompile_package(
                archive_path.read_bytes(),
                registry_path.read_bytes(),
            )
            for vehicle in vehicles:
                authored_scene.project_to_scene(context, vehicle)
        except Exception as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report(
            {"INFO"},
            f"Decoded {len(vehicles)} independent {vehicles[0].game} "
            "vehicle(s)",
        )
        return {"FINISHED"}


class V8_OT_export_authored_project(Operator, ExportHelper):
    bl_idname = "v8.export_authored_project"
    bl_label = "Export Vehicle Project"
    bl_description = "Export named decoded fields with no source or passthrough data"

    filename_ext = ".json"
    filter_glob: StringProperty(default="*.json", options={"HIDDEN"})

    def invoke(self, context, event):
        collection = active_vehicle(context)
        if collection is not None:
            self.filepath = str(collection.get("v8_stable_id", "vehicle")) + ".json"
        return ExportHelper.invoke(self, context, event)

    def execute(self, context):
        collection = active_vehicle(context)
        if collection is None:
            self.report({"ERROR"}, "Select an authored vehicle collection")
            return {"CANCELLED"}
        try:
            vehicle = authored_scene.scene_to_project(collection)
            Path(self.filepath).write_text(
                json.dumps(project.to_dict(vehicle), indent=2) + "\n",
                encoding="utf-8",
            )
        except Exception as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report({"INFO"}, f"Wrote strict project {self.filepath}")
        return {"FINISHED"}


class V8_OT_compile_authored_package(Operator, ExportHelper):
    bl_idname = "v8.compile_authored_package"
    bl_label = "Compile Native Vehicle Package"
    bl_description = "Build CUSTOM.EXP and append-only VEHICLES.V8R"

    filename_ext = ".V8R"
    filter_glob: StringProperty(default="*.V8R;*.v8r", options={"HIDDEN"})
    export_all: BoolProperty(
        name="All authored vehicles for this game",
        default=True,
    )

    def invoke(self, context, event):
        self.filepath = "VEHICLES.V8R"
        return ExportHelper.invoke(self, context, event)

    def execute(self, context):
        selected = active_vehicle(context)
        if selected is None:
            self.report({"ERROR"}, "Select an authored vehicle collection")
            return {"CANCELLED"}
        try:
            collections = (
                sorted(
                    (
                        collection
                        for collection in bpy.data.collections
                        if collection.get("v8_role") == authored_scene.ROLE_VEHICLE
                        and collection.get("v8_game") == selected.get("v8_game")
                    ),
                    key=lambda collection: str(collection.get("v8_stable_id", "")),
                )
                if self.export_all
                else [selected]
            )
            vehicles = tuple(
                authored_scene.scene_to_project(collection)
                for collection in collections
            )
            package = registry.compile_package(vehicles)
            registry_path = Path(self.filepath)
            registry_path.write_bytes(package.registry)
            (registry_path.parent / "CUSTOM.EXP").write_bytes(package.archive)
        except Exception as error:
            self.report({"ERROR"}, str(error))
            return {"CANCELLED"}
        self.report(
            {"INFO"},
            f"Compiled {len(vehicles)} independent {package.game} vehicle(s)",
        )
        return {"FINISHED"}


class V8_OT_select_vehicle_part(Operator):
    bl_idname = "v8.select_vehicle_part"
    bl_label = "Select and Frame Vehicle Part"
    bl_description = "Select this named native part and frame it in the 3D View"
    bl_options = {"REGISTER"}

    object_name: StringProperty(name="Object")

    def execute(self, context):
        obj = bpy.data.objects.get(self.object_name)
        if obj is None:
            self.report({"ERROR"}, f"Vehicle part not found: {self.object_name}")
            return {"CANCELLED"}
        for selected in tuple(context.selected_objects):
            selected.select_set(False)
        obj.hide_viewport = False
        obj.hide_set(False)
        obj.select_set(True)
        context.view_layer.objects.active = obj

        framed = False
        for window in context.window_manager.windows:
            area = next(
                (candidate for candidate in window.screen.areas
                 if candidate.type == "VIEW_3D"),
                None,
            )
            if area is None:
                continue
            region = next(
                (candidate for candidate in area.regions
                 if candidate.type == "WINDOW"),
                None,
            )
            if region is None:
                continue
            with context.temp_override(
                window=window,
                area=area,
                region=region,
                active_object=obj,
                selected_objects=[obj],
                selected_editable_objects=[obj],
            ):
                bpy.ops.view3d.view_selected(use_all_regions=False)
            framed = True
            break
        self.report(
            {"INFO"},
            f"Selected {obj.name}" + ("" if framed else " (no 3D View to frame)"),
        )
        return {"FINISHED"}


class V8_OT_assign_transform_object(Operator):
    bl_idname = "v8.assign_transform_object"
    bl_label = "Use Selected Wheel Replacement"
    bl_description = (
        "Assign the selected top-level transformation-bank object to this "
        "mode and wheel position"
    )
    bl_options = {"REGISTER", "UNDO"}

    mode: IntProperty(min=1, max=3)
    wheel: IntProperty(min=0, max=5)

    def execute(self, context):
        collection = active_vehicle(context)
        obj = context.active_object
        if collection is None or collection.get("v8_game") != "V8_2":
            self.report({"ERROR"}, "Select a V8:2 authored vehicle")
            return {"CANCELLED"}
        if (
            obj is None
            or obj.get("v8_role") != "authored_slot"
            or obj.parent is not None
            or not any(
                owner.get("v8_bank") == "transformation"
                for owner in obj.users_collection
            )
        ):
            self.report(
                {"ERROR"},
                "Select a top-level object in the Transformation Object Bank",
            )
            return {"CANCELLED"}
        collection[f"v8_transform_{self.mode}_{self.wheel}"] = int(
            obj["v8_slot_index"]
        )
        authored_scene.refresh_transformation_names(collection)
        self.report(
            {"INFO"},
            f"{TRANSFORM_MODE_NAMES[self.mode]} {WHEEL_NAMES[self.wheel]} "
            f"uses {obj.name}",
        )
        return {"FINISHED"}


class V8_OT_assign_standard_wheel_object(Operator):
    bl_idname = "v8.assign_standard_wheel_object"
    bl_label = "Use Selected Standard Wheel Object"
    bl_description = (
        "Assign the selected top-level native object as this standard wheel "
        "model; named wheel anchors still control placement"
    )
    bl_options = {"REGISTER", "UNDO"}

    field_name: StringProperty()

    def execute(self, context):
        collection = active_vehicle(context)
        obj = context.active_object
        if collection is None or self.field_name not in {
            "wheel_kind_front",
            "wheel_kind_rear",
        }:
            self.report({"ERROR"}, "Select an authored vehicle")
            return {"CANCELLED"}
        expected_bank = (
            "body" if collection.get("v8_game") == "V8" else "transformation"
        )
        if (
            obj is None
            or obj.get("v8_role") != "authored_slot"
            or obj.parent is not None
            or not any(
                owner.get("v8_bank") == expected_bank
                for owner in obj.users_collection
            )
        ):
            self.report(
                {"ERROR"},
                f"Select a top-level object in the {expected_bank} object bank",
            )
            return {"CANCELLED"}
        collection[f"v8_stat_{self.field_name}"] = int(
            obj["v8_slot_index"]
        )
        if collection.get("v8_game") == "V8_2":
            authored_scene.refresh_transformation_names(collection)
        self.report({"INFO"}, f"Standard wheel object set to {obj.name}")
        return {"FINISHED"}


class V8_PT_vehicle_tools(Panel):
    bl_label = "Vigilante 8 Native Vehicles"
    bl_idname = "V8_PT_vehicle_tools"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"

    def draw(self, context):
        layout = self.layout
        settings = context.scene.v8_vehicle_settings
        row = layout.row(align=True)
        row.prop(settings, "new_game", text="")
        row.operator(V8_OT_new_authored_vehicle.bl_idname, text="New", icon="ADD")
        row = layout.row(align=True)
        row.operator(
            V8_OT_import_native_package.bl_idname,
            text="Native Package",
            icon="IMPORT",
        )
        row.operator(V8_OT_import_authored_project.bl_idname, icon="IMPORT")
        row.operator(V8_OT_export_authored_project.bl_idname, icon="EXPORT")
        layout.operator(V8_OT_compile_authored_package.bl_idname, icon="PACKAGE")
        collection = active_vehicle(context)
        if collection is None:
            layout.label(text="No authored vehicle selected", icon="INFO")
            return
        layout.prop_search(
            settings, "vehicle_collection", bpy.data, "collections", text="Active"
        )
        display = layout.box()
        display.label(text="Authoring View", icon="HIDE_OFF")
        display.label(
            text="Collision boxes: automatic on export",
            icon="MOD_PHYSICS",
        )
        display.prop(settings, "preview_state")
        if collection.get("v8_game") == "V8_2":
            display.prop(settings, "transformation_preview_mode")
        display.prop(settings, "show_attachment_guides")
        display.prop(settings, "show_advanced_engine_objects")
        if (
            collection.get("v8_game") == "V8_2"
            and settings.show_advanced_engine_objects
        ):
            display.prop(settings, "show_transformation_library")
        box = layout.box()
        box.prop(collection, '["v8_stable_id"]', text="Stable ID")
        box.prop(collection, '["v8_display_name"]', text="Display name")
        box.label(
            text=f"{collection.get('v8_game')} schema "
            f"{collection.get('v8_schema_version')}"
        )
        try:
            vehicle = authored_scene.scene_to_project(
                collection, prepare_collision=False
            )
            banks = [project.bank_memory_usage(vehicle)]
            if vehicle.transformation_bank is not None:
                banks.append(project.bank_memory_usage(vehicle.transformation_bank))
            box.label(
                text=(
                    f"Selected-bank textures: "
                    f"{sum(item.texels for item in banks):,} texels / "
                    f"{sum(item.expanded_rgba_bytes for item in banks) // 1024:,} KiB RGBA"
                )
            )
            box.label(
                text=(
                    f"Native texture payload: "
                    f"{sum(item.native_texture_bytes for item in banks) // 1024:,} KiB"
                )
            )
        except Exception as error:
            box.label(text=f"Budget validation: {error}", icon="ERROR")
        box.label(text="No retail source, donor entry, or opaque payload.")


class V8_PT_parts_and_attachments(Panel):
    bl_label = "Native Object Hierarchy"
    bl_idname = "V8_PT_parts_and_attachments"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"
    bl_parent_id = V8_PT_vehicle_tools.bl_idname

    RECORD_ROLES = {
        "authored_render_group",
        "authored_collision_stream",
        "authored_animation",
    }

    @staticmethod
    def _draw_select_row(layout, obj, depth: int, icon: str) -> None:
        row = layout.row(align=True)
        if depth:
            row.separator(factor=depth * 1.5)
        operator = row.operator(
            V8_OT_select_vehicle_part.bl_idname,
            text=obj.name,
            icon=icon,
        )
        operator.object_name = obj.name

    def _draw_slot_tree(self, layout, obj, slots, depth=0) -> None:
        role = str(obj.get("v8_part_role", ""))
        icon = (
            "MESH_DATA"
            if obj.type == "MESH"
            else "EMPTY_SINGLE_ARROW"
            if role == "weapon_mount"
            else "EMPTY_ARROWS"
            if role in {"wheel_anchor", "transformation_wheel_root"}
            else "EMPTY_AXIS"
        )
        self._draw_select_row(layout, obj, depth, icon)

        records = sorted(
            (
                child for child in obj.children
                if child.get("v8_role") in self.RECORD_ROLES
            ),
            key=lambda child: child.name,
        )
        for record in records:
            record_icon = (
                "MESH_DATA"
                if record.type == "MESH"
                else "PHYSICS"
                if record.get("v8_role") == "authored_collision_stream"
                else "ACTION"
            )
            self._draw_select_row(layout, record, depth + 1, record_icon)

        children = sorted(
            (candidate for candidate in slots if candidate.parent == obj),
            key=lambda candidate: int(candidate.get("v8_slot_index", -1)),
        )
        for child in children:
            self._draw_slot_tree(layout, child, slots, depth + 1)

    def draw(self, context):
        layout = self.layout
        collection = active_vehicle(context)
        if collection is None:
            layout.label(text="Select an authored vehicle", icon="INFO")
            return

        layout.label(
            text=(
                "The tree below mirrors native slot parenting. "
                "Names are functional; native IDs remain advanced metadata."
            ),
            icon="INFO",
        )
        banks = sorted(
            (
                child for child in collection.children
                if child.get("v8_role") == authored_scene.ROLE_BANK
            ),
            key=lambda child: (
                0 if child.get("v8_bank") == "body" else 1,
                child.name,
            ),
        )
        for bank in banks:
            slots = sorted(
                (
                    obj for obj in bank.all_objects
                    if obj.get("v8_role") == "authored_slot"
                ),
                key=lambda obj: int(obj.get("v8_slot_index", -1)),
            )
            box = layout.box()
            box.label(
                text=f"{str(bank.get('v8_bank', '')).title()} Objects",
                icon="OUTLINER_COLLECTION",
            )
            roots = [
                obj for obj in slots
                if obj.parent is None
                or obj.parent.get("v8_role") != "authored_slot"
            ]
            for root in roots:
                self._draw_slot_tree(box, root, slots)

        active = context.active_object
        if (
            active is not None
            and active.get("v8_role") == "authored_slot"
            and active.name in collection.all_objects
        ):
            box = layout.box()
            box.label(text=f"Selected: {active.name}", icon="OBJECT_ORIGIN")
            row = box.row()
            role = str(active.get("v8_part_role", "unknown"))
            row.label(text=f"Role: {role.replace('_', ' ').title()}")
            row.label(
                text=f"Confidence: {active.get('v8_part_name_confidence', 'UNKNOWN')}"
            )
            native_key = str(active.get("v8_native_key_hex", ""))
            box.label(
                text=(
                    f"Engine attachment identity: {native_key}"
                    if native_key
                    else "Native class: "
                    + str(active.get("v8_native_object_class", "Ordinary object"))
                )
            )
            native_object_id = int(active.get("v8_native_object_id", -21846))
            if native_object_id & 0xFFFF != 0xAAAA:
                box.label(
                    text=f"Engine role ID: 0x{native_object_id & 0xFFFF:04X}"
                )
            box.label(text=str(active.get("v8_part_name_source", "")))
            box.prop(active, "location", text="Attachment Position")
            box.prop(active, "rotation_euler", text="Attachment Rotation")


class V8_PT_standard_wheels(Panel):
    bl_label = "Standard Wheel Objects and Placement"
    bl_idname = "V8_PT_standard_wheels"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"
    bl_parent_id = V8_PT_vehicle_tools.bl_idname

    @classmethod
    def poll(cls, context):
        return active_vehicle(context) is not None

    def draw(self, context):
        collection = active_vehicle(context)
        game = str(collection["v8_game"])
        bank_name = "body" if game == "V8" else "transformation"
        bank = next(
            child
            for child in collection.children
            if child.get("v8_bank") == bank_name
        )
        roots = {
            int(obj["v8_slot_index"]): obj
            for obj in bank.all_objects
            if obj.get("v8_role") == "authored_slot"
            and obj.parent is None
        }
        self.layout.label(
            text="Wheel anchors set placement; these objects set geometry.",
            icon="INFO",
        )
        labels = (
            ("wheel_kind_front", "Front axle"),
            (
                "wheel_kind_rear",
                "Rear axle" if game == "V8" else "Second/third axle",
            ),
        )
        for field_name, label in labels:
            row = self.layout.row(align=True)
            row.label(text=label)
            root_index = int(collection[f"v8_stat_{field_name}"])
            root = roots.get(root_index)
            if root is None:
                row.label(text="Missing native object", icon="ERROR")
            else:
                select = row.operator(
                    V8_OT_select_vehicle_part.bl_idname,
                    text=root.name,
                    icon="MESH_DATA" if root.type == "MESH" else "EMPTY_ARROWS",
                )
                select.object_name = root.name
            assign = row.operator(
                V8_OT_assign_standard_wheel_object.bl_idname,
                text="Use Selected",
                icon="EYEDROPPER",
            )
            assign.field_name = field_name


class V8_PT_stats(Panel):
    bl_label = "Vehicle Stats"
    bl_idname = "V8_PT_stats"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"
    bl_parent_id = V8_PT_vehicle_tools.bl_idname

    @staticmethod
    def _labels(layout, text: str, icon: str = "NONE") -> None:
        lines = textwrap.wrap(text, width=72) or [""]
        for index, line in enumerate(lines):
            layout.label(text=line, icon=icon if index == 0 else "NONE")

    def draw(self, context):
        layout = self.layout
        collection = active_vehicle(context)
        if collection is None:
            layout.label(text="Select an authored vehicle", icon="INFO")
            return
        settings = context.scene.v8_vehicle_settings
        game = str(collection["v8_game"])
        profile = stats.PROFILES[game]
        layout.label(
            text=(
                "Custom record: VEHICLES.V8R "
                f"({profile.record_size:#04x} bytes)"
            ),
            icon="FILE",
        )
        layout.label(
            text=f"Retail source: PS-X EXE at 0x{profile.table_address:08X}",
            icon="INFO",
        )
        layout.label(
            text="The game assigns the custom roster index during registration."
        )
        layout.prop(settings, "show_advanced")
        layout.prop(settings, "show_stat_explanations")
        if game == "V8_2":
            warning = layout.box()
            warning.alert = True
            self._labels(
                warning,
                "Retail V8:2 derives rear damping from its capability/type "
                "word. Custom registry v3 stores Rear Suspension Damping "
                "independently and reapplies each constructed wheel using "
                "its axle pair.",
                "INFO",
            )
        for field in profile.authoring_fields:
            if field.name in {
                "vehicle_type",
                "wheel_kind_front",
                "wheel_kind_rear",
            } or (
                field.advanced and not settings.show_advanced
            ):
                continue
            box = layout.box()
            row = box.row()
            row.enabled = field.editable
            row.prop(
                collection,
                f'["v8_stat_{field.name}"]',
                text=field.label,
            )
            if not field.editable:
                row.label(text="Read-only", icon="LOCKED")
            if not settings.show_stat_explanations:
                continue
            self._labels(box, field.description, "INFO")
            bounds = (
                f"Unit: {field.unit}. Native bound: "
                f"{field.native_min} to {field.native_max}."
            )
            if (
                field.authoring_min != field.native_min
                or field.authoring_max != field.native_max
            ):
                bounds += (
                    " Safe authoring bound: "
                    f"{field.authoring_min} to {field.authoring_max}."
                )
            if (
                field.recommended_min is not None
                and field.recommended_max is not None
            ):
                bounds += (
                    " Retail-tested range: "
                    f"{field.recommended_min} to {field.recommended_max}."
                )
            self._labels(box, bounds)
            if field.lower_effect:
                self._labels(box, f"Lower values: {field.lower_effect}.")
            if field.higher_effect:
                self._labels(box, f"Higher values: {field.higher_effect}.")


class V8_PT_powerups(Panel):
    bl_label = "V8:2 Powerups"
    bl_idname = "V8_PT_powerups"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"
    bl_parent_id = V8_PT_stats.bl_idname

    @classmethod
    def poll(cls, context):
        collection = active_vehicle(context)
        return collection is not None and collection.get("v8_game") == "V8_2"

    def draw(self, context):
        collection = active_vehicle(context)
        show_explanations = (
            context.scene.v8_vehicle_settings.show_stat_explanations
        )
        settings = {
            setting.name: setting
            for setting in stats.V82_POWERUP_SETTINGS
        }
        for name in project.V82_POWERUP_FIELDS:
            setting = settings[name]
            box = self.layout.box()
            box.prop(
                collection,
                f'["v8_powerup_{name}"]',
                text=setting.label,
            )
            if not show_explanations:
                continue
            V8_PT_stats._labels(box, setting.description, "INFO")
            V8_PT_stats._labels(
                box,
                f"Unit: {setting.unit}. Safe native immediate bound: "
                f"{setting.author_min} to {setting.author_max}. "
                f"Retail value: {setting.retail_value}.",
            )
            V8_PT_stats._labels(
                box, f"Lower values: {setting.lower_effect}."
            )
            V8_PT_stats._labels(
                box, f"Higher values: {setting.higher_effect}."
            )


class V8_PT_transformations(Panel):
    bl_label = "V8:2 Transformation Wheel Objects"
    bl_idname = "V8_PT_transformations"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "scene"
    bl_parent_id = V8_PT_stats.bl_idname
    bl_options = {"DEFAULT_CLOSED"}

    @classmethod
    def poll(cls, context):
        return V8_PT_powerups.poll(context)

    def draw(self, context):
        collection = active_vehicle(context)
        explanation = self.layout.box()
        explanation.label(
            text="The engine swaps each wheel to a replacement native object.",
            icon="INFO",
        )
        explanation.label(
            text="This table is object selection, not a bone or morph target."
        )
        explanation.label(
            text="Animations owned by the selected replacement object still run."
        )
        bank = next(
            child
            for child in collection.children
            if child.get("v8_bank") == "transformation"
        )
        roots = {
            int(obj["v8_slot_index"]): obj
            for obj in bank.all_objects
            if obj.get("v8_role") == "authored_slot"
            and obj.parent is None
        }
        standard = self.layout.box()
        standard.label(text="Standard")
        standard.label(
            text="Uses the six named wheel anchors; no replacement objects.",
            icon="INFO",
        )
        for mode, mode_name in enumerate(TRANSFORM_MODE_NAMES[1:], 1):
            box = self.layout.box()
            box.label(text=mode_name)
            for wheel, wheel_name in enumerate(WHEEL_NAMES):
                row = box.row(align=True)
                root_index = int(
                    collection[f"v8_transform_{mode}_{wheel}"]
                )
                root = roots.get(root_index)
                row.label(text=wheel_name)
                if root is None:
                    row.label(text="Missing native object", icon="ERROR")
                else:
                    select = row.operator(
                        V8_OT_select_vehicle_part.bl_idname,
                        text=root.name,
                        icon="MESH_DATA" if root.type == "MESH" else "EMPTY_ARROWS",
                    )
                    select.object_name = root.name
                assign = row.operator(
                    V8_OT_assign_transform_object.bl_idname,
                    text="Use Selected",
                    icon="EYEDROPPER",
                )
                assign.mode = mode
                assign.wheel = wheel


def _menu_import(self, _context):
    self.layout.operator(
        V8_OT_import_native_package.bl_idname,
        text="Vigilante 8 Native Vehicle Package (.V8R)",
    )
    self.layout.operator(
        V8_OT_import_authored_project.bl_idname,
        text="Vigilante 8 Vehicle Project (.json)",
    )


def _menu_export(self, _context):
    self.layout.operator(
        V8_OT_export_authored_project.bl_idname,
        text="Vigilante 8 Vehicle Project (.json)",
    )


CLASSES = (
    V8VehicleSettings,
    V8_OT_new_authored_vehicle,
    V8_OT_import_authored_project,
    V8_OT_import_native_package,
    V8_OT_export_authored_project,
    V8_OT_compile_authored_package,
    V8_OT_select_vehicle_part,
    V8_OT_assign_transform_object,
    V8_OT_assign_standard_wheel_object,
    V8_PT_vehicle_tools,
    V8_PT_parts_and_attachments,
    V8_PT_standard_wheels,
    V8_PT_stats,
    V8_PT_powerups,
    V8_PT_transformations,
)


def register() -> None:
    for cls in CLASSES:
        bpy.utils.register_class(cls)
    bpy.types.Scene.v8_vehicle_settings = bpy.props.PointerProperty(
        type=V8VehicleSettings
    )
    bpy.types.TOPBAR_MT_file_import.append(_menu_import)
    bpy.types.TOPBAR_MT_file_export.append(_menu_export)


def unregister() -> None:
    bpy.types.TOPBAR_MT_file_export.remove(_menu_export)
    bpy.types.TOPBAR_MT_file_import.remove(_menu_import)
    del bpy.types.Scene.v8_vehicle_settings
    for cls in reversed(CLASSES):
        bpy.utils.unregister_class(cls)
