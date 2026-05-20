/* powerline.c -- Hoover Dam PowerPlant_L/R fall + sparks.
 *
 * Source: HOOVRDAM.DLL  FUN_80101464.
 *
 * Tick-mode state machine:
 *   mode 0 (idle drop): subtract 0x3b9a from posZ each tick (slow fall).
 *     If hit by impulse, update 3D sfx pan.
 *   mode 3 (collision): if impulser is in state 2, set vy = 0xfff70f80
 *     (= -0xf080, sharp downward), call Water_FloodAtPos at impulse
 *     pos+0x24, play sfx 0x41, fall through to mode 2.
 *   mode 1 (init / re-init): acquire a voice + play sfx 2 looped.
 *   mode 2 (destroyed): Damage_Apply on the impulser, then init.
 *   mode 4 (teardown): stop the voice.
 *
 * Used by PowerPlant_L / PowerPlant_R -- the power-plant towers that
 * topple when struck.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t SfxPan_For3DPos(void *posXyz);
extern void SPU_VoiceVolume_Set(int ch, uint32_t lr);
extern uint32_t SfxChannel_Acquire(void);
extern void Audio_PlaySfxAtPos(uint32_t ch, uint32_t bank, int sfxId, int posXyzAddr);
extern void Audio_PlaySfx_inner(int ch, uint32_t bank, int sfxId, uint32_t pan);
extern void Audio_VoiceStop(int ch);
extern void Water_FloodAtPos(int posXyzAddr);
extern void Damage_Apply(void *obj);
extern uint32_t _DAT_800658fc;

uint32_t HD_PowerlineTick(int obj, uint32_t mode, int *impulse)
{
    int chOwner = obj;

    switch (mode) {
    case 0:
        *(int32_t *)(obj + 0x2c) -= 0x3b9a;
        if (impulse == NULL) return 0;
        {
            uint32_t pan = SfxPan_For3DPos((void *)(intptr_t)(obj + 0x24));
            SPU_VoiceVolume_Set((int)*(int8_t *)(obj + 5), pan);
        }
        /* fall through */
    case 3: {
        int hitObj = *impulse;
        if (*(int8_t *)(hitObj + 4) == 2) {
            chOwner = hitObj + 0x24;
            *(uint32_t *)(hitObj + 0x84) = 0xfff70f80u;
            Water_FloodAtPos(chOwner);
            uint32_t ch = SfxChannel_Acquire();
            Audio_PlaySfxAtPos(ch, _DAT_800658fc, 0x41, chOwner);
            goto case2;
        }
        break;
    }
case2:
    case 2:
        Damage_Apply((void *)(intptr_t)chOwner);
        obj = 1;
        /* fall through */
    case 1: {
        int8_t ch = (int8_t)SfxChannel_Acquire();
        *(int8_t *)(chOwner + 5) = ch;
        Audio_PlaySfx_inner(ch, *(uint32_t *)(*(int *)(chOwner + 0x58) + 8), 2, 0);
        /* fall through */
    }
    case 4:
        Audio_VoiceStop((int)*(int8_t *)(chOwner + 5));
        break;
    default:
        break;
    }
    return 0;
}
