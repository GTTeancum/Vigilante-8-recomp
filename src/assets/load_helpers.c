/* load_helpers.c -- small leaf helpers from LOAD.DLL.
 *
 * Source: LOAD.DLL
 *   FUN_80100148  -- Terrain_LoadBspNode: recursive BSP/kd-tree reader.
 *   FUN_801001ec  -- Terrain_BspInsertObject: insert a placed object-list
 *                    node into the BSP leaf selected by object +0x48/+0x50.
 *   FUN_8010063c  -- Map_ReadRegion: reads a 14-byte map-region record
 *                    (x,y, x2-x1+1, y2-y1+1, attr1, attr2) from the
 *                    map stream and links into the global region list.
 *   FUN_801061c4  -- DrawPrim_SetColor: rewrite the 24-bit colour in a
 *                    GP0 primitive header (keep the upper command byte).
 *   FUN_801064ac  -- DMA_RegisterCallback0
 *   FUN_801064d0  -- DMA_RegisterCallback1
 *   FUN_80106414  -- empty MDEC stub
 *   FUN_801060ac  -- empty MDEC stub
 *   FUN_801060d0  -- empty MDEC stub
 *   FUN_80106434  -- MDEC_StatusBit_29 (bit 29 of $v0 -- output FIFO ready)
 *   FUN_80106470  -- MDEC_StatusBit_24 (bit 24 of $v0 -- output queue empty)
 *
 * HIGH on each (every one is < 100 bytes).
 */
#include <stdint.h>

extern void *Heap_AllocOrRetry(uint32_t n);
extern int16_t XobfStream_ReadI16(void *streamRef);  /* func_0x800224b4 */
extern int32_t XobfStream_ReadI32(void *streamRef);  /* func_0x800224ec */
extern void DMACallback(int ch, void (*cb)(int));

extern void *_DAT_80065a00;
extern void **_DAT_80065aa8;
extern uint8_t DAT_80065aa4[];

/* HIGH: LOAD 80100148.  BSP stream node:
 *   i16 kind
 *   kind 0: leaf; runtime initializes an empty list sentinel in node[1..3].
 *   kind 1: X split, then i32 splitCoord, left subtree, right subtree.
 *   kind 2: Z split, then i32 splitCoord, left subtree, right subtree.
 *   other : rejected after the kind word, returning NULL.
 */
int32_t *Terrain_LoadBspNode(void *stream)
{
    int32_t kind = XobfStream_ReadI16(stream);
    int32_t *node;

    if (kind == 0) {
        node = (int32_t *)Heap_AllocOrRetry(0x10);
        node[1] = (int32_t)(uintptr_t)(node + 2);
        node[0] = 0;
        node[2] = 0;
        node[3] = (int32_t)(uintptr_t)(node + 1);
        return node;
    }
    if ((uint32_t)kind >= 3) {
        return 0;
    }

    node = (int32_t *)Heap_AllocOrRetry(0x10);
    node[0] = kind;
    node[1] = XobfStream_ReadI32(stream);
    node[2] = (int32_t)(uintptr_t)Terrain_LoadBspNode(stream);
    node[3] = (int32_t)(uintptr_t)Terrain_LoadBspNode(stream);
    return node;
}

/* HIGH: LOAD 801001ec.  Insert a list node whose payload is at child[2]
 * into the static-object BSP.  Object +0x48 is X and +0x50 is Z.
 */
void Terrain_BspInsertObject(int32_t *node, int32_t *child)
{
    int32_t kind = node[0];
    if (kind == 1) {
        uint8_t *obj = (uint8_t *)(uintptr_t)child[2];
        if (node[1] < *(int32_t *)(obj + 0x48)) {
            Terrain_BspInsertObject((int32_t *)(uintptr_t)node[3], child);
        } else {
            Terrain_BspInsertObject((int32_t *)(uintptr_t)node[2], child);
        }
        return;
    }
    if (kind == 2) {
        uint8_t *obj = (uint8_t *)(uintptr_t)child[2];
        if (node[1] < *(int32_t *)(obj + 0x50)) {
            Terrain_BspInsertObject((int32_t *)(uintptr_t)node[3], child);
        } else {
            Terrain_BspInsertObject((int32_t *)(uintptr_t)node[2], child);
        }
        return;
    }
    if (kind != 0) {
        return;
    }

    int32_t *tail = (int32_t *)(uintptr_t)node[3];
    node[3] = (int32_t)(uintptr_t)child;
    tail[0] = (int32_t)(uintptr_t)child;
    child[1] = (int32_t)(uintptr_t)tail;
    child[0] = (int32_t)(uintptr_t)(node + 2);
}

