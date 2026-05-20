/* silo_door.c -- Secret Base missile-silo blast door.
 *
 * Source: SCRTBASE.DLL  FUN_8010266c.
 *
 * Pair to bunker_door.c. Operates a vertical blast door tied to a
 * paired switch object: when both halves are armed (state byte at
 * self+8 reaches 2), opens to expose the silo bay, allowing
 * SB_HomingMissile launches.
 *
 * mode dispatch:
 *   0  -- per-tick: maintain SFX and check arm conditions
 *   1  -- post-spawn init (bind FX, set state=0)
 *   3,8 -- impact handling (switch on impactor kind matching pad)
 *   2  -- silent retire / re-arm
 *   4  -- silence FX
 *
 * MED. Same arm-pair structure as scrtbase/security_door.c.
 */
#include <stdint.h>

extern void Damage_FromImpulse(uint32_t *self, int *imp);
extern int  Damage_StandardVehicle(uint32_t *self, int *imp);
extern void Damage_Apply_AgainstSelf(void *self, void *param);
extern void Damage_RetireSelfAlt(uint32_t *self);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t a);
extern void Door_SlideTo(uint32_t *self, uint32_t target);
extern uint8_t Pool_AllocSFX(void);
extern void Pool_BindFXOnObject(uint8_t h, uint32_t bin, int slot, int aux);
extern void SFX_StopWorld(int h);
extern uint8_t SFX_PlayWorldXY(uint32_t *xyz);
extern void SFX_Update(int h, int posVoxel);

uint32_t SB_SiloDoor(uint32_t *self, uint32_t mode, int *imp)
{
    switch (mode) {
    case 0:
        SFX_Update((int)*((char *)self + 5), SFX_PlayWorldXY(self + 0x12));
        return 0;
    case 1: {
        *(uint8_t *)(self + 8) = 0;
        uint8_t h = Pool_AllocSFX();
        *((char *)self + 5) = (char)h;
        Pool_BindFXOnObject(h, *(uint32_t *)(self[0x16] + 8), 3, 0);
        return 0;
    }
    case 2: case 4:
        SFX_StopWorld((int)*((char *)self + 5));
        return 0;
    case 3:
        Damage_FromImpulse(self, imp);
        /* fall */
    case 8: {
        if (Damage_StandardVehicle(self, imp) == 0) return 0;
        uint8_t k = *(uint8_t *)(*imp + 4);
        if (k == 2 || k == 1) {
            uint8_t st = *(uint8_t *)(self + 8) + 1;
            *(uint8_t *)(self + 8) = st;
            if (st == 2) {
                Door_SlideTo(self, 0x80000);   /* fully open */
                Damage_Apply_AgainstSelf(self, (void *)(intptr_t)300);
                Damage_RetireSelfAlt(self);
                Damage_AccumulateOrFire(self, 0);
            }
        }
        return 0;
    }
    default: return 0;
    }
}
