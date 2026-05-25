/* object_lifecycle.c -- object construction, registration, and teardown.
 *
 * Source: SLUS_005.10
 *   FUN_8001a640  -- Object_BuildFromBin: alloc the in-memory Object
 *                    struct from a parsed BIN+ANM pair (XOBF_Parse).
 *   FUN_8001a994  -- Object_FinishBuild: walk the per-prim list of the
 *                    new object and call Font_BuildTexture on each
 *                    embedded TIM atlas (the renderer "upload" step).
 *   FUN_8001a91c  -- Object_RegisterInChain (DrawChain_Register): walks
 *                    the new object's per-prim list and shrinks the
 *                    template-allocated tail via Heap_Realloc.
 *   FUN_8001aa38  -- Object_FreeAndChildren: frees the 3 sub-pointers
 *                    (template, anim, sound bank) then unhooks the
 *                    object itself via Object_FreeAndUnhook.
 *   FUN_8001aa0c  -- Object_FreeWithoutSound: same minus the sound.
 *
 * These five form the cradle-to-grave object lifecycle that
 * VehicleExp_Load + XOBF_Parse drive at level/match start.
 *
 * MED-HIGH confidence on each (small, single-purpose functions).
 */
#include <stdint.h>
#include <stddef.h>

extern void *Heap_AllocOrRetry(uint32_t n);
extern void *Heap_Realloc(void *p, uint32_t n);
extern void  Heap_Free(void *p);
extern void  Audio_FreeSND(void *bank);         /* FUN_80044394 */
extern void  Object_FreeAndUnhook(void *p);
extern int  *FUN_8001b3d4(int *param_1, uint param_2);  /* BoneAnim_TouchSlot */
extern uint16_t DAT_800568fc[];
extern int   FUN_8001884c(int slot);   /* VRAM_FreeTexSlot (renderer stub) */

/* HIGH: build the in-memory Object struct from a parsed
 * { template[], stride 0xc, count obj[4] } pair. */
