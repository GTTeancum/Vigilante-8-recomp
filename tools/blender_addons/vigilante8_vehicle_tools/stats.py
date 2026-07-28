"""Editable, source-backed vehicle stat records in the retail PS-X EXEs."""

from __future__ import annotations

from dataclasses import dataclass
import struct
from typing import Iterator, Mapping

from .binary import FormatError, require_range, u32le


@dataclass(frozen=True)
class FieldSpec:
    name: str
    author_name: str
    label: str
    offset: int
    format: str
    description: str
    advanced: bool = False
    editable: bool = True
    unit: str = "native integer"
    lower_effect: str = ""
    higher_effect: str = ""
    recommended_min: int | None = None
    recommended_max: int | None = None
    author_min: int | None = None
    author_max: int | None = None
    default: int = 0
    confidence: str = "HIGH"

    @property
    def size(self) -> int:
        return struct.calcsize("<" + self.format)

    @property
    def native_min(self) -> int:
        return {
            "b": -0x80,
            "B": 0,
            "h": -0x8000,
            "H": 0,
            "i": -0x80000000,
            "I": 0,
        }[self.format]

    @property
    def native_max(self) -> int:
        return {
            "b": 0x7F,
            "B": 0xFF,
            "h": 0x7FFF,
            "H": 0xFFFF,
            "i": 0x7FFFFFFF,
            "I": 0xFFFFFFFF,
        }[self.format]

    @property
    def authoring_min(self) -> int:
        return self.native_min if self.author_min is None else self.author_min

    @property
    def authoring_max(self) -> int:
        return self.native_max if self.author_max is None else self.author_max

    @property
    def help_text(self) -> str:
        parts = [self.description, f"Unit: {self.unit}."]
        if self.lower_effect:
            parts.append(f"Lower: {self.lower_effect}.")
        if self.higher_effect:
            parts.append(f"Higher: {self.higher_effect}.")
        parts.append(
            f"Native bound: {self.native_min} to {self.native_max}."
        )
        if (
            self.authoring_min != self.native_min
            or self.authoring_max != self.native_max
        ):
            parts.append(
                f"Safe authoring bound: {self.authoring_min} to "
                f"{self.authoring_max}."
            )
        if self.recommended_min is not None and self.recommended_max is not None:
            parts.append(
                "Retail-tested range: "
                f"{self.recommended_min} to {self.recommended_max}."
            )
        parts.append(f"Semantic confidence: {self.confidence}.")
        return " ".join(parts)


@dataclass(frozen=True)
class GameProfile:
    game: str
    table_address: int
    record_size: int
    record_count: int
    fields: tuple[FieldSpec, ...]
    extension_fields: tuple[FieldSpec, ...] = ()

    @property
    def authoring_fields(self) -> tuple[FieldSpec, ...]:
        return self.fields + self.extension_fields


def _field(
    name: str,
    author_name: str,
    label: str,
    offset: int,
    format: str,
    description: str,
    advanced: bool = False,
    **metadata,
) -> FieldSpec:
    return FieldSpec(
        name, author_name, label, offset, format, description, advanced,
        **metadata,
    )


