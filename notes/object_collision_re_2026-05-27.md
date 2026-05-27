# Object Collision RE Notes - 2026-05-27

Scope: RE-only pass over object collision, obstacle streams, health, damage, destruction, and the callback contract. No implementation decisions are made here.

## Critical Corrections

- Runtime object collision is not raw visual polygon collision. It is driven by authored object/obstacle streams stored in XOBF and attached to runtime objects at `obj+0x5c`.
- `obj+0x5c` is a union. For ordinary scenery/props it is a collision/obstacle stream. For vehicles it is used as vehicle bounding/damage data. Callers must be interpreted by object kind.
- `FORM OBJ/LGHT` / loader `LOAD 80100e20` is light data, not collision AABB data, despite stale helper naming in cleaned code.
- The bone/object tree links are `obj+0x34 = next sibling`, `obj+0x38 = first child`, `obj+0x3c = parent`. Some comments still claim the reverse.
- Shape test and destruction are separate. The low-level collision pass only detects a hit and dispatches event `3`; health changes happen in callbacks.

## XOBF Object/Collision Data

XOBF BIN header:

- `+0x00 u32`: render group count.
- `+0x04 u32`: render group pointer table offset.
- `+0x08 u32`: obstacle table count minus one.
- `+0x0c u32`: obstacle table offset.
- `+0x10 u32`: secondary table/count.
- `+0x14 u32`: first offset after obstacle table region.
- `+0x18 u32`: slot count.
- `+0x1c`: slot records, 0x1c bytes each.

Slot record layout:

- `+0x00 i16 key0`: render group key. Non-negative keys map to primary render groups. `0xcxxx` keys are secondary visual groups found by `FUN_8001b0c4`.
- `+0x02 i16 key1`: obstacle stream index. If non-negative, this indexes the XOBF obstacle table and becomes runtime `obj+0x5c`.
- `+0x04/+0x08/+0x0c i32`: local X/Y/Z.
- `+0x10/+0x12/+0x14 i16`: local rotations.
- `+0x16 i16`: object flags/id copied into runtime fields.
- `+0x18 i16`: next sibling slot, runtime `+0x34`.
- `+0x1a i16`: first child slot, runtime `+0x38`.

Construction:

- `FUN_8001ac44` builds runtime object trees from slot records.
- `FUN_8001add0` fills/rebinds an existing tree from the same slot structure.
- `FUN_8001b49c` builds render packet/bounds data at `obj+0x30`.
- `FUN_8001b0c4` finds `0xc000` secondary visual groups and writes `obj+0x68/+0x6c`.
- `CollTree_HasContact` / `FUN_8001ec48` recursively marks `flag 0x800` when descendants contain collision streams, enabling child traversal at runtime.

Obstacle stream record kinds:

- Kind `0`: terminator.
- Kind `1`: 0x1c-byte AABB/flat obstacle record.
  - `+0x04 i32 minX`
  - `+0x08 i32 overrideY`
  - `+0x0c i32 minZ`
  - `+0x10 i32 maxX`
  - `+0x14 i32 max/local Y ceiling`
  - `+0x18 i32 maxZ`
- Kind `2`: convex/plane record. Header is `i16 kind`, `u16 plane_count`, followed by `plane_count * 12` bytes:
  - `i16 nx, ny, nz, pad`
  - `i32 plane_offset`
- Kind `3`: referenced as a split/tree kind in older comments, but no WILDWEST stream uses it and direct shape dispatch currently handles only kinds 1 and 2.

Kind `2` correction:

- The records are not visual mesh faces and not a standalone sphere/primitive list. The same 12-byte plane records are consumed by both object collision and terrain/object support.
- In `CollShape_Dispatch`, each record is passed to the separation-plane helper. The first 8 bytes are read as an `SVECTOR` normal plus pad, and the final i32 is the plane offset/support value.
- In `ObstacleLeaf_Test`, the normal is rotated into world space, then the code solves a vertical probe ray against all halfspaces. It tracks lower and upper Y intersections and accepts only when the candidate lies inside the convex patch.
- Therefore kind `2` is best named a convex halfspace/plane-list patch. Treating it as a visual mesh, decal, or simple flat box will produce wrong collision and wrong drivable support.