uint32_t *Object_BuildFromBin(int *templateBody, void *animPtr)
{
    uint32_t *obj = (uint32_t *)Heap_AllocOrRetry(templateBody[4] * 0xc + 0xc);
    obj[0] = (uintptr_t)templateBody;
    obj[1] = (uintptr_t)animPtr;
    obj[2] = 0;

    templateBody[1] = (int)(uintptr_t)templateBody + templateBody[1];
    for (int groupIdx = 0; groupIdx < templateBody[0]; groupIdx++) {
        int *groupTable = (int *)(uintptr_t)templateBody[1];
        int group = (int)(uintptr_t)groupTable + groupTable[groupIdx];
        groupTable[groupIdx] = group;

        *(int *)(uintptr_t)(group + 0x04) = group + *(int *)(uintptr_t)(group + 0x04);
        *(int *)(uintptr_t)(group + 0x0c) = group + *(int *)(uintptr_t)(group + 0x0c);
        int packet = group + *(int *)(uintptr_t)(group + 0x14);
        *(int *)(uintptr_t)(group + 0x14) = packet;

        uint16_t primCount = *(uint16_t *)(uintptr_t)(group + 0x10);
        for (uint16_t prim = 0; prim < primCount; prim++) {
            uint8_t *pkt = (uint8_t *)(uintptr_t)packet;
            uint8_t rawType = pkt[3];
            uint32_t normType = (uint32_t)(rawType & 0x0f) << 2;
            if ((rawType & 0x80) != 0) normType |= 0x40;
            if ((rawType & 0x10) != 0) normType |= 0x02;
            if ((rawType & 0x40) != 0) normType |= 0x80;
            pkt[3] = (uint8_t)normType;

            *(uint16_t *)(pkt + 0x04) = (uint16_t)(*(uint16_t *)(pkt + 0x04) << 3);
            *(uint16_t *)(pkt + 0x06) = (uint16_t)(*(uint16_t *)(pkt + 0x06) << 3);
            *(uint16_t *)(pkt + 0x08) = (uint16_t)(*(uint16_t *)(pkt + 0x08) << 3);

            int strideBase = packet;
            switch ((normType >> 2) & 0x0f) {
            case 1:
            case 3:
                pkt[0x1b] = 0x34;
                pkt[0x17] = 0x34;
                break;
            case 2:
                pkt[0x13] = 0x30;
                pkt[0x0f] = 0x30;
                break;
            case 4:
            case 5:
            case 7:
                *(uint16_t *)(pkt + 0x0a) = (uint16_t)(*(uint16_t *)(pkt + 0x0a) << 3);
                break;
            case 8:
            case 9:
            case 0x0b:
            case 0x0c:
                *(uint16_t *)(pkt + 0x0a) = (uint16_t)(*(uint16_t *)(pkt + 0x0a) << 3);
                *(uint16_t *)(pkt + 0x0c) = (uint16_t)(*(uint16_t *)(pkt + 0x0c) << 3);
                *(uint16_t *)(pkt + 0x0e) = (uint16_t)(*(uint16_t *)(pkt + 0x0e) << 3);
                break;
            case 0x0a:
                strideBase = packet + (uint32_t)*(uint16_t *)(pkt + 0x0a) * 4u;
                break;
            case 0x0d:
            case 0x0f:
                pkt[3] |= 1;
                break;
            default:
                break;
            }

            packet = strideBase
                   + *(uint16_t *)((uint8_t *)DAT_800568fc + (pkt[3] & 0x3c));
        }
    }

    templateBody[3] = (int)(uintptr_t)templateBody + templateBody[3];
    for (int i = 0; i < templateBody[2]; i++) {
        int *table = (int *)(uintptr_t)templateBody[3];
        table[i] = (int)(uintptr_t)table + table[i];
    }

    templateBody[5] = (int)(uintptr_t)templateBody + templateBody[5];
    for (int i = 0; i < templateBody[4]; i++) {
        int *table = (int *)(uintptr_t)templateBody[5];
        table[i] = (int)(uintptr_t)table + table[i];
        *(uint16_t *)((uint8_t *)obj + 0x0c + i * 0x0c) = 0;
    }

    return obj;
}

uint32_t *FUN_8001a640(int *templateBody, void *animPtr)
{
    return Object_BuildFromBin(templateBody, animPtr);
}

/* HIGH: walk the object's prim list, uploading each TIM atlas. */
void Object_FinishBuild(int **obj)
{
    if (obj[0] == NULL) return;
    int n = *(int *)((uintptr_t)obj[0] + 0x10);
    if (n <= 0) return;
    for (int i = 0, off = 0xc; i < n; i++, off += 0xc) {
        FUN_8001884c((int)(uintptr_t)obj + off);
    }
}

/* HIGH: per-prim bone touch-up + Heap_Realloc shrink. */
void Object_RegisterInChain(int **obj)
{
    int *hdr = (int *)obj[0];
    uint32_t n = (uint32_t)hdr[0x10 / 4];
    for (uint32_t i = 0; i < n; i++) {
        FUN_8001b3d4((int *)obj, (uint)(uint16_t)i);
    }
    /* Trim the allocation to its actual size. */
    int trimSize = hdr[0x14 / 4] - (int)(uintptr_t)hdr;
    Heap_Realloc(hdr, (uint32_t)trimSize);
}

/* HIGH: free the three sub-pointer slots if non-null + the object. */
void Object_FreeWithoutSound(int *obj)
{
    Object_FinishBuild((int **)obj);   /* matches FUN_8001a994 call */
    Heap_Free(obj);
}

void Object_FreeAndChildren(int *obj)
{
    if (obj[0] != 0) Heap_Free((void *)(intptr_t)obj[0]);
    if (obj[1] != 0) Heap_Free((void *)(intptr_t)obj[1]);
    if (obj[2] != 0) Audio_FreeSND((void *)(intptr_t)obj[2]);
    Object_FreeWithoutSound(obj);
}

