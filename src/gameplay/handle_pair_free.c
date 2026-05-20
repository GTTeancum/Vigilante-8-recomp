/* handle_pair_free.c -- free an "indirect handle + payload" pair.
 *
 * Source: SLUS_005.10  FUN_8003e2c4.
 *
 * Common pattern: a handle is allocated as a small heap block holding
 * a pointer (+0) and a bit of metadata. When freed, the indirect
 * target is released (via FUN_8001bddc -- still in auto/) before the
 * handle wrapper itself. FUN_8001bddc is a generic "free this object
 * and unhook its draw-OT references" -- pass 2 will rename it.
 *
 * HIGH confidence.
 */
#include <stdint.h>

extern void Object_FreeAndUnhook(void *p);   /* FUN_8001bddc */
extern void Heap_Free(void *p);

void HandlePair_Free(void **handle)
{
    if (handle == NULL) return;
    Object_FreeAndUnhook(handle[0]);
    Heap_Free(handle);
}
