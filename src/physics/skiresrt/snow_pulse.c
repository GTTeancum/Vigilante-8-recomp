/* snow_pulse.c -- Ski Resort snow-puff pulse emitter + damage.
 *
 * Source: SKIRESRT.DLL  FUN_801016ac.
 *
 * Per-tick handler that decrements obj+2 countdown; on -1, samples
 * a path waypoint (FUN_8001b038 / FUN_8001b07c), builds a local
 * matrix with CompMatrixLV, and spawns a snow puff (object id 0x20
 * from bank _DAT_800737d8). Puff velocity = waypoint normal * (rand
 * + 0x1000) scaled by 4.12. Puff orientation uses RotMatrixYXZ with
 * a random Y angle. Then reload countdown = 8.
 *
 * Each tick also drifts ALL child puffs in the chain (+0x88..+0x90
 * velocity decay by /64 + gravity +0x38 on Y).
 *
 * mode dispatch:
 *   0  -- tick (above)
 *   1  -- post-spawn init (bind FX channel 3 with 0x40000 minimum
 *         lifetime at obj+0x15)
 *   3  -- impulse propagate + damage check
 *   8  -- damage forwarded variant
 *   4  -- retire (silence FX)
 *
 * MED.
 */
#include <stdint.h>

extern int  Rand255(void);
extern uint32_t PathPiece_Find(uint32_t piece, uint32_t flag);    /* FUN_8001b038 */
extern uint32_t Pathfinder_BuildNormal(uint8_t *out, uint32_t cur); /* FUN_8001b07c */
extern void CompMatrixLV(uint32_t mat, uint32_t a, uint8_t *b);    /* FUN_8004cf04 */
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag);
extern void RotMatrixYXZ_gte(int *yawXyz, uint32_t *outMat);
extern void Object_BumpSubstate_Or_FX(uint32_t *self);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
extern void Damage_FromImpulse(uint32_t *self, int *imp);
extern int  Damage_StandardVehicle(uint32_t *self, int *imp);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t a);
extern void SFX_StopWorld(int h);
extern uint8_t Pool_AllocSFX(void);
extern void Pool_BindSnareFx(uint8_t h, uint32_t bin, int slot, int aux);
extern uint8_t SFX_PlayWorld(uint32_t *pos);
extern void SFX_Update(int h, int posVoxel);
extern uint32_t _DAT_800737d8;
extern int LAB_8004042c(int obj, int event, int param3);