/* ================================================================
 * FUN_8001a994 -- Object_FinishBuild / FreeVRAMSlots
 *
 * Walks the per-prim slot list (param_1[3..3+N*3] each 12 bytes apart)
 * for the bone bank at *param_1, calling FUN_8001884c (VRAM TIM free)
 * on each. The count is at (*param_1)[+0x10].
 *
 * FUN_8001884c is the renderer-side VRAM allocator's free fn; the
 * stub in panic_stubs.c is acceptable while renderer remains stubbed.
 * HIGH confidence (direct Ghidra port).
 * ================================================================ */
extern void Heap_Free(void *p);       /* FUN_80045088 */

void FUN_8001a994(int *param_1)
{
    int iVar1;
    int iVar2;

    if ((*param_1 != 0) && (iVar1 = 0, 0 < *(int *)(uintptr_t)(*param_1 + 0x10))) {
        iVar2 = 0xc;
        do {
            FUN_8001884c((int)(uintptr_t)param_1 + iVar2);
            iVar1 = iVar1 + 1;
            iVar2 = iVar2 + 0xc;
        } while (iVar1 < *(int *)(uintptr_t)(*param_1 + 0x10));
    }
}

/* ================================================================
 * FUN_8001aa0c -- BoneBank_FinalFree
 *   FreeVRAMSlots(self), then heap-free self.
 * HIGH confidence.
 * ================================================================ */
void FUN_8001aa0c(int *param_1)
{
    FUN_8001a994(param_1);
    Heap_Free(param_1);
}

/* ================================================================
 * FUN_8001aa38 -- Object_FreeAndChildren
 *
 * Frees the 3 sub-pointer slots of a heap-resident object record
 * (param_1[0]=model, param_1[1]=anim, param_1[2]=audio bank descr),
 * then calls BoneBank_FinalFree to free per-prim VRAM and the
 * record itself.
 * HIGH confidence.
 * ================================================================ */
extern void FUN_80044394(int audioBank);   /* SfxBank_Free (audio stub) */

