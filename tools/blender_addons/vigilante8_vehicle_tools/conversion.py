"""Semantic cross-game conversion for native Vigilante 8 vehicle projects.

The conversion operates only on decoded fields.  It never retains a source
BIN/ANM byte range, a retail registry entry, or an archive reference.
"""

from __future__ import annotations

from dataclasses import replace
from typing import Iterable, Mapping

from . import project


V82_INTERNAL_AUTHORING_DEFAULTS = {
    # Medians of the 18 retail V8:2 records.  These fields are copied by the
    # constructor but are not yet sufficiently understood for a meaningful
    # V8-to-V8:2 mapping.  Fixed authoring defaults avoid borrowing any one
    # retail vehicle's record.
    "unresolved_native_10": 23,
    "unresolved_native_11": 38,
    "unresolved_native_12": 68,
    "chassis_position_follow": 46,
    "upgraded_chassis_position_follow": 33,
}


def extract_roots(
    source: project.ObjectBank,
    roots: set[int],
    *,
    preserve_texture_layout: bool = False,
) -> tuple[project.ObjectBank, dict[int, int]]:
    """Copy the complete semantic dependency closure of top-level roots."""

    if any(
        root < 0
        or root >= len(source.slots)
        or source.slots[root].parent is not None
        for root in roots
    ):
        raise ValueError("selected native object kind is not a top-level root")

    selected = set(roots)
    changed = True
    while changed:
        changed = False
        for index, slot in enumerate(source.slots):
            if slot.parent in selected and index not in selected:
                selected.add(index)
                changed = True

    slot_indices = sorted(selected)
    slot_map = {old: new for new, old in enumerate(slot_indices)}
    group_indices = sorted(
        {
            source.slots[index].render_group
            for index in slot_indices
            if source.slots[index].render_group is not None
        }
    )
    group_map = {old: new for new, old in enumerate(group_indices)}
    collision_indices = sorted(
        {
            source.slots[index].collision
            for index in slot_indices
            if source.slots[index].collision is not None
        }
    )
    collision_map = {old: new for new, old in enumerate(collision_indices)}
    referenced_texture_indices = sorted(
        {
            face.texture
            for group_index in group_indices
            for face in source.groups[group_index].faces
            if face.texture is not None
        }
        | {
            binding.texture
            for animation in source.animations
            if animation.slot in slot_map
            for frame in animation.frames
            for binding in frame.texture_bindings
        }
    )
    texture_indices = (
        list(range(len(source.textures)))
        if preserve_texture_layout
        else referenced_texture_indices
    )
    texture_map = {old: new for new, old in enumerate(texture_indices)}

    def remap_face(
        native_group: project.RenderGroup,
        face: project.Face,
    ) -> project.Face:
        remapped_texture = (
            None
            if face.texture is None
            else (
                face.texture
                if preserve_texture_layout
                else texture_map[face.texture]
            )
        )
        native_slot = face.native_texture_slot
        if not preserve_texture_layout and remapped_texture is not None:
            # Kind-15 packets in groups with a local texture table address
            # that table, not the bank-wide texture array.  Preserve those
            # local indices exactly.  All other packet kinds can be rewritten
            # to the compacted bank index.
            is_local_animated_slot = (
                face.packet_kind == 15
                and native_group.texture_slot_count > 0
                and native_slot is not None
                and native_slot < native_group.texture_slot_count
            )
            if not is_local_animated_slot:
                native_slot = remapped_texture
        return replace(
            face,
            texture=remapped_texture,
            native_texture_slot=native_slot,
        )

    return (
        project.ObjectBank(
            groups=tuple(
                replace(
                    source.groups[old],
                    name=f"group_{new:03d}",
                    faces=tuple(
                        remap_face(source.groups[old], face)
                        for face in source.groups[old].faces
                    ),
                )
                for new, old in enumerate(group_indices)
            ),
            slots=tuple(
                replace(
                    source.slots[old],
                    name=f"slot_{new:03d}",
                    render_group=(
                        None
                        if source.slots[old].render_group is None
                        else group_map[source.slots[old].render_group]
                    ),
                    collision=(
                        None
                        if source.slots[old].collision is None
                        else collision_map[source.slots[old].collision]
                    ),
                    parent=(
                        None
                        if source.slots[old].parent is None
                        else slot_map[source.slots[old].parent]
                    ),
                )
                for new, old in enumerate(slot_indices)
            ),
            collisions=tuple(
                replace(
                    source.collisions[old],
                    name=f"collision_{new:03d}",
                )
                for new, old in enumerate(collision_indices)
            ),
            textures=tuple(
                replace(
                    source.textures[old],
                    name=f"texture_{new:03d}",
                )
                for new, old in enumerate(texture_indices)
            ),
            animations=tuple(
                replace(
                    animation,
                    slot=slot_map[animation.slot],
                    frames=tuple(
                        replace(
                            frame,
                            texture_bindings=tuple(
                                replace(
                                    binding,
                                    texture=(
                                        binding.texture
                                        if preserve_texture_layout
                                        else texture_map[binding.texture]
                                    ),
                                )
                                for binding in frame.texture_bindings
                            ),
                        )
                        for frame in animation.frames
                    ),
                )
                for animation in source.animations
                if animation.slot in slot_map
            ),
        ),
        {root: slot_map[root] for root in roots},
    )


