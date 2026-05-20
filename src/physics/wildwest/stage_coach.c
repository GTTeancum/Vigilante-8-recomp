/* stage_coach.c -- Wild West runaway stagecoach attract loop.
 *
 * Source: WILDWEST.DLL  FUN_8010129c.
 *
 * Path-following stagecoach object: walks the path-list at
 * 0x80065a50, advancing 0x5f5 (~1.46 4.12-units) per frame along the
 * sin/cos at -0x7ff9f84c/4a indexed by the path-piece's current yaw.
 * Y is locked to terrain height. Wheel sub-models (+0xe) spin +0x44
 * yaw per frame.
 *
 * Every 16th frame, emits dust puff (object id 0x2ba) with random
 * XZ jitter +/-0x2800 around current pos.
 *
 * mode dispatch:
 *   0  -- path-walk tick (above)
 *   1  -- post-spawn init (FX bind, path-pick reseed)
 *   3,8 -- damage event: force a path-reseed away from impactor;
 *          if HP drops, retire with debris cloud
 *
 * MED.
 */
#include <stdint.h>

extern int  Terrain_HeightAt(int32_t x, int32_t z);
extern int  Rand255(void);
extern uint32_t Path_Pick(uint32_t bank, int seed);
extern void Path_Init(uint32_t *pathState, uint32_t *posXYZ, uint32_t targetXY, uint32_t flag, int aux); /* FUN_80042ef0 */
extern uint32_t Path_Advance(uint32_t *self, uint32_t *pathState, int speed); /* func_0x800430a0 */
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag);
extern void Object_Suspend(void);
extern void Damage_FromImpulse(uint32_t *self, int *imp);
extern int  Damage_StandardVehicle(uint32_t *self, int *imp);
extern uint32_t _DAT_80065310;

uint32_t WW_StageCoach(uint32_t *self, int mode, int *imp)
{
    if (mode == 0) {
        if ((int16_t)self[0x21] < 0) {
            int seed = Rand255();
            uint32_t path = Path_Pick(0x80065a50u, (seed * 3 >> 15) - 0x19);
            Path_Init(self + 0x21, self + 0x12, path + 0x48, 0xffffffff, 0);
        }
        uint32_t hdg = Path_Advance(self, self + 0x21, 0x10000);
        int idx = (hdg & 0xfff) * 4;
        int sx = *(int16_t *)(idx - 0x7ff9f84c) * 0x5f5;
        if (sx < 0) sx += 0xfff;
        self[0x12] += sx >> 12;
        int sz = *(int16_t *)(idx - 0x7ff9f84a) * 0x5f5;
        if (sz < 0) sz += 0xfff;
        self[0x14] += sz >> 12;
        self[0x13] = (uint32_t)Terrain_HeightAt((int32_t)self[0x12], (int32_t)self[0x14]);
        uint32_t wheels = self[0xe];
        *(int16_t *)((char *)self + 0x42) += 0x44;
        *(int16_t *)(wheels + 0x42)      += 0x44;
        if (((_DAT_80065310 - (uint32_t)*((uint8_t *)self + 9)) & 0xf) == 0) {
            uint32_t *dust = (uint32_t *)Object_SpawnFromBank(self[0x16], 0x2ba, 0x80, 8);
            *(uint8_t *)(dust + 1) = 1;
            *dust = 0x34u;
            int rx = Rand255();
            dust[0x12] = self[0x12] + (rx * 0x5000 >> 15) - 0x2800;
            dust[0x13] = self[0x13];
            int rz = Rand255();
            dust[0x14] = self[0x14] + (rz * 0x5000 >> 15) - 0x2800;
            dust[0x19] = 0x8003e80cu;
            Object_Suspend();
        }
        return 0;
    }
    if (mode == 3) Damage_FromImpulse(self, imp);
    if (mode == 3 || mode == 8) Damage_StandardVehicle(self, imp);
    return 0;
}
