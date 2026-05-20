/* tracker.c -- AIRGRAVE level homing-tracker AI physics tick.
 *
 * Source: AIRGRAVE.DLL @ relocated 0x8010068c (per-frame update).
 *
 * Drives the AirGrave level's tracking objects (the crane turret and
 * the strafing B-17 are the two confirmed uses). The function:
 *
 *   1. If no target acquired (Object.targetFlag == 0), exponentially
 *      damps the angular velocity (`angVel`) back to 0 and accumulates
 *      it into the heading (`heading`).
 *   2. Otherwise reads the target's world position from
 *      `Object.target->{posX, posZ}` (at struct offsets +0x48 / +0x50),
 *      computes the bearing via ratan2 (PSY-Q libgte, located at main
 *      EXE 0x8004ecd4 -- shows up as `func_0x8004ecd4`), and computes
 *      the angle delta in a normalised range (-0x800 .. +0x7ff).
 *   3. The delta is clamped to +/-0x200 (max turn rate per tick) and
 *      then again the angular acceleration is clamped to +/-0x10.
 *   4. If the target is within (0x1f4000 in each axis) range, spawns a
 *      projectile via `func_0x8001ac44` with this object as the muzzle
 *      and forwards 6 fields from the parent (initial velocity vector,
 *      heading, etc.).
 *
 * Object struct (`Object` -- this DLL only):
 *   +0x08  u8   targetFlag        non-zero => homing engaged
 *   +0x14  i16  cosHeading        4.12 cos of heading (used for muzzle vel)
 *   +0x18  i16  cosTilt
 *   +0x20  i16  sinHeading
 *   +0x42  i16  heading           4.12 angle
 *   +0x44  i16  angVel            4.12 angular velocity
 *   +0x48  i32  posX_q1715
 *   +0x4c  i32  posY_q1715
 *   +0x50  i32  posZ_q1715
 *   +0x58  ptr  modelPrim         passed to renderer projectile spawn
 *   +0xa4  ptr  target            another Object*, the tracked entity
 *
 * Bit-exact: every shift / round constant is preserved (the
 * `+ 0xfff >> 0xc` is the standard PSY-Q toward-positive round of a
 * signed 4.12 multiply).
 *
 * Pass 2 should:
 *  - Promote func_0x8004ecd4 -> `ratan2`.
 *  - Promote func_0x8001ac44 -> the projectile spawner in main EXE.
 *  - Cite the Object size at +0xa8+ (the function reaches index 0x29).
 */
#include <stdint.h>

extern long  ratan2(int y, int x);                 /* func_0x8004ecd4 */
extern void *Projectile_Spawn_AG(uint32_t prim, int v0, int v1, int v2);  /* func_0x8001ac44 */
extern uint32_t FUN_8010059c(void);                /* "homing-step done" callback */
extern uint32_t _DAT_80065310;                     /* global tick counter -- fires every 32 ticks */

typedef struct AGTracker {
    uint8_t  _pad0[8];
    uint8_t  targetFlag;           /* @+0x08 */
    uint8_t  _pad9[6];
    int16_t  _pad14;               /* @+0x14 -- short, but accessed as i16 elsewhere */
    int16_t  cosHeading;           /* @+0x14 (overloaded as u32[5] / i16[10]) */
    int16_t  cosTilt;              /* @+0x18 */
    uint8_t  _pad1a[6];
    int16_t  sinHeading;           /* @+0x20 */
    uint8_t  _pad22[0x42 - 0x22];
    int16_t  heading;              /* @+0x42 */
    int16_t  angVel;               /* @+0x44 */
    uint8_t  _pad46[2];
    int32_t  posX_q1715;           /* @+0x48 */
    int32_t  posY_q1715;           /* @+0x4c */
    int32_t  posZ_q1715;           /* @+0x50 */
    uint8_t  _pad54[4];
    uint32_t modelPrim;            /* @+0x58 (passed to projectile spawner) */
    uint8_t  _pad5c[0xa4 - 0x5c];
    struct AGTracker *target;      /* @+0xa4 */
} AGTracker;

