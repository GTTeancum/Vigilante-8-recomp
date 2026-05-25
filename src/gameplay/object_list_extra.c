/* object_list_extra.c -- more object list utilities.
 *
 * Source: SLUS_005.10
 *   FUN_80020000  -- ObjList_FindBySpawnIdValue (returns payload, not node)
 *   FUN_8002002c  -- ObjList_TickListWith3Args
 *   FUN_80020120  -- ObjList_CountWithFlag
 *   FUN_80020190  -- ObjList_NthWithFlag
 *   FUN_800202f4  -- Object_RegisterInScene
 *   FUN_8001ffd4  -- ObjList_FindPayloadBySpawnId (list, id -> payload or 0)
 *
 * The lists at DAT_80065a18 (world), DAT_80065a60 (back-buf pending),
 * DAT_80065a80 (active draw chain) all share the 12-byte node layout:
 *   +0 next, +4 back, +8 payload.
 *
 * Object_RegisterInScene fast-clear-then-insert into up to three of
 * those chains based on the object's flag bits (4 -> draw chain, 0x80
 * -> back-buf, always -> world).
 *
 * "WithFlag" variants filter by: spawnId > 0x1f AND obj.flags has the
 * requested bit AND obj.flags has none of (0x8002 = invulnerable +
 * dead). The Nth helper additionally counts down to pick the Nth.
 *
 * HIGH.
 */
#include <stdint.h>
#include <stddef.h>

extern int *ObjList_FindBySpawnId(int **listHead, int spawnId, int excludePayload);   /* FUN_8001ff58 */
extern void ObjList_FastInsert(void *listHead, uint32_t *obj);                       /* FUN_8001fe50 */
extern uint8_t DAT_80065a18[];
extern uint8_t DAT_80065a60[];
extern uint8_t DAT_80065a80[];
extern uint8_t DAT_80065a50[];
extern intptr_t Host_TerrainFindPlaceholderById(int spawn_id);
extern int Host_TerrainCountPlaceholdersWithFlag(uint32_t flag);
extern intptr_t Host_TerrainNthPlaceholderWithFlag(uint32_t flag, int n);
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

typedef struct ObjListExtraHostNode {
    struct ObjListExtraHostNode *next;
    struct ObjListExtraHostNode *prev;
    uintptr_t payload;
    uint32_t deadline;
} ObjListExtraHostNode;

int ObjList_FindBySpawnIdValue(int **listHead, int spawnId, int exclude)
{
    int *node = ObjList_FindBySpawnId(listHead, spawnId, exclude);
    return (node != NULL) ? (int)((ObjListExtraHostNode *)node)->payload : 0;
}

void ObjList_TickListWith3Args(int **listHead, uint32_t arg2, uint32_t arg3)
{
    ObjListExtraHostNode *sentinel = (ObjListExtraHostNode *)listHead;
    ObjListExtraHostNode *node;
    if (sentinel == NULL || sentinel->prev == NULL)
        return;
    for (node = sentinel->next; node != NULL; node = node->next) {
        typedef int (*TickFn)(int, uint32_t, uint32_t);
        uintptr_t payload = node->payload;
        TickFn fn = (TickFn)Object_CallbackFromPsxSlot((const void *)payload);
        if (fn != NULL) fn((int)payload, arg2, arg3);
    }
}

int ObjList_CountWithFlag(int **listHead, uint32_t flag)
{
    ObjListExtraHostNode *sentinel = (ObjListExtraHostNode *)listHead;
    ObjListExtraHostNode *node;
    int count = 0;
    if (sentinel == NULL || sentinel->prev == NULL)
        return 0;
    for (node = sentinel->next; node != NULL; node = node->next) {
        uint32_t *payload = (uint32_t *)node->payload;
        if (*(int16_t *)((uintptr_t)payload + 6) > 0x1f
            && (payload[0] & flag) != 0
            && (payload[0] & 0x8002u) == 0)
        {
            count++;
        }
    }
    return count;
}

