/* tracker_dish.c -- AirGrave radar/AA dish destructible tracker.
 *
 * Source: AIRGRAVE.DLL  FUN_80100228.
 *
 * Multi-stage destructible: the dish rotates to track the local
 * player, and fires when armed. Each damage tick advances through
 * health stages (the +8 byte at obj+0x38 = current stage, decremented
 * by the damage path); when it reaches 1 the model swaps and the rate
 * of fire jumps. Final hit explodes.
 *
 * mode dispatch:
 *   0  -- per-tick rotate + fire-cycle update + AABB refit
 *   1  -- post-spawn AABB/ground init
 *   3  -- damage event (param_3 = impactor)
 *   8  -- explicit damage amount (param_3 = damage scalar)
 *
 * The 0x17d7 / nFrames constant is the projectile reload divisor
 * (4.12 fixed) -- larger nFrames slows reload.
 *
 * MED.
 */
#include <stdint.h>

extern int      Terrain_HeightAt(int32_t x, int32_t z);              /* FUN_80025400 */
extern int      Object_LocalToWorldPos(void);                        /* FUN_8001d624 -- returns ptr */
extern void     Object_RefitAABB(void *self);                        /* FUN_8001d708 */
extern int      ratan2(int dz, int dx);                              /* FUN_8004ecd4 */
extern void     SubModel_Detach(uint32_t modelHandle);               /* FUN_8003fc50 */
extern char     SubModel_PickInitialStage(uint32_t modelHandle);     /* FUN_8003fc94 */
extern int      SubModel_AddrAt(int worldXyz);                       /* FUN_8003fd24 */
extern uint32_t Pool_AllocProjectile(void);                          /* FUN_8004410c */
extern void     Pool_LaunchProjectile(uint32_t handle, uint32_t bin,
                                      int unused, int spawnXyz);     /* FUN_8004483c */
extern int      Damage_LookupImpactor(void *self, int kind,
                                      void *params, int n);          /* FUN_8002c958 */
extern int      Damage_AccumulateOrFire(uint32_t *self, uint16_t a); /* FUN_80020778 */
extern void     FX_RingFlash(void *self, int dy, void *params, int n); /* FUN_800176f8 */
extern uint32_t _DAT_80065ad4;   /* live target vehicle */
extern uint8_t  DAT_80100044, DAT_80100050;

