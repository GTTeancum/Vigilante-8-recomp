/* silo_slide.c -- Valley Farms tornado silo slide/teleport.
 *
 * Source: VALLYFRM.DLL  FUN_80100eb4.
 *
 * The silo physics: it drifts along its rail (Z velocity -0x1dcd per
 * frame in active substates), teleports vehicles caught in its base,
 * and emits chaff debris every 4 frames during travel.
 *
 * substate (self+2):
 *   0  -- approach: drift -Z. On entering "boundary" zones at certain
 *         Z ranges, swap object position to a snapshot point and
 *         spawn 2 chaff sub-instances (FUN_80021c6c) facing 0x400 and
 *         0xfc00. If Z drops below 0x4a90000, decrement DAT_80101308
 *         and enter substate 1.
 *   1  -- ride: drift -Z. If Z falls below 0x33e0000, push self back
 *         +0x1dcd then RetireDeferred if past threshold.
 *   2  -- terminal: same as substate 0 with X==0x4d30000 trigger
 *         transition.
 *
 * Every 4 ticks, emits a chaff puff (object id 0x1f from bank
 * _DAT_800737d8) with random X velocity (4.12-fixed) and Y velocity
 * -0x14000.
 *
 * mode dispatch:
 *   0  -- tick (above)
 *   1  -- post-spawn init + bind FX slot 3
 *   3  -- impact event (kind==2): substate-dependent damage push to
 *         impactor velocity vector with random sub-jitter; every 32nd
 *         impact triggers Damage_VsImpactor_Burst.
 *   4  -- retire (kill FX channel)
 *
 * MED.
 */
#include <stdint.h>

extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
extern void Object_RetireDeferred(uint32_t *self);
extern int  RandChaff(uint32_t *self);                              /* func_0x80021c6c */
extern void Object_Suspend(void);
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag);
extern int  Rand255(void);
extern uint32_t FUN_80100e70(int self);
extern void Object_BumpSubstate_Or_FX(uint32_t *self);              /* FUN_8001d4f0 */
extern uint8_t SFX_PlayWorldXY(uint32_t *posXyz);
extern void SFX_Update(int h, int posVoxel);
extern void Damage_VsImpactor_Burst(int imp);                        /* FUN_8002c4bc */
extern uint32_t Pool_AllocProjectile(void);
extern void Pool_LaunchProjectile(uint32_t h, uint32_t bin, int kind, uint32_t *xyz);
extern uint8_t Pool_AllocSFX(void);
extern void Pool_BindFXOnObject(uint32_t h, uint32_t bin, int slot, int aux);
extern void SFX_StopWorld(int h);
extern uint32_t _DAT_80065310, _DAT_800737d8;
extern int32_t  DAT_80101308;