V8_FIELDS = (
    _field("wheel_kind_front", "front_wheel_object", "Front wheel object", 0x00, "H",
           "Top-level body-bank object used for the front-left and front-right wheels.",
           unit="body-bank object reference",
           lower_effect="not a strength setting; selecting a numerically lower object changes which wheel model is used",
           higher_effect="not a strength setting; selecting a numerically higher object changes which wheel model is used"),
    _field("wheel_kind_rear", "rear_wheel_object", "Rear wheel object", 0x02, "H",
           "Top-level body-bank object used for the rear-left and rear-right wheels.",
           unit="body-bank object reference",
           lower_effect="not a strength setting; selecting a numerically lower object changes which wheel model is used",
           higher_effect="not a strength setting; selecting a numerically higher object changes which wheel model is used"),
    _field("front_spring_stiffness", "front_spring_stiffness", "Front spring stiffness", 0x04, "h",
           "How strongly the front suspension pushes back when compressed.",
           unit="native spring-force coefficient", lower_effect="softer front suspension with more compression; zero removes spring force",
           higher_effect="firmer front suspension with less compression", recommended_min=32,
           recommended_max=100, author_min=0, default=64),
    _field("rear_spring_stiffness", "rear_spring_stiffness", "Rear spring stiffness", 0x06, "h",
           "How strongly the rear suspension pushes back when compressed.",
           unit="native spring-force coefficient", lower_effect="softer rear suspension with more compression; zero removes spring force",
           higher_effect="firmer rear suspension with less compression", recommended_min=40,
           recommended_max=112, author_min=0, default=64),
    _field("front_suspension_damping", "front_suspension_damping", "Front suspension damping", 0x08, "h",
           "Resistance to front suspension movement; it controls how quickly bouncing settles after a bump.",
           unit="native damping coefficient", lower_effect="more bounce and slower settling; zero removes damping",
           higher_effect="less bounce and faster settling", recommended_min=80,
           recommended_max=200, author_min=0, default=180),
    _field("rear_suspension_damping", "rear_suspension_damping", "Rear suspension damping", 0x0A, "h",
           "Resistance to rear suspension movement; it controls how quickly bouncing settles after a bump.",
           unit="native damping coefficient", lower_effect="more bounce and slower settling; zero removes damping",
           higher_effect="less bounce and faster settling", recommended_min=64,
           recommended_max=200, author_min=0, default=180),
    _field("wheel_presence_mask", "enabled_wheels", "Enabled wheels", 0x0C, "B",
           "Bit flags for wheel placements: bit 0 front-left, bit 1 front-right, bit 2 rear-left, bit 3 rear-right.",
           unit="four-bit wheel mask",
           lower_effect="not monotonic; clearing a bit disables its specific wheel placement",
           higher_effect="not monotonic; setting a bit enables its specific wheel placement",
           recommended_min=0, recommended_max=15, author_max=15, default=15),
    _field("vehicle_type", "engine_assigned_vehicle_type", "Engine-assigned vehicle type", 0x0D, "B",
           "Roster identity assigned by the engine; authors do not choose it.", editable=False,
           unit="engine identity"),
    _field("maximum_drive_force", "maximum_drive_force", "Maximum drive force", 0x0E, "B",
           "Maximum signed throttle force sent into the wheel-force routine.",
           unit="native drive-force coefficient", lower_effect="slower acceleration",
           higher_effect="stronger acceleration and wheel drive", recommended_min=32,
           recommended_max=255, default=100),
    _field("unused_native_byte_0f", "internal_unused_native_byte_0f", "Unused native byte +0x0F", 0x0F, "B",
           "The constructor copies this byte to vehicle +0xB4, but no V8 runtime read has been found.",
           advanced=True, editable=False, unit="preserved native metadata",
           recommended_min=192, recommended_max=245, confidence="MED"),
    _field("speed_drag_coefficient", "speed_drag_coefficient", "Speed drag coefficient", 0x10, "I",
           "Velocity-proportional drag applied to all three movement axes; this is the main top-speed limiter.",
           unit="native fixed-point drag coefficient", lower_effect="less drag and a higher attainable speed",
           higher_effect="more drag and a lower attainable speed", recommended_min=2075,
           recommended_max=3891, default=3000),
    _field("pitch_angular_response", "pitch_angular_response", "Pitch angular response", 0x14, "h",
           "Inverse rotational inertia around the left-right axis: how readily the nose pitches up or down.",
           unit="native inverse-inertia coefficient", lower_effect="slower pitch rotation; zero prevents pitch response",
           higher_effect="faster pitch rotation from the same torque", recommended_min=64,
           recommended_max=100, author_min=0, default=100),
    _field("yaw_angular_response", "yaw_angular_response", "Yaw angular response", 0x16, "h",
           "Inverse rotational inertia around the vertical axis: how readily the vehicle turns left or right.",
           unit="native inverse-inertia coefficient", lower_effect="slower yaw rotation; zero prevents yaw response",
           higher_effect="faster yaw rotation from the same torque", recommended_min=24,
           recommended_max=64, author_min=0, default=48),
    _field("roll_angular_response", "roll_angular_response", "Roll angular response", 0x18, "h",
           "Inverse rotational inertia around the front-back axis: how readily the body rolls side to side.",
           unit="native inverse-inertia coefficient", lower_effect="slower body roll; zero prevents roll response",
           higher_effect="faster body roll from the same torque", recommended_min=48,
           recommended_max=100, author_min=0, default=64),
    _field("mass", "mass", "Vehicle mass", 0x1A, "H",
           "Mass used by collision momentum and by impulse/knockback calculations.",
           unit="native mass, 64 units = 1 collision-mass unit", lower_effect="more knockback and less collision momentum",
           higher_effect="less knockback and more collision momentum", recommended_min=3072,
           recommended_max=16384, author_min=1, default=6144),
    _field("health", "health", "Chassis health", 0x1C, "H",
           "Initial and maximum chassis health copied to damageable body children.",
           unit="damage points", lower_effect="destroyed by less damage",
           higher_effect="survives more damage", recommended_min=400,
           recommended_max=2175, author_min=1, default=1000),
    _field("low_speed_steering_response", "low_speed_steering_response", "Low-speed steering response", 0x1E, "b",
           "Base steering torque before the speed-dependent adjustment is added.",
           unit="native steering coefficient", lower_effect="slower steering response at low speed",
           higher_effect="sharper steering response at low speed", recommended_min=0,
           recommended_max=40, default=14),
    _field("steering_speed_falloff", "steering_speed_falloff", "Steering response change with speed", 0x1F, "b",
           "Signed amount multiplied by speed and added to low-speed steering response.",
           unit="native steering coefficient per speed unit", lower_effect="steering weakens more rapidly as speed rises",
           higher_effect="steering remains stronger at high speed", recommended_min=-12,
           recommended_max=0, default=-2),
    _field("rating_armor", "display_rating_armor", "Display rating: armor", 0x20, "B",
           "Front-end comparison value; only its ordering relative to the other three ratings affects the four-rank chart.",
           unit="relative display score", lower_effect="ranked below more categories",
           higher_effect="ranked above more categories", recommended_min=5, recommended_max=20, default=12),
    _field("rating_speed", "display_rating_speed", "Display rating: speed", 0x21, "B",
           "Front-end comparison value; only its ordering relative to the other three ratings affects the four-rank chart.",
           unit="relative display score", lower_effect="ranked below more categories",
           higher_effect="ranked above more categories", recommended_min=5, recommended_max=20, default=12),
    _field("rating_handling", "display_rating_handling", "Display rating: handling", 0x22, "B",
           "Front-end comparison value; only its ordering relative to the other three ratings affects the four-rank chart.",
           unit="relative display score", lower_effect="ranked below more categories",
           higher_effect="ranked above more categories", recommended_min=5, recommended_max=20, default=12),
    _field("rating_special", "display_rating_special", "Display rating: special", 0x23, "B",
           "Unused fourth comparison value in V8 retail records; all retail entries store zero.",
           unit="relative display score",
           lower_effect="no known V8 gameplay or displayed-chart effect",
           higher_effect="no known V8 gameplay or displayed-chart effect",
           recommended_min=0, recommended_max=0, default=0,
           confidence="MED"),
)


