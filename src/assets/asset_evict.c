/* asset_evict.c -- streaming asset eviction by frustum-aware kd-tree walk.
 *
 * Source: SLUS_005.10
 *   FUN_80020ad0 -- Evict_LeafChainBackBuffer
 *   FUN_80020bec -- Evict_TreeWalk
 *   FUN_80020d3c -- Asset_VisibilityEvictTick
 *
 * V8 streams geometry into RAM as the camera moves. Each LOD chunk is
 * double-buffered (per parity) so that the renderer can keep reading
 * the front-buffer copy while a new chunk loads into the back-buffer.
 * When memory pressure rises (Heap_Alloc returns NULL), we walk the
 * spatial tree from the OUTSIDE of the view frustum inward, freeing
 * the back-buffer copy of any chunk whose two parity slots are both
 * non-NULL.
 *
 * The tree is a 2D (X,Z) kd-tree rooted at iRam000006fc.  Each node:
 *
 *   i32 kind         -- 0 = leaf, 1 = split-X, 2 = split-Z
 *   if kind == 0:
 *     ptr* chainHead -- linked list of chunks (next-pointer at +0)
 *     ...           -- nodes' children stored elsewhere; leaf inlines
 *                       the chain head into param_1+1 of Evict_LeafChainBackBuffer
 *   if kind != 0:
 *     i32 splitPos   -- world coord of the split plane
 *     ptr leftChild
 *     ptr rightChild
 *
 * Each chunk record carries a +0x30 ptr to a geometry-pair struct
 * (+0x1c parity0 ptr / +0x20 parity1 ptr) and a +0x68 ptr to a
 * texture-pair struct laid out the same way. Evict_LeafChainBackBuffer
 * frees only the *back buffer* (1 - g_dispBufIndex) so the renderer's
 * read of the current frame is not torn.
 *
 * Asset_VisibilityEvictTick:
 *   1. Reads the GTE's R matrix (the current camera basis).
 *   2. Builds two near-clip-plane corner vectors:
 *      L = (-screenW/2, 0, near), R = (+screenW/2, 0, near).
 *   3. Normalises them, then applies the camera's WORLD rotation
 *      (DAT_8006f6e0) -- this produces the L,R bounds in world space.
 *   4. Restores the saved GTE matrix.
 *   5. Walks the kd-tree with [minX..maxX, minZ..maxZ] derived from
 *      camera position +/- (L,R) and an offset radius DAT_8006f6f4 /
 *      DAT_8006f6fc.
 *   6. If the main tree was empty, retries the global leaf chain at
 *      DAT_80065a18.
 *
 * Returns 1 if it evicted something, 0 if nothing was available to
 * evict (Heap_AllocOrRetry treats that as terminal -> _boot()).
 *
 * MED confidence: control flow and call sites verified; the meaning of
 * the +0x68 texture-pair pointer vs +0x30 geometry-pair pointer is
 * inferred from offset patterns. Pass 2 should consolidate after the
 * geometry-chunk struct is named.
 */
#include <stdint.h>
#include "structs.h"

extern void  Heap_Free(void *p);
extern void  ReadRotMatrix(MATRIX *m);
extern void  SetRotMatrix(MATRIX *m);
extern long  VectorNormalSS(SVECTOR *a, SVECTOR *out);
extern void  GTE_RotateSV(const SVECTOR *in, SVECTOR *out);     /* FUN_8004316c */

extern int32_t   iRam00000004;      /* current display parity */
extern uintptr_t iRam000006fc;       /* kd-tree root, or 0 if no terrain loaded */
extern int32_t   iRam000006d8;       /* near-plane screen width */
extern int16_t   sRam000006d4;       /* near-plane z */
extern MATRIX    DAT_8006f6e0;       /* camera world rotation */
extern int32_t   DAT_8006f6f4;       /* current camera world X */
extern int32_t   DAT_8006f6fc;       /* current camera world Z */
extern uintptr_t DAT_80065a18;       /* fallback global leaf chain */

typedef struct ChunkPair {     /* @ +0x1c / +0x20 inside parent records */
    void *frame0;
    void *frame1;
} ChunkPair;