def merge_banks(
    banks: Iterable[project.ObjectBank],
) -> tuple[project.ObjectBank, tuple[int, ...]]:
    """Merge independently decoded banks and return each source slot base."""

    groups: tuple[project.RenderGroup, ...] = ()
    slots: tuple[project.Slot, ...] = ()
    collisions: tuple[project.CollisionStream, ...] = ()
    textures: tuple[project.Texture, ...] = ()
    animations: tuple[project.SlotAnimation, ...] = ()
    slot_bases: list[int] = []

    for bank in banks:
        group_base = len(groups)
        slot_base = len(slots)
        collision_base = len(collisions)
        texture_base = len(textures)
        slot_bases.append(slot_base)

        def merge_face(
            group: project.RenderGroup,
            face: project.Face,
        ) -> project.Face:
            native_slot = face.native_texture_slot
            is_local_animated_slot = (
                face.packet_kind == 15
                and group.texture_slot_count > 0
                and native_slot is not None
                and native_slot < group.texture_slot_count
            )
            if native_slot is not None and not is_local_animated_slot:
                native_slot += texture_base
            return replace(
                face,
                texture=(
                    None
                    if face.texture is None
                    else (
                        native_slot
                        if is_local_animated_slot
                        else face.texture + texture_base
                    )
                ),
                native_texture_slot=native_slot,
            )

        groups += tuple(
            replace(
                group,
                name=f"group_{group_base + index:03d}",
                faces=tuple(
                    merge_face(group, face)
                    for face in group.faces
                ),
            )
            for index, group in enumerate(bank.groups)
        )
        slots += tuple(
            replace(
                slot,
                name=f"slot_{slot_base + index:03d}",
                render_group=(
                    None
                    if slot.render_group is None
                    else slot.render_group + group_base
                ),
                collision=(
                    None
                    if slot.collision is None
                    else slot.collision + collision_base
                ),
                parent=(
                    None if slot.parent is None else slot.parent + slot_base
                ),
            )
            for index, slot in enumerate(bank.slots)
        )
        collisions += tuple(
            replace(
                stream,
                name=f"collision_{collision_base + index:03d}",
            )
            for index, stream in enumerate(bank.collisions)
        )
        textures += tuple(
            replace(
                texture,
                name=f"texture_{texture_base + index:03d}",
            )
            for index, texture in enumerate(bank.textures)
        )
        animations += tuple(
            replace(
                animation,
                slot=animation.slot + slot_base,
                frames=tuple(
                    replace(
                        frame,
                        texture_bindings=tuple(
                            replace(
                                binding,
                                texture=binding.texture + texture_base,
                            )
                            for binding in frame.texture_bindings
                        ),
                    )
                    for frame in animation.frames
                ),
            )
            for animation in bank.animations
        )

    return (
        project.ObjectBank(
            groups=groups,
            slots=slots,
            collisions=collisions,
            textures=textures,
            animations=animations,
        ),
        tuple(slot_bases),
    )


