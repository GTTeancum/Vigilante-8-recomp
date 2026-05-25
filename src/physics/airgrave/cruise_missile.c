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

extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
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

static int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

static int32_t rtz12(int32_t v)
{
    if (v < 0) v = mips_addu_i32(v, 0xfff);
    return v >> 12;
}

static int32_t scale_sin_3b9a(int16_t v)
{
    int32_t t = mips_sll_i32(v, 2);
    t = mips_addu_i32(t, v);
    t = mips_sll_i32(t, 5);
    t = mips_subu_i32(t, v);
    t = mips_sll_i32(t, 4);
    t = mips_subu_i32(t, v);
    t = mips_sll_i32(t, 2);
    t = mips_subu_i32(t, mips_subu_i32(t >> 2, 0)); /* net *6 on previous term */
    t = mips_sll_i32(t, 1);
    return rtz12(t);
}

static int32_t scale_sin_beb(int16_t v)
{
    int32_t t = mips_sll_i32(v, 1);
    t = mips_addu_i32(t, v);
    t = mips_sll_i32(t, 6);
    t = mips_subu_i32(t, v);
    t = mips_sll_i32(t, 2);
    t = mips_subu_i32(t, v);
    t = mips_sll_i32(t, 2);
    t = mips_subu_i32(t, v);
    return rtz12(t);
}

