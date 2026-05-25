/* windmill_alarm.c -- Valley Farms windmill alarm dispatcher.
 *
 * Source: VALLYFRM.DLL  FUN_8010031c.
 *
 * Same dispatcher idiom as Hoover Dam siren_strobe.c (ring-buffer of
 * pose snapshots at self+0x22 -> 8 cells x 0x80 bytes), applied to
 * the windmill. Drains one ring slot per countdown into shared
 * display cell DAT_8008f660; on schedule fires an alarm-sprite
 * (0x184/0x185/0x186 by flag bits) at the impact point with target
 * yaw, then binds FX channel.
 *
 * mode dispatch (fall-through chain):
 *   0    -- per-frame: drain ring buffer + reseed DAT_80101304
 *   0x11 -- if windmill (self+6==0x61), arm path slot 0x61 at offset
 *           0x8f80
 *   1    -- post-spawn: pick path waypoint (Path_Pick @ 0x80065a50)
 *   2    -- schedule first 240-frame fire
 *   9    -- every 16 frames, spawn alarm sprite at impact
 *   0xb  -- bind FX channel
 *   7    -- retire FX
 *   4    -- free ring buffer
 *
 * MED.
 */
#include <stdint.h>

extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
extern int Rand255(void);
extern uint32_t Path_Pick(uint32_t bank, int seed);
extern int Path_AttachToObj(uint32_t flag, uint32_t path);
extern void Damage_StandardVehicleAlt(uint32_t *self, uint32_t *imp);
extern void VF_QueueDamage(void);
extern void Damage_Apply_AgainstSelf(void *self, void *param);
extern uint32_t Spawner_FindSlot(int idx);
extern void Path_ArmAt(uint32_t pathSlot, uint32_t offset);
extern uint32_t *Object_FetchPrevHead(uint32_t *self);
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag);
extern int  Math_Atan2_Pos(int xy);
extern void Object_Suspend(void);
extern uint8_t Pool_AllocSFX(void);
extern void Pool_BindFXOnObject(uint32_t h, uint32_t bin, int slot, int aux);
extern void Object_RetireFX(int port);
extern void Heap_Free(uint32_t p);

extern uint32_t DAT_80101304, DAT_80101308;
extern uint8_t  DAT_8008f660;
extern uint32_t _DAT_80065a10, _DAT_800659fc;
extern int LAB_8003e80c(int obj, int event, int param3);

