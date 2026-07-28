"""Build source-free default-vehicle replacement packages from retail semantics."""

from __future__ import annotations

from dataclasses import replace
import difflib
import json
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[3]
ADDONS = ROOT / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

from vigilante8_vehicle_tools import authored_scene, iff, project, registry, stats


def extract_roots(
    source: project.ObjectBank,
    roots: set[int],
) -> tuple[project.ObjectBank, dict[int, int]]:
    """Copy only the semantic dependency closure of selected top-level roots."""

    if any(
        root < 0
        or root >= len(source.slots)
        or source.slots[root].parent is not None
        for root in roots
    ):
        raise ValueError("selected native object kind is not a top-level root")
    selected_slots = set(roots)
    changed = True
    while changed:
        changed = False
        for index, slot in enumerate(source.slots):
            if slot.parent in selected_slots and index not in selected_slots:
                selected_slots.add(index)
                changed = True
    slot_indices = sorted(selected_slots)
    slot_map = {
        old_index: new_index
        for new_index, old_index in enumerate(slot_indices)
    }
    group_indices = sorted(
        {
            source.slots[index].render_group
            for index in slot_indices
            if source.slots[index].render_group is not None
        }
    )
    group_map = {
        old_index: new_index
        for new_index, old_index in enumerate(group_indices)
    }
    collision_indices = sorted(
        {
            source.slots[index].collision
            for index in slot_indices
            if source.slots[index].collision is not None
        }
    )
    collision_map = {
        old_index: new_index
        for new_index, old_index in enumerate(collision_indices)
    }
    texture_indices = sorted(
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
    texture_map = {
        old_index: new_index
        for new_index, old_index in enumerate(texture_indices)
    }
    groups = tuple(
        replace(
            source.groups[old_index],
            name=f"group_{new_index:03d}",
            faces=tuple(
                replace(
                    face,
                    texture=(
                        None
                        if face.texture is None
                        else texture_map[face.texture]
                    ),
                )
                for face in source.groups[old_index].faces
            ),
        )
        for new_index, old_index in enumerate(group_indices)
    )
    slots = tuple(
        replace(
            source.slots[old_index],
            name=f"slot_{new_index:03d}",
            render_group=(
                None
                if source.slots[old_index].render_group is None
                else group_map[source.slots[old_index].render_group]
            ),
            collision=(
                None
                if source.slots[old_index].collision is None
                else collision_map[source.slots[old_index].collision]
            ),
            parent=(
                None
                if source.slots[old_index].parent is None
                else slot_map[source.slots[old_index].parent]
            ),
        )
        for new_index, old_index in enumerate(slot_indices)
    )
    return (
        project.ObjectBank(
            groups=groups,
            slots=slots,
            collisions=tuple(
                replace(
                    source.collisions[old_index],
                    name=f"collision_{new_index:03d}",
                )
                for new_index, old_index in enumerate(collision_indices)
            ),
            textures=tuple(
                replace(
                    source.textures[old_index],
                    name=f"texture_{new_index:03d}",
                )
                for new_index, old_index in enumerate(texture_indices)
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
                                    texture=texture_map[
                                        binding.texture
                                    ],
                                )
                                for binding
                                in frame.texture_bindings
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


def bank(path: Path, game: str, index: int = 0) -> project.ObjectBank:
    form = tuple(iff.parse(path.read_bytes()).forms(b"XOBF"))[index]
    model_form = iff.IffChunk(
        tag=b"FORM",
        form_type=b"XOBF",
        children=tuple(
            child for child in form.children if child.tag in {b"BIN ", b"ANM "}
        ),
    )
    return registry._decode_bank(model_form, game)


def merge_v8_body_and_wheels(
    body: project.ObjectBank, wheels: project.ObjectBank
) -> tuple[project.ObjectBank, int]:
    """Build the independent V8 bank expected by the patched constructor.

    Retail keeps the selected vehicle body and the shared normal-wheel library
    in separate COMMON forms.  A custom/default replacement owns one compiled
    bank, so the semantic wheel roots are appended and the stats wheel kind is
    rebased to the appended root.  No source bytes or unrelated archive forms
    are retained.
    """

    group_base = len(body.groups)
    collision_base = len(body.collisions)
    texture_base = len(body.textures)
    slot_base = len(body.slots)
    merged_groups = body.groups + tuple(
        replace(
            group,
            name=f"group_{group_base + index:03d}",
            faces=tuple(
                replace(
                    face,
                    texture=(
                        None
                        if face.texture is None
                        else face.texture + texture_base
                    ),
                )
                for face in group.faces
            ),
        )
        for index, group in enumerate(wheels.groups)
    )
    merged_slots = body.slots + tuple(
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
        for index, slot in enumerate(wheels.slots)
    )
    merged_animations = body.animations + tuple(
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
        for animation in wheels.animations
    )
    merged = project.ObjectBank(
        groups=merged_groups,
        slots=merged_slots,
        collisions=body.collisions + tuple(
            replace(
                stream,
                name=f"collision_{collision_base + index:03d}",
            )
            for index, stream in enumerate(wheels.collisions)
        ),
        textures=body.textures + tuple(
            replace(
                texture,
                name=f"texture_{texture_base + index:03d}",
            )
            for index, texture in enumerate(wheels.textures)
        ),
        animations=merged_animations,
    )
    return merged, slot_base


def write(directory: Path, vehicle: project.VehicleProject) -> None:
    package = registry.compile_package((vehicle,))
    directory.mkdir(parents=True, exist_ok=True)
    (directory / "CUSTOM.EXP").write_bytes(package.archive)
    (directory / "VEHICLES.V8R").write_bytes(package.registry)
    decoded = registry.decompile_package(package.archive, package.registry)
    expected = project.to_dict(vehicle)
    actual = project.to_dict(decoded[0])
    if actual != expected:
        diff = "\n".join(
            difflib.unified_diff(
                json.dumps(expected, indent=2).splitlines(),
                json.dumps(actual, indent=2).splitlines(),
                fromfile="expected",
                tofile="decoded",
                n=3,
            )
        )
        raise AssertionError(
            "source-free replacement package changed on decode\n" + diff[:12000]
        )
    (directory / "project.json").write_text(
        json.dumps(project.to_dict(vehicle), indent=2) + "\n",
        encoding="utf-8",
    )


def main() -> None:
    output = ROOT / "artifacts" / "dual_game_default_roundtrip"

    v8_stats = stats.StatsFile(
        (ROOT / "PS1 game" / "SLUS_005.10").read_bytes(), "V8"
    ).record(0).values()
    v8_stats.pop("vehicle_type")
    v8_body, v8_body_roots = extract_roots(
        bank(output / "V8_COMMON_ORIGINAL.EXP", "V8"), {0}
    )
    wheel_kinds = {
        v8_stats["wheel_kind_front"],
        v8_stats["wheel_kind_rear"],
    }
    v8_wheels, v8_wheel_roots = extract_roots(
        bank(output / "V8_COMMON_ORIGINAL.EXP", "V8", 13),
        wheel_kinds,
    )
    v8_bank, v8_wheel_slot_base = merge_v8_body_and_wheels(
        v8_body, v8_wheels
    )
    v8_stats["wheel_kind_front"] = (
        v8_wheel_slot_base
        + v8_wheel_roots[v8_stats["wheel_kind_front"]]
    )
    v8_stats["wheel_kind_rear"] = (
        v8_wheel_slot_base
        + v8_wheel_roots[v8_stats["wheel_kind_rear"]]
    )
    v8_base = authored_scene.new_project(
        "V8", "replacement.v8.default_vehicle_0"
    )
    v8 = replace(
        v8_base,
        display_name="V8 Default Vehicle 0 — Source-Free Replacement",
        groups=v8_bank.groups,
        slots=v8_bank.slots,
        collisions=v8_bank.collisions,
        textures=v8_bank.textures,
        animations=v8_bank.animations,
        stats=v8_stats,
        body_kind=v8_body_roots[0],
    )
    v8.validate()
    write(output / "source_free_v8", v8)

    v82_bank_source = bank(output / "V82_COMMON_ORIGINAL.EXP", "V8_2")
    # The executable's four-by-six transformation table indexes the dedicated
    # wheel roots in COMMON form 18.  HOTRODS contains alternate full vehicle
    # bodies and is not the transformation-wheel bank.
    v82_transform_source = bank(
        output / "V82_COMMON_ORIGINAL.EXP", "V8_2", 18
    )
    v82_stats_file = stats.StatsFile(
        (ROOT / "V8_2_LOOSE" / "SLUS_008.68").read_bytes(), "V8_2"
    )
    v82_base = authored_scene.new_project(
        "V8_2", "replacement.v82.default_vehicle_0"
    )
    v82_record = v82_stats_file.record(0)
    v82_stats = v82_record.values()
    v82_stats.pop("vehicle_type")
    v82_stats["rear_suspension_damping"] = int.from_bytes(
        v82_record.raw[0x0C:0x0E], "little"
    )
    v82_bank, v82_body_roots = extract_roots(v82_bank_source, {0})
    original_modes = v82_stats_file.transform_modes()
    transform_roots = {
        v82_stats["wheel_kind_front"],
        v82_stats["wheel_kind_rear"],
        *(
            kind
            for mode in original_modes[1:]
            for kind in mode
        ),
    }
    v82_transform, transform_root_map = extract_roots(
        v82_transform_source, transform_roots
    )
    v82_stats["wheel_kind_front"] = transform_root_map[
        v82_stats["wheel_kind_front"]
    ]
    v82_stats["wheel_kind_rear"] = transform_root_map[
        v82_stats["wheel_kind_rear"]
    ]
    v82_modes = tuple(
        tuple(
            0 if mode_index == 0 else transform_root_map[kind]
            for kind in mode
        )
        for mode_index, mode in enumerate(original_modes)
    )
    v82 = replace(
        v82_base,
        display_name="V8:2 Default Vehicle 0 — Source-Free Replacement",
        groups=v82_bank.groups,
        slots=v82_bank.slots,
        collisions=v82_bank.collisions,
        textures=v82_bank.textures,
        animations=v82_bank.animations,
        stats=v82_stats,
        body_kind=v82_body_roots[0],
        transformation_bank=v82_transform,
        transform_modes=v82_modes,
        powerups=v82_stats_file.powerup_values(),
    )
    v82.validate()
    write(output / "source_free_v82", v82)

    # Form/stat 8 is a retail six-wheel body. Give its third axle a deliberately
    # authored value which differs from the retail capability/type overlap so
    # runtime proof can demonstrate that registry v3, not roster identity,
    # controls the final wheel damping.
    six_index = 8
    six_record = v82_stats_file.record(six_index)
    six_stats = six_record.values()
    six_stats.pop("vehicle_type")
    six_stats["rear_suspension_damping"] = 777
    six_body_source = bank(
        output / "V82_COMMON_ORIGINAL.EXP", "V8_2", six_index
    )
    six_body, six_body_roots = extract_roots(six_body_source, {0})
    six_transform_roots = {
        six_stats["wheel_kind_front"],
        six_stats["wheel_kind_rear"],
        *(
            kind
            for mode in original_modes[1:]
            for kind in mode
        ),
    }
    six_transform, six_transform_root_map = extract_roots(
        v82_transform_source, six_transform_roots
    )
    six_stats["wheel_kind_front"] = six_transform_root_map[
        six_stats["wheel_kind_front"]
    ]
    six_stats["wheel_kind_rear"] = six_transform_root_map[
        six_stats["wheel_kind_rear"]
    ]
    six_modes = tuple(
        tuple(
            0 if mode_index == 0 else six_transform_root_map[kind]
            for kind in mode
        )
        for mode_index, mode in enumerate(original_modes)
    )
    six_vehicle = replace(
        v82_base,
        stable_id="proof.v82.six_wheel_registry_v3",
        display_name="V8:2 Six-Wheel Registry-v3 Proof Vehicle",
        groups=six_body.groups,
        slots=six_body.slots,
        collisions=six_body.collisions,
        textures=six_body.textures,
        animations=six_body.animations,
        stats=six_stats,
        body_kind=six_body_roots[0],
        transformation_bank=six_transform,
        transform_modes=six_modes,
        powerups=v82_stats_file.powerup_values(),
    )
    six_vehicle.validate()
    write(
        ROOT / "artifacts" / "vehicle_plugin_proofs"
        / "v82_six_wheel_registry_v3",
        six_vehicle,
    )


if __name__ == "__main__":
    main()
