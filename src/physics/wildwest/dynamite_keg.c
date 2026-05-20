/* dynamite_keg.c -- Wild West dynamite keg destructible + reflect.
 *
 * Source: WILDWEST.DLL  FUN_801009a8.
 *
 * Per-tick handler for breakable dynamite kegs. Every 4 frames emits
 * a spark puff (object id 0x21 from _DAT_800737d8) when "lit"
 * (obj+2 high bit set). On impact:
 *   - vehicle hit (kind 7): apply damage; if killed, mark lit
 *     (obj+2 = 0xff), reset velocity to scaled facing dir, schedule
 *     300-frame explosion timer, alarm via FX_QueueAtTarget @0x80065a18
 *     kind 9, and free SFX.
 *   - shrapnel (kind 2): test collision via func_0x8001f974 +
 *     GTE rotate; compute impact-velocity reflection (extensive 64-bit
 *     math at lines 132-156 - cross-product impulse), apply -P/8 to
 *     impactor velocity via Damage_VsImpactorAlt, spawn fragment fx
 *     at impact point.
 *   - any kind with callback at iVar6+100: invoke callback(self, 8,
 *     1000) - chained damage handler, then detach + retire if lit.
 *
 * mode dispatch:
 *   0  -- tick + spark
 *   1  -- post-spawn: pick path-piece spawn (FUN_8001b038/8001b2fc)
 *         and bind FX channel 0
 *   2  -- impact-confirmed retire
 *   4  -- silence FX
 *   5,6 -- routed targets
 *
 * MED.
 */
#include <stdint.h>

extern uint8_t Pool_AllocSFX(void);
extern void Pool_BindSnareToObject(uint8_t h, uint32_t bin, int slot, uint32_t *xyz);
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag);
extern void Object_Suspend(void);
extern int  Damage_StandardVehicle(uint32_t *self, void *imp);
extern void Damage_Apply_AgainstSelf(void *self, void *param);
extern void SFX_StopWorld(int h);
extern void FX_QueueAtTarget(uint32_t bank, int kind, uint32_t *self);
extern void SubModel_Detach(uint32_t *self);
extern void Object_RetireDeferred(uint32_t *self);
extern int  ShrapnelCollide(uint32_t *self, int *imp, uint8_t *normal); /* func_0x8001f974 */
extern void GTE_BuildLocalNormal(uint32_t *mat, uint8_t *n, uint8_t *out); /* FUN_800434f8 */
extern int64_t GTE_CrossPlanar(int *a, uint8_t *b);                   /* FUN_80017240 */
extern void Object_BounceImpulse(int imp, uint32_t *p, uint8_t *out); /* FUN_80017594 */
extern void Damage_VsImpactorAlt(int imp, int dmg, void *p, int n);
extern void GTE_RotateLongMatTrans(uint32_t *mat, uint8_t *src, uint8_t *dst);
extern void Pool_BindFXFragment(uint32_t h, uint32_t bin, int kind, uint8_t *spawnXyz); /* FUN_800447e8 */
extern void Particles_Burst(uint8_t *spawnXyz);
extern int  PathPiece_Find(uint32_t *self, uint32_t flag);             /* FUN_8001b038 */
extern uint32_t Spawner_DefferedSlot(uint32_t bin, int kind, void *p); /* FUN_800407b4 */
extern void Path_BindSpawner(uint32_t *self, int pp, uint32_t spawner);/* FUN_8001b2fc */
extern void Damage_RetireSelf(uint32_t spawner);
extern void Object_BindFinalize(uint32_t spawner);
extern uint8_t SFX_PlayWorldXY(uint32_t *posXyz);
extern void SFX_Update(int h, int posVoxel);
extern uint32_t _DAT_80065310, _DAT_800737d8, _DAT_800658fc;
extern uint8_t  DAT_80100130, DAT_80060000;
extern uint32_t FUN_80100950;

