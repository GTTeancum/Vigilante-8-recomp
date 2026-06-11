# Weapon Lifecycle RE Map - 2026-06-10

Scope: live weapon pickups, vehicle attachment, attached-weapon firing,
fire animation/state callbacks, projectile/effect spawn, and renderer
visibility. This is a source-grounded map of where each stage occurs and
which trace row proves the live path.

The active fidelity target is the full chain: pickup spawn/materialization,
attachment timing and slot storage, mounted state update, fire
animation/dispatch, projectile/effect creation, and renderer submission.

This is intentionally broader than "does the mesh appear." A passing host
build must prove the same live object pointer crosses all major source
boundaries:

- loose pickup/source placeholder is present in the source object list
- pickup event `3` materializes the child
- `FUN_8002cbe8` appends that child to the vehicle and stores it in
  `vehicle+0x110/+0x114/+0x118`
- mounted dispatch calls the child callback for per-frame aim/animation
  event `0`, fire/gate/key events `11/12/13/14`, and wtype-specific combo
  events where applicable
- projectile/effect helpers allocate and register the visible type-7 or
  effect child
- renderer rows draw the same mounted child, its children, and any projectile
  rows without packet rejection or fabricated geometry

## Source Lifecycle

1. Pickup collision and child materialization

- Source file: `src/gameplay/split_child_tick.c`
- Function: `common_spawn_child`
- Source event: split-child pickup callback allocates the real child object
  from the source bank with `FUN_8001ac44`, installs the child callback, fires
  child event `1`, copies source-relative position/bone data, initializes the
  bone matrix, links the child to the vehicle, and destroys the pickup.
- Trace rows:
  - `weapon_lifecycle pickup_child_materialized`
  - `weapon_lifecycle pickup_child_positioned`
  - `weapon_lifecycle pickup_attach_begin`
  - `weapon_lifecycle pickup_attach_complete`
- Important fields logged: pickup object, source vehicle, child object,
  weapon kind, source slot, returned callback, pickup/source/child positions,
  selected hardpoint pointer, callback key/event-14 joint key, hardpoint
  offset/rotation, and whether the test forced a zero pickup-to-vehicle
  offset. A `forced_zero_offset=1` row proves callback coverage only; it is
  not acceptable visual-fidelity proof because it can bury the mounted object
  inside the vehicle.
- `pickup_attach_begin` and `pickup_attach_complete` bracket the real
  `FUN_8002cbe8` call. They log source slots before/after, child hierarchy
  links, hardpoint pointer, and child offsets so the trace answers when the
  pickup stops being a loose object and where it enters the vehicle hierarchy.
- The exact attach transition is therefore: source pickup callback event `3`
  -> child allocation/materialization -> hardpoint/joint selection -> source
  `FUN_8002cbe8` -> vehicle active slot write. Any host-only direct slot write
  is not source-equivalent evidence.

2. Vehicle attach and active-slot storage

- Source file: `src/gameplay/weapon_slot.c`
- Function: `FUN_8002cbe8` (`WeaponSlot_Attach`)
- Source event: clears child spawn id, appends child to vehicle with
  `FUN_8001d4f0`, handles duplicate weapon toggles, ejects an old slot if the
  three active slots are full, stores the child into vehicle `+0x110..+0x118`,
  and registers the live host visual attachment.
- Attachment moment: after pickup collision materializes the child, the source
  append runs immediately on the same gameplay path and then writes the child
  pointer to the first available active slot. The host renderer must draw only
  these stored `+0x110..+0x118` children as attached weapons; vehicle `+0x10c`
  is the chassis/built-in callback path and is not a pickup attachment.
- Trace rows:
  - `weapon_lifecycle attach_appended`
  - `weapon_lifecycle attach_duplicate_toggle`
  - `weapon_lifecycle attach_eject_slot`
  - `weapon_lifecycle attach_stored`
- Important fields logged: vehicle, child, active slot, kind byte, ammo/hp,
  status, callback, raw callback, child parent/first-child/sibling links,
  hardpoint pointer, child local/render offsets, and the three active slot
  pointers. These rows answer exactly when and where the weapon becomes
  attached: the child is appended to the source vehicle hierarchy and stored
  in `vehicle+0x110/+0x114/+0x118` immediately during the pickup collision
  event.
