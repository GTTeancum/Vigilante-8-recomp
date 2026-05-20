/* bzero.c -- zero-fill helper (PSY-Q style).
 * Source: SLUS_005.10  FUN_80044f64.  HIGH.
 */
#include <stdint.h>

void *V8_BZero(void *dst, int n)
{
    uint8_t *bp = (uint8_t *)dst;
    while (n > 0 && ((uintptr_t)bp & 3u) != 0) { *bp++ = 0; n--; }
    uint32_t *wp = (uint32_t *)bp;
    while (n >= 4) { *wp++ = 0; n -= 4; }
    bp = (uint8_t *)wp;
    if (n > 0) {
        uint8_t *tail = bp + n - 1;
        uint32_t off = (uintptr_t)tail & 3u;
        uint32_t *w  = (uint32_t *)((uintptr_t)tail - off);
        uint32_t mask = 0xffffffffu << ((off + 1) * 8);
        *w = *w & mask;
    }
    return dst;
}