/* HIGH: LOAD 801005c0. */
void Terrain_LoadBsp(void *payload)
{
    uint8_t *cursor = (uint8_t *)payload;
    _DAT_80065a00 = Terrain_LoadBspNode(&cursor);
}

/* Backwards-compatible old cleanup name from pass 1. */
void IffNode_Link(int *parent, int *child)
{
    Terrain_BspInsertObject((int32_t *)parent, (int32_t *)child);
}

/* HIGH: 14-byte map region record:
 *   +0x0c i16 x         +0x0e i16 y          (origin)
 *   +0x10 i16 w         +0x12 i16 h          (rect size, inclusive +1)
 *   +0x08 i16 attrA     +0x0a i16 attrB      (palette / icon ids)
 * Then linked into the global region list at DAT_80065aa4.
 */
void *Map_ReadRegion(void *payload)
{
    uint8_t *cursor = (uint8_t *)payload;
    uint16_t *r = (uint16_t *)Heap_AllocOrRetry(0x14);
    int16_t x1, y1, x2, y2;
    r[3 * 2 + 0] = (uint16_t)(x1 = XobfStream_ReadI16(&cursor));   /* x  @ +0x0c */
    r[3 * 2 + 1] = (uint16_t)(y1 = XobfStream_ReadI16(&cursor));   /* y  @ +0x0e */
    x2 = XobfStream_ReadI16(&cursor);  r[4 * 2 + 0] = (uint16_t)((x2 - x1) + 1);  /* w @+0x10 */
    y2 = XobfStream_ReadI16(&cursor);  r[4 * 2 + 1] = (uint16_t)((y2 - y1) + 1);  /* h @+0x12 */
    (void)XobfStream_ReadI16(&cursor); /* discarded i16 */
    r[2 * 2 + 0] = (uint16_t)XobfStream_ReadI16(&cursor);          /* attrA @ +0x08 */
    r[2 * 2 + 1] = (uint16_t)XobfStream_ReadI16(&cursor);          /* attrB @ +0x0a */
    /* Tail-link insertion. */
    uint32_t **head = (uint32_t **)_DAT_80065aa8;
    *_DAT_80065aa8  = (void *)r;
    _DAT_80065aa8   = (void **)r;
    r[2 * 2 + 2]    = (uint16_t)(uintptr_t)head;
    *(uint32_t *)r  = (uint32_t)(uintptr_t)DAT_80065aa4;
    return r;
}

void DrawPrim_SetColor(uint32_t *prim, uint32_t rgb24)
{
    *prim = (*prim & 0xff000000u) | (rgb24 & 0x00ffffffu);
}

void DMA_RegisterCallback0(void (*cb)(int)) { DMACallback(0, cb); }
void DMA_RegisterCallback1(void (*cb)(int)) { DMACallback(1, cb); }

void MDEC_Empty0(void) { /* @ 0x801060ac */ }
void MDEC_Empty1(void) { /* @ 0x801060d0 */ }
void MDEC_Empty2(void) { /* @ 0x80106414 */ }

uint32_t MDEC_StatusBit_29(uint32_t v0_latched) { return (v0_latched >> 29) & 1u; }
uint32_t MDEC_StatusBit_24(uint32_t v0_latched) { return (v0_latched >> 24) & 1u; }

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- ? FUN_801001ec  (from analysis/dll/LOAD/decomp/801001ec.c) --- */
// addr: 0x801001ec  name: FUN_801001ec

void FUN_801001ec(int *param_1,int *param_2)