typedef struct ChunkRecord {
    /* +0x30: ChunkPair *geomPair */
    /* +0x68: ChunkPair *texPair  */
    uint8_t  reserved_0x00[0x30];
    void    *geomPair;
    uint8_t  reserved_0x34[0x68 - 0x34];
    void    *texPair;
} ChunkRecord;

typedef struct LeafChainNode {
    struct LeafChainNode *next;
    struct LeafChainNode *unused;
    ChunkRecord          *chunk;
} LeafChainNode;

/* HIGH: walk one leaf's chain, free the first back-buffer copy we find. */
int Evict_LeafChainBackBuffer(LeafChainNode **headPtr)
{
    LeafChainNode *cur = *headPtr;
    LeafChainNode *nxt = (LeafChainNode *)cur->next;
    while (nxt != NULL) {
        ChunkRecord *ck = cur->chunk;
        void *geomPair  = (void *)((uintptr_t)ck + 0x30);
        void *gpA = *(void **)((uint8_t *)geomPair + 0);   /* the ChunkPair* */
        if (gpA != NULL) {
            void *frame0 = *(void **)((uint8_t *)gpA + 0x1c);
            void *frame1 = *(void **)((uint8_t *)gpA + 0x20);
            if (frame0 != NULL && frame1 != NULL) {
                int backOff = (1 - iRam00000004) * 4 + 0x1c;
                Heap_Free(*(void **)((uint8_t *)gpA + backOff));
                *(void **)((uint8_t *)gpA + backOff) = NULL;

                /* Also evict the texture pair's back buffer if both halves loaded. */
                void *texPair = *(void **)((uintptr_t)ck + 0x68);
                if (texPair != NULL) {
                    void *t0 = *(void **)((uint8_t *)texPair + 0x1c);
                    void *t1 = *(void **)((uint8_t *)texPair + 0x20);
                    if (t0 != NULL && t1 != NULL) {
                        Heap_Free(*(void **)((uint8_t *)texPair + backOff));
                        *(void **)((uint8_t *)texPair + backOff) = NULL;
                    }
                }
                return 1;
            }
        }
        cur = nxt;
        nxt = (LeafChainNode *)nxt->next;
    }
    return 0;
}

/* HIGH-MED: recursive kd-tree walk. The 4-int bbox is { minX, maxX, minZ, maxZ }. */
int Evict_TreeWalk(int32_t *node, const int32_t *bbox)
{
    int32_t kind = node[0];
    if (kind == 0) {
        return Evict_LeafChainBackBuffer((LeafChainNode **)(node + 1));
    }
    if (kind == 1) {           /* split-X */
        int32_t split = node[1];
        /* Visit nearer child first (the side the camera is closer to)
         * so we prefer evicting chunks farther from the view. */
        int32_t cam = bbox[0];   /* minX side serves as "camera proxy" */
        int32_t *first, *second;
        if (bbox[0] - split < split - bbox[1]) {
            first  = (int32_t *)(uintptr_t)node[2];
            second = (int32_t *)(uintptr_t)node[3];
        } else {
            first  = (int32_t *)(uintptr_t)node[3];
            second = (int32_t *)(uintptr_t)node[2];
        }
        if (Evict_TreeWalk(first,  bbox)) return 1;
        if (Evict_TreeWalk(second, bbox)) return 1;
        (void)cam;
        return 0;
    }
    if (kind == 2) {           /* split-Z */
        int32_t split = node[1];
        int32_t *first, *second;
        if (bbox[2] - split < split - bbox[3]) {
            first  = (int32_t *)(uintptr_t)node[2];
            second = (int32_t *)(uintptr_t)node[3];
        } else {
            first  = (int32_t *)(uintptr_t)node[3];
            second = (int32_t *)(uintptr_t)node[2];
        }
        if (Evict_TreeWalk(first,  bbox)) return 1;
        if (Evict_TreeWalk(second, bbox)) return 1;
        return 0;
    }
    return 2;
}

/* MED: walk the visibility tree once trying to free one back-buffer
 * geometry chunk. Returns 1 on eviction, 0 if the tree has nothing
 * evictable. */
