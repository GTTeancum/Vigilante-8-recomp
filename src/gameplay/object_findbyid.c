/* object_findbyid.c -- linear search by object spawn id (i16 @ +0xa).
 *
 * Source: SLUS_005.10  FUN_8001fd9c (gap-recovered).
 *
 * Walks the per-frame "named object" list anchored at piRam0000079c;
 * returns the list node whose payload's spawn-id (i16 at +0xa) matches
 * the requested id, or NULL.
 *
 * Each list node layout (12 bytes):
 *   +0 ptr next
 *   +4 ptr back-link
 *   +8 i32 payload (the actual object struct)
 *
 * The id-equals check is reading +0xa OF THE NODE, not the payload --
 * presumably the spawn-id is duplicated into the node header for fast
 * scan without dereferencing the payload pointer.
 *
 * Used by Hoover Dam's destructibles (HD_TransformerDestroy etc.) to
 * find the paired collapse-state object via spawnId + 1000.
 *
 * HIGH confidence.
 */
#include <stdint.h>
#include <stddef.h>

extern int32_t **piRam0000079c;

int32_t *Object_FindByIdPlusOffset(int targetId)
{
    int32_t *node = (int32_t *)piRam0000079c[0];
    int32_t **prev = piRam0000079c;
    while (node != NULL) {
        if (*(int16_t *)((uintptr_t)prev + 10) == targetId) return (int32_t *)prev;
        prev = (int32_t **)node;
        node = (int32_t *)*node;
    }
    return NULL;
}
