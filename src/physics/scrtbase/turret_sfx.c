/* turret_sfx.c -- SCRTBASE turret/MSilo continuous SFX manager.
 *
 * Source: SCRTBASE.DLL  FUN_801003f4.
 *
 * Each tick, computes the 3D sound pan for `obj+0x48` (the turret's
 * world pos). If the resulting volume is non-zero (the listener can
 * hear it), updates the SPU voice volume (if already playing) and
 * calls Audio_PlaySfxAtPos(stream=2) with the current pan. Otherwise,
 * if the voice was playing, stops it via Audio_VoiceStop.
 *
 * The mid-tick `param_1+5` byte tracks the currently-assigned SPU
 * voice channel (0 = "not playing").
 *
 * MED confidence.
 */
#include <stdint.h>

extern uint32_t SfxPan_For3DPos(void *posXyz);                   /* FUN_800449bc */
extern void SPU_VoiceVolume_Set(int ch, uint32_t lr);            /* FUN_80044574 */
extern uint32_t SfxChannel_Acquire(void);                        /* FUN_8004410c */
extern void Audio_PlaySfx_inner(int ch, uint32_t bank, int sfxId, uint32_t pan);  /* FUN_800443c8 */
extern void Audio_VoiceStop(int ch);                             /* FUN_800441c8 */

void SB_TurretSfxTick(int obj, int sfxId)
{
    uint32_t pan = SfxPan_For3DPos((void *)(intptr_t)(obj + 0x48));
    if (pan != 0) {
        int8_t ch = *(int8_t *)(obj + 5);
        if (ch != 0) SPU_VoiceVolume_Set(ch, pan);
        ch = (int8_t)SfxChannel_Acquire();
        *(int8_t *)(obj + 5) = ch;
        Audio_PlaySfx_inner(ch, *(uint32_t *)(*(int *)(obj + 0x58) + 8), sfxId, pan);
    }
    if (*(int8_t *)(obj + 5) != 0) {
        Audio_VoiceStop(*(int8_t *)(obj + 5));
        *(int8_t *)(obj + 5) = 0;
    }
}