{
  int iVar1;
  undefined4 *puVar2;
  
  iVar1 = *param_1;
  if (iVar1 != 1) {
    if (iVar1 != 0) {
      if (iVar1 == 2) {
        return;
      }
      param_1 = (int *)0x1;
    }
    puVar2 = (undefined4 *)param_1[3];
    param_1[3] = (int)param_2;
    *puVar2 = param_2;
    param_2[1] = (int)puVar2;
    *param_2 = (int)(param_1 + 2);
  }
  return;
}

/* --- ? FUN_8010063c  (from analysis/dll/LOAD/decomp/8010063c.c) --- */
// addr: 0x8010063c  name: FUN_8010063c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8010063c(undefined4 param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined2 uVar3;
  short sVar4;
  undefined4 *puVar5;
  undefined4 local_res0 [4];
  
  local_res0[0] = param_1;
  puVar5 = (undefined4 *)Heap_AllocOrRetry/*0x800116f4*/(0x14);
  uVar3 = func_0x800224b4(local_res0);
  *(undefined2 *)(puVar5 + 3) = uVar3;
  uVar3 = func_0x800224b4(local_res0);
  *(undefined2 *)((int)puVar5 + 0xe) = uVar3;
  sVar4 = func_0x800224b4(local_res0);
  *(short *)(puVar5 + 4) = (sVar4 - *(short *)(puVar5 + 3)) + 1;
  sVar4 = func_0x800224b4(local_res0);
  *(short *)((int)puVar5 + 0x12) = (sVar4 - *(short *)((int)puVar5 + 0xe)) + 1;
  func_0x800224b4(local_res0);
  uVar3 = func_0x800224b4(local_res0);
  *(undefined2 *)(puVar5 + 2) = uVar3;
  uVar3 = func_0x800224b4(local_res0);
  *(undefined2 *)((int)puVar5 + 10) = uVar3;
  puVar1 = _DAT_80065aa8;
  puVar2 = puVar5;
  *_DAT_80065aa8 = puVar5;
  _DAT_80065aa8 = puVar2;
  puVar5[1] = puVar1;
  *puVar5 = 0x80065aa4;
  return;
}

/* --- ? FUN_801061c4  (from analysis/dll/LOAD/decomp/801061c4.c) --- */
// addr: 0x801061c4  name: FUN_801061c4

void FUN_801061c4(uint *param_1,uint param_2)

{
  *param_1 = *param_1 & 0xff000000 | param_2 & 0xffffff;
  return;
}

/* --- ? FUN_801064ac  (from analysis/dll/LOAD/decomp/801064ac.c) --- */
// addr: 0x801064ac  name: FUN_801064ac

void FUN_801064ac(undefined4 param_1)

{
  DMACallback/*0x800480b4*/(0,param_1);
  return;
}

/* --- ? FUN_801064d0  (from analysis/dll/LOAD/decomp/801064d0.c) --- */
// addr: 0x801064d0  name: FUN_801064d0

void FUN_801064d0(undefined4 param_1)

{
  DMACallback/*0x800480b4*/(1,param_1);
  return;
}

/* --- ? FUN_80106414  (from analysis/dll/LOAD/decomp/80106414.c) --- */
// addr: 0x80106414  name: FUN_80106414

void FUN_80106414(void)

{
  return;
}

/* --- ? FUN_801060ac  (from analysis/dll/LOAD/decomp/801060ac.c) --- */
// addr: 0x801060ac  name: FUN_801060ac

void FUN_801060ac(void)

{
  return;
}

/* --- ? FUN_801060d0  (from analysis/dll/LOAD/decomp/801060d0.c) --- */
// addr: 0x801060d0  name: FUN_801060d0

void FUN_801060d0(void)

{
  return;
}

/* --- ? FUN_80106434  (from analysis/dll/LOAD/decomp/80106434.c) --- */
// addr: 0x80106434  name: FUN_80106434

uint FUN_80106434(void)

{
  uint in_v0;
  
  return in_v0 >> 0x1d & 1;
}

/* --- ? FUN_80106470  (from analysis/dll/LOAD/decomp/80106470.c) --- */
// addr: 0x80106470  name: FUN_80106470

uint FUN_80106470(void)

{
  uint in_v0;
  
  return in_v0 >> 0x18 & 1;
}

#endif  /* GHIDRA REF */