- Attachment location is not a single global offset. It is the selected source
  hardpoint pointer in the vehicle's child/joint tree, plus the child local
  transform written during pickup materialization. Validation must compare
  `pickup_attach_begin`, `attach_stored`, `mounted_dispatch_begin`, and
  `attached_draw` rows for the same child pointer and hardpoint.

3. Dropping/ejecting an attached weapon

- Source file: `src/gameplay/weapon_slot.c`
- Functions:
  - `FUN_8002ca94` (`WeaponSlot_Drop`)
  - `FUN_8002c99c` (`WeaponSlot_Spawn`)
  - `FUN_8002cb7c` (`WeaponSlot_OnChildDestroy`)
- Source event: weapon slot removal spawns or ejects the held child, clears
  host visual registration, memmoves the remaining slots down, and updates
  active weapon index at vehicle `+0xb3`.
- Trace rows:
  - `weapon_lifecycle child_destroy_lookup`
  - `weapon_lifecycle slot_drop_start`
  - `weapon_lifecycle slot_spawn_alloc`
  - `weapon_lifecycle slot_spawn_ready`
  - `weapon_lifecycle slot_drop_shifted`

4. Attached-weapon state callbacks, aiming, and fire animation

- Source file: `src/gameplay/vehicle_init_joints.c`
- Functions:
  - `Vehicle_StateSubObject0..11`
  - `Vehicle_StateTrackTarget12`
  - `Vehicle_FireBlobLauncher`
- Source event: state callbacks respond to per-frame event `0`, init event
  `1`, fire event `11`, availability event `12`, key event `13`, and visual
  key event `14`. These callbacks aim joints, animate pivots, decrement ammo,
  allocate effects/projectiles, and retire slots when ammo is empty.
- Firing moment: event `0` is the per-frame aim/animation tick; event `11` is
  the direct fire request; combo events are routed through the same callback
  table and can allocate different projectile/effect children. A valid render
  proof must include both callback trace rows and `renderer_projectile_draw`
  rows for the resulting type-7 runtime object.
- Attachment animation contract: after pickup storage, mounted state callbacks
  continue to receive event `0`. Those ticks are where pivots/joints animate
  toward targets or recoil phases (`state_weapon_aim`,
  `state_weapon_fire_anim`, `wtype6_tracking`). Event `11`/combo events create
  the projectile/effect. Renderer fidelity is only proven when the same mounted
  object also has `attached_draw` rows with sane hardpoint/chain data.
- Firing animation is callback-owned. The host renderer should not invent
  recoil/rotation. It should draw the source-mutated child/joint matrices after
  event `0` and fire/combo callbacks mutate them. Required proof rows are:
  `mounted_dispatch_begin`, one or more state/wtype animation rows, a fire
  row (`state_weapon_fire_*`, `wtype6_event11_projectile`, or combo row), and
  `attached_draw` for the mounted object after those events.
- Trace rows:
  - `weapon_lifecycle state_weapon_aim`
  - `weapon_lifecycle state_weapon_fire_anim`
  - `weapon_lifecycle state_weapon_fire_projectile`
  - `weapon_lifecycle state_weapon_fire_effect`
  - `weapon_lifecycle state_weapon_fire_blob`
  - `weapon_lifecycle state_weapon_fire_cluster`
  - `weapon_lifecycle mounted_dispatch_begin`
  - `weapon_lifecycle mounted_dispatch_ret`
- Important fields logged: state id, owner, pivot, projectile, effect, event,
  ammo, phase byte, callback, pivot pitch/yaw, projectile position/velocity,
  projectile target, effect callback/position.
- `mounted_dispatch_begin` and `mounted_dispatch_ret` are emitted from the
  active mounted slot dispatcher before and after callback events. They log
  the active slot, mounted node, callback, hardpoint, parent/child/sibling
  links, target pointer, and node transforms. These rows are the bridge
  between `attach_stored` and the state callback rows, so a valid proof can
  show the same child pointer being stored, dispatched, animated, fired, and
  drawn.