uint32_t *ObjList_NthWithFlag(int **listHead, uint32_t flag, int n)
{
    ObjListExtraHostNode *sentinel = (ObjListExtraHostNode *)listHead;
    ObjListExtraHostNode *node;
    if (sentinel == NULL || sentinel->prev == NULL)
        return NULL;
    for (node = sentinel->next; node != NULL; node = node->next) {
        uint32_t *payload = (uint32_t *)node->payload;
        if (*(int16_t *)((uintptr_t)payload + 6) > 0x1f
            && (payload[0] & flag) != 0
            && (payload[0] & 0x8002u) == 0)
        {
            if (--n == -1) return payload;
        }
    }
    return NULL;
}

void Object_RegisterInScene(uint32_t *obj)
{
    if ((obj[0] & 4u)    != 0) ObjList_FastInsert(DAT_80065a80, obj);
    if ((obj[0] & 0x80u) != 0) ObjList_FastInsert(DAT_80065a60, obj);
    ObjList_FastInsert(DAT_80065a18, obj);
}

/* ================================================================
 * FUN_8001ffd4 -- ObjList_FindPayloadBySpawnId
 *
 * Thin wrapper: calls FUN_8001ff58(param_1, param_2, 0) (ObjList_FindBySpawnId
 * with no exclusion), then returns the payload pointer at node[8] or 0.
 *
 * Equivalent to ObjList_FindBySpawnIdValue(param_1, param_2, 0) but with
 * the raw int types Ghidra inferred.
 *
 * HIGH confidence (direct Ghidra ref port).
 * ================================================================ */
intptr_t FUN_8001ffd4(intptr_t param_1, int param_2)
{
    if ((uintptr_t)param_1 == (uintptr_t)DAT_80065a50) {
        intptr_t host = Host_TerrainFindPlaceholderById(param_2);
        if (host != 0)
            return host;
    }
    if (*(uint32_t *)(uintptr_t)param_1 == 0)
        return 0;

    intptr_t iVar1 = (intptr_t)ObjList_FindBySpawnId((int **)(uintptr_t)param_1, param_2, 0);
    if (iVar1 == 0) return 0;
    return (intptr_t)((ObjListExtraHostNode *)(uintptr_t)iVar1)->payload;
}

/* Hex-name aliases for functions above (used in Ghidra-style call sites). */
int FUN_80020120(int *param_1, uint32_t param_2)
{
    if ((uintptr_t)param_1 == (uintptr_t)DAT_80065a50) {
        int count = Host_TerrainCountPlaceholdersWithFlag(param_2);
        if (count != 0)
            return count;
    }
    return ObjList_CountWithFlag((int **)param_1, param_2);
}
uint32_t *FUN_80020190(int *param_1, uint32_t param_2, int param_3)
{
    if ((uintptr_t)param_1 == (uintptr_t)DAT_80065a50) {
        intptr_t obj = Host_TerrainNthPlaceholderWithFlag(param_2, param_3);
        if (obj != 0)
            return (uint32_t *)obj;
    }
    return ObjList_NthWithFlag((int **)param_1, param_2, param_3);
}

/* FUN_800202f4 -- Object_RegisterInScene: alias so callers using PSX name link. */
void FUN_800202f4(uint32_t *obj) { Object_RegisterInScene(obj); }

/* FUN_800207c4 -- Object_BindFinalize: set bit 2 then insert into draw-bind list.
 * MIPS-verified: 12 instructions. HIGH. */
void FUN_800207c4(uint32_t *param_1)
{
    *param_1 |= 4u;
    ObjList_FastInsert(DAT_80065a80, param_1);
}

