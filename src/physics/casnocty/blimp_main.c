/* blimp_main.c -- Casino City blimp main flight/hostility tick.
 *
 * Source: CASNOCTY.DLL  FUN_80100c88.
 *
 * The casino blimp main object: cruises in a circle, drops payload,
 * descends on damage. Inner state byte (obj[2]) drives a 7-step
 * lifecycle:
 *   0  -- gentle arrive: home toward path waypoint at +0x23
 *   1  -- normal cruise: re-pick waypoint via Path_Pick + Path_Step
 *         (FUN_8001ffd4 / FUN_80024d30 / FUN_80042e78); orbit until
 *         yaw delta wraps negative.
 *   2  -- transitioning to descent
 *   3  -- attack: lock at +0x64000 above terrain, spawn payload
 *         (Pool_LaunchProjectile from _DAT_800658fc, type 0x14),
 *         arm 900-frame retreat timer at obj+0xaa
 *   4  -- retreat timer ticking; on -1 -> state 5 with bbox kill
 *   5  -- death drop: detach SubModel, retire after splash
 *   6  -- post-retire: clear FX sfx handle, free
 *
 * Outer event switch (param_2):
 *   0  -- per-tick
 *   1  -- post-spawn init: alloc path-follower, bind FX
 *   2  -- explicit retire (sets *self &= ~0x20)
 *   3  -- impact (param_3 = impactor)
 *   4  -- pause / partial reset
 *   6  -- cycle-through-init for re-entry
 *
 * Impact response:
 *   - target vehicle yaw close (<0x1000 diff) and state==3: blimp
 *     "catches" the player into a controlled-falling carrier slot via
 *     a 0x30-byte heap-alloced descriptor (positions + lift point).
 *     Plays a pilot-ejection sequence with FUN_801006d4 binding.
 *   - shrapnel (kind 7): receive damage, bounce, spawn payload.
 *
 * Constants:
 *   0x40000  -- spread radius for yaw-rotate
 *   0x113a0  -- payload spawn alt
 *   0xbeb    -- 0.7434 in 4.12 (drag coefficient)
 *   0x64000, 0x5000 -- hover/dive Y offset above terrain
 *   0x32000, 0xfa000 -- catch-attach + lift Y deltas
 *
 * MED. Mirrors the FALL-THROUGH inner switch from the original;
 * geometry math (clamped error * 0x100000 >> 0x14) is the casino-
 * scale "small-angle 12.20 fixed-point yaw error" idiom.
 */
#include <stdint.h>

extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
extern int      Terrain_HeightAt(int32_t x, int32_t z);
extern uint32_t Pool_AllocProjectile(void);
extern void     Pool_LaunchProjectile(uint32_t h, uint32_t bin, int kind, void *pos);
extern void     Damage_Apply_AgainstSelf(void *self, void *param);
extern int      Path_Pick(uint32_t base, int seed);                  /* FUN_8001ffd4 */
extern uint32_t Path_Step(uint32_t *pos, uint32_t target,
                          int speed, int flag);                       /* FUN_80024d30 */
extern void     Path_RotateToward(uint32_t *yawCell, uint32_t step);  /* FUN_80042e78 */
extern int      Path_DistToWaypoint(uint32_t *self, uint32_t *yaw,
                                    uint32_t radius);                 /* FUN_80042f98 */
extern void     Path_Cleanup(uint32_t *yawCell);                      /* FUN_80042f5c */
extern int      Rand255(void);                                        /* FUN_80017160 */
extern void     Object_RefitAABB(uint32_t *self);                     /* FUN_8001d708 */
extern void     SubModel_Detach(uint32_t *self);                      /* FUN_8003fc50 */
extern void     Object_RetireDeferred(uint32_t *self);                /* FUN_800205f8 */
extern void     SFX_StopWorld(int handle);                            /* FUN_800441c8 */
extern uint8_t  SFX_PlayWorld(uint32_t *posXyz);                      /* FUN_800449bc */
extern void     SFX_Update(int handle, int posVoxel);                 /* FUN_80044574 */
extern int      Damage_VsImpactor(void *self, int kind);              /* func_0x80022320 */
extern void     FX_RingFlash_Init(uint32_t *self, uint32_t *imp);     /* FUN_8001f5a0 */
extern int      Math_Atan2_Pos(uint32_t *xy);                         /* FUN_80016c88 */
extern int      Spawner_FindSlot(int mode);                           /* func_0x8001fd9c */
extern void     Spawner_Promote(uint32_t *target);                    /* func_0x8003dbb0 */
extern uint8_t  Pool_AllocProjectile_NoArg(void);                     /* FUN_8004410c */
extern void     Pool_BindFXOnObject(int h, uint32_t bin, int slot, int aux); /* FUN_800443c8 */
extern void    *Heap_AllocOrRetry(uint32_t n);                        /* FUN_800116f4 */
extern uint32_t _DAT_80065310;
extern uint32_t _DAT_800658fc;
extern uint8_t  DAT_80065a50;
extern void     FUN_801006d4(uint32_t *self, int mode);  /* blimp_pilot.c */

static int32_t clamp_pm(int32_t v, int32_t lim) {
    return v < -lim ? -lim : v > lim ? lim : v;
}

