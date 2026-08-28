"""Donor-free Blender scene representation for strict vehicle projects.

No retail archive, executable, binary record, or opaque payload is stored in
the scene.  Every exported byte is rebuilt from named, editable scene data.
"""

from __future__ import annotations

import bpy
from mathutils import Matrix, Vector

from . import project, stats


ROLE_VEHICLE = "authored_vehicle"
ROLE_BANK = "authored_bank"
ROLE_TRANSFORMATION_PREVIEW = "authored_transformation_preview"
NATIVE_NORMAL_TABLE_PROPERTY = "v8_native_normal_table"
ATTACHMENT_GUIDE_ROLES = {
    "wheel_anchor",
    "weapon_mount",
    "headlight_emitter",
    "tail_light_emitter",
    "destruction_effect_anchor",
    "auxiliary_powered_attachment",
    "auxiliary_attachment",
    "special_weapon_anchor",
    "damage_effect_anchor",
    "transform_node",
    "collision_part",
}
ADVANCED_MODEL_ROLES = {
    "distance_lod_model",
    "hud_vehicle_icon_source",
    "acceleration_upgrade_model",
    "top_speed_upgrade_model",
    "armor_upgrade_model",
    "handling_upgrade_model",
    "body_damage_debris",
    "body_damage_collision",
    "destruction_debris_model",
    "physical_destruction_debris",
    "destruction_particle_model",
    "destruction_collision",
}
ENGINE_RECORD_ROLES = {
    "authored_render_control",
    "authored_render_control_entry",
    "authored_collision_stream",
    "authored_collision_shape",
    "authored_animation",
    "authored_animation_frame",
    "authored_animation_texture_binding",
}
ANGLE = 2.0 * 3.141592653589793 / 4096.0
FIXED = 65536.0
# Native pickup names are I_Hover, I_Float, and I_Ski; the corresponding
# engine _WHEELS values are Ground, Air, Sea, and Snow.
TRANSFORM_MODE_NAMES = ("Standard", "Hover", "Float", "Ski")
TRANSFORM_WHEEL_NAMES = (
    "Front Left",
    "Front Right",
    "Middle Left",
    "Middle Right",
    "Rear Left",
    "Rear Right",
)


def _native_vector_to_blender(
    value: tuple[int, int, int], scale: float
) -> tuple[float, float, float]:
    x, y, z = value
    return (x / scale, z / scale, -y / scale)


def _blender_vector_to_native(
    value, scale: float
) -> tuple[int, int, int]:
    return (
        int(round(value[0] * scale)),
        int(round(-value[2] * scale)),
        int(round(value[1] * scale)),
    )


def _native_rotation_to_blender(
    rotation_yxz: tuple[int, int, int]
) -> tuple[float, float, float]:
    rot_y, rot_x, rot_z = rotation_yxz
    return (rot_x * ANGLE, rot_z * ANGLE, -rot_y * ANGLE)


def _blender_rotation_to_native(euler) -> tuple[int, int, int]:
    values = (
        round(-euler[2] / ANGLE),
        round(euler[0] / ANGLE),
        round(euler[1] / ANGLE),
    )
    return tuple(((value + 32768) & 0xFFFF) - 32768 for value in values)


def _prop_tuple(owner, name: str, count: int, default=None):
    value = owner.get(name, default)
    if value is None or len(value) != count:
        return None
    return tuple(int(round(item)) for item in value)


def _link_collection(parent: bpy.types.Collection, child: bpy.types.Collection) -> None:
    if child.name not in parent.children:
        parent.children.link(child)


def _objects(collection: bpy.types.Collection, role: str) -> list[bpy.types.Object]:
    return [obj for obj in collection.all_objects if obj.get("v8_role") == role]


def _srgb_to_linear(value: float) -> float:
    return (
        value / 12.92
        if value <= 0.04045
        else ((value + 0.055) / 1.055) ** 2.4
    )


def _linear_to_srgb(value: float) -> float:
    value = max(0.0, min(1.0, value))
    return (
        value * 12.92
        if value <= 0.0031308
        else 1.055 * (value ** (1.0 / 2.4)) - 0.055
    )


def _bgr555_rgba(color: int) -> tuple[float, float, float, float]:
    return (
        _srgb_to_linear((color & 31) / 31.0),
        _srgb_to_linear(((color >> 5) & 31) / 31.0),
        _srgb_to_linear(((color >> 10) & 31) / 31.0),
        0.0 if color == 0 else 1.0,
    )


def _rgba_to_bgr555(pixel: tuple[float, float, float, float]) -> int:
    if pixel[3] < 0.5:
        return 0
    red = round(_linear_to_srgb(pixel[0]) * 31.0)
    green = round(_linear_to_srgb(pixel[1]) * 31.0)
    blue = round(_linear_to_srgb(pixel[2]) * 31.0)
    return 0x8000 | red | (green << 5) | (blue << 10)


def _nearest_palette(pixel: tuple[float, float, float, float], palette: tuple[int, ...]) -> int:
    return min(
        range(len(palette)),
        key=lambda index: (
            (
                pixel[0]
                - _srgb_to_linear((palette[index] & 31) / 31.0)
            )
            ** 2
            + (
                pixel[1]
                - _srgb_to_linear(
                    ((palette[index] >> 5) & 31) / 31.0
                )
            )
            ** 2
            + (
                pixel[2]
                - _srgb_to_linear(
                    ((palette[index] >> 10) & 31) / 31.0
                )
            )
            ** 2
        ),
    )


def _make_texture_image(
    vehicle: bpy.types.Collection,
    bank_name: str,
    index: int,
    texture: project.Texture,
) -> bpy.types.Image:
    image = bpy.data.images.new(
        f"{vehicle.name}.{bank_name}.texture_{index:03d}",
        width=texture.width,
        height=texture.height,
        alpha=True,
    )
    # Native banks may contain textures referenced only by animation or
    # runtime state. They still belong to the authored bank and must survive
    # saving even when no currently visible material node uses them.
    image.use_fake_user = True
    image.colorspace_settings.name = "Non-Color"
    image["v8_role"] = "authored_texture"
    image["v8_vehicle_collection"] = vehicle.name
    image["v8_bank"] = bank_name
    image["v8_texture_index"] = index
    image["v8_texture_name"] = texture.name
    image["v8_depth"] = texture.depth
    image["v8_compressed"] = texture.compressed
    image["v8_palette_bgr555"] = list(texture.palette_bgr555)
    image["v8_palette_indices"] = list(texture.indices)
    image["v8_palette_origin"] = list(texture.palette_origin)
    image["v8_image_origin"] = list(texture.image_origin)
    image["v8_direct_pixels_bgr555"] = list(
        texture.direct_pixels_bgr555
    )
    image["v8_storage"] = (
        "Direct 16-bit BGR555 pixels"
        if texture.depth == 2
        else (
            "4-bit indexed BGR555 palette"
            if texture.depth == 0
            else "8-bit indexed BGR555 palette"
        )
    )
    pixels = []
    for row in range(texture.height - 1, -1, -1):
        start = row * texture.width
        if texture.depth == 2:
            for color in texture.direct_pixels_bgr555[
                start : start + texture.width
            ]:
                pixels.extend(_bgr555_rgba(color))
        else:
            for palette_index in texture.indices[
                start : start + texture.width
            ]:
                pixels.extend(
                    _bgr555_rgba(texture.palette_bgr555[palette_index])
                )
    image.pixels.foreach_set(pixels)
    image.update()
    image.pack()
    return image


def _material(
    vehicle: bpy.types.Collection,
    bank_name: str,
    color: tuple[int, int, int],
    texture_index: int | None,
    images: dict[int, bpy.types.Image],
    environment_id: int | None = None,
    preview_pass: str = "SURFACE",
    paired_environment_id: int | None = None,
) -> bpy.types.Material:
    environment_names = {
        0x3FFE: "Dynamic Gloss Map",
        0x3FFF: "Dynamic Arena Reflection",
    }
    suffix = (
        f"env_{environment_id:04x}"
        if environment_id is not None
        else "flat"
        if texture_index is None
        else f"tex{texture_index}"
    )
    material = bpy.data.materials.new(
        f"{vehicle.name}.{bank_name}.{suffix}.{color[0]:02x}{color[1]:02x}{color[2]:02x}"
    )
    material["v8_role"] = "authored_face_material"
    material["v8_face_color"] = list(color)
    material["v8_texture_index"] = -1 if texture_index is None else texture_index
    material.diffuse_color = tuple(
        _srgb_to_linear(component / 255.0)
        for component in color
    ) + (1.0,)
    material["v8_environment_material_id"] = (
        -1 if environment_id is None else environment_id
    )
    material["v8_preview_pass"] = preview_pass
    material["v8_paired_environment_material_id"] = (
        -1
        if paired_environment_id is None
        else paired_environment_id
    )
    if environment_id is not None:
        material["v8_environment_material_name"] = environment_names.get(
            environment_id,
            f"Native Environment Texture {environment_id}",
        )
        material.use_nodes = True
        nodes = material.node_tree.nodes
        links = material.node_tree.links
        if preview_pass == "PAIRED_ENVIRONMENT":
            material.name = (
                f"{environment_names.get(environment_id, 'Environment Map')} "
                f"(0x{environment_id:04X}) — Paired Native Pass"
            )
            material[
                "v8_preview_explanation"
            ] = (
                "This exact native packet is preserved for export. Its "
                "coplanar surface partner carries the combined Blender "
                "preview so the two packets do not depth-fight."
            )
            output = nodes["Material Output"]
            shader = nodes.get("Principled BSDF")
            if shader is not None:
                nodes.remove(shader)
            transparent = nodes.new("ShaderNodeBsdfTransparent")
            links.new(transparent.outputs["BSDF"], output.inputs["Surface"])
            material.diffuse_color = (0.0, 0.0, 0.0, 0.0)
            material.surface_render_method = "BLENDED"
            return material

        material.name = (
            f"{environment_names.get(environment_id, 'Environment Map')} "
            f"(0x{environment_id:04X})"
        )
        shader = nodes["Principled BSDF"]
        shader.inputs["Base Color"].default_value = (0.0, 0.0, 0.0, 1.0)
        shader.inputs["Specular IOR Level"].default_value = 0.0
        if texture_index is not None:
            material["v8_texture_width"] = images[texture_index].size[0]
            material["v8_texture_height"] = images[texture_index].size[1]
            coordinates = nodes.new("ShaderNodeTexCoord")
            mapping = nodes.new("ShaderNodeMapping")
            mapping.inputs["Location"].default_value = (0.5, 0.5, 0.0)
            mapping.inputs["Scale"].default_value = (0.5, 0.5, 1.0)
            image_node = nodes.new("ShaderNodeTexImage")
            image_node.image = images[texture_index]
            image_node.interpolation = "Closest"
            links.new(coordinates.outputs["Normal"], mapping.inputs["Vector"])
            links.new(mapping.outputs["Vector"], image_node.inputs["Vector"])
            links.new(
                image_node.outputs["Color"],
                shader.inputs["Emission Color"],
            )
            shader.inputs["Emission Strength"].default_value = 1.0
        else:
            # 0x3FFE and 0x3FFF are engine-global, view-dependent maps rather
            # than missing vehicle textures. Their exact appearance depends
            # on the current arena, so the donor-free authoring view uses the
            # packet's own color as a neutral preview instead of inventing a
            # bright studio reflection.
            shader.inputs["Emission Color"].default_value = tuple(
                _srgb_to_linear(component / 255.0)
                for component in color
            ) + (1.0,)
            shader.inputs["Emission Strength"].default_value = 1.0
        shader.inputs["Metallic"].default_value = 0.0
        shader.inputs["Roughness"].default_value = 0.7
        shader.inputs["Specular IOR Level"].default_value = 0.0
    elif texture_index is not None:
        material.name = (
            f"Diffuse Texture {texture_index:03d}"
            + (
                " + Dynamic Gloss Preview"
                if paired_environment_id is not None
                else ""
            )
        )
        material["v8_texture_width"] = images[texture_index].size[0]
        material["v8_texture_height"] = images[texture_index].size[1]
        material.use_nodes = True
        nodes = material.node_tree.nodes
        image_node = nodes.new("ShaderNodeTexImage")
        image_node.image = images[texture_index]
        image_node.interpolation = "Closest"
        shader = nodes["Principled BSDF"]
        shader.inputs["Base Color"].default_value = (0.0, 0.0, 0.0, 1.0)
        shader.inputs["Specular IOR Level"].default_value = 0.0
        material.node_tree.links.new(
            image_node.outputs["Color"],
            shader.inputs["Emission Color"],
        )
        shader.inputs["Emission Strength"].default_value = 1.0
        material.node_tree.links.new(
            image_node.outputs["Alpha"], shader.inputs["Alpha"]
        )
        material.surface_render_method = "DITHERED"
        if paired_environment_id is not None:
            material[
                "v8_preview_explanation"
            ] = (
                "The paired native environment packet is preserved on its "
                "coplanar face; this surface carries the stable combined "
                "Blender preview."
            )
            shader.inputs["Metallic"].default_value = 0.0
            shader.inputs["Roughness"].default_value = 1.0
            shader.inputs["Specular IOR Level"].default_value = 0.0
    elif paired_environment_id is not None:
        material.name = "Native Body Color + Dynamic Gloss Preview"
        material[
            "v8_preview_explanation"
        ] = (
            "The paired native environment packet is preserved on its "
            "coplanar face; this surface carries the stable combined Blender "
            "preview."
        )
        material.use_nodes = True
        shader = material.node_tree.nodes["Principled BSDF"]
        shader.inputs["Base Color"].default_value = (0.0, 0.0, 0.0, 1.0)
        shader.inputs["Emission Color"].default_value = material.diffuse_color
        shader.inputs["Emission Strength"].default_value = 1.0
        shader.inputs["Metallic"].default_value = 0.0
        shader.inputs["Roughness"].default_value = 1.0
        shader.inputs["Specular IOR Level"].default_value = 0.0
    return material


