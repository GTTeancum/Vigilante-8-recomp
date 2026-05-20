/* cheat_decode.c -- decode cheat input bytes into ASCII letters.
 *
 * Source: SHELL.DLL  FUN_801016b4.
 *
 * Walks a 14-byte buffer at virtual address 1 (effectively a hardcoded
 * stack scribble or BIOS scratchpad slot used by the cheat-entry UI),
 * adding 'A' to each byte to convert from 0-based-letter codes to
 * ASCII. The terminator at +0xe ([0xf in the global table] is zeroed.
 *
 * Returns 1 (always); the side-effects are the buffer mutation.
 *
 * The pcVar1 = (char *)0x1 pattern is the same observed-bug pattern as
 * Cheat_Check -- preserved per the 1:1 contract.
 *
 * HIGH on shape; MED on semantics (the input buffer alias is unusual).
 */
#include <stdint.h>

extern uint8_t DAT_00000000;
extern uint8_t DAT_0000000f;

uint32_t Cheat_DecodeToAscii(void)
{
    uint8_t *p = (uint8_t *)1;
    int i = 0;
    do {
        *p = (uint8_t)(*p + 'A');
        p = (uint8_t *)((uintptr_t)0 + (i + 2));  /* original: pcVar1 = (char *)(i + 2) */
        i++;
    } while (i < 0xe);
    DAT_0000000f = 0;
    return 1;
}
