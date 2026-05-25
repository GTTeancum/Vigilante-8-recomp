/* divdi3.c -- signed 64-bit divide helper used by gameplay math.
 *
 * Source: SLUS_005.10 FUN_8004779c / __divdi3.
 *
 * MED: mirrors the MIPS libgcc sign handling and returns the low 32 bits of
 * the quotient, which is how the game consumes mine terrain-evasion weights.
 */
#include <stdint.h>

int FUN_8004779c(uint32_t lo, int hi, int numerator, int sign)
{
    uint32_t negate = 0;
    uint64_t dividend = ((uint64_t)(uint32_t)hi << 32) | lo;
    uint64_t divisor = ((uint64_t)(uint32_t)sign << 32) | (uint32_t)numerator;

    if (hi < 0) {
        negate = 0xffffffffu;
        dividend = 0 - dividend;
    }

    if (sign < 0) {
        negate = ~negate;
        divisor = 0 - divisor;
    }

    if (divisor == 0)
        return 0;

    uint32_t quotient = (uint32_t)(dividend / divisor);
    if (negate != 0)
        quotient = 0 - quotient;

    return (int32_t)quotient;
}
