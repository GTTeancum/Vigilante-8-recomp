/* load_misc.c -- small misc helpers from LOAD.DLL.
 *
 * Source: LOAD.DLL
 *   FUN_80106234 -- LoadDLL_ResetCallback: if `mode == 0` calls
 *     ResetCallback(); else no-op. The MDEC video player's tear-down
 *     hook.
 *   FUN_801062f4 -- LoadDLL_CopyMDECParams: copies two 16-u32 blocks
 *     into DAT_80106e00 and DAT_80106e40 -- the MDEC quantization
 *     tables (luma + chroma) for the next video.
 *   FUN_80106398 -- LoadDLL_BitFlag_GP0_Hack: weird bit manipulation
 *     on caller's *param_1, gated on flag bits in param_2. Preserved
 *     pattern from original; the `param_1 = (uint*)1` clobber is a
 *     Ghidra reading artifact of a delayed-branch param load.
 *
 * HIGH-MED.
 */
#include <stdint.h>

extern void ResetCallback(void);
extern uint32_t DAT_80106e00[16];
extern uint32_t DAT_80106e40[16];

void LoadDLL_ResetCallback(int mode)
{
    if (mode == 0) ResetCallback();
}

void LoadDLL_CopyMDECParams(const uint32_t *quantTables)
{
    for (int i = 0; i < 16; i++) DAT_80106e00[i] = quantTables[i];
    for (int i = 0; i < 16; i++) DAT_80106e40[i] = quantTables[16 + i];
}

void LoadDLL_BitFlag_GP0_Hack(uint32_t *target, uint32_t flags)
{
    if ((flags & 1u) != 0) target = (uint32_t *)1;
    *target |= 0x8000000u;
    if ((flags & 2u) != 0) target = (uint32_t *)1;
    *target &= ~0x2000000u;
}
