/* ice_chunk.c -- Ski Resort falling-ice chunk physics.
 *
 * Source: SKIRESRT.DLL  FUN_80101a94.
 *
 * Variant of the boulder-roll idiom for icicle chunks that detach
 * from the slope ceiling. Same Terrain_QueryAt+bounce flow with the
 * ski layout, but the spin axis is held constant (icicles spin
 * around their long axis rather than aligned to motion).
 *
 * mode: 0=tick, 3=impact (forwards to Damage_StandardVehicle).
 *
 * MED.
 */
#include <stdint.h>

extern int  Terrain_QueryAt(uint32_t *self, uint32_t *pos, int16_t *nOut, int flag);
extern void Object_RefitAABB(uint32_t *self);
extern void MatrixNormal(uint32_t *m, uint32_t *out);
extern void Damage_StandardVehicle(uint32_t *self, uint32_t *imp);
extern void Object_RetireDeferred(uint32_t *self);
extern uint32_t _DAT_80065310;

uint32_t SK_IceChunk(uint32_t *self, int mode, uint32_t *imp)
{
    if (mode == 3) goto damage;
    if (mode != 0) return 0;

    int16_t  n[3] = { 0, 0, 0 };
    uint32_t pos[3] = { self[9], self[10], self[0xb] };
    int gy = Terrain_QueryAt(self, pos, n, 0);
    if (gy < (int)pos[1] + 0x800) {
        int32_t vdot = (int32_t)self[0x20] * n[0]
                     + (int32_t)self[0x21] * n[1]
                     + (int32_t)self[0x22] * n[2];
        if (vdot < 0) vdot += 0x7ff;
        vdot >>= 11;
        if (vdot < 0) {
            int32_t ax = vdot * n[0]; if (ax < 0) ax += 0xfff;
            int32_t ay = vdot * n[1]; if (ay < 0) ay += 0xfff;
            int32_t az = vdot * n[2]; if (az < 0) az += 0xfff;
            self[0x20] -= ax >> 12;
            self[0x21] = ((self[0x21] - (ay >> 12)) / 2);
            self[0x22] -= az >> 12;
        }
    }
    self[0x21] += 0x5a;
    self[9]  += self[0x20];
    self[10] += self[0x21];
    self[0xb]+= self[0x22];
    Object_RefitAABB(self);
    if (((_DAT_80065310 - (uint32_t)*((uint8_t *)self + 9)) & 0xf) == 0)
        MatrixNormal(self + 4, self + 4);
    if ((int)self[10] > 0x4b00000) Object_RetireDeferred(self);
    return 0;
damage:
    Damage_StandardVehicle(self, imp);
    return 0;
}