def _face_int_attribute(
    mesh: bpy.types.Mesh, name: str, values: list[int]
) -> None:
    attribute = mesh.attributes.new(name=name, type="INT", domain="FACE")
    for index, value in enumerate(values):
        attribute.data[index].value = int(value)


def _face_int(mesh: bpy.types.Mesh, name: str, index: int, default: int) -> int:
    attribute = mesh.attributes.get(name)
    return default if attribute is None else int(attribute.data[index].value)


def _import_group(
    vehicle: bpy.types.Collection,
    bank_collection: bpy.types.Collection,
    bank_name: str,
    index: int,
    group: project.RenderGroup,
    images: dict[int, bpy.types.Image],
    display_name: str,
    target: bpy.types.Object | None = None,
) -> bpy.types.Object:
    scale = float(1 << group.scale_shift)
    mesh = bpy.data.meshes.new(
        f"{vehicle.name}.{bank_name}.{display_name}"
    )
    mesh.from_pydata(
        [_native_vector_to_blender(vertex, scale) for vertex in group.vertices],
        [],
        [face.vertices for face in group.faces],
    )
    mesh.update()
    loop_normals: list[Vector] = [
        Vector((0.0, 0.0, 1.0))
        for _loop in mesh.loops
    ]
    has_native_loop_normals = False
    for face_index, polygon in enumerate(mesh.polygons):
        face = group.faces[face_index]
        if not face.normal_indices:
            for loop_index in polygon.loop_indices:
                loop_normals[loop_index] = polygon.normal.copy()
            continue
        polygon.use_smooth = True
        has_native_loop_normals = True
        for corner, loop_index in enumerate(polygon.loop_indices):
            native_normal_index = face.normal_indices[
                corner if len(face.normal_indices) > 1 else 0
            ]
            if native_normal_index >= len(group.normals):
                raise ValueError(
                    f"{display_name} face {face_index} references native "
                    f"normal {native_normal_index}, but the table contains "
                    f"{len(group.normals)} entries"
                )
            native_normal = group.normals[native_normal_index]
            converted = Vector(
                _native_vector_to_blender(
                    (
                        native_normal[0],
                        native_normal[1],
                        native_normal[2],
                    ),
                    4096.0,
                )
            )
            if converted.length_squared == 0.0:
                converted = polygon.normal.copy()
            else:
                converted.normalize()
            loop_normals[loop_index] = converted
    if has_native_loop_normals:
        mesh.normals_split_custom_set(loop_normals)
    if target is None:
        obj = bpy.data.objects.new(display_name, mesh)
        obj["v8_role"] = "authored_render_group"
        bank_collection.objects.link(obj)
    else:
        obj = target
        obj.data = mesh
    obj["v8_group_index"] = index
    obj["v8_group_name"] = group.name
    obj["v8_scale_shift"] = group.scale_shift
    obj["v8_texture_slot_count"] = group.texture_slot_count
    obj["v8_render_extent"] = group.render_extent

    _face_int_attribute(
        mesh,
        "v8_packet_index",
        [
            -1 if face.packet_index is None else face.packet_index
            for face in group.faces
        ],
    )
    _face_int_attribute(
        mesh,
        "v8_packet_kind",
        [-1 if face.packet_kind is None else face.packet_kind for face in group.faces],
    )
    _face_int_attribute(
        mesh,
        "v8_native_texture_slot",
        [
            -1
            if face.native_texture_slot is None
            else face.native_texture_slot
            for face in group.faces
        ],
    )
    for field_name in (
        "packet_flags",
        "material_parameter",
        "texture_flags",
        "post_parameter",
    ):
        _face_int_attribute(
            mesh,
            f"v8_{field_name}",
            [int(getattr(face, field_name)) for face in group.faces],
        )
    _face_int_attribute(
        mesh,
        "v8_normal_count",
        [len(face.normal_indices) for face in group.faces],
    )
    _face_int_attribute(
        mesh,
        "v8_environment_count",
        [len(face.environment_parameters) for face in group.faces],
    )
    _face_int_attribute(
        mesh,
        "v8_gouraud_count",
        [len(face.gouraud_colors) for face in group.faces],
    )
    for item in range(3):
        _face_int_attribute(
            mesh,
            f"v8_normal_{item}",
            [
                face.normal_indices[item]
                if item < len(face.normal_indices)
                else 0
                for face in group.faces
            ],
        )
    for item in range(4):
        _face_int_attribute(
            mesh,
            f"v8_environment_{item}",
            [
                face.environment_parameters[item]
                if item < len(face.environment_parameters)
                else 0
                for face in group.faces
            ],
        )
    for item in range(2):
        for component, suffix in enumerate(("r", "g", "b")):
            _face_int_attribute(
                mesh,
                f"v8_gouraud_{item}_{suffix}",
                [
                    face.gouraud_colors[item][component]
                    if item < len(face.gouraud_colors)
                    else 0
                    for face in group.faces
                ],
            )

    if group.normals:
        normal_mesh = bpy.data.meshes.new(f"{mesh.name}.native_normals")
        normal_mesh.from_pydata(
            [
                _native_vector_to_blender(
                    (normal[0], normal[1], normal[2]), 4096.0
                )
                for normal in group.normals
            ],
            [],
            [],
        )
        pad_attribute = normal_mesh.attributes.new(
            name="v8_normal_pad", type="INT", domain="POINT"
        )
        for normal_index, normal in enumerate(group.normals):
            pad_attribute.data[normal_index].value = normal[3]
        # Native normals are ordered shading directions, not model-space
        # points. Keep them decoded and editable without drawing a point cloud.
        obj[NATIVE_NORMAL_TABLE_PROPERTY] = normal_mesh

    for control in group.controls:
        control_obj = bpy.data.objects.new(
            f"{display_name} — Render Control {control.packet_index:03d}",
            None,
        )
        control_obj["v8_role"] = "authored_render_control"
        control_obj["v8_packet_index"] = control.packet_index
        control_obj["v8_packet_kind"] = control.packet_kind
        control_obj["v8_packet_flags"] = control.packet_flags
        control_obj["v8_color"] = list(control.color)
        control_obj["v8_vertex_indices"] = list(control.vertex_indices)
        control_obj.parent = obj
        bank_collection.objects.link(control_obj)
        for entry_index, entry in enumerate(control.entries):
            entry_obj = bpy.data.objects.new(
                f"{control_obj.name}.entry_{entry_index:03d}", None
            )
            entry_obj["v8_role"] = "authored_render_control_entry"
            entry_obj["v8_entry_index"] = entry_index
            entry_obj["v8_values"] = list(entry)
            entry_obj.parent = control_obj
            bank_collection.objects.link(entry_obj)

    triangle_faces: dict[tuple[int, int, int], list[int]] = {}
    for face_index, face in enumerate(group.faces):
        triangle_faces.setdefault(
            tuple(sorted(face.vertices)), []
        ).append(face_index)
    paired_environment_faces: set[int] = set()
    paired_environment_by_surface: dict[int, int] = {}
    for face_indices in triangle_faces.values():
        for position, face_index in enumerate(face_indices):
            face = group.faces[face_index]
            if (
                face.packet_kind != 12
                or not face.environment_parameters
            ):
                continue
            surface_index = next(
                (
                    candidate
                    for candidate in face_indices[position + 1 :]
                    if group.faces[candidate].packet_kind != 12
                ),
                None,
            )
            if surface_index is None:
                continue
            paired_environment_faces.add(face_index)
            paired_environment_by_surface[surface_index] = (
                face.environment_parameters[0] & 0x3FFF
            )

    material_indices: dict[
        tuple[
            tuple[int, int, int],
            int | None,
            int | None,
            str,
            int | None,
        ],
        int,
    ] = {}
    uv_layer = mesh.uv_layers.new(name="V8 Native UV")
    for face_index, face in enumerate(group.faces):
        environment_id = (
            face.environment_parameters[0] & 0x3FFF
            if face.packet_kind == 12 and face.environment_parameters
            else None
        )
        preview_pass = (
            "PAIRED_ENVIRONMENT"
            if face_index in paired_environment_faces
            else "SURFACE"
        )
        paired_environment_id = paired_environment_by_surface.get(face_index)
        key = (
            face.color,
            face.texture,
            environment_id,
            preview_pass,
            paired_environment_id,
        )
        if key not in material_indices:
            material_indices[key] = len(mesh.materials)
            mesh.materials.append(
                _material(
                    vehicle,
                    bank_name,
                    face.color,
                    face.texture,
                    images,
                    environment_id,
                    preview_pass,
                    paired_environment_id,
                )
            )
        polygon = mesh.polygons[face_index]
        polygon.material_index = material_indices[key]
        # Blender can still depth-occlude a coplanar surface with a fully
        # transparent shader. Hide only the paired environment polygon in the
        # default authoring view; it remains a real editable polygon with all
        # native attributes and is still exported. Alt-H in Edit Mode reveals
        # it for advanced packet work.
        polygon.hide = face_index in paired_environment_faces
        denominator_u = (
            max(1, images[face.texture].size[0] - 1)
            if face.texture is not None
            else 255
        )
        denominator_v = (
            max(1, images[face.texture].size[1] - 1)
            if face.texture is not None
            else 255
        )
        for corner, loop_index in enumerate(polygon.loop_indices):
            u, v = face.uv[corner]
            uv_layer.data[loop_index].uv = (
                u / denominator_u,
                1.0 - v / denominator_v,
            )
    obj["v8_hidden_paired_environment_faces"] = len(
        paired_environment_faces
    )
    obj[
        "v8_hidden_paired_environment_explanation"
    ] = (
        "Native coplanar gloss packets are hidden only for stable Blender "
        "display. Enter Edit Mode and press Alt-H to reveal them; export "
        "always includes them."
    )
    return obj


def _spatial_region(position: tuple[int, int, int]) -> str:
    """Describe native placement without exposing an arbitrary slot index."""

    x, y, z = position
    lateral = (
        "Outer Left"
        if x < -32768
        else "Left"
        if x < -4096
        else "Outer Right"
        if x > 32768
        else "Right"
        if x > 4096
        else ""
    )
    longitudinal = (
        "Far Front"
        if z > 20000
        else "Front"
        if z > 8192
        else "Far Rear"
        if z < -20000
        else "Rear"
        if z < -8192
        else ""
    )
    if lateral or longitudinal:
        return " ".join(part for part in (lateral, longitudinal) if part)
    return "Upper Center" if y < -4096 else "Lower Center" if y > 4096 else "Center"


