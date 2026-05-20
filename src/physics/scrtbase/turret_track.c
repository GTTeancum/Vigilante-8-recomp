/* turret_track.c -- Secret Base turret target tracking + fire.
 *
 * Source: SCRTBASE.DLL  FUN_80100970.
 *
 * Per-tick handler for SB defense turrets:
 *   case 0 (tick): rotate turret toward currently-locked target (held
 *       at self+0x78). Atan2(localX/localZ) -> yaw delta /4; pitch
 *       similarly with -1.0 scale, clamped to [-0x155, +0x155]. If
 *       target lost (Z>=0xfa001 or HP==0) clear lock.
 *   case 2: silent retire.
 *   case 3, 8: damage event, decrement counter (Damage_AccumulateOr-
 *       Fire) and reschedule damage timer.
 *   case 1: post-spawn init.
 *
 * Lock acquisition (when self+0x78 == 0): walk the global target
 * list at _DAT_80065a18; first kind==2 with HP!=0 within radius
 * 0xc7fff becomes the new lock; spawns the lock-on FX (object id
 * 0x1e3, attached via FUN_80031300 with sub-id 0x1e1, kind 0x98,
 * tick callback FUN_8010076c).
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t Object_LocalToWorldPos(int sub);                  /* FUN_8001d624 */
extern void Util_TransposeMatRotate(uint32_t mat, int p, int *out); /* FUN_800435c0 */
extern int  ratan2(int dz, int dx);
extern void Object_RefitAABB(void);                                /* FUN_8001d708 */
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag); /* FUN_8001ac44 */
extern uint32_t Object_SpawnAttached(int parent, int subBin, int kindA, int kindB, uint32_t *parentSlot); /* FUN_80031300 */
extern uint32_t Pool_AllocSFX(void);
extern void Pool_BindFXOnObject(uint32_t h, uint32_t bin, int slot, void *xyz); /* FUN_8004483c */
extern void Pool_BindSnareToObject(uint32_t h, uint32_t bin, int slot, int xyz); /* FUN_800447e8 */
extern void Object_BindLifecycle(uint32_t *self);                  /* FUN_800202f4 */
extern void Object_BindFinalize(void);                              /* FUN_800207c4 */
extern void Damage_RetireSelf(int self);                            /* FUN_80020744 */
extern int  Math_Distance(int xy1, uint32_t *xy2);                  /* FUN_80016aac */
extern int  Damage_StandardVehicle(int self, int *imp);             /* func_0x80022320 */
extern void Damage_RetireSelfAlt(int self);                         /* func_0x80020844 */
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t a);    /* FUN_80020778 */
extern void Damage_Apply_AgainstSelf(void *self, void *param);
extern int  *_DAT_80065a18;
extern uint32_t FUN_8010076c;