/* ================================================================
 * FUN_80020744 -- Object_BindToBackBuf: set bit 7, insert into back-buf
 *                 pending list (DAT_80065a60).
 * FUN_80020778 -- Object_UnbindFromBackBuf: clear bit 7, remove from
 *                 back-buf list (returns 0 if bit was already clear).
 * FUN_800207f8 -- Object_UnbindFromWorldBind: clear bit 2, remove from
 *                 world-bind list (DAT_80065a80).
 *
 * As with FUN_800207c4, Ghidra pseudo-C drops the implicit `move a1,a0`
 * at the MIPS prologue, so the trailing helper call appears to take
 * only 1 arg.  The MIPS shows 2.  HIGH: direct port.
 * ================================================================ */
extern int FUN_8001fe8c(uint32_t *param_1, int param_2);  /* Object_UnlinkFromList */

void FUN_80020744(uint32_t *param_1)
{
    *param_1 |= 0x80u;
    ObjList_FastInsert(DAT_80065a60, param_1);
}

uint32_t FUN_80020778(uint32_t *param_1)
{
    uint32_t uVar1;
    if ((*param_1 & 0x80u) == 0) {
        uVar1 = 0;
    } else {
        *param_1 &= 0xffffff7fu;
        uVar1 = (uint32_t)FUN_8001fe8c((uint32_t *)DAT_80065a60, (int)(uintptr_t)param_1);
    }
    return uVar1;
}

