/* quadtree_nav.c -- Terrain quadtree A* pathfinding.
 *
 * Source: SLUS_005.10
 *   FUN_800247dc  -- QuadTree_DescendToCell    (~40 instr)
 *   FUN_80024888  -- QuadTree_FindNeighborCell  (~45 instr)
 *   FUN_80024998  -- QuadTree_GetOpenNeighbors  (~85 instr)
 *   FUN_80024cc0  -- QuadTree_CellDistance       (~15 instr)
 *   FUN_80024d54  -- QuadTree_AStarPath          (~160 instr)
 *
 * The terrain is represented as a hierarchical quadtree where each
 * cell is at most 10 bytes:
 *   [+0..+1] : unknown / flags
 *   [+2..+3] : child offset for quadrant 0 (NW, x<0 z<0)
 *   [+4..+5] : child offset for quadrant 1 (NE, x>=0 z<0)
 *   [+6..+7] : child offset for quadrant 2 (SW, x<0 z>=0)
 *   [+8..+9] : child offset for quadrant 3 (SE, x>=0 z>=0)
 * If child offset == 0 → no child.
 * If child offset & 0x8000 → leaf cell.
 * Otherwise → child base = current_base + offset * 10.
 *
 * A* search nodes (from the pool at DAT_800738a0) are 7 int32s = 28 bytes:
 *   node[0]    = next ptr (free list / open / closed list chaining)
 *   node[1]    = parent ptr
 *   node[2]    = cell base address (pointer into quadtree data)
 *   u16@node+12 = cell x coordinate (aligned to cell size)
 *   u16@node+14 = cell z coordinate (aligned to cell size)
 *   u8@node+16  = quadrant index within parent (0..3)
 *   u8@node+17  = cell depth / log2(cell size)
 *   node[5]    = f-score (g + heuristic)
 *   node[6]    = g-cost (path cost so far)
 *
 * The open list is kept sorted ascending by f-score using
 * ObjList_InsertSorted (FUN_80024748).
 *
 * MED confidence: direct Ghidra ref translation; semantics confirmed
 * by cross-referencing with standard A* + quadtree neighbor logic.
 */
#include <stdint.h>
#include <stddef.h>

/* Quadtree root: base address of the terrain navigation quadtree data.
 * Set by level load. */
extern uintptr_t iRam000006ec;

/* A* node free-list head (both aliases refer to PSX RAM address 0x7ec). */
extern int32_t    *piRam000007ec;   /* used as int*   (signed)  */
extern uint32_t   *puRam000007ec;   /* used as uint*  (unsigned) -- same address */

/* A* node pool: 0x400 nodes × 7 int32s (28 bytes) each. */
extern uint32_t DAT_800738a0[];

/* ObjList_InsertSorted (FUN_80024748) -- inserts param_2 into the
 * sorted list *param_1 and returns the new head. */
extern int *FUN_80024748(int *param_1, int *param_2);

/* ObjList_ClearFlags (FUN_8002479c) -- clears 0x6000 TPage bits on a chain. */
extern void FUN_8002479c(int *param_1);

/* GetFrameTickCount -- returns 32-bit frame tick counter. */
extern uint32_t FUN_80015010(void);

/* Heap_AllocOrRetry (FUN_800116f4) -- allocate or wait + retry. */
extern int FUN_800116f4(int size);

/* SquareRoot0 -- integer square root, from libgte. */
extern long SquareRoot0(long n);

static uint32_t *host_astar_node_pool(void)
{
    static uint32_t *pool = NULL;
    if (pool == NULL)
        pool = (uint32_t *)(uintptr_t)FUN_800116f4(0x400 * 7 * 4);
    return pool != NULL ? pool : DAT_800738a0;
}

/* ================================================================
 * FUN_800247dc -- QuadTree_DescendToCell
 *
 * Allocates a node from the free list and fills it with the
 * deepest quadtree cell containing (param_1, param_2).
 *
 * Traversal: converts (param_1, param_2) to a bit-stream by shifting
 * left by 0x15 (= 21) bits, then walks the quadtree consuming 2 bits
 * per level.  The bits determine which child (quadrant) to follow:
 *   bit 31 of shifted_x → 0/1 for x quadrant
 *   sign of shifted_z    → 0/2 for z quadrant (bit 1)
 *
 * Returns the allocated node (or NULL if free list is empty).
 *
 * HIGH confidence.
 * ================================================================ */