uint32_t AG_CruiseMissile(uint32_t *self, int mode, int *arg)
{
    if (mode == 0) {
        if ((char)self[2] >= 0) {
            /* Apply integration. */
            self[0x12] = (uint32_t)mips_addu_i32((int32_t)self[0x12], (int32_t)self[0x20]);
            self[0x13] = (uint32_t)mips_addu_i32((int32_t)self[0x13], (int32_t)self[0x21]);
            self[0x14] = (uint32_t)mips_addu_i32((int32_t)self[0x14], (int32_t)self[0x22]);
            self[9]    = self[0x12];
            self[10]   = self[0x13];
            self[0xb]  = self[0x14];
        }
        int stage = (int)(int8_t)((uint8_t)mips_addu_i32((uint8_t)self[2], 1));

        /* Run all stages from `stage` upward (fall-through). */
        if (stage <= 1) {
            FX_TrailPuff(self, (void *)(self[0x17] + 4));
            if (((uint32_t)mips_subu_i32((int32_t)_DAT_80065310,
                                         (int32_t)*((uint8_t *)self + 9)) & 3) == 0) {
                uint32_t *puff = (uint32_t *)Object_SpawnFromBank(_DAT_800737d8, 0x21, 0x80, 8);
                *puff |= 0x4b4u;
                puff[0x12] = self[9];
                puff[0x13] = self[10];
                puff[0x14] = self[0xb];
                *(int16_t *)(puff + 0x11) = (int16_t)mips_mult_lo_i32((int32_t)_DAT_80065310, 0x60);
                Object_SetCallbackPsxSlot(puff, (uintptr_t)&FUN_80100c18);
                Object_Suspend(self);
            }
            if (stage == 0) {
                uint32_t v = (uint32_t)mips_addu_i32((int32_t)self[0x20], 0x20);
                self[0x20] = v;
                if ((int)v > 0x3b9a) *(uint8_t *)(self + 2) = 1;
            }
        }
        if (stage <= 2) {  /* case 2 fall-through target */
            *(int16_t *)(self + 0x11) = (int16_t)mips_addu_i32((int16_t)self[0x11], 1);
            *(int16_t *)((char *)self + 0x42) =
                (int16_t)mips_addu_i32(*(int16_t *)((char *)self + 0x42), 1);
            Object_RefitAABB(self);
            if ((int)self[0x21] > -0x7bf)
                self[0x21] = (uint32_t)mips_addu_i32((int32_t)self[0x21], -6);
            self[0x20] = (uint32_t)scale_sin_3b9a((int16_t)self[5]);
            self[0x22] = (uint32_t)scale_sin_3b9a((int16_t)self[8]);
            if ((int16_t)self[0x11] > 0x155) *(uint8_t *)(self + 2) = 2;
        }
        if (stage <= 3) {
            if ((uint32_t)mips_addu_i32((int32_t)_DAT_80065b38, 0x400000) < self[0x12]) {
                *(uint16_t *)(self + 0x10)              = 0;
                *(uint16_t *)((char *)self + 0x42)     = 0xfc00;
                *(uint16_t *)(self + 0x11)              = 0;
                self[0x20] = 0xffffc466u;
                self[0x21] = 0x7bf;
                self[0x22] = 0;
                self[0x12] = 0x4d30968;
                int gy = Terrain_HeightAt(0x4018000, 0x3c50000);
                self[0x14] = 0x3c50000;
                self[0x13] = (uint32_t)mips_subu_i32(gy,
                                                     mips_mult_lo_i32((int32_t)self[0x21], 900));
                *(uint8_t *)(self + 2) = 3;
                Object_RefitAABB(self);
            }
        }
        if (stage <= 4) {
            int gy = Terrain_HeightAt((int32_t)self[0x12], (int32_t)self[0x14]);
            if (gy <= (int32_t)self[0x13]) { self[0x21] = 0; *(uint8_t *)(self + 2) = 4; }
        }
        if (stage <= 5) {
            uint32_t v = (uint32_t)mips_addu_i32((int32_t)self[0x20], 0x18);
            self[0x20] = v;
            if ((int)v > -0xbec) *(uint8_t *)(self + 2) = 5;
        }
        if (stage <= 6) {
            if ((int32_t)self[0x12] < 0x3a44b29) *(uint8_t *)(self + 2) = 6;
        }
        if (stage <= 7) {
            *(int16_t *)((char *)self + 0x42) =
                (int16_t)mips_addu_i32(*(int16_t *)((char *)self + 0x42), -8);
            Object_RefitAABB(self);
            self[0x20] = (uint32_t)scale_sin_beb((int16_t)self[5]);
            self[0x22] = (uint32_t)scale_sin_beb((int16_t)self[8]);
            if (*(int16_t *)((char *)self + 0x42) < -0x7ff) *(uint8_t *)(self + 2) = 7;
        }
        if (stage <= 8) {
            if ((int32_t)self[0x14] < 0x39acb29) *(uint8_t *)(self + 2) = 8;
        }
        if (stage <= 9) {
            *(int16_t *)((char *)self + 0x42) =
                (int16_t)mips_addu_i32(*(int16_t *)((char *)self + 0x42), -8);
            Object_RefitAABB(self);
            self[0x20] = (uint32_t)scale_sin_beb((int16_t)self[5]);
            self[0x22] = (uint32_t)scale_sin_beb((int16_t)self[8]);
            if (*(int16_t *)((char *)self + 0x42) < -0xbff) *(uint8_t *)(self + 2) = 0;
        }

        /* Spin all child submodels, refresh world sfx. */
        if (arg == NULL) return 0;
        for (uintptr_t c = self[0xe]; c != 0; c = *(uint32_t *)(c + 0x34)) {
            if (*(int16_t *)(c + 6) == 0) {
                *(int16_t *)(c + 0x44) =
                    (int16_t)mips_addu_i32(*(int16_t *)(c + 0x44),
                                           mips_sll_i32((int32_t)(intptr_t)arg, 8));
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
        self[0x20] = (uint32_t)mips_sll_i32((int32_t)self[0x20], 7);
        self[0x21] = (uint32_t)mips_sll_i32((int32_t)self[0x21], 7);
        self[0x22] = (uint32_t)mips_sll_i32((int32_t)self[0x22], 7);
        Damage_Apply_AgainstSelf(self, (void *)(intptr_t)300);
        for (int i = 0; i < 3; i++) {
            int jx = mips_addu_i32(mips_mult_lo_i32(AG_Rand255(), 0xbeb) >> 15, -0x5f5);
            int jy = 0xffffee1f;
            int jz = mips_addu_i32(mips_mult_lo_i32(AG_Rand255(), 0xbeb) >> 15, -0x5f5);
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
            Object_SetCallbackPsxSlot(self, (uintptr_t)&FUN_8010068c);
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

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- AIRGRAVE.DLL FUN_80100c70  (from analysis/dll/AIRGRAVE/decomp/80100c70.c) --- */
// addr: 0x80100c70  name: FUN_80100c70

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100c70(uint *param_1,undefined4 param_2,int *param_3)

{
  short sVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  uint *puVar6;
  uint uVar7;
  uint uVar8;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  int local_28;
  undefined4 local_24;
  int local_20;
  
  switch(param_2) {
  case 0:
    if (-1 < (char)param_1[2]) {
      param_1[0x12] = param_1[0x12] + param_1[0x20];
      param_1[0x13] = param_1[0x13] + param_1[0x21];
      param_1[0x14] = param_1[0x14] + param_1[0x22];
      param_1[9] = param_1[0x12];
      param_1[10] = param_1[0x13];
      param_1[0xb] = param_1[0x14];
    }
    switch((int)(((byte)param_1[2] + 1) * 0x1000000) >> 0x18) {
    case 0:
      FUN_8001787c/*0x8001787c*/(param_1,param_1[0x17] + 4);
      if ((_DAT_80065310 - (uint)*(byte *)((int)param_1 + 9) & 3) != 0) {
        return 0;
      }
      puVar6 = (uint *)FUN_8001ac44/*0x8001ac44*/(_DAT_800737d8,0x21,0x80,8);
      sVar1 = (short)_DAT_80065310;
      *puVar6 = *puVar6 | 0x4b4;
      uVar8 = param_1[10];
      uVar7 = param_1[0xb];
      puVar6[0x12] = param_1[9];
      puVar6[0x13] = uVar8;
      puVar6[0x14] = uVar7;
      *(short *)(puVar6 + 0x11) = sVar1 * 0x60;
      puVar6[0x19] = (uint)FUN_80100c18;
      FUN_8002036c/*0x8002036c*/();
    case 1:
      uVar8 = param_1[0x20];
      param_1[0x20] = uVar8 + 0x20;
      if (0x3b9a < (int)(uVar8 + 0x20)) {
        *(undefined1 *)(param_1 + 2) = 1;
switchD_80100d54_caseD_2:
        *(short *)(param_1 + 0x11) = (short)param_1[0x11] + 1;
        *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + 1;
        FUN_8001d708/*0x8001d708*/(param_1);
        if (-0x7bf < (int)param_1[0x21]) {
          param_1[0x21] = param_1[0x21] - 6;
        }
        iVar3 = (short)param_1[5] * 0x3b9a;
        if (iVar3 < 0) {
          iVar3 = iVar3 + 0xfff;
        }
        param_1[0x20] = iVar3 >> 0xc;
        iVar3 = (short)param_1[8] * 0x3b9a;
        if (iVar3 < 0) {
          iVar3 = iVar3 + 0xfff;
        }
        param_1[0x22] = iVar3 >> 0xc;
        if (0x155 < (short)param_1[0x11]) {
          *(undefined1 *)(param_1 + 2) = 2;
switchD_80100d54_caseD_3:
          if (_DAT_80065b38 + 0x400000U < param_1[0x12]) {
            *(undefined2 *)(param_1 + 0x10) = 0;
            *(undefined2 *)((int)param_1 + 0x42) = 0xfc00;
            *(undefined2 *)(param_1 + 0x11) = 0;
            param_1[0x20] = 0xffffc466;
            param_1[0x21] = 0x7bf;
            param_1[0x22] = 0;
            param_1[0x12] = 0x4d30968;
            iVar3 = Terrain_HeightAt/*0x80025400*/(0x4018000,0x3c50000);
            param_1[0x14] = 0x3c50000;
            param_1[0x13] = iVar3 + param_1[0x21] * -900;
            *(undefined1 *)(param_1 + 2) = 3;
            FUN_8001d708/*0x8001d708*/(param_1);
switchD_80100d54_caseD_4:
            iVar3 = Terrain_HeightAt/*0x80025400*/(param_1[0x12],param_1[0x14]);
            if (iVar3 <= (int)param_1[0x13]) {
              param_1[0x21] = 0;
              *(undefined1 *)(param_1 + 2) = 4;
switchD_80100d54_caseD_5:
              uVar8 = param_1[0x20];
              param_1[0x20] = uVar8 + 0x18;
              if (-0xbec < (int)(uVar8 + 0x18)) {
                *(undefined1 *)(param_1 + 2) = 5;
switchD_80100d54_caseD_6:
                if ((int)param_1[0x12] < 0x3a44b29) {
                  *(undefined1 *)(param_1 + 2) = 6;
switchD_80100d54_caseD_7:
                  *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + -8;
                  FUN_8001d708/*0x8001d708*/(param_1);
                  iVar3 = (short)param_1[5] * 0xbeb;
                  if (iVar3 < 0) {
                    iVar3 = iVar3 + 0xfff;
                  }
                  param_1[0x20] = iVar3 >> 0xc;
                  iVar3 = (short)param_1[8] * 0xbeb;
                  if (iVar3 < 0) {
                    iVar3 = iVar3 + 0xfff;
                  }
                  param_1[0x22] = iVar3 >> 0xc;
                  if (*(short *)((int)param_1 + 0x42) < -0x7ff) {
                    *(undefined1 *)(param_1 + 2) = 7;
switchD_80100d54_caseD_8:
                    if ((int)param_1[0x14] < 0x39acb29) {
                      *(undefined1 *)(param_1 + 2) = 8;
switchD_80100d54_caseD_9:
                      *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + -8;
                      FUN_8001d708/*0x8001d708*/(param_1);
                      iVar3 = (short)param_1[5] * 0xbeb;
                      if (iVar3 < 0) {
                        iVar3 = iVar3 + 0xfff;
                      }
                      param_1[0x20] = iVar3 >> 0xc;
                      iVar3 = (short)param_1[8] * 0xbeb;
                      if (iVar3 < 0) {
                        iVar3 = iVar3 + 0xfff;
                      }
                      param_1[0x22] = iVar3 >> 0xc;
                      if (*(short *)((int)param_1 + 0x42) < -0xbff) {
                        *(undefined1 *)(param_1 + 2) = 0;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    default:
      if (param_3 == (int *)0x0) {
        return 0;
      }
      for (uVar8 = param_1[0xe]; uVar8 != 0; uVar8 = *(uint *)(uVar8 + 0x34)) {
        if (*(short *)(uVar8 + 6) == 0) {
          *(short *)(uVar8 + 0x44) = *(short *)(uVar8 + 0x44) + (short)param_3 * 0x100;
          FUN_8001d708/*0x8001d708*/(uVar8);
        }
      }
      iVar3 = FUN_800449bc/*0x800449bc*/(param_1 + 0x12);
      FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),iVar3 << 1);
      break;
    case 2:
      goto switchD_80100d54_caseD_2;
    case 3:
      goto switchD_80100d54_caseD_3;
    case 4:
      goto switchD_80100d54_caseD_4;
    case 5:
      goto switchD_80100d54_caseD_5;
    case 6:
      goto switchD_80100d54_caseD_6;
    case 7:
      goto switchD_80100d54_caseD_7;
    case 8:
      goto switchD_80100d54_caseD_8;
    case 9:
      goto switchD_80100d54_caseD_9;
    }
    break;
  case 1:
    goto switchD_80100cb4_caseD_1;
  case 2:
    goto switchD_80100cb4_caseD_2;
  case 3:
    break;
  case 4:
    goto switchD_80100cb4_caseD_4;
  default:
    goto switchD_80100cb4_caseD_5;
  case 7:
    goto switchD_80100cb4_caseD_7;
  case 8:
    goto switchD_80100cb4_caseD_8;
  }
  iVar3 = *param_3;
  if (*(short *)(param_3[1] + 2) == 0) {
LAB_801012c8:
    cVar2 = *(char *)(iVar3 + 4);
  }
  else {
    cVar2 = *(char *)(iVar3 + 4);
    if (cVar2 == '\x02') {
      FUN_8001f5a0/*0x8001f5a0*/(param_1,param_3);
      local_38 = 0x80000;
      if (param_3[5] < 0) {
        local_38 = 0xfff80000;
      }
      local_34 = 0xfffc0000;
      local_30 = 0x40000;
      GTE_RotateLongMat/*0x80043358*/(param_1 + 4,&local_38,&local_38);
      param_3 = param_3 + 5;
      GTE_RotateLongMatTrans/*0x80043408*/(param_1 + 4,param_3,param_3);
      FUN_800176f8/*0x800176f8*/(iVar3,&local_38,param_3);
      FUN_800435c0/*0x800435c0*/(iVar3 + 0x10,param_3,param_3);
      uVar5 = FUN_8004410c/*0x8004410c*/();
      FUN_800447e8/*0x800447e8*/(uVar5,*(undefined4 *)(param_1[0x16] + 8),3,param_3);
      FUN_80040234/*0x80040234*/(param_3);
      FUN_8002c6fc/*0x8002c6fc*/(iVar3,0xffffff9c,param_3,1);
      goto LAB_801012c8;
    }
  }
  if (cVar2 == '\a') {
    param_3 = (int *)(uint)*(ushort *)(iVar3 + 0xc);
switchD_80100cb4_caseD_8:
    iVar3 = FUN_80022320/*0x80022320*/(param_1,param_3);
    if (iVar3 != 0) {
      iVar3 = 0;
      *(undefined1 *)(param_1 + 2) = 0xff;
      param_1[0x20] = param_1[0x20] << 7;
      param_1[0x21] = param_1[0x21] << 7;
      param_1[0x22] = param_1[0x22] << 7;
      FUN_80020890/*0x80020890*/(param_1,300);
      do {
        iVar3 = iVar3 + 1;
        iVar4 = FUN_80017160/*0x80017160*/();
        local_28 = (iVar4 * 0xbeb >> 0xf) + -0x5f5;
        local_24 = 0xffffee1f;
        iVar4 = FUN_80017160/*0x80017160*/();
        local_20 = (iVar4 * 0xbeb >> 0xf) + -0x5f5;
        FUN_8003cee0/*0x8003cee0*/(0x7f780000,param_1 + 0x12,&local_28);
      } while (iVar3 < 3);
      FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
      *(undefined1 *)((int)param_1 + 5) = 0;
switchD_80100cb4_caseD_2:
      FUN_8003fc50/*0x8003fc50*/(param_1);
      FUN_800205f8/*0x800205f8*/(param_1);
switchD_80100cb4_caseD_7:
      FUN_8001ac44/*0x8001ac44*/(param_1,(uint)param_3 & 0xffff,0xa8,0);
switchD_80100cb4_caseD_1:
      *(undefined2 *)(param_1 + 0x27) = 0x40;
      *(undefined2 *)((int)param_1 + 0x9e) = 0x40;
      *(undefined2 *)(param_1 + 0x28) = 0x40;
      puVar6 = param_1;
      if ((char)param_1[2] != '\0') {
        param_1[0x19] = (uint)FUN_8010068c;
        *(undefined1 *)(param_1 + 2) = 0;
        *param_1 = *param_1 | 0x12a;
        FUN_80020890/*0x80020890*/(param_1,0x3c);
        puVar6 = (uint *)0x1;
      }
      *param_1 = *param_1 | 0x188;
      cVar2 = FUN_8004410c/*0x8004410c*/(puVar6);
      *(char *)((int)param_1 + 5) = cVar2;
      FUN_800443c8/*0x800443c8*/((int)cVar2,*(undefined4 *)(param_1[0x16] + 8),1,0);
      uVar5 = FUN_8001bda0/*0x8001bda0*/(param_1[0x16],0xe0);
      func_0x8003e564(param_1,uVar5);
switchD_80100cb4_caseD_4:
      FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
    }
switchD_80100cb4_caseD_5:
  }
  return 0;
}

#endif  /* GHIDRA REF */
