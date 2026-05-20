/* water.c -- Hoover Dam water-spill collision and bobbing.
 *
 * Source: HOOVRDAM.DLL  FUN_801011b0.
 *
 * Tick callback for the dam's water-spill effect (PipeOut_1 + the
 * dam itself). On each idle tick:
 *   - mode 0 + impulse: bob up-and-down -- obj+0x28 = baseY + sin
 *     amplitude scaled by an offset into a global sin table at
 *     0x800607b4 (size 0x80 entries, 4-byte stride? 7-bit indexing).
 *     The phase uses (global frame counter + obj+9 jitter byte).
 *   - mode 3: collision was a weapon hit; on flag 0x10000 already set
 *     skip; else mark obj+0x24 as flooded via FUN_80040234, sfx 0x41.
 *   - mode 4: skip the dim down step.
 * Then always: sub-state 7, status |= 0x80, drop posY by 0xa000,
 * play sfx 2.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t SfxChannel_Acquire(void);
extern uint32_t SfxPan_For3DPos(void *posXyz);
extern void SPU_VoiceVolume_Set(int ch, uint32_t lr);
extern void Audio_PlaySfxAtPos(uint32_t ch, uint32_t bank, int sfxId, int posXyzAddr);
extern void Audio_PlaySfx_inner(int ch, uint32_t bank, int sfxId, uint32_t pan);
extern void Water_FloodAtPos(int posXyzAddr);   /* FUN_80040234 */
extern uint32_t _DAT_80065310;                   /* global frame counter */
extern uint32_t _DAT_800658fc;
extern int16_t DAT_800607b4[];                   /* sin lookup */

uint32_t HD_WaterBob(uint32_t *self, int mode, int *impulse)
{
    if (mode == 1) goto tail;
    if (mode == 0 || mode == 2) {
        if (impulse == NULL) return 0;
        uint32_t phase = (_DAT_80065310 + (uint32_t)*((uint8_t *)self + 9)) * 0x80 & 0x3f80u;
        int16_t bob = DAT_800607b4[phase / 2];
        self[10] = self[0x13] + (uint32_t)bob * 10;
        if ((self[0] & 0x20000u) == 0) self[0] &= ~0x10000u;
        self[0] &= ~0x20000u;
        uint32_t pan = SfxPan_For3DPos(self + 9);
        SPU_VoiceVolume_Set((int)*(int8_t *)((uintptr_t)self + 5), pan);
    } else if (mode != 3) {
        if (mode == 4) goto tail;
        goto tail;
    } else {
        /* mode 3 */
        if ((self[0] & 0x10000u) == 0) {
            Water_FloodAtPos(*impulse + 0x24);
            uint32_t ch = SfxChannel_Acquire();
            Audio_PlaySfxAtPos(ch, _DAT_800658fc, 0x41, *impulse + 0x24);
        }
        self[0] |= 0x30000u;
    }

tail:
    *((int8_t *)self + 4) = 7;
    *((uint16_t *)self + 6) = 10;
    self[0] |= 0x80u;
    self[0x13] -= 0xa000;
    {
        int ch = (int)(int8_t)SfxChannel_Acquire();
        *((int8_t *)self + 5) = (int8_t)ch;
        Audio_PlaySfx_inner(ch, *(uint32_t *)(self[0x16] + 8), 2, 0);
    }
    return 0;
}
