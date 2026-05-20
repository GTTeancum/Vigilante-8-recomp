/* elevator.c -- Sand Factory M2_elevator continuous SFX manager.
 *
 * Source: SANDFACT.DLL  FUN_80100b34.
 *
 * Each tick:
 *   - If mode == 0 or 2 (running): if a voice was already playing,
 *     update its volume; clear back-buffer pending bit (FUN_80020778)
 *     to keep the sound aligned with the parity buffer; force sub-state
 *     0x78 (= "running"); release the voice; clear the slot.
 *   - Then re-acquire a fresh voice for the next pan computation. If a
 *     stale voice slot was found (should be 0 after the clear), trip
 *     the OOM trap -- this is a sanity check that catches double-bind.
 *   - Re-bind the voice, register for post-update.
 *   - Sub-state always set to 0x78 + obj+0x9 (the elevator's "doors
 *     opening" animation phase).
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
extern void Object_DefaultDispatch(int obj, int mode, uint32_t arg);
extern char s_PROB__801000d0[];

uint32_t SF_ElevatorTick(int obj, int mode)
{
    if (mode == 1) goto applyAnim;

    if (mode == 0 || mode == 2) {
        uint32_t pan = SfxPan_For3DPos((void *)(intptr_t)(obj + 0x48));
        if (pan != 0) SPU_VoiceVolume_Set((int)*(int8_t *)(obj + 5), pan);
        Object_ClearBackBufferFlag((uint32_t *)(intptr_t)obj);
        Object_SetSubState(obj, 0x78);
        Audio_VoiceStop((int)*(int8_t *)(obj + 5));
        *(int8_t *)(obj + 5) = 0;
    }

    /* Re-acquire and play. */
    uint32_t pan2 = SfxPan_For3DPos((void *)(intptr_t)(obj + 0x48));
    if (pan2 != 0) {
        if (*(int8_t *)(obj + 5) != 0) Stream_FatalOom(s_PROB__801000d0);
        int ch = (int)(int8_t)SfxChannel_Acquire();
        *(int8_t *)(obj + 5) = (int8_t)ch;
        Audio_PlaySfx_inner(ch, *(uint32_t *)(*(int *)(obj + 0x58) + 8), 0, pan2);
        Object_RegisterPostUpdate((uint32_t *)(intptr_t)obj);
    }
    Object_SetSubState(obj, 0x78);
applyAnim:
    Object_SetSubState(obj, (*(uint8_t *)(obj + 9)) + 0x78);
    Object_DefaultDispatch(obj, mode, 0);
    return 0;
}