static int32_t mips_addu_i32(int32_t a, int32_t b) {
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static int32_t mips_subu_i32(int32_t a, int32_t b) {
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static int32_t mips_sll_i32(int32_t v, unsigned sh) {
    return (int32_t)((uint32_t)v << sh);
}

static int32_t mips_mult_lo_i32(int32_t a, int32_t b) {
    return (int32_t)((uint64_t)(uint32_t)a * (uint64_t)(uint32_t)b);
}

static int32_t rtz_shift_i32(int32_t v, unsigned sh, int32_t bias) {
    if (v < 0) v = mips_addu_i32(v, bias);
    return v >> sh;
}

static int32_t mips_abs_i32(int32_t v) {
    return v < 0 ? mips_subu_i32(0, v) : v;
}

uint32_t CC_BlimpMain(uint32_t *self, uint32_t mode, uint32_t *imp)
{
    switch (mode) {
    case 0:
        /* Integrate. */
        self[0x12] = (uint32_t)mips_addu_i32((int32_t)self[0x12], (int32_t)self[0x20]);
        self[0x13] = (uint32_t)mips_addu_i32((int32_t)self[0x13], (int32_t)self[0x21]);
        self[0x14] = (uint32_t)mips_addu_i32((int32_t)self[0x14], (int32_t)self[0x22]);
        switch ((int)(((uint8_t)self[2] + 1) * 0x1000000) >> 24) {
        case 1: {
            int seed = Rand255();
            uint32_t path = Path_Pick(0x80065a50, ((seed << 3) >> 15) - 0x20);
            self[0x23] = path;
            uint32_t step = Path_Step(self + 9, path + 0x48, 0x113a0, 0);
            Path_RotateToward(self + 0x25, step);
            *(uint8_t *)(self + 2) = 1;
        } /* fallthrough */
        case 2: {
            if (((_DAT_80065310 - (uint32_t)*((uint8_t *)self + 9)) & 0xff) == 0) {
                uint32_t step = Path_Step(self + 9, self[0x23] + 0x48, 0x113a0, 0);
                Path_RotateToward(self + 0x25, step);
            }
            if ((int16_t)self[0x25] == 0) {
                self[0x27] = *(uint32_t *)(self[0x23] + 0x48);
                self[0x28] = *(uint32_t *)(self[0x23] + 0x50);
            }
            int yawErr = Path_DistToWaypoint(self, self + 0x25, 0x40000);
            if ((int16_t)self[0x25] < 0) *(uint8_t *)(self + 2) = 2;
            int dy = mips_sll_i32(mips_subu_i32(yawErr, (uint16_t)self[0x2a]), 20) >> 20;
            dy = clamp_pm(dy, 8);
            int newY = mips_addu_i32((uint16_t)self[0x2a], dy);
            int sm  = mips_sll_i32(newY, 16) >> 16;
            *(int16_t *)(self + 0x2a) = (int16_t)newY;
            if (sm < 0) sm += 0x3f;
            *(int16_t *)((char *)self + 0x42) =
                (int16_t)mips_addu_i32(*(int16_t *)((char *)self + 0x42), sm >> 6);
        } /* fall into case 0 */
        case 0:
            if ((char)self[2] < 0 && (
                (((int32_t)self[0x20] >= 0 ||
                  *(int16_t *)((char *)self + 0x4a) < *(int16_t *)self[0x29]) &&
                 (int)*(int16_t *)self[0x29] + (int)((int16_t *)self[0x29])[2]
                     < (int)*(int16_t *)((char *)self + 0x4a)) ||
                (((int32_t)self[0x22] >= 0 ||
                  *(int16_t *)((char *)self + 0x52) < *(int16_t *)(self[0x29] + 2)) &&
                 (int)*(int16_t *)(self[0x29] + 2) + (int)*(int16_t *)(self[0x29] + 6)
                     < (int)*(int16_t *)((char *)self + 0x52))))
            {
                *(int16_t *)((char *)self + 0x42) =
                    (int16_t)mips_addu_i32(*(int16_t *)((char *)self + 0x42), 0x10);
                self[0x20] = (uint32_t)mips_subu_i32(0, (int32_t)self[0x20]);
                self[0x22] = (uint32_t)mips_subu_i32(0, (int32_t)self[0x22]);
            }
            Object_RefitAABB(self);
            /* Drag toward facing direction. */
            int32_t drag5 = mips_mult_lo_i32((int16_t)self[5], 0xbeb);
            int32_t adj   = mips_subu_i32(rtz_shift_i32(drag5, 12, 0xfff),
                                          (int32_t)self[0x20]);
            self[0x20] = (uint32_t)mips_addu_i32((int32_t)self[0x20],
                                                 clamp_pm(rtz_shift_i32(adj, 4, 0xf), 0x40));
            int32_t drag8 = mips_mult_lo_i32((int16_t)self[8], 0xbeb);
            adj = mips_subu_i32(rtz_shift_i32(drag8, 12, 0xfff), (int32_t)self[0x22]);
            self[0x22] = (uint32_t)mips_addu_i32((int32_t)self[0x22],
                                                 clamp_pm(rtz_shift_i32(adj, 4, 0xf), 0x40));
            /* Vertical hover. */
            int ty = mips_subu_i32(Terrain_HeightAt(self[0x12], self[0x14]),
                                   mips_addu_i32((int32_t)self[0x13], 0x64000));
            ty = rtz_shift_i32(ty, 4, 0xf);
            if (ty < -0x2fa) ty = -0x2fa;
            int dvy = mips_subu_i32(ty, (int32_t)self[0x21]);
            self[0x21] = (uint32_t)mips_addu_i32((int32_t)self[0x21], clamp_pm(dvy, 0x40));
            /* fall through to case 3 */
        case 3: {
            int ty = mips_subu_i32(Terrain_HeightAt(self[0x12], self[0x14]),
                                   mips_addu_i32((int32_t)self[0x13], 0x5000));
            int yawErr = mips_sll_i32(mips_subu_i32((uint16_t)*(uint16_t *)(self[0x23] + 0x42),
                                                    (uint16_t)*(uint16_t *)((char *)self + 0x42)),
                                      20) >> 20;
            int dy = mips_subu_i32(yawErr, (int16_t)self[0x2a]);
            dy = clamp_pm(dy, 8);
            int newY = mips_addu_i32((uint16_t)self[0x2a], dy);
            *(int16_t *)(self + 0x2a) = (int16_t)newY;
            int sm = mips_sll_i32(newY, 16) >> 16;
            if (sm < 0) sm += 0x3f;
            *(int16_t *)((char *)self + 0x42) =
                (int16_t)mips_addu_i32(*(int16_t *)((char *)self + 0x42), sm >> 6);
            int32_t adj = rtz_shift_i32(mips_subu_i32(0, (int32_t)self[0x20]), 4, 0xf);
            self[0x20] = (uint32_t)mips_addu_i32((int32_t)self[0x20], clamp_pm(adj, 0x40));
            adj = rtz_shift_i32(mips_subu_i32(0, (int32_t)self[0x22]), 4, 0xf);
            self[0x22] = (uint32_t)mips_addu_i32((int32_t)self[0x22], clamp_pm(adj, 0x40));
            int dvy = rtz_shift_i32(ty, 4, 0xf);
            if (dvy > 0x2fa) dvy = 0x2fa;
            dvy = mips_subu_i32(dvy, (int32_t)self[0x21]);
            self[0x21] = (uint32_t)mips_addu_i32((int32_t)self[0x21], clamp_pm(dvy, 0x40));
            Object_RefitAABB(self);
            if (ty < 0x199 && (int16_t)self[0x2a] < 0x40) {
                *(uint8_t *)(self + 2) = 3;
                *(int16_t *)((char *)self + 0xaa) = 900;
                uint32_t h = Pool_AllocProjectile();
                Pool_LaunchProjectile(h, _DAT_800658fc, 0x14, self + 0x12);
            }
        } /* fall through */
        case 4: {
            int16_t t = (int16_t)mips_addu_i32(*(int16_t *)((char *)self + 0xaa), -1);
            *(int16_t *)((char *)self + 0xaa) = t;
            if (t == -1) {
                *(int16_t *)((char *)self + 0xaa) = 0;
                *(uint8_t *)(self + 2) = 0;
            } else break;
        } /* fall through to case 5 */
        case 5:
            /* Death descent dynamics. */
            if ((int32_t)self[0x20] < 0) {
                if (*(int16_t *)((char *)self + 0x4a) < *(int16_t *)self[0x29]) {
                    self[0x22] = 0;
                    goto blimp_death_check_x_hi;
                }
            } else {
blimp_death_check_x_hi:
                if ((int)*(int16_t *)((char *)self + 0x4a) >
                    (int)*(int16_t *)self[0x29] + (int)((int16_t *)self[0x29])[2]) {
                    self[0x22] = 0;
                    self[0x20] = 0;
                    goto blimp_death_after_bounds;
                }
            }
            if ((int32_t)self[0x22] < 0) {
                if (*(int16_t *)((char *)self + 0x52) < *(int16_t *)(self[0x29] + 2)) {
                    self[0x22] = 0;
                    goto blimp_death_check_z_hi;
                }
            } else {
blimp_death_check_z_hi:
                if ((int)*(int16_t *)(self[0x29] + 2) + (int)*(int16_t *)(self[0x29] + 6) <
                    (int)*(int16_t *)((char *)self + 0x52)) {
                    self[0x22] = 0;
                    self[0x20] = 0;
                }
            }
blimp_death_after_bounds:
            if (-0x200 < (int16_t)self[0x10]) {
                *(int16_t *)(self + 0x10) =
                    (int16_t)mips_addu_i32((int16_t)self[0x10], -4);
            }
            Object_RefitAABB(self);
            { int32_t a = mips_mult_lo_i32((int16_t)self[5], 0xbeb);
              int32_t d=mips_subu_i32(rtz_shift_i32(a, 12, 0xfff), (int32_t)self[0x20]);
              self[0x20] = (uint32_t)mips_addu_i32((int32_t)self[0x20],
                                                   clamp_pm(rtz_shift_i32(d, 4, 0xf), 0x40));
              a = mips_mult_lo_i32(*(int16_t *)((char *)self+0x1a), 0xbeb);
              d=mips_subu_i32(rtz_shift_i32(a, 12, 0xfff), (int32_t)self[0x21]);
              self[0x21] = (uint32_t)mips_addu_i32((int32_t)self[0x21],
                                                   clamp_pm(rtz_shift_i32(d, 4, 0xf), 0x40));
              a = mips_mult_lo_i32((int16_t)self[8], 0xbeb);
              d=mips_subu_i32(rtz_shift_i32(a, 12, 0xfff), (int32_t)self[0x22]);
              self[0x22] = (uint32_t)mips_addu_i32((int32_t)self[0x22],
                                                   clamp_pm(rtz_shift_i32(d, 4, 0xf), 0x40)); }
            { int ty = Terrain_HeightAt(self[0x12], self[0x14]);
              if (ty < (int)self[0x13] && (*self & 0x8000) == 0) {
                  SubModel_Detach(self);
                  *(uint8_t *)(self + 2) = 5;
                  /* fall to case 6 */
              } else break; }
            /* fall through */
        case 6:
            if ((*self & 0x8000) == 0) {
                SFX_StopWorld((int)*((char *)self + 5));
                *((char *)self + 5) = 0;
                Object_RetireDeferred(self);
            }
            break;
        default: break;
        }
        /* Spin sub-children. */
        if (imp == NULL) return 0;
        imp = (uint32_t *)((intptr_t)imp * 0x100);
        for (uintptr_t c = self[0xe]; c != 0; c = *(uint32_t *)(c + 0x34)) {
            int v = 1;
            if (*(int16_t *)(c + 6) == 0) {
                v = mips_addu_i32((uint16_t)*(uint16_t *)(c + 0x44), (int32_t)(intptr_t)imp);
                *(int16_t *)(c + 0x44) = (int16_t)v;
            }
            if (*(int16_t *)(c + 6) == v) {
                if ((char)self[2] == 3) {
                    if (0x354 < *(int16_t *)(c + 0x40)) continue;
                    *(int16_t *)(c + 0x40) =
                        (int16_t)mips_addu_i32(*(int16_t *)(c + 0x40), 0x20);
                }
                if (*(int16_t *)(c + 0x40) != 0) {
                    *(int16_t *)(c + 0x40) =
                        (int16_t)mips_addu_i32(*(int16_t *)(c + 0x40), -0x20);
                    Object_RefitAABB((uint32_t *)c);
                }
            }
        }
        SFX_Update((int)*((char *)self + 5), SFX_PlayWorld(self + 0x12));
        return 0;

    case 1: {                        /* post-spawn init */
        *self = 0x84u;
        int slot = Spawner_FindSlot(0);
        self[0x29] = (uint32_t)(slot + 0xc);
        char sfx = (char)Pool_AllocProjectile_NoArg();
        *((char *)self + 5) = sfx;
        Pool_BindFXOnObject(sfx, *(uint32_t *)(self[0x16] + 8), 4, 0);
    }
    /* fall through */
    case 4:
        Path_Cleanup(self + 0x25);
        SFX_StopWorld((int)*((char *)self + 5));
        return 0;
    case 2:
        *self &= ~0x20u;
        /* fall to case 3 */
    case 3: {
        uint32_t *impact = imp;
        uint32_t *p = (uint32_t *)(uintptr_t)*imp;
        uint32_t *sourceVec = impact;
        int blimpState = (int)(char)self[2];
        if ((char)p[1] == 2 && *(int16_t *)((char *)p + 6) < 0) {
            if (blimpState == 3) {
                int yaw = Math_Atan2_Pos(p + 4);
                int d   = mips_sll_i32(mips_subu_i32(yaw,
                                                      (uint16_t)*((uint16_t *)((char *)self + 0x42))),
                                       20) >> 16;
                d = mips_abs_i32(d);
                if (d < 0x1000) {
                    uint32_t *bin = (uint32_t *)(uintptr_t)p[0x38];
                    self[0x24] = (uint32_t)(uintptr_t)p;
                    Object_SetCallbackPsxSlot(p, (uintptr_t)&FUN_801006d4);
                    p[0x1e] = (uint32_t)(uintptr_t)self;
                    *p = (*p & 0xfffffff7u) | 0x1000020u;
                    p[0x20] = (uint32_t)mips_sll_i32(mips_subu_i32((int32_t)self[0x12],
                                                                    (int32_t)p[9]), 2);
                    p[0x21] = (uint32_t)mips_sll_i32(mips_subu_i32((int32_t)self[0x13],
                                                                    (int32_t)p[10]), 2);
                    p[0x22] = (uint32_t)mips_sll_i32(mips_subu_i32((int32_t)self[0x14],
                                                                    (int32_t)p[0xb]), 2);
                    *(int16_t *)(p + 0x29) = 0;
                    SFX_Update((int)*((char *)p + 5), 0);
                    Damage_Apply_AgainstSelf(p, (void *)(intptr_t)0x20);
                    if (bin) {
                        uint32_t *ctx = (uint32_t *)Heap_AllocOrRetry(0x30);
                        ctx[0] = bin[0x12]; ctx[1] = bin[0x13]; ctx[2] = bin[0x14];
                        ctx[3] = 0x78;
                        ctx[4] = (uint32_t)mips_subu_i32((int32_t)self[9],
                                                         mips_mult_lo_i32((int16_t)self[5], 100));
                        ctx[5] = (uint32_t)mips_addu_i32((int32_t)self[10], -0x32000);
                        ctx[6] = (uint32_t)mips_subu_i32((int32_t)self[0xb],
                                                         mips_mult_lo_i32((int16_t)self[8], 100));
                        ctx[7] = 0;
                        Spawner_Promote(bin);
                        bin[0x25] = 0xfa000u;
                        *(int16_t *)((char *)self + 0xaa) = 0;
                        *(uint8_t *)(self + 2) = 0;
                        sourceVec = bin;
                    }
                    Pool_AllocProjectile_NoArg();
                }
            }
        } else {
            blimpState = (int)(char)self[2];
        }
        if (blimpState < 4) {
            uint8_t k = (uint8_t)p[1];
            if (k == 7) {
                if (Damage_VsImpactor(self, (int)(int16_t)p[3]) == 0) return 0;
                *(uint8_t *)(self + 2) = 4;
            }
            if (k == 3) return 0;
            FX_RingFlash_Init(self, sourceVec);
            int32_t vdot = mips_addu_i32(
                mips_addu_i32(mips_mult_lo_i32((int32_t)self[0x20], (int16_t)sourceVec[8]),
                              mips_mult_lo_i32((int32_t)self[0x21],
                                               *(int16_t *)((char *)sourceVec + 0x22))),
                mips_mult_lo_i32((int32_t)self[0x22], (int16_t)sourceVec[9]));
            vdot = rtz_shift_i32(vdot, 11, 0x7ff);
            if (vdot >= 0) return 0;
            int32_t bx = rtz_shift_i32(mips_mult_lo_i32(vdot, (int16_t)sourceVec[8]), 12, 0xfff);
            int32_t by = rtz_shift_i32(mips_mult_lo_i32(vdot, *(int16_t *)((char *)sourceVec + 0x22)), 12, 0xfff);
            int32_t bz = rtz_shift_i32(mips_mult_lo_i32(vdot, (int16_t)sourceVec[9]), 12, 0xfff);
            self[0x20] = (uint32_t)mips_subu_i32((int32_t)self[0x20], bx);
            self[0x21] = (uint32_t)mips_subu_i32((int32_t)self[0x21], by);
            self[0x22] = (uint32_t)mips_subu_i32((int32_t)self[0x22], bz);
            uint32_t h = Pool_AllocProjectile();
            Pool_LaunchProjectile(h, _DAT_800658fc, 7, self + 0x12);
            return 0;
        }
        if (blimpState >= 5) return 0;
        if ((uint8_t)p[1] != 0) return 0;
        if (*self & 0x8000) return 0;
        SubModel_Detach(self);
        *(uint8_t *)(self + 2) = 5;
        /* fall to case 6 */
    }
    /* fall through */
    case 6:
        /* Reuse post-spawn init for re-entry. */
        *self = 0x84u;
        return 0;
    }
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

/* --- CASNOCTY.DLL FUN_80100c88  (from analysis/dll/CASNOCTY/decomp/80100c88.c) --- */
// addr: 0x80100c88  name: FUN_80100c88

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100c88(uint *param_1,undefined4 param_2,uint *param_3)

{
  char cVar1;
  short sVar2;
  uint *puVar3;
  int iVar4;
  undefined4 uVar5;
  int iVar6;
  int iVar7;
  int extraout_v1;
  uint uVar8;
  uint uVar9;
  uint *puVar10;
  undefined8 uVar11;
  
  switch(param_2) {
  case 0:
    param_1[0x12] = param_1[0x12] + param_1[0x20];
    param_1[0x13] = param_1[0x13] + param_1[0x21];
    param_1[0x14] = param_1[0x14] + param_1[0x22];
    switch((int)(((byte)param_1[2] + 1) * 0x1000000) >> 0x18) {
    case 0:
      goto switchD_80100d3c_caseD_0;
    case 1:
      iVar6 = FUN_80017160/*0x80017160*/();
      uVar8 = FUN_8001ffd4/*0x8001ffd4*/(0x80065a50,((iVar6 << 3) >> 0xf) + -0x20);
      param_1[0x23] = uVar8;
      uVar5 = FUN_80024d30/*0x80024d30*/(param_1 + 9,uVar8 + 0x48,0x113a0,0);
      FUN_80042e78/*0x80042e78*/(param_1 + 0x25,uVar5);
      *(undefined1 *)(param_1 + 2) = 1;
      break;
    case 2:
      break;
    case 3:
      goto switchD_80100d3c_caseD_3;
    case 4:
      goto switchD_80100d3c_caseD_4;
    case 5:
      goto switchD_80100d3c_caseD_5;
    case 6:
      goto switchD_80100d3c_caseD_6;
    default:
      goto switchD_80100d3c_default;
    }
    if ((_DAT_80065310 - (uint)*(byte *)((int)param_1 + 9) & 0xff) == 0) {
      uVar5 = FUN_80024d30/*0x80024d30*/(param_1 + 9,param_1[0x23] + 0x48,0x113a0,0);
      FUN_80042e78/*0x80042e78*/(param_1 + 0x25,uVar5);
    }
    if ((short)param_1[0x25] == 0) {
      param_1[0x27] = *(uint *)(param_1[0x23] + 0x48);
      param_1[0x28] = *(uint *)(param_1[0x23] + 0x50);
    }
    iVar6 = FUN_80042f98/*0x80042f98*/(param_1,param_1 + 0x25,0x40000);
    if ((short)param_1[0x25] < 0) {
      *(undefined1 *)(param_1 + 2) = 2;
    }
    iVar4 = (int)((iVar6 - (uint)(ushort)param_1[0x2a]) * 0x100000) >> 0x14;
    iVar6 = -8;
    if ((-9 < iVar4) && (iVar6 = 8, iVar4 < 9)) {
      iVar6 = iVar4;
    }
    iVar6 = (uint)(ushort)param_1[0x2a] + iVar6;
    iVar4 = iVar6 * 0x10000 >> 0x10;
    *(short *)(param_1 + 0x2a) = (short)iVar6;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0x3f;
    }
    *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + (short)(iVar4 >> 6);
switchD_80100d3c_caseD_0:
    if (((char)param_1[2] < '\0') &&
       ((((-1 < (int)param_1[0x20] || (*(short *)((int)param_1 + 0x4a) < *(short *)param_1[0x29]))
         && ((int)*(short *)param_1[0x29] + (int)((short *)param_1[0x29])[2] <
             (int)*(short *)((int)param_1 + 0x4a))) ||
        (((-1 < (int)param_1[0x22] ||
          (*(short *)((int)param_1 + 0x52) < *(short *)(param_1[0x29] + 2))) &&
         ((int)*(short *)(param_1[0x29] + 2) + (int)*(short *)(param_1[0x29] + 6) <
          (int)*(short *)((int)param_1 + 0x52))))))) {
      *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + 0x10;
      param_1[0x20] = -param_1[0x20];
      param_1[0x22] = -param_1[0x22];
    }
    FUN_8001d708/*0x8001d708*/(param_1);
    iVar6 = (short)param_1[5] * 0xbeb;
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xfff;
    }
    iVar6 = (iVar6 >> 0xc) - param_1[0x20];
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xf;
    }
    iVar6 = iVar6 >> 4;
    iVar4 = -0x40;
    if ((-0x41 < iVar6) && (iVar4 = 0x40, iVar6 < 0x41)) {
      iVar4 = iVar6;
    }
    iVar6 = (short)param_1[8] * 0xbeb;
    param_1[0x20] = param_1[0x20] + iVar4;
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xfff;
    }
    iVar6 = (iVar6 >> 0xc) - param_1[0x22];
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xf;
    }
    iVar6 = iVar6 >> 4;
    iVar4 = -0x40;
    if ((-0x41 < iVar6) && (iVar4 = 0x40, iVar6 < 0x41)) {
      iVar4 = iVar6;
    }
    param_1[0x22] = param_1[0x22] + iVar4;
    iVar6 = Terrain_HeightAt/*0x80025400*/(param_1[0x12],param_1[0x14]);
    iVar6 = iVar6 - (param_1[0x13] + 0x64000);
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xf;
    }
    iVar4 = -0x2fa;
    if (-0x2fa < iVar6 >> 4) {
      iVar4 = iVar6 >> 4;
    }
    iVar4 = iVar4 - param_1[0x21];
    iVar6 = -0x40;
    if ((-0x41 < iVar4) && (iVar6 = 0x40, iVar4 < 0x41)) {
      iVar6 = iVar4;
    }
    param_1[0x21] = param_1[0x21] + iVar6;
