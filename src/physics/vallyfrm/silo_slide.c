/* silo_slide.c -- Valley Farms tornado silo slide/teleport.
 *
 * Source: VALLYFRM.DLL  FUN_80100eb4.
 *
 * The silo physics: it drifts along its rail (Z velocity -0x1dcd per
 * frame in active substates), teleports vehicles caught in its base,
 * and emits chaff debris every 4 frames during travel.
 *
 * substate (self+2):
 *   0  -- approach: drift -Z. On entering "boundary" zones at certain
 *         Z ranges, swap object position to a snapshot point and
 *         spawn 2 chaff sub-instances (FUN_80021c6c) facing 0x400 and
 *         0xfc00. If Z drops below 0x4a90000, decrement DAT_80101308
 *         and enter substate 1.
 *   1  -- ride: drift -Z. If Z falls below 0x33e0000, push self back
 *         +0x1dcd then RetireDeferred if past threshold.
 *   2  -- terminal: same as substate 0 with X==0x4d30000 trigger
 *         transition.
 *
 * Every 4 ticks, emits a chaff puff (object id 0x1f from bank
 * _DAT_800737d8) with random X velocity (4.12-fixed) and Y velocity
 * -0x14000.
 *
 * mode dispatch:
 *   0  -- tick (above)
 *   1  -- post-spawn init + bind FX slot 3
 *   3  -- impact event (kind==2): substate-dependent damage push to
 *         impactor velocity vector with random sub-jitter; every 32nd
 *         impact triggers Damage_VsImpactor_Burst.
 *   4  -- retire (kill FX channel)
 *
 * MED.
 */
#include <stdint.h>

extern void Object_RetireDeferred(uint32_t *self);
extern int  RandChaff(uint32_t *self);                              /* func_0x80021c6c */
extern void Object_Suspend(void);
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag);
extern int  Rand255(void);
extern uint32_t FUN_80100e70;
extern void Object_BumpSubstate_Or_FX(uint32_t *self);              /* FUN_8001d4f0 */
extern uint8_t SFX_PlayWorldXY(uint32_t *posXyz);
extern void SFX_Update(int h, int posVoxel);
extern void Damage_VsImpactor_Burst(int imp);                        /* FUN_8002c4bc */
extern uint32_t Pool_AllocProjectile(void);
extern void Pool_LaunchProjectile(uint32_t h, uint32_t bin, int kind, uint32_t *xyz);
extern uint8_t Pool_AllocSFX(void);
extern void Pool_BindFXOnObject(uint32_t h, uint32_t bin, int slot, int aux);
extern void SFX_StopWorld(int h);
extern uint32_t _DAT_80065310, _DAT_800737d8;
extern int32_t  DAT_80101308;

uint32_t VF_SiloSlide(uint32_t *self, int mode, int *arg)
{
    uint32_t kind = 1;
    if (mode == 1) goto bind_fx;
    if (mode == 0 || mode == 3 || mode != 4) {
        if (mode == 0 || (mode != 3 && mode != 4)) {
            uint8_t sub = *(uint8_t *)(self + 2);
            if (sub == 1) {
ride:
                self[9] -= 0x1dcd;
                int z = (int)self[9];
                if (z < 0x33e0000) {
                    int back = self[9] + 0x1dcd;
                    self[9] = back;
                    if (z < back) Object_RetireDeferred(self);
                }
            } else {
                uint32_t target = 2;
                if (sub > 1 || (target = 0x4d30000, sub != 0)) {
                    if (sub == target) { target = 0x3920000; goto edge; }
                }
                /* boundary check */
                int ozb = (int)self[0xb];
                int zb  = ozb - 0x1dcd;
                self[0xb] = zb;
                if ((zb <= (int)(target | 0xffff) && ozb > 0x4d40000)
                    || (zb < 0x4b50000 && ozb > 0x4b50000)) {
                    self[0x12] = self[9]; self[0x13] = self[10]; self[0x14] = self[0xb];
                    uint32_t *c1 = (uint32_t *)(intptr_t)RandChaff(self);
                    *(uint8_t *)(c1 + 8) = 1;
                    *(uint16_t *)((char *)c1 + 0x42) = 0x400;
                    Object_Suspend();
                    uint32_t *c2 = (uint32_t *)(intptr_t)RandChaff(self);
                    *(uint8_t *)(c2 + 8) = 2;
                    *(uint16_t *)((char *)c2 + 0x42) = 0xfc00u;
                    Object_Suspend();
                }
edge:
                if ((int)self[0xb] < 0x4a90000) { DAT_80101308--; goto ride; }
            }
            if ((_DAT_80065310 - (uint32_t)*((uint8_t *)self + 9)) & 3) {
                ;
            } else {
                uint32_t *p = (uint32_t *)Object_SpawnFromBank(_DAT_800737d8, 0x1f, 0x80, 8);
                *p = 0x10u;
                int r = Rand255();
                p[9]  = ((uint32_t)(r & 0xff) - 0x80) * 0x400;
                p[10] = 0;
                p[0xb] = 0xfffec000u;
                p[0x19] = (uint32_t)(uintptr_t)&FUN_80100e70;
                Object_BumpSubstate_Or_FX(self);
            }
            if (arg == NULL) return 0;
            SFX_Update((int)*((char *)self + 5), SFX_PlayWorldXY(self + 9));
        }
        if (mode == 3 || mode != 4) {
            int imp = *arg;
            if (*(char *)(imp + 4) != 2) return 0;
            uint8_t sub = *(uint8_t *)(self + 2);
            uint32_t mag;
            int jitter = 1;
            if (sub == kind) { mag = 0xfffecf00u; }
            else {
                if ((int)sub < 2) {
                    mag = (sub != 0) ? 0xfffecf00u : 0xfffe0000u;
                    if (sub == 0) {
                        *(uint32_t *)(imp + 0x88) += mag | 0xcf00u;
                        jitter = 0;
                        mag = 0xfffecf00u;
                    }
                } else {
                    mag = (sub != 2) ? 0xfffecf00u : 0x10000u;
                    if (sub != 2) {
                        *(uint32_t *)(imp + 0x88) += mag | 0xcf00u;
                        mag = 0xfffecf00u;
                    }
                }
            }
            *(uint32_t *)(imp + 0x80) += mag | 0x3100u;
            if ((uint32_t)Rand255() & 0x1f) {
                ;
            } else Damage_VsImpactor_Burst(imp);
            uint32_t h = Pool_AllocProjectile();
            Pool_LaunchProjectile(h, *(uint32_t *)(self[0x16] + 8), 5, self + 9);
            self = (uint32_t *)1;
        }
    }
bind_fx:
    *self = 0x84u;
    {
        char hh = (char)Pool_AllocSFX();
        *((char *)self + 5) = hh;
        Pool_BindFXOnObject(hh, *(uint32_t *)(self[0x16] + 8), 3, 0);
    }
    SFX_StopWorld((int)*((char *)self + 5));
    return 0;
}