/* ================================================================
 * FUN_800244c4 -- QuadTree_CellAtPos
 *
 * Descends the terrain quadtree (root at iRam000006ec) using sign
 * bits of (param_1<<5) and (param_2<<5) to pick a quadrant at each
 * level. Stops when a child slot is 0 (out of map) or has bit 15
 * set (leaf). Returns the leaf code (or 0 if outside).
 *
 * Cell layout: header(2 bytes) then 4 u16 child offsets [+2, +4, +6, +8].
 * HIGH confidence: direct Ghidra port.
 * ================================================================ */
uint32_t FUN_800244c4(int param_1, int param_2)
{
    uint16_t uVar1;
    uint32_t uVar2;
    uint32_t uVar3;
    uintptr_t iVar4;

    uVar3   = (uint32_t)(param_1 << 5);
    param_2 = param_2 << 5;
    iVar4   = iRam000006ec;
    if (iVar4 == 0)
        return 0;
    while (1) {
        uVar2 = uVar3 >> 0x1f;                 /* x sign -> bit 0 */
        if (param_2 < 0) uVar2 |= 2u;          /* z sign -> bit 1 */
        uVar1 = *(uint16_t *)(iVar4 + uVar2 * 2 + 2);
        uVar2 = (uint32_t)uVar1;
        if (uVar2 == 0 || (uVar1 & 0x8000) != 0) break;
        iVar4   = iVar4 + (int)uVar2 * 10;
        uVar3   = uVar3 << 1;
        param_2 = param_2 << 1;
    }
    return uVar2;
}

int *FUN_800247dc(int param_1, int param_2)
{
    uint16_t uVar1;
    int      *piVar2;
    uint32_t  uVar3;
    uintptr_t iVar4;
    int       iVar5;
    uint32_t  uVar6;
    uint32_t  uVar7;

    piVar2 = piRam000007ec;
    if (piRam000007ec != NULL) {
        piRam000007ec = (int *)(uintptr_t)*piRam000007ec;
        puRam000007ec = (uint32_t *)piRam000007ec;
    }
    uVar7 = 0xb;
    if (piVar2 != NULL) {
        uVar6 = (uint32_t)(param_1 << 0x15);
        iVar5 = param_2 << 0x15;
        iVar4 = iRam000006ec;
        while (1) {
            uVar7 = uVar7 - 1;
            uVar3 = uVar6 >> 0x1f;
            if (iVar5 < 0)
                uVar3 = uVar3 | 2;
            uVar1 = *(uint16_t *)(iVar4 + (int)uVar3 * 2 + 2);
            if ((uVar1 == 0) || (uVar6 = uVar6 << 1, (uVar1 & 0x8000) != 0))
                break;
            iVar5 = iVar5 << 1;
            iVar4 = iVar4 + (uint32_t)uVar1 * 10;
        }
        uVar1 = (uint16_t)((-1) << (uVar7 & 0x1f));
        piVar2[2] = (int32_t)iVar4;
        *((uint8_t *)piVar2 + 0x10) = (uint8_t)uVar3;
        *((uint8_t *)piVar2 + 0x11) = (uint8_t)uVar7;
        *(uint16_t *)(piVar2 + 3) = (uint16_t)param_1 & uVar1;
        *(uint16_t *)((uint8_t *)piVar2 + 0xe) = (uint16_t)param_2 & uVar1;
    }
    return piVar2;
}

/* ================================================================
 * FUN_80024888 -- QuadTree_FindNeighborCell
 *
 * Given a node param_1 and target cell coordinates (param_2, param_3),
 * ascends the quadtree from param_1's cell until the path to the
 * target is covered, then descends to find the deepest cell containing
 * (param_2, param_3) at the appropriate resolution.
 *
 * Returns a new allocated node for the neighbor cell, or NULL if
 * the target is outside the tree or the free list is empty.
 *
 * MED confidence.
 * ================================================================ */
