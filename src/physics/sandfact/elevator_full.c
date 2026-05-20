/* elevator_full.c -- Sand Factory M2_elevator full state machine.
 *
 * Source: SANDFACT.DLL  FUN_80100854.
 *
 * Larger sibling to SF_ElevatorTick (src/physics/sandfact/elevator.c)
 * but with full state machine including:
 *   - Up/down direction handling (param_1[2] flag)
 *   - 0x393 unit movement per tick along Z axis
 *   - Limit clamping at obj[0xe]+0x4c (max-Z) and -0x7fff (min-Z)
 *   - 3D SFX pan refresh every frame (via SfxPan_For3DPos at
 *     obj+0x14 inside obj[0xe])
 *   - Hit cue SFX 1 on direction-toggle
 *   - Counter at obj+9 advanced by param_3; on overflow (>0x31)
 *     toggles direction and plays sfx 2.
 *
 * Used by all SandFact elevators including the central material lift.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t SfxChannel_Acquire(void);
extern uint32_t SfxPan_For3DPos(void *posXyz);
extern void Audio_PlaySfx_inner(int ch, uint32_t bank, int sfxId, uint32_t pan);
extern void Audio_PlaySfxAtPosVar(uint32_t ch, uint32_t bank, int sfxId, void *pos);
extern void SPU_VoiceVolume_Set(int ch, uint32_t lr);
extern void Audio_VoiceStop(int ch);
extern void Object_RegisterPostUpdate(uint32_t *obj);
extern void *Matrix_ComposeParentChain(int obj);   /* FUN_8001d624 */

uint32_t SF_ElevatorFullTick(uint32_t *self, int mode, int delta)
{
    if (mode != 0 && mode != 2) return 0;

    uint32_t carBody = self[0xe];
    if (delta != 0) {
        uint8_t *carMatrix = Matrix_ComposeParentChain((int)(uintptr_t)carBody);
        uint32_t pan = SfxPan_For3DPos(carMatrix + 0x14);
        SPU_VoiceVolume_Set((int)*(int8_t *)((uintptr_t)self + 5), pan);
    }

    int8_t  dirByte = *(int8_t *)((uintptr_t)self + 8);
    int32_t *carZ = (int32_t *)(uintptr_t)(carBody + 0x28);
    if (dirByte != 0) {
        *carZ -= 0x393;
        if (*carZ <= *(int32_t *)(uintptr_t)(carBody + 0x4c)) return 0;   /* hit lower limit */
    } else {
        *carZ += 0x393;
        if (*carZ < -0x7fff) return 0;
        uint8_t *carMatrix2 = Matrix_ComposeParentChain((int)(uintptr_t)carBody);
        uint32_t ch = SfxChannel_Acquire();
        Audio_PlaySfxAtPosVar(ch, *(uint32_t *)(self[0x16] + 8), 1, carMatrix2 + 0x14);
        *(int8_t *)((uintptr_t)self + 5) = 0;
        Audio_VoiceStop((int)*(int8_t *)((uintptr_t)self + 5));
        if (((int8_t)self[2] == 0) && self[0xe] != 0 && (self[0] & 0x80u) == 0) {
            uint8_t newCounter = (uint8_t)(*(uint8_t *)((uintptr_t)self + 9) + delta);
            *(uint8_t *)((uintptr_t)self + 9) = newCounter;
            if (newCounter > 0x31) {
                *(uint8_t *)((uintptr_t)self + 9) = 0;
                Object_RegisterPostUpdate(self);
                int8_t ch2 = (int8_t)SfxChannel_Acquire();
                *(int8_t *)((uintptr_t)self + 5) = ch2;
                Audio_PlaySfx_inner(ch2, *(uint32_t *)(self[0x16] + 8), 2, 0);
                Audio_VoiceStop((int)ch2);
            }
        }
    }
    return 0;
}
