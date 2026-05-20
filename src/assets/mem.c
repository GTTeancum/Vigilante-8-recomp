/* mem.c -- Vigilante 8 memcpy / memset / memmove implementations.
 *
 * Source: SLUS_005.10:
 *   FUN_80044efc -- memset (byte-fill with u32 unroll)
 *   FUN_80044c44 -- memcpy (forward; unrolled 16-byte body)
 *   FUN_80044d9c -- memmove (handles overlap by choosing direction)
 *
 * HIGH confidence: standard libc / PSY-Q implementations. The unrolled
 * tail handling uses unaligned-load tricks (uses 3..0 byte shifts with
 * a 4-byte aligned access mask). Preserved verbatim because alignment
 * behavior could be observable on PSX hardware via cache or write-combine.
 *
 * The functions are exposed as V8_MemSet / V8_MemCopy / V8_MemMove because
 * PSY-Q does NOT name them memcpy/memset -- they live in the game binary as
 * inline-able locals. Keep the V8_ prefix to avoid colliding with the host
 * toolchain's libc in the eventual port.
 */
#include <stdint.h>
#include <stddef.h>

/* HIGH: byte-fill. Aligns to 4-byte boundary, then writes u32 words with
 * the byte replicated, then writes the misaligned tail with a read-mask-write.
 */
void *V8_MemSet(void *dst, int value, int n)
{
    if (n == 0) return dst;

    uint8_t  *bp   = (uint8_t *)dst;
    uint32_t  fill = (uint32_t)(value & 0xff);

    /* head: byte writes until 4-byte aligned */
    while (((uintptr_t)bp & 3u) != 0) {
        *bp++ = (uint8_t)fill;
        if (--n == 0) return dst;
    }

    fill |= fill << 8;
    fill |= fill << 16;

    uint32_t *wp = (uint32_t *)bp;
    while (n >= 4) {
        *wp++ = fill;
        n   -= 4;
    }

    /* tail: <4 bytes left, written via masked u32 read-modify-write to
     * match the original code's unaligned-store sequence. */
    if (n > 0) {
        uint8_t *tail = (uint8_t *)wp + (n - 1);
        uint32_t  off = (uintptr_t)tail & 3u;
        uint32_t *w   = (uint32_t *)((uintptr_t)tail - off);
        uint32_t  mask = 0xffffffffu << ((off + 1) * 8);   /* preserve bytes above */
        uint32_t  bits = fill         >> ((3 - off) * 8);
        *w = (*w & mask) | bits;
    }
    return dst;
}

/* HIGH: forward copy, 16-byte unrolled body. NOT safe for overlap when
 * src < dst -- use V8_MemMove for that. */
void *V8_MemCopy(void *dst, const void *src, int n)
{
    if (n == 0) return dst;

    const uint8_t *s = (const uint8_t *)src;
    uint8_t       *d = (uint8_t *)dst;

    /* head: align source to 4 by bytewise copy */
    while (((uintptr_t)s & 3u) != 0) {
        *d++ = *s++;
        if (--n == 0) return dst;
    }

    if (((uintptr_t)d & 3u) == 0) {
        const uint32_t *sw = (const uint32_t *)s;
        uint32_t       *dw = (uint32_t *)d;
        while (n >= 16) {
            dw[0] = sw[0]; dw[1] = sw[1]; dw[2] = sw[2]; dw[3] = sw[3];
            sw += 4; dw += 4; n -= 16;
        }
        while (n >= 4) {
            *dw++ = *sw++;
            n   -= 4;
        }
        s = (const uint8_t *)sw;
        d = (uint8_t *)dw;
    }

    while (n > 0) {
        *d++ = *s++;
        n--;
    }
    return dst;
}

/* HIGH: overlap-safe move. Copies backward when src precedes dst. */
void *V8_MemMove(void *dst, const void *src, int n)
{
    if (n == 0) return dst;
    if ((uintptr_t)src >= (uintptr_t)dst) {
        return V8_MemCopy(dst, src, n);
    }
    /* backward copy */
    uint8_t       *d = (uint8_t *)dst + n;
    const uint8_t *s = (const uint8_t *)src + n;
    while (n > 0) {
        *--d = *--s;
        n--;
    }
    return dst;
}