uint32_t WW_DynamiteKeg(uint32_t *self, uint32_t mode, int *imp)
{
    if (mode > 6) return 0;
    switch (mode) {
    case 0:
        if (self[0x29] != 0 && (*(uint16_t *)(self[0x29] + 0xc) & 0x100)) {
            uint8_t h = Pool_AllocSFX();
            Pool_BindSnareToObject(h, *(uint32_t *)(self[0x16] + 8), 6, self + 9);
        }
        if ((char)self[2] < 0) {
            if ((_DAT_80065310 - (uint32_t)*((uint8_t *)self + 9)) & 3) return 0;
            uint32_t *spark = (uint32_t *)Object_SpawnFromBank(_DAT_800737d8, 0x21, 0x80, 8);
            int16_t  cnt   = (int16_t)_DAT_80065310;
            *spark |= 0x4b4u;
            spark[0x12] = self[9]; spark[0x13] = self[10]; spark[0x14] = self[0xb];
            *(int16_t *)(spark + 0x11) = (int16_t)(cnt * 0x60);
            spark[0x19] = (uint32_t)(uintptr_t)&FUN_80100950;
            Object_Suspend();
        }
        if (imp == NULL) return 0;
        SFX_Update((int)*((char *)self + 5), SFX_PlayWorldXY(self + 9));
        break;
    case 1: goto path_bind;
    case 2: goto retire;
    case 4: goto silence_fx;
    case 5: return 0;
    case 6: goto path_bind;
    }

    int      impObj = *imp;
    uint8_t  k      = *(uint8_t *)(impObj + 4);
    uint8_t  want   = 2;
    if (k == 7) {
        if ((char)self[2] < 0) return 0;
        if (Damage_StandardVehicle(self, (void *)(intptr_t)*(uint16_t *)(impObj + 0xc)) == 0) return 0;
        int vx = (int)*(int16_t *)(self + 5) * self[0x2a];
        self[0x29] = 0;
        *(uint8_t *)(self + 2) = 0xff;
        *self &= ~0x100u;
        if (vx < 0) vx += 0x1f;
        self[0x20] = vx >> 5;
        self[0x21] = 0;
        int vz = (int)*(int16_t *)(self + 8) * self[0x2a];
        if (vz < 0) vz += 0x1f;
        self[0x22] = vz >> 5;
        Damage_Apply_AgainstSelf(self, (void *)(intptr_t)300);
        self[0x25] = 30000;
        SFX_StopWorld((int)*((char *)self + 5));
        *((char *)self + 5) = 0;
        Damage_Apply_AgainstSelf(self, (void *)(intptr_t)300);
        FX_QueueAtTarget(0x80065a18u, 9, self);
        want = 0;
    }
    if (k == want) {
        uint8_t hit[20], dn[12], xy[8], wpos[16];
        ShrapnelCollide(self, imp, hit);
        GTE_BuildLocalNormal(self + 4, xy, hit + 8);
        if (*(int16_t *)(hit + 12) >= 0x801) return 0;
        int vx = (int)*(int16_t *)(self + 5) * self[0x2a];
        if (vx < 0) vx += 0x1f;
        int local[3];
        local[0] = *(int *)(impObj + 0x80) - (vx >> 5);
        local[1] = *(int *)(impObj + 0x84);
        int vz = (int)*(int16_t *)(self + 8) * self[0x2a];
        if (vz < 0) vz += 0x1f;
        local[2] = *(int *)(impObj + 0x88) - (vz >> 5);
        int64_t r = GTE_CrossPlanar(local, xy);
        int     hi = (int)(r >> 32);
        uint32_t p = (uint32_t)((uint32_t)r >> 11 | (uint32_t)hi << 21);
        if ((int)p >= 0) return 0;
        Object_BounceImpulse(impObj, (uint32_t *)&p, dn);
        Damage_VsImpactorAlt(impObj, (int)(p + 0x1fffu) >> 13, dn, 1);
        GTE_RotateLongMatTrans((uint32_t *)(impObj + 0x10), dn, wpos);
        uint32_t h = Pool_AllocSFX();
        Pool_BindFXFragment(h, _DAT_800658fc, 5, wpos);
        Particles_Burst(wpos);
    }
    if (*(void (**)(int, int, int))(impObj + 100) != NULL) {
        (*(void (**)(int, int, int))(impObj + 100))(impObj, 8, 1000);
retire:
        if ((char)self[2] < 0) {
            SubModel_Detach(self);
            Object_RetireDeferred(self);
        }
        *(uint8_t *)(self + 2) = 0xff;
        SubModel_Detach(self);
        SFX_StopWorld((int)*((char *)self + 5));
        *((char *)self + 5) = 0;
        Damage_Apply_AgainstSelf(self, (void *)(intptr_t)300);
path_bind: {
            int pp = PathPiece_Find(self, 0x8000);
            if (pp != 0) {
                uint32_t sp = Spawner_DefferedSlot(_DAT_800737d8, 6, &DAT_80100130);
                Path_BindSpawner(self, pp, sp);
                Damage_RetireSelf(sp);
                if ((*self & 4) == 0) Object_BindFinalize(sp);
                char hh = (char)Pool_AllocSFX();
                *((char *)self + 5) = hh;
                Pool_BindSnareToObject((uint8_t)hh, *(uint32_t *)(self[0x16] + 8), 0, NULL);
silence_fx:
                SFX_StopWorld((int)*((char *)self + 5));
            }
        }
    }
    return 0;
}