uint32_t FUN_800207f8(uint32_t *param_1)
{
    uint32_t uVar1;
    if ((*param_1 & 4u) == 0) {
        uVar1 = 0;
    } else {
        *param_1 &= 0xfffffffbu;
        uVar1 = (uint32_t)FUN_8001fe8c((uint32_t *)DAT_80065a80, (int)(uintptr_t)param_1);
    }
    return uVar1;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80020000  (from analysis/SLUS_005.10/decomp/80020000.c) --- */
// addr: 0x80020000  name: FUN_80020000

undefined4 FUN_80020000(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_8001ff58();
  uVar2 = 0;
  if (iVar1 != 0) {
    uVar2 = *(undefined4 *)(iVar1 + 8);
  }
  return uVar2;
}

/* --- SLUS_005.10 FUN_8002002c  (from analysis/SLUS_005.10/decomp/8002002c.c) --- */
// addr: 0x8002002c  name: FUN_8002002c

void FUN_8002002c(int *param_1,undefined4 param_2,undefined4 param_3)

{
  int *piVar1;
  int *piVar2;
  code *pcVar3;
  int iVar4;
  
  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while( true ) {
    if (piVar2 == (int *)0x0) {
      return;
    }
    pcVar3 = *(code **)(piVar1[2] + 100);
    if (pcVar3 == (code *)0x0) {
      iVar4 = 0;
    }
    else {
      iVar4 = (*pcVar3)(piVar1[2],param_2,param_3);
    }
    if (iVar4 != 0) break;
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  return;
}

/* --- SLUS_005.10 FUN_80020120  (from analysis/SLUS_005.10/decomp/80020120.c) --- */
// addr: 0x80020120  name: FUN_80020120

int FUN_80020120(int *param_1,uint param_2)

{
  int *piVar1;
  int *piVar2;
  uint uVar3;
  int iVar4;
  
  iVar4 = 0;
  piVar1 = (int *)*param_1;
  for (piVar2 = (int *)*(int *)*param_1; piVar2 != (int *)0x0; piVar2 = (int *)*piVar2) {
    if (((0x1f < *(short *)(piVar1[2] + 6)) && (uVar3 = *(uint *)piVar1[2], (uVar3 & param_2) != 0))
       && ((uVar3 & 0x8002) == 0)) {
      iVar4 = iVar4 + 1;
    }
    piVar1 = piVar2;
  }
  return iVar4;
}

/* --- SLUS_005.10 FUN_80020190  (from analysis/SLUS_005.10/decomp/80020190.c) --- */
// addr: 0x80020190  name: FUN_80020190

uint * FUN_80020190(int *param_1,uint param_2,int param_3)

{
  int *piVar1;
  int *piVar2;
  uint *puVar3;
  
  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while( true ) {
    if (piVar2 == (int *)0x0) {
      return (uint *)0x0;
    }
    puVar3 = (uint *)piVar1[2];
    if ((((0x1f < *(short *)((int)puVar3 + 6)) && ((*puVar3 & param_2) != 0)) &&
        ((*puVar3 & 0x8002) == 0)) && (param_3 = param_3 + -1, param_3 == -1)) break;
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  return puVar3;
}

/* --- SLUS_005.10 FUN_800202f4  (from analysis/SLUS_005.10/decomp/800202f4.c) --- */
// addr: 0x800202f4  name: FUN_800202f4

void FUN_800202f4(uint *param_1)

{
  if ((*param_1 & 4) != 0) {
    FUN_8001fe50(&DAT_80065a80,param_1);
  }
  if ((*param_1 & 0x80) != 0) {
    FUN_8001fe50(&DAT_80065a60,param_1);
  }
  FUN_8001fe50(&DAT_80065a18,param_1);
  return;
}

/* --- SLUS_005.10 FUN_800207c4  (from analysis/SLUS_005.10/mips/800207c4.s) ---
 *
 * Object_BindFinalize: set the "in world bind list" flag (bit 2 of obj[0]),
 * then insert the object into the world-bind active list (DAT_80065a80).
 *
 * MIPS (12 instructions):
 *   800207c4: addiu sp,sp,-0x18
 *   800207c8: move  a1,a0            ; a1 = param_1 (object ptr)
 *   800207cc: sw    ra,0x10(sp)
 *   800207d0: lw    v0,0x0(a1)
 *   800207d4: lui   a0,0x8006
 *   800207d8: addiu a0,a0,0x5a80     ; a0 = &DAT_80065a80
 *   800207dc: ori   v0,v0,0x4
 *   800207e0: jal   0x8001fe50       ; FUN_8001fe50(&DAT_80065a80, param_1)
 *   800207e4: _sw   v0,0x0(a1)       ; delay: *param_1 |= 4
 *
 * Note: Ghidra pseudoC omits the `move a1,a0` and shows a 1-arg call;
 * the MIPS confirms both args are passed.
 * HIGH: line-for-line from MIPS. */
void FUN_800207c4(uint *param_1)
{
    *param_1 |= 4u;
    FUN_8001fe50((int)&DAT_80065a80, (int)param_1);
}

/* --- SLUS_005.10 FUN_8001ff58  (from analysis/SLUS_005.10/decomp/8001ff58.c) --- */
// addr: 0x8001ff58  name: FUN_8001ff58

int * FUN_8001ff58(int *param_1,int param_2,int param_3)

{
  int *piVar1;
  int *piVar2;
  
  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while( true ) {
    if (piVar2 == (int *)0x0) {
      return (int *)0x0;
    }
    if ((piVar1[2] != param_3) && (*(short *)(piVar1[2] + 6) == param_2)) break;
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  return piVar1;
}

/* --- SLUS_005.10 FUN_8001fe50  (from analysis/SLUS_005.10/decomp/8001fe50.c) --- */
// addr: 0x8001fe50  name: FUN_8001fe50

void FUN_8001fe50(int param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  
  piVar1 = piRam0000076c;
  iVar3 = *piRam0000076c;
  *(undefined **)(iVar3 + 4) = &DAT_80065a70;
  piVar2 = piRam0000076c + 2;
  piRam0000076c = (int *)iVar3;
  *piVar2 = param_2;
  piVar2 = *(int **)(param_1 + 8);
  *(int **)(param_1 + 8) = piVar1;
  *piVar2 = (int)piVar1;
  piVar1[1] = (int)piVar2;
  *piVar1 = param_1 + 4;
  return;
}

#endif  /* GHIDRA REF */