Wild West audit facts from current data:

- Bank 0: 223 render groups, 699 slots, 70 obstacle table entries.
- Bank 1: 15 render groups, 15 slots, 12 obstacle table entries.
- Slots with `key1 >= 0`: 80.
- Original traversal roots: 118.
- Placed slots: 130.
- Used obstacle streams: 38.
- Observed obstacle records: kind 1 = 130, kind 2 = 11, terminator = 122.
- Detailed WILDWEST-only stream dump is in `analysis/object_collision_wildwest_stream_detail.txt`.

WILDWEST kind `2` streams observed:

- `M1depot_1`: two sloped/convex patches, 7 planes and 5 planes.
- `M1_warehouse_1`: one 5-plane patch.
- `M1teepee_1`: one 4-plane patch, referenced by six placements.
- `bonfire_1`: one 5-plane patch, referenced by two placements.

WILDWEST train-related streams observed:

- `M1train_engine_1`: kind 1 box, stream 26.
- `M1train_coalcar_1`: kind 1 box, stream 28.
- `M1train_flatbed_1`: main flatbed kind 1 box, stream 30.
- `M1train_flatbed_1`: four small cube/child kind 1 streams, streams 31-34.

No WILDWEST-used stream contains kind `3`. The map's static object collision format, for this baseline level, is fully covered by kind `0`, kind `1`, and kind `2`.

## Runtime Collision Frame

`Frame_CollideAll` / `FUN_8002169c` is the frame-level dispatcher.

Per frame it:

1. Iterates the live object list from `piRam00000714`, with fallback `DAT_80065a18`.
2. Skips any object with flag `0x20`.
3. Clears the two per-object contact slots at `obj+0x74` and `obj+0x78`.
4. Pair-tests later live objects unless the other object has flag `0x20`, or both objects have flag `0x200`.
5. If the object does not have flag `0x100`, also traverses static kd-tree root `uRam000006fc` using `Tree_CollideObject` / `FUN_80020f14`.

`Tree_CollideObject` traverses a static kd-tree:

- Node kind `0`: leaf list.
- Node kind `1`: X split.
- Node kind `2`: Z split.
- Split traversal compares object `+0x24/+0x2c` and radius `+0x54`.
- Leaf entries call `Object_TestCollision` / `FUN_8001edb4` unless static object flag `0x20` is set.

`Object_TestCollision`:

- Rejects same collision layer/spawn id: int16 `obj+6` equal means no collision.
- Broadphase compares absolute position deltas at `+0x24/+0x28/+0x2c` to the sum of radii at `+0x54`.
- If candidate B has flag `0x40`, B is stored into A's contact slots `+0x74`, then `+0x78`.
- If candidate B has flag `0x800`, traverses child collision streams through `CollTree_Traverse`.
- Otherwise or after child failure, dispatches direct shape test through `CollShape_Dispatch`.
- On hit, builds a scratchpad collision descriptor and calls A callback with event/mode `3`.
- If A callback returns 0 or -1, swaps descriptor order and calls B callback with event `3`.
- A positive nonzero return from A consumes the hit and B is not called.
- A negative return contributes early-exit bits to the frame/static traversal. A's negative return sets bit 0; B's negative return sets bit 1.
- Static tree traversal uses awkward inverted return values in leaves: the important behavior is that a negative callback result can stop further testing.

Callback wrapper:

- `FUN_8001e120(obj, mode, arg)` is the generic callback invoke helper.
- If `obj+0x64` is null, it returns 0.
- Otherwise it saves/restores the real stack/return address through scratchpad locations and calls the callback with the original object, mode, and argument.
- There is no hidden dispatch table in this wrapper. All collision/destruction semantics are in the per-object callback or the default callback it delegates to.