5. Wtype split-child and seeker-specific tracking/firing

- Source file: `src/gameplay/split_child_wtype_ticks.c`
- Functions:
  - `LAB_80031fa0` mounted wtype 2 dispatcher
  - `FUN_80031dfc` bullet/projectile spawner used by mounted wtype 2
  - `FUN_80031e34` / projectile callback for spawned wtype 2 projectile bodies
  - `LAB_8003302c` / `FUN_80032f7c` for wtype 4 spring-guided projectile
  - `LAB_800336fc` for wtype 6 attached dispatch
- Raw SLUS correction, 2026-06-11: `LAB_80031fa0` at `0x80031fa0` is the
  mounted wtype 2 event dispatcher from the vehicle sub-object callback table,
  not the projectile flight tick. The projectile path is `FUN_80031dfc`, which
  installs the projectile callback on the spawned child. The old port code and
  probe comments that treated wtype 2 event `0` as unsafe projectile motion
  were stale and wrong.
- Wtype 2 source event map:
  - event `0`: mounted update through `FUN_8003c538(slot, vehicle)`
  - event `1`: initialize ammo/status (`+0x0c = 10`, `+0x08 = 1`)
  - event `2`: special fire, spawn projectile kind `5`, flag `0x01000000`,
    cooldown at `+0x0e`
  - event `9`, command `0x422`: combo fire, spawn kind `20`, flag
    `0x00010000`, consume two ammo
  - event `9`, command `0x424`: combo fire through the event `2` style kind
    `5` path
  - event `11`: normal fire, spawn kind `5`, consume one ammo, return `30`
  - event `12`: target angle gate using `FUN_800435c0` and `FUN_8004ecd4`
  - event `13`: weapon key `1`
  - event `14`: hardpoint key `0x8011`
- Wtype 2 verified host proof after this correction:
  `analysis/20260611_wtype2_dispatch_probe_after_gate.log` shows the same
  attached object pointer `00000000103F59F0` in `pickup_attach_begin`,
  `attach_stored`, `wtype2_mounted_dispatch` event `0`, event `11`, event `9`
  for `0x422` and `0x424`, `spawn_bullet weapon=00000000103F59F0`, and
  `attached_draw tag=player`, with zero `runtime_packet_skip`,
  `runtime_packet_invalid`, `FAIL`, or crash rows.
- Source event: wtype 6 is the mounted tracking/seeker path. It updates joint
  pitch/yaw toward the owner target and spawns missile children on fire/combo
  events.
- Source role distinction: wtype 2 now has source-proven mounted update and
  fire routes. Wtype 4 still only has the decoded projectile/spring callback in
  this port; do not invent its mounted update/fire route until raw SLUS or an
  emulator trace proves it. Current guarded host proof for wtype 4
  (`analysis/20260611_wtype4_guarded_probe_current.log`) shows attach/storage
  and draw, but explicitly logs `mounted_update_route_not_decoded` and
  `mounted_fire_route_not_decoded`.
- Seeker fidelity requirement: trace rows must show changing joint pitch/yaw
  before/after values, then a missile child spawn, then attached draw rows for
  both the base and rotating child joint, plus visible projectile submission.
- Trace rows:
  - `weapon_lifecycle wtype6_dispatch`
  - `weapon_lifecycle wtype6_init_ready`
  - `weapon_lifecycle wtype6_tracking`
  - `weapon_lifecycle wtype6_event11_projectile`
  - `weapon_lifecycle wtype6_combo_242_projectile`
  - `weapon_lifecycle wtype6_combo_244_first_projectile`
  - `weapon_lifecycle wtype6_combo_244_extra_projectile`

6. Projectile and effect creation helpers

- Source file: `src/gameplay/weapon_spawn.c`
- Functions:
  - `FUN_80031dfc` (`WeaponShot_SpawnBullet`)
  - `FUN_80032e48` (`WeaponShot_SpawnShard`)
  - `FUN_8003351c` (`WeaponShot_SpawnMissile`)
  - `FUN_800346cc` (`WeaponShot_SpawnGuided`)
  - `FUN_800354e0` (`WeaponShot_SpawnGeneric`)
