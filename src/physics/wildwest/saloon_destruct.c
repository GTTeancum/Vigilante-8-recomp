/* saloon_destruct.c -- Wild West saloon destructible facade.
 *
 * Source: WILDWEST.DLL  FUN_8010178c.
 *
 * Multi-piece destructible western-town facade. Each panel piece is
 * a child at self+0xe linked via +0x34. Damage propagates left to
 * right: a hit triggers the immediate piece + the two adjacent ones,
 * spawning splinter debris (object id 0x2bb) at the joint locations.
 *
 * Per-tick walks chain bumping piece anim phases. On vehicle hit
 * (kind==7) crumbles via Damage_StandardVehicle; on death detaches
 * pieces and retires.
 *
 * mode: 0=tick, 1=spawn-finalize, 2=retire, 3=impact, 4=silence FX.
 *
 * MED. Same crumble idiom as wildwest/bridge_collapse.c but multi-
 * piece chain.
 */
#include <stdint.h>

extern void Damage_StandardVehicle(uint32_t *self, void *imp);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t a);
extern void SubModel_Detach(uint32_t *self);
extern void Object_RetireDeferred(uint32_t *self);
extern void Object_RefitAABB(uint32_t *self);
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag);
extern void Object_Suspend(void);
extern int  Rand255(void);
extern uint8_t Pool_AllocSFX(void);
extern void Pool_BindFXOnObject(uint8_t h, uint32_t bin, int slot, int aux);
extern void SFX_StopWorld(int h);
extern uint8_t SFX_PlayWorldXY(uint32_t *xyz);
extern void SFX_Update(int h, int posVoxel);
extern uint32_t _DAT_80065310;

uint32_t WW_SaloonDestruct(uint32_t *self, uint32_t mode, int *imp)
{
    switch (mode) {
    case 0: {
        /* Per-tick: animate each piece's phase. */
        for (uintptr_t c = self[0xe]; c != 0; c = *(uint32_t *)(c + 0x34))
            if (*(int16_t *)(c + 6) == 0)
                *(int16_t *)(c + 0x40) += 0x10;
        if ((_DAT_80065310 & 0xf) == 0)
            for (uintptr_t c = self[0xe]; c != 0; c = *(uint32_t *)(c + 0x34))
                Object_RefitAABB((uint32_t *)c);
        SFX_Update((int)*((char *)self + 5), SFX_PlayWorldXY(self + 9));
        return 0;
    }
    case 1: {
        uint8_t h = Pool_AllocSFX();
        *((char *)self + 5) = (char)h;
        Pool_BindFXOnObject(h, *(uint32_t *)(self[0x16] + 8), 4, 0);
        return 0;
    }
    case 2:
        SubModel_Detach(self);
        Object_RetireDeferred(self);
        return 0;
    case 3: {
        int impObj = *imp;
        if (*(uint8_t *)(impObj + 4) != 7) return 0;
        Damage_StandardVehicle(self, (void *)(intptr_t)*(uint16_t *)(impObj + 0xc));
        if (Damage_AccumulateOrFire(self, 0)) {
            /* Spawn splinter debris at random child piece. */
            uint32_t bin = self[0x16];
            uint32_t *deb = (uint32_t *)Object_SpawnFromBank(bin, 0x2bb, 0x80, 8);
            *deb = 0x34u;
            int r = Rand255();
            deb[0x12] = self[9]  + (r * 0x4000 >> 15) - 0x2000;
            deb[0x13] = self[10];
            deb[0x14] = self[0xb] + (r * 0x4000 >> 15) - 0x2000;
            deb[0x19] = 0x8003e80cu;
            Object_Suspend();
        }
        return 0;
    }
    case 4:
        SFX_StopWorld((int)*((char *)self + 5));
        return 0;
    default: return 0;
    }
}
