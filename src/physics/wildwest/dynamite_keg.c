/* dynamite_keg.c -- Wild West dynamite keg destructible + reflect.
 *
 * Source: WILDWEST.DLL  FUN_801009a8.
 *
 * Per-tick handler for breakable dynamite kegs. Every 4 frames emits
 * a spark puff (object id 0x21 from _DAT_800737d8) when "lit"
 * (obj+2 high bit set). On impact:
 *   - vehicle hit (kind 7): apply damage; if killed, mark lit
 *     (obj+2 = 0xff), reset velocity to scaled facing dir, schedule
 *     300-frame explosion timer, alarm via FX_QueueAtTarget @0x80065a18
 *     kind 9, and free SFX.
 *   - shrapnel (kind 2): test collision via func_0x8001f974 +
 *     GTE rotate; compute impact-velocity reflection (extensive 64-bit
 *     math at lines 132-156 - cross-product impulse), apply -P/8 to
 *     impactor velocity via Damage_VsImpactorAlt, spawn fragment fx
 *     at impact point.
 *   - any kind with callback at iVar6+100: invoke callback(self, 8,
 *     1000) - chained damage handler, then detach + retire if lit.
 *
 * mode dispatch:
 *   0  -- tick + spark
 *   1  -- post-spawn: pick path-piece spawn (FUN_8001b038/8001b2fc)
 *         and bind FX channel 0
 *   2  -- impact-confirmed retire
 *   4  -- silence FX
 *   5,6 -- routed targets
 *
 * MED.
 */
#include <stdint.h>

extern uint8_t Pool_AllocSFX(void);
extern void Pool_BindSnareToObject(uint8_t h, uint32_t bin, int slot, uint32_t *xyz);
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag);
extern void Object_Suspend(void);
extern int  Damage_StandardVehicle(uint32_t *self, void *imp);
extern void Damage_Apply_AgainstSelf(void *self, void *param);
extern void SFX_StopWorld(int h);
extern void FX_QueueAtTarget(uint32_t bank, int kind, uint32_t *self);
extern void SubModel_Detach(uint32_t *self);
extern void Object_RetireDeferred(uint32_t *self);
extern int  ShrapnelCollide(uint32_t *self, int *imp, uint8_t *normal); /* func_0x8001f974 */
extern void GTE_BuildLocalNormal(uint32_t *mat, uint8_t *n, uint8_t *out); /* FUN_800434f8 */
extern int64_t GTE_CrossPlanar(int *a, uint8_t *b);                   /* FUN_80017240 */
extern void Object_BounceImpulse(int imp, uint32_t *p, uint8_t *out); /* FUN_80017594 */
extern void Damage_VsImpactorAlt(int imp, int dmg, void *p, int n);
extern void GTE_RotateLongMatTrans(uint32_t *mat, uint8_t *src, uint8_t *dst);
extern void Pool_BindFXFragment(uint32_t h, uint32_t bin, int kind, uint8_t *spawnXyz); /* FUN_800447e8 */
extern void Particles_Burst(uint8_t *spawnXyz);
extern int  PathPiece_Find(uint32_t *self, uint32_t flag);             /* FUN_8001b038 */
extern uint32_t Spawner_DefferedSlot(uint32_t bin, int kind, void *p); /* FUN_800407b4 */
extern void Path_BindSpawner(uint32_t *self, int pp, uint32_t spawner);/* FUN_8001b2fc */
extern void Damage_RetireSelf(uint32_t spawner);
extern void Object_BindFinalize(uint32_t spawner);
extern uint8_t SFX_PlayWorldXY(uint32_t *posXyz);
extern void SFX_Update(int h, int posVoxel);
extern uint32_t _DAT_80065310, _DAT_800737d8, _DAT_800658fc;
extern uint8_t  DAT_80100130, DAT_80060000;
extern uint32_t FUN_80100950;