V82_FIELDS = (
    _field("wheel_kind_front", "first_axle_standard_wheel_object", "First-axle standard wheel object", 0x00, "H",
           "Top-level transformation-bank object used by wheel positions 0 and 1 (the front axle) in standard mode.",
           unit="transformation-bank object reference",
           lower_effect="not a strength setting; selecting a numerically lower object changes which standard wheel model is used",
           higher_effect="not a strength setting; selecting a numerically higher object changes which standard wheel model is used"),
    _field("wheel_kind_rear", "second_and_third_axle_standard_wheel_object", "Second/third-axle standard wheel object", 0x02, "H",
           "Top-level transformation-bank object used by positions 2 through 5: the rear axle on a four-wheel vehicle and both trailing axles on a six-wheel vehicle.",
           unit="transformation-bank object reference",
           lower_effect="not a strength setting; selecting a numerically lower object changes which standard wheel model is used",
           higher_effect="not a strength setting; selecting a numerically higher object changes which standard wheel model is used"),
    _field("front_spring_stiffness", "first_axle_spring_stiffness", "First-axle spring stiffness (front)", 0x04, "h",
           "How strongly wheel positions 0 and 1, the front axle, push back when compressed.",
           unit="native spring-force coefficient", lower_effect="softer front suspension with more compression; zero removes spring force",
           higher_effect="firmer front suspension with less compression", recommended_min=24,
           recommended_max=156, author_min=0, default=64),
    _field("middle_spring_stiffness", "second_axle_spring_stiffness", "Second-axle spring stiffness (rear on 4-wheel)", 0x06, "h",
           "How strongly wheel positions 2 and 3 push back when compressed. This is the rear axle on a four-wheel vehicle and the middle axle on a six-wheel vehicle.",
           unit="native spring-force coefficient", lower_effect="softer second-axle suspension with more compression; zero removes spring force",
           higher_effect="firmer second-axle suspension with less compression", recommended_min=28,
           recommended_max=156, author_min=0, default=64),
    _field("front_damping_rear_stiffness", "first_axle_damping_and_third_axle_spring_stiffness_shared",
           "First-axle damping / third-axle spring (shared)", 0x08, "h",
           "The retail V8:2 layout reuses this value as damping for positions 0/1 and spring stiffness for positions 4/5. Four-wheel vehicles use only its first-axle damping role.",
           unit="shared native suspension coefficient",
           lower_effect="more first-axle bounce and a softer third-axle spring; zero removes both coefficients",
           higher_effect="faster first-axle settling and a firmer third-axle spring",
           recommended_min=16, recommended_max=255, author_min=0, default=128),
    _field("middle_suspension_damping", "second_axle_suspension_damping",
           "Second-axle suspension damping (rear on 4-wheel)", 0x0A, "h",
           "Resistance to movement at positions 2/3. This damps the rear axle on a four-wheel vehicle and the middle axle on a six-wheel vehicle.",
           unit="native damping coefficient", lower_effect="more bounce and slower settling; zero removes damping",
           higher_effect="less bounce and faster settling", recommended_min=16,
           recommended_max=255, author_min=0, default=128),
    _field("capability_mask", "enabled_wheels_and_capabilities", "Enabled wheels/capabilities", 0x0C, "B",
           "Bit flags consumed by the six wheel/capability objects. This byte also forms the low byte of the retail rear-damping word.",
           unit="native capability bit mask",
           lower_effect="not monotonic; clearing a bit disables the capability assigned to that bit",
           higher_effect="not monotonic; setting a bit enables the capability assigned to that bit",
           recommended_min=0, recommended_max=255, default=15),
    _field("vehicle_type", "engine_assigned_vehicle_type", "Engine-assigned vehicle type", 0x0D, "B",
           "Roster and transformation-owner identity assigned by the engine. It also forms the high byte of the retail rear-damping word.",
           editable=False, unit="engine identity"),
    _field("low_speed_steering_response", "low_speed_steering_response",
           "Low-speed steering response", 0x0E, "b",
           "Base steering torque before the speed-dependent adjustment is added.",
           unit="native steering coefficient", lower_effect="slower steering response at low speed",
           higher_effect="sharper steering response at low speed", recommended_min=0,
           recommended_max=40, default=12),
    _field("steering_speed_falloff", "steering_speed_falloff",
           "Steering response change with speed", 0x0F, "b",
           "Signed amount multiplied by speed and added to low-speed steering response.",
           unit="native steering coefficient per speed unit",
           lower_effect="steering weakens more rapidly as speed rises",
           higher_effect="steering remains stronger at high speed", recommended_min=-6,
           recommended_max=0, default=-2),
    _field("unresolved_native_10", "internal_native_byte_10_read_only",
           "Internal native byte +0x10", 0x10, "B",
           "Copied to vehicle +0xC3; its complete gameplay effect is still being traced.",
           advanced=True, editable=False, unit="preserved native metadata",
           recommended_min=6, recommended_max=57, confidence="LOW"),
    _field("unresolved_native_11", "internal_native_byte_11_read_only",
           "Internal native byte +0x11", 0x11, "B",
           "Copied to vehicle +0xC4; its complete gameplay effect is still being traced.",
           advanced=True, editable=False, unit="preserved native metadata",
           recommended_min=12, recommended_max=73, confidence="LOW"),
    _field("unresolved_native_12", "internal_native_byte_12_read_only",
           "Internal native byte +0x12", 0x12, "B",
           "Copied to vehicle +0xC5; its complete gameplay effect is still being traced.",
           advanced=True, editable=False, unit="preserved native metadata",
           recommended_min=25, recommended_max=147, confidence="LOW"),
    _field("maximum_drive_force", "standard_maximum_drive_force",
           "Maximum drive force (standard)", 0x13, "B",
           "Maximum signed throttle force sent into the wheel-force routine before weapon upgrade.",
           unit="native drive-force coefficient", lower_effect="slower acceleration",
           higher_effect="stronger acceleration and wheel drive", recommended_min=20,
           recommended_max=191, default=100),
    _field("upgraded_maximum_drive_force", "upgraded_maximum_drive_force",
           "Maximum drive force (upgraded)", 0x14, "B",
           "Maximum signed throttle force after weapon upgrade; the engine interpolates from the standard value.",
           unit="native drive-force coefficient", lower_effect="slower upgraded acceleration",
           higher_effect="stronger upgraded acceleration and wheel drive", recommended_min=24,
           recommended_max=255, default=128),
    _field("chassis_position_follow", "internal_chassis_position_follow_read_only",
           "Chassis position follow (standard)", 0x15, "B",
           "Controls how quickly the vehicle's current position is moved toward its physics target, as value/256.",
           advanced=True, editable=False, unit="target-follow fraction per tick",
           lower_effect="slower, smoother positional following",
           higher_effect="faster, tighter positional following", recommended_min=18,
           recommended_max=56, confidence="MED"),
    _field("upgraded_chassis_position_follow",
           "internal_upgraded_chassis_position_follow_read_only",
           "Chassis position follow (upgraded)", 0x16, "B",
           "Weapon-upgraded target-follow value; the engine interpolates from the standard value.",
           advanced=True, editable=False, unit="target-follow fraction per tick",
           lower_effect="slower, smoother positional following",
           higher_effect="faster, tighter positional following", recommended_min=12,
           recommended_max=40, confidence="MED"),
    _field("reserved_native_17", "internal_reserved_native_byte_17_read_only",
           "Reserved native byte +0x17", 0x17, "B",
           "All 18 retail V8:2 records store zero here and the constructor does not read it.",
           advanced=True, editable=False, unit="reserved native metadata",
           recommended_min=0, recommended_max=0, confidence="HIGH"),
    _field("health", "standard_health", "Chassis health (standard)", 0x18, "H",
           "Initial and maximum chassis health before weapon upgrade.",
           unit="damage points", lower_effect="destroyed by less damage",
           higher_effect="survives more damage", recommended_min=628,
           recommended_max=1400, author_min=1, default=1000),
    _field("upgraded_health", "upgraded_health", "Chassis health (upgraded)", 0x1A, "H",
           "Maximum chassis health after weapon upgrade; the engine interpolates from the standard value.",
           unit="damage points", lower_effect="less upgraded durability",
           higher_effect="more upgraded durability", recommended_min=928,
           recommended_max=1800, author_min=1, default=1300),
    _field("speed_drag_coefficient", "standard_speed_drag_coefficient",
           "Speed drag coefficient (standard)", 0x1C, "I",
           "Velocity-proportional drag applied to all movement axes before weapon upgrade; this limits top speed.",
           unit="native fixed-point drag coefficient",
           lower_effect="less drag and a higher attainable speed",
           higher_effect="more drag and a lower attainable speed", recommended_min=1541,
           recommended_max=5764, default=3500),
    _field("upgraded_speed_drag_coefficient", "upgraded_speed_drag_coefficient",
           "Speed drag coefficient (upgraded)", 0x20, "I",
           "Velocity-proportional drag after weapon upgrade; the engine interpolates from the standard value.",
           unit="native fixed-point drag coefficient",
           lower_effect="less upgraded drag and a higher attainable speed",
           higher_effect="more upgraded drag and a lower attainable speed", recommended_min=1297,
           recommended_max=4577, default=2800),
    _field("pitch_angular_response", "pitch_angular_response", "Pitch angular response", 0x24, "h",
           "Inverse rotational inertia around the left-right axis: how readily the nose pitches up or down.",
           unit="native inverse-inertia coefficient", lower_effect="slower pitch rotation; zero prevents pitch response",
           higher_effect="faster pitch rotation from the same torque", recommended_min=64,
           recommended_max=64, author_min=0, default=64),
    _field("yaw_angular_response", "yaw_angular_response", "Yaw angular response", 0x26, "h",
           "Inverse rotational inertia around the vertical axis: how readily the vehicle turns left or right.",
           unit="native inverse-inertia coefficient", lower_effect="slower yaw rotation; zero prevents yaw response",
           higher_effect="faster yaw rotation from the same torque", recommended_min=64,
           recommended_max=64, author_min=0, default=64),
    _field("roll_angular_response", "roll_angular_response", "Roll angular response", 0x28, "h",
           "Inverse rotational inertia around the front-back axis: how readily the body rolls side to side.",
           unit="native inverse-inertia coefficient", lower_effect="slower body roll; zero prevents roll response",
           higher_effect="faster body roll from the same torque", recommended_min=64,
           recommended_max=64, author_min=0, default=64),
    _field("mass", "mass", "Vehicle mass", 0x2A, "H",
           "Mass used by collision momentum and by impulse/knockback calculations.",
           unit="native mass, 64 units = 1 collision-mass unit",
           lower_effect="more knockback and less collision momentum",
           higher_effect="less knockback and more collision momentum", recommended_min=2048,
           recommended_max=18432, author_min=1, default=8192),
    _field("rating_armor", "display_rating_armor", "Display rating: armor", 0x2C, "B",
           "Front-end comparison value; only its ordering relative to the other ratings affects the four-rank chart.",
           unit="relative display score", lower_effect="ranked below more categories",
           higher_effect="ranked above more categories", recommended_min=25,
           recommended_max=200, default=128),
    _field("rating_speed", "display_rating_speed", "Display rating: speed", 0x2D, "B",
           "Front-end comparison value; only its ordering relative to the other ratings affects the four-rank chart.",
           unit="relative display score", lower_effect="ranked below more categories",
           higher_effect="ranked above more categories", recommended_min=25,
           recommended_max=200, default=128),
    _field("rating_handling", "display_rating_handling", "Display rating: handling", 0x2E, "B",
           "Front-end comparison value; only its ordering relative to the other ratings affects the four-rank chart.",
           unit="relative display score", lower_effect="ranked below more categories",
           higher_effect="ranked above more categories", recommended_min=25,
           recommended_max=200, default=128),
    _field("rating_special", "display_rating_special", "Display rating: special", 0x2F, "B",
           "Front-end comparison value; only its ordering relative to the other ratings affects the four-rank chart.",
           unit="relative display score", lower_effect="ranked below more categories",
           higher_effect="ranked above more categories", recommended_min=25,
           recommended_max=200, default=128),
)