def v8_bank_to_v82(source: project.ObjectBank) -> project.ObjectBank:
    """Translate decoded V8 render packets into the V8:2 packet dialect."""

    def convert_face(face: project.Face) -> project.Face:
        if face.packet_kind != 12 or len(face.environment_parameters) != 2:
            return face

        # The retail V8 and V8:2 loaders use the same packet-mode rewrite:
        #
        #   render = (native & 0x0f) << 2
        #          | (native & 0x80 ? 0x40 : 0)
        #          | (native & 0x10 ? 0x02 : 0)
        #          | (native & 0x40 ? 0x80 : 0)
        #
        # Native bit 0x20 is ignored by both loaders, so remove it without
        # changing the render mode.  Retail V8:2 vehicle banks then use one
        # exact tuple for each of the two environment roles:
        #
        #   opaque arena reflection: type 0x0C, (0x3FFF, 0x8080, 0, 0)
        #   translucent gloss pass:  type 0x1C, (0x7FFE, 0x8080, 0, 0)
        #
        # V8 stores both global roles under selector 0x3FFF and distinguishes
        # the gloss pass with native mode bit 0x10.  Translate that semantic
        # pair to the sequel's own measured packet dialect.  Direct local
        # texture selectors are retained rather than being mistaken for a
        # global vehicle material.
        source_environment = face.environment_parameters[0]
        translucent = bool(face.packet_flags & 0x10)
        target_environment = source_environment
        if (source_environment & 0x3FFF) == 0x3FFF:
            target_environment = 0x7FFE if translucent else 0x3FFF

        return replace(
            face,
            packet_flags=face.packet_flags & ~0x20,
            environment_parameters=(
                target_environment,
                0x8080,
                0,
                0,
            ),
        )

    groups = tuple(
        replace(
            group,
            faces=tuple(convert_face(face) for face in group.faces),
        )
        for group in source.groups
    )
    slots = tuple(
        replace(
            slot,
            # V8 uses an eight-bit group field, so native class 0xF is
            # decoded as 0xFF00.  V8:2 widened the group field to eleven
            # bits and represents the same class as 0xF000.
            render_flags=(
                0xF000 if slot.render_flags == 0xFF00
                else slot.render_flags
            ),
            # A decoded V8 no-model object stores 0x00FF.  The equivalent
            # V8:2 sentinel is 0x07FF; emitting generic -1 would accidentally
            # add V8:2 render-class bits and prevent root construction.
            key=(
                # V8's vehicle constructor takes the first immediate child
                # of a wheel anchor as its authored suspension-travel marker.
                # V8:2 performs the same lookup by the explicit 0x8000 key.
                # Retag only that exact semantic child so the sequel's
                # unchanged constructor consumes the original marker value.
                0x8000
                if slot.parent is not None
                and source.slots[slot.parent].key in range(0x8000, 0x8006)
                and slot.render_group is None
                and slot.key is None
                else (
                    0x07FF
                    if slot.render_group is None and slot.key is None
                    else slot.key
                )
            ),
        )
        for slot in source.slots
    )
    return replace(source, groups=groups, slots=slots)


def add_v82_flamethrower_mount(
    body: project.ObjectBank,
    body_kind: int,
) -> project.ObjectBank:
    """Add V8:2's seventh standard weapon mount when a V8 body lacks it."""

    direct_mounts = {
        slot.key: slot
        for slot in body.slots
        if slot.parent == body_kind and slot.key is not None
    }
    if 0x8016 in direct_mounts:
        return body
    source = direct_mounts.get(0x8010)
    if source is None:
        raise ValueError(
            "V8 body has no direct machine-gun mount from which to place "
            "the V8:2 flamethrower emitter"
        )
    return replace(
        body,
        slots=body.slots
        + (
            project.Slot(
                name=f"slot_{len(body.slots):03d}",
                render_group=None,
                collision=None,
                key=0x8016,
                position=source.position,
                rotation_yxz=source.rotation_yxz,
                flags=-21846,
                parent=body_kind,
            ),
        ),
    )