def _slot_display_names(
    groups: tuple[project.RenderGroup, ...],
    slots: tuple[project.Slot, ...],
    bank_name: str,
    body_root: int | None,
    transform_roles: dict[int, str],
) -> tuple[tuple[str, str, str], ...]:
    """Name native objects by the engine behavior that consumes them."""

    children: dict[int, list[int]] = {}
    for index, slot in enumerate(slots):
        if slot.parent is not None:
            children.setdefault(slot.parent, []).append(index)

    def ancestors(index: int):
        parent = slots[index].parent
        while parent is not None:
            yield parent
            parent = slots[parent].parent

    def has_ancestor_key(index: int, key: int) -> bool:
        return any(slots[parent].key == key for parent in ancestors(index))

    def render_class(slot: project.Slot) -> int:
        return slot.render_flags & 0xF000

    def object_id(slot: project.Slot) -> int:
        return slot.flags & 0xFFFF

    def group_longitudinal_region(slot: project.Slot) -> str:
        if slot.render_group is None or not 0 <= slot.render_group < len(groups):
            return _spatial_region(slot.position)
        group = groups[slot.render_group]
        if not group.vertices:
            return _spatial_region(slot.position)
        center_z = (
            slot.position[2]
            + round(
                sum(vertex[2] for vertex in group.vertices)
                / len(group.vertices)
            )
            * (1 << group.scale_shift)
        )
        if center_z > 8192:
            return "Front"
        if center_z < -8192:
            return "Rear"
        return "Center"

    direct_body_children = (
        set(children.get(body_root, ())) if body_root is not None else set()
    )
    body_sections = {
        index: group_longitudinal_region(slot)
        for index, slot in enumerate(slots)
        if index in direct_body_children
        and slot.key is None
        and slot.render_group is not None
        and render_class(slot) == 0
        and object_id(slot) < 4
    }

    def body_section_ancestor(index: int) -> int | None:
        if index in body_sections:
            return index
        return next(
            (parent for parent in ancestors(index) if parent in body_sections),
            None,
        )

    def damage_stage(index: int, section: int) -> int:
        stage = 0
        cursor: int | None = index
        while cursor is not None and cursor != section:
            if render_class(slots[cursor]) == 0xF000:
                stage += 1
            cursor = slots[cursor].parent
        return stage

    def destruction_sequence_ancestor(index: int) -> int | None:
        candidates = (index, *ancestors(index))
        return next(
            (
                candidate
                for candidate in candidates
                if slots[candidate].parent == body_root
                and render_class(slots[candidate]) == 0xF000
            ),
            None,
        )

    weapon_mounts = {
        0x8010: "Machine Gun Mount",
        0x8011: "Rocket Launcher Mount",
        0x8012: "Missile Launcher Mount",
        0x8013: "Cannon Mount",
        0x8014: "Mortar Mount",
        0x8015: "Mine Dripper Mount",
        0x8016: "Flamethrower Mount",
    }
    upgrade_models = {
        0x100: (
            "Acceleration Upgrade Appearance",
            "acceleration_upgrade_model",
        ),
        0x101: (
            "Top-Speed Upgrade Appearance",
            "top_speed_upgrade_model",
        ),
        0x102: (
            "Armor Upgrade Appearance",
            "armor_upgrade_model",
        ),
        0x103: (
            "Handling Upgrade Appearance",
            "handling_upgrade_model",
        ),
    }
    stage_names = {
        1: "Damaged",
        2: "Heavily Damaged",
        3: "Destroyed",
    }

    result = []
    for index, slot in enumerate(slots):
        direct_body_child = index in direct_body_children
        region = _spatial_region(slot.position)
        section = body_section_ancestor(index)
        destruction = destruction_sequence_ancestor(index)
        special_assembly = (
            slot.key == 0x801F or has_ancestor_key(index, 0x801F)
        )

        if bank_name == "transformation" and slot.parent is None:
            role = transform_roles.get(index)
            if role is not None:
                result.append((role, "transformation_wheel_root", "HIGH"))
            else:
                result.append(
                    (
                        "Unused Transformation Wheel Object",
                        "transformation_library_root",
                        "HIGH",
                    )
                )
            continue

        if bank_name == "transformation":
            root_index = index
            depth = 0
            while slots[root_index].parent is not None:
                root_index = slots[root_index].parent
                depth += 1
            root_name = transform_roles.get(
                root_index, "Unused Transformation Wheel Object"
            )
            native_class = render_class(slot)
            structural_part = (
                "Distance LOD"
                if native_class == 0xC000 and depth == 1
                else f"Child Model - {region} - Distance LOD"
                if native_class == 0xC000
                else f"Attached Model - {region}"
                if depth == 1 and slot.render_group is not None
                else "Placement"
                if depth == 1
                else f"Child Model - {region}"
                if slot.render_group is not None
                else f"Child Placement - {region}"
            )
            result.append(
                (
                    f"{root_name} - {structural_part}",
                    (
                        "render_part"
                        if slot.render_group is not None
                        else "transform_node"
                    ),
                    "HIGH",
                )
            )
            continue

        if body_root is not None and index == body_root:
            result.append(("Vehicle Root", "vehicle_root", "HIGH"))
            continue

        if slot.key is not None:
            key = slot.key
            if direct_body_child and 0x8000 <= key <= 0x8005:
                wheel = key - 0x8000
                axle = (
                    "Front"
                    if wheel < 2
                    else "Rear"
                    if wheel < 4
                    else "Additional Rear"
                )
                side = "Left" if wheel % 2 == 0 else "Right"
                result.append(
                    (
                        f"{axle} {side} Wheel Anchor",
                        "wheel_anchor",
                        "HIGH" if wheel < 4 else "MED",
                    )
                )
                continue
            if direct_body_child and key in weapon_mounts:
                result.append((weapon_mounts[key], "weapon_mount", "HIGH"))
                continue
            if direct_body_child and key == 0x801F:
                result.append(("Special Weapon Mount", "weapon_mount", "HIGH"))
                continue
            light_names = {
                0x8040: ("Left Headlight Emitter", "headlight_emitter"),
                0x8041: ("Right Headlight Emitter", "headlight_emitter"),
                0x8042: ("Left Tail-Light Emitter", "tail_light_emitter"),
                0x8043: ("Right Tail-Light Emitter", "tail_light_emitter"),
            }
            if key in light_names:
                name, role = light_names[key]
                result.append((name, role, "HIGH"))
                continue
            if direct_body_child and key == 0x8101:
                result.append(
                    (
                        "Vehicle Destruction Effect Anchor",
                        "destruction_effect_anchor",
                        "HIGH",
                    )
                )
                continue
            if direct_body_child and key == 0x8100:
                result.append(
                    (
                        "Auxiliary Powered Attachment Anchor",
                        "auxiliary_powered_attachment",
                        "MED",
                    )
                )
                continue
            if special_assembly:
                pivot = "Forward Pivot" if slot.position[2] > 0 else "Rear Pivot"
                result.append(
                    (
                        f"Special Weapon Assembly - {pivot}",
                        "special_weapon_anchor",
                        "HIGH",
                    )
                )
                continue
            if destruction is not None or section is not None:
                if 0x8400 <= key <= 0x843C:
                    kind = (
                        "Colliding Debris Burst Emitter"
                        if slot.collision is not None
                        else "Debris Burst Emitter"
                    )
                elif 0x8500 <= key <= 0x8514:
                    kind = "Damage Particle Emitter"
                elif 0x8700 <= key <= 0x8714:
                    kind = "Damage Spark Emitter"
                elif 0x8800 <= key <= 0x8807:
                    kind = "Colored Damage Flash Emitter"
                elif key >> 12 == 9:
                    kind = "Damage Sound Emitter"
                else:
                    kind = "Damage Effect Anchor"
                if section is not None:
                    stage = damage_stage(index, section)
                    owner = (
                        f"{body_sections[section]} Body "
                        f"{stage_names.get(stage, 'Damage')}"
                    )
                else:
                    owner = "Vehicle Destruction"
                result.append(
                    (
                        f"{owner} - {kind} - {region}",
                        "damage_effect_anchor",
                        "HIGH",
                    )
                )
                continue
            result.append(
                (
                    f"Auxiliary Attachment Anchor - {region}",
                    "auxiliary_attachment",
                    "MED",
                )
            )
            continue

        native_class = render_class(slot)
        if index in body_sections:
            result.append(
                (
                    f"{body_sections[index]} Body - Intact",
                    "body",
                    "HIGH",
                )
            )
            continue
        if direct_body_child and native_class == 0xB000:
            result.append(
                (
                    "HUD Vehicle Icon Texture Source",
                    "hud_vehicle_icon_source",
                    "MED",
                )
            )
            continue
        if direct_body_child and native_class == 0xC000:
            result.append(
                ("Vehicle Distance LOD", "distance_lod_model", "HIGH")
            )
            continue
        if direct_body_child and object_id(slot) in upgrade_models:
            name, role = upgrade_models[object_id(slot)]
            result.append((name, role, "HIGH"))
            continue
        if direct_body_child and native_class == 0xF000:
            result.append(
                (
                    "Vehicle Destruction Sequence",
                    "vehicle_destruction_sequence",
                    "HIGH",
                )
            )
            continue
        if section is not None and native_class == 0xF000:
            stage = damage_stage(index, section)
            result.append(
                (
                    f"{body_sections[section]} Body - "
                    f"{stage_names.get(stage, 'Damage Replacement')}",
                    "body_damage_stage",
                    "HIGH",
                )
            )
            continue
        if special_assembly:
            depth = sum(1 for _parent in ancestors(index))
            mount_depth = next(
                (
                    sum(1 for _parent in ancestors(parent))
                    for parent in ancestors(index)
                    if slots[parent].key == 0x801F
                ),
                0,
            )
            relative_depth = max(1, depth - mount_depth)
            assembly_names = {
                1: "Special Weapon Assembly - Base",
                2: "Special Weapon Assembly - Articulated Section",
                3: "Special Weapon Assembly - Emitter",
            }
            result.append(
                (
                    assembly_names.get(
                        relative_depth,
                        f"Special Weapon Assembly - {region}",
                    ),
                    "special_weapon_model",
                    "HIGH",
                )
            )
            continue
        if destruction is not None:
            if native_class == 0xD000:
                name = f"Physical Destruction Debris - {region}"
                role = "physical_destruction_debris"
            elif native_class == 0xE000:
                name = f"Animated Destruction Particle - {region}"
                role = "destruction_particle_model"
            elif slot.render_group is not None:
                name = f"Vehicle Destruction Debris - {region}"
                role = "destruction_debris_model"
            elif slot.collision is not None:
                name = f"Vehicle Destruction Collision - {region}"
                role = "destruction_collision"
            else:
                name = f"Vehicle Destruction Effect Origin - {region}"
                role = "damage_effect_anchor"
            result.append((name, role, "HIGH"))
            continue
        if section is not None:
            stage = damage_stage(index, section)
            context = (
                f"{body_sections[section]} Body "
                f"{stage_names.get(stage, 'Damage')}"
            )
            if slot.render_group is not None:
                name = f"{context} Detachable Debris - {region}"
                role = "body_damage_debris"
            elif slot.collision is not None:
                name = f"{context} Collision - {region}"
                role = "body_damage_collision"
            else:
                name = f"{context} Effect Origin - {region}"
                role = "damage_effect_anchor"
            result.append((name, role, "HIGH"))
            continue
        if slot.render_group is not None and slot.collision is not None:
            result.append(
                (
                    f"Attached Physical Model - {region}",
                    "render_collision_part",
                    "MED",
                )
            )
        elif slot.render_group is not None:
            result.append(
                (f"Attached Model - {region}", "render_part", "MED")
            )
        elif slot.collision is not None:
            result.append(
                (f"Collision Volume - {region}", "collision_part", "HIGH")
            )
        else:
            result.append(
                (f"Placement Anchor - {region}", "transform_node", "HIGH")
            )
    return tuple(result)


