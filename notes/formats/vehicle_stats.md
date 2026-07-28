# Vehicle stat records

Status: active reverse-engineering document. `SLUS_005.10` and
`SLUS_008.68` were traced from each table load through the vehicle
constructors and physics consumers. All offsets are relative to one vehicle
record. Values remain native integers; Blender does not convert them to
floating point.

Two bounds are intentionally distinguished:

- **Native bound** is the binary storage limit. Crossing it cannot be encoded.
- **Retail-tested range** is the minimum and maximum used by the shipped US
  roster. It is the safe starting range for authored vehicles, not a claim
  that every combination inside it has been soak-tested.
- **Safe authoring bound** is enforced when the full storage range has a
  nonphysical branch: signed spring, damping, and angular-response
  coefficients use `0..32767`; mass and health use `1..65535`. Zero can
  deliberately disable a spring, damper, or rotational response, while
  negative coefficients and zero mass/health are rejected.

## Vigilante 8 — 0x24-byte record

Retail table: `0x8005EA60`, 13 records. The constructor is
`Vehicle_Construct` / `0x8002E630`.

| Offset | Authoring name | Native bound | Retail-tested | What it means | Lower value | Higher value |
|---:|---|---:|---:|---|---|---|
| 00 | Front wheel object | body-bank reference | 0–8 | Top-level object used for the front wheel pair. | Different object, not “less.” | Different object, not “more.” |
| 02 | Rear wheel object | body-bank reference | 1–8 | Top-level object used for the rear wheel pair. | Different object, not “less.” | Different object, not “more.” |
| 04 | Front spring stiffness | -32768–32767 | 32–100 | Force pushing the front suspension back toward its resting height. | Softer; compresses farther. | Firmer; compresses less. |
| 06 | Rear spring stiffness | -32768–32767 | 40–112 | Force pushing the rear suspension back toward its resting height. | Softer; compresses farther. | Firmer; compresses less. |
| 08 | Front suspension damping | -32768–32767 | 80–200 | Resistance to suspension movement. A damper stops repeated bouncing after a bump. | More bounce; slower settling. | Less bounce; faster settling. |
| 0A | Rear suspension damping | -32768–32767 | 64–200 | Rear-axle resistance to suspension movement. | More bounce; slower settling. | Less bounce; faster settling. |
| 0C | Enabled wheels | 0–15 authoring mask | 3–15 | Bits 0–3 enable front-left, front-right, rear-left, rear-right. | Bit mask; numeric direction is meaningless. | Bit mask; numeric direction is meaningless. |
| 0D | Engine-assigned vehicle type | 0–255 | 0–12 | Runtime roster identity. Read-only for custom vehicles. | Not authorable. | Not authorable. |
| 0E | Maximum drive force | 0–255 | 32–255 | Maximum signed throttle force sent to wheel physics. | Slower acceleration. | Stronger acceleration/drive. |
| 0F | Internal unused byte | 0–255 | 192–245 | Copied to vehicle `+0xB4`; no runtime read was found. Preserved read-only. | Unknown. | Unknown. |
| 10 | Speed drag coefficient | 0–4294967295 | 2075–3891 | Velocity-proportional drag applied to X/Y/Z; primary top-speed limiter. | Less drag; higher attainable speed. | More drag; lower attainable speed. |
| 14 | Pitch angular response | -32768–32767 | 64–100 | Inverse rotational inertia around the left-right axis. | Nose pitches more slowly. | Nose pitches more readily. |
| 16 | Yaw angular response | -32768–32767 | 24–64 | Inverse rotational inertia around the vertical axis. | Turns/rotates more slowly. | Turns/rotates more readily. |
| 18 | Roll angular response | -32768–32767 | 48–100 | Inverse rotational inertia around the front-back axis. | Body rolls more slowly. | Body rolls more readily. |
| 1A | Mass | 0–65535 | 3072–16384 | Collision momentum and impulse resistance; 64 native units equal one collision-mass unit. | More knockback, less momentum. | Less knockback, more momentum. |
| 1C | Chassis health | 0–65535 | 400–2175 | Initial/maximum body health. | Destroyed by less damage. | Survives more damage. |
| 1E | Low-speed steering response | -128–127 | 0–40 | Base steering torque before the speed term. | Slower low-speed steering. | Sharper low-speed steering. |
| 1F | Steering response change with speed | -128–127 | -12–0 | Signed speed multiplier added to the base steering response. | Steering weakens faster with speed. | Steering remains stronger at speed. |
| 20–23 | Armor/speed/handling/special display ratings | 0–255 each | 0–20 | Front-end comparison values. The engine sorts the four values and stores only their relative ranks. | Ranked below more categories. | Ranked above more categories. |

Suspension evidence is in `src/physics/wheel_physics.c`: wheel `+0x8C`
multiplies suspension compression (spring stiffness), while wheel `+0x8E`
multiplies the change in suspension travel before a `>> 5` (damping).
Steering evidence is in `src/gameplay/vehicle_damaged_physics.c`.
Drag is consumed at `0x80030784`. Mass is consumed by vehicle collision and
weapon impulse code.

## Vigilante 8: 2nd Offense — 0x30-byte record

Retail table: `0x80063A80`, 18 records. The constructor is `0x8003C464`.
Weapon upgrade interpolation is performed by `0x8003C9C4`.