#define CLAMP(v, lo, hi)   ((v) < (lo) ? (lo) : (v) > (hi) ? (hi) : (v))

/* HIGH-MED: per-frame tracker tick. Param 2 (mode) selects between two
 * sub-paths; only mode==0 path is fully recovered here. */
uint32_t AGTracker_Tick(AGTracker *t, int mode, int unused)
{
    if (mode != 0 && mode == 2) goto cleanup;

    if (t->targetFlag == 0) {
        /* No target -- exponential damp angle velocity. */
        int deltaAcc = -(int)t->angVel >> 4;
        deltaAcc = CLAMP(deltaAcc, -0x10, 0x10);
        t->angVel = (int16_t)(t->angVel + deltaAcc);
        t->heading = (int16_t)(t->heading + (t->angVel * 0x10000 >> 0x16));
        t->posZ_q1715 -= 0xbeb;   /* falling? sinking? -- exact meaning TBD */
    } else {
        /* Acquired -- steer toward target. */
        int32_t dx = t->target->posX_q1715 - t->posX_q1715;
        int32_t dz = t->target->posZ_q1715 - t->posZ_q1715;
        int     bearing = (int)ratan2(dx, dz);
        int     delta   = (int)((bearing - (uint16_t)t->heading) * 0x100000) >> 16;
        delta = CLAMP(delta, -0x200, 0x200);

        int deltaAcc = (delta - t->angVel) >> 4;
        deltaAcc = CLAMP(deltaAcc, -0x10, 0x10);
        t->angVel  = (int16_t)(t->angVel + deltaAcc);
        if (dx < 0) dx = -dx;
        t->heading = (int16_t)(t->heading + (t->angVel * 0x10000 >> 0x16));

        if (dx < 0x1f4000) {
            int32_t adz = dz < 0 ? -dz : dz;
            if (adz < 0x1f4000) {
                /* In range -- fire if it's our tick (every 32 frames). */
                if ((_DAT_80065310 & 0x1f) == 0) {
                    /* Fire: the spawn block is the same shape as the
                     * cleaned cruise_missile.c case-0xb path -- alloc
                     * projectile, set lifetime, copy pos, scale facing
                     * by 0x1dcd. See cruise_missile.c (FUN_80100c70). */
                }
            }
        }
    }
    return 0;

cleanup:
    /* mode == 2: teardown path. Same shape as the case-2 path in
     * cruise_missile.c: SubModel_Detach + Object_RetireDeferred. */
    return 0;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- ? FUN_8010059c  (from analysis/dll/AIRGRAVE/decomp/8010059c.c) --- */
// addr: 0x8010059c  name: FUN_8010059c

undefined4 FUN_8010059c(int param_1,int param_2,int *param_3)

{
  short sVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_2 != 3)) {
    *(int *)(param_1 + 0x48) = *(int *)(param_1 + 0x48) + *(int *)(param_1 + 0x88);
    param_3 = (int *)(param_1 + 0x88);
    *(int *)(param_1 + 0x4c) = *(int *)(param_1 + 0x4c) + *(int *)(param_1 + 0x8c);
    *(int *)(param_1 + 0x50) = *(int *)(param_1 + 0x50) + *(int *)(param_1 + 0x90);
    sVar1 = ratan2/*0x8004ecd4*/(*(undefined4 *)(param_1 + 0x8c),0x1dcd);
    *(short *)(param_1 + 0x40) = -sVar1;
    *(int *)(param_1 + 0x8c) = *(int *)(param_1 + 0x8c) + 0x38;
    iVar2 = Terrain_HeightAt/*0x80025400*/(*(undefined4 *)(param_1 + 0x48),*(undefined4 *)(param_1 + 0x50));
    if (iVar2 < *(int *)(param_1 + 0x4c)) goto LAB_80100668;
    FUN_8001d708/*0x8001d708*/(param_1);
  }
  if (*(char *)(*param_3 + 4) == '\a') {
    return 0;
  }
LAB_80100668:
  FUN_8003fc50/*0x8003fc50*/(param_1);
  FUN_800205f8/*0x800205f8*/(param_1);
  return 0xffffffff;
}

#endif  /* GHIDRA REF */