def _transformation_roles(
    vehicle_collection: bpy.types.Collection,
) -> dict[int, tuple[str, tuple[str, ...]]]:
    assignments: dict[int, list[tuple[str, str]]] = {}
    standard_fields = (
        ("wheel_kind_front", "Front Axle"),
        ("wheel_kind_rear", "Middle/Rear Axles"),
    )
    for field_name, axle_name in standard_fields:
        root = int(vehicle_collection[f"v8_stat_{field_name}"])
        assignments.setdefault(root, []).append(("Standard", axle_name))
    if bool(vehicle_collection.get("v8_supports_transformations", True)):
        for mode_index in range(1, project.V82_TRANSFORM_MODE_COUNT):
            for wheel in range(project.V82_TRANSFORM_WHEEL_COUNT):
                root = int(
                    vehicle_collection[f"v8_transform_{mode_index}_{wheel}"]
                )
                assignments.setdefault(root, []).append(
                    (
                        TRANSFORM_MODE_NAMES[mode_index],
                        TRANSFORM_WHEEL_NAMES[wheel],
                    )
                )
    result = {}
    for root, uses in assignments.items():
        modes = {mode for mode, _wheel in uses}
        if len(uses) == 1:
            mode, wheel = uses[0]
            name = f"{mode} - {wheel} Wheel"
        elif len(modes) == 1:
            mode = uses[0][0]
            wheel_names = [wheel for _mode, wheel in uses]
            wheel_sides = {
                wheel.rsplit(" ", 1)[-1] for wheel in wheel_names
            }
            if wheel_sides <= {"Left", "Right"} and len(wheel_sides) == 1:
                side = next(iter(wheel_sides))
                positions = "/".join(
                    wheel.rsplit(" ", 1)[0] for wheel in wheel_names
                )
                name = f"{mode} - {side} Wheels ({positions})"
            else:
                name = f"{mode} - {'/'.join(wheel_names)}"
        else:
            detail = "; ".join(f"{mode} {wheel}" for mode, wheel in uses)
            name = f"Shared Wheel Form - {detail}"
        result[root] = (
            name,
            tuple(f"{mode}: {wheel}" for mode, wheel in uses),
        )
    return result


def refresh_transformation_names(
    vehicle_collection: bpy.types.Collection,
) -> None:
    roles = _transformation_roles(vehicle_collection)
    bank = next(
        (
            child
            for child in vehicle_collection.children
            if child.get("v8_bank") == "transformation"
        ),
        None,
    )
    if bank is None:
        return
    for obj in bank.all_objects:
        if (
            obj.get("v8_role") != "authored_slot"
            or obj.parent is not None
        ):
            continue
        slot_index = int(obj["v8_slot_index"])
        name, assignments = roles.get(
            slot_index,
            ("Unused Transformation Wheel Object", ()),
        )
        obj.name = name
        obj["v8_transform_assignment_summary"] = (
            "Not assigned to a transformation mode"
            if not assignments
            else "; ".join(assignments)
        )


def update_transformation_preview(
    vehicle_collection: bpy.types.Collection,
    mode_name: str = "STANDARD",
) -> None:
    """Build non-exported, non-instanced wheel geometry at body anchors."""

    for obj in tuple(vehicle_collection.all_objects):
        if obj.get("v8_role") == ROLE_TRANSFORMATION_PREVIEW:
            bpy.data.objects.remove(obj, do_unlink=True)
    if vehicle_collection.get("v8_game") != "V8_2" or mode_name == "NONE":
        return

    body_bank = next(
        (
            child
            for child in vehicle_collection.children
            if child.get("v8_bank") == "body"
        ),
        None,
    )
    transform_bank = next(
        (
            child
            for child in vehicle_collection.children
            if child.get("v8_bank") == "transformation"
        ),
        None,
    )
    if body_bank is None or transform_bank is None:
        return

    if mode_name == "STANDARD":
        front = int(vehicle_collection["v8_stat_wheel_kind_front"])
        rear = int(vehicle_collection["v8_stat_wheel_kind_rear"])
        roots = (front, front, rear, rear, rear, rear)
    else:
        if not bool(
            vehicle_collection.get("v8_supports_transformations", True)
        ):
            return
        mode_index = {"HOVER": 1, "FLOAT": 2, "SKI": 3}.get(mode_name)
        if mode_index is None:
            return
        roots = tuple(
            int(vehicle_collection[f"v8_transform_{mode_index}_{wheel}"])
            for wheel in range(project.V82_TRANSFORM_WHEEL_COUNT)
        )

    source_roots = {
        int(obj["v8_slot_index"]): obj
        for obj in transform_bank.all_objects
        if (
            obj.get("v8_role") == "authored_slot"
            and obj.parent is None
        )
    }
    anchors = sorted(
        (
            obj
            for obj in body_bank.all_objects
            if (
                obj.get("v8_role") == "authored_slot"
                and obj.get("v8_part_role") == "wheel_anchor"
                and 0x8000 <= int(obj.get("v8_attachment_key", -1)) <= 0x8005
            )
        ),
        key=lambda obj: int(obj["v8_attachment_key"]),
    )
    vehicle_root = next(
        (
            obj
            for obj in body_bank.all_objects
            if (
                obj.get("v8_role") == "authored_slot"
                and obj.get("v8_part_role") == "vehicle_root"
            )
        ),
        None,
    )
    for anchor in anchors:
        wheel_index = int(anchor["v8_attachment_key"]) - 0x8000
        if wheel_index >= len(roots):
            continue
        source_root = source_roots.get(roots[wheel_index])
        if source_root is None:
            continue
        # V8:2 0x8003E4A8 selects this root but copies only native rotation
        # fields +0x10..+0x14 into the live wheel object. It does not apply the
        # library root's +0x04..+0x0C position. Compose from matrix_basis
        # explicitly: the raw transformation bank is hidden by default, and
        # Blender can leave matrix_world stale for objects in a hidden bank.
        # Standard mode writes native vr.z=2048 for odd/right wheel indices;
        # native Z rotation maps to Blender Y after the coordinate conversion.
        side_rotation = (
            Matrix.Rotation(3.141592653589793, 4, "Y")
            if mode_name == "STANDARD" and wheel_index % 2 == 1
            else Matrix.Identity(4)
        )
        pivot_correction = Matrix.Identity(4)
        if mode_name != "STANDARD" and vehicle_root is not None:
            standard_root_index = int(
                vehicle_collection[
                    "v8_stat_wheel_kind_front"
                    if wheel_index < 2
                    else "v8_stat_wheel_kind_rear"
                ]
            )
            standard_root = source_roots.get(standard_root_index)
            if standard_root is not None:
                # At the transformation midpoint, FUN_3E4A8 places the
                # temporary wheel pivot at DAT_E4 - physics2.X. For the
                # stock/global path those values are -vehicle_root.y and
                # -standard_wheel_root.y respectively. Preserve anchor X/Z,
                # and reproduce that native Y correction exactly.
                root_y = _blender_vector_to_native(
                    vehicle_root.location, FIXED
                )[1]
                standard_y = _blender_vector_to_native(
                    standard_root.location, FIXED
                )[1]
                anchor_y = _blender_vector_to_native(
                    anchor.location, FIXED
                )[1]
                mounted_y = -root_y + standard_y
                pivot_correction = Matrix.Translation(
                    (0.0, 0.0, -(mounted_y - anchor_y) / FIXED)
                )

        def runtime_part_matrix(source: bpy.types.Object) -> Matrix:
            """Compose the exact mounted hierarchy without the library offset."""

            chain = []
            cursor = source
            while cursor is not None and cursor is not source_root:
                chain.append(cursor)
                cursor = cursor.parent
            if cursor is None:
                return Matrix.Identity(4)

            if mode_name == "STANDARD":
                result = side_rotation.copy()
            else:
                result = (
                    pivot_correction
                    @ source_root.matrix_basis.to_3x3().to_4x4()
                )
            for child in reversed(chain):
                result = result @ child.matrix_basis
            return result

        def runtime_constructs_mesh(obj: bpy.types.Object) -> bool:
            """Match the wheel-specific 0x8002C17C construction walk."""

            if obj.type != "MESH" or obj.get("v8_role") != "authored_slot":
                return False
            cursor = obj
            while True:
                # ConfigContainer.flag is tested as signed.  Negative records
                # (including 0xC000 LOD descriptors) are traversal controls,
                # not simultaneously rendered wheel pieces.
                if int(cursor.get("v8_render_flags", 0)) & 0x8000:
                    return False
                if cursor is source_root:
                    return True
                # Wheel.FUN_2C344 requests descendants with flags 0x29.
                # 0x8002C17C then skips child objIDs above 255.  The retail
                # standard wheel's apparent "second wheel" is objID 0x0100,
                # so displaying it produced the false center-wheel pair.
                object_id = int(
                    cursor.get("v8_native_object_id", -0x5556)
                )
                # The engine casts ConfigContainer.objID to signed short
                # before comparing it with 255.  In particular, the native
                # "unassigned" value 0xAAAA is -21846 and is therefore
                # constructible.  Treating it as unsigned discards the real
                # hover/float/ski subassemblies.  A genuine positive 0x0100
                # role ID is still skipped, as it is by the retail engine.
                object_id = ((object_id + 0x8000) & 0xFFFF) - 0x8000
                if object_id > 0xFF:
                    return False
                cursor = cursor.parent
                if cursor is None:
                    return False

        source_meshes = [
            obj
            for obj in (source_root, *source_root.children_recursive)
            if runtime_constructs_mesh(obj)
        ]
        for part_index, source in enumerate(source_meshes):
            preview = source.copy()
            preview.data = source.data.copy()
            preview.name = (
                f"{mode_name.title()} Preview — {anchor.name}"
                if len(source_meshes) == 1
                else (
                    f"{mode_name.title()} Preview — {anchor.name} — "
                    f"{source.name}"
                )
            )
            preview["v8_role"] = ROLE_TRANSFORMATION_PREVIEW
            preview["v8_preview_mode"] = mode_name
            preview["v8_preview_wheel"] = wheel_index
            preview["v8_preview_source"] = source.name
            for key in (
                "v8_slot_index",
                "v8_group_index",
                "v8_render_group",
                "v8_collision",
                "v8_part_role",
            ):
                if key in preview:
                    del preview[key]
            preview.parent = anchor
            preview.matrix_parent_inverse.identity()
            preview.matrix_basis = runtime_part_matrix(source)
            preview.hide_select = True
            preview.show_in_front = False
            body_bank.objects.link(preview)


def _model_display_names(
    groups: tuple[project.RenderGroup, ...],
    slots: tuple[project.Slot, ...],
    semantics: tuple[tuple[str, str, str], ...],
    bank_name: str,
) -> tuple[str, ...]:
    priority = {
        "vehicle_root": 0,
        "wheel_anchor": 1,
        "transformation_wheel_root": 1,
        "headlight_emitter": 2,
        "tail_light_emitter": 2,
        "destruction_effect_anchor": 2,
        "render_collision_part": 3,
        "render_part": 4,
        "nested_engine_attachment": 5,
        "transform_node": 6,
    }
    names = []
    for group_index in range(len(groups)):
        references = [
            (slot_index, semantics[slot_index])
            for slot_index, slot in enumerate(slots)
            if slot.render_group == group_index
        ]
        if references:
            slot_index, (slot_name, role, _confidence) = min(
                references,
                key=lambda item: (
                    priority.get(item[1][1], 99),
                    item[0],
                ),
            )
            if role == "vehicle_root":
                base = "Vehicle Body Shell"
            elif role == "wheel_anchor":
                base = slot_name.replace(" Anchor", " Model")
            elif role == "transformation_wheel_root":
                base = slot_name.replace(" Root ", " Model ")
            elif role.endswith("_emitter"):
                base = slot_name.replace(" Emitter", " Housing")
            elif role == "destruction_effect_anchor":
                base = "Destruction Model"
            else:
                base = slot_name
            if len(references) > 1:
                base += f" — Shared by {len(references)} Parts"
        elif bank_name == "body" and group_index == 0:
            base = "Primary Vehicle Body Model"
        elif bank_name == "transformation":
            base = "Transformation Library Model"
        else:
            base = "Unassigned Vehicle Model"
        names.append(base)
    return tuple(names)


def _collision_display_names(
    collisions: tuple[project.CollisionStream, ...],
    slots: tuple[project.Slot, ...],
    semantics: tuple[tuple[str, str, str], ...],
) -> tuple[str, ...]:
    names = []
    for collision_index in range(len(collisions)):
        references = [
            (slot_index, semantics[slot_index])
            for slot_index, slot in enumerate(slots)
            if slot.collision == collision_index
        ]
        if references:
            slot_index, (slot_name, role, _confidence) = min(
                references,
                key=lambda item: (
                    0 if item[1][1] == "vehicle_root" else 1,
                    item[0],
                ),
            )
            if role == "vehicle_root":
                base = "Vehicle Body Collision"
            elif role == "wheel_anchor":
                base = slot_name.replace(" Anchor", " Collision")
            else:
                base = f"{slot_name} Collision"
        else:
            base = "Unassigned Collision"
        names.append(base)
    return tuple(names)