void FUN_8001aa38(int *param_1)
{
    if (*param_1    != 0) Heap_Free((void *)(uintptr_t)*param_1);
    if (param_1[1]  != 0) Heap_Free((void *)(uintptr_t)param_1[1]);
    if (param_1[2]  != 0) FUN_80044394(param_1[2]);
    FUN_8001aa0c(param_1);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001a640  (from analysis/SLUS_005.10/decomp/8001a640.c) --- */
// addr: 0x8001a640  name: FUN_8001a640

undefined4 * FUN_8001a640(int *param_1,undefined4 param_2)

{
  byte bVar1;
  undefined4 *puVar2;
  int iVar3;
  int *piVar4;
  uint uVar5;
  int iVar6;
  undefined4 *puVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  
  puVar2 = (undefined4 *)FUN_800116f4(param_1[4] * 0xc + 0xc);
  *puVar2 = param_1;
  puVar2[1] = param_2;
  puVar2[2] = 0;
  iVar9 = 0;
  param_1[1] = (int)param_1 + param_1[1];
  if (0 < *param_1) {
    do {
      piVar4 = (int *)(iVar9 * 4 + param_1[1]);
      iVar6 = param_1[1] + *piVar4;
      *piVar4 = iVar6;
      iVar10 = 0;
      iVar3 = iVar6 + *(int *)(iVar6 + 0x14);
      *(int *)(iVar6 + 4) = iVar6 + *(int *)(iVar6 + 4);
      *(int *)(iVar6 + 0xc) = iVar6 + *(int *)(iVar6 + 0xc);
      *(int *)(iVar6 + 0x14) = iVar3;
      if (*(short *)(iVar6 + 0x10) != 0) {
        do {
          bVar1 = *(byte *)(iVar3 + 3);
          uVar5 = (bVar1 & 0xf) << 2;
          if ((bVar1 & 0x80) != 0) {
            uVar5 = uVar5 | 0x40;
          }
          if ((bVar1 & 0x10) != 0) {
            uVar5 = uVar5 | 2;
          }
          if ((bVar1 & 0x40) != 0) {
            uVar5 = uVar5 | 0x80;
          }
          *(char *)(iVar3 + 3) = (char)uVar5;
          *(short *)(iVar3 + 4) = *(short *)(iVar3 + 4) << 3;
          *(short *)(iVar3 + 6) = *(short *)(iVar3 + 6) << 3;
          *(short *)(iVar3 + 8) = *(short *)(iVar3 + 8) << 3;
          iVar8 = iVar3;
          switch(uVar5 >> 2 & 0xf) {
          case 1:
          case 3:
                    /* Possible PsyQ macro: setPolyGT3() */
            *(undefined1 *)(iVar3 + 0x1b) = 0x34;
                    /* Possible PsyQ macro: setPolyGT3() */
            *(undefined1 *)(iVar3 + 0x17) = 0x34;
            break;
          case 2:
                    /* Possible PsyQ macro: setPolyG3() */
            *(undefined1 *)(iVar3 + 0x13) = 0x30;
                    /* Possible PsyQ macro: setPolyG3() */
            *(undefined1 *)(iVar3 + 0xf) = 0x30;
            break;
          case 4:
          case 5:
          case 7:
            *(short *)(iVar3 + 10) = *(short *)(iVar3 + 10) << 3;
            break;
          case 8:
          case 9:
          case 0xb:
          case 0xc:
            *(short *)(iVar3 + 10) = *(short *)(iVar3 + 10) << 3;
            *(short *)(iVar3 + 0xc) = *(short *)(iVar3 + 0xc) << 3;
            *(short *)(iVar3 + 0xe) = *(short *)(iVar3 + 0xe) << 3;
            break;
          case 10:
            iVar8 = iVar3 + (uint)*(ushort *)(iVar3 + 10) * 4;
            break;
          case 0xd:
          case 0xf:
            *(byte *)(iVar3 + 3) = *(byte *)(iVar3 + 3) | 1;
          }
          iVar10 = iVar10 + 1;
          iVar3 = iVar8 + (uint)*(ushort *)(&DAT_800568fc + (*(byte *)(iVar3 + 3) & 0x3c));
        } while (iVar10 < (int)(uint)*(ushort *)(iVar6 + 0x10));
      }
      iVar9 = iVar9 + 1;
    } while (iVar9 < *param_1);
  }
  iVar9 = 0;
  param_1[3] = (int)param_1 + param_1[3];
  if (0 < param_1[2]) {
    do {
      piVar4 = (int *)(iVar9 * 4 + param_1[3]);
      *piVar4 = param_1[3] + *piVar4;
      iVar9 = iVar9 + 1;
    } while (iVar9 < param_1[2]);
  }
  iVar9 = 0;
  param_1[5] = (int)param_1 + param_1[5];
  puVar7 = puVar2;
  if (0 < param_1[4]) {
    do {
      piVar4 = (int *)(iVar9 * 4 + param_1[5]);
      *piVar4 = param_1[5] + *piVar4;
      *(undefined2 *)(puVar7 + 3) = 0;
      iVar9 = iVar9 + 1;
      puVar7 = puVar7 + 3;
    } while (iVar9 < param_1[4]);
  }
  return puVar2;
}

/* --- SLUS_005.10 FUN_8001a994  (from analysis/SLUS_005.10/decomp/8001a994.c) --- */
// addr: 0x8001a994  name: FUN_8001a994

void FUN_8001a994(int *param_1)

{
  int iVar1;
  int iVar2;
  
  if ((*param_1 != 0) && (iVar1 = 0, 0 < *(int *)(*param_1 + 0x10))) {
    iVar2 = 0xc;
    do {
      FUN_8001884c((int)param_1 + iVar2);
      iVar1 = iVar1 + 1;
      iVar2 = iVar2 + 0xc;
    } while (iVar1 < *(int *)(*param_1 + 0x10));
  }
  return;
}

/* --- SLUS_005.10 FUN_8001a91c  (from analysis/SLUS_005.10/decomp/8001a91c.c) --- */
// addr: 0x8001a91c  name: FUN_8001a91c

void FUN_8001a91c(int *param_1)

{
  uint uVar1;
  int iVar2;
  
  iVar2 = *param_1;
  uVar1 = 0;
  if (0 < *(int *)(iVar2 + 0x10)) {
    do {
      FUN_8001b3d4(param_1,uVar1 & 0xffff);
      uVar1 = uVar1 + 1;
    } while ((int)uVar1 < *(int *)(iVar2 + 0x10));
  }
  FUN_80045134(iVar2,*(int *)(iVar2 + 0x14) - iVar2);
  return;
}

/* --- SLUS_005.10 FUN_8001aa38  (from analysis/SLUS_005.10/decomp/8001aa38.c) --- */
// addr: 0x8001aa38  name: FUN_8001aa38

void FUN_8001aa38(int *param_1)

{
  if (*param_1 != 0) {
    FUN_80045088();
  }
  if (param_1[1] != 0) {
    FUN_80045088();
  }
  if (param_1[2] != 0) {
    FUN_80044394();
  }
  FUN_8001aa0c(param_1);
  return;
}

/* --- SLUS_005.10 FUN_8001aa0c  (from analysis/SLUS_005.10/decomp/8001aa0c.c) --- */
// addr: 0x8001aa0c  name: FUN_8001aa0c

void FUN_8001aa0c(undefined4 param_1)

{
  FUN_8001a994();
  FUN_80045088(param_1);
  return;
}

/* --- SLUS_005.10 FUN_8001884c  (from analysis/SLUS_005.10/decomp/8001884c.c) --- */
// addr: 0x8001884c  name: FUN_8001884c

undefined4 FUN_8001884c(short *param_1)

{
  ushort uVar1;
  ushort uVar2;
  undefined4 uVar3;
  
  if (*param_1 == 0) {
    uVar3 = 1;
  }
  else {
    uVar1 = param_1[4];
    uVar2 = param_1[3];
    if (param_1[5] != 0) {
      FUN_8001859c(param_1[5]);
    }
    *param_1 = 0;
    uVar3 = FUN_80018530((uVar1 & 0xf) * 0x40 + ((int)(uVar2 & 0xff) >> (2 - (uVar1 >> 7) & 0x1f)),
                         (uVar1 & 0x10) * 0x10 + (uint)(uVar2 >> 8));
  }
  return uVar3;
}

/* --- SLUS_005.10 FUN_80044394  (from analysis/SLUS_005.10/decomp/80044394.c) --- */
// addr: 0x80044394  name: FUN_80044394

void FUN_80044394(int param_1)

{
  SpuFree((uint)*(ushort *)(param_1 + 2) << 3);
  FUN_80045088(param_1);
  return;
}

/* --- SLUS_005.10 FUN_8001b3d4  (from analysis/SLUS_005.10/decomp/8001b3d4.c) --- */
// addr: 0x8001b3d4  name: FUN_8001b3d4

int * FUN_8001b3d4(int *param_1,uint param_2)

{
  short sVar1;
  int *piVar2;
  
  piVar2 = param_1 + (param_2 & 0xffff) * 3 + 3;
  sVar1 = (short)*piVar2 + 1;
  *(short *)piVar2 = sVar1;
  if (sVar1 == 1) {
    FUN_800187e4(*(undefined4 *)((param_2 & 0xffff) * 4 + *(int *)(*param_1 + 0x14)),piVar2);
  }
  return piVar2;
}

#endif  /* GHIDRA REF */