uint32_t VF_SiloSlide(uint32_t *self, int mode, int *arg)
{
    uint32_t kind = 1;
    if (mode == 1) goto bind_fx;
    if (mode == 0 || mode == 3 || mode != 4) {
        if (mode == 0 || (mode != 3 && mode != 4)) {
            uint8_t sub = *(uint8_t *)(self + 2);
            if (sub == 1) {
ride:
                self[9] -= 0x1dcd;
                int z = (int)self[9];
                if (z < 0x33e0000) {
                    int back = self[9] + 0x1dcd;
                    self[9] = back;
                    if (z < back) Object_RetireDeferred(self);
                }
            } else {
                uint32_t target = 2;
                if (sub > 1 || (target = 0x4d30000, sub != 0)) {
                    if (sub == target) { target = 0x3920000; goto edge; }
                }
                /* boundary check */
                int ozb = (int)self[0xb];
                int zb  = ozb - 0x1dcd;
                self[0xb] = zb;
                if ((zb <= (int)(target | 0xffff) && ozb > 0x4d40000)
                    || (zb < 0x4b50000 && ozb > 0x4b50000)) {
                    self[0x12] = self[9]; self[0x13] = self[10]; self[0x14] = self[0xb];
                    uint32_t *c1 = (uint32_t *)(intptr_t)RandChaff(self);
                    *(uint8_t *)(c1 + 8) = 1;
                    *(uint16_t *)((char *)c1 + 0x42) = 0x400;
                    Object_Suspend();
                    uint32_t *c2 = (uint32_t *)(intptr_t)RandChaff(self);
                    *(uint8_t *)(c2 + 8) = 2;
                    *(uint16_t *)((char *)c2 + 0x42) = 0xfc00u;
                    Object_Suspend();
                }
edge:
                if ((int)self[0xb] < 0x4a90000) { DAT_80101308--; goto ride; }
            }
            if ((_DAT_80065310 - (uint32_t)*((uint8_t *)self + 9)) & 3) {
                ;
            } else {
                uint32_t *p = (uint32_t *)Object_SpawnFromBank(_DAT_800737d8, 0x1f, 0x80, 8);
                *p = 0x10u;
                int r = Rand255();
                p[9]  = ((uint32_t)(r & 0xff) - 0x80) * 0x400;
                p[10] = 0;
                p[0xb] = 0xfffec000u;
                Object_SetCallbackPsxSlot(p, (uintptr_t)&FUN_80100e70);
                Object_BumpSubstate_Or_FX(self);
            }
            if (arg == NULL) return 0;
            SFX_Update((int)*((char *)self + 5), SFX_PlayWorldXY(self + 9));
        }
        if (mode == 3 || mode != 4) {
            int imp = *arg;
            if (*(char *)(imp + 4) != 2) return 0;
            uint8_t sub = *(uint8_t *)(self + 2);
            uint32_t mag;
            int jitter = 1;
            if (sub == kind) { mag = 0xfffecf00u; }
            else {
                if ((int)sub < 2) {
                    mag = (sub != 0) ? 0xfffecf00u : 0xfffe0000u;
                    if (sub == 0) {
                        *(uint32_t *)(imp + 0x88) += mag | 0xcf00u;
                        jitter = 0;
                        mag = 0xfffecf00u;
                    }
                } else {
                    mag = (sub != 2) ? 0xfffecf00u : 0x10000u;
                    if (sub != 2) {
                        *(uint32_t *)(imp + 0x88) += mag | 0xcf00u;
                        mag = 0xfffecf00u;
                    }
                }
            }
            *(uint32_t *)(imp + 0x80) += mag | 0x3100u;
            if ((uint32_t)Rand255() & 0x1f) {
                ;
            } else Damage_VsImpactor_Burst(imp);
            uint32_t h = Pool_AllocProjectile();
            Pool_LaunchProjectile(h, *(uint32_t *)(self[0x16] + 8), 5, self + 9);
            self = (uint32_t *)1;
        }
    }
bind_fx:
    *self = 0x84u;
    {
        char hh = (char)Pool_AllocSFX();
        *((char *)self + 5) = hh;
        Pool_BindFXOnObject(hh, *(uint32_t *)(self[0x16] + 8), 3, 0);
    }
    SFX_StopWorld((int)*((char *)self + 5));
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

/* --- VALLYFRM.DLL FUN_80100eb4  (from analysis/dll/VALLYFRM/decomp/80100eb4.c) --- */
// addr: 0x80100eb4  name: FUN_80100eb4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100eb4(undefined4 *param_1,int param_2,int *param_3)

{
  bool bVar1;
  char cVar2;
  uint uVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  
  uVar9 = 1;
  puVar4 = param_1;
  if (param_2 != 1) {
    if (param_2 == 0) {
LAB_80100ef8:
      uVar6 = (uint)*(char *)(param_1 + 2);
      if (uVar6 == 1) {
LAB_80101004:
        uVar3 = param_1[9] - 0x1dcd;
        param_1[9] = uVar3;
        if ((int)uVar3 < 0x33e0000) {
LAB_80101028:
          iVar7 = param_1[9];
          param_1[9] = iVar7 + 0x1dcd;
          if ((int)uVar3 < iVar7 + 0x1dcd) {
            FUN_800205f8/*0x800205f8*/(param_1);
          }
        }
      }
      else {
        uVar3 = 2;
        if (((1 < (int)uVar6) || (uVar3 = 0x4d30000, uVar6 != 0)) &&
           (bVar1 = uVar6 == uVar3, uVar3 = 0x3920000, bVar1)) goto LAB_80101028;
        iVar8 = param_1[0xb];
        iVar7 = iVar8 + -0x1dcd;
        param_1[0xb] = iVar7;
        if (((iVar7 <= (int)(uVar3 | 0xffff)) && (0x4d40000 < iVar8)) ||
           ((iVar7 < 0x4b50000 && (0x4b50000 < iVar8)))) {
          param_1[0x12] = param_1[9];
          param_1[0x13] = param_1[10];
          param_1[0x14] = param_1[0xb];
          iVar7 = func_0x80021c6c(param_1);
          *(undefined1 *)(iVar7 + 8) = 1;
          *(undefined2 *)(iVar7 + 0x42) = 0x400;
          FUN_8002036c/*0x8002036c*/();
          iVar7 = func_0x80021c6c(param_1);
          *(undefined1 *)(iVar7 + 8) = 2;
          *(undefined2 *)(iVar7 + 0x42) = 0xfc00;
          FUN_8002036c/*0x8002036c*/();
        }
        if ((int)param_1[0xb] < 0x4a90000) {
          DAT_80101308 = DAT_80101308 + -1;
          goto LAB_80101004;
        }
      }
      if ((_DAT_80065310 - (uint)*(byte *)((int)param_1 + 9) & 3) == 0) {
        uVar9 = 0x80;
        puVar4 = (undefined4 *)FUN_8001ac44/*0x8001ac44*/(_DAT_800737d8,0x1f,0x80,8);
        *puVar4 = 0x10;
        uVar6 = FUN_80017160/*0x80017160*/();
        puVar4[9] = ((uVar6 & 0xff) - 0x80) * 0x400;
        puVar4[10] = 0;
        puVar4[0xb] = 0xfffec000;
        puVar4[0x19] = FUN_80100e70;
        FUN_8001d4f0/*0x8001d4f0*/(param_1);
      }
      if (param_3 == (int *)0x0) {
        return 0;
      }
      uVar5 = FUN_800446dc/*0x800446dc*/(param_1 + 9);
      FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar5);
    }
    else if (param_2 != 3) {
      if (param_2 == 4) goto LAB_801011f8;
      goto LAB_80100ef8;
    }
    iVar7 = *param_3;
    if (*(char *)(iVar7 + 4) != '\x02') {
      return 0;
    }
    uVar6 = (uint)*(char *)(param_1 + 2);
    if (uVar6 == uVar9) {
LAB_8010115c:
      iVar8 = 1;
      uVar6 = 0xfffecf00;
    }
    else {
      iVar8 = iVar7;
      if ((int)uVar6 < 2) {
        bVar1 = uVar6 != 0;
        uVar6 = 0xfffe0000;
        if (bVar1) {
          iVar8 = 1;
          goto LAB_80101138;
        }
LAB_80101148:
        *(uint *)(iVar7 + 0x88) = *(int *)(iVar7 + 0x88) + (uVar6 | 0xcf00);
        goto LAB_8010115c;
      }
LAB_80101138:
      bVar1 = uVar6 != 2;
      uVar6 = 0x10000;
      if (bVar1) goto LAB_80101148;
    }
    *(uint *)(iVar7 + 0x80) = *(int *)(iVar7 + 0x80) + (uVar6 | 0x3100);
    uVar9 = FUN_80017160/*0x80017160*/(iVar8);
    if ((uVar9 & 0x1f) == 0) {
      FUN_8002c4bc/*0x8002c4bc*/(iVar7);
    }
    uVar5 = FUN_8004410c/*0x8004410c*/();
    FUN_8004483c/*0x8004483c*/(uVar5,*(undefined4 *)(param_1[0x16] + 8),5,param_1 + 9);
    puVar4 = (undefined4 *)0x1;
  }
  *param_1 = 0x84;
  cVar2 = FUN_8004410c/*0x8004410c*/(puVar4);
  *(char *)((int)param_1 + 5) = cVar2;
  FUN_800443c8/*0x800443c8*/((int)cVar2,*(undefined4 *)(param_1[0x16] + 8),3,0);
LAB_801011f8:
  FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
  return 0;
}

#endif  /* GHIDRA REF */
