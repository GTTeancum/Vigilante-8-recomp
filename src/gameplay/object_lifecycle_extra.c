/* object_lifecycle_extra.c -- Object/bone-bank lifecycle helpers.
 *
 * Source: SLUS_005.10
 *   FUN_8001fe8c  -- Object_UnlinkFromList: remove one object node from a
 *                    doubly-linked scene list and recycle the node.
 *   FUN_8001bda0  -- BoneBank_AllocByIndex: allocate a bone-bank slot by index.
 *   FUN_8001bddc  -- BoneBank_Free: free a display-list bone bank (deferred
 *                    for the current page, immediate for the other).
 *   FUN_800118b4  -- DeferFree_Enqueue: insert a buffer into the per-page
 *                    deferred-free queue (freed after GPU DrawSync clears page).
 *   FUN_800203fc  -- Object_UnregisterFromScene: fire event 4, remove from
 *                    all active scene lists based on flag bits.
 *   FUN_800204dc  -- Object_HeapFreeRecursive: unregister + free a full object
 *                    subtree (iterates siblings, recurses into children).
 *   FUN_80020540  -- Object_Free: bone-bank + optional physics teardown + recurse.
 *   FUN_8003e2c4  -- BoneBankSlot_Free: free a single embedded bone-bank slot.
 *
 * HIGH confidence: direct Ghidra port.
 */
#include <stdint.h>

/* ---- heap / display-list allocators ---- */
extern int  FUN_800116f4(int size);           /* Heap_AllocOrRetry */
extern void FUN_80045088(int p);              /* Heap_Free (int alias) */
extern void *Host_HeapBase(void);
extern uint32_t Host_HeapSize(void);

/* ---- bone-bank subsystem ---- */
extern uint16_t *FUN_8001b49c(int *param_1, uint32_t slot); /* Bone_AllocLevel */
extern int32_t iRam00000004;                             /* current DL page (0 or 1) */
extern void FUN_800118b4(int param_1);                   /* DeferFree_Enqueue (forward) */

/* ---- scene lists ---- */
/* Object scene-list heads (4-byte PSX pointer values; declared as uint8_t[]
 * to match the panic_stubs.c definitions; cast to uint32_t* on use). */
extern uint8_t DAT_80065a60[];  /* back-buffer pending list    (flag bit 0x80) */
extern uint8_t DAT_80065a80[];  /* physics active list          (flag bit 0x04) */
extern uint8_t DAT_80065ac0[];  /* draw/tick active list        (flag bit 0x01) */
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

/* Node recycler head: piRam00000774 points to the free-node pool head.
 * Object_UnlinkFromList prepends unlinked nodes here. */
extern int32_t *piRam00000774;
/* DAT_80065a74: scene-list sentinel (inserted into recycled nodes). */
extern uint32_t DAT_80065a74;

/* ---- per-page deferred-free ring sentinels ---- */
/* These live at PSX RAM 0x8006eca4 / 0x8006eca8 (two pages × 3 words each).
 * On host they are independent globals; the code indexes via [iVar1 * 3]
 * which is valid for these 8-element arrays. */
uint32_t DAT_8006eca4[8] = {0};   /* sentinel next/prev/data for pages 0 and 1 */
uint32_t DAT_8006eca8[8] = {0};   /* tail pointer for pages 0 and 1 */

/* ---- physics / weapon slot teardown ---- */
extern void FUN_8003e2c4(uint32_t *param_1);   /* BoneBankSlot_Free (defined below) */
void FUN_80020540(uint32_t *param_1);

static int host_low_heap_ptr(uint32_t p)
{
    uintptr_t base = (uintptr_t)Host_HeapBase();
    uintptr_t end = base + Host_HeapSize();
    return (uintptr_t)p >= base && (uintptr_t)p < end;
}

/* ================================================================
 * FUN_800118b4  -- DeferFree_Enqueue
 *
 * Insert param_1 (a buffer handle) into the deferred-free ring for
 * the current display-list page (iRam00000004).  The buffer is
 * actually freed once DrawSync signals that the GPU is done with
 * this page.
 *
 * Ring layout per page (stride 3 × uint32_t = 12 bytes):
 *   sentinel[0] = next ptr
 *   sentinel[1] = prev ptr   (at +4 = DAT_8006eca8 base)
 *   sentinel[2] = data (unused for sentinel)
 * ================================================================ */
