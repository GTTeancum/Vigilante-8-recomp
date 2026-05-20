/* object_teardown.c -- safe Heap_Free for objects that own parity-paired
 * front/back-buffer payloads.
 *
 * Source: SLUS_005.10  FUN_8001bddc.
 *
 * V8's draw-prim handle struct holds two parity-paired buffer pointers
 * at offsets +0x1c (front) and +0x20 (back). When freeing such a
 * handle, only the parity not currently being read by the renderer
 * (= 1 - g_dispBufIndex) is safe to free immediately; the other goes
 * through Buffer_DeferFree so the renderer finishes its current pass
 * first.
 *
 * After both parity slots are handled, the handle itself is freed.
 *
 * HIGH confidence: matches the canonical "free a parity-paired buffer
 * handle" idiom seen across the binary.
 */
#include <stdint.h>

extern void Heap_Free(void *p);
extern void Buffer_DeferFree(void *p);   /* FUN_800118b4 */
extern int32_t iRam00000004;             /* g_dispBufIndex */

void Object_FreeAndUnhook(void *handle)
{
    if (handle == NULL) return;
    uint8_t *h = (uint8_t *)handle;
    if (*(void **)(h + iRam00000004 * 4 + 0x1c) != NULL) {
        Buffer_DeferFree(*(void **)(h + iRam00000004 * 4 + 0x1c));
    }
    if (*(void **)(h + (1 - iRam00000004) * 4 + 0x1c) != NULL) {
        Heap_Free(*(void **)(h + (1 - iRam00000004) * 4 + 0x1c));
    }
    Heap_Free(handle);
}