int *FUN_80024888(int param_1, uint32_t param_2, uint32_t param_3)
{
    uint16_t  uVar1;
    int       *piVar2;
    uint16_t  *puVar3;
    uint32_t   uVar4;
    int        iVar5;
    uint32_t   uVar6;
    uint32_t   uVar7;

    piVar2 = piRam000007ec;
    if (piRam000007ec != NULL) {
        piRam000007ec = (int *)(uintptr_t)*piRam000007ec;
        puRam000007ec = (uint32_t *)piRam000007ec;
        uVar7 = (uint32_t)*(uint8_t *)((uint8_t *)(uintptr_t)param_1 + 0x11);
        puVar3 = (uint16_t *)(uintptr_t)*(uint32_t *)((uint8_t *)(uintptr_t)param_1 + 8);
        iVar5 = (int)(*(uint16_t *)((uint8_t *)(uintptr_t)param_1 + 0xc) ^ param_2 |
                      *(uint16_t *)((uint8_t *)(uintptr_t)param_1 + 0xe) ^ param_3) >>
                (uVar7 & 0x1f);
        while (1) {
            iVar5 = iVar5 >> 1;
            uVar7 = uVar7 + 1;
            if (iVar5 == 0) break;
            if (*puVar3 == 0) {
                return (int *)NULL;
            }
            /* Ascending: puVar3 += offset * -5  (MIPS signed mult → go back one cell).
             * Each cell is 10 bytes = 5 ushorts; subtract to reach parent. */
            puVar3 -= (ptrdiff_t)*puVar3 * 5;
        }
        uVar6 = param_2 << (0x20 - uVar7 & 0x1f);
        iVar5 = (int)(param_3 << (0x20 - uVar7 & 0x1f));
        while (1) {
            uVar7 = uVar7 - 1;
            uVar4 = uVar6 >> 0x1f;
            if (iVar5 < 0)
                uVar4 = uVar4 | 2;
            uVar1 = puVar3[uVar4 + 1];
            if ((uVar1 == 0) || (uVar6 = uVar6 << 1, (uVar1 & 0x8000) != 0))
                break;
            iVar5 = iVar5 << 1;
            puVar3 = puVar3 + (uint32_t)uVar1 * 5;
        }
        uVar1 = (uint16_t)((-1) << (uVar7 & 0x1f));
        piVar2[2] = (int)(uintptr_t)puVar3;
        *((uint8_t *)piVar2 + 0x10) = (uint8_t)uVar4;
        *((uint8_t *)piVar2 + 0x11) = (uint8_t)uVar7;
        *(uint16_t *)(piVar2 + 3)           = (uint16_t)param_2 & uVar1;
        *(uint16_t *)((uint8_t *)piVar2 + 0xe) = (uint16_t)param_3 & uVar1;
    }
    return piVar2;
}

/* ================================================================
 * FUN_80024998 -- QuadTree_GetOpenNeighbors
 *
 * Returns a linked list of neighboring cells for param_1 that are
 * walkable (cell dirty-flag == 0).  Neighbor directions checked are
 * keyed by bits in the cell's tpage flags:
 *   0x100 → left  (x - 1): iterate z upward
 *   0x200 → right (x + cell_size): iterate z upward
 *   0x800 → back  (z - 1): iterate x rightward
 *   0x400 → fwd   (z + cell_size): iterate x rightward
 *   0x1000 → diagonal (from embedded direction bytes at cell+12..13)
 *
 * Each direction iterates through all neighboring cells that overlap
 * the current node's x or z range until the termination bound.
 *
 * Returns the head of the neighbor list (chained via node[0]).
 *
 * MED confidence.
 * ================================================================ */
