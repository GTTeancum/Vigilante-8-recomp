/* elevator_full.c -- Sand Factory M2_elevator full state machine.
 *
 * Source: SANDFACT.DLL  FUN_80100854.
 *
 * Full elevator state machine:
 *   - mode 0 moves the car body along Z, refreshes active loop volume,
 *     plays the end-stop cue, toggles the direction byte, and may
 *     fall through into the source impact path.
 *   - modes 2 and 4 are deferred SFX/state transitions.
 *   - modes 3 and 8 share the damage/occupancy path.
 *
 * Used by all SandFact elevators including the central material lift.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t SfxChannel_Acquire(void);
extern uint32_t SfxPan_For3DPos(void *posXyz);
extern void Audio_PlaySfx_inner(int ch, uint32_t bank, int sfxId, uint32_t pan);
extern void Audio_PlaySfxAtPosVar(uint32_t ch, uint32_t bank, int sfxId, void *pos);
extern void SPU_VoiceVolume_Set(int ch, uint32_t lr);
extern void Audio_VoiceStop(int ch);
extern void Object_RegisterPostUpdate(uint32_t *obj);
extern uint32_t Object_ClearBackBufferFlag(uint32_t *obj);
extern void FUN_80020890(uint32_t *obj, int timer);
extern uint32_t FUN_80022320(uint32_t *self, uint32_t amount);
extern void *Matrix_ComposeParentChain(int obj);   /* FUN_8001d624 */

static int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

