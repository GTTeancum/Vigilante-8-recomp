/* missile_track.c -- Secret Base homing missile.
 *
 * Source: SCRTBASE.DLL  FUN_801010f4.
 *
 * Tracks the locked target stored at self+0x21 (set by the turret
 * tracker), turning toward it with bounded yaw/pitch rate and
 * accelerating to a 0x3b9a/4096 terminal velocity scaled by facing.
 * Spawns a smoke puff (object id 0x13 with tick FUN_8010100c) every
 * 4 frames. On terrain hit or self-timer (>0x259, or >0x3c with
 * planar distance < 0xfa000), explodes at terrain-projected world
 * position via FUN_8003fdcc (debris pool) and apply damage; also
 * spawns particles (kind 0x29, lifetime 300) and a debris cloud
 * (FUN_8001d470 at radius 0x1484 with tick FUN_80100ca0).
 *
 * mode dispatch:
 *   0  -- per-tick integrate + steer
 *   1  -- post-spawn FX bind
 *   2  -- already-explosion path
 *   3  -- explode (terrain hit)
 *   4  -- silent retire
 *
 * MED.
 */
#include <stdint.h>

extern int  Terrain_HeightAt(int32_t x, int32_t z);
extern uint8_t Pool_AllocSFX(void);
extern void Pool_BindFXOnObject(uint8_t h, uint32_t bin, int slot, int aux);
extern void Pool_LaunchProjectile(uint8_t h, uint32_t bin, int kind, uint32_t *xyz);
extern uint8_t SFX_PlayWorldXY(uint32_t *xyz);
extern void SFX_Update(int h, int posVoxel);
extern void SFX_StopWorld(int h);
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag);
extern void Object_Suspend(void);
extern void GTE_RotateLongMatTrans(uint32_t *mat, int *src, int *dst);
extern void GTE_RotateLongMtxLow(uint32_t *mat, int *src, int *dst);
extern int  ratan2(int dz, int dx);
extern void Object_RefitMatYaw(uint32_t *mat, int yaw, int pitch, int roll); /* FUN_800439b8 */
extern void Util_TransposeMatRotate(uint32_t mat, int *src);
extern void MatrixNormal(uint32_t *m, uint32_t *out);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t a);
extern void Object_RetireDeferred(uint32_t *self);
extern void Damage_Apply_AgainstSelf(void *self, void *param);
extern uint32_t *Debris_PoolAlloc(int *spawnXyz, int kind, int life);   /* FUN_8003fdcc */
extern void Debris_AttachTrack(int *spawnXyz, int p);                   /* FUN_8003fd24 */
extern uint32_t Object_AllocCloud(int rad);                              /* FUN_8001d470 */
extern uint32_t _DAT_800658fc, _DAT_80065310, _DAT_800737d8;
extern uint32_t FUN_8010100c, FUN_80100ca0;

