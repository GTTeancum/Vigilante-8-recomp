/* memcmp.c -- V8 bytewise memcmp.
 *
 * Source: SLUS_005.10  FUN_800523a0 (thunked at 0x80052384).
 *
 * Standard libc/PSY-Q memcmp. Compares byte-by-byte; returns the
 * signed difference of the first mismatched byte pair, or 0 when
 * the buffers match for `n` bytes.
 *
 * Called by Iso_OpenPath as the 12-byte / 8-byte name comparator,
 * and by Iso_VerifyOriginalDisc as the 32-byte PVD signature
 * comparator.
 *
 * HIGH confidence.
 */
#include <stdint.h>

int Util_MemEq(const void *a, const void *b, int n)
{
    const uint8_t *p = (const uint8_t *)a;
    const uint8_t *q = (const uint8_t *)b;
    while (n > 0) {
        if (*p != *q) return (int)(uint8_t)*p - (int)(uint8_t)*q;
        p++; q++;
        n--;
    }
    return 0;
}
