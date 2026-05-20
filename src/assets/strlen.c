/* strlen.c -- V8 strlen (null-safe).
 *
 * Source: SLUS_005.10  FUN_80052544.
 *
 * Standard libc/PSY-Q strlen with one twist: returns 0 if passed a
 * NULL pointer (regular libc strlen would crash). HIGH confidence.
 */
#include <stdint.h>

int Util_StrLen(const char *s)
{
    if (s == NULL) return 0;
    int n = 0;
    while (*s != '\0') {
        s++;
        n++;
    }
    return n;
}
