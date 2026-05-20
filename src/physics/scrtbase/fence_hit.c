/* fence_hit.c -- Secret Base fence_left / fence_right hit + bell-rope.
 *
 * Source: SCRTBASE.DLL  FUN_80101acc.
 *
 * On collision with a missile or vehicle:
 *   - mode 3/8: look up paired object by sub-state at impulse:
 *       if obj.posX@+0x80 is negative -> bell-rope id 0x74
 *       else                          -> 0x75
 *   - Mark paired as "hit" (state 1, sub-state 600, flag 0x20).
 *   - Play sfx 0x14 at impulse pos+0x24.
 *   - Run Damage_FromImpulse + Damage_AccumulateOrFire.
 *   - On kill: invoke func_0x80020844 (FX teardown).
 *   - At end: clear back-buffer bit on child @+0x38, set state byte
 *     in obj+0x38+4.
 *
 * MED.
 */
#include <stdint.h>

extern int  Damage_FromImpulse(uint32_t *self, int *impulse);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t amount);
extern uint32_t Object_FindByPairedSpawnId(uint32_t sid);   /* func_0x80021808 */
extern int  Object_FrameCounterBump(int payload, uint16_t arg);
extern void Object_SetSubState(int obj, int sub);
extern uint32_t SfxChannel_Acquire(void);
extern void Audio_PlaySfxAtPosVar(uint32_t ch, uint32_t bank, int sfxId, void *pos);
extern void FX_Teardown(int obj);   /* func_0x80020844 */
extern uint32_t _DAT_800658fc;

uint32_t SB_FenceHit(int self, uint32_t mode, int *impulse)
{
    uint32_t *paired = NULL;
    if (mode == 2) goto clearBit;
    if (mode < 3 && mode == 1) goto applyState;
    if (mode != 3 && mode != 8) goto applyState;

    paired = (uint32_t *)(uintptr_t)impulse[3];
    int impulser = *impulse;
    if ((uint8_t)((uintptr_t)paired >> 8) == mode
        && *(int8_t *)(impulser + 4) == 2)
    {
        uint32_t sid = (*(int32_t *)(impulser + 0x80) < 0) ? 0x74 : 0x75;
        uint32_t pairedObj = Object_FindByPairedSpawnId(sid);
        Object_FrameCounterBump((int)pairedObj, *(uint16_t *)(*(int *)(pairedObj + 0x60)));
        *(int8_t *)(pairedObj + 8) = 1;
        Object_SetSubState((int)pairedObj, 600);
        Object_SetSubState(self, 600);
        paired[0] |= 0x20u;

        uint32_t ch = SfxChannel_Acquire();
        Audio_PlaySfxAtPosVar(ch, _DAT_800658fc, 0x14, (void *)(intptr_t)(impulser + 0x24));
        self = 1;
    }
    Damage_FromImpulse((uint32_t *)(intptr_t)self, impulse);
    self = 1;

applyState: {
        int killed = Damage_AccumulateOrFire((uint32_t *)(intptr_t)self, (uint16_t)(uintptr_t)impulse);
        if (killed == 0) return 0;
        FX_Teardown(self);
    }
clearBit: {
        uint32_t *child = *(uint32_t **)(self + 0x38);
        child[0] &= ~0x20u;
        if (paired != NULL) {
            *(int8_t *)(*(int *)(self + 0x38) + 4) = (int8_t)(uintptr_t)paired;
        }
    }
    return 0;
}
