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

uint32_t CC_BlimpMain(uint32_t *self, uint32_t mode, uint32_t *imp)
{
    switch (mode) {
    case 0:
        /* Integrate. */
        self[0x12] += self[0x20];
        self[0x13] += self[0x21];
        self[0x14] += self[0x22];
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
            int dy = (int)((yawErr - (uint16_t)self[0x2a]) * 0x100000) >> 20;
            dy = clamp_pm(dy, 8);
            int newY = (uint16_t)self[0x2a] + dy;
            int sm  = (newY << 16) >> 16;
            *(int16_t *)(self + 0x2a) = (int16_t)newY;
            if (sm < 0) sm += 0x3f;
            *(int16_t *)((char *)self + 0x42) += (int16_t)(sm >> 6);
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
                *(int16_t *)((char *)self + 0x42) += 0x10;
                self[0x20] = -self[0x20];
                self[0x22] = -self[0x22];
            }
            Object_RefitAABB(self);
            /* Drag toward facing direction. */
            int32_t drag5 = ((int16_t)self[5]) * 0xbeb; if (drag5 < 0) drag5 += 0xfff;
            int32_t adj   = (drag5 >> 12) - (int32_t)self[0x20];
            if (adj < 0) adj += 0xf;
            self[0x20] += clamp_pm(adj >> 4, 0x40);
            int32_t drag8 = ((int16_t)self[8]) * 0xbeb; if (drag8 < 0) drag8 += 0xfff;
            adj = (drag8 >> 12) - (int32_t)self[0x22];
            if (adj < 0) adj += 0xf;
            self[0x22] += clamp_pm(adj >> 4, 0x40);
            /* Vertical hover. */
            int ty = Terrain_HeightAt(self[0x12], self[0x14])
                     - (self[0x13] + 0x64000);
            if (ty < 0) ty += 0xf;
            ty = ty >> 4;
            if (ty < -0x2fa) ty = -0x2fa;
            int dvy = ty - (int32_t)self[0x21];
            self[0x21] += clamp_pm(dvy, 0x40);
            /* fall through to case 3 */
        case 3: {
            int ty = Terrain_HeightAt(self[0x12], self[0x14])
                     - (self[0x13] + 0x5000);
            int yawErr = (int)(((uint16_t)*(uint16_t *)(self[0x23] + 0x42)
                              - (uint16_t)*(uint16_t *)((char *)self + 0x42))
                              * 0x100000) >> 20;
            int dy = yawErr - (int16_t)self[0x2a];
            dy = clamp_pm(dy, 8);
            int newY = (uint16_t)self[0x2a] + dy;
            *(int16_t *)(self + 0x2a) = (int16_t)newY;
            int sm = (newY << 16) >> 16;
            if (sm < 0) sm += 0x3f;
            *(int16_t *)((char *)self + 0x42) += (int16_t)(sm >> 6);
            int32_t adj = -(int32_t)self[0x20]; if (adj < 0) adj += 0xf;
            self[0x20] += clamp_pm(adj >> 4, 0x40);
            adj = -(int32_t)self[0x22]; if (adj < 0) adj += 0xf;
            self[0x22] += clamp_pm(adj >> 4, 0x40);
            int tmp = ty; if (tmp < 0) tmp += 0xf;
            int dvy = (tmp >> 4 > 0x2fa) ? 0x2fa : tmp >> 4;
            dvy -= (int32_t)self[0x21];
            self[0x21] += clamp_pm(dvy, 0x40);
            Object_RefitAABB(self);
            if (ty < 0x199 && (int16_t)self[0x2a] < 0x40) {
                *(uint8_t *)(self + 2) = 3;
                *(int16_t *)((char *)self + 0xaa) = 900;
                uint32_t h = Pool_AllocProjectile();
                Pool_LaunchProjectile(h, _DAT_800658fc, 0x14, self + 0x12);
            }
        } /* fall through */
        case 4: {
            int16_t t = (int16_t)((char *)self)[0xaa] - 1;
            *(int16_t *)((char *)self + 0xaa) = t;
            if (t == -1) {
                *(int16_t *)((char *)self + 0xaa) = 0;
                *(uint8_t *)(self + 2) = 0;
            } else break;
        } /* fall through to case 5 */
        case 5:
            /* Death descent dynamics. */
            if (-0x200 < (int16_t)self[0x10]) *(int16_t *)(self + 0x10) -= 4;
            Object_RefitAABB(self);
            { int32_t a = ((int16_t)self[5]) * 0xbeb; if (a<0) a+=0xfff;
              int32_t d=(a>>12)-(int32_t)self[0x20]; if (d<0) d+=0xf;
              self[0x20] += clamp_pm(d>>4, 0x40);
              a = (*(int16_t *)((char *)self+0x1a)) * 0xbeb; if (a<0) a+=0xfff;
              d=(a>>12)-(int32_t)self[0x21]; if (d<0) d+=0xf;
              self[0x21] += clamp_pm(d>>4, 0x40);
              a = ((int16_t)self[8]) * 0xbeb; if (a<0) a+=0xfff;
              d=(a>>12)-(int32_t)self[0x22]; if (d<0) d+=0xf;
              self[0x22] += clamp_pm(d>>4, 0x40); }
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
                v = (uint16_t)*(uint16_t *)(c + 0x44) + (intptr_t)imp;
                *(int16_t *)(c + 0x44) = (int16_t)v;
            }
            if (*(int16_t *)(c + 6) == v) {
                if ((char)self[2] == 3) {
                    if (0x354 < *(int16_t *)(c + 0x40)) continue;
                    *(int16_t *)(c + 0x40) += 0x20;
                }
                if (*(int16_t *)(c + 0x40) != 0) {
                    *(int16_t *)(c + 0x40) -= 0x20;
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
        uint32_t *p = (uint32_t *)(uintptr_t)*imp;
        int blimpState = (int)(char)self[2];
        if ((char)p[1] == 2 && *(int16_t *)((char *)p + 6) < 0) {
            if (blimpState == 3) {
                int yaw = Math_Atan2_Pos(p + 4);
                int d   = (int)((yaw - (uint16_t)*((uint16_t *)((char *)self + 0x42))) * 0x100000) >> 16;
                if (d < 0) d = -d;
                if (d < 0x1000) {
                    uint32_t *bin = (uint32_t *)(uintptr_t)p[0x38];
                    self[0x24] = (uint32_t)(uintptr_t)p;
                    p[0x19] = (uint32_t)(uintptr_t)&FUN_801006d4;
                    p[0x1e] = (uint32_t)(uintptr_t)self;
                    *p = (*p & 0xfffffff7u) | 0x1000020u;
                    p[0x20] = (self[0x12] - p[9])  * 4;
                    p[0x21] = (self[0x13] - p[10]) * 4;
                    p[0x22] = (self[0x14] - p[0xb]) * 4;
                    *(int16_t *)(p + 0x29) = 0;
                    SFX_Update((int)*((char *)p + 5), 0);
                    Damage_Apply_AgainstSelf(p, (void *)(intptr_t)0x20);
                    if (bin) {
                        uint32_t *ctx = (uint32_t *)Heap_AllocOrRetry(0x30);
                        ctx[0] = bin[0x12]; ctx[1] = bin[0x13]; ctx[2] = bin[0x14];
                        ctx[3] = 0x78;
                        ctx[4] = self[9]  + (int16_t)self[5] * -100;
                        ctx[5] = self[10] - 0x32000;
                        ctx[6] = self[0xb]+ (int16_t)self[8] * -100;
                        ctx[7] = 0;
                        Spawner_Promote(bin);
                        bin[0x25] = 0xfa000u;
                        *(int16_t *)((char *)self + 0xaa) = 0;
                        *(uint8_t *)(self + 2) = 0;
                    }
                    Pool_AllocProjectile_NoArg();
                }
            }
        }
        if (blimpState >= 4) return 0;
        uint8_t k = (uint8_t)p[1];
        int target = 3;
        if (k == 7) {
            if (Damage_VsImpactor(self, (int)(int16_t)p[3]) == 0) return 0;
            *(uint8_t *)(self + 2) = 4;
            target = 0;
        }
        if (k != target) return 0;
        FX_RingFlash_Init(self, p);
        int32_t vdot = (int32_t)self[0x20] * (int16_t)p[8]
                     + (int32_t)self[0x21] * *(int16_t *)((char *)p + 0x22)
                     + (int32_t)self[0x22] * (int16_t)p[9];
        if (vdot < 0) vdot += 0x7ff;
        vdot >>= 11;
        if (vdot >= 0) return 0;
        int32_t bx = vdot * (int16_t)p[8];                   if (bx < 0) bx += 0xfff;
        int32_t by = vdot * *(int16_t *)((char *)p + 0x22);   if (by < 0) by += 0xfff;
        int32_t bz = vdot * (int16_t)p[9];                   if (bz < 0) bz += 0xfff;
        self[0x20] -= bx >> 12; self[0x21] -= by >> 12; self[0x22] -= bz >> 12;
        uint32_t h = Pool_AllocProjectile();
        Pool_LaunchProjectile(h, _DAT_800658fc, 7, self + 0x12);
        if ((char)p[1] != 0) return 0;
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