`CollTree_Traverse`:

- Starts at B first child `+0x38`.
- Walks siblings through `+0x34`.
- Skips children without `+0x5c` and without descendant flag `0x800`.
- Composes child matrix with parent matrix before testing its shape stream.

`CollShape_Dispatch`:

- Reads stream pointer from `obj+0x5c`.
- Supports kind 1 AABB-like tests and kind 2 convex/plane tests.
- AABB-vs-AABB checks overlap in both directions.
- AABB-vs-plane-list uses `ShapePlane_SepTest`.
- Hit descriptor layout, as used by later code:
  - `+0x00`: current/other object, overwritten during callback ordering.
  - `+0x04`: shape A.
  - `+0x08`: shape B.
  - `+0x0c`: object A.
  - `+0x10`: object B / candidate chain root.
  - `+0x14..+0x1c`: contact point.
  - `+0x20`: contact normal axis.
  - `+0x28`: local axis.
  - `+0x30`: overlap/restitution/depth.

`FUN_8001f5a0` fills/refines the response axis/contact point for later collision response. It expects the host shape to be kind 1 and handles candidate kind 1 or kind 2.

## Terrain/Object Obstacle Probe

Terrain/object support is a second use of the same obstacle streams.

`Terrain_HeightAndProbe` / `FUN_8001d748`:

- Samples terrain height/material/normal.
- If `obj+0x74` is nonzero, calls `Object_FindObstacleAt` on that object.
- If no hit and `obj+0x78` is nonzero, tries the second contact object.
- If object obstacle overrides terrain, returned material is 0.

`Object_FindObstacleAt` / `FUN_8001f51c`:

- If object has flag `0x800`, walks children through `ObstacleChain_Walk`.
- Always tests the parent stream at `obj+0x5c`.

`ObstacleLeaf_Test`:

- Kind 1 checks whether the transformed point is inside a local box and returns the record's override Y.
- Kind 2 treats records as convex/sloped patches. It tracks lower/upper Y intersections and accepts when the lower plane is below upper, the result beats terrain, the vertical band is within `0x2800`, and the selected normal has `normal.y < -0x800`.
- The kind 2 math uses `plane_offset * 0x1000 - normal.x * localX - normal.z * localZ`, divided by the rotated Y component of the normal. Normals with zero Y are side walls; negative Y updates the lower support plane, positive Y updates the upper reject plane.

This path explains why object collision streams affect driving surface height even when no destruction occurs.

## Health and Destruction

Health is usually loaded from `STRN` by `XOBF_LoadHealth`:

- `obj+0x0c`: health.
- `obj+0x0e`: max health.
- For type byte `obj+4 == 5` and non-negative layer/spawn `obj+6`, both are scaled by difficulty as `(value * (difficulty + 2)) / 4` with MIPS round-toward-zero behavior.
- Child objects inherit parent health when their own loaded health is zero.

Universal destructible helpers:

- `Damage_AccumulateOrFire` / `FUN_80022320(self, amount)`:
  - If flag `0x8000` is set, damage is ignored.
  - If `health < amount`, health resets to max health, an explosion/effect is requested with `FUN_8003fc50`, world callback event `0x11` is broadcast if the effect exists, and it returns 1.
  - Otherwise subtracts amount and returns 0.
  - Exact equality leaves health at 0 and does not explode in this helper call because the comparison is strict `<`.
- `Damage_FromImpulse` / `FUN_8002239c(self, impulse)`:
  - Reads the hit object from the collision descriptor.
  - Only applies if the hit object kind byte is `7`.
  - Damage amount comes from hit object `+0x0c`.