| Offset | Authoring name | Native bound | Retail-tested | What it means | Lower value | Higher value |
|---:|---|---:|---:|---|---|---|
| 00 | Front wheel object | transformation-bank reference | 0–52 | Standard-mode wheel object for the front pair. | Different object. | Different object. |
| 02 | Middle/rear wheel object | transformation-bank reference | 0–52 | Standard-mode wheel object for both remaining pairs. | Different object. | Different object. |
| 04 | First-axle spring stiffness (positions 0/1, front) | -32768–32767 | 24–156 | Front-axle suspension restoring force. | Softer; more compression. | Firmer; less compression. |
| 06 | Second-axle spring stiffness (positions 2/3) | -32768–32767 | 28–156 | Rear axle on a four-wheel vehicle; middle axle on a six-wheel vehicle. | Softer; more compression. | Firmer; less compression. |
| 08 | First-axle damping / third-axle stiffness (shared) | -32768–32767 | 16–255 | Damping for positions 0/1 and spring stiffness for positions 4/5; a four-wheel vehicle uses only the first role. | More front bounce and softer third axle. | Faster front settling and firmer third axle. |
| 0A | Second-axle suspension damping (positions 2/3) | -32768–32767 | 16–255 | Rear damping on a four-wheel vehicle; middle damping on a six-wheel vehicle. | More bounce; slower settling. | Less bounce; faster settling. |
| 0C | Enabled wheels/capabilities | 0–255 | 3–255 | Wheel/capability bit flags. It is also the low byte of the retail rear-damping word. | Bit mask; numeric direction is meaningless. | Bit mask; numeric direction is meaningless. |
| 0D | Engine-assigned vehicle type | 0–255 | 0–17 | Roster/transformation identity and high byte of the retail rear-damping word. Read-only. | Not authorable. | Not authorable. |
| 0E | Low-speed steering response | -128–127 | 0–40 | Base steering torque. | Slower low-speed steering. | Sharper low-speed steering. |
| 0F | Steering response change with speed | -128–127 | -6–0 | Signed speed multiplier added to base steering response. | Steering weakens faster with speed. | Steering remains stronger at speed. |
| 10–12 | Internal native bytes | 0–255 | 6–147 | Copied to vehicle `+0xC3..+0xC5`; semantics are not complete. Preserved read-only, not presented as authoring controls. | Unknown. | Unknown. |
| 13 | Maximum drive force (standard) | 0–255 | 20–191 | Pre-upgrade throttle-force ceiling. | Slower acceleration. | Stronger acceleration/drive. |
| 14 | Maximum drive force (upgraded) | 0–255 | 24–255 | Post-upgrade ceiling; interpolated from offset 13. | Slower upgraded acceleration. | Stronger upgraded acceleration. |
| 15 | Chassis position follow (standard) | 0–255 | 18–56 | Per-tick fraction, value/256, used to move current position toward its physics target. Read-only pending complete system naming. | Slower/smoother following. | Faster/tighter following. |
| 16 | Chassis position follow (upgraded) | 0–255 | 12–40 | Post-upgrade value interpolated from offset 15. Read-only pending complete system naming. | Slower/smoother following. | Faster/tighter following. |
| 17 | Reserved | 0–255 | 0 | Unread by the constructor; zero in all retail records. Read-only. | No known effect. | No known effect. |
| 18 | Chassis health (standard) | 0–65535 | 628–1400 | Initial/pre-upgrade health. | Destroyed by less damage. | Survives more damage. |
| 1A | Chassis health (upgraded) | 0–65535 | 928–1800 | Post-upgrade health; interpolated from offset 18. | Less upgraded durability. | More upgraded durability. |
| 1C | Speed drag coefficient (standard) | 0–4294967295 | 1541–5764 | Pre-upgrade velocity-proportional drag and top-speed limiter. | Less drag; higher speed. | More drag; lower speed. |
| 20 | Speed drag coefficient (upgraded) | 0–4294967295 | 1297–4577 | Post-upgrade drag; interpolated from offset 1C. | Less upgraded drag; higher speed. | More upgraded drag; lower speed. |
| 24 | Pitch angular response | -32768–32767 | 64 | Inverse pitch inertia. | Slower pitch. | Faster pitch. |
| 26 | Yaw angular response | -32768–32767 | 64 | Inverse yaw inertia. | Slower yaw. | Faster yaw. |
| 28 | Roll angular response | -32768–32767 | 64 | Inverse roll inertia. | Slower roll. | Faster roll. |
| 2A | Mass | 0–65535 | 2048–18432 | Collision momentum and impulse resistance; 64 native units equal one collision-mass unit. | More knockback, less momentum. | Less knockback, more momentum. |
| 2C–2F | Armor/speed/handling/special display ratings | 0–255 each | 25–200 | Front-end values sorted into relative four-rank order. | Ranked below more categories. | Ranked above more categories. |

### V8:2 rear-damping overlap

The six-wheel constructor computes its stat pointer as
`record + ((wheel_index >> 1) * 2)`, then reads stiffness at `+4` and damping
at `+8`. Therefore:

| Wheel pair | Stiffness source | Damping source |
|---|---|---|
| First/front (positions 0/1) | record +04 | record +08 |
| Second (positions 2/3; rear on 4-wheel) | record +06 | record +0A |
| Third/rear (positions 4/5; six-wheel only) | record +08 | record +0C |

The rear damping word is consequently
`enabled_wheels_and_capabilities | (engine_assigned_vehicle_type << 8)`.
That relationship remains exact for unmodified retail records.

Custom `VEHICLES.V8R` version 3 entries add one decoded unsigned 16-bit
extension internally named `rear_suspension_damping` and shown to authors as
`third_axle_suspension_damping` (valid authoring range 0–32767).
After the native constructor creates all six wheels, the V8:2 host writes
first-, second-, and third-axle stiffness to wheel `+0x8C` and damping to wheel
`+0x8E` from the named authored values. Thus a custom vehicle's damping no
longer changes when its append-only roster ID changes. Version-2 registries
remain readable; their rear value is migrated from the historical `+0x0C`
word.