- Source event: helpers allocate projectiles/effects from source pools, copy
  matrices, assign callbacks, set timers/velocity/targets, register objects in
  the world, play source sound events, and decrement weapon ammo.
- Trace rows:
  - `weapon_lifecycle spawn_bullet`
  - `weapon_lifecycle spawn_shard`
  - `weapon_lifecycle spawn_missile`
  - `weapon_lifecycle spawn_guided`
  - `weapon_lifecycle spawn_generic`

7. Weapon event dispatch, projectile collision, and special fire cases

- Source file: `src/gameplay/weapon_event.c`
- Functions:
  - `FUN_80031454` (`WeaponHit_Apply`)
  - `FUN_80031864` (`WeaponObj_Dispatch`)
  - `FUN_80034920` (`WeaponSpray_Dispatch`)
  - `FUN_8003733c` (`MissileTracker_Dispatch`)
- Source event: generic weapon callbacks handle event dispatch, event-0
  projectile update dispatch, event-11 fire/effect creation, spray combo
  projectiles, and projectile impact side effects.
- Trace rows:
  - `weapon_lifecycle event_dispatch`
  - `weapon_lifecycle fire_event0_projectile_dispatch`
  - `weapon_lifecycle fire_event11_effect_attached`
  - `weapon_lifecycle fire_event11_projectile_registered`
  - `weapon_lifecycle spray_event0_projectile_dispatch`
  - `weapon_lifecycle spray_combo_fire`
  - `weapon_lifecycle spray_combo_projectile`
  - `weapon_lifecycle spray_event11_projectile`

8. Renderer draw visibility for loose pickups, attached weapons, and projectiles

- Source file: `platform/renderer.c`
- Functions:
  - `draw_runtime_object_tree`
  - `draw_vehicle_attached_weapons`
  - `draw_machine_gun_projectiles`
  - runtime list draw loop for type-3 pickups and type-7 projectiles
- Host seam event: source runtime objects are converted to OpenGL draw meshes.
  This does not define gameplay truth, but it proves whether live source
  objects are visible in the host renderer.
- Trace rows:
  - `attached_draw tag=loose-pickup`
  - `attached_draw tag=static-pickup`
  - `attached_draw tag=player`
  - `attached_draw tag=ai`
  - `weapon_lifecycle renderer_projectile_draw`
  - `runtime_draw`
- Diagnostic renderer rows now include source primitive count, emitted
  triangle/quad/line counts, UV vertex count, and packet-kind histogram at the
  object boundary. This lets a capture distinguish "object exists but only
  half its source packet stream emitted" from culling, lighting, texture, or
  lifecycle identity problems.

## Required Proof Matrix

A weapon-fidelity pass is not complete until the following are true for every
attached weapon family and at least one live source pickup path:

1. Pickup visual before collision: `attached_draw tag=loose-pickup` or
   `tag=static-pickup` with nonzero primitive/vertex counts and no
   `runtime_packet_skip`/`runtime_packet_invalid`.
2. Attach transition: `pickup_attach_begin`, `attach_appended`, `attach_stored`,
   and `pickup_attach_complete` show the same child pointer entering a
   `vehicle+0x110..+0x118` active slot.
3. Mount location: `mounted_dispatch_begin` and `attached_draw tag=player` or
   `tag=ai` show the same child pointer, hardpoint pointer, parent/child links,
   and sane chain depth.
4. Mounted animation/tracking: event `0` rows show the source callback mutating
   aim/recoil/targeting state. For seeker/wtype6 this specifically means
   `wtype6_tracking` changes joint pitch/yaw.
5. Fire path: event `11` or combo events produce the source fire row and the
   appropriate `spawn_*` helper row.
6. Projectile/effect visibility: type-7/effect object rows include
   `renderer_projectile_draw` or equivalent source effect draw evidence after
   the fire row.
7. Visual packet fidelity: the mounted object and loose pickup rows include
   source primitive/packet histograms that match the expected object family
   without unsupported packet kinds, packet rejects, or host-fabricated quads.

## End-to-End Identity Gate