- `Object_DefaultDispatch` / `FUN_800223dc`:
  - Event 3: `Damage_FromImpulse`.
  - Event 8: `Damage_AccumulateOrFire`.
  - Event 1: effect/score bookkeeping.
  - On destruction, increments score/stat global by max health.

Removal:

- `Damage_Apply` / `FUN_800205f8` removes an object from the live list or static tree, then retires it.
- `Object_Free` / `FUN_80020540` frees secondary visual/bone resources and recursively unregisters the subtree.
- `FUN_800204dc` sends event 4 callbacks before removing scene-list entries and freeing bone-level data.

Scene/list ownership:

- `Object_RegisterInScene` / `FUN_800202f4` always inserts the object into `DAT_80065a18`.
- If flag `0x04` is set, it also inserts into `DAT_80065a80`.
- If flag `0x80` is set, it also inserts into `DAT_80065a60`.
- `FUN_800207c4` sets `0x04` and inserts into `DAT_80065a80`; `FUN_800207f8` clears `0x04` and removes from that list.
- `FUN_80020744` sets `0x80` and inserts into `DAT_80065a60`; `FUN_80020778` clears `0x80` and removes from that list.
- Static kd-tree ownership is separate: `Damage_RouteByTree` first removes from the live list, and if not present searches/removes from the static tree.

Delayed post events:

- `Object_SchedulePostEvent` / `FUN_80020890` maintains a sorted queue of `{next, prev, obj, deadline}` nodes.
- If the object already has flag bit `0x01`, its old queue node is removed first.
- New deadline is `current_frame + delay`; the object flag bit is set while queued.
- Queue processing clears bit `0x01` and invokes the object callback with mode `2`.
- This is why many "destroyed" objects are not immediately freed on collision. Collision event `3` or forced damage event `8` often schedules mode `2`, and the callback does final detach/retire/visual state later.

Destruction is callback-owned. A collision hit alone does not imply removal; the callback decides whether damage applies, whether an effect is spawned, and whether the object should be retired immediately or after a timer.

## Vehicle/Object Collision

Vehicle collision callback `FUN_8002d82c` is the heavy event-3 responder.

It decodes the collision descriptor:

- `+0x00`: other object.
- `+0x10`: sub-object pointer; kind 3 is treated specially/skipped.
- `+0x14`: contact velocity/point vector.
- `+0x20`: normal.
- `+0x30`: depth/restitution coefficient.

Other-object kind handling:

- Kind `7`: weapon/projectile. Vehicle takes damage through `FUN_8002c6fc(self, -other_damage, contact, 1)` and hit/kill bookkeeping may run.
- Kind `1`: light collision damage, capped around self health / 4, then `FUN_8002c958`.
- Kind `2`: vehicle-vs-vehicle elastic response; both sides receive impulses and may take damage/SFX.
- Default: static obstacle/prop contact. Floor-like contacts from contact slots can be ignored when `normal.y < -0x800`; otherwise the vehicle may call the obstacle callback with event 8 using penetration-derived damage. If the obstacle handles it, physical response can be skipped. If not, the vehicle applies an inelastic collision impulse to itself.

Vehicle damage helper:

- `FUN_8002c958` dispatches to abort/cleanup path `FUN_8002c018` if vehicle `+0x11e` is nonzero; otherwise it calls `FUN_8002c6fc`.
- `FUN_8002c6fc` distributes damage into vehicle zones using contact location relative to the bbox pointer at vehicle `+0x5c`.
- Vehicle zone subobjects at `+0xec/+0xf0/+0xf4` can explode independently.
- All three zones zero can soft-kill or destroy the vehicle depending on residual damage and flags.

Impulse physics:

- `FUN_800176f8` applies world impulse at a contact point: adds linear velocity at `+0x80..+0x88`, computes `(contact - position) x impulse`, rotates to local frame, scales by inverse inertia `+0x9c/+0x9e/+0xa0`, and adds angular velocity at `+0x90/+0x94/+0x98`.
- `FUN_80017ba8` applies standard inelastic collision response when relative velocity dot normal is negative.
- `FUN_80017594` is the lower-level angular/linear impulse helper used by wheel and collision code.
- `Object_OBBSuspension` / `FUN_8001787c` probes an object's 8 bbox corners against terrain/object obstacle height, accumulates spring/damper force and torque, then integrates.

## Movable Object Physics Bridge

`Object_GeneralTick` / `FUN_80030c08` is the common movable-object integrator for non-player dynamic props/projectiles that are not on the full vehicle path.

Per tick it:

- Calls `Terrain_HeightAndProbe`, so the same `obj+0x74/+0x78` contact-slot obstacle streams affect dynamic prop support.
- Computes terrain/object penetration and derives bounce/support response.
- Updates position/orientation through `Object_IntegrateAndOrient`.
- Applies angular damping by multiplying angular velocity by `0xf80 >> 12`.
- Applies linear drag by subtracting velocity/64.
- Maintains previous-position trail fields and callback-local timers.

This bridges authored static collision to loose props: static streams are not only blockers, they can become support surfaces for dynamic objects through the obstacle probe path.

## Projectile/Weapon Collision

Projectile callbacks also receive event 3 from the same collision system.

`WeaponHit_Apply` / `FUN_80031454`:

- Ignores collider kind 3.
- If collider kind is 2, computes impulse from projectile damage/speed and collider mass-ish field `+0xa2`, clamps components to `[-0x80000, +0x80000]`, and applies impulse with `FUN_800176f8`.
- If vehicle target has player/slot id negative, triggers HUD damage feedback.
- Plays SFX when not suppressed.
- Spawns debris/effect at projectile position.
- Broadcasts world callback event `0x11`.
- Retires the projectile.
- Returns `-1` for consumed collision, 0 for ignored terrain/player kind path.

Known projectile event-3 variants:

- Bullet `LAB_80031634`: ignores collider kind 3, otherwise impact effect/SFX and retire.
- Slow/sticky projectile `LAB_80031bbc`: if flag `0x10000` and collider kind 2, attaches as child to the vehicle and changes callback; otherwise calls `WeaponHit_Apply`.
- Homing projectile `LAB_80033290`: unarmed uses `WeaponHit_Apply`; armed ignores terrain kind 3, otherwise spawns explosion debris, applies vehicle impulse, and retires.
- Mine rolling/dead variants: usually call `WeaponHit_Apply`; large mine ignores kind 7 to avoid self/proxy collision.
- Attached bomb tick applies fixed vehicle damage to its parent via `FUN_8002c958(parent, -40, pos, 1)`.

## Wild West Object Families Observed

These are WILDWEST-specific callbacks, but they all use the same global event contract.

- Shack (`FUN_801005e4`): mode 1/2 scheduling and SFX; otherwise falls through default destructible dispatch.
- Bridge (`FUN_801003ec`): mode 3 weapon hit and mode 8 forced damage; on success sets a bone-level state and queues particles.
- Gallow (`FUN_80100fa4`): mode 0 retires when health is zero; mode 3/8 damage path; on kill broadcasts event 9, spawns explosion, and sets flag `0x10000` for matching ids.
- Hotel/bonfire misc (`FUN_8010035c`/nearby): requires hit object kind 7 for mode 3; uses hit damage and may set paired object state.
- Bridge collapse (`FUN_801010a8`): projectile damage can spawn debris/rigid body and schedule detach/damage; mode 8 is forced damage; mode 9 marks matching layer.
- Dynamite keg (`FUN_801009a8`): projectile hit can light the keg, clear static-query flag, set velocity/timer/FX; kind 2 path reflects/applies damage to impactor.
- Train (`FUN_801004cc` plus init/tick): train pathing/movement refits bounds through `FUN_8001787c`; default dispatch still handles object damage where applicable.
- Stage coach (`FUN_8010129c`): mode 0 follows a path and locks Y to terrain; mode 3/8 standard damage.
- Saloon destruct chunks (`FUN_8010178c`): rolling debris bounces with terrain/collision response, can attach to vehicles, then detach/retire by events/timers.