int Asset_VisibilityEvictTick(void)
{
    if (iRam000006fc == 0) return 0;

    MATRIX saved;
    ReadRotMatrix(&saved);

    SVECTOR vL, vR;
    vL.vx = (int16_t)(-iRam000006d8 / 2);
    vL.vy = 0;
    vL.vz = sRam000006d4;
    vR.vx = (int16_t)( iRam000006d8 / 2);
    vR.vy = 0;
    vR.vz = sRam000006d4;
    VectorNormalSS(&vL, &vL);
    VectorNormalSS(&vR, &vR);

    SetRotMatrix(&DAT_8006f6e0);
    GTE_RotateSV(&vL, &vL);
    GTE_RotateSV(&vR, &vR);
    SetRotMatrix(&saved);

    int32_t lx = vL.vx, rx = vR.vx;
    int32_t lo = (lx < rx) ? lx : rx; if (lo > 0) lo = 0;
    int32_t hi = (lx > rx) ? lx : rx; if (hi < 0) hi = 0;
    int32_t lz = vL.vz, rz = vR.vz;
    int32_t loZ = (lz < rz) ? lz : rz; if (loZ > 0) loZ = 0;
    int32_t hiZ = (lz > rz) ? lz : rz; if (hiZ < 0) hiZ = 0;

    int32_t bbox[4] = {
        DAT_8006f6f4 + lo  * 0x400,
        DAT_8006f6f4 + hi  * 0x400,
        DAT_8006f6fc + loZ * 0x400,
        DAT_8006f6fc + hiZ * 0x400,
    };

    if (Evict_TreeWalk((int32_t *)iRam000006fc, bbox))    return 1;
    if (Evict_LeafChainBackBuffer((LeafChainNode **)&DAT_80065a18)) return 1;
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

/* --- SLUS_005.10 FUN_80020ad0  (from analysis/SLUS_005.10/decomp/80020ad0.c) --- */
// addr: 0x80020ad0  name: FUN_80020ad0

undefined4 FUN_80020ad0(int *param_1)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  piVar1 = (int *)*param_1;
  piVar2 = (int *)*(int *)*param_1;
  while( true ) {
    if (piVar2 == (int *)0x0) {
      return 0;
    }
    iVar5 = piVar1[2];
    iVar3 = *(int *)(iVar5 + 0x30);
    if (((iVar3 != 0) && (*(int *)(iVar3 + 0x1c) != 0)) && (*(int *)(iVar3 + 0x20) != 0)) break;
    piVar1 = piVar2;
    piVar2 = (int *)*piVar2;
  }
  FUN_80045088(*(undefined4 *)(iVar3 + (1 - iRam00000004) * 4 + 0x1c));
  iVar4 = (1 - iRam00000004) * 4;
  *(undefined4 *)(*(int *)(iVar5 + 0x30) + iVar4 + 0x1c) = 0;
  iVar3 = *(int *)(iVar5 + 0x68);
  if (iVar3 == 0) {
    return 1;
  }
  if (*(int *)(iVar3 + 0x1c) == 0) {
    return 1;
  }
  if (*(int *)(iVar3 + 0x20) != 0) {
    FUN_80045088(*(undefined4 *)(iVar3 + iVar4 + 0x1c));
    *(undefined4 *)(*(int *)(iVar5 + 0x68) + (1 - iRam00000004) * 4 + 0x1c) = 0;
  }
  return 1;
}

/* --- SLUS_005.10 FUN_80020bec  (from analysis/SLUS_005.10/decomp/80020bec.c) --- */
// addr: 0x80020bec  name: FUN_80020bec

undefined4 FUN_80020bec(int *param_1,int *param_2)