switchD_80100d3c_caseD_3:
    iVar4 = Terrain_HeightAt/*0x80025400*/(param_1[0x12],param_1[0x14]);
    iVar4 = iVar4 - (param_1[0x13] + 0x5000);
    iVar7 = ((int)(((uint)*(ushort *)(param_1[0x23] + 0x42) - (uint)*(ushort *)((int)param_1 + 0x42)
                   ) * 0x100000) >> 0x14) - (int)(short)param_1[0x2a];
    iVar6 = -8;
    if ((-9 < iVar7) && (iVar6 = 8, iVar7 < 9)) {
      iVar6 = iVar7;
    }
    iVar6 = (uint)(ushort)param_1[0x2a] + iVar6;
    iVar7 = iVar6 * 0x10000 >> 0x10;
    *(short *)(param_1 + 0x2a) = (short)iVar6;
    if (iVar7 < 0) {
      iVar7 = iVar7 + 0x3f;
    }
    iVar6 = -param_1[0x20];
    *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + (short)(iVar7 >> 6);
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xf;
    }
    iVar6 = iVar6 >> 4;
    iVar7 = -0x40;
    if ((-0x41 < iVar6) && (iVar7 = 0x40, iVar6 < 0x41)) {
      iVar7 = iVar6;
    }
    iVar6 = -param_1[0x22];
    param_1[0x20] = param_1[0x20] + iVar7;
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xf;
    }
    iVar6 = iVar6 >> 4;
    iVar7 = -0x40;
    if ((-0x41 < iVar6) && (iVar7 = 0x40, iVar6 < 0x41)) {
      iVar7 = iVar6;
    }
    param_1[0x22] = param_1[0x22] + iVar7;
    iVar6 = iVar4;
    if (iVar4 < 0) {
      iVar6 = iVar4 + 0xf;
    }
    iVar7 = 0x2fa;
    if (iVar6 >> 4 < 0x2fa) {
      iVar7 = iVar6 >> 4;
    }
    iVar7 = iVar7 - param_1[0x21];
    iVar6 = -0x40;
    if ((-0x41 < iVar7) && (iVar6 = 0x40, iVar7 < 0x41)) {
      iVar6 = iVar7;
    }
    param_1[0x21] = param_1[0x21] + iVar6;
    FUN_8001d708/*0x8001d708*/(param_1);
    if ((iVar4 < 0x199) && ((short)param_1[0x2a] < 0x40)) {
      *(undefined1 *)(param_1 + 2) = 3;
      *(undefined2 *)((int)param_1 + 0xaa) = 900;
      uVar5 = FUN_8004410c/*0x8004410c*/();
      FUN_8004483c/*0x8004483c*/(uVar5,_DAT_800658fc,0x14,param_1 + 0x12);
switchD_80100d3c_caseD_4:
      sVar2 = *(short *)((int)param_1 + 0xaa) + -1;
      *(short *)((int)param_1 + 0xaa) = sVar2;
      if (sVar2 == -1) {
        *(undefined2 *)((int)param_1 + 0xaa) = 0;
        *(undefined1 *)(param_1 + 2) = 0;
switchD_80100d3c_caseD_5:
        if ((int)param_1[0x20] < 0) {
          if (*(short *)((int)param_1 + 0x4a) < *(short *)param_1[0x29]) {
            param_1[0x22] = 0;
            goto LAB_80101330;
          }
LAB_80101354:
          if ((int)param_1[0x22] < 0) {
            if (*(short *)((int)param_1 + 0x52) < *(short *)(param_1[0x29] + 2)) {
              param_1[0x22] = 0;
              goto LAB_80101388;
            }
          }
          else {
LAB_80101388:
            if ((int)*(short *)(param_1[0x29] + 2) + (int)*(short *)(param_1[0x29] + 6) <
                (int)*(short *)((int)param_1 + 0x52)) goto LAB_801013ac;
          }
        }
        else {
LAB_80101330:
          if ((int)*(short *)((int)param_1 + 0x4a) <=
              (int)*(short *)param_1[0x29] + (int)((short *)param_1[0x29])[2]) goto LAB_80101354;
LAB_801013ac:
          param_1[0x22] = 0;
          param_1[0x20] = 0;
        }
        if (-0x200 < (short)param_1[0x10]) {
          *(short *)(param_1 + 0x10) = (short)param_1[0x10] + -4;
        }
        FUN_8001d708/*0x8001d708*/(param_1);
        iVar6 = (short)param_1[5] * 0xbeb;
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xfff;
        }
        iVar6 = (iVar6 >> 0xc) - param_1[0x20];
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xf;
        }
        iVar6 = iVar6 >> 4;
        iVar4 = -0x40;
        if ((-0x41 < iVar6) && (iVar4 = 0x40, iVar6 < 0x41)) {
          iVar4 = iVar6;
        }
        iVar6 = *(short *)((int)param_1 + 0x1a) * 0xbeb;
        param_1[0x20] = param_1[0x20] + iVar4;
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xfff;
        }
        iVar6 = (iVar6 >> 0xc) - param_1[0x21];
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xf;
        }
        iVar6 = iVar6 >> 4;
        iVar4 = -0x40;
        if ((-0x41 < iVar6) && (iVar4 = 0x40, iVar6 < 0x41)) {
          iVar4 = iVar6;
        }
        iVar6 = (short)param_1[8] * 0xbeb;
        param_1[0x21] = param_1[0x21] + iVar4;
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xfff;
        }
        iVar6 = (iVar6 >> 0xc) - param_1[0x22];
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xf;
        }
        iVar6 = iVar6 >> 4;
        iVar4 = -0x40;
        if ((-0x41 < iVar6) && (iVar4 = 0x40, iVar6 < 0x41)) {
          iVar4 = iVar6;
        }
        param_1[0x22] = param_1[0x22] + iVar4;
        iVar6 = Terrain_HeightAt/*0x80025400*/(param_1[0x12],param_1[0x14]);
        if ((iVar6 < (int)param_1[0x13]) && ((*param_1 & 0x8000) == 0)) {
          FUN_8003fc50/*0x8003fc50*/(param_1);
          *(undefined1 *)(param_1 + 2) = 5;
switchD_80100d3c_caseD_6:
          if ((*param_1 & 0x8000) == 0) {
            FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
            *(undefined1 *)((int)param_1 + 5) = 0;
            FUN_800205f8/*0x800205f8*/(param_1);
          }
        }
      }
    }