uint32_t *FUN_80024998(int param_1)
{
    uint16_t   uVar1;
    uint32_t  *puVar2;
    uint32_t  *puVar4;
    int        iVar5;
    int        iVar3;

/* Helper macros to access node fields by byte offset. */
#define N_PTR8(n,off)   (*(uint8_t  *)((uint8_t *)(uintptr_t)(n) + (off)))
#define N_PTR16(n,off)  (*(uint16_t *)((uint8_t *)(uintptr_t)(n) + (off)))
#define N_PTR32(n,off)  (*(int32_t  *)((uint8_t *)(uintptr_t)(n) + (off)))

#define NODE_CELL(n)   ((uintptr_t)(uint32_t)N_PTR32(n, 8)) /* node[2] */
#define NODE_X(n)      (N_PTR16(n, 0xc))        /* cell x coord */
#define NODE_Z(n)      (N_PTR16(n, 0xe))        /* cell z coord */
#define NODE_QUAD(n)   (N_PTR8 (n, 0x10))       /* quadrant idx */
#define NODE_DEPTH(n)  (N_PTR8 (n, 0x11))       /* cell depth   */
#define NODE_CSIZE(n)  (1u << (NODE_DEPTH(n) & 0x1f))  /* cell size */
#define NODE_DIRTY(n)  (*(int16_t *)(NODE_CELL(n) + NODE_QUAD(n) * 2 + 2))

    uVar1 = *(uint16_t *)(NODE_CELL(param_1) + (uint32_t)N_PTR8(param_1, 0x10) * 2 + 2);
    puVar4 = NULL;
    uint16_t flags = (uVar1 != 0) ? uVar1 : (uint16_t)0xf00;

    /* ---- 0x100: left neighbor (x - 1), iterate z upward ---- */
    if ((flags & 0x100) != 0) {
        uint32_t tx  = (uint32_t)NODE_X(param_1) - 1;
        uint32_t z   = (uint32_t)NODE_Z(param_1);
        iVar5 = (int)((uint32_t)NODE_Z(param_1) + NODE_CSIZE(param_1));
        puVar2 = (uint32_t *)FUN_80024888(param_1, tx, z);
        while (puVar2 != NULL) {
            if (NODE_DIRTY(puVar2) != 0) { *puVar2 = (uint32_t)(uintptr_t)puVar4; puVar4 = puVar2; }
            z = (uint32_t)NODE_Z(puVar2) + NODE_CSIZE(puVar2);
            if (iVar5 <= (int)z) break;
            puVar2 = (uint32_t *)FUN_80024888((int)(uintptr_t)puVar2, tx, z);
        }
    }

    /* ---- 0x200: right neighbor (x + cell_size), iterate z upward ---- */
    if ((flags & 0x200) != 0) {
        uint32_t tx  = (uint32_t)NODE_X(param_1) + NODE_CSIZE(param_1);
        uint32_t z   = (uint32_t)NODE_Z(param_1);
        iVar5 = (int)((uint32_t)NODE_Z(param_1) + NODE_CSIZE(param_1));
        puVar2 = (uint32_t *)FUN_80024888(param_1, tx, z);
        while (puVar2 != NULL) {
            if (NODE_DIRTY(puVar2) != 0) { *puVar2 = (uint32_t)(uintptr_t)puVar4; puVar4 = puVar2; }
            z = (uint32_t)NODE_Z(puVar2) + NODE_CSIZE(puVar2);
            if (iVar5 <= (int)z) break;
            puVar2 = (uint32_t *)FUN_80024888((int)(uintptr_t)puVar2, tx, z);
        }
    }

    /* ---- 0x800: back neighbor (z - 1), iterate x rightward ---- */
    iVar5 = (int)((uint32_t)NODE_X(param_1) + NODE_CSIZE(param_1));
    if ((flags & 0x800) != 0) {
        uint32_t tz  = (uint32_t)NODE_Z(param_1) - 1;
        iVar3 = (int)(uint32_t)NODE_X(param_1);
        puVar2 = (uint32_t *)FUN_80024888(param_1, (uint32_t)iVar3, tz);
        while (puVar2 != NULL) {
            if (NODE_DIRTY(puVar2) != 0) { *puVar2 = (uint32_t)(uintptr_t)puVar4; puVar4 = puVar2; }
            iVar3 = (int)((uint32_t)NODE_X(puVar2) + NODE_CSIZE(puVar2));
            if (iVar5 <= iVar3) break;
            puVar2 = (uint32_t *)FUN_80024888((int)(uintptr_t)puVar2, (uint32_t)iVar3, tz);
        }
    }

    /* ---- 0x400: forward neighbor (z + cell_size), iterate x rightward ---- */
    if ((flags & 0x400) != 0) {
        uint32_t tz  = (uint32_t)NODE_Z(param_1) + NODE_CSIZE(param_1);
        iVar3 = (int)(uint32_t)NODE_X(param_1);
        puVar2 = (uint32_t *)FUN_80024888(param_1, (uint32_t)iVar3, tz);
        while (puVar2 != NULL) {
            if (NODE_DIRTY(puVar2) != 0) { *puVar2 = (uint32_t)(uintptr_t)puVar4; puVar4 = puVar2; }
            iVar3 = (int)((uint32_t)NODE_X(puVar2) + NODE_CSIZE(puVar2));
            if (iVar5 <= iVar3) break;
            puVar2 = (uint32_t *)FUN_80024888((int)(uintptr_t)puVar2, (uint32_t)iVar3, tz);
        }
    }

    /* ---- 0x1000: diagonal neighbor (direction bytes embedded in cell data) ---- */
    if ((flags & 0x1000) != 0) {
        uintptr_t iBase = NODE_CELL(param_1) + 10 + (int)(uint32_t)N_PTR8(param_1, 0x10) * 2;
        puVar2 = (uint32_t *)FUN_80024888(param_1,
                 (uint32_t)((int)NODE_X(param_1) + (int)*(int8_t *)(iBase + 2)),
                 (uint32_t)((int)NODE_Z(param_1) + (int)*(int8_t *)(iBase + 3)));
        if (puVar2 != NULL && NODE_DIRTY(puVar2) != 0) {
            *puVar2 = (uint32_t)(uintptr_t)puVar4;
            puVar4 = puVar2;
        }
    }

#undef N_PTR8
#undef N_PTR16
#undef N_PTR32
#undef NODE_CELL
#undef NODE_X
#undef NODE_Z
#undef NODE_QUAD
#undef NODE_DEPTH
#undef NODE_CSIZE
#undef NODE_DIRTY

    return puVar4;
}

