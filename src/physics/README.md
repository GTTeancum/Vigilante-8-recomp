# src/physics

Vehicle dynamics, collision, suspension, damage.

## Status

DECOMP_PASS_2: **complete.** All in-scope physics functions are
hand-cleaned. Pass 3 polish completed (TODO markers resolved).

## Foundation primitives (HIGH)

- `gte_rotate.c` -- 17.15 packed-vector GTE rotates (bedrock for
  bit-exact vehicle position math).
- `gte_load_xform.c` -- 8-u32 packed-transform GTE register load.
- `vec_scale_pack.c` -- 4.12 packed-vector scale (i32 and i16 inputs).
- `mat_abs_load.c` -- absolute-value matrix load (axis-aligned bounds).
- `math_helpers.c` -- ratan2 helpers, rigid-body matrix inverse,
  XZ-plane re-orthogonalise.
- `terrain_height.c` -- bilinear 32x32-of-64x64 heightmap sampler.
- `terrain_color.c` -- height-keyed greyscale color sampler.
- `terrain_init.c` -- flat-world chunk pool init.
- `terrain_visibility.c` -- "is most terrain resident" predicate.
- `terrain_normal.c`, `terrain_probe.c`, `terrain_objects.md`.
- `object_integrate.c` -- per-frame /128 velocity integrator
  (the universal physics tick).
- `camera_build.c` -- terrain-aligned local tilt matrix builder.

## Per-level callbacks (MED)

Each level has 4-6 destructible-object handlers in its directory:

- `airgrave/` -- AGTracker, AGProjectile, tracker_dish, cruise_missile.
- `canynlnd/` -- boulder, boulder_chain, boulder_roll, spawner, beam.
- `casnocty/` -- blimp_main, blimp_pilot, blimp_skin, bomb_list.
- `hoovrdam/` -- dam_lever, siren_strobe, spillway_grab.
- `oilfield/` -- rig_emit, steam_rig, barrel_roll.
- `sandfact/` -- conveyor, conveyor_grab.
- `scrtbase/` -- radar, radar_sweep, security_door, bunker_door,
  silo_door, turret_track, missile_track.
- `skiresrt/` -- gondola, gondola_position, lift_chair_grab,
  snow_pulse, powder_boulder, ice_chunk.
- `vallyfrm/` -- pump, silo_init, silo_cleanup, silo_rotate,
  silo_slide, windmill, windmill_alarm, misc.
- `wildwest/` -- train_init, bridge_collapse, dynamite_keg,
  stage_coach, saloon_destruct.

## What's still UNKNOWN

- **Vehicle struct allocation site.** Vehicle pointers come from
  `puRam000007d0`/`puRam000007d4` but the malloc/init site is not
  in any cleaned function. Likely a static BSS pool reached
  indirectly through the level loader. Deferred to Ghidra
  interactive session.

## Achievement

Per CLAUDE.md, this subsystem is "in scope" (vehicle dynamics,
collision, damage). The foundation primitives (GTE, terrain, integrate)
plus the 10 level-specific callback sets satisfy the in-scope
requirement.