V82_EXTENSION_FIELDS = (
    _field(
        "rear_suspension_damping",
        "third_axle_suspension_damping",
        "Third-axle suspension damping (6-wheel rear)",
        -1,
        "h",
        "Resistance to movement at wheel positions 4/5, the third/rear axle "
        "on a six-wheel custom vehicle. "
        "This explicit registry value removes the retail dependency on the "
        "engine-assigned roster type.",
        unit="native damping coefficient",
        lower_effect="more rear bounce and slower settling",
        higher_effect="less rear bounce and faster settling",
        recommended_min=12,
        recommended_max=4367,
        author_min=0,
        default=128,
    ),
)


PROFILES = {
    "V8": GameProfile("V8", 0x8005EA60, 0x24, 13, V8_FIELDS),
    "V8_2": GameProfile(
        "V8_2", 0x80063A80, 0x30, 18, V82_FIELDS, V82_EXTENSION_FIELDS
    ),
}


def authoring_values(game: str, values: Mapping[str, int]) -> dict[str, int]:
    """Return the human-facing stat names used in exported project JSON."""

    profile = PROFILES[game]
    return {
        field.author_name: int(values[field.name])
        for field in profile.authoring_fields
        if field.name != "vehicle_type"
    }


def canonicalize_values(game: str, values: Mapping[str, int]) -> dict[str, int]:
    """Accept current author names, internal names, or schema-4 legacy names."""

    profile = PROFILES[game]
    supplied = {str(name): int(value) for name, value in values.items()}
    result: dict[str, int] = {}
    for field in profile.authoring_fields:
        if field.name == "vehicle_type":
            continue
        if field.author_name in supplied:
            result[field.name] = supplied[field.author_name]
        elif field.name in supplied:
            result[field.name] = supplied[field.name]

    legacy = {
        "V8": {
            "wheel_param_a_front": "front_spring_stiffness",
            "wheel_param_a_rear": "rear_spring_stiffness",
            "wheel_param_b_front": "front_suspension_damping",
            "wheel_param_b_rear": "rear_suspension_damping",
            "variant_flags": "maximum_drive_force",
            "control_state": "unused_native_byte_0f",
            "template_reference": "speed_drag_coefficient",
            "handling_a": "roll_angular_response",
            "handling_b": "mass",
            "handling_signed_a": "low_speed_steering_response",
            "handling_signed_b": "steering_speed_falloff",
        },
        "V8_2": {
            "front_wheel_object": "wheel_kind_front",
            "middle_and_rear_wheel_object": "wheel_kind_rear",
            "front_spring_stiffness": "front_spring_stiffness",
            "middle_spring_stiffness": "middle_spring_stiffness",
            "front_damping_and_rear_stiffness_shared": "front_damping_rear_stiffness",
            "middle_suspension_damping": "middle_suspension_damping",
            "rear_suspension_damping": "rear_suspension_damping",
            "wheel_param_a_front": "front_spring_stiffness",
            "wheel_param_a_middle": "middle_spring_stiffness",
            "wheel_param_a_rear": "front_damping_rear_stiffness",
            "wheel_param_b_rear": "middle_suspension_damping",
            "handling_signed_a": "low_speed_steering_response",
            "handling_signed_b": "steering_speed_falloff",
            "handling_signed_c": "unresolved_native_10",
            "handling_signed_d": "unresolved_native_11",
            "handling_signed_e": "unresolved_native_12",
            "handling_signed_f": "maximum_drive_force",
            "handling_signed_g": "chassis_position_follow",
            "reserved_1a": "upgraded_health",
            "physics_coefficient_a": "speed_drag_coefficient",
            "physics_coefficient_b": "upgraded_speed_drag_coefficient",
            "handling_a": "roll_angular_response",
            "handling_b": "mass",
        },
    }[game]
    for old_name, new_name in legacy.items():
        if new_name not in result and old_name in supplied:
            value = supplied[old_name]
            if game == "V8_2" and old_name.startswith("handling_signed_"):
                value &= 0xFF
            result[new_name] = value

    packed = supplied.get("motion_coefficients")
    if packed is not None:
        result.setdefault("pitch_angular_response", packed & 0xFFFF)
        result.setdefault("yaw_angular_response", (packed >> 16) & 0xFFFF)

    for field in profile.authoring_fields:
        if field.name != "vehicle_type":
            result.setdefault(field.name, field.default)
    return result

