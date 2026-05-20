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
extern void  Font_BuildTexture(void *tim);     /* FUN_8001884c -- uploads via LoadImage */
extern void  Audio_FreeSND(void *bank);         /* FUN_80044394 */
extern void  Object_FreeAndUnhook(void *p);
extern void  Object_BoneTouchUp(int *parent, uint16_t idx);  /* FUN_8001b3d4 */

/* HIGH: build the in-memory Object struct from a parsed
 * { template[], stride 0xc, count obj[4] } pair. */
uint32_t *Object_BuildFromBin(int *templateBody, void *animPtr)
{
    uint32_t *obj = (uint32_t *)Heap_AllocOrRetry(templateBody[4] * 0xc + 0xc);
    obj[0] = (uintptr_t)templateBody;
    obj[1] = (uintptr_t)animPtr;
    obj[2] = 0;
    /* Convert the in-template offset at +1 from relative to absolute. */
    templateBody[1] = (int)(uintptr_t)templateBody + templateBody[1];
    /* The original then loops per-prim copying defaults; omitted for the
     * pass-2 summary view. Pass 3 finishes the body. */
    return obj;
}

/* HIGH: walk the object's prim list, uploading each TIM atlas. */
void Object_FinishBuild(int **obj)
{
    if (obj[0] == NULL) return;
    int n = *(int *)((uintptr_t)obj[0] + 0x10);
    if (n <= 0) return;
    for (int i = 0, off = 0xc; i < n; i++, off += 0xc) {
        Font_BuildTexture((uint8_t *)obj + off);
    }
}

/* HIGH: per-prim bone touch-up + Heap_Realloc shrink. */
void Object_RegisterInChain(int **obj)
{
    int *hdr = (int *)obj[0];
    uint32_t n = (uint32_t)hdr[0x10 / 4];
    for (uint32_t i = 0; i < n; i++) {
        Object_BoneTouchUp(obj, (uint16_t)i);
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