def _import_slots(
    bank_collection: bpy.types.Collection,
    slots: tuple[project.Slot, ...],
    semantics: tuple[tuple[str, str, str], ...],
) -> tuple[bpy.types.Object, ...]:
    objects = []
    for index, slot in enumerate(slots):
        display_name, part_role, confidence = semantics[index]
        placeholder_mesh = (
            bpy.data.meshes.new(f"{display_name}.native_model")
            if slot.render_group is not None
            else None
        )
        obj = bpy.data.objects.new(display_name, placeholder_mesh)
        if slot.render_group is not None:
            pass
        elif part_role == "wheel_anchor":
            obj.empty_display_type = "CIRCLE"
        elif part_role == "weapon_mount":
            obj.empty_display_type = "SINGLE_ARROW"
        elif part_role in {"headlight_emitter", "tail_light_emitter"}:
            obj.empty_display_type = "SPHERE"
        elif part_role in {
            "transformation_wheel_root",
            "transformation_library_root",
        }:
            obj.empty_display_type = "ARROWS"
        elif part_role in {
            "destruction_effect_anchor",
            "auxiliary_powered_attachment",
        }:
            obj.empty_display_type = "CUBE"
        else:
            obj.empty_display_type = "PLAIN_AXES"
        obj.empty_display_size = 0.08
        obj.show_in_front = False
        obj.show_name = False
        obj["v8_role"] = "authored_slot"
        obj["v8_slot_index"] = index
        obj["v8_slot_name"] = slot.name
        obj["v8_render_group"] = -1 if slot.render_group is None else slot.render_group
        obj["v8_render_flags"] = slot.render_flags
        obj["v8_collision"] = -1 if slot.collision is None else slot.collision
        obj["v8_attachment_key"] = -1 if slot.key is None else slot.key
        obj["v8_native_key_hex"] = (
            "" if slot.key is None else f"0x{slot.key:04X}"
        )
        obj["v8_part_role"] = part_role
        if part_role == "body":
            obj["v8_damage_state"] = "INTACT"
        elif part_role == "body_damage_stage":
            lowered_name = display_name.lower()
            obj["v8_damage_state"] = (
                "DESTROYED"
                if "destroyed" in lowered_name
                else "HEAVILY_DAMAGED"
                if "heavily damaged" in lowered_name
                else "DAMAGED"
            )
        elif part_role == "vehicle_destruction_sequence":
            obj["v8_damage_state"] = "DESTROYED"
        obj["v8_part_name_confidence"] = confidence
        obj["v8_part_name_source"] = (
            "Derived from native key, object class, hierarchy, and engine use"
            if slot.key is not None
            else "Derived from native object class, hierarchy, and engine use"
        )
        obj["v8_native_object_id"] = slot.flags
        obj["v8_native_object_id_meaning"] = (
            "No object ID assigned"
            if slot.flags & 0xFFFF == 0xAAAA
            else "Engine role ID; reflected in the semantic object name"
        )
        obj["v8_native_object_class"] = (
            "Attachment/effect key"
            if slot.key is not None
            else {
                0x0000: "Ordinary model or placement",
                0xB000: "HUD vehicle-icon texture source",
                0xC000: "Distance LOD model",
                0xD000: "Physical debris model",
                0xE000: "Animated particle model",
                0xF000: "Damage replacement",
            }.get(
                slot.render_flags & 0xF000,
                "Engine-special model",
            )
        )
        obj.location = _native_vector_to_blender(slot.position, FIXED)
        obj.rotation_mode = "XYZ"
        obj.rotation_euler = _native_rotation_to_blender(slot.rotation_yxz)
        bank_collection.objects.link(obj)
        objects.append(obj)
    for index, slot in enumerate(slots):
        if slot.parent is not None:
            objects[index].parent = objects[slot.parent]
            objects[index].matrix_parent_inverse.identity()
    return tuple(objects)


def _import_collisions(
    bank_collection: bpy.types.Collection,
    collisions: tuple[project.CollisionStream, ...],
    display_names: tuple[str, ...],
    slots: tuple[project.Slot, ...],
    slot_objects: tuple[bpy.types.Object, ...],
) -> None:
    bpy.context.view_layer.update()
    for stream_index, stream in enumerate(collisions):
        display_name = display_names[stream_index]
        root = bpy.data.objects.new(display_name, None)
        root["v8_role"] = "authored_collision_stream"
        root["v8_collision_index"] = stream_index
        root["v8_collision_name"] = stream.name
        bank_collection.objects.link(root)
        references = [
            slot_index
            for slot_index, slot in enumerate(slots)
            if slot.collision == stream_index
        ]
        if references:
            root.parent = slot_objects[references[0]]
            root.matrix_parent_inverse.identity()
            root["v8_primary_slot"] = references[0]
            root["v8_referencing_slots"] = references
        for shape_index, shape in enumerate(stream.shapes):
            shape_name = (
                "AABB"
                if isinstance(shape, project.CollisionAabb)
                else "Convex Shape"
            )
            obj = bpy.data.objects.new(
                f"{display_name} — {shape_name} {shape_index:03d}",
                None,
            )
            obj["v8_role"] = "authored_collision_shape"
            obj["v8_shape_index"] = shape_index
            obj.parent = root
            bank_collection.objects.link(obj)
            if isinstance(shape, project.CollisionAabb):
                obj["v8_shape_kind"] = "aabb"
                obj["v8_minimum"] = list(shape.minimum)
                obj["v8_maximum"] = list(shape.maximum)
                obj["v8_override_y"] = shape.override_y
                obj["v8_collision_flags"] = shape.flags
                obj.empty_display_type = "CUBE"
                native_center = tuple(
                    (shape.minimum[axis] + shape.maximum[axis]) / 2.0
                    for axis in range(3)
                )
                native_half = tuple(
                    (shape.maximum[axis] - shape.minimum[axis]) / 2.0
                    for axis in range(3)
                )
                obj.location = _native_vector_to_blender(
                    native_center, FIXED)
                obj.scale = (
                    native_half[0] / FIXED,
                    native_half[2] / FIXED,
                    native_half[1] / FIXED,
                )
            else:
                obj["v8_shape_kind"] = "convex"
                for plane_index, plane in enumerate(shape.planes):
                    child = bpy.data.objects.new(
                        f"{obj.name} — Plane {plane_index:03d}", None
                    )
                    child["v8_role"] = "authored_collision_plane"
                    child["v8_plane_index"] = plane_index
                    child["v8_normal"] = list(plane.normal)
                    child["v8_offset"] = plane.offset
                    child["v8_pad"] = plane.pad
                    child.parent = obj
                    bank_collection.objects.link(child)
        if references:
            bounds = _native_subtree_bounds(slot_objects[references[0]])
            if bounds is not None:
                root["v8_auto_mesh_minimum"] = list(bounds[0])
                root["v8_auto_mesh_maximum"] = list(bounds[1])


def _native_subtree_bounds(
    owner: bpy.types.Object,
) -> tuple[tuple[int, int, int], tuple[int, int, int]] | None:
    """Return intact render bounds in the collision owner's local space."""

    candidates = _collision_mesh_candidates(owner)
    if not candidates:
        return None

    inverse = owner.matrix_world.inverted_safe()
    native_points = []
    for candidate in candidates:
        relative = inverse @ candidate.matrix_world
        native_points.extend(
            _blender_vector_to_native(relative @ vertex.co, FIXED)
            for vertex in candidate.data.vertices
        )
    if not native_points:
        return None
    minimum = tuple(min(point[axis] for point in native_points) for axis in range(3))
    maximum = tuple(max(point[axis] for point in native_points) for axis in range(3))
    return minimum, maximum


def _slot_subtree(owner: bpy.types.Object) -> list[bpy.types.Object]:
    result = []
    stack = [owner]
    while stack:
        current = stack.pop()
        result.append(current)
        stack.extend(
            child
            for child in current.children
            if child.get("v8_role") == "authored_slot"
        )
    return result


def _collision_mesh_candidates(
    owner: bpy.types.Object,
) -> list[bpy.types.Object]:
    candidates = []
    for current in _slot_subtree(owner):
        if (
            current.type != "MESH"
            or current.get("v8_role") != "authored_slot"
        ):
            continue
        part_role = str(current.get("v8_part_role", ""))
        if part_role in ADVANCED_MODEL_ROLES:
            continue
        ancestor = current
        damage_state = None
        while (
            ancestor is not None
            and ancestor.get("v8_role") == "authored_slot"
        ):
            if ancestor.get("v8_damage_state"):
                damage_state = str(ancestor["v8_damage_state"])
                break
            ancestor = ancestor.parent
        if damage_state not in {None, "INTACT"}:
            continue
        candidates.append(current)
    return candidates


def _local_mesh_bounds(
    obj: bpy.types.Object,
) -> tuple[tuple[int, int, int], tuple[int, int, int]] | None:
    points = [
        _blender_vector_to_native(vertex.co, FIXED)
        for vertex in obj.data.vertices
    ]
    if not points:
        return None
    return (
        tuple(min(point[axis] for point in points) for axis in range(3)),
        tuple(max(point[axis] for point in points) for axis in range(3)),
    )


def _collision_subtree_changed(owner: bpy.types.Object) -> bool:
    for obj in _slot_subtree(owner):
        if _prop_tuple(obj, "v8_auto_source_position", 3) != (
            _blender_vector_to_native(obj.location, FIXED)
        ):
            return True
        if _prop_tuple(obj, "v8_auto_source_rotation", 3) != (
            _blender_rotation_to_native(obj.rotation_euler)
        ):
            return True
    for obj in _collision_mesh_candidates(owner):
        bounds = _local_mesh_bounds(obj)
        if bounds is None:
            continue
        if (
            _prop_tuple(obj, "v8_auto_source_mesh_minimum", 3) != bounds[0]
            or _prop_tuple(obj, "v8_auto_source_mesh_maximum", 3) != bounds[1]
        ):
            return True
    return False


def _remove_object_tree(obj: bpy.types.Object) -> None:
    for child in tuple(obj.children):
        _remove_object_tree(child)
    bpy.data.objects.remove(obj, do_unlink=True)


def _replace_collision_with_box(
    bank: bpy.types.Collection,
    root: bpy.types.Object,
    minimum: tuple[int, int, int],
    maximum: tuple[int, int, int],
) -> None:
    old_shapes = tuple(
        child
        for child in root.children
        if child.get("v8_role") == "authored_collision_shape"
    )
    flags = next(
        (
            int(shape.get("v8_collision_flags", 0))
            for shape in old_shapes
            if shape.get("v8_shape_kind") == "aabb"
        ),
        0,
    )
    for shape in old_shapes:
        _remove_object_tree(shape)

    shape = bpy.data.objects.new(f"{root.name} — Automatic Box", None)
    shape["v8_role"] = "authored_collision_shape"
    shape["v8_shape_index"] = 0
    shape["v8_shape_kind"] = "aabb"
    shape["v8_minimum"] = list(minimum)
    shape["v8_maximum"] = list(maximum)
    shape["v8_override_y"] = minimum[1]
    shape["v8_collision_flags"] = flags
    shape["v8_generated_from_mesh"] = True
    shape.empty_display_type = "CUBE"
    native_center = tuple(
        (minimum[axis] + maximum[axis]) / 2.0 for axis in range(3)
    )
    native_half = tuple(
        max(1.0, (maximum[axis] - minimum[axis]) / 2.0)
        for axis in range(3)
    )
    shape.location = _native_vector_to_blender(native_center, FIXED)
    shape.scale = (
        native_half[0] / FIXED,
        native_half[2] / FIXED,
        native_half[1] / FIXED,
    )
    shape.parent = root
    bank.objects.link(shape)
    shape.hide_set(True)


def update_automatic_collision_boxes(
    vehicle_collection: bpy.types.Collection,
) -> None:
    """Regenerate native box primitives during export when geometry changed."""

    policy = str(
        vehicle_collection.get("v8_collision_export", "AUTO_IF_CHANGED")
    )
    if policy == "PRESERVE":
        return
    bpy.context.view_layer.update()
    for bank in (
        child
        for child in vehicle_collection.children
        if child.get("v8_role") == ROLE_BANK
    ):
        roots = sorted(
            _objects(bank, "authored_collision_stream"),
            key=lambda obj: int(obj["v8_collision_index"]),
        )
        for root in roots:
            owner = root.parent
            if owner is None or owner.get("v8_role") != "authored_slot":
                continue
            bounds = _native_subtree_bounds(owner)
            if bounds is None:
                continue
            changed = _collision_subtree_changed(owner)
            if policy == "AUTO_ALWAYS" or changed:
                _replace_collision_with_box(
                    bank, root, bounds[0], bounds[1]
                )
                root["v8_auto_mesh_minimum"] = list(bounds[0])
                root["v8_auto_mesh_maximum"] = list(bounds[1])