The issue is larger than pickup box geometry. A valid test has to prove the
whole source lifecycle for the same object identity:

1. Loose source pickup object is visible in the renderer as
   `attached_draw tag=loose-pickup` or `attached_draw tag=static-pickup`.
2. Pickup event `3` enters `common_spawn_child`, materializes one `child`, and
   emits `pickup_child_materialized`.
3. `pickup_attach_begin` logs the same `child` immediately before
   `FUN_8002cbe8(source, child)`.
4. `attach_appended`, `attach_stored`, and `attached_visual_register` log that
   same `child` in exactly one active source slot at
   `vehicle+0x110/+0x114/+0x118`.
5. `pickup_attach_complete` logs the same `child` after source slot storage.
6. `mounted_dispatch_begin` and `mounted_dispatch_ret` log the same `child`
   receiving mounted callback event `0` or fire/key events through the source
   slot dispatcher.
7. The mounted callback produces an animation or tracking row. Examples:
   `state_weapon_aim`, `state_weapon_fire_anim`, or `wtype6_tracking`.
8. The fire path produces a source creation row. Examples:
   `state_weapon_fire_projectile`, `state_weapon_fire_effect`,
   `wtype6_event11_projectile`, `spray_combo_projectile`, plus the helper row
   such as `spawn_missile`, `spawn_guided`, or `spawn_generic`.
9. Renderer rows prove visibility after source creation:
   `attached_draw tag=player` or `tag=ai` for the mounted child and
   `weapon_lifecycle renderer_projectile_draw` for projectile/effect runtime
   objects.
10. Cleanup does not leave stale host visuals. Slot drop/eject paths must show
    `slot_drop_start` or `attach_eject_slot`, `slot_spawn_*` when source spawns
    a dropped weapon, and `attached_visual_clear` for the removed source child.

Counts alone are not enough. The same pointer must be followed across attach,
dispatch, fire, draw, and cleanup rows wherever the trace has enough evidence
to identify it.

## Current Evidence Snapshot

Captured logs:

- `analysis/20260610_weapon_lifecycle_probe_v8_latest.log`
- `analysis/20260610_weapon_lifecycle_visual_probe_v8_latest.log`
- `analysis/20260610_weapon_lifecycle_expanded_probe_after_guard.log`
- `analysis/20260610_weapon_visual_wtype6_after_guard.log`
- `analysis/20260611_weapon_probe_after_default_texture.log`
- `analysis/20260611_weapon_visual_wtype6_after_default_texture.log`
- `analysis/20260611_live_pickup_wtype14_after_default_texture.log`
- `analysis/20260611_slick_wtype14_after_cache_guard.log`
- `analysis/20260611_weapon_chain_matrix_skiresrt.csv`
- `analysis/20260611_weapon_probe_after_cache_guard.log`
- `analysis/20260611_current_wtype6_attached_oblique.log`

The latest traces contain live rows for:

- pickup child materialization and positioning
- attach append and attach storage into vehicle slots
- wtype 6 target tracking with changing joint yaw/pitch
- state weapon aim and gated state fire attempts
- missile/generic projectile spawn helpers
- renderer projectile draw rows
- loose pickup and attached player weapon draw rows

Important boundary: `--weapon-probe` has two different forms of evidence.
Real pickup evidence comes from source pickup objects and is strict. Forced
state evidence constructs a state weapon on the current vehicle solely for
coverage; if the current vehicle bank/hardpoint cannot materialize the
requested kind, the probe now logs `weapon_probe state=N skip construct ...`
instead of crashing. That skip is not visual fidelity evidence for that state.
It only proves the artificial construction was not a valid source-bank path for
the selected vehicle. Use real pickup/mounted traces and render visual probes
for live attachment fidelity.

The wtype 6 visual probe currently proves the full live chain:

- `pickup_child_materialized` and `attach_stored` for the source pickup path
- `wtype6_tracking` with changing joint pitch/yaw
- `spawn_missile` and `wtype6_event11_projectile`
- `attached_draw tag=player` for the mounted weapon and rotating child joint
- `renderer_projectile_draw` for visible projectile submission

2026-06-11 source/host decoder finding:

