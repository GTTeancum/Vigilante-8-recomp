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
 * Mode 0 and mode 2 are recovered from the source control flow. The
 * callback intentionally falls through from mode 0 into the mode-2 reacquire
 * block when the third argument is non-zero, matching the overlay code.
 */
#include <stdint.h>

extern long  ratan2(int y, int x);                 /* func_0x8004ecd4 */
extern void *Projectile_Spawn_AG(uint32_t prim, int v0, int v1, int v2);  /* func_0x8001ac44 */
extern uint32_t FUN_8010059c(void);                /* "homing-step done" callback */
extern uint32_t _DAT_80065310;                     /* global tick counter -- fires every 32 ticks */
extern uint32_t _DAT_80065ad4;
extern uint32_t _DAT_80065ad8;
extern void Object_Suspend(void *self);            /* FUN_8002036c */
extern void Object_RefitAABB(void *self);          /* FUN_8001d708 */
extern void Object_ClearBackBufferFlag(uint32_t *self); /* FUN_80020778 */
extern void FUN_80020744(uint32_t *self);          /* Object_RegisterPostUpdate */
extern void Damage_Apply_AgainstSelf(void *self, int ticks); /* FUN_80020890 */
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
extern int  Terrain_HeightAt(uint32_t x, uint32_t z); /* FUN_80025400 */
extern int  FUN_8001fd08(uint32_t x, uint32_t z);  /* terrain/object probe */
extern uint8_t SFX_PlayWorld(int32_t *posXyz);     /* FUN_800449bc */
extern void SFX_Update(int handle, int posVoxel);  /* FUN_80044574 */
extern void SFX_StopWorld(int handle);             /* FUN_800441c8 */
extern uint8_t Pool_AllocProjectile(void);         /* FUN_8004410c */
extern void Pool_BindFXOnObject(int h, uint32_t bin, int slot, int aux); /* FUN_800443c8 */

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

static int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static int32_t mips_sll_i32(int32_t v, unsigned sh)
{
    return (int32_t)((uint32_t)v << sh);
}

static int32_t rtz12(int32_t v)
{
    if (v < 0) v = mips_addu_i32(v, 0xfff);
    return v >> 12;
}

static int32_t scale_sin_1dcd(int16_t v)
{
    int32_t t = mips_sll_i32(v, 2);
    t = mips_addu_i32(t, v);
    t = mips_sll_i32(t, 5);
    t = mips_subu_i32(t, v);
    t = mips_sll_i32(t, 4);
    t = mips_subu_i32(t, v);
    int32_t t4 = mips_sll_i32(t, 2);
    return rtz12(mips_subu_i32(t4, t));
}

static int32_t mips_abs_i32(int32_t v)
{
    return (v < 0) ? mips_subu_i32(0, v) : v;
}