uint32_t SF_ElevatorFullTick(uint32_t *self, int mode, uintptr_t arg)
{
    uint32_t bank = *(uint32_t *)(uintptr_t)(self[0x16] + 8);

    switch (mode) {
    case 0: {
        uint32_t carBody = self[0xe];

        if (arg != 0) {
            uint8_t *carMatrix = Matrix_ComposeParentChain((int)(uintptr_t)carBody);
            uint32_t pan = SfxPan_For3DPos(carMatrix + 0x14);
            SPU_VoiceVolume_Set((int)*(int8_t *)((uint8_t *)self + 5), pan);
        }

        int32_t *carZ = (int32_t *)(uintptr_t)(carBody + 0x28);
        if (*(int8_t *)((uint8_t *)self + 8) != 0) {
            int32_t z = mips_addu_i32(*carZ, -0x393);
            *carZ = z;
            if (z >= *(int32_t *)(uintptr_t)(carBody + 0x4c))
                return 0;
        }

        int32_t z = mips_addu_i32(*carZ, 0x393);
        *carZ = z;
        if (z < -0x7fff)
            return 0;

        uint8_t *carMatrix = Matrix_ComposeParentChain((int)(uintptr_t)carBody);
        Audio_PlaySfxAtPosVar((uint32_t)(int)*(int8_t *)((uint8_t *)self + 5),
                              bank, 1, carMatrix + 0x14);
        *(int8_t *)((uint8_t *)self + 5) = 0;
        Object_ClearBackBufferFlag(self);

        uint8_t newDir = (uint8_t)mips_subu_i32(1, *(uint8_t *)((uint8_t *)self + 8));
        *(uint8_t *)((uint8_t *)self + 8) = newDir;
        if ((int8_t)newDir == 0)
            return 0;
        FUN_80020890(self, 300);
        /* Source falls into the mode-3 impact branch. */
    }
        /* fall through */
    case 3: {
        uint32_t impactObj = *(uint32_t *)(uintptr_t)arg;
        uint8_t kind = *(uint8_t *)(uintptr_t)(impactObj + 4);

        if (kind == 2 &&
            *(uint32_t *)(uintptr_t)(arg + 0xc) == self[0xe] &&
            *(int8_t *)((uint8_t *)self + 8) == 0 &&
            (self[0] & 0x80u) != 0) {
            uint8_t *carMatrix = Matrix_ComposeParentChain((int)(uintptr_t)self[0xe]);
            Audio_PlaySfxAtPosVar((uint32_t)(int)*(int8_t *)((uint8_t *)self + 5),
                                  bank, 1, carMatrix + 0x14);
            *(int8_t *)((uint8_t *)self + 5) = 0;
            Object_ClearBackBufferFlag(self);
            *(int8_t *)((uint8_t *)self + 8) = 1;
            FUN_80020890(self, 0x1e);
            kind = *(uint8_t *)(uintptr_t)(impactObj + 4);
        }
        if (kind != 7)
            return 0;
        arg = *(uint16_t *)(uintptr_t)(impactObj + 0xc);
    }
        /* fall through */
    case 8:
        if (FUN_80022320(self, (uint32_t)arg) != 0) {
            *(int8_t *)((uint8_t *)self + 8) = -1;
            if ((self[0] & 0x80u) != 0)
                Object_ClearBackBufferFlag(self);
            Audio_VoiceStop((int)*(int8_t *)((uint8_t *)self + 5));
            *(int8_t *)((uint8_t *)self + 5) = 0;
        }
        if (*(int8_t *)((uint8_t *)self + 8) != 0)
            return 0;
        {
            uint8_t counter = (uint8_t)mips_addu_i32(*(uint8_t *)((uint8_t *)self + 9),
                                                     (int32_t)arg);
            *(uint8_t *)((uint8_t *)self + 9) = counter;
            if (counter < 0x32)
                return 0;
        }
        *(uint8_t *)((uint8_t *)self + 9) = 0;
        /* fall through */
    case 2:
        if (self[0xe] != 0 && (self[0] & 0x80u) == 0) {
            Object_RegisterPostUpdate(self);
            uint32_t cueVoice = SfxChannel_Acquire();
            uint8_t *carMatrix = Matrix_ComposeParentChain((int)(uintptr_t)self[0xe]);
            Audio_PlaySfxAtPosVar(cueVoice, bank, 1, carMatrix + 0x14);
            int8_t loopVoice = (int8_t)SfxChannel_Acquire();
            *(int8_t *)((uint8_t *)self + 5) = loopVoice;
            Audio_PlaySfx_inner((int)loopVoice, bank, 2, 0);
            /* fall through */
    case 4:
            Audio_VoiceStop((int)*(int8_t *)((uint8_t *)self + 5));
        }
        return 0;
    default:
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

/* --- SANDFACT.DLL FUN_80100854  (from analysis/dll/SANDFACT/decomp/80100854.c) --- */
// addr: 0x80100854  name: FUN_80100854

undefined4 FUN_80100854(uint *param_1,undefined4 param_2,int *param_3)

{
  char cVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  undefined4 uVar5;
  
  switch(param_2) {
  case 0:
    uVar4 = param_1[0xe];
    if (param_3 != (int *)0x0) {
      iVar3 = FUN_8001d624/*0x8001d624*/(uVar4);
      uVar5 = FUN_800449bc/*0x800449bc*/(iVar3 + 0x14);
      FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar5);
    }
    if (((char)param_1[2] != '\0') &&
       (iVar3 = *(int *)(uVar4 + 0x28) + -0x393, *(int *)(uVar4 + 0x28) = iVar3,
       *(int *)(uVar4 + 0x4c) <= iVar3)) {
      return 0;
    }
    iVar3 = *(int *)(uVar4 + 0x28) + 0x393;
    *(int *)(uVar4 + 0x28) = iVar3;
    if (iVar3 < -0x7fff) {
      return 0;
    }
    iVar3 = FUN_8001d624/*0x8001d624*/(uVar4);
    FUN_800447e8/*0x800447e8*/((int)*(char *)((int)param_1 + 5),*(undefined4 *)(param_1[0x16] + 8),1,
                    iVar3 + 0x14);
    *(undefined1 *)((int)param_1 + 5) = 0;
    FUN_80020778/*0x80020778*/(param_1);
    cVar1 = '\x01' - (char)param_1[2];
    *(char *)(param_1 + 2) = cVar1;
    if (cVar1 == '\0') {
      return 0;
    }
    FUN_80020890/*0x80020890*/(param_1,300);
    break;
  default:
    goto switchD_80100890_caseD_1;
  case 2:
    goto switchD_80100890_caseD_2;
  case 3:
    break;
  case 4:
    goto switchD_80100890_caseD_4;
  case 8:
    goto switchD_80100890_caseD_8;
  }
  iVar3 = *param_3;
  cVar1 = *(char *)(iVar3 + 4);
  if ((((cVar1 == '\x02') && (param_3[3] == param_1[0xe])) && ((char)param_1[2] == '\0')) &&
     ((*param_1 & 0x80) != 0)) {
    iVar2 = FUN_8001d624/*0x8001d624*/();
    FUN_800447e8/*0x800447e8*/((int)*(char *)((int)param_1 + 5),*(undefined4 *)(param_1[0x16] + 8),1,
                    iVar2 + 0x14);
    *(undefined1 *)((int)param_1 + 5) = 0;
    FUN_80020778/*0x80020778*/(param_1);
    *(undefined1 *)(param_1 + 2) = 1;
    FUN_80020890/*0x80020890*/(param_1,0x1e);
    cVar1 = *(char *)(iVar3 + 4);
  }
  if (cVar1 == '\a') {
    param_3 = (int *)(uint)*(ushort *)(iVar3 + 0xc);
switchD_80100890_caseD_8:
    iVar3 = FUN_80022320/*0x80022320*/(param_1,param_3);
    if (iVar3 != 0) {
      *(undefined1 *)(param_1 + 2) = 0xff;
      if ((*param_1 & 0x80) != 0) {
        FUN_80020778/*0x80020778*/(param_1);
      }
      FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
      *(undefined1 *)((int)param_1 + 5) = 0;
    }
    if (((char)param_1[2] == '\0') &&
       (uVar4 = (uint)*(byte *)((int)param_1 + 9) + (int)param_3,
       *(char *)((int)param_1 + 9) = (char)uVar4, 0x31 < (uVar4 & 0xff))) {
      *(undefined1 *)((int)param_1 + 9) = 0;
switchD_80100890_caseD_2:
      if ((param_1[0xe] != 0) && ((*param_1 & 0x80) == 0)) {
        FUN_80020744/*0x80020744*/(param_1);
        uVar5 = FUN_8004410c/*0x8004410c*/();
        iVar3 = FUN_8001d624/*0x8001d624*/(param_1[0xe]);
        FUN_800447e8/*0x800447e8*/(uVar5,*(undefined4 *)(param_1[0x16] + 8),1,iVar3 + 0x14);
        cVar1 = FUN_8004410c/*0x8004410c*/();
        *(char *)((int)param_1 + 5) = cVar1;
        FUN_800443c8/*0x800443c8*/((int)cVar1,*(undefined4 *)(param_1[0x16] + 8),2,0);
switchD_80100890_caseD_4:
        FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
      }
    }
  }
switchD_80100890_caseD_1:
  return 0;
}

#endif  /* GHIDRA REF */