uint32_t SB_HomingMissile(uint32_t *self, uint32_t mode, int param3)
{
    uint32_t *t;
    switch (mode) {
    case 0: break;
    case 1: {
        char h = (char)Pool_AllocSFX();
        *((char *)self + 5) = h;
        Pool_BindFXOnObject((uint8_t)h, *(uint32_t *)(self[0x16] + 8), 3, 0);
        return 0;
    }
    case 2: goto fx_bind;
    case 3: goto explode;
    case 4: goto silence;
    default: return 0;
    }

    self[0x12] += self[0x22];
    self[0x13] += self[0x23];
    self[0x14] += self[0x24];
    self[9]  = self[0x12];
    self[10] = self[0x13];
    self[0xb]= self[0x14];
    if (param3 != 0) SFX_Update((int)*((char *)self + 5), SFX_PlayWorldXY(self + 0x12));

    uint16_t age = (uint16_t)self[0x25] + 1;
    *(uint16_t *)(self + 0x25) = age;
    if ((age & 3) == 0) {
        uint32_t *puff = (uint32_t *)Object_SpawnFromBank(_DAT_800737d8, 0x13, 0x98, 8);
        *(uint8_t *)(puff + 1) = 4;
        *puff = 0x4b4u;
        puff[0x12] = self[0x12]; puff[0x13] = self[0x13]; puff[0x14] = self[0x14];
        puff[0x19] = (uint32_t)(uintptr_t)&FUN_8010100c;
        *(int16_t *)(puff + 0x11) = (int16_t)(self[0x25] * 0x60);
        puff[0x22] = -(int)*(int16_t *)(self + 5);
        puff[0x23] = -(int)*(int16_t *)((char *)self + 0x1a);
        puff[0x24] = -(int)*(int16_t *)(self + 8);
        Object_Suspend();
    }

    if ((int16_t)self[0x25] < 0xf0) {
        int s5 = (int)*(int16_t *)(self + 5) * 0x1c; if (s5 < 0) s5 += 0xfff;
        self[0x22] += s5 >> 12;
        int s7 = (int)*(int16_t *)((char *)self + 0x1a) * 0x1c; if (s7 < 0) s7 += 0xfff;
        self[0x23] += s7 >> 12;
        int s8 = (int)*(int16_t *)(self + 8) * 0x1c; if (s8 < 0) s8 += 0xfff;
        self[0x24] += s8 >> 12;
    }

    uint32_t hitbox = self[0x17];
    int *trg = (int *)self[0x21];
    int local[3];
    if (trg == NULL) {
        local[0] = 0; local[1] = 0x1000; local[2] = 0;
    } else if (*(int16_t *)(trg + 3) == 0) {
        self[0x21] = 0;
    }
    int *tpos = trg + 0x12;
    if ((char)self[2] != 0) Util_TransposeMatRotate((uint32_t)(intptr_t)(self + 4), tpos);
    local[0] = tpos[0] - (int)self[0x12];
    local[1] = 0;
    local[2] = tpos[2] - (int)self[0x14];
    GTE_RotateLongMtxLow(self + 4, local, local);

    int yaw = ratan2(-local[1], local[2]);
    int yc  = (yaw < -0x40) ? -0x40 : (yaw > 0x40) ? 0x40 : yaw;
    int pit = ratan2(local[0], local[2]);
    int pc  = (pit < -0x40) ? -0x40 : (pit > 0x40) ? 0x40 : pit;
    Object_RefitMatYaw(self + 4, yc, pc, 0);
    if (((_DAT_80065310 - (uint32_t)*((uint8_t *)self + 9)) & 0x1f) == 0)
        MatrixNormal(self + 4, self + 4);

    /* Accelerate. */
    for (int i = 0; i < 3; i++) {
        static const int axisOfs[3] = {5, 0x1a/4, 8};
        int s = ((int)*(int16_t *)((char *)self + axisOfs[i] * 4) * 0x3b9a);
        if (s < 0) s += 0xfff;
        int d = (s >> 12) - (int)self[0x22 + i];
        if (d < 0) d += 0xf;
        d >>= 4;
        if (d < -0x100) d = -0x100;
        if (d >  0x100) d =  0x100;
        self[0x22 + i] += d;
    }

    /* Lifetime / proximity check. */
    int hit = 0;
    if ((int16_t)self[0x25] < 0x259) {
        if ((int16_t)self[0x25] > 0x3c) {
            int dz = local[2] < 0 ? -local[2] : local[2];
            int dx = local[0] < 0 ? -local[0] : local[0];
            if (dz < dx) dz = dx;
            if (dz < 0xfa000) hit = 1;
        }
    } else hit = 1;
    if (hit) *(uint8_t *)(self + 2) = 1;

    /* Terrain hit check. */
    {
        int mid[3];
        mid[0] = (*(int *)(hitbox + 4)  + *(int *)(hitbox + 0x10)) / 2;
        mid[1] = (*(int *)(hitbox + 8)  + *(int *)(hitbox + 0x14)) / 2;
        mid[2] = *(int *)(hitbox + 0x18);
        GTE_RotateLongMatTrans(self + 4, mid, mid);
        int gy = Terrain_HeightAt(mid[0], mid[2]);
        if (mid[1] < gy) return 0;
    }
explode: {
        uint32_t hb = self[0x17];
        int mid[4];
        mid[0] = (*(int *)(hb + 4) + *(int *)(hb + 0x10)) / 2;
        mid[1] = (*(int *)(hb + 8) + *(int *)(hb + 0x14)) / 2;
        mid[3] = *(int *)(hb + 0x18);
        GTE_RotateLongMatTrans(self + 4, mid, mid);
        mid[1] = Terrain_HeightAt(mid[0], mid[3]);
        uint32_t *deb = Debris_PoolAlloc(mid, 0x29, 300);
        *deb |= 0x10u;
        Debris_AttachTrack(mid, 0);
        uint8_t fh = Pool_AllocSFX();
        Pool_LaunchProjectile(fh, _DAT_800658fc, 0x38, (uint32_t *)mid);
        *self |= 0x22u;
        Damage_Apply_AgainstSelf(self, (void *)(intptr_t)0xf);
        Damage_AccumulateOrFire(self, 0);
        uint32_t *cloud = (uint32_t *)Object_AllocCloud(0x1484);
        cloud[0x19] = (uint32_t)(uintptr_t)&FUN_80100ca0;
        cloud[0x12] = mid[0]; cloud[0x13] = mid[1]; cloud[0x14] = mid[3];
        *cloud = 0xa0u;
        Object_Suspend();
    }
fx_bind: {
        uint8_t f2 = Pool_AllocSFX();
        Pool_LaunchProjectile(f2, _DAT_800658fc, 0x39, self + 9);
        Object_RetireDeferred(self);
silence:
        SFX_StopWorld((int)*((char *)self + 5));
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

/* --- SCRTBASE.DLL FUN_801010f4  (from analysis/dll/SCRTBASE/decomp/801010f4.c) --- */
// addr: 0x801010f4  name: FUN_801010f4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801010f4(uint *param_1,undefined4 param_2,int param_3)

{
  char cVar1;
  ushort uVar2;
  int iVar3;
  uint *puVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  uint uVar7;
  int iVar8;
  int *piVar9;
  int iVar10;
  int *piVar11;
  int iVar12;
  uint uVar13;
  int local_38;
  int local_34;
  int local_30;
  int local_28;
  int local_24;
  undefined4 local_20;
  
  puVar4 = param_1;
  switch(param_2) {
  case 0:
    break;
  case 2:
switchD_80101130_caseD_2:
    uVar5 = FUN_8004410c/*0x8004410c*/(puVar4);
    FUN_8004483c/*0x8004483c*/(uVar5,_DAT_800658fc,0x39,param_1 + 9);
    FUN_800205f8/*0x800205f8*/(param_1);
    puVar4 = (uint *)0x1;
  case 1:
    cVar1 = FUN_8004410c/*0x8004410c*/(puVar4);
    *(char *)((int)param_1 + 5) = cVar1;
    FUN_800443c8/*0x800443c8*/((int)cVar1,*(undefined4 *)(param_1[0x16] + 8),3,0);
  case 4:
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
  default:
    return 0;
  case 3:
    goto switchD_80101130_caseD_3;
  }
  param_1[0x12] = param_1[0x12] + param_1[0x22];
  param_1[0x13] = param_1[0x13] + param_1[0x23];
  param_1[0x14] = param_1[0x14] + param_1[0x24];
  param_1[9] = param_1[0x12];
  param_1[10] = param_1[0x13];
  param_1[0xb] = param_1[0x14];
  if (param_3 != 0) {
    uVar5 = FUN_800446dc/*0x800446dc*/(param_1 + 0x12);
    FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar5);
  }
  uVar2 = (short)param_1[0x25] + 1;
  *(ushort *)(param_1 + 0x25) = uVar2;
  if ((uVar2 & 3) == 0) {
    puVar6 = (undefined4 *)FUN_8001ac44/*0x8001ac44*/(_DAT_800737d8,0x13,0x98,8);
    *(undefined1 *)(puVar6 + 1) = 4;
    *puVar6 = 0x4b4;
    uVar7 = param_1[0x13];
    uVar13 = param_1[0x14];
    puVar6[0x12] = param_1[0x12];
    puVar6[0x13] = uVar7;
    puVar6[0x14] = uVar13;
    puVar6[0x19] = FUN_8010100c;
    *(short *)(puVar6 + 0x11) = (short)param_1[0x25] * 0x60;
    puVar6[0x22] = -(int)(short)param_1[5];
    puVar6[0x23] = -(int)*(short *)((int)param_1 + 0x1a);
    puVar6[0x24] = -(int)(short)param_1[8];
    FUN_8002036c/*0x8002036c*/();
  }
  if ((short)param_1[0x25] < 0xf0) {
    iVar8 = (short)param_1[5] * 0x1c;
    if (iVar8 < 0) {
      iVar8 = iVar8 + 0xfff;
    }
    iVar10 = *(short *)((int)param_1 + 0x1a) * 0x1c;
    param_1[0x22] = param_1[0x22] + (iVar8 >> 0xc);
    if (iVar10 < 0) {
      iVar10 = iVar10 + 0xfff;
    }
    iVar8 = (short)param_1[8] * 0x1c;
    param_1[0x23] = param_1[0x23] + (iVar10 >> 0xc);
    if (iVar8 < 0) {
      iVar8 = iVar8 + 0xfff;
    }
    param_1[0x24] = param_1[0x24] + (iVar8 >> 0xc);
  }
  uVar7 = param_1[0x17];
  piVar11 = (int *)param_1[0x21];
  if (piVar11 == (int *)0x0) {
    local_38 = 0;
    local_34 = 0x1000;
    local_30 = 0;
  }
  if ((short)piVar11[3] == 0) {
    param_1[0x21] = 0;
  }
  piVar9 = piVar11 + 0x12;
  if ((char)param_1[2] != '\0') {
    piVar11 = &local_38;
    FUN_800435c0/*0x800435c0*/(param_1 + 4,piVar9);
  }
  local_38 = piVar11[0x12] - param_1[0x12];
  local_34 = 0;
  local_30 = piVar11[0x14] - param_1[0x14];
  GTE_RotateLongMtxLow/*0x8004352c*/(param_1 + 4,&local_38,&local_38);
  iVar10 = ratan2/*0x8004ecd4*/(-local_34,local_30);
  iVar8 = -0x40;
  if ((-0x41 < iVar10) && (iVar8 = 0x40, iVar10 < 0x41)) {
    iVar8 = iVar10;
  }
  iVar3 = ratan2/*0x8004ecd4*/(local_38,local_30);
  iVar10 = -0x40;
  if ((iVar3 < -0x40) || (iVar10 = iVar3, iVar12 = 0x40, iVar3 < 0x41)) {
    iVar12 = iVar10;
  }
  FUN_800439b8/*0x800439b8*/(param_1 + 4,iVar8,iVar12,0);
  if ((_DAT_80065310 - (uint)*(byte *)((int)param_1 + 9) & 0x1f) == 0) {
    MatrixNormal/*0x8004c934*/(param_1 + 4,param_1 + 4);
  }
  iVar8 = (short)param_1[5] * 0x3b9a;
  if (iVar8 < 0) {
    iVar8 = iVar8 + 0xfff;
  }
  iVar8 = (iVar8 >> 0xc) - param_1[0x22];
  if (iVar8 < 0) {
    iVar8 = iVar8 + 0xf;
  }
  iVar8 = iVar8 >> 4;
  iVar10 = -0x100;
  if ((-0x101 < iVar8) && (iVar10 = 0x100, iVar8 < 0x101)) {
    iVar10 = iVar8;
  }
  iVar8 = *(short *)((int)param_1 + 0x1a) * 0x3b9a;
  param_1[0x22] = param_1[0x22] + iVar10;
  if (iVar8 < 0) {
    iVar8 = iVar8 + 0xfff;
  }
  iVar8 = (iVar8 >> 0xc) - param_1[0x23];
  if (iVar8 < 0) {
    iVar8 = iVar8 + 0xf;
  }
  iVar8 = iVar8 >> 4;
  iVar10 = -0x100;
  if ((-0x101 < iVar8) && (iVar10 = 0x100, iVar8 < 0x101)) {
    iVar10 = iVar8;
  }
  iVar8 = (short)param_1[8] * 0x3b9a;
  param_1[0x23] = param_1[0x23] + iVar10;
  if (iVar8 < 0) {
    iVar8 = iVar8 + 0xfff;
  }
  iVar8 = (iVar8 >> 0xc) - param_1[0x24];
  if (iVar8 < 0) {
    iVar8 = iVar8 + 0xf;
  }
  iVar8 = iVar8 >> 4;
  iVar10 = -0x100;
  if ((-0x101 < iVar8) && (iVar10 = 0x100, iVar8 < 0x101)) {
    iVar10 = iVar8;
  }
  param_1[0x24] = param_1[0x24] + iVar10;
  if ((short)param_1[0x25] < 0x259) {
    if (0x3c < (short)param_1[0x25]) {
      if (local_30 < 0) {
        local_30 = -local_30;
      }
      if (local_38 < 0) {
        local_38 = -local_38;
      }
      if (local_30 < local_38) {
        local_30 = local_38;
      }
      if (local_30 < 0xfa000) goto LAB_8010160c;
    }
  }
  else {
LAB_8010160c:
    *(undefined1 *)(param_1 + 2) = 1;
  }
  local_38 = (*(int *)(uVar7 + 4) + *(int *)(uVar7 + 0x10)) / 2;
  local_34 = (*(int *)(uVar7 + 8) + *(int *)(uVar7 + 0x14)) / 2;
  local_30 = *(undefined4 *)(uVar7 + 0x18);
  GTE_RotateLongMatTrans/*0x80043408*/(param_1 + 4,&local_38,&local_38);
  iVar8 = Terrain_HeightAt/*0x80025400*/(local_38,local_30);
  if (local_34 < iVar8) {
    return 0;
  }
switchD_80101130_caseD_3:
  uVar7 = param_1[0x17];
  local_28 = (*(int *)(uVar7 + 4) + *(int *)(uVar7 + 0x10)) / 2;
  local_24 = (*(int *)(uVar7 + 8) + *(int *)(uVar7 + 0x14)) / 2;
  local_20 = *(undefined4 *)(uVar7 + 0x18);
  GTE_RotateLongMatTrans/*0x80043408*/(param_1 + 4,&local_28,&local_28);
  local_24 = Terrain_HeightAt/*0x80025400*/(local_28,local_20);
  puVar4 = (uint *)FUN_8003fdcc/*0x8003fdcc*/(&local_28,0x29,300);
  *puVar4 = *puVar4 | 0x10;
  FUN_8003fd24/*0x8003fd24*/(&local_28,0);
  uVar5 = FUN_8004410c/*0x8004410c*/();
  FUN_8004483c/*0x8004483c*/(uVar5,_DAT_800658fc,0x38,&local_28);
  *param_1 = *param_1 | 0x22;
  FUN_80020890/*0x80020890*/(param_1,0xf);
  FUN_80020778/*0x80020778*/(param_1);
  puVar6 = (undefined4 *)FUN_8001d470/*0x8001d470*/(0x1484);
  puVar6[0x19] = FUN_80100ca0;
  puVar6[0x12] = local_28;
  puVar6[0x13] = local_24;
  puVar6[0x14] = local_20;
  *puVar6 = 0xa0;
  FUN_8002036c/*0x8002036c*/();
  puVar4 = (uint *)0x1;
  goto switchD_80101130_caseD_2;
}

#endif  /* GHIDRA REF */
