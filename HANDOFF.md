# Session Handoff — Vigilante 8 Recomp
Generated: 2026-05-20 (end of vehicle-damaged-physics session)

---

## What Was Just Finished

| Function | Name | File | Size |
|---|---|---|---|
| FUN_8002efe0 | Vehicle_DamagedPhysics | src/gameplay/vehicle_damaged_physics.c | 620 instr |

Three mode paths:

- **mode 2** (normal per-frame): direction-bit/inputMul update (opposing-dir forces → flip
  direction or negate maxInput; normal → FUN_8002ee94/c59c dispatch + latch/decay inputMul).
  Tilt physics (`flags&0x1800`): upright (`mat[1][1]>0`) computes skid flag `0x40000000`
  via `mat[0][2]*vx/128 + mat[2][2]*vz/128 < -0x4c4000`; steer-force (`flags&0x400`)
  increments steer ±32 and adjusts dragMass by ±0x500; coast-drift moves steer toward limits
  then applies `steer * max(0, sub04+speed*sub06/4096) / 16` to dragMass. Flipped path
  adjusts `self+0x98` ±0x4000. Drag-only (`flags&0x1800==0`): steer decays via
  `steer(u16) - RTZ(steer*speed/32768)`.

- **mode 3** (state init): steer = `(lut[0x10]-128)*5`; direction flip when dir<0 AND
  throttle>=17 → dir=+1; `flags&0x100` overrides to dir=-1. inputMul: reverse+flag → maxInput,
  reverse+no-flag → 0; forward → `RTZ((l11-l12)*maxInput/256)` or `RTZ(l11*maxInput/256)`.
  Calls `FUN_8002ee94`. Post-call: flipped → `self+0x98 += (lut[0x10]-lut[0x14])*256`;
  upright-steer-proximity → `steer*2 ± drag`; normal → speed*sub06 formula.

- **modes 4/5** (player-aim): lut[0x12] adjusts `player+0x8e` (aim angle); lut[0x13]
  adjusts `player+0x94` (player dragMass) clamped to `[2*maxSpeed, 0x140000]`; lut[0x10]
  sets `self+0xa4 = delta²/24`. inputMul = `(0x80-throttle)*maxInput/128`.

Stubs added to platform/panic_stubs.c:
- `FUN_8002ee94` (wheel-force dispatch)
- `FUN_8002c59c` (enemy/boss variant)
- `FUN_8002d054` (catchup step)

Build: **PASS**. Selftest: **PASS** (RNG, Vec3, ratan2, SquareRoot0).

---

## Next Function in Dependency Chain

| Address | Name (working) | Notes |
|---|---|---|
| FUN_8002ee94 | Vehicle_WheelForceDispatch | 150–200 instr est. Called from Vehicle_DamagedPhysics + others. Currently stub. |
| FUN_8002f9bc | Wheel_PhysicsBody | 1069 instr. Pass-3; very large. Currently panic stub. |
| FUN_8002c59c | Vehicle_EnemyInputDispatch | Enemy variant of FUN_8002ee94. |

Recommend FUN_8002ee94 next as it is the most-called unimplemented helper in the physics chain.
MIPS: extract from EXE at vaddr 0x8002ee94 if not yet in analysis/SLUS_005.10/mips/.

---

## Build Instructions

```
cd "C:/Programming/GitHub/Vigilante 8 recomp"
cmake --build build --config Debug
```
Test:
```
./build/Debug/v8.exe --selftest
```
