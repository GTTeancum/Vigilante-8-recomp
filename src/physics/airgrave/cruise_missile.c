/* cruise_missile.c -- AirGrave cruise-missile state machine.
 *
 * Source: AIRGRAVE.DLL  FUN_80100c70.
 *
 * Per-tick handler for an in-flight cruise missile that arcs from
 * launch over the map until it hits terrain or a target. Two switch
 * layers:
 *
 *   outer (param_2): event kind
 *     0 = per-tick integrate+state-step
 *     1 = post-spawn finalize (set hitbox + bind retire callback)
 *     2 = on-explode (detach submodels, spawn debris)
 *     3 = damage event (param_3 = impactor)
 *     4 = re-collide (after retire delay)
 *     7,8 = forwarded damage variants
 *
 *   inner (obj+2 high byte): boost/cruise/dive sub-stage 0..8
 *     0  -- ramping boost; per-frame +0x20 to vel; emit smoke puff
 *           every (rng % 4) == 0 frames
 *     1  -- end-of-boost: orient body+1 each frame, drop pitch up,
 *           recompute velocity at 0x3b9a / 4096 scale from facing
 *     2  -- mid-cruise: warp toward fixed waypoint (0x4d30968,
 *           groundY-900*pitch, 0x3c50000) when X passes 0x40000400
 *     3  -- dive: when terrain.Y >= obj.Y, transition to ground-skim
 *     4  -- ground-skim: velocity decays, pitch zeroed
 *     5  -- waypoint X < 0x3a44b29 trigger
 *     6  -- final descent: shrink pitch by -8 deg/frame, vel rescaled
 *           by 0xbeb/4096 from facing
 *     7  -- waypoint Z < 0x39acb29 trigger
 *     8  -- terminal dive: pitch keeps shrinking until reaches -0xbff,
 *           then state -> 0 (cycle complete: explode and respawn)
 *
 * After each state-step, walks child submodels at obj+0xe..+0x34 and
 * spins each by (param_3 * 0x100) units around the dish yaw. Updates
 * SFX position via FUN_80044574.
 *
 * MED. Inner cascade is exactly the FALL-THROUGH switch from the
 * original; goto-target labels match the pseudo-C.
 */
#include <stdint.h>

extern int  Terrain_HeightAt(int32_t x, int32_t z);                  /* FUN_80025400 */
extern void Object_RefitAABB(void *self);                            /* FUN_8001d708 */
extern void FX_TrailPuff(void *self, void *src);                     /* FUN_8001787c */
extern void *Object_SpawnFromBank(uint32_t bank, uint8_t kind,
                                  uint8_t prio, uint8_t flag);       /* FUN_8001ac44 */
extern void Object_Suspend(void *self);                              /* FUN_8002036c */
extern uint8_t SFX_PlayWorld(int32_t *posXyz);                       /* FUN_800449bc */
extern void SFX_Update(int handle, int posVoxel);                    /* FUN_80044574 */
extern uint32_t _DAT_80065310;
extern uint32_t _DAT_80065b38;
extern uint32_t _DAT_800737d8;
extern void     FUN_80100c18(void);  /* puff tick callback */
extern uint8_t  FUN_8010068c;        /* default tick callback */

/* damage path (case 3+4): */
extern int  Damage_Apply_AgainstSelf(void *self, void *impactor);    /* FUN_80020890 */
extern void Damage_Pool_DropParticles(uint32_t color, int32_t *pos,
                                      int32_t *jitter);              /* FUN_8003cee0 */
extern void SFX_StopWorld(int handle);                               /* FUN_800441c8 */
extern void SubModel_Detach(void *self);                             /* FUN_8003fc50 */
extern void Object_RetireDeferred(void *self);                       /* FUN_800205f8 */
extern uint32_t Vehicle_GetTexFromBank(uint32_t bank, int idx);      /* FUN_8001bda0 */
extern int  Damage_VsImpactor(void *self, int *impactor);            /* func_0x80022320 */
extern void GTE_RotateLongMat(uint32_t *mat, void *src, void *dst);  /* FUN_80043358 */
extern void GTE_RotateLongMatTrans(uint32_t *mat, void *src, void *dst); /* FUN_80043408 */
extern void Util_TransposeMatRotate(int mat, void *src, void *dst);  /* FUN_800435c0 */
extern uint32_t Pool_AllocProjectile(void);                          /* FUN_8004410c */
extern void Pool_LaunchProjectileFx(uint32_t handle, uint32_t bin,
                                    int kind, int *spawnXyz);        /* FUN_800447e8 */