uint32_t WW_DynamiteKeg(uint32_t *self, uint32_t mode, int *imp)
{
    if (mode > 6) return 0;
    switch (mode) {
    case 0:
        if (self[0x29] != 0 && (*(uint16_t *)(self[0x29] + 0xc) & 0x100)) {
            uint8_t h = Pool_AllocSFX();
            Pool_BindSnareToObject(h, *(uint32_t *)(self[0x16] + 8), 6, self + 9);
        }
        if ((char)self[2] < 0) {
            if ((_DAT_80065310 - (uint32_t)*((uint8_t *)self + 9)) & 3) return 0;
            uint32_t *spark = (uint32_t *)Object_SpawnFromBank(_DAT_800737d8, 0x21, 0x80, 8);
            int16_t  cnt   = (int16_t)_DAT_80065310;
            *spark |= 0x4b4u;
            spark[0x12] = self[9]; spark[0x13] = self[10]; spark[0x14] = self[0xb];
            *(int16_t *)(spark + 0x11) = (int16_t)(cnt * 0x60);
            spark[0x19] = (uint32_t)(uintptr_t)&FUN_80100950;
            Object_Suspend();
        }
        if (imp == NULL) return 0;
        SFX_Update((int)*((char *)self + 5), SFX_PlayWorldXY(self + 9));
        break;
    case 1: goto path_bind;
    case 2: goto retire;
    case 4: goto silence_fx;
    case 5: return 0;
    case 6: goto path_bind;
    }

    int      impObj = *imp;
    uint8_t  k      = *(uint8_t *)(impObj + 4);
    uint8_t  want   = 2;
    if (k == 7) {
        if ((char)self[2] < 0) return 0;
        if (Damage_StandardVehicle(self, (void *)(intptr_t)*(uint16_t *)(impObj + 0xc)) == 0) return 0;
        int vx = (int)*(int16_t *)(self + 5) * self[0x2a];
        self[0x29] = 0;
        *(uint8_t *)(self + 2) = 0xff;
        *self &= ~0x100u;
        if (vx < 0) vx += 0x1f;
        self[0x20] = vx >> 5;
        self[0x21] = 0;
        int vz = (int)*(int16_t *)(self + 8) * self[0x2a];
        if (vz < 0) vz += 0x1f;
        self[0x22] = vz >> 5;
        Damage_Apply_AgainstSelf(self, (void *)(intptr_t)300);
        self[0x25] = 30000;
        SFX_StopWorld((int)*((char *)self + 5));
        *((char *)self + 5) = 0;
        Damage_Apply_AgainstSelf(self, (void *)(intptr_t)300);
        FX_QueueAtTarget(0x80065a18u, 9, self);
        want = 0;
    }
    if (k == want) {
        uint8_t hit[20], dn[12], xy[8], wpos[16];
        ShrapnelCollide(self, imp, hit);
        GTE_BuildLocalNormal(self + 4, xy, hit + 8);
        if (*(int16_t *)(hit + 12) >= 0x801) return 0;
        int vx = (int)*(int16_t *)(self + 5) * self[0x2a];
        if (vx < 0) vx += 0x1f;
        int local[3];
        local[0] = *(int *)(impObj + 0x80) - (vx >> 5);
        local[1] = *(int *)(impObj + 0x84);
        int vz = (int)*(int16_t *)(self + 8) * self[0x2a];
        if (vz < 0) vz += 0x1f;
        local[2] = *(int *)(impObj + 0x88) - (vz >> 5);
        int64_t r = GTE_CrossPlanar(local, xy);
        int     hi = (int)(r >> 32);
        uint32_t p = (uint32_t)((uint32_t)r >> 11 | (uint32_t)hi << 21);
        if ((int)p >= 0) return 0;
        Object_BounceImpulse(impObj, (uint32_t *)&p, dn);
        Damage_VsImpactorAlt(impObj, (int)(p + 0x1fffu) >> 13, dn, 1);
        GTE_RotateLongMatTrans((uint32_t *)(impObj + 0x10), dn, wpos);
        uint32_t h = Pool_AllocSFX();
        Pool_BindFXFragment(h, _DAT_800658fc, 5, wpos);
        Particles_Burst(wpos);
    }
    if (*(void (**)(int, int, int))(impObj + 100) != NULL) {
        (*(void (**)(int, int, int))(impObj + 100))(impObj, 8, 1000);
retire:
        if ((char)self[2] < 0) {
            SubModel_Detach(self);
            Object_RetireDeferred(self);
        }
        *(uint8_t *)(self + 2) = 0xff;
        SubModel_Detach(self);
        SFX_StopWorld((int)*((char *)self + 5));
        *((char *)self + 5) = 0;
        Damage_Apply_AgainstSelf(self, (void *)(intptr_t)300);
path_bind: {
            int pp = PathPiece_Find(self, 0x8000);
            if (pp != 0) {
                uint32_t sp = Spawner_DefferedSlot(_DAT_800737d8, 6, &DAT_80100130);
                Path_BindSpawner(self, pp, sp);
                Damage_RetireSelf(sp);
                if ((*self & 4) == 0) Object_BindFinalize(sp);
                char hh = (char)Pool_AllocSFX();
                *((char *)self + 5) = hh;
                Pool_BindSnareToObject((uint8_t)hh, *(uint32_t *)(self[0x16] + 8), 0, NULL);
silence_fx:
                SFX_StopWorld((int)*((char *)self + 5));
            }
        }
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

/* --- WILDWEST.DLL FUN_801009a8  (from analysis/dll/WILDWEST/decomp/801009a8.c) --- */
// addr: 0x801009a8  name: FUN_801009a8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801009a8(uint *param_1,uint param_2,int *param_3)

{
  longlong lVar1;
  short sVar2;
  char cVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  uint uVar8;
  uint extraout_v1;
  uint *puVar9;
  undefined8 uVar10;
  undefined1 auStack_80 [20];
  undefined1 auStack_6c [12];
  undefined1 auStack_60 [8];
  ushort local_58;
  ushort local_56;
  ushort local_54;
  int local_50;
  undefined1 auStack_48 [4];
  short local_44;
  int local_40;
  undefined4 local_3c;
  int local_38;
  uint local_30;
  uint local_2c;
  uint local_28;
  undefined1 auStack_20 [16];
  
  if (6 < param_2) {
    return 0;
  }
  switch(param_2) {
  case 0:
    if ((((&switchD_801009e4::switchdataD_80100140)[param_2] != (undefined *)0x0) &&
        (param_1[0x29] != 0)) && ((*(ushort *)(param_1[0x29] + 0xc) & 0x100) != 0)) {
      uVar7 = FUN_8004410c/*0x8004410c*/(param_1);
      FUN_800447e8/*0x800447e8*/(uVar7,*(undefined4 *)(param_1[0x16] + 8),6,param_1 + 9);
    }
    if ((char)param_1[2] < '\0') {
      param_3 = (int *)&DAT_80060000;
      if ((_DAT_80065310 - (uint)*(byte *)((int)param_1 + 9) & 3) != 0) {
        return 0;
      }
      puVar9 = (uint *)FUN_8001ac44/*0x8001ac44*/(_DAT_800737d8,0x21,0x80,8);
      sVar2 = (short)_DAT_80065310;
      *puVar9 = *puVar9 | 0x4b4;
      uVar8 = param_1[10];
      uVar4 = param_1[0xb];
      puVar9[0x12] = param_1[9];
      puVar9[0x13] = uVar8;
      puVar9[0x14] = uVar4;
      *(short *)(puVar9 + 0x11) = sVar2 * 0x60;
      puVar9[0x19] = (uint)FUN_80100950;
      FUN_8002036c/*0x8002036c*/();
    }
    if (param_3 == (int *)0x0) {
      return 0;
    }
    uVar7 = FUN_800449bc/*0x800449bc*/(param_1 + 9);
    FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar7);
    break;
  case 1:
    goto switchD_801009e4_caseD_1;
  case 2:
    goto switchD_801009e4_caseD_2;
  case 4:
    goto switchD_801009e4_caseD_4;
  case 5:
    goto switchD_801009e4_caseD_5;
  case 6:
    goto switchD_801009e4_caseD_6;
  }
  iVar6 = *param_3;
  uVar8 = (uint)*(byte *)(iVar6 + 4);
  uVar4 = 2;
  if (uVar8 == 7) {
    if ((char)param_1[2] < '\0') {
      return 0;
    }
    iVar5 = FUN_80022320/*0x80022320*/(param_1,*(undefined2 *)(iVar6 + 0xc));
    if (iVar5 == 0) {
      return 0;
    }
    iVar5 = (int)(short)param_1[5] * param_1[0x2a];
    param_1[0x29] = 0;
    *(undefined1 *)(param_1 + 2) = 0xff;
    *param_1 = *param_1 & 0xfffffeff;
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0x1f;
    }
    param_1[0x20] = iVar5 >> 5;
    param_1[0x21] = 0;
    iVar5 = (int)(short)param_1[8] * param_1[0x2a];
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0x1f;
    }
    param_1[0x22] = iVar5 >> 5;
    FUN_80020890/*0x80020890*/(param_1,300);
    param_1[0x25] = 30000;
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
    *(undefined1 *)((int)param_1 + 5) = 0;
    FUN_80020890/*0x80020890*/(param_1,300);
    FUN_8002002c/*0x8002002c*/(0x80065a18,9,param_1);
    uVar4 = 0;
    uVar8 = extraout_v1;
  }
  if (uVar8 == uVar4) {
    func_0x8001f974(param_1,param_3,auStack_80);
    FUN_800434f8/*0x800434f8*/(param_1 + 4,auStack_60,auStack_48);
    if (local_44 < 0x801) {
      return 0;
    }
    iVar5 = (int)(short)param_1[5] * param_1[0x2a];
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0x1f;
    }
    local_40 = *(int *)(iVar6 + 0x80) - (iVar5 >> 5);
    local_3c = *(undefined4 *)(iVar6 + 0x84);
    iVar5 = (int)(short)param_1[8] * param_1[0x2a];
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0x1f;
    }
    local_38 = *(int *)(iVar6 + 0x88) - (iVar5 >> 5);
    uVar10 = FUN_80017240/*0x80017240*/(&local_40,auStack_60);
    param_1 = (uint *)((uint)uVar10 >> 0xb | (int)((ulonglong)uVar10 >> 0x20) << 0x15);
    if (-1 < (int)param_1) {
      return 0;
    }
    uVar4 = (int)((uint)local_58 << 0x10) >> 0x10;
    uVar8 = -((int)param_1 + local_50);
    iVar5 = -(uint)(uVar8 != 0) - ((int)param_1 + local_50 >> 0x1f);
    lVar1 = (ulonglong)uVar4 * (ulonglong)uVar8;
    local_30 = (uint)lVar1 >> 0xc |
               ((int)((ulonglong)lVar1 >> 0x20) + uVar4 * iVar5 +
               uVar8 * ((int)((uint)local_58 << 0x10) >> 0x1f)) * 0x100000;
    uVar4 = (int)((uint)local_56 << 0x10) >> 0x10;
    lVar1 = (ulonglong)uVar4 * (ulonglong)uVar8;
    local_2c = (uint)lVar1 >> 0xc |
               ((int)((ulonglong)lVar1 >> 0x20) + uVar4 * iVar5 +
               uVar8 * ((int)((uint)local_56 << 0x10) >> 0x1f)) * 0x100000;
    uVar4 = (int)((uint)local_54 << 0x10) >> 0x10;
    lVar1 = (ulonglong)uVar4 * (ulonglong)uVar8;
    local_28 = (uint)lVar1 >> 0xc |
               ((int)((ulonglong)lVar1 >> 0x20) + uVar4 * iVar5 +
               uVar8 * ((int)((uint)local_54 << 0x10) >> 0x1f)) * 0x100000;
    FUN_80017594/*0x80017594*/(iVar6,&local_30,auStack_6c);
    FUN_8002c958/*0x8002c958*/(iVar6,(int)((int)param_1 + 0x1fffU) >> 0xd,auStack_6c,1);
    GTE_RotateLongMatTrans/*0x80043408*/(iVar6 + 0x10,auStack_6c,auStack_20);
    uVar7 = FUN_8004410c/*0x8004410c*/();
    FUN_800447e8/*0x800447e8*/(uVar7,_DAT_800658fc,5,auStack_20);
    FUN_80040234/*0x80040234*/(auStack_20);
  }
  if (*(code **)(iVar6 + 100) != (code *)0x0) {
    (**(code **)(iVar6 + 100))(iVar6,8,1000);
switchD_801009e4_caseD_2:
    puVar9 = param_1;
    if ((char)param_1[2] < '\0') {
      FUN_8003fc50/*0x8003fc50*/(param_1);
      FUN_800205f8/*0x800205f8*/(param_1);
      puVar9 = (uint *)0x1;
    }
    *(undefined1 *)(param_1 + 2) = 0xff;
    FUN_8003fc50/*0x8003fc50*/(puVar9);
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
    *(undefined1 *)((int)param_1 + 5) = 0;
    FUN_80020890/*0x80020890*/(param_1,300);
switchD_801009e4_caseD_6:
switchD_801009e4_caseD_1:
    iVar6 = FUN_8001b038/*0x8001b038*/(param_1,0x8000);
    if (iVar6 != 0) {
      uVar7 = FUN_800407b4/*0x800407b4*/(_DAT_800737d8,6,&DAT_80100130);
      FUN_8001b2fc/*0x8001b2fc*/(param_1,iVar6,uVar7);
      FUN_80020744/*0x80020744*/(uVar7);
      if ((*param_1 & 4) == 0) {
        FUN_800207c4/*0x800207c4*/(uVar7);
      }
      cVar3 = FUN_8004410c/*0x8004410c*/();
      *(char *)((int)param_1 + 5) = cVar3;
      FUN_800443c8/*0x800443c8*/((int)cVar3,*(undefined4 *)(param_1[0x16] + 8),0,0);
switchD_801009e4_caseD_4:
      FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
    }
  }
switchD_801009e4_caseD_5:
  return 0;
}

#endif  /* GHIDRA REF */