switchD_80100d3c_default:
    if (param_3 == (uint *)0x0) {
      return 0;
    }
    param_3 = (uint *)((int)param_3 * 0x100);
    for (uVar8 = param_1[0xe]; uVar8 != 0; uVar8 = *(uint *)(uVar8 + 0x34)) {
      iVar6 = 1;
      if (*(short *)(uVar8 + 6) == 0) {
        iVar6 = (uint)*(ushort *)(uVar8 + 0x44) + (int)param_3;
        *(short *)(uVar8 + 0x44) = (short)iVar6;
      }
      if (*(short *)(uVar8 + 6) == iVar6) {
        if ((char)param_1[2] == '\x03') {
          if (0x354 < *(short *)(uVar8 + 0x40)) goto LAB_80101648;
          *(short *)(uVar8 + 0x40) = *(short *)(uVar8 + 0x40) + 0x20;
        }
        if (*(short *)(uVar8 + 0x40) != 0) {
          *(short *)(uVar8 + 0x40) = *(short *)(uVar8 + 0x40) + -0x20;
          FUN_8001d708/*0x8001d708*/(uVar8);
        }
      }
LAB_80101648:
    }
    uVar5 = FUN_800449bc/*0x800449bc*/(param_1 + 0x12);
    FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar5);
    break;
  case 1:
switchD_80100cc4_caseD_1:
    *param_1 = 0x84;
    iVar6 = FUN_8001fd9c/*0x8001fd9c*/(0);
    param_1[0x29] = iVar6 + 0xc;
    cVar1 = FUN_8004410c/*0x8004410c*/();
    *(char *)((int)param_1 + 5) = cVar1;
    FUN_800443c8/*0x800443c8*/((int)cVar1,*(undefined4 *)(param_1[0x16] + 8),4,0);
  case 4:
    FUN_80042f5c/*0x80042f5c*/(param_1 + 0x25);
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
switchD_80100cc4_caseD_5:
    return 0;
  case 2:
    break;
  case 3:
    goto switchD_80100cc4_caseD_3;
  default:
    goto switchD_80100cc4_caseD_5;
  case 6:
    goto switchD_80100cc4_caseD_6;
  }
  *param_1 = *param_1 & 0xffffffdf;
switchD_80100cc4_caseD_3:
  puVar10 = (uint *)*param_3;
  if (((char)puVar10[1] == '\x02') && (*(short *)((int)puVar10 + 6) < 0)) {
    iVar6 = (int)(char)param_1[2];
    if (iVar6 == 3) {
      iVar6 = Math_Atan2_Pos/*0x80016c88*/(puVar10 + 4);
      iVar6 = (int)((iVar6 - (uint)*(ushort *)((int)param_1 + 0x42)) * 0x100000) >> 0x10;
      if (iVar6 < 0) {
        iVar6 = -iVar6;
      }
      if (iVar6 < 0x1000) {
        param_3 = (uint *)puVar10[0x38];
        param_1[0x24] = (uint)puVar10;
        puVar10[0x19] = (uint)FUN_801006d4;
        puVar10[0x1e] = (uint)param_1;
        *puVar10 = *puVar10 & 0xfffffff7 | 0x1000020;
        puVar10[0x20] = (param_1[0x12] - puVar10[9]) * 4;
        puVar10[0x21] = (param_1[0x13] - puVar10[10]) * 4;
        puVar10[0x22] = (param_1[0x14] - puVar10[0xb]) * 4;
        *(undefined2 *)(puVar10 + 0x29) = 0;
        FUN_80044574/*0x80044574*/((int)*(char *)((int)puVar10 + 5),0);
        FUN_80020890/*0x80020890*/(puVar10,0x20);
        if (param_3 != (uint *)0x0) {
          puVar3 = (uint *)Heap_AllocOrRetry/*0x800116f4*/(0x30);
          uVar8 = param_3[0x13];
          uVar9 = param_3[0x14];
          *puVar3 = param_3[0x12];
          puVar3[1] = uVar8;
          puVar3[2] = uVar9;
          puVar3[3] = 0x78;
          puVar3[4] = param_1[9] + (short)param_1[5] * -100;
          puVar3[5] = param_1[10] - 0x32000;
          puVar3[6] = param_1[0xb] + (short)param_1[8] * -100;
          puVar3[7] = 0;
          func_0x8003dbb0(param_3);
          param_3[0x25] = 0xfa000;
          *(undefined2 *)((int)param_1 + 0xaa) = 0;
          *(undefined1 *)(param_1 + 2) = 0;
        }
        FUN_8004410c/*0x8004410c*/();
      }
      goto LAB_8010185c;
    }
  }
  else {
LAB_8010185c:
    iVar6 = (int)(char)param_1[2];
  }
  if (iVar6 < 4) {
    uVar9 = (uint)(byte)puVar10[1];
    uVar8 = 3;
    if (uVar9 == 7) {
      uVar11 = FUN_80022320/*0x80022320*/(param_1,(short)puVar10[3]);
      uVar9 = (uint)((ulonglong)uVar11 >> 0x20);
      if ((int)uVar11 == 0) {
        return 0;
      }
      *(undefined1 *)(param_1 + 2) = 4;
      uVar8 = 0;
    }
    if (uVar9 == uVar8) {
      return 0;
    }
    FUN_8001f5a0/*0x8001f5a0*/(param_1,param_3);
    iVar6 = param_1[0x20] * (int)(short)param_3[8] +
            param_1[0x21] * (int)*(short *)((int)param_3 + 0x22) +
            param_1[0x22] * (int)(short)param_3[9];
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0x7ff;
    }
    iVar6 = iVar6 >> 0xb;
    if (-1 < iVar6) {
      return 0;
    }
    iVar4 = iVar6 * (short)param_3[8];
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xfff;
    }
    param_1[0x20] = param_1[0x20] - (iVar4 >> 0xc);
    iVar4 = iVar6 * *(short *)((int)param_3 + 0x22);
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xfff;
    }
    param_1[0x21] = param_1[0x21] - (iVar4 >> 0xc);
    iVar6 = iVar6 * (short)param_3[9];
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xfff;
    }
    param_1[0x22] = param_1[0x22] - (iVar6 >> 0xc);
    uVar5 = FUN_8004410c/*0x8004410c*/();
    FUN_8004483c/*0x8004483c*/(uVar5,_DAT_800658fc,7,param_1 + 0x12);
    iVar6 = extraout_v1;
  }
  if (4 < iVar6) {
    return 0;
  }
  if ((char)puVar10[1] != '\0') {
    return 0;
  }
  if ((*param_1 & 0x8000) != 0) {
    return 0;
  }
  FUN_8003fc50/*0x8003fc50*/(param_1);
  *(undefined1 *)(param_1 + 2) = 5;
switchD_80100cc4_caseD_6:
  goto switchD_80100cc4_caseD_1;
}

#endif  /* GHIDRA REF */