uint32_t SB_TurretTrack(int self, uint32_t mode, int *imp)
{
    int sub = *(int *)(self + 0x38);
    int trg = *(int *)(self + 0x78);

    switch (mode) {
    case 0: {
        uint32_t wm = Object_LocalToWorldPos(sub);
        int local[3] = { 0, 0, 0 };
        Util_TransposeMatRotate(wm, trg + 0x48, local);
        int yaw = ratan2(local[0], local[2]) << 20 >> 20;
        int pit = ratan2(local[1], local[2]) * -0x100000 >> 20;
        if (yaw < 0) yaw += 3;
        *(int16_t *)(sub + 0x42) += (int16_t)(yaw >> 2);
        if (pit < 0) pit += 3;
        int next = (int)*(int16_t *)(sub + 0x40) + (pit >> 2);
        if (next < -0x155) next = -0x155;
        if (next >  0x155) next =  0x155;
        *(int16_t *)(sub + 0x40) = (int16_t)next;
        Object_RefitAABB();
        if (local[2] < 0xfa001 && *(int16_t *)(trg + 0xc) != 0) return 0;
        *(int *)(self + 0x78) = 0;
        break;
    }
    case 1: goto schedule;
    case 2: break;
    case 3: goto damage;
    case 8: goto forward;
    default: return 0;
    }

    /* Reacquire lock. */
    if (*(int *)(self + 0x78) == 0) goto search;

    {
        uint32_t parent_fx = Object_SpawnFromBank(*(uint32_t *)(self + 0x58), 0x1e3, 0x80, 8);
        uint32_t fx       = Object_SpawnAttached(self, *(int *)(self + 0x38), 0x1e1, 0x98,
                                                 (uint32_t *)(uintptr_t)parent_fx);
        uint32_t *fp = (uint32_t *)(uintptr_t)fx;
        *fp = 0x84u;
        *(int16_t *)(fp + 3) = 0x32;
        fp[0x19] = (uint32_t)(uintptr_t)&FUN_8010076c;
        *(int16_t *)((char *)fp + 0x96) = 4;
        *(int16_t *)(fp + 0x25) = 8;
        Object_BindLifecycle(fp);
        *(uint32_t *)(uintptr_t)parent_fx = 0x10u;
        *(uint32_t *)(uintptr_t)(parent_fx + 0x19 * 4) = 0x8003e80cu;
        Object_BindFinalize();
        uint32_t h = Pool_AllocSFX();
        Pool_BindFXOnObject(h, *(uint32_t *)(*(int *)(self + 0x58) + 8), 0, fp + 9);
        for (;;) {
            *(uint32_t **)(self + 0x78) = fp;
            Damage_RetireSelf(self);
            uint32_t h2 = Pool_AllocSFX();
            Pool_BindSnareToObject(h2, *(uint32_t *)(*(int *)(self + 0x58) + 8), 2, trg);
search:
            if ((int *)*_DAT_80065a18 == NULL) break;
            int  scan = self + 0x48;
            int *prev = _DAT_80065a18;
            int *cur  = (int *)*_DAT_80065a18;
            while (cur != NULL) {
                uint32_t *cand = (uint32_t *)prev[2];
                int d;
                if (*(char *)(cand + 1) == 2 && *(int16_t *)(cand + 3) != 0 &&
                    (d = Math_Distance(scan, cand + 0x12)) <= 0xc7fff)
                    break;
                prev = cur;
                cur  = (int *)*cur;
            }
            if (cur == NULL) goto post_search;
        }
    }
post_search:
    imp = (int *)0x3c;
damage:
    if (*(char *)(*imp + 4) == 7) {
        imp = (int *)(uintptr_t)*(uint16_t *)(*imp + 0xc);
forward:
        if (Damage_StandardVehicle(self, imp) != 0) {
            Damage_RetireSelfAlt(self);
            Damage_AccumulateOrFire((uint32_t *)(uintptr_t)self, 0);
schedule:
            Damage_Apply_AgainstSelf((void *)(intptr_t)self,
                                     (void *)(intptr_t)(*(uint8_t *)(self + 9) + 0x1e));
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

/* --- SCRTBASE.DLL FUN_80100970  (from analysis/dll/SCRTBASE/decomp/80100970.c) --- */
// addr: 0x80100970  name: FUN_80100970

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100970(int param_1,undefined4 param_2,int *param_3)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int iVar4;
  undefined2 uVar5;
  int *piVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  int *piVar10;
  int unaff_s2;
  undefined4 local_30;
  undefined4 local_2c;
  int local_28;
  
  switch(param_2) {
  case 0:
    iVar8 = *(int *)(param_1 + 0x38);
    unaff_s2 = *(int *)(param_1 + 0x78);
    uVar3 = FUN_8001d624/*0x8001d624*/(iVar8);
    FUN_800435c0/*0x800435c0*/(uVar3,unaff_s2 + 0x48,&local_30);
    iVar4 = ratan2/*0x8004ecd4*/(local_30,local_28);
    iVar7 = (iVar4 << 0x14) >> 0x14;
    iVar4 = ratan2/*0x8004ecd4*/(local_2c,local_28);
    iVar4 = iVar4 * -0x100000 >> 0x14;
    if (iVar7 < 0) {
      iVar7 = iVar7 + 3;
    }
    *(short *)(iVar8 + 0x42) = *(short *)(iVar8 + 0x42) + (short)(iVar7 >> 2);
    if (iVar4 < 0) {
      iVar4 = iVar4 + 3;
    }
    iVar7 = (int)*(short *)(iVar8 + 0x40) + (iVar4 >> 2);
    iVar4 = -0x155;
    if ((iVar7 < -0x155) || (uVar5 = 0x155, iVar4 = iVar7, iVar7 < 0x156)) {
      uVar5 = (undefined2)iVar4;
    }
    *(undefined2 *)(iVar8 + 0x40) = uVar5;
    FUN_8001d708/*0x8001d708*/();
    if ((local_28 < 0xfa001) && (*(short *)(unaff_s2 + 0xc) != 0)) {
      return 0;
    }
    *(undefined4 *)(param_1 + 0x78) = 0;
    break;
  case 1:
    goto switchD_801009bc_caseD_1;
  case 2:
    break;
  case 3:
    goto switchD_801009bc_caseD_3;
  default:
    goto switchD_801009bc_caseD_4;
  case 8:
    goto switchD_801009bc_caseD_8;
  }
  if (*(int *)(param_1 + 0x78) == 0) goto LAB_80100ba0;
  puVar1 = (undefined4 *)FUN_8001ac44/*0x8001ac44*/(*(undefined4 *)(param_1 + 0x58),0x1e3,0x80,8);
  puVar2 = (undefined4 *)FUN_80031300/*0x80031300*/(param_1,*(undefined4 *)(param_1 + 0x38),0x1e1,0x98,puVar1);
  *puVar2 = 0x84;
  *(undefined2 *)(puVar2 + 3) = 0x32;
  puVar2[0x19] = FUN_8010076c;
  *(undefined2 *)((int)puVar2 + 0x96) = 4;
  *(undefined2 *)(puVar2 + 0x25) = 8;
  FUN_800202f4/*0x800202f4*/(puVar2);
  *puVar1 = 0x10;
  puVar1[0x19] = 0x8003e80c;
  FUN_800207c4/*0x800207c4*/();
  uVar3 = FUN_8004410c/*0x8004410c*/();
  FUN_8004483c/*0x8004483c*/(uVar3,*(undefined4 *)(*(int *)(param_1 + 0x58) + 8),0,puVar2 + 9);
  while( true ) {
    *(undefined4 **)(param_1 + 0x78) = puVar2;
    FUN_80020744/*0x80020744*/(param_1);
    uVar3 = FUN_8004410c/*0x8004410c*/();
    FUN_800447e8/*0x800447e8*/(uVar3,*(undefined4 *)(*(int *)(param_1 + 0x58) + 8),2,unaff_s2);
LAB_80100ba0:
    if ((int *)*_DAT_80065a18 == (int *)0x0) break;
    unaff_s2 = param_1 + 0x48;
    piVar6 = _DAT_80065a18;
    piVar9 = (int *)*_DAT_80065a18;
    while (((puVar2 = (undefined4 *)piVar6[2], *(char *)(puVar2 + 1) != '\x02' ||
            (*(short *)(puVar2 + 3) == 0)) ||
           (iVar4 = FUN_80016aac/*0x80016aac*/(unaff_s2,puVar2 + 0x12), 0xc7fff < iVar4))) {
      piVar10 = (int *)*piVar9;
      piVar6 = piVar9;
      piVar9 = piVar10;
      if (piVar10 == (int *)0x0) goto LAB_80100c14;
    }
  }
LAB_80100c14:
  param_3 = (int *)&DAT_0000003c;
switchD_801009bc_caseD_3:
  if (*(char *)(*param_3 + 4) == '\a') {
    param_3 = (int *)(uint)*(ushort *)(*param_3 + 0xc);
switchD_801009bc_caseD_8:
    iVar4 = FUN_80022320/*0x80022320*/(param_1,param_3);
    if (iVar4 != 0) {
      func_0x80020844(param_1);
      FUN_80020778/*0x80020778*/(param_1);
switchD_801009bc_caseD_1:
      FUN_80020890/*0x80020890*/(param_1,*(byte *)(param_1 + 9) + 0x1e);
    }
  }
switchD_801009bc_caseD_4:
  return 0;
}

#endif  /* GHIDRA REF */
