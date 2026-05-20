/* bit_interleave.c -- 16-bit interleave / spread (Morton-like).
 *
 * Source: SLUS_005.10  FUN_80012088.
 *
 * Reads a 16-bit mask in `bits`, walks it LSB-first; for each set bit
 * it OR's the 17-bit pair (0x10001) into the result, shifted by the
 * current 4-bit lane index taken from the low nibble of a 64-bit
 * rotating accumulator (hi:lo = param_4:param_3).
 *
 * This is the loose-bit "spread" function -- given a 16-bit pattern and
 * a 64-bit per-lane shift table, it produces a 32-bit value packing
 * each set source bit twice (low + high half) at the per-lane offset.
 *
 * It's used by the renderer / HUD to expand a bitmask into a per-pair
 * GP0 packet field. Out-of-scope for 1:1 physics (renderer detail),
 * but cheap and trivial to keep in.
 *
 * HIGH confidence on the math; MED on intended use until a caller is
 * cleaned up.
 */
#include <stdint.h>

uint32_t Util_BitSpread(uint32_t bits, uint32_t lo, uint32_t hi)
{
    uint32_t result = 0;
    while ((bits & 0xffffu) != 0) {
        result |= (bits & 0x10001u) << (lo & 0xfu);
        lo      = (lo >> 4) | (hi << 28);
        hi      =  hi >> 4;
        bits  >>= 1;
    }
    return result;
}
