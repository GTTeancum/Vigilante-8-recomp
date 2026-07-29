"""Strict donor-free vehicle project schema.

Every field in this module has native meaning.  Unknown keys are rejected;
there is intentionally no raw/blob/passthrough property.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any, Mapping, Sequence


SCHEMA_VERSION = 4
MAX_TEXTURE_DIMENSION = 256
MAX_TEXTURES_PER_BANK = 256
MAX_TEXTURE_TEXELS_PER_BANK = 262_144
MAX_NATIVE_TEXTURE_BYTES_PER_BANK = 128 * 1024
MAX_NATIVE_BANK_SOURCE_BYTES = 256 * 1024
MAX_CUSTOM_VEHICLES = 192

V82_POWERUP_FIELDS = (
    "repair_amount",
    "weapon_upgrade_duration",
    "radar_jammer_duration",
    "shield_duration",
    "transformation_duration",
)
V82_TRANSFORM_MODE_COUNT = 4
V82_TRANSFORM_WHEEL_COUNT = 6


def _strict(mapping: Mapping[str, Any], allowed: set[str], context: str) -> None:
    unknown = set(mapping) - allowed
    if unknown:
        raise ValueError(
            f"{context} contains unsupported fields: {', '.join(sorted(unknown))}"
        )


def _tuple_int(values: Sequence[Any], count: int, context: str) -> tuple[int, ...]:
    if len(values) != count:
        raise ValueError(f"{context} requires {count} values")
    return tuple(int(value) for value in values)


@dataclass(frozen=True)
class Face:
    vertices: tuple[int, int, int]
    color: tuple[int, int, int] = (128, 128, 128)
    texture: int | None = None
    uv: tuple[tuple[int, int], tuple[int, int], tuple[int, int]] = (
        (0, 0),
        (0, 0),
        (0, 0),
    )
    # Native renderer semantics.  packet_kind=None requests the ordinary
    # authored flat/untextured or flat/textured triangle chosen by the
    # compiler.  Imported retail packets always carry an explicit kind.
    packet_index: int | None = None
    packet_kind: int | None = None
    packet_flags: int = 0
    normal_indices: tuple[int, ...] = ()
    material_parameter: int = 0
    texture_flags: int = 0
    gouraud_colors: tuple[tuple[int, int, int], ...] = ()
    environment_parameters: tuple[int, ...] = ()
    post_parameter: int = 0

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "Face":
        _strict(
            value,
            {
                "vertices",
                "color",
                "texture",
                "uv",
                "packet_index",
                "packet_kind",
                "packet_flags",
                "normal_indices",
                "material_parameter",
                "texture_flags",
                "gouraud_colors",
                "environment_parameters",
                "post_parameter",
            },
            "face",
        )
        uv = value.get("uv", ((0, 0), (0, 0), (0, 0)))
        if len(uv) != 3:
            raise ValueError("face UV data requires three pairs")
        return cls(
            vertices=_tuple_int(value["vertices"], 3, "face vertices"),
            color=_tuple_int(value.get("color", (128, 128, 128)), 3, "face color"),
            texture=(
                None if value.get("texture") is None else int(value["texture"])
            ),
            uv=tuple(_tuple_int(pair, 2, "UV pair") for pair in uv),
            packet_index=(
                None
                if value.get("packet_index") is None
                else int(value["packet_index"])
            ),
            packet_kind=(
                None
                if value.get("packet_kind") is None
                else int(value["packet_kind"])
            ),
            packet_flags=int(value.get("packet_flags", 0)),
            normal_indices=tuple(
                int(item) for item in value.get("normal_indices", ())
            ),
            material_parameter=int(value.get("material_parameter", 0)),
            texture_flags=int(value.get("texture_flags", 0)),
            gouraud_colors=tuple(
                _tuple_int(item, 3, "Gouraud color")
                for item in value.get("gouraud_colors", ())
            ),
            environment_parameters=tuple(
                int(item)
                for item in value.get("environment_parameters", ())
            ),
            post_parameter=int(value.get("post_parameter", 0)),
        )


@dataclass(frozen=True)
class RenderControl:
    """A native non-triangle packet with decoded fixed-width control vectors."""

    packet_index: int
    packet_kind: int
    packet_flags: int
    color: tuple[int, int, int]
    vertex_indices: tuple[int, int, int]
    entries: tuple[tuple[int, int, int, int], ...]

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "RenderControl":
        _strict(
            value,
            {
                "packet_index",
                "packet_kind",
                "packet_flags",
                "color",
                "vertex_indices",
                "entries",
            },
            "render control",
        )
        return cls(
            packet_index=int(value["packet_index"]),
            packet_kind=int(value["packet_kind"]),
            packet_flags=int(value.get("packet_flags", 0)),
            color=_tuple_int(value.get("color", (0, 0, 0)), 3, "control color"),
            vertex_indices=_tuple_int(
                value["vertex_indices"], 3, "control vertex indices"
            ),
            entries=tuple(
                _tuple_int(entry, 4, "control entry")
                for entry in value.get("entries", ())
            ),
        )


@dataclass(frozen=True)
class RenderGroup:
    name: str
    scale_shift: int
    vertices: tuple[tuple[int, int, int], ...]
    faces: tuple[Face, ...]
    normals: tuple[tuple[int, int, int, int], ...] = ()
    controls: tuple[RenderControl, ...] = ()

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "RenderGroup":
        _strict(
            value,
            {
                "name",
                "scale_shift",
                "vertices",
                "normals",
                "faces",
                "controls",
            },
            "render group",
        )
        return cls(
            name=str(value["name"]),
            scale_shift=int(value.get("scale_shift", 8)),
            vertices=tuple(
                _tuple_int(vertex, 3, "render-group vertex")
                for vertex in value["vertices"]
            ),
            faces=tuple(Face.from_dict(face) for face in value["faces"]),
            normals=tuple(
                _tuple_int(normal, 4, "render-group normal")
                for normal in value.get("normals", ())
            ),
            controls=tuple(
                RenderControl.from_dict(control)
                for control in value.get("controls", ())
            ),
        )


@dataclass(frozen=True)
class Slot:
    name: str
    render_group: int | None
    collision: int | None
    render_flags: int = 0
    key: int | None = None
    position: tuple[int, int, int] = (0, 0, 0)
    rotation_yxz: tuple[int, int, int] = (0, 0, 0)
    flags: int = 0
    parent: int | None = None

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "Slot":
        _strict(
            value,
            {
                "name",
                "render_group",
                "render_flags",
                "collision",
                "key",
                "position",
                "rotation_yxz",
                "flags",
                "parent",
            },
            "slot",
        )
        return cls(
            name=str(value["name"]),
            render_group=(
                None
                if value.get("render_group") is None
                else int(value["render_group"])
            ),
            render_flags=int(value.get("render_flags", 0)),
            collision=(
                None if value.get("collision") is None else int(value["collision"])
            ),
            key=None if value.get("key") is None else int(value["key"]),
            position=_tuple_int(value.get("position", (0, 0, 0)), 3, "slot position"),
            rotation_yxz=_tuple_int(
                value.get("rotation_yxz", (0, 0, 0)), 3, "slot rotation"
            ),
            flags=int(value.get("flags", 0)),
            parent=None if value.get("parent") is None else int(value["parent"]),
        )


@dataclass(frozen=True)
class CollisionAabb:
    minimum: tuple[int, int, int]
    maximum: tuple[int, int, int]
    override_y: int
    flags: int = 0

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "CollisionAabb":
        _strict(
            value, {"kind", "minimum", "maximum", "override_y", "flags"}, "AABB"
        )
        return cls(
            minimum=_tuple_int(value["minimum"], 3, "AABB minimum"),
            maximum=_tuple_int(value["maximum"], 3, "AABB maximum"),
            override_y=int(value.get("override_y", value["minimum"][1])),
            flags=int(value.get("flags", 0)),
        )


@dataclass(frozen=True)
class CollisionPlane:
    normal: tuple[int, int, int]
    offset: int
    pad: int = 0

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "CollisionPlane":
        _strict(value, {"normal", "offset", "pad"}, "collision plane")
        return cls(
            normal=_tuple_int(value["normal"], 3, "collision normal"),
            offset=int(value["offset"]),
            pad=int(value.get("pad", 0)),
        )


@dataclass(frozen=True)
class CollisionConvex:
    planes: tuple[CollisionPlane, ...]

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "CollisionConvex":
        _strict(value, {"kind", "planes"}, "convex collision")
        return cls(
            planes=tuple(
                CollisionPlane.from_dict(plane) for plane in value["planes"]
            )
        )


CollisionShape = CollisionAabb | CollisionConvex


@dataclass(frozen=True)
class CollisionStream:
    name: str
    shapes: tuple[CollisionShape, ...]

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "CollisionStream":
        _strict(value, {"name", "shapes"}, "collision stream")
        shapes = []
        for item in value["shapes"]:
            kind = item.get("kind")
            if kind == "aabb":
                shapes.append(CollisionAabb.from_dict(item))
            elif kind == "convex":
                shapes.append(CollisionConvex.from_dict(item))
            else:
                raise ValueError(f"unsupported collision shape kind {kind!r}")
        return cls(name=str(value["name"]), shapes=tuple(shapes))


@dataclass(frozen=True)
class Texture:
    name: str
    width: int
    height: int
    depth: int
    palette_bgr555: tuple[int, ...]
    indices: bytes
    compressed: bool = False
    direct_pixels_bgr555: tuple[int, ...] = ()
    palette_origin: tuple[int, int] = (0, 0)
    image_origin: tuple[int, int] = (0, 0)

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "Texture":
        _strict(
            value,
            {
                "name",
                "width",
                "height",
                "depth",
                "palette_bgr555",
                "indices",
                "compressed",
                "direct_pixels_bgr555",
                "palette_origin",
                "image_origin",
            },
            "texture",
        )
        return cls(
            name=str(value["name"]),
            width=int(value["width"]),
            height=int(value["height"]),
            depth=int(value["depth"]),
            palette_bgr555=tuple(int(color) for color in value["palette_bgr555"]),
            indices=bytes(int(index) for index in value["indices"]),
            compressed=bool(value.get("compressed", False)),
            direct_pixels_bgr555=tuple(
                int(color)
                for color in value.get("direct_pixels_bgr555", ())
            ),
            palette_origin=_tuple_int(
                value.get("palette_origin", (0, 0)), 2, "palette origin"
            ),
            image_origin=_tuple_int(
                value.get("image_origin", (0, 0)), 2, "image origin"
            ),
        )


@dataclass(frozen=True)
class BankMemoryUsage:
    texture_count: int
    texels: int
    native_texture_bytes: int
    expanded_rgba_bytes: int


def bank_memory_usage(
    bank: "VehicleProject | ObjectBank",
) -> BankMemoryUsage:
    texels = sum(texture.width * texture.height for texture in bank.textures)
    native_bytes = sum(
        (
            texture.width * texture.height * 2
            if texture.depth == 2
            else (
                texture.width
                * texture.height
                + (3 if texture.depth == 0 else 1)
            )
            // (4 if texture.depth == 0 else 2)
            * 2
        )
        + len(texture.palette_bgr555) * 2
        for texture in bank.textures
    )
    return BankMemoryUsage(
        texture_count=len(bank.textures),
        texels=texels,
        native_texture_bytes=native_bytes,
        expanded_rgba_bytes=texels * 4,
    )


@dataclass(frozen=True)
class TextureBinding:
    target: int
    texture: int

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "TextureBinding":
        _strict(value, {"target", "texture"}, "animation texture binding")
        return cls(
            target=int(value["target"]),
            texture=int(value["texture"]),
        )


@dataclass(frozen=True)
class AnimationFrame:
    frame_delta: int
    rotation_yxz: tuple[int, int, int] | None = None
    rotation_parameter: int = 0
    translation_absolute: tuple[int, int, int] | None = None
    translation_delta: tuple[int, int, int] | None = None
    texture_bindings: tuple[TextureBinding, ...] = ()
    scale: tuple[int, int, int, int] | None = None
    mesh_vectors: tuple[tuple[int, int, int, int], ...] = ()

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "AnimationFrame":
        _strict(
            value,
            {
                "frame_delta",
                "rotation_yxz",
                "rotation_parameter",
                "translation_absolute",
                "translation_delta",
                "texture_bindings",
                "scale",
                "mesh_vectors",
            },
            "animation frame",
        )

        def optional(name: str, count: int):
            source = value.get(name)
            return None if source is None else _tuple_int(source, count, name)

        return cls(
            frame_delta=int(value["frame_delta"]),
            rotation_yxz=optional("rotation_yxz", 3),
            rotation_parameter=int(value.get("rotation_parameter", 0)),
            translation_absolute=optional("translation_absolute", 3),
            translation_delta=optional("translation_delta", 3),
            texture_bindings=tuple(
                TextureBinding.from_dict(binding)
                for binding in value.get("texture_bindings", ())
            ),
            scale=optional("scale", 4),
            mesh_vectors=tuple(
                _tuple_int(vector, 4, "mesh animation vector")
                for vector in value.get("mesh_vectors", ())
            ),
        )


@dataclass(frozen=True)
class SlotAnimation:
    slot: int
    loop: bool
    frames: tuple[AnimationFrame, ...]

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "SlotAnimation":
        _strict(value, {"slot", "loop", "frames"}, "slot animation")
        return cls(
            slot=int(value["slot"]),
            loop=bool(value.get("loop", True)),
            frames=tuple(
                AnimationFrame.from_dict(frame) for frame in value["frames"]
            ),
        )


@dataclass(frozen=True)
class ObjectBank:
    """A complete independently compiled native XOBF object bank."""

    groups: tuple[RenderGroup, ...]
    slots: tuple[Slot, ...]
    collisions: tuple[CollisionStream, ...]
    textures: tuple[Texture, ...]
    animations: tuple[SlotAnimation, ...]

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "ObjectBank":
        _strict(
            value,
            {"groups", "slots", "collisions", "textures", "animations"},
            "object bank",
        )
        return cls(
            groups=tuple(RenderGroup.from_dict(group) for group in value["groups"]),
            slots=tuple(Slot.from_dict(slot) for slot in value["slots"]),
            collisions=tuple(
                CollisionStream.from_dict(stream)
                for stream in value.get("collisions", ())
            ),
            textures=tuple(
                Texture.from_dict(texture)
                for texture in value.get("textures", ())
            ),
            animations=tuple(
                SlotAnimation.from_dict(animation)
                for animation in value.get("animations", ())
            ),
        )


@dataclass(frozen=True)
class VehicleProject:
    schema_version: int
    stable_id: str
    display_name: str
    game: str
    groups: tuple[RenderGroup, ...]
    slots: tuple[Slot, ...]
    collisions: tuple[CollisionStream, ...]
    textures: tuple[Texture, ...]
    animations: tuple[SlotAnimation, ...]
    stats: Mapping[str, int]
    body_kind: int = 0
    transformation_bank: ObjectBank | None = None
    selector_preview_bank: ObjectBank | None = None
    selector_preview_body_kind: int = 0
    transform_modes: tuple[tuple[int, ...], ...] = ()
    powerups: Mapping[str, int] = field(default_factory=dict)
    quest: Mapping[str, Any] = field(default_factory=dict)
    unlock: Mapping[str, Any] = field(default_factory=dict)
    sounds: tuple[Mapping[str, Any], ...] = ()

    @classmethod
    def from_dict(cls, value: Mapping[str, Any]) -> "VehicleProject":
        _strict(
            value,
            {
                "schema_version",
                "stable_id",
                "display_name",
                "game",
                "groups",
                "slots",
                "collisions",
                "textures",
                "animations",
                "stats",
                "body_kind",
                "transformation_bank",
                "selector_preview_bank",
                "selector_preview_body_kind",
                "transform_modes",
                "powerups",
                "quest",
                "unlock",
                "sounds",
            },
            "vehicle project",
        )
        normalized_game = str(value["game"]).upper().replace(":", "_")
        from . import stats as vehicle_stats

        project = cls(
            schema_version=int(value["schema_version"]),
            stable_id=str(value["stable_id"]),
            display_name=str(value["display_name"]),
            game=normalized_game,
            groups=tuple(
                RenderGroup.from_dict(group) for group in value["groups"]
            ),
            slots=tuple(Slot.from_dict(slot) for slot in value["slots"]),
            collisions=tuple(
                CollisionStream.from_dict(stream)
                for stream in value.get("collisions", ())
            ),
            textures=tuple(
                Texture.from_dict(texture) for texture in value.get("textures", ())
            ),
            animations=tuple(
                SlotAnimation.from_dict(animation)
                for animation in value.get("animations", ())
            ),
            stats=vehicle_stats.canonicalize_values(
                normalized_game, value["stats"]
            ),
            body_kind=int(value.get("body_kind", 0)),
            transformation_bank=(
                None
                if value.get("transformation_bank") is None
                else ObjectBank.from_dict(value["transformation_bank"])
            ),
            selector_preview_bank=(
                None
                if value.get("selector_preview_bank") is None
                else ObjectBank.from_dict(value["selector_preview_bank"])
            ),
            selector_preview_body_kind=int(
                value.get("selector_preview_body_kind", 0)
            ),
            transform_modes=tuple(
                _tuple_int(
                    mode, V82_TRANSFORM_WHEEL_COUNT, "transformation wheel mode"
                )
                for mode in value.get("transform_modes", ())
            ),
            powerups={
                str(name): int(number)
                for name, number in value.get("powerups", {}).items()
            },
            quest=dict(value.get("quest", {})),
            unlock=dict(value.get("unlock", {})),
            sounds=tuple(dict(sound) for sound in value.get("sounds", ())),
        )
        project.validate()
        return project

    def validate(self) -> None:
        if self.schema_version != SCHEMA_VERSION:
            raise ValueError(
                f"vehicle schema {self.schema_version} is unsupported; "
                f"expected {SCHEMA_VERSION}"
            )
        if self.game not in {"V8", "V8_2"}:
            raise ValueError("game must be V8 or V8_2")
        if not self.stable_id or any(
            character not in "abcdefghijklmnopqrstuvwxyz0123456789_.-"
            for character in self.stable_id
        ):
            raise ValueError(
                "stable_id must use lowercase ASCII letters, digits, _, -, or ."
            )
        self._validate_bank(self, "body")
        roots = {
            index for index, slot in enumerate(self.slots) if slot.parent is None
        }
        if self.body_kind not in roots:
            raise ValueError("body_kind must select a top-level body object")
        if self.transformation_bank is not None:
            self._validate_bank(self.transformation_bank, "transformation")
        if self.selector_preview_bank is not None:
            self._validate_bank(self.selector_preview_bank, "selector preview")
            preview_roots = {
                index
                for index, slot in enumerate(self.selector_preview_bank.slots)
                if slot.parent is None
            }
            if self.selector_preview_body_kind not in preview_roots:
                raise ValueError(
                    "selector_preview_body_kind must select a top-level "
                    "selector preview object"
                )
        wheel_roots = (
            roots
            if self.game == "V8"
            else (
                set()
                if self.transformation_bank is None
                else {
                    index
                    for index, slot in enumerate(self.transformation_bank.slots)
                    if slot.parent is None
                }
            )
        )
        for name in ("wheel_kind_front", "wheel_kind_rear"):
            wheel_kind = self.stats.get(name)
            if wheel_kind is not None and wheel_kind not in wheel_roots:
                bank_name = "body" if self.game == "V8" else "wheel/transformation"
                raise ValueError(
                    f"{name} must select a top-level object in the {bank_name} bank"
                )
        if self.game == "V8":
            if self.transformation_bank is not None or self.transform_modes:
                raise ValueError("native transformation banks are V8:2-only")
            if self.powerups:
                raise ValueError("native vehicle powerup profiles are V8:2-only")
        else:
            if len(self.transform_modes) != V82_TRANSFORM_MODE_COUNT:
                raise ValueError("V8:2 requires four explicit transformation modes")
            if any(self.transform_modes[0]):
                raise ValueError("V8:2 transformation mode 0 must use normal wheels")
            transform_roots = (
                set()
                if self.transformation_bank is None
                else {
                    index
                    for index, slot in enumerate(self.transformation_bank.slots)
                    if slot.parent is None
                }
            )
            for mode_index, mode in enumerate(self.transform_modes[1:], 1):
                for wheel_index, kind in enumerate(mode):
                    if kind < 0 or kind > 0xFFFF:
                        raise ValueError(
                            f"transform mode {mode_index} wheel {wheel_index} "
                            "kind must fit unsigned 16-bit"
                        )
                    if kind not in transform_roots:
                        raise ValueError(
                            f"transform mode {mode_index} wheel {wheel_index} "
                            "must select a top-level transformation-bank object"
                        )
                    root_slot = self.transformation_bank.slots[kind]
                    if root_slot.collision is None or root_slot.flags != -21846:
                        raise ValueError(
                            f"transform mode {mode_index} wheel {wheel_index} "
                            "root needs owned collision and native 0xAAAA object flags"
                        )
                    has_instance_marker = any(
                        slot.parent == kind and
                        slot.render_group is not None and
                        slot.render_flags & 0xC000 == 0xC000
                        for slot in self.transformation_bank.slots
                    )
                    if not has_instance_marker:
                        raise ValueError(
                            f"transform mode {mode_index} wheel {wheel_index} "
                            "root needs an explicit native instance-marker child"
                        )
            supplied_powerups = set(self.powerups)
            required_powerups = set(V82_POWERUP_FIELDS)
            if supplied_powerups != required_powerups:
                missing = sorted(required_powerups - supplied_powerups)
                extra = sorted(supplied_powerups - required_powerups)
                detail = []
                if missing:
                    detail.append("missing " + ", ".join(missing))
                if extra:
                    detail.append("unsupported " + ", ".join(extra))
                raise ValueError(
                    "V8:2 powerups must be explicit: " + "; ".join(detail)
                )
            from .stats import V82_POWERUP_SETTINGS
            powerup_specs = {
                setting.name: setting for setting in V82_POWERUP_SETTINGS
            }
            for name, value in self.powerups.items():
                setting = powerup_specs[name]
                if not setting.author_min <= value <= setting.author_max:
                    raise ValueError(
                        f"V8:2 {name} must be within the safe authoring "
                        f"bound {setting.author_min}..{setting.author_max}"
                    )
        from .stats import PROFILES

        required_stats = {
            field.name
            for field in PROFILES[self.game].authoring_fields
            if field.name != "vehicle_type"
        }
        supplied_stats = set(self.stats)
        if supplied_stats != required_stats:
            missing = sorted(required_stats - supplied_stats)
            extra = sorted(supplied_stats - required_stats)
            detail = []
            if missing:
                detail.append("missing " + ", ".join(missing))
            if extra:
                detail.append("unsupported " + ", ".join(extra))
            raise ValueError("stats must be explicit: " + "; ".join(detail))
        for field in PROFILES[self.game].authoring_fields:
            if field.name == "vehicle_type":
                continue
            value = self.stats[field.name]
            minimum = (
                field.authoring_min if field.editable else field.native_min
            )
            maximum = (
                field.authoring_max if field.editable else field.native_max
            )
            if value < minimum or value > maximum:
                raise ValueError(
                    f"{field.author_name}={value} is outside its "
                    f"{minimum}..{maximum} authoring bound"
                )
        if (
            self.game == "V8_2"
            and self.stats["rear_suspension_damping"] < 0
        ):
            raise ValueError(
                "rear_suspension_damping must be 0..32767 because damping "
                "opposes suspension motion"
            )
        if self.quest:
            raise ValueError(
                "quest authoring schema is not decoded yet; opaque quest data is forbidden"
            )
        if self.unlock:
            raise ValueError(
                "unlock authoring schema is not decoded yet; opaque unlock data is forbidden"
            )
        if self.sounds:
            raise ValueError(
                "sound authoring schema is not decoded yet; opaque SND data is forbidden"
            )

    def _validate_bank(
        self, bank: "VehicleProject | ObjectBank", label: str
    ) -> None:
        if not bank.groups:
            raise ValueError(f"{label} bank needs at least one render group")
        if not bank.slots:
            raise ValueError(f"{label} bank needs at least one object slot")
        for group_index, group in enumerate(bank.groups):
            if not 0 <= group.scale_shift <= 15:
                raise ValueError(
                    f"{label} group {group_index} scale_shift must be 0..15"
                )
            if not group.vertices:
                raise ValueError(f"{label} group {group_index} contains no vertices")
            for vertex in group.vertices:
                if any(component < -32768 or component > 32767 for component in vertex):
                    raise ValueError(
                        f"{label} group {group_index} vertex exceeds signed 16-bit"
                    )
            for normal in group.normals:
                if any(component < -32768 or component > 32767 for component in normal):
                    raise ValueError(
                        f"{label} group {group_index} normal exceeds signed 16-bit"
                    )
            packet_indices = []
            for face in group.faces:
                if any(index < 0 or index >= len(group.vertices) for index in face.vertices):
                    raise ValueError(
                        f"{label} group {group_index} face has an invalid vertex"
                    )
                if any(component < 0 or component > 255 for component in face.color):
                    raise ValueError(
                        f"{label} group {group_index} face color is outside 0..255"
                    )
                if face.texture is not None and not 0 <= face.texture < len(bank.textures):
                    raise ValueError(
                        f"{label} group {group_index} references an invalid texture"
                    )
                if any(
                    component < 0 or component > 255
                    for pair in face.uv
                    for component in pair
                ):
                    raise ValueError(
                        f"{label} group {group_index} UV is outside 0..255"
                    )
                if face.packet_kind is not None and not 0 <= face.packet_kind <= 15:
                    raise ValueError(
                        f"{label} group {group_index} packet kind is outside 0..15"
                    )
                if face.packet_kind == 14:
                    raise ValueError(
                        f"{label} group {group_index} uses unsupported native kind 14"
                    )
                if face.packet_flags < 0 or face.packet_flags & ~0xF0:
                    raise ValueError(
                        f"{label} group {group_index} packet flags contain unknown bits"
                    )
                if face.texture_flags < 0 or face.texture_flags & ~0xC000:
                    raise ValueError(
                        f"{label} group {group_index} texture flags contain unknown bits"
                    )
                if any(
                    index < 0 or index >= len(group.normals)
                    for index in face.normal_indices
                ):
                    raise ValueError(
                        f"{label} group {group_index} face has an invalid normal"
                    )
                if any(
                    component < 0 or component > 255
                    for color in face.gouraud_colors
                    for component in color
                ):
                    raise ValueError(
                        f"{label} group {group_index} Gouraud color is outside 0..255"
                    )
                for scalar in (
                    face.material_parameter,
                    face.post_parameter,
                    *face.environment_parameters,
                ):
                    if scalar < 0 or scalar > 0xFFFF:
                        raise ValueError(
                            f"{label} group {group_index} packet parameter "
                            "must fit unsigned 16-bit"
                        )
                if face.packet_index is not None:
                    packet_indices.append(face.packet_index)
            for control in group.controls:
                if control.packet_kind != 10:
                    raise ValueError(
                        f"{label} group {group_index} only kind-10 controls "
                        "are currently defined"
                    )
                if control.packet_flags < 0 or control.packet_flags & ~0xF0:
                    raise ValueError(
                        f"{label} group {group_index} control flags contain unknown bits"
                    )
                if any(
                    index < 0 or index >= len(group.vertices)
                    for index in control.vertex_indices
                ):
                    raise ValueError(
                        f"{label} group {group_index} control has an invalid vertex"
                    )
                if any(
                    scalar < 0 or scalar > 0xFFFF
                    for entry in control.entries
                    for scalar in entry
                ):
                    raise ValueError(
                        f"{label} group {group_index} control entry must fit "
                        "unsigned 16-bit"
                    )
                packet_indices.append(control.packet_index)
            if packet_indices:
                expected = list(range(len(group.faces) + len(group.controls)))
                if sorted(packet_indices) != expected:
                    raise ValueError(
                        f"{label} group {group_index} native packet order "
                        "must be contiguous and unique"
                    )
        roots = 0
        for slot_index, slot in enumerate(bank.slots):
            if slot.key is not None and slot.render_group is not None:
                raise ValueError(
                    f"{label} slot {slot_index} cannot have both a render group and an "
                    "engine attachment key"
                )
            if slot.key is not None and slot.render_flags != 0:
                raise ValueError(
                    f"{label} slot {slot_index} cannot combine a native key "
                    "with render flags"
                )
            if slot.render_group is None and slot.render_flags != 0:
                raise ValueError(
                    f"{label} slot {slot_index} render flags require a render group"
                )
            render_flag_mask = 0xFF00 if self.game == "V8" else 0xF800
            if slot.render_flags < 0 or slot.render_flags & ~render_flag_mask:
                raise ValueError(
                    f"{label} slot {slot_index} render-class bits contain "
                    "non-native values"
                )
            group_mask = 0x00FF if self.game == "V8" else 0x07FF
            if (
                slot.render_group is not None
                and slot.render_group > group_mask
            ):
                raise ValueError(
                    f"{label} slot {slot_index} render group exceeds the "
                    f"{self.game} native object-field width"
                )
            if slot.key is not None and not 0 <= slot.key < 0xFFFF:
                raise ValueError(
                    f"{label} slot {slot_index} engine attachment key must be 0..65534"
                )
            if slot.render_group is not None and not (
                0 <= slot.render_group < len(bank.groups)
            ):
                raise ValueError(
                    f"{label} slot {slot_index} has an invalid render group"
                )
            if slot.collision is not None and not (
                0 <= slot.collision < len(bank.collisions)
            ):
                raise ValueError(
                    f"{label} slot {slot_index} has an invalid collision stream"
                )
            if slot.parent is None:
                roots += 1
            elif slot.parent < 0 or slot.parent >= len(bank.slots):
                raise ValueError(f"{label} slot {slot_index} has an invalid parent")
            elif slot.parent == slot_index:
                raise ValueError(f"{label} slot {slot_index} cannot parent itself")
        if roots == 0:
            raise ValueError(f"{label} bank requires at least one top-level object")
        for slot_index in range(len(bank.slots)):
            visited = set()
            current = slot_index
            while current is not None:
                if current in visited:
                    raise ValueError(f"{label} slot hierarchy contains a cycle")
                visited.add(current)
                current = bank.slots[current].parent
        usage = bank_memory_usage(bank)
        if usage.texture_count > MAX_TEXTURES_PER_BANK:
            raise ValueError(
                f"{label} bank has {usage.texture_count} textures; "
                f"limit is {MAX_TEXTURES_PER_BANK}"
            )
        if usage.texels > MAX_TEXTURE_TEXELS_PER_BANK:
            raise ValueError(
                f"{label} bank has {usage.texels} texture texels; "
                f"limit is {MAX_TEXTURE_TEXELS_PER_BANK}"
            )
        if usage.native_texture_bytes > MAX_NATIVE_TEXTURE_BYTES_PER_BANK:
            raise ValueError(
                f"{label} bank uses {usage.native_texture_bytes} native texture "
                f"bytes; limit is {MAX_NATIVE_TEXTURE_BYTES_PER_BANK}"
            )
        for texture_index, texture in enumerate(bank.textures):
            if (
                texture.width < 1 or texture.width > MAX_TEXTURE_DIMENSION or
                texture.height < 1 or texture.height > MAX_TEXTURE_DIMENSION
            ):
                raise ValueError(
                    f"{label} texture {texture_index} dimensions must be "
                    f"1..{MAX_TEXTURE_DIMENSION}"
                )
            expected_palette = (
                16
                if texture.depth == 0
                else 256
                if texture.depth == 1
                else 0
            )
            if texture.depth == 2:
                if texture.palette_bgr555 or texture.indices:
                    raise ValueError(
                        f"{label} direct-color texture {texture_index} "
                        "must not contain a palette or palette indices"
                    )
                if (
                    len(texture.direct_pixels_bgr555)
                    != texture.width * texture.height
                ):
                    raise ValueError(
                        f"{label} direct-color texture {texture_index} "
                        "pixel count is incorrect"
                    )
                if any(
                    color < 0 or color > 0xFFFF
                    for color in texture.direct_pixels_bgr555
                ):
                    raise ValueError(
                        f"{label} direct-color texture {texture_index} "
                        "contains a value outside BGR555"
                    )
            elif expected_palette == 0:
                raise ValueError(
                    f"{label} texture {texture_index} depth must be 0, 1, or 2"
                )
            elif not 1 <= len(texture.palette_bgr555) <= 256:
                raise ValueError(
                    f"{label} texture {texture_index} requires "
                    "1..256 native CLUT entries"
                )
            elif len(texture.indices) != texture.width * texture.height:
                raise ValueError(
                    f"{label} texture {texture_index} pixel count is incorrect"
                )
            elif any(
                index >= len(texture.palette_bgr555)
                or index >= expected_palette
                for index in texture.indices
            ):
                raise ValueError(
                    f"{label} texture {texture_index} has an invalid palette index"
                )
            if texture.depth != 2 and texture.direct_pixels_bgr555:
                raise ValueError(
                    f"{label} indexed texture {texture_index} must not "
                    "contain direct-color pixels"
                )
            if texture.compressed and self.game != "V8_2":
                raise ValueError("native compressed textures are V8:2-only")
        animated_slots = set()
        for animation in bank.animations:
            if animation.slot < 0 or animation.slot >= len(bank.slots):
                raise ValueError(f"{label} animation references an invalid slot")
            if animation.slot in animated_slots:
                raise ValueError(
                    f"{label} slot may have only one animation stream"
                )
            animated_slots.add(animation.slot)
            if not animation.frames:
                raise ValueError(
                    f"{label} animation stream requires at least one frame"
                )
            for frame_index, frame in enumerate(animation.frames):
                for binding in frame.texture_bindings:
                    if binding.target < 0:
                        raise ValueError(
                            f"{label} animation {animation.slot} frame "
                            f"{frame_index} has a negative texture target"
                        )
                    if (
                        binding.texture < 0 or
                        binding.texture >= len(bank.textures)
                    ):
                        raise ValueError(
                            f"{label} animation {animation.slot} frame "
                            f"{frame_index} references invalid texture "
                            f"{binding.texture}"
                        )


def _shape_to_dict(shape: CollisionShape) -> dict[str, Any]:
    if isinstance(shape, CollisionAabb):
        return {
            "kind": "aabb",
            "minimum": list(shape.minimum),
            "maximum": list(shape.maximum),
            "override_y": shape.override_y,
            "flags": shape.flags,
        }
    return {
        "kind": "convex",
        "planes": [
            {
                "normal": list(plane.normal),
                "offset": plane.offset,
                "pad": plane.pad,
            }
            for plane in shape.planes
        ],
    }


def _bank_to_dict(bank: VehicleProject | ObjectBank) -> dict[str, Any]:
    return {
        "groups": [
            {
                "name": group.name,
                "scale_shift": group.scale_shift,
                "vertices": [list(vertex) for vertex in group.vertices],
                "normals": [list(normal) for normal in group.normals],
                "faces": [
                    {
                        "vertices": list(face.vertices),
                        "color": list(face.color),
                        "packet_index": face.packet_index,
                        "packet_kind": face.packet_kind,
                        "packet_flags": face.packet_flags,
                        "normal_indices": list(face.normal_indices),
                        "material_parameter": face.material_parameter,
                        "texture_flags": face.texture_flags,
                        "gouraud_colors": [
                            list(color) for color in face.gouraud_colors
                        ],
                        "environment_parameters": list(
                            face.environment_parameters
                        ),
                        "post_parameter": face.post_parameter,
                        **(
                            {}
                            if face.texture is None
                            else {
                                "texture": face.texture,
                                "uv": [list(pair) for pair in face.uv],
                            }
                        ),
                    }
                    for face in group.faces
                ],
                "controls": [
                    {
                        "packet_index": control.packet_index,
                        "packet_kind": control.packet_kind,
                        "packet_flags": control.packet_flags,
                        "color": list(control.color),
                        "vertex_indices": list(control.vertex_indices),
                        "entries": [list(entry) for entry in control.entries],
                    }
                    for control in group.controls
                ],
            }
            for group in bank.groups
        ],
        "slots": [
            {
                "name": slot.name,
                "render_group": slot.render_group,
                "render_flags": slot.render_flags,
                "collision": slot.collision,
                "key": slot.key,
                "position": list(slot.position),
                "rotation_yxz": list(slot.rotation_yxz),
                "flags": slot.flags,
                "parent": slot.parent,
            }
            for slot in bank.slots
        ],
        "collisions": [
            {
                "name": stream.name,
                "shapes": [_shape_to_dict(shape) for shape in stream.shapes],
            }
            for stream in bank.collisions
        ],
        "textures": [
            {
                "name": texture.name,
                "width": texture.width,
                "height": texture.height,
                "depth": texture.depth,
                "palette_bgr555": list(texture.palette_bgr555),
                "indices": list(texture.indices),
                "compressed": texture.compressed,
                "direct_pixels_bgr555": list(
                    texture.direct_pixels_bgr555
                ),
                "palette_origin": list(texture.palette_origin),
                "image_origin": list(texture.image_origin),
            }
            for texture in bank.textures
        ],
        "animations": [
            {
                "slot": animation.slot,
                "loop": animation.loop,
                "frames": [
                    {
                        "frame_delta": frame.frame_delta,
                        **(
                            {}
                            if frame.rotation_yxz is None
                            else {
                                "rotation_yxz": list(frame.rotation_yxz),
                                "rotation_parameter": frame.rotation_parameter,
                            }
                        ),
                        **(
                            {}
                            if frame.translation_absolute is None
                            else {
                                "translation_absolute": list(
                                    frame.translation_absolute
                                )
                            }
                        ),
                        **(
                            {}
                            if frame.translation_delta is None
                            else {
                                "translation_delta": list(frame.translation_delta)
                            }
                        ),
                        **(
                            {}
                            if not frame.texture_bindings
                            else {
                                "texture_bindings": [
                                    {
                                        "target": binding.target,
                                        "texture": binding.texture,
                                    }
                                    for binding in frame.texture_bindings
                                ]
                            }
                        ),
                        **(
                            {}
                            if frame.scale is None
                            else {"scale": list(frame.scale)}
                        ),
                        **(
                            {}
                            if not frame.mesh_vectors
                            else {
                                "mesh_vectors": [
                                    list(vector) for vector in frame.mesh_vectors
                                ]
                            }
                        ),
                    }
                    for frame in animation.frames
                ],
            }
            for animation in bank.animations
        ],
    }


def to_dict(vehicle: VehicleProject) -> dict[str, Any]:
    """Serialize a strict project without adding passthrough or opaque fields."""

    vehicle.validate()
    from . import stats as vehicle_stats

    result = {
        "schema_version": vehicle.schema_version,
        "stable_id": vehicle.stable_id,
        "display_name": vehicle.display_name,
        "game": vehicle.game,
        "body_kind": vehicle.body_kind,
        **_bank_to_dict(vehicle),
        "stats": vehicle_stats.authoring_values(vehicle.game, vehicle.stats),
        "transform_modes": [list(mode) for mode in vehicle.transform_modes],
        "powerups": dict(vehicle.powerups),
        "quest": dict(vehicle.quest),
        "unlock": dict(vehicle.unlock),
        "sounds": [dict(sound) for sound in vehicle.sounds],
    }
    result["transformation_bank"] = (
        None
        if vehicle.transformation_bank is None
        else _bank_to_dict(vehicle.transformation_bank)
    )
    result["selector_preview_bank"] = (
        None
        if vehicle.selector_preview_bank is None
        else _bank_to_dict(vehicle.selector_preview_bank)
    )
    result["selector_preview_body_kind"] = vehicle.selector_preview_body_kind
    return result