- Source `FUN_8001b49c` cases `0xc` and `0xd` do not make default texture
  records disappear. Case `0xc` treats low 14 bits `0x3fff` as
  `DAT_80065a28`; case `0xd` treats raw `0xffff` the same way. Both still
  emit textured primitives.
- Host `platform/xobf_texture.c` had been returning no UVs for raw `0xffff`,
  which changed valid source textured faces into host fallback/invalid visual
  faces. This is a source-backed explanation for visually incomplete pickup
  boxes and attached weapon panels.
- The 2026-06-11 build restores the textured path with a host
  `runtime_default_texture_fallback` trace row. Current verification:
  live wtype14 pickup smoke PASS, one `pickup_child_materialized`, one
  `attach_stored`, post-collision `attached_draw tag=player`, 221 loose pickup
  draw rows, 32 fallback rows, and zero `runtime_packet_skip` /
  `runtime_packet_invalid` rows.
- The 2026-06-11 wtype6 visual probe logs 482 `attached_draw tag=player`
  rows, 479 `wtype6_tracking` rows with changing joint rotation, 416
  `renderer_projectile_draw` rows, 32 default texture fallback rows, and zero
  runtime packet rejects. Screenshot:
  `analysis/20260611_weapon_visual_wtype6_after_default_texture.png`.

2026-06-11 cache-guard correction and stricter lifecycle matrix:

- A broad SKIRESRT vehicle/weapon matrix found a false-pass case:
  Slick Clyde (`--vehicle 1`) plus wtype 14 materialized, attached, and drew
  one row, then crashed with `0xc0000005`.
- Symbol resolution from `PS1 game/v8.map` placed the fault at `rdptr32p`
  inside `runtime_cached_mesh_for_obj`. Host code read `cached+0x1c` before
  proving `obj+0x30` was a valid source cached primitive header. If `cached`
  was null or source-invalid, the host dereferenced `0x1c`.
- `platform/renderer.c` now reads generated packet pointer `cached+0x1c`
  only after `runtime_cached_fields()` validates the source cache header.
  This preserves source behavior while preventing invalid host dereferences.
- The exact repro now passes:
  `analysis/20260611_slick_wtype14_after_cache_guard.log` has one
  `pickup_child_materialized`, one `attach_stored`, 216
  `attached_draw tag=player` rows, 355 `renderer_projectile_draw` rows, and
  zero `FATAL`, `FAIL`, `runtime_packet_skip`, or `runtime_packet_invalid`
  rows.
- The stricter SKIRESRT matrix covers normal roster vehicles 0..11 against
  wtypes 2, 4, 6, 8, 9, and 14. Every one of 72 combinations requires:
  process exit 0, `pickup_child_materialized > 0`, `attach_stored > 0`,
  `attached_draw tag=player > 0`, no `FATAL`, no `FAIL`, no packet skips, and
  no packet-invalid rows. Wtype 6 additionally requires `wtype6_tracking > 0`
  and `renderer_projectile_draw > 0`. The matrix passes; evidence is in
  `analysis/20260611_weapon_chain_matrix_skiresrt.csv`.
- Visual screenshots from `V8_CAMERA_ATTACHED=top/0` are useful for crash and
  broad placement sanity, but not sufficient as final visual proof for the
  small top-mounted seeker mesh because the current camera angle can hide the
  mesh behind the vehicle body. Keep lifecycle counters and source traces as
  hard pass/fail gates, but do not treat those screenshots alone as complete
  human-visible fidelity proof.

Current full-chain gate for this issue:

- pickup population/materialization: `pickup_child_materialized`
- attachment moment/storage: `attach_appended` and `attach_stored`
- mounted update/animation: state callback rows and, for seeker, changing
  `wtype6_tracking` pitch/yaw on the rotating child joint
- fire/effect/projectile creation: state fire rows plus spawn helper rows
  (`spawn_missile`, `spawn_generic`, etc.)
- renderer submission: `attached_draw tag=player` for mounted base/children
  and `weapon_lifecycle renderer_projectile_draw` for visible projectiles
- visual decoder health: no `runtime_packet_skip`, no `runtime_packet_invalid`,
  and no host crash/FATAL rows

