/* font_decode_stub.c -- host-safe texture slot initialisation seam.
 *
 * FUN_800187e4 is the original font/TIM decode helper used by object bank
 * allocation when a model references a texture slot for the first time.
 * Renderer/VRAM upload is outside the decomp scope, but gameplay-side object
 * construction expects the small destination record to be initialised.
 */
#include <stdint.h>

void FUN_800187e4(uint32_t src, void *dst)
{
    uint16_t *slot = (uint16_t *)dst;

    (void)src;
    if (slot == 0)
        return;

    slot[0] = 1;
    slot[1] = 0;
    slot[2] = 0;
    slot[3] = 0;
    slot[4] = 0;
    slot[5] = 0;
}