/* HIGH-MED: per-frame tracker tick and mode-2 reacquire/reset path. */
uint32_t AGTracker_Tick(AGTracker *t, int mode, int unused)
{
    uint32_t *self = (uint32_t *)t;
    int32_t delta = 0;

    if (mode != 0) {
        if (mode == 2) goto mode2;
        return 0;
    }

    if ((int8_t)self[2] != 0) {
        uint32_t target = self[0x29];
        int32_t dx = mips_subu_i32(*(int32_t *)(uintptr_t)(target + 0x48), (int32_t)self[0x12]);
        int32_t dz = mips_subu_i32(*(int32_t *)(uintptr_t)(target + 0x50), (int32_t)self[0x14]);
        delta = mips_sll_i32(mips_subu_i32((int32_t)ratan2(dx, dz),
                                           *(uint16_t *)((uint8_t *)self + 0x42)), 20) >> 16;
        int32_t turn = CLAMP(delta, -0x200, 0x200);
        int32_t acc = mips_subu_i32(turn, *(int16_t *)((uint8_t *)self + 0x44)) >> 4;
        acc = CLAMP(acc, -0x10, 0x10);
        int32_t angVel = mips_addu_i32(*(uint16_t *)((uint8_t *)self + 0x44), acc);
        *(int16_t *)((uint8_t *)self + 0x44) = (int16_t)angVel;
        *(int16_t *)((uint8_t *)self + 0x42) =
            (int16_t)mips_addu_i32(*(uint16_t *)((uint8_t *)self + 0x42),
                                   mips_sll_i32(angVel, 16) >> 22);

        if (mips_abs_i32(dx) < 0x1f4000) {
            if (mips_abs_i32(dz) < 0x1f4000) {
                if ((_DAT_80065310 & 0x1f) == 0) {
                    uint32_t *p = (uint32_t *)Projectile_Spawn_AG(self[0x16], 0xdb, 0x98, 0);
                    *(uint8_t *)(p + 1) = 7;
                    *p |= 0x80;
                    Object_SetCallbackPsxSlot(p, (uintptr_t)&FUN_8010059c);
                    *(uint16_t *)(p + 3) = 100;
                    *(uint16_t *)((uint8_t *)p + 6) = *(uint16_t *)((uint8_t *)self + 6);
                    p[0x12] = self[0x12];
                    p[0x13] = self[0x13];
                    p[0x14] = self[0x14];
                    *(uint16_t *)((uint8_t *)p + 0x42) = *(uint16_t *)((uint8_t *)self + 0x42);
                    p[0x22] = (uint32_t)scale_sin_1dcd(*(int16_t *)((uint8_t *)self + 0x14));
                    p[0x24] = (uint32_t)scale_sin_1dcd(*(int16_t *)((uint8_t *)self + 0x20));
                    p[0x23] = 0;
                    Object_Suspend(p);
                }
                if ((int32_t)self[0x13] < mips_addu_i32(*(int32_t *)(uintptr_t)(target + 0x4c),
                                                        -0x23000)) {
                    self[0x13] = (uint32_t)mips_addu_i32((int32_t)self[0x13], 0x5f5);
                    goto post_target_height;
                }
            }
        }

        if (mips_addu_i32(*(int32_t *)(uintptr_t)(target + 0x4c), -0x64000) <
            (int32_t)self[0x13]) {
            self[0x13] = (uint32_t)mips_addu_i32((int32_t)self[0x13], -0xbeb);
        }

post_target_height:
        if (*(int16_t *)(uintptr_t)(target + 0xc) == 0) {
            *(uint8_t *)(self + 2) = 0;
            Damage_Apply_AgainstSelf(self, 600);
            goto no_target;
        }
        if (mips_abs_i32(delta) > 0x4000) {
            *(uint8_t *)(self + 2) = 0;
            Damage_Apply_AgainstSelf(self, 600);
            goto no_target;
        }
    } else {
no_target:
        {
            int32_t acc = mips_subu_i32(0, *(int16_t *)((uint8_t *)self + 0x44)) >> 4;
            acc = CLAMP(acc, -0x10, 0x10);
            int32_t angVel = mips_addu_i32(*(uint16_t *)((uint8_t *)self + 0x44), acc);
            *(int16_t *)((uint8_t *)self + 0x44) = (int16_t)angVel;
            *(int16_t *)((uint8_t *)self + 0x42) =
                (int16_t)mips_addu_i32(*(uint16_t *)((uint8_t *)self + 0x42),
                                       mips_sll_i32(angVel, 16) >> 22);
            self[0x13] = (uint32_t)mips_addu_i32((int32_t)self[0x13], -0xbeb);
        }
    }

    self[0x12] = (uint32_t)mips_addu_i32((int32_t)self[0x12],
                                         scale_sin_1dcd(*(int16_t *)((uint8_t *)self + 0x14)));
    self[0x14] = (uint32_t)mips_addu_i32((int32_t)self[0x14],
                                         scale_sin_1dcd(*(int16_t *)((uint8_t *)self + 0x20)));
    Object_RefitAABB(self);
    if (unused == 0) return 0;
    for (uint32_t child = self[0xe]; child != 0; child = *(uint32_t *)(uintptr_t)(child + 0x34)) {
        if (*(int16_t *)(uintptr_t)(child + 6) == 0) {
            *(int16_t *)(uintptr_t)(child + 0x44) =
                (int16_t)mips_addu_i32(*(int16_t *)(uintptr_t)(child + 0x44),
                                       mips_sll_i32(unused, 8));
            Object_RefitAABB((void *)(uintptr_t)child);
        }
    }
    {
        int sfxPos = SFX_PlayWorld((int32_t *)(self + 0x12));
        SFX_Update((int)*(int8_t *)((uint8_t *)self + 5), mips_sll_i32(sfxPos, 1));
    }

mode2:
    if ((*self & 0x80) != 0) {
        Object_ClearBackBufferFlag(self);
        SFX_StopWorld((int)*(int8_t *)((uint8_t *)self + 5));
        *(uint8_t *)((uint8_t *)self + 5) = 0;
        *self |= 0x22;
        *(uint16_t *)((uint8_t *)self + 0x42) ^= 0x800;
        self[0x13] = (uint32_t)mips_addu_i32(Terrain_HeightAt(self[0x12], self[0x14]), -0x64000);
    }
    {
        uint32_t target = _DAT_80065ad4;
        int hit = FUN_8001fd08(*(uint32_t *)(uintptr_t)(_DAT_80065ad4 + 0x24),
                               *(uint32_t *)(uintptr_t)(_DAT_80065ad4 + 0x2c));
        if ((hit == 0 || *(int16_t *)(uintptr_t)(hit + 10) != 0) && _DAT_80065ad8 != 0) {
            hit = FUN_8001fd08(*(uint32_t *)(uintptr_t)(_DAT_80065ad8 + 0x24),
                               *(uint32_t *)(uintptr_t)(_DAT_80065ad8 + 0x2c));
            if (hit != 0 && *(int16_t *)(uintptr_t)(hit + 10) == 0) {
                target = _DAT_80065ad8;
            } else {
                hit = 0;
            }
        }
        if (hit != 0) {
            int8_t state = (int8_t)mips_addu_i32((uint8_t)self[2], 1);
            *(uint8_t *)(self + 2) = (uint8_t)state;
            if (state != 1) {
                self[0x29] = target;
                *self &= 0xffffffddu;
                uint8_t h = Pool_AllocProjectile();
                *(uint8_t *)((uint8_t *)self + 5) = h;
                Pool_BindFXOnObject((int)(int8_t)h, *(uint32_t *)(uintptr_t)(self[0x16] + 8), 1, 0);
                FUN_80020744(self);
            }
        }
    }
    *(uint8_t *)(self + 2) = 0;
    Damage_Apply_AgainstSelf(self, 0x3c);
    return 0;
}

uint32_t FUN_8010068c(AGTracker *t, int mode, int unused)
{
    return AGTracker_Tick(t, mode, unused);
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
