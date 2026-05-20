/* xobf_helpers.c -- small XOBF/Object helper functions called by LOAD.DLL.
 *
 * Source: LOAD.DLL
 *   FUN_80100408  -- XOBF_AppendExtra: appends an XOBF object into the
 *                    global object table at the first free slot past
 *                    index 0x12 (so [0..0x11] = 18 slots reserved for
 *                    18 entries -- larger than the 14 vehicles; pass 2
 *                    will figure out what 14..17 are reserved for).
 *   FUN_801005e8  -- XOBF_LoadBlob: alloc+memcpy a generic blob and
 *                    expose it via _DAT_800659f0 / _DAT_800659e8 (ptr
 *                    + size). Used by font and palette callbacks.
 *   FUN_80100594  -- XOBF_LoadFNT: hands the chunk body to the font
 *                    decoder (FUN_800187e4) and sets a default glyph
 *                    paint at DAT_800659d2 = 0x80.
 *   FUN_801005c0  -- XOBF_StashReturn: latches whatever's in v0 into
 *                    _DAT_80065a00. Used as a callback that captures
 *                    the previous handler's return value.
 *   FUN_8010243c  -- XOBF_AllocArray: alloc a (count * 0x10 + 0x30)-byte
 *                    record and store the count at off +0x1c. Generic
 *                    array-of-struct allocator.
 *   FUN_80100e20  -- XOBF_ReadCollisionBox: reads a 0x14-byte block at
 *                    `dst+0x80`: three i32 (cx, cy, cz) + three i16
 *                    (half-extents). Per-object collision AABB.
 *
 * HIGH-MED confidence on each.
 */
#include <stdint.h>

extern void *Heap_AllocOrRetry(uint32_t n);
extern void *V8_MemCopy(void *dst, const void *src, int n);
extern void *XOBF_Parse(uint32_t remaining, int chainRegister);
extern void  Font_DecodeFNT(void *src, void *dst);     /* FUN_800187e4 */
extern int32_t XobfStream_ReadI32(void *st);            /* func_0x800224ec */
extern int16_t XobfStream_ReadI16(void *st);            /* func_0x800224b4 */

extern void *DAT_800737a0[]; /* vehicle + extra-object array */
extern void *_DAT_800659f0;
extern uint32_t _DAT_800659e8;
extern uint8_t   DAT_800659d2;
extern void    *_DAT_80065a00;
extern uint8_t   DAT_80065a28[];   /* font atlas storage */

/* HIGH: append an XOBF-parsed object after the reserved [0..0x12) slots. */
void XOBF_AppendExtra(uint32_t remaining)
{
    int idx = 0x12;
    if (DAT_800737a0[idx] != NULL) {
        /* Walk to first empty slot. */
        while (DAT_800737a0[++idx] != NULL) { /* nop */ }
    }
    DAT_800737a0[idx] = XOBF_Parse(remaining, 1);
}

/* HIGH: alloc + memcpy a raw blob (palette, lookup, etc.). */
void XOBF_LoadBlob(const void *src, uint32_t size)
{
    _DAT_800659f0 = Heap_AllocOrRetry(size);
    V8_MemCopy(_DAT_800659f0, src, (int)size);
    _DAT_800659e8 = size;
}

/* HIGH: pass an FNT body to the font decoder. */
void XOBF_LoadFNT(void *src)
{
    Font_DecodeFNT(src, DAT_80065a28);
    DAT_800659d2 = 0x80;
}

/* HIGH: latch the previously-returned value (from $v0) into a global. */
void XOBF_StashReturn(uint32_t v0Latched)
{
    _DAT_80065a00 = (void *)(uintptr_t)v0Latched;
}

/* HIGH: array-of-struct allocator -- (count * 0x10 + 0x30) bytes. */
void *XOBF_AllocArray(int count)
{
    uint8_t *r = (uint8_t *)Heap_AllocOrRetry(count * 0x10 + 0x30);
    *(int *)(r + 0x1c) = count;
    return r;
}

/* HIGH: read a per-object collision AABB out of an XOBF stream into
 * the object at `+0x80..+0x91`:
 *   +0x80 i32 centerX
 *   +0x84 i32 centerY
 *   +0x88 i32 centerZ
 *   +0x8c i16 halfX
 *   +0x8e i16 halfY
 *   +0x90 i16 halfZ
 */
void XOBF_ReadCollisionBox(uint8_t *dst, void *stream)
{
    *(int32_t *)(dst + 0x80) = XobfStream_ReadI32(stream);
    *(int32_t *)(dst + 0x84) = XobfStream_ReadI32(stream);
    *(int32_t *)(dst + 0x88) = XobfStream_ReadI32(stream);
    *(int16_t *)(dst + 0x8c) = XobfStream_ReadI16(stream);
    *(int16_t *)(dst + 0x8e) = XobfStream_ReadI16(stream);
    *(int16_t *)(dst + 0x90) = XobfStream_ReadI16(stream);
}