void FUN_800118b4(int param_1)
{
    uint32_t *puVar2;
    uint32_t *puVar3;
    int       iVar1;

    puVar2 = (uint32_t *)(uintptr_t)FUN_800116f4(0xc);
    iVar1  = iRam00000004;
    puVar2[2] = (uint32_t)param_1;
    /* DAT_8006eca8[iVar1 * 3] holds the tail-node pointer for this page's
     * deferred-free ring.  We prepend puVar2 before the sentinel at eca4. */
    puVar3 = (uint32_t *)(uintptr_t)DAT_8006eca8[iVar1 * 3];
    DAT_8006eca8[iVar1 * 3] = (uint32_t)(uintptr_t)puVar2;
    *puVar3 = (uint32_t)(uintptr_t)puVar2;
    puVar2[1] = (uint32_t)(uintptr_t)puVar3;
    *puVar2   = (uint32_t)(uintptr_t)(&DAT_8006eca4[iVar1 * 3]);
}

/* ================================================================
 * FUN_8001bda0  -- BoneBank_AllocByIndex
 *
 * Allocate a display-list bone bank for the slot at param_2 inside
 * the bank array pointed to by param_1.
 *
 * Slot table layout (per entry, stride 0x1c bytes):
 *   +0x1c : uint16 -- slot descriptor (lower 11 bits = Bone_AllocLevel arg)
 * ================================================================ */
uintptr_t FUN_8001bda0(int *param_1, uint32_t param_2)
{
    return (uintptr_t)FUN_8001b49c(param_1,
        *(uint16_t *)(uintptr_t)(*param_1 + (int)(param_2 & 0xffffu) * 0x1c + 0x1c) & 0x7ff);
}

/* ================================================================
 * FUN_8001bddc  -- BoneBank_Free
 *
 * Free a display-list bone bank handle.
 *
 * Layout at param_1 (base of bank struct):
 *   +0x1c + page*4 : int -- DL buffer for page 0/1
 *     current page buffer → DeferFree_Enqueue (GPU may still use it)
 *     other   page buffer → Heap_Free (GPU is done)
 *
 * Uses iRam00000004 (0 or 1) as the current display-list page.
 * ================================================================ */
void FUN_8001bddc(int param_1)
{
    if (param_1 != 0) {
        int currentPageBuf = *(int *)(uintptr_t)(param_1 + iRam00000004 * 4 + 0x1c);
        int otherPageBuf = *(int *)(uintptr_t)(param_1 + (1 - iRam00000004) * 4 + 0x1c);

        if (currentPageBuf != 0 && host_low_heap_ptr((uint32_t)currentPageBuf)) {
            /* Host renderer does not consume the PSX deferred-free ring, and
             * some display-list slots currently contain renderer/fixed-point
             * data rather than heap-owned buffers. Free only heap-backed
             * buffers until the renderer DL allocator is fully source-backed. */
            FUN_80045088(currentPageBuf);
        }
        if (otherPageBuf != 0 && host_low_heap_ptr((uint32_t)otherPageBuf)) {
            FUN_80045088(otherPageBuf);
        }
        FUN_80045088(param_1);
    }
}

/* ================================================================
 * FUN_8001fe8c  -- Object_UnlinkFromList
 *
 * Walk the doubly-linked scene list rooted at *param_1, find the
 * node whose [2] field equals param_2 (the object pointer), unlink
 * it, and prepend it to the free-node pool headed by piRam00000774.
 *
 * Node layout (three int32_t words):
 *   [0] = next node ptr
 *   [1] = prev node ptr
 *   [2] = object ptr (key)
 *
 * Returns 1 if found and unlinked, 0 if not present.
 * ================================================================ */
int FUN_8001fe8c(uint32_t *param_1, int param_2)
{
    typedef struct HostObjListNode {
        struct HostObjListNode *next;
        struct HostObjListNode *prev;
        uintptr_t payload;
        uint32_t deadline;
    } HostObjListNode;
    HostObjListNode *sentinel = (HostObjListNode *)param_1;
    HostObjListNode *node;

    if (sentinel == NULL || sentinel->prev == NULL)
        return 0;
    for (node = sentinel->next; node != NULL; node = node->next) {
        if (node->payload == (uintptr_t)(uint32_t)param_2)
            break;
    }
    if (node == NULL)
        return 0;

    if (node->prev != NULL)
        node->prev->next = node->next;
    if (node->next != NULL)
        node->next->prev = node->prev;
    if (sentinel->next == node)
        sentinel->next = node->next;
    if (sentinel->prev == node)
        sentinel->prev = (node->prev == sentinel) ? sentinel : node->prev;
    node->next = NULL;
    node->prev = NULL;
    node->payload = 0;
    return 1;
}