def capture_collision_mesh_bounds(
    vehicle_collection: bpy.types.Collection,
) -> None:
    """Record the fully evaluated import state for change detection."""

    bpy.context.view_layer.update()
    for bank in (
        child
        for child in vehicle_collection.children
        if child.get("v8_role") == ROLE_BANK
    ):
        for obj in _objects(bank, "authored_slot"):
            obj["v8_auto_source_position"] = list(
                _blender_vector_to_native(obj.location, FIXED)
            )
            obj["v8_auto_source_rotation"] = list(
                _blender_rotation_to_native(obj.rotation_euler)
            )
            if obj.type == "MESH":
                local_bounds = _local_mesh_bounds(obj)
                if local_bounds is not None:
                    obj["v8_auto_source_mesh_minimum"] = list(local_bounds[0])
                    obj["v8_auto_source_mesh_maximum"] = list(local_bounds[1])
        for root in _objects(bank, "authored_collision_stream"):
            owner = root.parent
            if owner is None or owner.get("v8_role") != "authored_slot":
                continue
            bounds = _native_subtree_bounds(owner)
            if bounds is not None:
                root["v8_auto_mesh_minimum"] = list(bounds[0])
                root["v8_auto_mesh_maximum"] = list(bounds[1])


def _import_animations(
    bank_collection: bpy.types.Collection,
    animations: tuple[project.SlotAnimation, ...],
    semantics: tuple[tuple[str, str, str], ...],
    slot_objects: tuple[bpy.types.Object, ...],
) -> None:
    for animation in animations:
        part_name = (
            semantics[animation.slot][0]
            if 0 <= animation.slot < len(semantics)
            else f"Unknown Slot {animation.slot:03d}"
        )
        root = bpy.data.objects.new(
            f"{part_name} — Animation [Slot {animation.slot:03d}]",
            None,
        )
        root["v8_role"] = "authored_animation"
        root["v8_animation_slot"] = animation.slot
        root["v8_animation_loop"] = animation.loop
        bank_collection.objects.link(root)
        if 0 <= animation.slot < len(slot_objects):
            root.parent = slot_objects[animation.slot]
            root.matrix_parent_inverse.identity()
        for frame_index, frame in enumerate(animation.frames):
            obj = bpy.data.objects.new(
                f"{root.name} — Frame {frame_index:03d}",
                None,
            )
            obj["v8_role"] = "authored_animation_frame"
            obj["v8_frame_index"] = frame_index
            obj["v8_frame_delta"] = frame.frame_delta
            obj["v8_rotation_parameter"] = frame.rotation_parameter
            for name in (
                "rotation_yxz",
                "translation_absolute",
                "translation_delta",
                "scale",
            ):
                value = getattr(frame, name)
                obj[f"v8_has_{name}"] = value is not None
                if value is not None:
                    obj[f"v8_{name}"] = list(value)
            obj.parent = root
            bank_collection.objects.link(obj)
            for binding_index, binding in enumerate(
                frame.texture_bindings
            ):
                child = bpy.data.objects.new(
                    f"{obj.name} — Texture Binding {binding_index:03d}",
                    None,
                )
                child["v8_role"] = "authored_animation_texture_binding"
                child["v8_binding_index"] = binding_index
                child["v8_target"] = binding.target
                child["v8_texture"] = binding.texture
                child.parent = obj
                bank_collection.objects.link(child)
            for vector_index, vector in enumerate(frame.mesh_vectors):
                child = bpy.data.objects.new(
                    f"{obj.name} — Mesh Vector {vector_index:03d}",
                    None,
                )
                child["v8_role"] = "authored_mesh_vector"
                child["v8_vector_index"] = vector_index
                child["v8_vector"] = list(vector)
                child.parent = obj
                bank_collection.objects.link(child)


def _import_bank(
    vehicle_collection: bpy.types.Collection,
    bank_name: str,
    bank: project.VehicleProject | project.ObjectBank,
) -> None:
    collection_name = (
        "Body Object Bank (native XOBF)"
        if bank_name == "body"
        else "Transformation Object Bank (native XOBF)"
    )
    collection = bpy.data.collections.new(
        collection_name
    )
    collection["v8_role"] = ROLE_BANK
    collection["v8_bank"] = bank_name
    _link_collection(vehicle_collection, collection)

    transform_role_details: dict[int, tuple[str, tuple[str, ...]]] = {}
    if bank_name == "transformation" and isinstance(bank, project.ObjectBank):
        transform_role_details = _transformation_roles(vehicle_collection)
    transform_roles = {
        root: name for root, (name, _assignments) in transform_role_details.items()
    }

    body_root = (
        bank.body_kind
        if bank_name == "body" and isinstance(bank, project.VehicleProject)
        else None
    )
    semantics = list(
        _slot_display_names(
            bank.groups,
            bank.slots,
            bank_name,
            body_root,
            transform_roles,
        )
    )
    if bank_name == "body" and vehicle_collection.get("v8_game") == "V8":
        standard_wheels: dict[int, list[str]] = {}
        standard_wheels.setdefault(
            int(vehicle_collection["v8_stat_wheel_kind_front"]), []
        ).append("Front")
        standard_wheels.setdefault(
            int(vehicle_collection["v8_stat_wheel_kind_rear"]), []
        ).append("Rear")
        for slot_index, axles in standard_wheels.items():
            if 0 <= slot_index < len(semantics) and slot_index != body_root:
                semantics[slot_index] = (
                    "Standard Wheel Model — "
                    + ("All Axles" if len(axles) == 2 else f"{axles[0]} Axle"),
                    "standard_wheel_model",
                    "HIGH",
                )
    semantics = tuple(semantics)

    slot_objects = _import_slots(
        collection,
        bank.slots,
        semantics,
    )
    for root, (_name, assignments) in transform_role_details.items():
        if 0 <= root < len(slot_objects):
            slot_objects[root]["v8_transform_assignment_summary"] = (
                "; ".join(assignments)
            )

    images = {
        index: _make_texture_image(
            vehicle_collection, bank_name, index, texture
        )
        for index, texture in enumerate(bank.textures)
    }
    model_names = _model_display_names(
        bank.groups, bank.slots, semantics, bank_name
    )
    for index, group in enumerate(bank.groups):
        references = [
            slot_index
            for slot_index, slot in enumerate(bank.slots)
            if slot.render_group == index
        ]
        if references:
            for slot_index in references:
                model = _import_group(
                    vehicle_collection,
                    collection,
                    bank_name,
                    index,
                    group,
                    images,
                    semantics[slot_index][0],
                    target=slot_objects[slot_index],
                )
                model["v8_referencing_slots"] = references
                model["v8_primary_slot"] = slot_index
                model["v8_shared_native_group"] = len(references) > 1
        else:
            model = _import_group(
                vehicle_collection,
                collection,
                bank_name,
                index,
                group,
                images,
                model_names[index],
            )
            model["v8_referencing_slots"] = []

    _import_collisions(
        collection,
        bank.collisions,
        _collision_display_names(
            bank.collisions, bank.slots, semantics
        ),
        bank.slots,
        slot_objects,
    )
    _import_animations(
        collection,
        bank.animations,
        semantics,
        slot_objects,
    )


def _slot_damage_state(obj: bpy.types.Object) -> str | None:
    current = obj
    while current is not None and current.get("v8_role") == "authored_slot":
        state = current.get("v8_damage_state")
        if state:
            return str(state)
        current = current.parent
    return None


def apply_authoring_visibility(
    vehicle_collection: bpy.types.Collection,
    *,
    preview_state: str = "INTACT",
    show_attachment_guides: bool = False,
    show_advanced: bool = False,
    show_transform_library: bool = False,
) -> None:
    """Apply viewport-only visibility without changing export data."""

    valid_states = {"INTACT", "DAMAGED", "HEAVILY_DAMAGED", "DESTROYED", "ALL"}
    state = preview_state if preview_state in valid_states else "INTACT"
    label_roles = ATTACHMENT_GUIDE_ROLES | {"transformation_wheel_root"}

    for bank in (
        child
        for child in vehicle_collection.children
        if child.get("v8_role") == ROLE_BANK
    ):
        bank_name = bank.get("v8_bank")
        is_transform = bank_name == "transformation"
        is_selector_preview = bank_name == "selector_preview"
        bank.hide_viewport = bool(
            (is_transform and not show_transform_library)
            or is_selector_preview
        )
        for obj in bank.all_objects:
            role = str(obj.get("v8_role", ""))
            part_role = str(obj.get("v8_part_role", ""))
            hidden = False

            if role in ENGINE_RECORD_ROLES:
                hidden = not show_advanced
            elif role == "authored_slot":
                damage_state = _slot_damage_state(obj)
                if state != "ALL" and damage_state is not None:
                    hidden = damage_state != state
                if part_role in ADVANCED_MODEL_ROLES and not show_advanced:
                    hidden = True
                if (
                    part_role == "vehicle_destruction_sequence"
                    and state not in {"DESTROYED", "ALL"}
                ):
                    hidden = True
                if obj.type == "EMPTY":
                    hidden = hidden or not (
                        show_attachment_guides or show_advanced
                    )
                obj.show_name = bool(
                    show_attachment_guides and part_role in label_roles
                )
                obj.show_in_front = bool(show_attachment_guides)

            obj.hide_set(hidden)


def project_to_scene(context, vehicle: project.VehicleProject) -> bpy.types.Collection:
    """Create a fully decoded, donor-free Blender collection."""

    vehicle.validate()
    collection = bpy.data.collections.new(vehicle.display_name)
    collection["v8_role"] = ROLE_VEHICLE
    collection["v8_schema_version"] = vehicle.schema_version
    collection["v8_game"] = vehicle.game
    collection["v8_stable_id"] = vehicle.stable_id
    collection["v8_display_name"] = vehicle.display_name
    collection["v8_body_kind"] = vehicle.body_kind
    collection["v8_controller_class"] = vehicle.controller_class
    collection["v8_supports_transformations"] = (
        vehicle.supports_transformations
    )
    collection["v8_selector_preview_body_kind"] = (
        vehicle.selector_preview_body_kind
    )
    collection["v8_collision_export"] = "AUTO_IF_CHANGED"
    profile = stats.PROFILES[vehicle.game]
    for field in profile.authoring_fields:
        if field.name == "vehicle_type":
            continue
        key = f"v8_stat_{field.name}"
        collection[key] = int(vehicle.stats[field.name])
        ui = collection.id_properties_ui(key)
        metadata = {
            "description": field.help_text,
            # Blender's ID-property UI metadata uses a signed C long on
            # Windows even when the native game field is unsigned.
            "min": max(field.authoring_min, -0x80000000),
            "max": min(field.authoring_max, 0x7FFFFFFF),
        }
        if field.recommended_min is not None:
            metadata["soft_min"] = field.recommended_min
        if field.recommended_max is not None:
            metadata["soft_max"] = field.recommended_max
        ui.update(**metadata)
    powerup_specs = {
        setting.name: setting for setting in stats.V82_POWERUP_SETTINGS
    }
    for name, value in vehicle.powerups.items():
        key = f"v8_powerup_{name}"
        collection[key] = value
        setting = powerup_specs[name]
        collection.id_properties_ui(key).update(
            description=(
                f"{setting.description} Unit: {setting.unit}. "
                f"Safe authoring bound: {setting.author_min} to "
                f"{setting.author_max}. Retail value: "
                f"{setting.retail_value}. Lower: {setting.lower_effect}. "
                f"Higher: {setting.higher_effect}."
            ),
            min=setting.author_min,
            max=setting.author_max,
            soft_min=setting.retail_value,
            soft_max=setting.retail_value,
        )
    for mode, values in enumerate(vehicle.transform_modes):
        for wheel, value in enumerate(values):
            collection[f"v8_transform_{mode}_{wheel}"] = value
    context.scene.collection.children.link(collection)
    _import_bank(collection, "body", vehicle)
    if vehicle.transformation_bank is not None:
        _import_bank(collection, "transformation", vehicle.transformation_bank)
    if vehicle.selector_preview_bank is not None:
        _import_bank(
            collection,
            "selector_preview",
            vehicle.selector_preview_bank,
        )
    capture_collision_mesh_bounds(collection)
    settings = context.scene.v8_vehicle_settings
    settings.vehicle_collection = collection.name
    settings.preview_state = "INTACT"
    settings.transformation_preview_mode = "STANDARD"
    settings.show_attachment_guides = False
    settings.show_advanced_engine_objects = False
    settings.show_transformation_library = False
    apply_authoring_visibility(
        collection,
        preview_state=settings.preview_state,
        show_attachment_guides=settings.show_attachment_guides,
        show_advanced=settings.show_advanced_engine_objects,
        show_transform_library=settings.show_transformation_library,
    )
    update_transformation_preview(
        collection, settings.transformation_preview_mode
    )
    for window in context.window_manager.windows:
        for area in window.screen.areas:
            if area.type == "VIEW_3D":
                area.spaces.active.shading.type = "MATERIAL"
    return collection