V82_TRANSFORM_TABLE_ADDRESS = 0x80063F74
V82_TRANSFORM_MODE_COUNT = 4
V82_TRANSFORM_WHEEL_COUNT = 6


@dataclass(frozen=True)
class ImmediateSetting:
    name: str
    label: str
    address: int
    expected_upper: int
    description: str
    unit: str
    lower_effect: str
    higher_effect: str
    author_min: int
    author_max: int
    retail_value: int


V82_POWERUP_SETTINGS = (
    ImmediateSetting(
        "repair_amount",
        "Repair amount",
        0x80049E48,
        0x2405,
        "Base repair amount passed to the retail repair callback.",
        "damage points restored",
        "restores less chassis health; zero repairs nothing",
        "restores more chassis health, capped by maximum health",
        0,
        0x7FFF,
        500,
    ),
    ImmediateSetting(
        "weapon_upgrade_duration",
        "Weapon upgrade duration",
        0x80049EB0,
        0x2402,
        "Weapon-upgrade timer in simulation ticks.",
        "simulation ticks",
        "the weapon upgrade expires sooner; zero expires immediately",
        "the weapon upgrade remains active longer",
        0,
        0x7FFF,
        900,
    ),
    ImmediateSetting(
        "radar_jammer_duration",
        "Radar jammer duration",
        0x80049EBC,
        0x2402,
        "Radar-jammer timer in simulation ticks.",
        "simulation ticks",
        "the radar jammer expires sooner; zero expires immediately",
        "the radar jammer remains active longer",
        0,
        0x7FFF,
        900,
    ),
    ImmediateSetting(
        "shield_duration",
        "Shield duration",
        0x80049EC8,
        0x2402,
        "Shield timer in simulation ticks.",
        "simulation ticks",
        "the shield expires sooner; zero expires immediately",
        "the shield remains active longer",
        0,
        0x7FFF,
        900,
    ),
    ImmediateSetting(
        "transformation_duration",
        "Transformation duration",
        0x80049F80,
        0x2406,
        "Hover, ski, and hydro transformation timer in simulation ticks.",
        "simulation ticks",
        "the transformed mode expires sooner; zero expires immediately",
        "the transformed mode remains active longer",
        0,
        0x7FFF,
        500,
    ),
)


