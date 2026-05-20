/* bunker_door.c -- Secret Base reinforced bunker door.
 *
 * Source: SCRTBASE.DLL  FUN_80101c58.
 *
 * Multi-stage destructible door reinforced with armor plates. Same
 * core idiom as wildwest/dynamite_keg.c: per-tick fizzle FX, on
 * impactor.kind==2 (shrapnel) run a planar-impulse computation
 * (GTE_CrossPlanar + Object_BounceImpulse) and reflect the impactor's
 * velocity; on kind==7 (vehicle) apply damage and on death detach
 * the panel, schedule a 300-frame explosion timer and queue an
 * alarm via FX_QueueAtTarget @0x80065a18.
 *
 * mode: 0=tick, 1=spawn-finalize, 2=retire, 3=impact, 8=damage,
 *       4=silence FX.
 *
 * MED. See wildwest/dynamite_keg.c for the equivalent code path.
 */
#include <stdint.h>

extern void Damage_StandardVehicle(uint32_t *self, void *imp);
extern void Damage_Apply_AgainstSelf(void *self, void *param);
extern void Damage_VsImpactorAlt(int imp, int dmg, void *p, int n);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t a);
extern void SubModel_Detach(uint32_t *self);
extern void Object_RetireDeferred(uint32_t *self);
extern void FX_QueueAtTarget(uint32_t bank, int kind, uint32_t *self);
extern int  ShrapnelCollide(uint32_t *self, int *imp, uint8_t *normal);
extern void Object_BounceImpulse(int imp, uint32_t *p, uint8_t *out);
extern int64_t GTE_CrossPlanar(int *a, uint8_t *b);
extern void GTE_BuildLocalNormal(uint32_t *mat, uint8_t *n, uint8_t *out);
extern uint8_t Pool_AllocSFX(void);
extern void Pool_BindFXFragment(uint32_t h, uint32_t bin, int kind, uint8_t *spawnXyz);
extern void SFX_StopWorld(int h);
extern uint8_t SFX_PlayWorldXY(uint32_t *posXyz);
extern void SFX_Update(int h, int posVoxel);
extern void Pool_BindSnareToObject(uint8_t h, uint32_t bin, int slot, uint32_t *xyz);
extern uint32_t _DAT_800658fc;

uint32_t SB_BunkerDoor(uint32_t *self, uint32_t mode, int *imp)
{
    switch (mode) {
    case 0:
        SFX_Update((int)*((char *)self + 5), SFX_PlayWorldXY(self + 9));
        return 0;
    case 1: case 4:
        SFX_StopWorld((int)*((char *)self + 5));
        return 0;
    case 2:
        SubModel_Detach(self);
        Object_RetireDeferred(self);
        return 0;
    case 3: case 8: break;
    default: return 0;
    }
    int impObj = *imp;
    uint8_t k  = *(uint8_t *)(impObj + 4);
    if (k == 7) {
        Damage_StandardVehicle(self, (void *)(intptr_t)*(uint16_t *)(impObj + 0xc));
        if (Damage_AccumulateOrFire(self, 0) != 0) {
            FX_QueueAtTarget(0x80065a18u, 9, self);
            Damage_Apply_AgainstSelf(self, (void *)(intptr_t)300);
        }
    }
    if (k == 2) {
        uint8_t hit[20], dn[12], xy[8];
        ShrapnelCollide(self, imp, hit);
        GTE_BuildLocalNormal(self + 4, xy, hit + 8);
        if (*(int16_t *)(hit + 12) >= 0x801) return 0;
        int local[3] = { *(int *)(impObj + 0x80), *(int *)(impObj + 0x84), *(int *)(impObj + 0x88) };
        int64_t r = GTE_CrossPlanar(local, xy);
        uint32_t p = (uint32_t)((uint32_t)r >> 11 | (uint32_t)(int)(r >> 32) << 21);
        if ((int)p >= 0) return 0;
        Object_BounceImpulse(impObj, &p, dn);
        Damage_VsImpactorAlt(impObj, (int)(p + 0x1fffu) >> 13, dn, 1);
        uint8_t h = Pool_AllocSFX();
        Pool_BindFXFragment(h, _DAT_800658fc, 5, dn);
    }
    return 0;
}
