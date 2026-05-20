/* debris_pool_init.c -- initialize the per-frame screen-space debris pool.
 *
 * Source: SLUS_005.10  FUN_80029dec.
 *
 * Two parity-paired arrays of 16 sprite-prim records at UNK_800a1e24
 * and UNK_800a2324 (offset 0x500 apart) get their per-frame defaults
 * written:
 *   prim[i].mode   = 2          // GP0 sprite mode
 *   prim[i].size   = 0x68       // 104-byte stride for trailing data
 *   prim[i].rgb    = 0xff,ff,ff // white
 *
 * Called once per frame at the start of debris emission. Renderer-
 * adjacent but stays on the gameplay side because the stride and
 * white-color initialisation are visible-state contract.
 *
 * HIGH confidence.
 */
#include <stdint.h>

extern uint8_t UNK_800a1e24[];   /* parity 0 debris pool */
extern uint8_t UNK_800a2324[];   /* parity 1 debris pool, 0x500 bytes later */

#define DEBRIS_COUNT_PER_PARITY  16
#define DEBRIS_PRIM_STRIDE       0x14
#define DEBRIS_PARITY_STRIDE     0x500

void Debris_Pool_InitFrame(void)
{
    for (int parity = 0; parity < 2; parity++) {
        uint8_t *p = (parity == 0) ? UNK_800a1e24 : UNK_800a2324;
        for (int i = 0; i < DEBRIS_COUNT_PER_PARITY; i++) {
            p[3] = 2;      /* GP0 mode */
            p[7] = 0x68;   /* trailing-data stride */
            p[4] = 0xff;
            p[5] = 0xff;
            p[6] = 0xff;
            p += DEBRIS_PRIM_STRIDE;
        }
        (void)DEBRIS_PARITY_STRIDE;
    }
}