uint32_t AG_TrackerDish(uint32_t *self, int mode, int *arg)
{
    uint32_t modelH = self[0xe];
    int      sub    = *(int *)(modelH + 0x38);

    if (mode == 1) goto ground_align;
    if (mode == 0 || mode == 8) {
        if (mode == 0) {
            /* Advance reload-cycle: pos += rate; if not yet firing, */
            /* tick rate by +0x10e per frame.                        */
            int pos  = *(int *)(sub + 0x28);
            int rate = *(int *)(sub + 0x48);
            pos += rate;
            *(int *)(sub + 0x28) = pos;
            if (*(char *)(sub + 8) == 0) {
                *(int *)(sub + 0x48) = rate + 0x10e;
                if (*(int *)(sub + 0x50) < pos) {
                    /* Reached fire-position: shoot. */
                    *(int *)(sub + 0x28) = *(int *)(sub + 0x50);
                    int8_t  curStage = *(char *)(modelH + 8);
                    uint16_t hp      = *(uint16_t *)((char *)self + 0xe);
                    uint8_t  stageMax= (uint8_t)self[2];
                    uint8_t  nFrames = (uint8_t)self[3];
                    *(char *)(sub + 8) = 1;
                    *(int *)(sub + 0x48) =
                        (int)(((uint32_t)hp * (curStage - 1) + nFrames) * -0x17d7)
                        / (int)((uint32_t)hp * (stageMax - 1));
                    int spawnAddr   = Object_LocalToWorldPos();
                    uint32_t projH  = Pool_AllocProjectile();
                    arg = NULL;
                    Pool_LaunchProjectile(projH,
                                          *(uint32_t *)(self[0x16] + 8),
                                          0, spawnAddr + 0x14);
                    SubModel_AddrAt(spawnAddr + 0x14);
                }
            }
            int pos2 = *(int *)(sub + 0x28);
            if (pos2 < *(int *)(sub + 0x4c)) {
                *(int *)(sub + 0x28) = *(int *)(sub + 0x4c);
                *(int *)(sub + 0x48) = 0;
                *(char *)(sub + 8)   = 0;
            }
            if ((int16_t)self[3] == 0) return 0;
            if ((*self & 0x4000) == 0)  return 0;
            int aimYaw = ratan2(*(int *)(_DAT_80065ad4 + 0x48) - (int)self[0x12],
                                *(int *)(_DAT_80065ad4 + 0x50) - (int)self[0x14]);
            uint16_t *yawCell = (uint16_t *)(modelH + 0x42);
            int delta = aimYaw - (int)*(uint16_t *)((char *)self + 0x42)
                              - (*yawCell - 0x800);
            *yawCell += (int16_t)(int8_t)((delta * 0x100000) >> 24);
            Object_RefitAABB(self);
        }
        /* fall-through into damage handling for mode==0 wasn't taken;
         * mode==8 lands here directly.                              */
    }
    if (mode == 3 || mode == 8 || mode == 0) {
        if (mode == 3 || mode == 0) {
            int    imp = *arg;
            if (*(uint8_t *)(imp + 4) == 2) {
                if (((int *)arg)[3] != *(int *)(self[0xe] + 0x38)) return 0;
                int wp = Object_LocalToWorldPos();
                FX_RingFlash((void *)(intptr_t)imp, &DAT_80100044, NULL, wp + 0x14);
                if (Damage_LookupImpactor((void *)(intptr_t)imp, -12,
                                          &DAT_80100050, 1) != 0) return 0;
            }
            arg = (int *)(uintptr_t)*(uint16_t *)(imp + 0xc);
        }
        int16_t newHp = (int16_t)((uint16_t)self[3] - (int)(intptr_t)arg);
        if ((int)(uint16_t)self[3] < (int)(intptr_t)arg) {
            /* Stage transition or final break. */
            int subAddr = *(int *)(self[0xe] + 0x38);
            if (*(char *)(self[0xe] + 8) == 0) return 0;
            uint8_t  state = *(uint8_t  *)(subAddr + 8);
            uint32_t pos   = *(uint32_t *)(subAddr + 0x28);
            uint32_t rate  = *(uint32_t *)(subAddr + 0x48);
            uint32_t firePt= *(uint32_t *)(subAddr + 0x50);
            SubModel_Detach(self[0xe]);
            int next = *(int *)(self[0xe] + 0x38);
            int8_t   stage = *(char *)(self[0xe] + 8);
            *(char *)(self[0xe] + 8) = (char)(stage - 1);
            if (next != 0 && stage != 1) {
                *(uint8_t  *)(next + 8)    = state;
                *(uint32_t *)(next + 0x28) = pos;
                *(uint32_t *)(next + 0x48) = rate;
                *(uint32_t *)(next + 0x50) = firePt;
                *(uint16_t *)(self + 3)    = *(uint16_t *)((char *)self + 0xe);
            }
            newHp = (int16_t)Damage_AccumulateOrFire(self, 0);
            *(uint16_t *)(self + 3) = 0;
        }
        *(int16_t *)(self + 3) = newHp;
    }
ground_align: {
        uint32_t mh    = self[0xe];
        int      sub2  = *(int *)(mh + 0x38);
        int      wp    = Object_LocalToWorldPos();
        int      grd   = Terrain_HeightAt(*(int32_t *)(wp + 0x14),
                                          *(int32_t *)(wp + 0x1c));
        *(int *)(sub2 + 0x50) = (grd - *(int *)(wp + 0x18)) + *(int *)(sub2 + 0x4c);
        char     stage = SubModel_PickInitialStage(mh);
        *(char *)(mh + 8)        = stage;
        *(char *)((char *)self + 8) = stage + 1;
        *self = (*self & 0xfffffffb) | 0x80;
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

/* --- AIRGRAVE.DLL FUN_80100228  (from analysis/dll/AIRGRAVE/decomp/80100228.c) --- */
// addr: 0x80100228  name: FUN_80100228

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100228(uint *param_1,int param_2,int *param_3)

{
  undefined1 uVar1;
  ushort uVar2;
  uint uVar3;
  uint uVar4;
  char cVar5;
  short sVar6;
  int iVar7;
  undefined4 uVar8;
  int iVar9;
  uint uVar10;
  int iVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined8 uVar14;
  
  if (param_2 == 1) goto LAB_80100510;
  if (param_2 == 0) {
LAB_80100274:
    uVar10 = param_1[0xe];
    iVar9 = *(int *)(uVar10 + 0x38);
    iVar7 = *(int *)(iVar9 + 0x28) + *(int *)(iVar9 + 0x48);
    *(int *)(iVar9 + 0x28) = iVar7;
    if (*(char *)(iVar9 + 8) == '\0') {
      *(int *)(iVar9 + 0x48) = *(int *)(iVar9 + 0x48) + 0x10e;
      if (*(int *)(iVar9 + 0x50) < *(int *)(iVar9 + 0x28)) {
        *(int *)(iVar9 + 0x28) = *(int *)(iVar9 + 0x50);
        cVar5 = *(char *)(uVar10 + 8);
        uVar2 = *(ushort *)((int)param_1 + 0xe);
        uVar3 = param_1[2];
        uVar4 = param_1[3];
        *(undefined1 *)(iVar9 + 8) = 1;
        *(int *)(iVar9 + 0x48) =
             (int)(((uint)uVar2 * (cVar5 + -1) + (uint)(ushort)uVar4) * -0x17d7) /
             (int)((uint)uVar2 * ((char)uVar3 + -1));
        iVar7 = FUN_8001d624/*0x8001d624*/();
        uVar8 = FUN_8004410c/*0x8004410c*/();
        param_3 = (int *)0x0;
        FUN_8004483c/*0x8004483c*/(uVar8,*(undefined4 *)(param_1[0x16] + 8),0,iVar7 + 0x14);
        iVar9 = 0x17;
        iVar7 = FUN_8003fd24/*0x8003fd24*/(iVar7 + 0x14);
        goto LAB_8010035c;
      }
    }
    else {
LAB_8010035c:
      if (iVar7 < *(int *)(iVar9 + 0x4c)) {
        *(int *)(iVar9 + 0x28) = *(int *)(iVar9 + 0x4c);
        *(undefined4 *)(iVar9 + 0x48) = 0;
        *(undefined1 *)(iVar9 + 8) = 0;
      }
    }
    if ((short)param_1[3] == 0) {
      return 0;
    }
    if ((*param_1 & 0x4000) == 0) {
      return 0;
    }
    iVar7 = ratan2/*0x8004ecd4*/(*(int *)(_DAT_80065ad4 + 0x48) - param_1[0x12],
                            *(int *)(_DAT_80065ad4 + 0x50) - param_1[0x14]);
    *(ushort *)(uVar10 + 0x42) =
         *(ushort *)(uVar10 + 0x42) +
         (short)(char)(((iVar7 - (uint)*(ushort *)((int)param_1 + 0x42)) -
                       (*(ushort *)(uVar10 + 0x42) - 0x800)) * 0x100000 >> 0x18);
    FUN_8001d708/*0x8001d708*/();
LAB_801003f4:
    iVar7 = *param_3;
    uVar14 = CONCAT44((uint)*(byte *)(iVar7 + 4),7);
    if (*(byte *)(iVar7 + 4) == 2) {
      if (param_3[3] != *(int *)(param_1[0xe] + 0x38)) {
        return 0;
      }
      iVar9 = FUN_8001d624/*0x8001d624*/();
      FUN_800176f8/*0x800176f8*/(iVar7,&DAT_80100044,iVar9 + 0x14);
      uVar14 = FUN_8002c958/*0x8002c958*/(iVar7,0xfffffff4,&DAT_80100050,1);
    }
    if ((int)((ulonglong)uVar14 >> 0x20) != (int)uVar14) {
      return 0;
    }
    param_3 = (int *)(uint)*(ushort *)(iVar7 + 0xc);
  }
  else {
    if (param_2 == 3) goto LAB_801003f4;
    if (param_2 != 8) goto LAB_80100274;
  }
  sVar6 = (ushort)param_1[3] - (short)param_3;
  if ((int)(uint)(ushort)param_1[3] < (int)param_3) {
    uVar10 = param_1[0xe];
    iVar7 = *(int *)(uVar10 + 0x38);
    if (*(char *)(uVar10 + 8) == '\0') {
      return 0;
    }
    uVar1 = *(undefined1 *)(iVar7 + 8);
    uVar8 = *(undefined4 *)(iVar7 + 0x28);
    uVar12 = *(undefined4 *)(iVar7 + 0x48);
    uVar13 = *(undefined4 *)(iVar7 + 0x50);
    FUN_8003fc50/*0x8003fc50*/(uVar10);
    iVar7 = *(int *)(uVar10 + 0x38);
    if ((iVar7 != 0) &&
       (cVar5 = *(char *)(uVar10 + 8), *(char *)(uVar10 + 8) = cVar5 + -1, cVar5 != '\x01')) {
      *(undefined1 *)(iVar7 + 8) = uVar1;
      *(undefined4 *)(iVar7 + 0x28) = uVar8;
      *(undefined4 *)(iVar7 + 0x48) = uVar12;
      *(undefined4 *)(iVar7 + 0x50) = uVar13;
      *(undefined2 *)(param_1 + 3) = *(undefined2 *)((int)param_1 + 0xe);
    }
    sVar6 = FUN_80020778/*0x80020778*/(param_1);
    *(undefined2 *)(param_1 + 3) = 0;
  }
  *(short *)(param_1 + 3) = sVar6;
LAB_80100510:
  uVar10 = param_1[0xe];
  iVar11 = *(int *)(uVar10 + 0x38);
  iVar7 = FUN_8001d624/*0x8001d624*/(iVar11);
  iVar9 = Terrain_HeightAt/*0x80025400*/(*(undefined4 *)(iVar7 + 0x14),*(undefined4 *)(iVar7 + 0x1c));
  *(int *)(iVar11 + 0x50) = (iVar9 - *(int *)(iVar7 + 0x18)) + *(int *)(iVar11 + 0x4c);
  cVar5 = FUN_8003fc94/*0x8003fc94*/(uVar10);
  *(char *)(uVar10 + 8) = cVar5;
  *(char *)(param_1 + 2) = cVar5 + '\x01';
  *param_1 = *param_1 & 0xfffffffb | 0x80;
  return 0;
}

#endif  /* GHIDRA REF */