uint32_t SK_SnowPulse(uint32_t *self, int mode, int param3)
{
    switch (mode) {
    case 0: {
        int8_t cd = (int8_t)(*(uint8_t *)(self + 2)) - 1;
        *(int8_t *)(self + 2) = cd;
        if (cd == -1) {
            uint32_t ch = self[0xe];
            int speed = ((Rand255() << 8) >> 15) + 0x1000;
            uint32_t wp = PathPiece_Find(ch, 0x8000);
            uint8_t  loc[28];
            uint32_t nrm = Pathfinder_BuildNormal(loc, wp);
            CompMatrixLV(ch + 0x10, nrm, loc);
            uint32_t *puff = (uint32_t *)Object_SpawnFromBank(_DAT_800737d8, 0x20, 0xa0, 8);
            *puff |= 0x410u;
            int16_t nx = *(int16_t *)(loc + 0);
            int16_t ny = *(int16_t *)(loc + 6);
            int16_t nz = *(int16_t *)(loc + 12);
            int vx = nx * speed; if (vx < 0) vx += 0xfff;
            puff[0x22] = vx >> 12;
            int vy = ny * speed; if (vy < 0) vy += 0xfff;
            puff[0x23] = vy >> 12;
            int vz = nz * speed; if (vz < 0) vz += 0xfff;
            puff[0x24] = vz >> 12;
            int rot[3] = { 0, 0, Rand255() };
            RotMatrixYXZ_gte(rot, puff + 4);
            puff[9]  = *(uint32_t *)(loc + 16);  /* spawn pos copy */
            puff[10] = *(uint32_t *)(loc + 20);
            puff[0xb]= *(uint32_t *)(loc + 24);
            Object_SetCallbackPsxSlot(puff, (uintptr_t)&LAB_8004042c);
            Object_BumpSubstate_Or_FX(self);
            *(uint8_t *)(self + 2) = 8;
        }
        for (int c = *(int *)(self[0xe] + 0x34); c != 0; c = *(int *)(c + 0x34)) {
            int vx = *(int *)(c + 0x88);
            *(int *)(c + 0x24) += vx;
            *(int *)(c + 0x28) += *(int *)(c + 0x8c);
            *(int *)(c + 0x2c) += *(int *)(c + 0x90);
            int t = vx; if (t < 0) t += 0x3f;
            *(int *)(c + 0x88) = vx - (t >> 6);
            int vz = *(int *)(c + 0x90);
            int u = vz; if (u < 0) u += 0x3f;
            *(int *)(c + 0x90) = vz - (u >> 6);
            *(int *)(c + 0x8c) += 0x38;
        }
        if (param3 == 0) return 0;
        SFX_Update((int)*((char *)self + 5), SFX_PlayWorld(self + 9));
        /* fall to case 3 */
    }
    case 3:
        Damage_FromImpulse(self, (int *)(intptr_t)param3);
        /* fall to case 8 */
    case 8:
        if (Damage_StandardVehicle(self, (int *)(intptr_t)param3) != 0) {
            Damage_AccumulateOrFire(self, 0);
            SFX_StopWorld((int)*((char *)self + 5));
            *((char *)self + 5) = 0;
            goto init;
        }
        break;
    case 1:
init: {
        *(uint8_t *)(self + 2) = *((uint8_t *)self + 9) & 7;
        *self |= 0x80u;
        uint32_t life = 0x40000;
        if ((int)self[0x15] > 0x40000) life = self[0x15];
        self[0x15] = life;
        char h = (char)Pool_AllocSFX();
        *((char *)self + 5) = h;
        Pool_BindSnareFx((uint8_t)h, *(uint32_t *)(self[0x16] + 8), 3, 0);
    }
    /* fall to case 4 */
    case 4:
        SFX_StopWorld((int)*((char *)self + 5));
        break;
    default: break;
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

/* --- SKIRESRT.DLL FUN_801016ac  (from analysis/dll/SKIRESRT/decomp/801016ac.c) --- */
// addr: 0x801016ac  name: FUN_801016ac

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801016ac(uint *param_1,undefined4 param_2,int param_3)

{
  char cVar1;
  undefined4 uVar2;
  uint *puVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  undefined1 auStack_40 [4];
  short local_3c;
  short local_36;
  short local_30;
  uint local_2c;
  uint local_28;
  uint local_24;
  undefined2 local_20;
  undefined2 local_1e;
  undefined2 local_1c;
  
  switch(param_2) {
  case 0:
    cVar1 = (char)param_1[2] + -1;
    *(char *)(param_1 + 2) = cVar1;
    if (cVar1 == -1) {
      uVar7 = param_1[0xe];
      iVar4 = FUN_80017160/*0x80017160*/();
      iVar8 = ((iVar4 << 8) >> 0xf) + 0x1000;
      uVar2 = FUN_8001b038/*0x8001b038*/(uVar7,0x8000);
      uVar2 = FUN_8001b07c/*0x8001b07c*/(auStack_40,uVar2);
      CompMatrixLV/*0x8004cf04*/(uVar7 + 0x10,uVar2,auStack_40);
      puVar3 = (uint *)FUN_8001ac44/*0x8001ac44*/(_DAT_800737d8,0x20,0xa0,8);
      *puVar3 = *puVar3 | 0x410;
      iVar4 = local_3c * iVar8;
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0xfff;
      }
      puVar3[0x22] = iVar4 >> 0xc;
      iVar4 = local_36 * iVar8;
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0xfff;
      }
      puVar3[0x23] = iVar4 >> 0xc;
      iVar8 = local_30 * iVar8;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      puVar3[0x24] = iVar8 >> 0xc;
      local_20 = 0;
      local_1e = 0;
      local_1c = FUN_80017160/*0x80017160*/();
      RotMatrixYXZ_gte/*0x8004dab4*/(&local_20,puVar3 + 4);
      puVar3[9] = local_2c;
      puVar3[10] = local_28;
      puVar3[0xb] = local_24;
      puVar3[0x19] = 0x8004042c;
      FUN_8001d4f0/*0x8001d4f0*/(param_1);
      *(undefined1 *)(param_1 + 2) = 8;
    }
    for (iVar4 = *(int *)(param_1[0xe] + 0x34); iVar4 != 0; iVar4 = *(int *)(iVar4 + 0x34)) {
      iVar6 = *(int *)(iVar4 + 0x88);
      *(int *)(iVar4 + 0x24) = *(int *)(iVar4 + 0x24) + iVar6;
      *(int *)(iVar4 + 0x28) = *(int *)(iVar4 + 0x28) + *(int *)(iVar4 + 0x8c);
      *(int *)(iVar4 + 0x2c) = *(int *)(iVar4 + 0x2c) + *(int *)(iVar4 + 0x90);
      iVar8 = iVar6;
      if (iVar6 < 0) {
        iVar8 = iVar6 + 0x3f;
      }
      iVar5 = *(int *)(iVar4 + 0x90);
      *(int *)(iVar4 + 0x88) = iVar6 - (iVar8 >> 6);
      iVar8 = iVar5;
      if (iVar5 < 0) {
        iVar8 = iVar5 + 0x3f;
      }
      *(int *)(iVar4 + 0x90) = iVar5 - (iVar8 >> 6);
      *(int *)(iVar4 + 0x8c) = *(int *)(iVar4 + 0x8c) + 0x38;
    }
    if (param_3 == 0) {
      return 0;
    }
    uVar2 = FUN_800449bc/*0x800449bc*/(param_1 + 9);
    FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar2);
  case 3:
    FUN_8002239c/*0x8002239c*/(param_1,param_3);
switchD_801016f0_caseD_8:
    iVar4 = FUN_80022320/*0x80022320*/(param_1,param_3);
    if (iVar4 != 0) {
      FUN_80020778/*0x80020778*/(param_1);
      FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
      *(undefined1 *)((int)param_1 + 5) = 0;
switchD_801016f0_caseD_1:
      *(byte *)(param_1 + 2) = *(byte *)((int)param_1 + 9) & 7;
      *param_1 = *param_1 | 0x80;
      uVar7 = 0x40000;
      if (0x40000 < (int)param_1[0x15]) {
        uVar7 = param_1[0x15];
      }
      param_1[0x15] = uVar7;
      cVar1 = FUN_8004410c/*0x8004410c*/();
      *(char *)((int)param_1 + 5) = cVar1;
      FUN_80044484/*0x80044484*/((int)cVar1,*(undefined4 *)(param_1[0x16] + 8),3,0);
      goto switchD_801016f0_caseD_4;
    }
    break;
  case 1:
    goto switchD_801016f0_caseD_1;
  default:
    break;
  case 4:
switchD_801016f0_caseD_4:
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
    break;
  case 8:
    goto switchD_801016f0_caseD_8;
  }
  return 0;
}

#endif  /* GHIDRA REF */