/* ================================================================
 * FUN_80024cc0 -- QuadTree_CellDistance
 *
 * Computes the heuristic distance from the centre of the cell
 * described by param_1 to the point (param_2, param_3) in 2D.
 *
 * Returns sqrt((cx_centre - param_2)^2 + (cz_centre - param_3)^2) * 128.
 *
 * HIGH confidence.
 * ================================================================ */
int FUN_80024cc0(int param_1, int16_t param_2, int16_t param_3)
{
    int32_t iVar2;
    int32_t iVar3;

    iVar2 = (1 << (*(uint8_t *)((uint8_t *)(uintptr_t)param_1 + 0x11) & 0x1f)) / 2;
    iVar3 = (int)((uint32_t)*(uint16_t *)((uint8_t *)(uintptr_t)param_1 + 0xc) + (uint32_t)iVar2) - (int)param_2;
    iVar2 = (int)((uint32_t)*(uint16_t *)((uint8_t *)(uintptr_t)param_1 + 0xe) + (uint32_t)iVar2) - (int)param_3;
    long lVar1 = SquareRoot0(iVar3 * iVar3 + iVar2 * iVar2);
    return (int)(lVar1 << 7);
}

/* ================================================================
 * FUN_80024d54 -- QuadTree_AStarPath
 *
 * A* pathfinder from param_1 to param_2 on the terrain quadtree.
 *
 * param_1: pointer to destination record {... i16 x @+2, i16 z @+0xa ...}
 * param_2: pointer to source record (same layout)
 * param_3: time budget (in FUN_80015010 ticks); 0 = unlimited
 * param_4: early-exit flag (1 = stop when over budget even w/o path)
 *
 * Returns a Heap_AllocOrRetry'd waypoint array:
 *   [0,1]  = destination (x,z) as i16 pair
 *   [2,3]  = intermediate waypoints in reverse order (destination first)
 *   ...
 *   [2n-1] = last waypoint pair
 *   [2n,2n+1] = 0,0 sentinel
 * Returns 0 if no path found.
 *
 * MED confidence: faithful translation of Ghidra ref.
 * ================================================================ */