## Remaining Active Problem

Lifecycle coverage does not prove visual fidelity. The active bug remains in
the live visual path for pickup/attached weapon geometry and mount fidelity:

- pickup boxes can still appear visually incomplete in user screenshots
- pickup center/pivot behavior still needs source-exact confirmation
- attached weapon meshes must be validated per vehicle hardpoint
- seeker must be validated as both tracking in source state and visually
  rotating/projectile-visible in the host renderer

Do not close the attached-weapon visual-fidelity goal until the live runtime
path proves all of the above, not just the pickup materialization step.

2026-06-11 expanded verification boundary:

- `--weapon-visual-probe` now distinguishes authored source pickups from
  synthetic probes. If an authored pickup of the requested wtype exists, the
  probe uses that object and offsets it from the vehicle before invoking the
  real pickup callback. That produces `weapon_visual_probe authored_pickup`
  and `forced_zero_offset=0` rows, followed by a nonzero
  `pickup_child_positioned pickup_delta`.
- Synthetic fallback remains available, but it emits
  `weapon_probe synthetic_pickup` and `forced_zero_offset=1`. Treat those rows
  as callback/tracking/projectile coverage only. They are not mounted visual
  fidelity proof because they can collapse the pickup, child, and vehicle
  origins.
- Current SKIRESRT authored/synthetic sweep:
  `analysis/20260611_authored_sweep_summary.txt`

  | wtype | authored | synthetic | attach | draw | tracking | projectile | bad |
  | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
  | 2 | 1 | 0 | 1 | 71 | 0 | 358 | 0 |
  | 4 | 1 | 0 | 1 | 71 | 0 | 358 | 0 |
  | 6 | 0 | 1 | 1 | 142 | 139 | 358 | 0 |
  | 8 | 0 | 1 | 1 | 71 | 0 | 358 | 0 |
  | 9 | 1 | 0 | 1 | 142 | 0 | 358 | 0 |
  | 14 | 1 | 0 | 1 | 71 | 0 | 358 | 0 |

- The offset-preserving wtype14 run proves the harness: it logs
  `authored=1`, `forced_zero_offset=0`, `pickup_delta=(16384,8192,65536)`,
  active slot storage, attached draw chain, state aim callback rows, projectile
  draw rows, and zero packet rejects.
- The wtype6/seeker run still falls back to synthetic on SKIRESRT. It proves
  callback tracking and projectile rendering, but not authored pickup/mount
  placement fidelity. Seeker visual fidelity remains open until an authored
  source path or executable trace proves the same full chain with a nonzero
  pickup-to-vehicle delta.
- The 2026-06-11 expanded trace build
  `SHA256 7CAEC6D2ACB6C0D86663F65D5C67470B948E943E14431E622E16CE7DC91D7F7C`
  adds explicit attach-bracketing and mounted-dispatch rows. AIRGRAVE authored
  wtype6 visual probe
  `analysis/20260611_lifecycle_scope_airgrave_wtype6_b.log` exits 0 with
  `pickup_child_materialized=1`, `pickup_attach_begin=1`,
  `attach_stored=1`, `pickup_attach_complete=1`,
  `mounted_dispatch_begin=218`, `mounted_dispatch_ret=218`,
  `wtype6_tracking=219`, `renderer_projectile_draw=419`, and zero packet
  rejects. This proves authored attach, mounted event-0 dispatch/tracking, and
  visible projectile submission in a render run, but it does not itself prove
  mounted event-11 fire because that probe does not press the fire path.
- AIRGRAVE source weapon probe
  `analysis/20260611_lifecycle_scope_airgrave_weapon_probe.log` exits 0 and
  reaches `weapon_probe PASS failures=0`; it includes multiple
  `pickup_attach_begin`/`attach_stored`/`pickup_attach_complete` rows,
  `wtype6_event11_projectile=1`, `wtype6_combo_242_projectile=1`, and
  `spawn_missile=2`. This proves source fire/combo projectile creation. Treat
  it as callback/fire coverage, not visual fidelity, because it is the source
  probe rather than the mounted visual render path.