class VehicleStatsRecord:
    def __init__(self, owner: "StatsFile", index: int):
        if index < 0 or index >= owner.profile.record_count:
            raise IndexError(index)
        self.owner = owner
        self.index = index

    @property
    def offset(self) -> int:
        return self.owner.table_offset + self.index * self.owner.profile.record_size

    def values(self) -> dict[str, int]:
        return {
            field.name: struct.unpack_from(
                "<" + field.format, self.owner._data, self.offset + field.offset
            )[0]
            for field in self.owner.profile.fields
        }

    def get(self, name: str) -> int:
        field = self.owner.field(name)
        return struct.unpack_from(
            "<" + field.format, self.owner._data, self.offset + field.offset
        )[0]

    def set(self, name: str, value: int) -> None:
        field = self.owner.field(name)
        try:
            struct.pack_into(
                "<" + field.format,
                self.owner._data,
                self.offset + field.offset,
                int(value),
            )
        except struct.error as error:
            raise ValueError(f"{name} value {value} is outside its native range") from error

    @property
    def raw(self) -> bytes:
        size = self.owner.profile.record_size
        return bytes(self.owner._data[self.offset : self.offset + size])


class StatsFile:
    """A PS-X EXE with editable retail vehicle records."""

    def __init__(self, data: bytes, game: str | None = None):
        self._data = bytearray(data)
        require_range(data, 0, 0x800)
        if data[:8] != b"PS-X EXE":
            raise FormatError("vehicle stats input is not a PS-X EXE")
        self.load_address = u32le(data, 0x18)
        self.text_size = u32le(data, 0x1C)
        if game is None:
            game = self._detect_game()
        normalized = game.upper().replace(":", "_")
        if normalized not in PROFILES:
            raise ValueError(f"unknown game profile {game!r}")
        self.profile = PROFILES[normalized]
        self.table_offset = self.address_to_offset(self.profile.table_address)
        require_range(
            self._data,
            self.table_offset,
            self.profile.record_count * self.profile.record_size,
        )
        for index in range(self.profile.record_count):
            if self.record(index).get("vehicle_type") != index:
                raise FormatError(
                    f"{self.profile.game} stat record {index} has unexpected "
                    f"vehicle type {self.record(index).get('vehicle_type')}"
                )

    @property
    def data(self) -> bytes:
        return bytes(self._data)

    def address_to_offset(self, address: int) -> int:
        relative = address - self.load_address
        if relative < 0 or relative >= self.text_size:
            raise FormatError(f"PS-X address 0x{address:08X} is outside the executable")
        return 0x800 + relative

    def _profile_matches(self, profile: GameProfile) -> bool:
        relative = profile.table_address - self.load_address
        offset = 0x800 + relative
        total = profile.record_count * profile.record_size
        if relative < 0 or relative + total > self.text_size:
            return False
        return all(
            self._data[offset + index * profile.record_size + 0x0D] == index
            for index in range(profile.record_count)
        )

    def _detect_game(self) -> str:
        matches = [
            profile.game
            for profile in PROFILES.values()
            if self._profile_matches(profile)
        ]
        if len(matches) != 1:
            raise FormatError(
                "could not uniquely identify a V8 or V8:2 executable stat table"
            )
        return matches[0]

    def field(self, name: str) -> FieldSpec:
        for field in self.profile.fields:
            if field.name == name:
                return field
        raise KeyError(name)

    def record(self, index: int) -> VehicleStatsRecord:
        return VehicleStatsRecord(self, index)

    def records(self) -> Iterator[VehicleStatsRecord]:
        for index in range(self.profile.record_count):
            yield self.record(index)

    def apply(self, index: int, values: Mapping[str, int]) -> None:
        record = self.record(index)
        for name, value in values.items():
            record.set(name, value)

    def _require_v82(self) -> None:
        if self.profile.game != "V8_2":
            raise ValueError("powerup and transformation tables exist only in V8:2")

    def _powerup_setting(self, name: str) -> ImmediateSetting:
        self._require_v82()
        for setting in V82_POWERUP_SETTINGS:
            if setting.name == name:
                return setting
        raise KeyError(name)

    def powerup_values(self) -> dict[str, int]:
        self._require_v82()
        result = {}
        for setting in V82_POWERUP_SETTINGS:
            offset = self.address_to_offset(setting.address)
            instruction = u32le(self._data, offset)
            if instruction >> 16 != setting.expected_upper:
                raise FormatError(
                    f"{setting.name} patch site 0x{setting.address:08X} "
                    "does not contain the expected retail instruction"
                )
            result[setting.name] = instruction & 0xFFFF
        return result

    def set_powerup(self, name: str, value: int) -> None:
        setting = self._powerup_setting(name)
        if value < 0 or value > 0x7FFF:
            raise ValueError("powerup values must fit a non-negative MIPS immediate")
        offset = self.address_to_offset(setting.address)
        instruction = u32le(self._data, offset)
        if instruction >> 16 != setting.expected_upper:
            raise FormatError(
                f"{name} patch site 0x{setting.address:08X} "
                "does not contain the expected retail instruction"
            )
        struct.pack_into("<I", self._data, offset, (instruction & 0xFFFF0000) | value)

    def transform_modes(self) -> tuple[tuple[int, ...], ...]:
        self._require_v82()
        offset = self.address_to_offset(V82_TRANSFORM_TABLE_ADDRESS)
        require_range(
            self._data,
            offset,
            V82_TRANSFORM_MODE_COUNT * V82_TRANSFORM_WHEEL_COUNT * 2,
        )
        return tuple(
            tuple(
                struct.unpack_from(
                    "<H",
                    self._data,
                    offset + (mode * V82_TRANSFORM_WHEEL_COUNT + wheel) * 2,
                )[0]
                for wheel in range(V82_TRANSFORM_WHEEL_COUNT)
            )
            for mode in range(V82_TRANSFORM_MODE_COUNT)
        )

    def set_transform_wheel_kind(self, mode: int, wheel: int, value: int) -> None:
        self._require_v82()
        if mode < 0 or mode >= V82_TRANSFORM_MODE_COUNT:
            raise IndexError(mode)
        if wheel < 0 or wheel >= V82_TRANSFORM_WHEEL_COUNT:
            raise IndexError(wheel)
        if value < 0 or value > 0xFFFF:
            raise ValueError("transformation object kind must fit unsigned 16-bit")
        offset = self.address_to_offset(V82_TRANSFORM_TABLE_ADDRESS)
        struct.pack_into(
            "<H",
            self._data,
            offset + (mode * V82_TRANSFORM_WHEEL_COUNT + wheel) * 2,
            value,
        )
