/* b17_hit.c -- AIRGRAVE b17_1 spin + hit-response state machine.
 *
 * Source: AIRGRAVE.DLL  FUN_801014d0.
 *
 * Two main modes:
 *   mode 0 (per-tick): rotate the parent's child object (obj[0xe])
 *     by +0x22 around its yaw axis (faster than the radar's +0x11).
 *     If a hit impulse arrives (param_3 != NULL), spark.
 *   mode 3 (hit-response): if the impulser is a Player vehicle in
 *     "weapon-fire" state and we're visible (flag 0x80), spawn a
 *     spark + play sfx 2, then create a new "B-17 down" event object
 *     via FUN_8001ffd4 with id 0x71, and link it to the impulser
 *     (or its current target) at offset +0xa4.
 *
 * MED confidence.
 */
#include <stdint.h>

extern void ImpactSparks_Spawn(void *obj);     /* FUN_8001d708 */
extern int  Object_FrameCounterBump(int payload, uint16_t arg);  /* FUN_8001f9cc */
extern uint32_t SfxChannel_Acquire(void);                          /* FUN_8004410c */
extern void Audio_PlaySfxAtPosVar(uint32_t ch, uint32_t bank, int sfxId, void *pos);  /* FUN_800447e8 */
extern uint32_t *Object_FastList_Insert_TypedId(int listSentinel, uint8_t kind); /* FUN_8001ffd4 */

uint32_t AG_B17HitTick(uint32_t *parent, int mode, uint32_t *impulse)
{
    switch (mode) {
    case 0: {
        uint8_t *child = (uint8_t *)(uintptr_t)parent[0xe];
        *(int16_t *)(child + 0x42) = (int16_t)(*(int16_t *)(child + 0x42) + 0x22);
        if (impulse == NULL) return 0;
        ImpactSparks_Spawn(child);
        /* fall through to mode 3 */
    }
    case 3: {
        uint32_t *src = (uint32_t *)(uintptr_t)impulse[3];
        if (*(int8_t *)((uintptr_t)src + 4) == 3
            && *(int8_t *)(*impulse + 4) == 2
            && (parent[0] & 0x80u) != 0)
        {
            src[0] |= 0x20u;
            Object_FrameCounterBump((int)(uintptr_t)parent,
                                     (uint16_t)(*(uint16_t *)((uintptr_t)parent + 0x46)
                                               + *(uint16_t *)(uintptr_t)parent[0x18]));
            uint32_t ch = SfxChannel_Acquire();
            Audio_PlaySfxAtPosVar(ch,
                                  *(uint32_t *)(parent[0x16] + 8),
                                  2,
                                  parent + 0x12);
            uint32_t *evt = Object_FastList_Insert_TypedId(0x80065a18, 0x71);
            if (evt != NULL) {
                uint32_t srcVA = *impulse;
                *((uint8_t *)evt + 8) = 2;
                uint32_t aiTarget = *(uint32_t *)(srcVA + 0xe4);
                if (aiTarget == 0) aiTarget = srcVA;
                evt[0x29] = aiTarget;
                evt[0] &= 0xffffffddu;
            }
        }
        return 0;
    }
    }
    return 0;
}