uint32_t VF_WindmillAlarm(uint32_t *self, uint32_t mode, uint32_t *arg)
{
    switch (mode) {
    case 0:
        if (arg) {
            uint32_t v = self[0x20];
            self[0x20] = v - (uintptr_t)arg;
            if ((int)(v - (uintptr_t)arg) < 0) {
                do {
                    uint32_t idx = self[0x21];
                    uint32_t *src = (uint32_t *)((idx & 7) * 0x80 + self[0x22]);
                    self[0x20] += 6;
                    uint32_t *dst = (uint32_t *)&DAT_8008f660;
                    for (int i = 0; i < 8; i++) {
                        dst[0] = src[0]; dst[1] = src[1];
                        dst[2] = src[2]; dst[3] = src[3];
                        src += 4; dst += 4;
                    }
                    self[0x21] = idx + 1;
                } while ((int)self[0x20] < 0);
            }
        }
        DAT_80101304 = (uint32_t)Rand255();
        /* fall through */
    case 0x11:
        if (*(int16_t *)((char *)self + 6) == 0x61) {
            uint32_t slot = Spawner_FindSlot(0x61);
            if (slot != 0) Path_ArmAt(slot + 0xc, 0x8f80);
        }
        Damage_StandardVehicleAlt(self, arg);
        /* fall through */
    case 1: {
        *self = 0x80u;
        self[0x22] = 0x80u;
        uint32_t path = Path_Pick(0x80065a50u, 0x100);
        _DAT_80065a10 = (Path_AttachToObj(0x7f000000u, path) != 0);
        DAT_80101308 = 0;
    }
    /* fall through */
    case 2:
        VF_QueueDamage();
        Damage_Apply_AgainstSelf(self, (void *)(intptr_t)0xf0);
        /* fall through */
    case 9:
        if ((char)self[1] == 8 &&
            (DAT_80101304 = DAT_80101304 + 1, (DAT_80101304 & 0xf) == 0)) {
            uint32_t *head = Object_FetchPrevHead(self);
            uint32_t kind  = 0x186u;
            if (0xee5 < *(int *)((uintptr_t)head + 0x8c)) {
                kind = 0x185u;
                if (*self & 0x80000) kind = 0x184u;
            }
            uint32_t *p = (uint32_t *)Object_SpawnFromBank(
                *(uint32_t *)(_DAT_800659fc + 0x58), kind, 0x80, 8);
            *(uint8_t *)(p + 1) = 1;
            *p = 0x24u;
            p[0x12] = arg[0]; p[0x13] = arg[1]; p[0x14] = arg[2];
            *(uint16_t *)((char *)p + 0x42) =
                (uint16_t)Math_Atan2_Pos((int)(uintptr_t)((uintptr_t)head + 0x10));
            Object_SetCallbackPsxSlot(p, (uintptr_t)&LAB_8003e80c);
            Object_Suspend();
            self = (uint32_t *)1;
        case 0xb:
            *(uint8_t *)self = Pool_AllocSFX();
            Pool_BindFXOnObject((uintptr_t)self & 0xff,
                                *(uint32_t *)(*(int *)(_DAT_800659fc + 0x58) + 8), 4, 0);
        case 7:
            Object_RetireFX(0x8c);
        case 4:
            Heap_Free(self[0x22]);
        }
        break;
    default:
        break;
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

/* --- VALLYFRM.DLL FUN_8010031c  (from analysis/dll/VALLYFRM/decomp/8010031c.c) --- */
// addr: 0x8010031c  name: FUN_8010031c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_8010031c(uint *param_1,undefined4 param_2,undefined4 *param_3)

{
  undefined2 uVar1;
  uint uVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  
  switch(param_2) {
  case 0:
    if ((param_3 != (undefined4 *)0x0) &&
       (uVar2 = param_1[0x20], param_1[0x20] = uVar2 - (int)param_3, (int)(uVar2 - (int)param_3) < 0
       )) {
      do {
        uVar2 = param_1[0x21];
        puVar3 = (undefined4 *)((uVar2 & 7) * 0x80 + param_1[0x22]);
        puVar7 = puVar3 + 0x20;
        param_1[0x20] = param_1[0x20] + 6;
        puVar6 = (undefined4 *)&DAT_8008f660;
        do {
          uVar5 = puVar3[1];
          uVar8 = puVar3[2];
          uVar9 = puVar3[3];
          *puVar6 = *puVar3;
          puVar6[1] = uVar5;
          puVar6[2] = uVar8;
          puVar6[3] = uVar9;
          puVar3 = puVar3 + 4;
          puVar6 = puVar6 + 4;
        } while (puVar3 != puVar7);
        param_1[0x21] = uVar2 + 1;
      } while ((int)param_1[0x20] < 0);
    }
    DAT_80101304 = FUN_80017160/*0x80017160*/();
  case 0x11:
    if (*(short *)((int)param_1 + 6) == 0x61) {
      iVar4 = FUN_8001fd9c/*0x8001fd9c*/(0x61);
      if (iVar4 != 0) {
        FUN_80024718/*0x80024718*/(iVar4 + 0xc,0x8f80);
      }
    }
    func_0x80022120(param_1,param_3);
  case 1:
    *param_1 = 0x80;
    param_1[0x22] = 0x80;
    uVar5 = FUN_8001ffd4/*0x8001ffd4*/(0x80065a50,0x100);
    iVar4 = func_0x8003d080(0x7f000000,uVar5);
    _DAT_80065a10 = (uint)(iVar4 != 0);
    DAT_80101308 = 0;
  case 2:
    func_0x80023d00();
    FUN_80020890/*0x80020890*/(param_1,0xf0);
  case 9:
    if (((char)param_1[1] == '\b') && (DAT_80101304 = DAT_80101304 + 1, (DAT_80101304 & 0xf) == 0))
    {
      iVar4 = FUN_8001d5a0/*0x8001d5a0*/(param_1);
      uVar5 = 0x186;
      if ((0xee5 < *(int *)(iVar4 + 0x8c)) && (uVar5 = 0x185, (*param_1 & 0x80000) != 0)) {
        uVar5 = 0x184;
      }
      puVar6 = (undefined4 *)FUN_8001ac44/*0x8001ac44*/(*(undefined4 *)(_DAT_800659fc + 0x58),uVar5,0x80,8);
      *(undefined1 *)(puVar6 + 1) = 1;
      *puVar6 = 0x24;
      uVar5 = param_3[1];
      uVar8 = param_3[2];
      puVar6[0x12] = *param_3;
      puVar6[0x13] = uVar5;
      puVar6[0x14] = uVar8;
      uVar1 = Math_Atan2_Pos/*0x80016c88*/(iVar4 + 0x10);
      *(undefined2 *)((int)puVar6 + 0x42) = uVar1;
      puVar6[0x19] = 0x8003e80c;
      FUN_8002036c/*0x8002036c*/();
      param_1 = (uint *)0x1;
switchD_80100358_caseD_b:
      param_1 = (uint *)FUN_8004410c/*0x8004410c*/(param_1);
      FUN_800443c8/*0x800443c8*/(param_1,*(undefined4 *)(*(int *)(_DAT_800659fc + 0x58) + 8),4,0);
      goto switchD_80100358_caseD_7;
    }
    break;
  case 7:
switchD_80100358_caseD_7:
    FUN_8001d470/*0x8001d470*/(0x8c);
  case 4:
    Heap_Free/*0x80045088*/(param_1[0x22]);
  default:
    break;
  case 0xb:
    goto switchD_80100358_caseD_b;
  }
  return 0;
}

#endif  /* GHIDRA REF */
