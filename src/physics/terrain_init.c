/* terrain_init.c -- initialize the terrain chunk pool with a flat world.
 *
 * Source: SLUS_005.10  FUN_800251fc.
 *
 * Called at level-load time before terrain chunks are streamed in. It:
 *
 *   1. Frees all 64 (0x40) currently-held chunk pointers in
 *      DAT_8007a8a0[64].
 *   2. Allocates a single 0x3000-byte "empty chunk" containing:
 *        - 0x2000 bytes of u16 height samples initialised to 0x45ff
 *          (≈1535 -- the engine's flat-world height)
 *        - 0x1000 bytes of u8 material indices initialised to 0
 *   3. Stores that pointer in slot [0] and zeroes slots [1..63].
 *   4. Points every entry of the 32x33 chunk-index table DAT_800911a0
 *      at the empty chunk, so any Terrain_HeightAt query returns
 *      0x45ff.
 *   5. Reinitialises two object template arrays:
 *        UNK_80092220 -- 0x900 entries of stride 0x1c (templates with
 *                        byte[3]=6, byte[7]=0x30).
 *        UNK_8007a9a0 -- 0x800 entries of stride 0x28 (templates with
 *                        byte[3]=9, byte[7]=0x34, byte[0x13]=0x34,
 *                        byte[0x1f]=0x34).
 *   6. Latches the supplied half-word parameter into uRam0000082c
 *      (presumably the level id).
 *
 * MED confidence: the geometry constants are taken verbatim; the meaning
 * of UNK_80092220 / UNK_8007a9a0 entries (likely instance templates for
 * world objects + collision primitives) needs pass-2 cross-reference.
 */
#include <stdint.h>

extern void *Heap_Free(void *p);
extern void *Heap_AllocOrRetry(uint32_t n);

extern uintptr_t DAT_8007a8a0[64];
extern uintptr_t DAT_800911a0[];        /* 32 * 33 entries (out-of-bounds row is alias) */
extern uint8_t   UNK_80092220[];
extern uint8_t   UNK_8007a9a0[];
extern uint16_t  uRam0000082c;
extern uint32_t  uRam00000814;

void Terrain_InitFlatWorld(uint16_t levelId)
{
    /* Step 1: free any existing chunk pointers. */
    uRam00000814  = 0;
    uRam0000082c  = levelId;
    for (int i = 0; i < 64; i++) {
        if (DAT_8007a8a0[i] != 0) Heap_Free((void *)DAT_8007a8a0[i]);
    }

    /* Step 2-3: allocate one shared "empty" chunk. */
    uint8_t *empty = (uint8_t *)Heap_AllocOrRetry(0x3000);
    DAT_8007a8a0[0] = (uintptr_t)empty;
    for (int i = 1; i < 64; i++) DAT_8007a8a0[i] = 0;

    /* Heights: 0x45ff for the entire 0x2000-byte (64x64 u16) region. */
    for (int row = 0; row < 64; row++) {
        for (int col = 0; col < 64; col++) {
            *(uint16_t *)(empty + row * 0x80 + col * 2) = 0x45ff;
            empty[0x2000 + row * 0x40 + col] = 0;
        }
    }

    /* Step 4: point every chunk-index entry at the empty chunk. */
    for (int row = -1; row < 33; row++) {
        for (int col = 0; col < 32; col++) {
            DAT_800911a0[row * 32 + col] = (uintptr_t)empty;
        }
    }

    /* Step 5a: 0x900 templates of stride 0x1c. */
    for (uint32_t i = 0; i < 0x900; i++) {
        uint8_t *t = &UNK_80092220[i * 0x1c];
        t[3] = 6;
        t[7] = 0x30;
    }

    /* Step 5b: 0x800 templates of stride 0x28. */
    for (uint32_t i = 0; i < 0x800; i++) {
        uint8_t *t = &UNK_8007a9a0[i * 0x28];
        t[3]    = 9;
        t[7]    = 0x34;
        t[0x13] = 0x34;
        t[0x1f] = 0x34;
    }
    /* Pass 2: function continues with two more init loops -- left as
     * UNKNOWN until the affected templates are reverse-mapped. */
}