extern void Particles_Burst(int *pos);                               /* FUN_80040234 */
extern int  Damage_VsImpactorAlt(int impactor, int kind, int *pos, int n); /* FUN_8002c6fc */
extern void FX_RingFlash(int kind, void *params, int *spawnXyz);     /* FUN_800176f8 */
extern void Pool_BindFXOnObject(int handle, uint32_t bin, int slot, int aux); /* FUN_800443c8 */
extern void Object_BindTexAndModel(void *self, uint32_t tex);        /* func_0x8003e564 */
extern void FX_RingFlash_Init(void *self, int kind, int dy, int unused); /* FUN_8001f5a0 */
extern int  AG_Rand255(void);                                         /* FUN_80017160 */

uint32_t AG_CruiseMissile(uint32_t *self, int mode, int *arg)
{
    if (mode == 0) {
        if ((char)self[2] >= 0) {
            /* Apply integration. */
            self[0x12] += self[0x20];
            self[0x13] += self[0x21];
            self[0x14] += self[0x22];
            self[9]    = self[0x12];
            self[10]   = self[0x13];
            self[0xb]  = self[0x14];
        }
        int stage = (int)(int8_t)(((uint8_t)self[2] + 1));

        /* Run all stages from `stage` upward (fall-through). */
        if (stage <= 1) {
            FX_TrailPuff(self, (void *)(self[0x17] + 4));
            if (((_DAT_80065310 - (uint32_t)*((uint8_t *)self + 9)) & 3) == 0) {
                uint32_t *puff = (uint32_t *)Object_SpawnFromBank(_DAT_800737d8, 0x21, 0x80, 8);
                *puff |= 0x4b4u;
                puff[0x12] = self[9];
                puff[0x13] = self[10];
                puff[0x14] = self[0xb];
                *(int16_t *)(puff + 0x11) = (int16_t)(_DAT_80065310 * 0x60);
                puff[0x19] = (uint32_t)(uintptr_t)&FUN_80100c18;
                Object_Suspend(self);
            }
            if (stage == 0) {
                uint32_t v = self[0x20] + 0x20;
                self[0x20] = v;
                if ((int)v > 0x3b9a) *(uint8_t *)(self + 2) = 1;
            }
        }
        if (stage <= 2) {  /* case 2 fall-through target */
            *(int16_t *)(self + 0x11) = (int16_t)((int16_t)self[0x11] + 1);
            *(int16_t *)((char *)self + 0x42) += 1;
            Object_RefitAABB(self);
            if ((int)self[0x21] > -0x7bf) self[0x21] -= 6;
            int s5 = ((int16_t)self[5]) * 0x3b9a; if (s5 < 0) s5 += 0xfff;
            self[0x20] = s5 >> 12;
            int s8 = ((int16_t)self[8]) * 0x3b9a; if (s8 < 0) s8 += 0xfff;
            self[0x22] = s8 >> 12;
            if ((int16_t)self[0x11] > 0x155) *(uint8_t *)(self + 2) = 2;
        }
        if (stage <= 3) {
            if (_DAT_80065b38 + 0x400000u < self[0x12]) {
                *(uint16_t *)(self + 0x10)              = 0;
                *(uint16_t *)((char *)self + 0x42)     = 0xfc00;
                *(uint16_t *)(self + 0x11)              = 0;
                self[0x20] = 0xffffc466u;
                self[0x21] = 0x7bf;
                self[0x22] = 0;
                self[0x12] = 0x4d30968;
                int gy = Terrain_HeightAt(0x4018000, 0x3c50000);
                self[0x14] = 0x3c50000;
                self[0x13] = gy + (int32_t)self[0x21] * -900;
                *(uint8_t *)(self + 2) = 3;
                Object_RefitAABB(self);
            }
        }
        if (stage <= 4) {
            int gy = Terrain_HeightAt((int32_t)self[0x12], (int32_t)self[0x14]);
            if (gy <= (int32_t)self[0x13]) { self[0x21] = 0; *(uint8_t *)(self + 2) = 4; }
        }
        if (stage <= 5) {
            uint32_t v = self[0x20] + 0x18;
            self[0x20] = v;
            if ((int)v > -0xbec) *(uint8_t *)(self + 2) = 5;
        }
        if (stage <= 6) {
            if ((int32_t)self[0x12] < 0x3a44b29) *(uint8_t *)(self + 2) = 6;
        }
        if (stage <= 7) {
            *(int16_t *)((char *)self + 0x42) -= 8;
            Object_RefitAABB(self);
            int s5 = ((int16_t)self[5]) * 0xbeb; if (s5 < 0) s5 += 0xfff;
            self[0x20] = s5 >> 12;
            int s8 = ((int16_t)self[8]) * 0xbeb; if (s8 < 0) s8 += 0xfff;
            self[0x22] = s8 >> 12;
            if (*(int16_t *)((char *)self + 0x42) < -0x7ff) *(uint8_t *)(self + 2) = 7;
        }
        if (stage <= 8) {
            if ((int32_t)self[0x14] < 0x39acb29) *(uint8_t *)(self + 2) = 8;
        }
        if (stage <= 9) {
            *(int16_t *)((char *)self + 0x42) -= 8;
            Object_RefitAABB(self);
            int s5 = ((int16_t)self[5]) * 0xbeb; if (s5 < 0) s5 += 0xfff;
            self[0x20] = s5 >> 12;
            int s8 = ((int16_t)self[8]) * 0xbeb; if (s8 < 0) s8 += 0xfff;
            self[0x22] = s8 >> 12;
            if (*(int16_t *)((char *)self + 0x42) < -0xbff) *(uint8_t *)(self + 2) = 0;
        }

        /* Spin all child submodels, refresh world sfx. */
        if (arg == NULL) return 0;
        for (uintptr_t c = self[0xe]; c != 0; c = *(uint32_t *)(c + 0x34)) {
            if (*(int16_t *)(c + 6) == 0) {
                *(int16_t *)(c + 0x44) += (int16_t)((intptr_t)arg * 0x100);
                Object_RefitAABB((void *)c);
            }
        }
        int sfx = SFX_PlayWorld((int32_t *)(self + 0x12));
        SFX_Update((int)*((char *)self + 5), sfx << 1);
        return 0;
    }

    /* Non-tick modes -- damage, retire, explode. */
    if (mode == 1) goto bind_callback;
    if (mode == 2) goto detach_and_retire;
    if (mode == 3) {
        int impactor = *arg;
        if (*(int16_t *)(arg[1] + 2) != 0) {
            char k = *(char *)(impactor + 4);
            if (k == 2) {
                FX_RingFlash_Init(self, *arg, 0x80000, 0);
                /* fall-through to explode below */
                int local[4] = { 0x80000, 0xfffc0000, 0x40000, 0 };
                if ((int32_t)arg[5] < 0) local[0] = 0xfff80000;
                GTE_RotateLongMat(self + 4, local, local);
                GTE_RotateLongMatTrans(self + 4, arg + 5, arg + 5);
                FX_RingFlash(impactor, local, arg + 5);
                Util_TransposeMatRotate((int)(uintptr_t)(impactor + 0x10), arg + 5, arg + 5);
                uint32_t projH = Pool_AllocProjectile();
                Pool_LaunchProjectileFx(projH, *(uint32_t *)(self[0x16] + 8), 3, arg + 5);
                Particles_Burst(arg + 5);
                Damage_VsImpactorAlt(impactor, -100, arg + 5, 1);
            }
        }
        if (*(char *)(impactor + 4) == 7) {
            arg = (int *)(uintptr_t)*(uint16_t *)(impactor + 0xc);
            goto explode_check;
        }
        return 0;
    }
    if (mode == 4) goto retire_sfx;
    if (mode == 7) goto retire_sfx;
    if (mode == 8) goto explode_check;
    return 0;

explode_check: {
        int dmg = Damage_VsImpactor((void *)self, arg);
        if (dmg == 0) return 0;
        *(uint8_t *)(self + 2) = 0xff;
        self[0x20] <<= 7;
        self[0x21] <<= 7;
        self[0x22] <<= 7;
        Damage_Apply_AgainstSelf(self, (void *)(intptr_t)300);
        for (int i = 0; i < 3; i++) {
            int jx = (AG_Rand255() * 0xbeb >> 15) - 0x5f5;
            int jy = 0xffffee1f;
            int jz = (AG_Rand255() * 0xbeb >> 15) - 0x5f5;
            int jit[3] = { jx, jy, jz };
            Damage_Pool_DropParticles(0x7f780000u, (int32_t *)(self + 0x12), jit);
        }
        SFX_StopWorld((int)*((char *)self + 5));
        *((char *)self + 5) = 0;
detach_and_retire:
        SubModel_Detach(self);
        Object_RetireDeferred(self);
retire_sfx:
        Object_SpawnFromBank((uint32_t)(uintptr_t)self, (uint8_t)((uintptr_t)arg & 0xffff),
                             0xa8, 0);
bind_callback:
        *(uint16_t *)(self + 0x27) = 0x40;
        *(uint16_t *)((char *)self + 0x9e) = 0x40;
        *(uint16_t *)(self + 0x28) = 0x40;
        uint32_t *retire = self;
        if ((char)self[2] != 0) {
            self[0x19] = (uint32_t)(uintptr_t)&FUN_8010068c;
            *(uint8_t *)(self + 2) = 0;
            *self |= 0x12au;
            Damage_Apply_AgainstSelf(self, (void *)(intptr_t)0x3c);
            retire = (uint32_t *)1;
        }
        *self |= 0x188u;
        char sfx = (char)Pool_AllocProjectile();
        *((char *)self + 5) = sfx;
        Pool_BindFXOnObject(sfx, *(uint32_t *)(self[0x16] + 8), 1, 0);
        uint32_t tex = Vehicle_GetTexFromBank(self[0x16], 0xe0);
        Object_BindTexAndModel(self, tex);
        SFX_StopWorld((int)*((char *)self + 5));
        return 0;
    }
}