def _texture_from_image(image: bpy.types.Image) -> project.Texture:
    palette = tuple(int(value) for value in image["v8_palette_bgr555"])
    original_indices = tuple(
        int(value) for value in image.get("v8_palette_indices", ())
    )
    original_direct = tuple(
        int(value)
        for value in image.get("v8_direct_pixels_bgr555", ())
    )
    pixels = tuple(image.pixels)
    if int(image["v8_depth"]) == 2:
        direct = []
        for output_row in range(image.size[1]):
            source_row = image.size[1] - 1 - output_row
            for column in range(image.size[0]):
                offset = (source_row * image.size[0] + column) * 4
                native_pixel = output_row * image.size[0] + column
                pixel = pixels[offset : offset + 4]
                preserved = (
                    original_direct[native_pixel]
                    if native_pixel < len(original_direct)
                    else -1
                )
                if preserved >= 0:
                    original = _bgr555_rgba(preserved)
                    if all(
                        abs(pixel[channel] - original[channel]) < 0.01
                        for channel in range(4)
                    ):
                        direct.append(preserved)
                        continue
                direct.append(_rgba_to_bgr555(pixel))
        return project.Texture(
            name=str(image.get("v8_texture_name", image.name)),
            width=image.size[0],
            height=image.size[1],
            depth=2,
            palette_bgr555=(),
            indices=b"",
            compressed=bool(image.get("v8_compressed", False)),
            direct_pixels_bgr555=tuple(direct),
            palette_origin=_prop_tuple(
                image, "v8_palette_origin", 2, (0, 0)
            ),
            image_origin=_prop_tuple(
                image, "v8_image_origin", 2, (0, 0)
            ),
        )

    indices = bytearray(image.size[0] * image.size[1])
    addressable_palette = (
        palette[:16]
        if int(image["v8_depth"]) == 0
        else palette[:256]
    )
    for output_row in range(image.size[1]):
        source_row = image.size[1] - 1 - output_row
        for column in range(image.size[0]):
            offset = (source_row * image.size[0] + column) * 4
            native_pixel = output_row * image.size[0] + column
            pixel = pixels[offset : offset + 4]
            preserved = (
                original_indices[native_pixel]
                if native_pixel < len(original_indices)
                else -1
            )
            if 0 <= preserved < len(palette):
                original = _bgr555_rgba(palette[preserved])
                if all(
                    abs(pixel[channel] - original[channel]) < 0.01
                    for channel in range(4)
                ):
                    indices[native_pixel] = preserved
                    continue
            indices[native_pixel] = _nearest_palette(
                pixel, addressable_palette
            )
    return project.Texture(
        name=str(image.get("v8_texture_name", image.name)),
        width=image.size[0],
        height=image.size[1],
        depth=int(image["v8_depth"]),
        palette_bgr555=palette,
        indices=bytes(indices),
        compressed=bool(image.get("v8_compressed", False)),
        direct_pixels_bgr555=(),
        palette_origin=_prop_tuple(
            image, "v8_palette_origin", 2, (0, 0)
        ),
        image_origin=_prop_tuple(
            image, "v8_image_origin", 2, (0, 0)
        ),
    )


def _export_group(obj: bpy.types.Object) -> project.RenderGroup:
    mesh = obj.data
    shift = int(obj["v8_scale_shift"])
    scale = float(1 << shift)
    vertices = tuple(
        _blender_vector_to_native(vertex.co, scale)
        for vertex in mesh.vertices
    )
    uv_layer = mesh.uv_layers.active
    faces = []
    for face_index, polygon in enumerate(mesh.polygons):
        if len(polygon.vertices) != 3:
            raise ValueError(f"{obj.name} contains a non-triangle face")
        material = mesh.materials[polygon.material_index]
        color = _prop_tuple(material, "v8_face_color", 3)
        if color is None:
            color = tuple(round(component * 255) for component in material.diffuse_color[:3])
        texture_index = int(material.get("v8_texture_index", -1))
        uv = ((0, 0), (0, 0), (0, 0))
        if uv_layer is not None:
            denominator_u = (
                max(1, int(material.get("v8_texture_width", 256)) - 1)
                if texture_index >= 0
                else 255
            )
            denominator_v = (
                max(1, int(material.get("v8_texture_height", 256)) - 1)
                if texture_index >= 0
                else 255
            )
            uv = tuple(
                (
                    int(round(
                        uv_layer.data[loop].uv.x * denominator_u)),
                    int(round(
                        (1.0 - uv_layer.data[loop].uv.y)
                        * denominator_v)),
                )
                for loop in polygon.loop_indices
            )
        packet_kind = (
            None
            if _face_int(mesh, "v8_packet_kind", face_index, -1) < 0
            else _face_int(mesh, "v8_packet_kind", face_index, -1)
        )
        environment_parameters = tuple(
            _face_int(
                mesh, f"v8_environment_{item}", face_index, 0
            )
            for item in range(
                _face_int(
                    mesh, "v8_environment_count", face_index, 0
                )
            )
        )
        if packet_kind == 12 and environment_parameters:
            environment_id = int(
                material.get(
                    "v8_environment_material_id",
                    environment_parameters[0] & 0x3FFF,
                )
            )
            if environment_id >= 0:
                environment_parameters = (
                    (environment_parameters[0] & 0xC000)
                    | (environment_id & 0x3FFF),
                    *environment_parameters[1:],
                )
        faces.append(
            project.Face(
                vertices=tuple(int(value) for value in polygon.vertices),
                color=color,
                texture=None if texture_index < 0 else texture_index,
                native_texture_slot=(
                    None
                    if _face_int(
                        mesh, "v8_native_texture_slot", face_index, -1
                    ) < 0
                    else _face_int(
                        mesh, "v8_native_texture_slot", face_index, -1
                    )
                ),
                uv=uv,
                packet_index=(
                    None
                    if _face_int(
                        mesh, "v8_packet_index", face_index, -1
                    ) < 0
                    else _face_int(
                        mesh, "v8_packet_index", face_index, -1
                    )
                ),
                packet_kind=packet_kind,
                packet_flags=_face_int(
                    mesh, "v8_packet_flags", face_index, 0
                ),
                normal_indices=tuple(
                    _face_int(mesh, f"v8_normal_{item}", face_index, 0)
                    for item in range(
                        _face_int(mesh, "v8_normal_count", face_index, 0)
                    )
                ),
                material_parameter=_face_int(
                    mesh, "v8_material_parameter", face_index, 0
                ),
                texture_flags=_face_int(
                    mesh, "v8_texture_flags", face_index, 0
                ),
                gouraud_colors=tuple(
                    tuple(
                        _face_int(
                            mesh,
                            f"v8_gouraud_{item}_{suffix}",
                            face_index,
                            0,
                        )
                        for suffix in ("r", "g", "b")
                    )
                    for item in range(
                        _face_int(mesh, "v8_gouraud_count", face_index, 0)
                    )
                ),
                environment_parameters=environment_parameters,
                post_parameter=_face_int(
                    mesh, "v8_post_parameter", face_index, 0
                ),
            )
        )
    normal_mesh = obj.get(NATIVE_NORMAL_TABLE_PROPERTY)
    if not isinstance(normal_mesh, bpy.types.Mesh):
        # Backward compatibility for older .blend files that used a visible
        # loose-vertex object for this decoded table.
        normal_tables = [
            child
            for child in obj.children
            if child.get("v8_role") == "authored_normal_table"
        ]
        normal_mesh = normal_tables[0].data if normal_tables else None
    normals = ()
    if isinstance(normal_mesh, bpy.types.Mesh):
        pad_attribute = normal_mesh.attributes.get("v8_normal_pad")
        normals = tuple(
            _blender_vector_to_native(vertex.co, 4096.0)
            + (
                0
                if pad_attribute is None
                else int(pad_attribute.data[index].value),
            )
            for index, vertex in enumerate(normal_mesh.vertices)
        )
    controls = []
    for control_obj in sorted(
        (
            child
            for child in obj.children
            if child.get("v8_role") == "authored_render_control"
        ),
        key=lambda child: int(child["v8_packet_index"]),
    ):
        entries = sorted(
            (
                child
                for child in control_obj.children
                if child.get("v8_role") == "authored_render_control_entry"
            ),
            key=lambda child: int(child["v8_entry_index"]),
        )
        controls.append(
            project.RenderControl(
                packet_index=int(control_obj["v8_packet_index"]),
                packet_kind=int(control_obj["v8_packet_kind"]),
                packet_flags=int(control_obj["v8_packet_flags"]),
                color=_prop_tuple(control_obj, "v8_color", 3),
                vertex_indices=_prop_tuple(
                    control_obj, "v8_vertex_indices", 3
                ),
                entries=tuple(
                    _prop_tuple(entry, "v8_values", 4)
                    for entry in entries
                ),
            )
        )
    return project.RenderGroup(
        name=str(obj.get("v8_group_name", obj.name)),
        scale_shift=shift,
        vertices=vertices,
        faces=tuple(faces),
        normals=normals,
        controls=tuple(controls),
        texture_slot_count=int(obj.get("v8_texture_slot_count", 0)),
        render_extent=int(obj.get("v8_render_extent", 0)),
    )


def _export_slots(
    bank: bpy.types.Collection,
    render_group_overrides: dict[int, int] | None = None,
) -> tuple[project.Slot, ...]:
    objects = sorted(
        _objects(bank, "authored_slot"), key=lambda obj: int(obj["v8_slot_index"])
    )
    indices = {obj: index for index, obj in enumerate(objects)}
    result = []
    for obj in objects:
        slot_index = int(obj["v8_slot_index"])
        render_group = (
            render_group_overrides[slot_index]
            if render_group_overrides is not None
            and slot_index in render_group_overrides
            else int(obj.get("v8_render_group", -1))
        )
        collision = int(obj.get("v8_collision", -1))
        key = int(obj.get("v8_attachment_key", -1))
        result.append(
            project.Slot(
                name=str(obj.get("v8_slot_name", obj.name)),
                render_group=None if render_group < 0 else render_group,
                render_flags=int(obj.get("v8_render_flags", 0)),
                collision=None if collision < 0 else collision,
                key=None if key < 0 else key,
                position=_blender_vector_to_native(obj.location, FIXED),
                rotation_yxz=_blender_rotation_to_native(
                    obj.rotation_euler),
                flags=int(
                    obj.get(
                        "v8_native_object_id",
                        obj.get("v8_object_flags", 0),
                    )
                ),
                parent=indices.get(obj.parent),
            )
        )
    return tuple(result)


def _export_collisions(bank: bpy.types.Collection) -> tuple[project.CollisionStream, ...]:
    streams = sorted(
        _objects(bank, "authored_collision_stream"),
        key=lambda obj: int(obj["v8_collision_index"]),
    )
    result = []
    for root in streams:
        shapes = sorted(
            (child for child in root.children if child.get("v8_role") == "authored_collision_shape"),
            key=lambda obj: int(obj["v8_shape_index"]),
        )
        decoded = []
        for obj in shapes:
            if obj["v8_shape_kind"] == "aabb":
                decoded.append(
                    project.CollisionAabb(
                        minimum=_prop_tuple(obj, "v8_minimum", 3),
                        maximum=_prop_tuple(obj, "v8_maximum", 3),
                        override_y=int(obj["v8_override_y"]),
                        flags=int(obj.get("v8_collision_flags", 0)),
                    )
                )
            else:
                planes = sorted(
                    (child for child in obj.children if child.get("v8_role") == "authored_collision_plane"),
                    key=lambda child: int(child["v8_plane_index"]),
                )
                decoded.append(
                    project.CollisionConvex(
                        planes=tuple(
                            project.CollisionPlane(
                                normal=_prop_tuple(plane, "v8_normal", 3),
                                offset=int(plane["v8_offset"]),
                                pad=int(plane.get("v8_pad", 0)),
                            )
                            for plane in planes
                        )
                    )
                )
        result.append(
            project.CollisionStream(
                name=str(root.get("v8_collision_name", root.name)),
                shapes=tuple(decoded),
            )
        )
    return tuple(result)


