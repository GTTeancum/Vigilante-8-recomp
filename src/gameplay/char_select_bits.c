/* char_select_bits.c -- bit manipulation in DAT_80065950 (char-select grid).
 *
 * Source: SHELL.DLL  FUN_8010c690.
 *
 * Bit-twiddles the per-slot byte at DAT_80065950 + g_playerSlotIdx[0]
 * based on the current cursor index DAT_80065904. The pattern:
 *
 *   if DAT_80065904 == 0x1f - $v0   // 31 - returned-from-prev-call
 *     set bit (1<<idx) but also turn on (2<<idx)
 *   bit = 1<<idx
 *   *p |= bit
 *   DAT_80065920 = 0x1f - bit   // "highlight mask" complement
 *   *p = bVar1                  // restore original -- so the OR is
 *                                  effectively reverted! Net effect
 *                                  is only the side-effect on
 *                                  DAT_80065920.
 *
 * This is highly unusual -- the function appears to compute an LED-
 * style highlight mask while leaving the persistent grid byte
 * unchanged. Possibly a debug-only or sound-cue-only artifact.
 *
 * LOW confidence on semantics; the bit math is exact.
 */
#include <stdint.h>

extern uint8_t DAT_80065950[];
extern uint8_t DAT_80065674[];   /* g_playerSlotIdx */
extern int32_t DAT_80065904;     /* cursor index */
extern int8_t  DAT_80065920;     /* highlight mask complement */

void CharSelect_BitMath(int32_t v0Latched)
{
    uint8_t *slot = &DAT_80065950[DAT_80065674[0]];
    uint8_t orig  = *slot;
    if (DAT_80065904 == 0x1f - v0Latched) {
        *slot = (uint8_t)((orig & ~(uint8_t)(1u << (DAT_80065904 & 0x1fu)))
                          | (uint8_t)(2u << (DAT_80065904 & 0x1fu)));
    }
    uint8_t bit = (uint8_t)(1u << (DAT_80065904 & 0x1fu));
    *slot |= bit;
    DAT_80065920 = (int8_t)(0x1f - bit);
    *slot = orig;
}
