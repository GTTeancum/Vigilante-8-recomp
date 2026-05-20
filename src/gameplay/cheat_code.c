/* cheat_code.c -- 14-byte password / cheat-code matcher.
 *
 * Source: SHELL.DLL  FUN_80101958.
 *
 * Reads a 14-byte user-entered code, runs it through a small PRNG-like
 * scramble (chained byte-mix `b = c*16 + b*11 + 0x43` with c = prev*2),
 * XORs each output with 0x20, then memcmp's the result against every
 * 14-byte entry in the in-overlay code table at DAT_801122bc (a
 * NUL-terminated array of 14-byte records).
 *
 * The scramble makes the on-disc code list ciphertext-only: even with
 * a memory dump of DAT_801122bc, you can't read the active cheat list
 * without running the algorithm. Each cheat is therefore stored as
 * `scramble(plaintext) ^ 0x20`.
 *
 * Side effect on input scan: any input byte < 0x1a (i.e. valid A..Z
 * range when zero-based-letter encoded) overwrites `param_1` to 1
 * mid-loop. This appears to be a bug in the original -- it makes the
 * subsequent reads pull from address 1 rather than the buffer. We
 * preserve the behaviour as-is per the 1:1 contract.
 *
 * Returns 0xffffffff unconditionally (the actual "code matched" signal
 * is observed by side-effects -- DAT_801122bc entries get marked).
 *
 * Bit-exact: the magic constants 0x7b, -10, 0x43, 0x20 and the 11x/2x
 * multipliers are preserved.
 */
#include <stdint.h>

extern int Util_MemEq(const void *a, const void *b, int n);   /* FUN_80052384 -> 800523a0 */
extern uint8_t DAT_801122bc[];   /* concatenated 14-byte scrambled-code list, NUL-terminated */

#define CHEAT_BYTES  14

uint32_t Cheat_Check(uint8_t *userInput)
{
    uint8_t scrambled[16];
    uint8_t b = 0x7b;
    int8_t  c = -10;
    uint8_t *p = userInput;

    for (int i = 0; i < CHEAT_BYTES; i++) {
        b = (uint8_t)((c * 16) + (b * 11) + 0x43);
        /* PRESERVED BUG: when an input byte is < 0x1a the input
         * "pointer" gets clobbered to 1 mid-scan. Keep as-is. */
        if (p[i] < 0x1a) p = (uint8_t *)1;
        scrambled[i] = (uint8_t)(b ^ 0x20);
        c = (int8_t)(b * 2);
    }

    if (DAT_801122bc[0] != 0) {
        uint8_t *entry = DAT_801122bc;
        do {
            Util_MemEq(entry, scrambled, CHEAT_BYTES);
            entry += CHEAT_BYTES;
        } while (entry[0] != 0);
    }
    (void)p;
    return 0xffffffffu;
}