int FUN_80024d54(intptr_t param_1, intptr_t param_2, uint32_t param_3, int param_4)
{
    uint16_t   uVar1;
    int        bVar2;
    uint32_t  *puVar3;
    int        iVar4;
    uint32_t  *puVar5;
    uint32_t  *puVar6;
    uint32_t  *puVar7;
    uint32_t   uVar8;
    uint32_t  *puVar9;
    uint16_t  *puVar10;
    int        iVar11;
    int16_t   *psVar12;
    uint32_t  *puVar13;
    uint32_t  *puVar14;
    int        iVar15;
    uint32_t  *puVar16;
    uintptr_t  uPtr;

    /* Initialise the node pool free list. */
    iVar4 = (int)FUN_80015010();
    iVar15 = 0x3fe;
    {
        uint32_t *pool = host_astar_node_pool();
        puRam000007ec = &pool[0];
        piRam000007ec = (int32_t *)&pool[0];
        puVar6 = &pool[0];
    }
    do {
        puVar16 = puVar6 + 7;
        *puVar6  = (uint32_t)(uintptr_t)puVar16;
        iVar15   = iVar15 - 1;
        puVar6   = puVar16;
    } while (-1 < iVar15);
    *puVar16 = 0;

    /* Find the source cell (param_2) and goal cell (param_1). */
    puVar5 = (uint32_t *)FUN_800247dc((int)*(int16_t *)(param_2 + 2),
                                       (int)*(int16_t *)(param_2 + 10));
    puVar6 = puVar5;
    if (*(int16_t *)((uintptr_t)puVar5[2] + (uint32_t)*((uint8_t *)puVar5 + 0x10) * 2 + 2) == 0) {
        puVar6 = FUN_80024998((int)(uintptr_t)puVar5);
        if (puVar6 == NULL) {
            return 0;
        }
        *puVar5 = (uint32_t)(uintptr_t)puRam000007ec;
        puRam000007ec = puVar5;
        piRam000007ec = (int32_t *)puRam000007ec;
    }

    puVar7 = (uint32_t *)FUN_800247dc((int)*(int16_t *)(param_1 + 2),
                                       (int)*(int16_t *)(param_1 + 10));
    puVar5 = puVar7;
    if (*(int16_t *)((uintptr_t)puVar7[2] + (uint32_t)*((uint8_t *)puVar7 + 0x10) * 2 + 2) == 0) {
        puVar5 = FUN_80024998((int)(uintptr_t)puVar7);
        if (puVar5 == NULL) {
            return 0;
        }
        *puVar7 = (uint32_t)(uintptr_t)puRam000007ec;
        puRam000007ec = puVar7;
        piRam000007ec = (int32_t *)puRam000007ec;
    }

    /* Initialise the start node. */
    puVar5[6] = 0;   /* g-cost = 0 */
    uVar8 = (uint32_t)FUN_80024cc0((int)(uintptr_t)puVar16,
                                    (int16_t)*(uint16_t *)((uint8_t *)puVar6 + 0xc - 4),
                                    (int16_t)*(uint16_t *)((uint8_t *)puVar6 + 0xe - 4));
    puVar5[5] = uVar8;  /* f-score = heuristic */
    *puVar5   = 0;      /* next = NULL */
    puVar5[1] = 0;      /* parent = NULL */
    puVar16 = NULL;     /* closed list head */

    /* A* main loop. */
    while (puVar5 != NULL) {
        iVar15 = (int)FUN_80015010();
        bVar2  = (int)(param_3 < (uint32_t)(iVar15 - iVar4));
        if (bVar2 && param_4 != 0) break;

        puVar7 = (uint32_t *)(uintptr_t)*puVar5;   /* rest of open list */

        if (bVar2 ||
            (puVar5[2] == puVar6[2] &&
             *((uint8_t *)puVar5 + 0x10) == *((uint8_t *)puVar6 + 0x10))) {
            /* ---- Path found (or timeout): reconstruct waypoints ---- */
            uPtr = (uintptr_t)puVar5[2] + (uint32_t)*((uint8_t *)puVar5 + 0x10) * 2;
            *(uint16_t *)(uPtr + 2) = *(uint16_t *)(uPtr + 2) & 0x9fff;

            /* Count path length. */
            iVar15 = 0;
            for (uPtr = (uintptr_t)puVar5[1]; uPtr != 0; uPtr = *(uint32_t *)(uPtr + 4)) {
                iVar15 = iVar15 + 1;
            }
            iVar15 = iVar15 - (int)(iVar15 != 0);

            /* Allocate waypoint array: (iVar15 + 2) × 4 bytes (i16 pairs). */
            iVar4   = FUN_800116f4((iVar15 + 2) * 4);
            puVar10 = (uint16_t *)(uintptr_t)(iVar15 * 4 + iVar4);

            /* Sentinel at end. */
            puVar10[3] = 0;
            puVar10[2] = 0;

            /* Write destination as first waypoint. */
            *puVar10 = *(uint16_t *)(param_2 + 2);
            iVar15   = iVar15 - 1;
            puVar10[1] = *(uint16_t *)(param_2 + 10);

            /* Write intermediate waypoints in reverse order. */
            if (iVar15 != -1) {
                psVar12 = (int16_t *)(uintptr_t)(iVar15 * 4 + iVar4);
                do {
                    puVar5   = (uint32_t *)(uintptr_t)puVar5[1];
                    *psVar12 = *(int16_t *)(puVar5 + 3) +
                               (int16_t)((1 << (*((uint8_t *)puVar5 + 0x11) & 0x1f)) / 2);
                    iVar15   = iVar15 - 1;
                    psVar12[1] = *(int16_t *)((uint8_t *)puVar5 + 0xe) +
                                 (int16_t)((1 << (*((uint8_t *)puVar5 + 0x11) & 0x1f)) / 2);
                    psVar12 = psVar12 - 2;
                } while (iVar15 != -1);
            }
            FUN_8002479c((int *)puVar7);
            FUN_8002479c((int *)puVar16);
            return iVar4;
        }

        /* ---- Expand neighbors ---- */
        puVar9 = FUN_80024998((int)(uintptr_t)puVar5);

    joined_r0x80025008:
        puVar3 = puVar9;
        if (puVar3 != NULL) {
            puVar9 = (uint32_t *)(uintptr_t)*puVar3;

            /* Cost through current node to this neighbor. */
            puVar3[6] = puVar5[6] +
                        ((uint32_t)*((uint8_t *)((uintptr_t)puVar3[2] + (uint32_t)*((uint8_t *)puVar3 + 0x10) * 2 + 2)) <<
                         (*((uint8_t *)puVar3 + 0x11) & 0x1f));

            uPtr = (uintptr_t)puVar3[2] + (uint32_t)*((uint8_t *)puVar3 + 0x10) * 2;
            uVar1  = *(uint16_t *)(uPtr + 2);
            puVar14 = NULL;

            if ((uVar1 & 0x4000) != 0) goto code_r0x80025060;

            /* New node: mark as open, compute heuristic, insert into open list. */
            *(uint16_t *)(uPtr + 2) = uVar1 | 0x4000;
            iVar15 = FUN_80024cc0((int)(uintptr_t)puVar3,
                                   (int16_t)*(uint16_t *)((uint8_t *)puVar6 + 0xc - 4),
                                   (int16_t)*(uint16_t *)((uint8_t *)puVar6 + 0xe - 4));
            puVar3[1] = (uint32_t)(uintptr_t)puVar5;
            puVar3[5] = puVar3[6] + (uint32_t)iVar15;
            /* Insert into sorted open list. */
            puVar7 = (uint32_t *)FUN_80024748((int *)puVar7, (int *)puVar3);
            goto joined_r0x80025008;
        }

        /* ---- No more neighbors: move current node to closed list ---- */
        *puVar5 = (uint32_t)(uintptr_t)puVar16;
        uPtr = (uintptr_t)puVar5[2] + (uint32_t)*((uint8_t *)puVar5 + 0x10) * 2;
        *(uint16_t *)(uPtr + 2) = *(uint16_t *)(uPtr + 2) | 0x6000;
        puVar16 = puVar5;
        puVar5  = puVar7;
        continue;

    code_r0x80025060:
        /* Neighbor already visited. Check if the new path is better. */
        puVar13 = puVar7;
        if ((uVar1 & 0x2000) != 0) {
            puVar13 = puVar16;
        }
        /* Find puVar3 in the appropriate list (open or closed). */
        if ((puVar3[2] != puVar13[2]) || (*((uint8_t *)puVar3 + 0x10) != *((uint8_t *)puVar13 + 0x10))) {
            do {
                do {
                    puVar14  = puVar13;
                    puVar13  = (uint32_t *)(uintptr_t)*puVar14;
                } while (puVar3[2] != puVar13[2]);
            } while (*((uint8_t *)puVar3 + 0x10) != *((uint8_t *)puVar13 + 0x10));
        }

        /* Release the new node back to free list (puVar13 is already there). */
        *puVar3 = (uint32_t)(uintptr_t)puRam000007ec;
        iVar11  = (int)puVar13[6];   /* old g-cost in list */
        iVar15  = (int)puVar3[6];    /* new (lower) g-cost */
        puRam000007ec = puVar3;
        piRam000007ec = (int32_t *)puRam000007ec;

        if (0 < iVar11 - iVar15) {
            /* New path is better: re-activate puVar13 with updated cost. */
            if (puVar14 == NULL) {
                if ((uVar1 & 0x2000) == 0)
                    puVar7  = (uint32_t *)(uintptr_t)*puVar13;
                else
                    puVar16 = (uint32_t *)(uintptr_t)*puVar13;
            } else {
                *puVar14 = *puVar13;
            }
            *(uint16_t *)((uintptr_t)puVar13[2] + (uint32_t)*((uint8_t *)puVar13 + 0x10) * 2 + 2) =
                uVar1 & 0xdfff;
            uVar8 = puVar3[6];
            puVar13[1] = (uint32_t)(uintptr_t)puVar5;
            puVar13[6] = uVar8;
            puVar13[5] = puVar13[5] - (uint32_t)(iVar11 - iVar15);
            /* Re-insert into sorted open list. */
            puVar7 = (uint32_t *)FUN_80024748((int *)puVar7, (int *)puVar13);
        }
        goto joined_r0x80025008;
    }

    /* No path found. */
    FUN_8002479c((int *)puVar5);
    FUN_8002479c((int *)puVar16);
    return 0;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_800247dc  (from analysis/SLUS_005.10/decomp/800247dc.c) --- */
// addr: 0x800247dc  name: FUN_800247dc

int * FUN_800247dc(int param_1,int param_2)

{
  ushort uVar1;
  int *piVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;

  piVar2 = piRam000007ec;
  if (piRam000007ec != (int *)0x0) {
    piRam000007ec = (int *)*piRam000007ec;
  }
  uVar7 = 0xb;
  if (piVar2 != (int *)0x0) {
    uVar6 = param_1 << 0x15;
    iVar5 = param_2 << 0x15;
    iVar4 = iRam000006ec;
    while( true ) {
      uVar7 = uVar7 - 1;
      uVar3 = uVar6 >> 0x1f;
      if (iVar5 < 0) {
        uVar3 = uVar3 | 2;
      }
      uVar1 = *(ushort *)(iVar4 + uVar3 * 2 + 2);
      if ((uVar1 == 0) || (uVar6 = uVar6 << 1, (uVar1 & 0x8000) != 0)) break;
      iVar5 = iVar5 << 1;
      iVar4 = iVar4 + (uint)uVar1 * 10;
    }
    uVar1 = (ushort)(-1 << (uVar7 & 0x1f));
    piVar2[2] = iVar4;
    *(char *)(piVar2 + 4) = (char)uVar3;
    *(char *)((int)piVar2 + 0x11) = (char)uVar7;
    *(ushort *)(piVar2 + 3) = (ushort)param_1 & uVar1;
    *(ushort *)((int)piVar2 + 0xe) = (ushort)param_2 & uVar1;
  }
  return piVar2;
}

/* --- SLUS_005.10 FUN_80024888  (from analysis/SLUS_005.10/decomp/80024888.c) --- */
// addr: 0x80024888  name: FUN_80024888
/* (see analysis/SLUS_005.10/decomp/80024888.c) */

/* --- SLUS_005.10 FUN_80024998  (from analysis/SLUS_005.10/decomp/80024998.c) --- */
// addr: 0x80024998  name: FUN_80024998
/* (see analysis/SLUS_005.10/decomp/80024998.c) */

/* --- SLUS_005.10 FUN_80024cc0  (from analysis/SLUS_005.10/decomp/80024cc0.c) --- */
// addr: 0x80024cc0  name: FUN_80024cc0

int FUN_80024cc0(int param_1,short param_2,short param_3)

{
  long lVar1;
  int iVar2;
  int iVar3;

  iVar2 = (1 << (*(byte *)(param_1 + 0x11) & 0x1f)) / 2;
  iVar3 = ((uint)*(ushort *)(param_1 + 0xc) + iVar2) - (int)param_2;
  iVar2 = ((uint)*(ushort *)(param_1 + 0xe) + iVar2) - (int)param_3;
  lVar1 = SquareRoot0(iVar3 * iVar3 + iVar2 * iVar2);
  return lVar1 << 7;
}

/* --- SLUS_005.10 FUN_80024d54  (from analysis/SLUS_005.10/decomp/80024d54.c) --- */
// addr: 0x80024d54  name: FUN_80024d54
/* (see analysis/SLUS_005.10/decomp/80024d54.c) */

#endif  /* GHIDRA REF */
