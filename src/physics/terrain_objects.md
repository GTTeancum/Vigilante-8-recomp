# Terrain DLL: per-level object physics

Each `input/TERRAIN/<LEVEL>.DLL` overlay supplies the per-object
physics + AI code for that level's destructibles, traps, and
level-specific events.

## Shared pattern

Per Ghidra analysis of all 10 terrain DLLs, every overlay follows the
same skeleton:

1. **String table** (early in image) holds the per-object names. The
   first entry is always the level codename (e.g. `AirGrave`,
   `WildWest`); subsequent strings are object instance names typically
   suffixed `_1`, `_2`, ... (e.g. `Crane_1`, `Shack_1`,
   `TBridge_1`).
2. **Object-template construction code** that, on overlay load, builds
   each named object's collision/AI record and registers it with the
   global object table via `XOBF_AppendExtra` (LOAD.DLL @ 0x80100408).
3. **Per-object tick callbacks** -- short functions invoked from the
   match-tick dispatcher for each registered object that requested
   physics. These are the actual gameplay loops we need.
4. **Per-object destroy callbacks** -- triggered when health hits zero;
   spawn fragments + sound.

## Recovered object name registries

| Level         | DLL          | Function count | Sample objects                                  |
|---------------|--------------|----------------|-------------------------------------------------|
| Aircraft Graveyard | AIRGRAVE | 6           | Crane_1, b17_1, control_tower_1                 |
| Canyonlands   | CANYNLND     | 5              | boulder_L, BeamUp, overpass_1, bridge_1         |
| Casino City   | CASNOCTY     | 13             | BurgerS_1, palm1_1, manhole_1, blimp_1          |
| Hoover Dam    | HOOVRDAM     | 14             | parking_meter_1, transformer_1, Dam_Lever       |
| Oil Fields    | OILFIELD     | 9              | rig_1, pipe_end_1, OilPump_1, sphere_1          |
| Sand Factory  | SANDFACT     | 5              | factory_1, M2_elevator_1, M2_Conveyor           |
| Secret Base   | SCRTBASE     | 16             | radar_1, fence_left, fence_right, turret_1, MSilo, aurora_1, catwalk_1, protoSaucer |
| Ski Resort    | SKIRESRT     | 12             | gondola_1, liftpole_1, liftstation_1, snow_mach_1, Ball_1, Ball_2 |
| Valley Farms  | VALLYFRM     | 8              | Windmill_1, silo_1, pump_1                      |
| Wild West     | WILDWEST     | 12             | Shack_1, TBridge_1, Gallow_1, hotel_1, M1train_engine_1, M1train_coalcar_1 |

## Hand-cleaned

So far one per-tick function recovered:
- **AIRGRAVE FUN_8010068c -> `AGTracker_Tick`** (src/physics/airgrave/tracker.c)
  -- the homing tracker shared by Crane_1 and b17_1.

## Pass 3 targets

For each remaining terrain DLL, pass 3 should:
1. Locate the `Renderer_BuildObject` / `XOBF_AppendExtra` calls in the
   load function -- they record which Object struct each per-tick
   callback animates.
2. Promote those tick callbacks into `src/physics/<level>/<object>.c`
   files (similar to how AGTracker_Tick lives in airgrave/tracker.c).
3. Identify shared sub-routines that all DLLs link against in the main
   EXE (e.g. `ratan2`, `Projectile_Spawn_AG`'s analogues, the per-tick
   damage accumulator) and document the seam.