def _export_animations(bank: bpy.types.Collection) -> tuple[project.SlotAnimation, ...]:
    roots = sorted(
        _objects(bank, "authored_animation"),
        key=lambda obj: int(obj["v8_animation_slot"]),
    )
    result = []
    for root in roots:
        frame_objects = sorted(
            (child for child in root.children if child.get("v8_role") == "authored_animation_frame"),
            key=lambda obj: int(obj["v8_frame_index"]),
        )
        frames = []
        for obj in frame_objects:
            optional = {}
            for name, count in (
                ("rotation_yxz", 3),
                ("translation_absolute", 3),
                ("translation_delta", 3),
                ("scale", 4),
            ):
                optional[name] = (
                    _prop_tuple(obj, f"v8_{name}", count)
                    if obj.get(f"v8_has_{name}", False)
                    else None
                )
            bindings = sorted(
                (
                    child
                    for child in obj.children
                    if child.get("v8_role")
                    == "authored_animation_texture_binding"
                ),
                key=lambda child: int(child["v8_binding_index"]),
            )
            vectors = sorted(
                (child for child in obj.children if child.get("v8_role") == "authored_mesh_vector"),
                key=lambda child: int(child["v8_vector_index"]),
            )
            frames.append(
                project.AnimationFrame(
                    frame_delta=int(obj["v8_frame_delta"]),
                    rotation_parameter=int(obj.get("v8_rotation_parameter", 0)),
                    texture_bindings=tuple(
                        project.TextureBinding(
                            target=int(child["v8_target"]),
                            texture=int(child["v8_texture"]),
                        )
                        for child in bindings
                    ),
                    mesh_vectors=tuple(
                        _prop_tuple(child, "v8_vector", 4) for child in vectors
                    ),
                    **optional,
                )
            )
        result.append(
            project.SlotAnimation(
                slot=int(root["v8_animation_slot"]),
                loop=bool(root.get("v8_animation_loop", True)),
                frames=tuple(frames),
            )
        )
    return tuple(result)


def _export_bank(
    vehicle: bpy.types.Collection, bank_name: str
) -> project.ObjectBank:
    bank = next(
        (
            child
            for child in vehicle.children
            if child.get("v8_role") == ROLE_BANK and child.get("v8_bank") == bank_name
        ),
        None,
    )
    if bank is None:
        raise ValueError(f"{vehicle.name} has no {bank_name} bank")
    group_objects: dict[int, list[bpy.types.Object]] = {}
    for obj in bank.all_objects:
        if (
            obj.type == "MESH"
            and obj.get("v8_role") in {"authored_slot", "authored_render_group"}
            and "v8_group_index" in obj
        ):
            group_objects.setdefault(int(obj["v8_group_index"]), []).append(obj)
    if sorted(group_objects) != list(range(len(group_objects))):
        raise ValueError(
            f"{bank.name} native model identities must be contiguous from zero"
        )
    original_groups = []
    split_groups = []
    render_group_overrides: dict[int, int] = {}
    for group_index in sorted(group_objects):
        candidates = sorted(
            group_objects[group_index],
            key=lambda obj: (
                0 if obj.get("v8_role") == "authored_slot" else 1,
                int(obj.get("v8_slot_index", 0x7FFFFFFF)),
                obj.name,
            ),
        )
        decoded = [_export_group(obj) for obj in candidates]
        original_groups.append(decoded[0])
        unique_variants = [decoded[0]]
        variant_indices = [group_index]
        for obj, candidate in zip(candidates, decoded):
            try:
                variant = unique_variants.index(candidate)
            except ValueError:
                variant = len(unique_variants)
                unique_variants.append(candidate)
                variant_indices.append(
                    len(group_objects) + len(split_groups)
                )
                split_groups.append(candidate)
            if obj.get("v8_role") == "authored_slot":
                render_group_overrides[
                    int(obj["v8_slot_index"])
                ] = variant_indices[variant]
    groups = tuple(original_groups + split_groups)
    textures = tuple(
        _texture_from_image(image)
        for image in sorted(
            (
                image
                for image in bpy.data.images
                if image.get("v8_role") == "authored_texture"
                and image.get("v8_vehicle_collection") == vehicle.name
                and image.get("v8_bank") == bank_name
            ),
            key=lambda image: int(image["v8_texture_index"]),
        )
    )
    return project.ObjectBank(
        groups=groups,
        slots=_export_slots(bank, render_group_overrides),
        collisions=_export_collisions(bank),
        textures=textures,
        animations=_export_animations(bank),
    )


def scene_to_project(
    collection: bpy.types.Collection,
    *,
    prepare_collision: bool = True,
) -> project.VehicleProject:
    if collection.get("v8_role") != ROLE_VEHICLE:
        raise ValueError("select a donor-free authored vehicle collection")
    if prepare_collision:
        update_automatic_collision_boxes(collection)
    game = str(collection["v8_game"])
    controller_class = str(collection.get("v8_controller_class", "ground"))
    supports_transformations = bool(
        collection.get("v8_supports_transformations", game == "V8_2")
    )
    profile = stats.PROFILES[game]
    body = _export_bank(collection, "body")
    transform = (
        _export_bank(collection, "transformation")
        if game == "V8_2" and any(
            child.get("v8_role") == ROLE_BANK
            and child.get("v8_bank") == "transformation"
            for child in collection.children
        )
        else None
    )
    selector_preview = (
        _export_bank(collection, "selector_preview")
        if any(
            child.get("v8_role") == ROLE_BANK
            and child.get("v8_bank") == "selector_preview"
            for child in collection.children
        )
        else None
    )
    vehicle = project.VehicleProject(
        schema_version=int(collection["v8_schema_version"]),
        stable_id=str(collection["v8_stable_id"]),
        display_name=str(collection["v8_display_name"]),
        game=game,
        groups=body.groups,
        slots=body.slots,
        collisions=body.collisions,
        textures=body.textures,
        animations=body.animations,
        stats={
            field.name: int(collection[f"v8_stat_{field.name}"])
            for field in profile.authoring_fields
            if field.name != "vehicle_type"
        },
        body_kind=int(collection["v8_body_kind"]),
        transformation_bank=transform,
        selector_preview_bank=selector_preview,
        selector_preview_body_kind=int(
            collection.get("v8_selector_preview_body_kind", 0)
        ),
        transform_modes=(
            tuple(
                tuple(
                    int(collection[f"v8_transform_{mode}_{wheel}"])
                    for wheel in range(project.V82_TRANSFORM_WHEEL_COUNT)
                )
                for mode in range(project.V82_TRANSFORM_MODE_COUNT)
            )
            if game == "V8_2" and supports_transformations
            else ()
        ),
        powerups=(
            {
                name: int(collection[f"v8_powerup_{name}"])
                for name in project.V82_POWERUP_FIELDS
            }
            if game == "V8_2"
            else {}
        ),
        controller_class=controller_class,
        supports_transformations=supports_transformations,
    )
    vehicle.validate()
    return vehicle


def _base_group(name: str) -> project.RenderGroup:
    return project.RenderGroup(
        name=name,
        scale_shift=8,
        vertices=((-96, 0, -160), (96, 0, -160), (96, 0, 160), (-96, 0, 160), (0, -72, 0)),
        faces=(
            project.Face((0, 1, 4), (48, 112, 192)),
            project.Face((1, 2, 4), (48, 112, 192)),
            project.Face((2, 3, 4), (32, 88, 160)),
            project.Face((3, 0, 4), (32, 88, 160)),
            project.Face((0, 3, 2), (24, 24, 28)),
            project.Face((0, 2, 1), (24, 24, 28)),
        ),
    )


def _default_stats(game: str, wheel_kind: int) -> dict[str, int]:
    result = {}
    for field in stats.PROFILES[game].authoring_fields:
        if field.name == "vehicle_type":
            continue
        if field.name in {"wheel_kind_front", "wheel_kind_rear"}:
            value = wheel_kind
        elif field.name in {"wheel_presence_mask", "capability_mask"}:
            value = 0x0F if game == "V8" else 0x3F
        else:
            value = field.default
        result[field.name] = value
    return result


def new_project(game: str, stable_id: str = "new.original_vehicle") -> project.VehicleProject:
    """Return a valid native project that owns all of its data."""

    game = game.upper().replace(":", "_")
    wheel_count = 4 if game == "V8" else 6
    slots = [project.Slot("body", 0, 0)]
    positions = (
        (-65536, 16384, 98304),
        (65536, 16384, 98304),
        (-65536, 16384, -98304),
        (65536, 16384, -98304),
    )
    if wheel_count == 6:
        positions = positions[:2] + ((-65536, 16384, 0), (65536, 16384, 0)) + positions[2:]
    for index, position in enumerate(positions):
        slots.append(
            project.Slot(
                f"wheel_joint_{index}",
                None,
                None,
                key=0x8000 + index,
                position=position,
                parent=0,
            )
        )
    slots.append(
        project.Slot(
            "special_attachment",
            None,
            None,
            key=0x8100,
            position=(0, -65536, -65536),
            parent=0,
        )
    )
    wheel_kind = len(slots)
    slots.append(project.Slot("normal_wheel", 0, None))
    weapon_count = 6 if game == "V8" else 7
    for index in range(weapon_count):
        slots.append(
            project.Slot(
                f"weapon_mount_{index}",
                None,
                None,
                key=0x8010 + index,
                parent=0,
            )
        )
    slots.append(
        project.Slot("weapon_pickup_mount", None, None, key=0x801F, parent=0)
    )
    slots.append(
        project.Slot(
            "special_attachment_secondary",
            None,
            None,
            key=0x8101,
            parent=0,
        )
    )
    collision = project.CollisionStream(
        "body_collision",
        (
            project.CollisionAabb(
                (-98304, -73728, -163840),
                (98304, 32768, 163840),
                -73728,
            ),
        ),
    )
    transform_bank = None
    modes = ()
    powerups = {}
    if game == "V8_2":
        transform_slots = []
        transform_roots = {}
        for name in (
            "hover_left",
            "hover_right",
            "float_left",
            "float_right",
            "ski_left",
            "ski_right",
        ):
            root = len(transform_slots)
            transform_roots[name] = root
            transform_slots.extend(
                (
                    project.Slot(name, 0, 0, flags=-21846),
                    project.Slot(f"{name}_mesh", 0, None, flags=-21846, parent=root),
                    project.Slot(
                        f"{name}_distance_lod",
                        0,
                        None,
                        render_flags=0xC000,
                        parent=root,
                    ),
                    # Positive object IDs above 255 are constructor-special
                    # records and are skipped by the retail 0x8002C17C walk.
                    # Keep one in the fixture so previews cannot regress to
                    # displaying the false duplicate/center-wheel geometry.
                    project.Slot(
                        f"{name}_engine_special",
                        0,
                        None,
                        flags=0x0100,
                        parent=root,
                    ),
                )
            )
        transform_collision = project.CollisionStream(
            "transformation_wheel_collision",
            (project.CollisionAabb((-32768, -32768, -8192), (32768, 32768, 8192), -32768),),
        )
        transform_bank = project.ObjectBank(
            groups=(_base_group("transformation_wheel"),),
            slots=tuple(transform_slots),
            collisions=(transform_collision,),
            textures=(),
            animations=(),
        )
        modes = (
            (0, 0, 0, 0, 0, 0),
            tuple(
                transform_roots[
                    "hover_left" if wheel % 2 == 0 else "hover_right"
                ]
                for wheel in range(6)
            ),
            tuple(
                transform_roots[
                    "float_left" if wheel % 2 == 0 else "float_right"
                ]
                for wheel in range(6)
            ),
            tuple(
                transform_roots[
                    "ski_left" if wheel % 2 == 0 else "ski_right"
                ]
                for wheel in range(6)
            ),
        )
        powerups = {
            "repair_amount": 480,
            "weapon_upgrade_duration": 840,
            "radar_jammer_duration": 780,
            "shield_duration": 720,
            "transformation_duration": 540,
        }
    vehicle = project.VehicleProject(
        schema_version=project.SCHEMA_VERSION,
        stable_id=stable_id,
        display_name="New Original Vehicle",
        game=game,
        groups=(_base_group("body"),),
        slots=tuple(slots),
        collisions=(collision,),
        textures=(),
        animations=(),
        stats=_default_stats(
            game, 0 if game == "V8_2" else wheel_kind
        ),
        body_kind=0,
        transformation_bank=transform_bank,
        transform_modes=modes,
        powerups=powerups,
    )
    vehicle.validate()
    return vehicle