## Object Flags and Fields

Common flags:

- `0x00000001`: active/tick list membership.
- `0x00000004`: world-bind/draw/physics active.
- `0x00000008`: owns scenery attach/bone-bank slot.
- `0x00000020`: inactive/dead/pass-through for collision.
- `0x00000040`: eligible to be stored in other objects' contact slots.
- `0x00000080`: post-update/back-buffer pending list.
- `0x00000100`: skip static kd-tree collision query.
- `0x00000200`: pairwise skip if both objects have it.
- `0x00000800`: subtree contains collidable descendants.
- `0x00008000`: damage ignored or delayed-effect/in-progress marker, callback-dependent.
- `0x00010000`: detached/event/broadcast marker, callback-dependent.
- `0x00018000`: vehicle collision response flags.
- `0x00020000`: vehicle sound entity already initialized.

Common object fields:

- `+0x00`: flags.
- `+0x04`: object kind/type byte.
- `+0x06`: collision layer/spawn id or player/slot id depending kind.
- `+0x0a`: bone/slot index.
- `+0x0c`: health or projectile damage amount.
- `+0x0e`: max health.
- `+0x10`: matrix.
- `+0x24/+0x28/+0x2c`: render/current position.
- `+0x30`: bone-level render/bounds handle.
- `+0x34`: next sibling.
- `+0x38`: first child.
- `+0x3c`: parent.
- `+0x48/+0x4c/+0x50`: world/physics position or matrix translation.
- `+0x54`: broadphase radius.
- `+0x58`: XOBF bank pointer.
- `+0x5c`: collision stream for props, bbox/damage data for vehicles.
- `+0x60`: animation slot/state.
- `+0x64`: callback pointer.
- `+0x68/+0x6c`: secondary visual group handle/rate.
- `+0x74/+0x78`: contact slots populated by broadphase candidates with flag `0x40`.
- `+0x80/+0x84/+0x88`: linear velocity for movable objects, callback-local state for some static objects.
- `+0x90/+0x94/+0x98`: angular velocity or callback-local state.
- `+0x9c/+0x9e/+0xa0`: inverse inertia factors for impulse response.
- Vehicle damage zones: `+0xec/+0xf0/+0xf4`.

## Current Confidence / Remaining Unknowns

High confidence:

- Slot-to-runtime object tree mapping.
- Meaning of `key1` as obstacle stream index.
- Runtime collision frame order and callback protocol.
- Shape stream kinds 1 and 2.
- Complete WILDWEST-used obstacle stream grammar: kinds 0, 1, and 2 only.
- Terrain/object obstacle override path.
- Scene list/static kd-tree ownership and delayed event-queue lifecycle.
- Universal health/destruction helper behavior.
- Projectile `WeaponHit_Apply` bridge.
- Vehicle impulse and damage-distribution entry points.

Still not fully named:

- Exact semantic labels for every callback-local use of flags `0x8000` and `0x10000`.
- Whether kind 3 obstacle streams exist outside WILDWEST or only in an older/unused dispatch path. This is deliberately not resolved in this pass because WILDWEST is the baseline map.
- Some level-specific callback cleaned files have stale comments or incomplete active names compared with Ghidra; treat source comments as lower authority than MIPS/Ghidra refs for final implementation.

Implementation implication:

To reproduce WILDWEST object collision correctly, implementation must load XOBF obstacle streams into the runtime object tree, preserve child/sibling traversal and `flag 0x800`, run the static kd-tree/broadphase/contact-slot flow, use the same streams for terrain/object support probes, honor delayed mode-2 lifecycle events, and dispatch event 3/event 8 callbacks for damage. Visual mesh data alone is insufficient.
