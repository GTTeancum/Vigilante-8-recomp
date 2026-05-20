/* sound_position.c -- compute pan/volume from a 3D sound source position.
 *
 * Source: SLUS_005.10
 *   FUN_800449bc  -- SfxPan_For3DPos(srcPos) -> u32 LR-volume pair
 *                    (used for instantaneous SFX placement).
 *   FUN_800446dc  -- SfxPan_For3DPosDelayed(srcPos)
 *                    (used for delayed/queued SFX where the panning
 *                    follows the projectile rather than the impact.)
 *
 * Each function:
 *   1. Transforms the source pos through the world->camera matrix at
 *      DAT_8006f680 via GTE_RotateLongMatTrans -- gets a camera-space
 *      vector.
 *   2. Branches on g_splitScreenMode (iRam00000010):
 *        0 (1P)  -> SfxPan_From1P_Camera{Stereo,Mono} based on
 *                   cRam000005a8 (output-mode flag: 0=stereo, !=0=mono).
 *        else    -> 2P-specific variant (not detailed here; the same
 *                   structure but uses split-screen camera basis).
 *
 * Returns a u32 packing (lvol << 16 | rvol) suitable for direct
 * use with SPU voice volume registers.
 *
 * Bit-exact: the camera-space transform is delegated to GTE so the
 * saturation matches the chip. Pan curve constants live in the inner
 * helpers (FUN_800448ec / FUN_80044890 / FUN_80044600 / FUN_800445b8)
 * still in auto/ -- pass 3 unifies them.
 *
 * MED-HIGH confidence.
 */
#include <stdint.h>
#include "structs.h"

extern void GTE_RotateLongMatTrans(const MATRIX *m, void *posXyz, void *outXyz);  /* FUN_80043408 */
extern uint32_t FUN_800448ec(const void *xyz);
extern int      FUN_80044890(const void *xyz);
extern uint32_t FUN_80044600(const void *xyz);
extern int      FUN_800445b8(const void *xyz);

extern MATRIX  DAT_8006f680;
extern int32_t iRam00000010;     /* g_splitScreenMode */
extern int8_t  cRam000005a8;     /* output-mode flag */

static inline uint32_t mono_to_pair(int v)
{
    /* The arithmetic ((v << 16) >> 16) + v << 16 forms a pair where both
     * halves equal v -- the canonical mono-output volume pair. */
    return (uint32_t)((v * 0x10000 >> 0x10) + v * 0x10000);
}

uint32_t SfxPan_For3DPos(void *srcPos)
{
    uint8_t camSpace[16];
    GTE_RotateLongMatTrans(&DAT_8006f680, srcPos, camSpace);
    if (iRam00000010 == 0) {
        if (cRam000005a8 == 0) return FUN_800448ec(camSpace);
        return mono_to_pair(FUN_80044890(camSpace));
    }
    /* 2P split-screen path -- pass 3 finishes. */
    return 0;
}

uint32_t SfxPan_For3DPosDelayed(void *srcPos)
{
    uint8_t camSpace[16];
    GTE_RotateLongMatTrans(&DAT_8006f680, srcPos, camSpace);
    if (iRam00000010 == 0) {
        if (cRam000005a8 == 0) return FUN_80044600(camSpace);
        return mono_to_pair(FUN_800445b8(camSpace));
    }
    return 0;
}
