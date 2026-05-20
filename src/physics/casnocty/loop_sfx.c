/* loop_sfx.c -- Casino City looping-SFX manager (sfx 3 = traffic loop).
 *
 * Source: CASNOCTY.DLL  FUN_80101fe0.
 *
 * Identical body to SF_ElevatorTick (SANDFACT) with sfxId hardcoded
 * to 3 instead of 0. Used for traffic/ambient loops in Casino City.
 *
 * See src/physics/sandfact/elevator.c for full details; this is a
 * pass-3 hoist candidate.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t SfxPan_For3DPos(void *posXyz);
extern void SPU_VoiceVolume_Set(int ch, uint32_t lr);
extern uint32_t SfxChannel_Acquire(void);
extern void Audio_PlaySfx_inner(int ch, uint32_t bank, int sfxId, uint32_t pan);
extern void Audio_VoiceStop(int ch);
extern void Stream_FatalOom(const char *msg);
extern uint32_t Object_ClearBackBufferFlag(uint32_t *obj);
extern void Object_SetSubState(int obj, int sub);
extern void Object_RegisterPostUpdate(uint32_t *obj);
extern char s_PROB__8010011c[];

uint32_t CC_LoopSfxTick(int obj, int mode)
{
    if (mode == 1) return 0;

    if (mode == 0 || mode == 2) {
        uint32_t pan = SfxPan_For3DPos((void *)(intptr_t)(obj + 0x48));
        if (pan != 0) SPU_VoiceVolume_Set((int)*(int8_t *)(obj + 5), pan);
        Object_ClearBackBufferFlag((uint32_t *)(intptr_t)obj);
        Object_SetSubState(obj, 0x78);
        Audio_VoiceStop((int)*(int8_t *)(obj + 5));
        *(int8_t *)(obj + 5) = 0;
    }
    uint32_t pan2 = SfxPan_For3DPos((void *)(intptr_t)(obj + 0x48));
    if (pan2 != 0) {
        if (*(int8_t *)(obj + 5) != 0) Stream_FatalOom(s_PROB__8010011c);
        int ch = (int)(int8_t)SfxChannel_Acquire();
        *(int8_t *)(obj + 5) = (int8_t)ch;
        Audio_PlaySfx_inner(ch, *(uint32_t *)(*(int *)(obj + 0x58) + 8), 3, pan2);
        Object_RegisterPostUpdate((uint32_t *)(intptr_t)obj);
    }
    Object_SetSubState(obj, 0x78);
    return 0;
}