/* ================================================================
 * FUN_800203fc  -- Object_UnregisterFromScene
 *
 * Dispatch event 4 ("about to die") to the object's tick callback,
 * then remove it from whichever scene lists it is registered in
 * (based on the flag bits in obj[0]):
 *   bit 0x80 → remove from DAT_80065a60 (back-buffer pending)
 *   bit 0x04 → remove from DAT_80065a80 (physics active)
 *   bit 0x01 → remove from DAT_80065ac0 (draw/tick active)
 * ================================================================ */
uint32_t *FUN_800203fc(uint32_t *param_1)
{
    typedef void (*TickFn)(uint32_t *, int, intptr_t);
    TickFn cb;

    cb = (TickFn)Object_CallbackFromPsxSlot(param_1);
    if (cb != (TickFn)0) {
        cb(param_1, 4, 0);
    }
    if ((*param_1 & 0x80u) != 0) {
        FUN_8001fe8c((uint32_t *)DAT_80065a60, (int)(uintptr_t)param_1);
    }
    if ((*param_1 & 4u) != 0) {
        FUN_8001fe8c((uint32_t *)DAT_80065a80, (int)(uintptr_t)param_1);
    }
    if ((*param_1 & 1u) != 0) {
        FUN_8001fe8c((uint32_t *)DAT_80065ac0, (int)(uintptr_t)param_1);
    }
    return param_1;
}

/* ================================================================
 * FUN_800204dc  -- Object_HeapFreeRecursive
 *
 * Walk the sibling chain (each node's +0x34 field) freeing every
 * object.  For each node: unregister from scene, free the bone bank
 * at +0x30, recurse into children at +0x38, then Heap_Free self.
 *
 * Object struct offsets (in uint32_t words unless noted):
 *   +0x30 : bone bank handle (int)
 *   +0x34 : next sibling ptr (int) -- used to advance after free
 *   +0x38 : first child ptr  (int) -- recursed
 * ================================================================ */
void FUN_800204dc(int param_1)
{
    int iVar1;

    while (param_1 != 0) {
        FUN_800203fc((uint32_t *)(uintptr_t)param_1);
        FUN_8001bddc(*(int *)(uintptr_t)(param_1 + 0x30));
        FUN_800204dc(*(int *)(uintptr_t)(param_1 + 0x38));  /* children */
        iVar1 = *(int *)(uintptr_t)(param_1 + 0x34);        /* next sibling */
        FUN_80045088(param_1);
        param_1 = iVar1;
    }
}

/* Public aliases */
void Object_HeapFree(int p)            { FUN_800204dc(p); }
void Vehicle_Free(uint32_t bin)        { FUN_800204dc((int)(uintptr_t)bin); }
void Object_FreeAndUnregister(uint32_t obj)
{
    FUN_800204dc((int)(uintptr_t)obj);
}

int Object_Destroy(int handle)
{
    FUN_80020540((uint32_t *)(uintptr_t)(uint32_t)handle);
    return 0;
}

/* ================================================================
 * FUN_8003e2c4  -- BoneBankSlot_Free
 *
 * Free a single embedded bone-bank slot struct:
 *   slot[0] = bone bank handle → BoneBank_Free
 *   then free the slot struct itself.
 * ================================================================ */
void FUN_8003e2c4(uint32_t *param_1)
{
    if (param_1 != (uint32_t *)0) {
        FUN_8001bddc((int)(uintptr_t)*param_1);
        FUN_80045088((int)(uintptr_t)param_1);
    }
}

/* ================================================================
 * FUN_80020540  -- Object_Free  (= TriggerVol_FreeOne)
 *
 * High-level object destructor:
 *   1. If obj[0x1a] != 0 → BoneBank_Free (secondary bone bank)
 *   2. If obj flag bit 0x08 set → BoneBankSlot_Free(obj[0x1c])
 *   3. Recursively free self + subtree via Object_HeapFreeRecursive.
 * ================================================================ */
void FUN_80020540(uint32_t *param_1)
{
    if (param_1[0x1a] != 0 && host_low_heap_ptr(param_1[0x1a])) {
        FUN_8001bddc((int)(uintptr_t)param_1[0x1a]);
    }
    if ((*param_1 & 8u) != 0 && host_low_heap_ptr(param_1[0x1c])) {
        FUN_8003e2c4((uint32_t *)(uintptr_t)param_1[0x1c]);
    }
    FUN_800204dc((int)(uintptr_t)param_1);
}

/* Readable-name aliases */
void TriggerVol_FreeOne(int *node) { FUN_80020540((uint32_t *)node); }