{
  undefined4 uVar1;
  int iVar2;
  
  iVar2 = *param_1;
  if (iVar2 == 1) {
    if (*param_2 - param_1[1] < param_1[1] - param_2[1]) {
      iVar2 = FUN_80020bec(param_1[2],param_2);
      if (iVar2 != 0) {
        return 1;
      }
      iVar2 = param_1[3];
    }
    else {
      iVar2 = FUN_80020bec(param_1[3],param_2);
      if (iVar2 != 0) {
        return 1;
      }
      iVar2 = param_1[2];
    }
    iVar2 = FUN_80020bec(iVar2,param_2);
    if (iVar2 != 0) {
      return 1;
    }
    return 0;
  }
  if (iVar2 == 0) {
    uVar1 = FUN_80020ad0(param_1 + 1,param_2);
    return uVar1;
  }
  if (iVar2 != 2) {
    return 2;
  }
  if (param_2[2] - param_1[1] < param_1[1] - param_2[3]) {
    iVar2 = FUN_80020bec(param_1[2],param_2);
    if (iVar2 != 0) {
      return 1;
    }
    iVar2 = param_1[3];
  }
  else {
    iVar2 = FUN_80020bec(param_1[3],param_2);
    if (iVar2 != 0) {
      return 1;
    }
    iVar2 = param_1[2];
  }
  iVar2 = FUN_80020bec(iVar2,param_2);
  if (iVar2 != 0) {
    return 1;
  }
  return 0;
}

/* --- SLUS_005.10 FUN_80020d3c  (from analysis/SLUS_005.10/decomp/80020d3c.c) --- */
// addr: 0x80020d3c  name: FUN_80020d3c

undefined4 FUN_80020d3c(void)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int local_58;
  int local_54;
  int local_50;
  int local_4c;
  SVECTOR local_48;
  SVECTOR local_40;
  MATRIX MStack_38;
  
  if (iRam000006fc == 0) {
    uVar1 = 0;
  }
  else {
    ReadRotMatrix(&MStack_38);
    local_48.vx = (short)(-iRam000006d8 / 2);
    local_48.vy = 0;
    local_48.vz = sRam000006d4;
    local_40.vx = (short)(iRam000006d8 / 2);
    local_40.vy = 0;
    local_40.vz = sRam000006d4;
    VectorNormalSS(&local_48,&local_48);
    VectorNormalSS(&local_40,&local_40);
    SetRotMatrix((MATRIX *)&DAT_8006f6e0);
    FUN_8004316c(&local_48,&local_48);
    FUN_8004316c(&local_40,&local_40);
    SetRotMatrix(&MStack_38);
    iVar4 = (int)local_40.vx;
    iVar5 = (int)local_48.vx;
    iVar3 = iVar4;
    if (iVar5 < iVar4) {
      iVar3 = iVar5;
    }
    iVar2 = 0;
    if (iVar3 < 0) {
      iVar2 = iVar3;
    }
    local_58 = DAT_8006f6f4 + iVar2 * 0x400;
    if (iVar4 < iVar5) {
      iVar4 = iVar5;
    }
    iVar3 = 0;
    if (0 < iVar4) {
      iVar3 = iVar4;
    }
    iVar5 = (int)local_48.vz;
    iVar4 = (int)local_40.vz;
    local_54 = DAT_8006f6f4 + iVar3 * 0x400;
    iVar3 = iVar4;
    if (iVar5 < iVar4) {
      iVar3 = iVar5;
    }
    iVar2 = 0;
    if (iVar3 < 0) {
      iVar2 = iVar3;
    }
    local_50 = DAT_8006f6fc + iVar2 * 0x400;
    if (iVar4 < iVar5) {
      iVar4 = iVar5;
    }
    iVar3 = 0;
    if (0 < iVar4) {
      iVar3 = iVar4;
    }
    local_4c = DAT_8006f6fc + iVar3 * 0x400;
    iVar4 = FUN_80020bec(iRam000006fc,&local_58);
    if ((iVar4 == 0) && (iVar4 = FUN_80020ad0(&DAT_80065a18,&local_58), iVar4 == 0)) {
      return 0;
    }
    uVar1 = 1;
  }
  return uVar1;
}

/* --- SLUS_005.10 FUN_8004316c  (from analysis/SLUS_005.10/decomp/8004316c.c) --- */
// addr: 0x8004316c  name: FUN_8004316c

undefined2 * FUN_8004316c(SVECTOR *param_1,undefined2 *param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  gte_ldv0(param_1);
  gte_rtv0_b();
  uVar1 = gte_stIR1();
  uVar2 = gte_stIR2();
  uVar3 = gte_stIR3();
  *param_2 = (short)uVar1;
  param_2[1] = (short)uVar2;
  param_2[2] = (short)uVar3;
  return param_2;
}

#endif  /* GHIDRA REF */