def closest_wheel_root(
    source: project.ObjectBank,
    source_root: int,
    target: project.ObjectBank,
    target_roots: Iterable[int],
) -> int:
    """Select the sequel wheel whose decoded mesh bounds best match V8."""

    def dimensions(bank: project.ObjectBank, root: int) -> tuple[int, int, int]:
        if root < 0 or root >= len(bank.slots):
            raise ValueError("wheel root is outside its object bank")
        slot = bank.slots[root]
        if slot.parent is not None or slot.render_group is None:
            raise ValueError("wheel selection requires a rendered top-level root")
        vertices = bank.groups[slot.render_group].vertices
        if not vertices:
            raise ValueError("wheel root has no decoded vertices")
        axes = tuple(zip(*vertices))
        return tuple(max(axis) - min(axis) for axis in axes)

    source_dimensions = dimensions(source, source_root)
    candidates = tuple(sorted(set(int(root) for root in target_roots)))
    if not candidates:
        raise ValueError("target wheel candidate set is empty")
    return min(
        candidates,
        key=lambda root: (
            sum(
                (left - right) ** 2
                for left, right in zip(
                    source_dimensions,
                    dimensions(target, root),
                )
            ),
            root,
        ),
    )


def v8_stats_to_v82(
    source: Mapping[str, int],
    *,
    front_wheel_kind: int,
    rear_wheel_kind: int,
) -> dict[str, int]:
    """Translate V8 values without relabeling sequel-only data as original.

    The three menu categories shared by both games use the same ordering
    semantics; V8 stores them on a 0..20 scale and V8:2 on a 0..200 scale.
    Multiplication by ten is therefore exact and preserves every authored
    source value. Retail V8 leaves its fourth byte at zero and does not show
    that category in the selector. V8:2 requires the fourth row, so only that
    row is a documented compatibility value.
    """

    drive = int(source["maximum_drive_force"])
    health = int(source["health"])
    drag = int(source["speed_drag_coefficient"])
    ratings = [
        int(source["rating_armor"]),
        int(source["rating_speed"]),
        int(source["rating_handling"]),
    ]
    return {
        "wheel_kind_front": front_wheel_kind,
        "wheel_kind_rear": rear_wheel_kind,
        "front_spring_stiffness": int(source["front_spring_stiffness"]),
        "middle_spring_stiffness": int(source["rear_spring_stiffness"]),
        "front_damping_rear_stiffness": int(
            source["front_suspension_damping"]
        ),
        "middle_suspension_damping": int(
            source["rear_suspension_damping"]
        ),
        # The bit layout occupies the same native byte in both constructors.
        # Keeping it preserves the original steering/drive capability flags.
        "capability_mask": int(source["wheel_presence_mask"]),
        "low_speed_steering_response": int(
            source["low_speed_steering_response"]
        ),
        "steering_speed_falloff": int(source["steering_speed_falloff"]),
        **V82_INTERNAL_AUTHORING_DEFAULTS,
        "maximum_drive_force": drive,
        "upgraded_maximum_drive_force": min(255, max(drive, (drive * 4 + 1) // 3)),
        "reserved_native_17": 0,
        "health": health,
        "upgraded_health": min(0xFFFF, health + 300),
        "speed_drag_coefficient": drag,
        "upgraded_speed_drag_coefficient": max(1, (drag * 4 + 2) // 5),
        "pitch_angular_response": int(source["pitch_angular_response"]),
        "yaw_angular_response": int(source["yaw_angular_response"]),
        "roll_angular_response": int(source["roll_angular_response"]),
        "mass": int(source["mass"]),
        "rating_armor": min(255, ratings[0] * 10),
        "rating_speed": min(255, ratings[1] * 10),
        "rating_handling": min(255, ratings[2] * 10),
        # V8's selector has three rows and every retail record leaves its
        # fourth rating byte at zero. The sequel's fourth row cannot be copied;
        # use a neutral rounded mean and keep its provenance explicit.
        "rating_special": min(255, (sum(ratings) * 10 + 1) // 3),
        "rear_suspension_damping": int(
            source["rear_suspension_damping"]
        ),
    }
