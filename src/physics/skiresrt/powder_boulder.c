/* powder_boulder.c -- Ski Resort rolling powder-snow boulder.
 *
 * Source: SKIRESRT.DLL  FUN_80100974.
 *
 * Multi-stage rolling powder-ball: same Terrain_QueryAt bounce
 * idiom as Canyonlands boulder_roll.c and Oil Fields barrel_roll.c
 * with the ski-resort layout. Substate (obj+0xa2) is a countdown
 * timer; on 1->0 transition, promotes the boulder by spawning a
 * larger sibling and bumping its growth stage.
 *
 * Bounce: v -= 2*(v.n)*n with /2 attenuation on Y. Each tick, walks
 * sub-debris chain at obj+0xe spinning each visible piece.
 *
 * mode: 0=tick, 1=spawn-finalize, 3=impact-vehicle (kind==7
 * Damage_VsImpactor then retire-with-detach), 2=silent retire.
 *
 * MED.
 */
#include <stdint.h>

extern int  Terrain_QueryAt(uint32_t *self, uint32_t *pos, int16_t *nOut, int flag);
extern void Object_OrientByAxis(uint32_t *mat, uint32_t *out, uint32_t *axis);
extern void MatrixNormal(uint32_t *m, uint32_t *out);
extern int  Damage_VsImpactor(uint32_t *self, int kind);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t a);
extern uint32_t Pool_AllocProjectile(uint32_t k);
extern void Pool_LaunchProjectile(uint32_t h, uint32_t bin, int u, void *xyz);
extern int64_t GTE_Dot32x16(uint32_t *v, uint32_t *axis);
extern void FX_RingFlash(int imp, void *p, uint32_t *xyz);
extern void Damage_VsImpactorAlt(int imp, int dmg, void *p, int n);
extern void FX_RingFlash_Init(uint32_t *self, void *p);
extern void Object_RubbleSpawn(uint8_t *bp, uint32_t *self);
extern void Object_RetireDeferred(uint32_t *self);
extern uint8_t  DAT_80100044, DAT_801012a0;
extern uint32_t _DAT_80065310;

uint32_t SK_PowderBoulder(uint32_t *self, int mode, uint32_t *arg)
{
    int16_t  n[3] = { 0, 0, 0 };
    if (mode == 2) goto retire;
    if (mode == 3) goto damage;

    /* Per-tick countdown gating substate promotion. */
    int16_t cd = *(int16_t *)((char *)self + 0xa2);
    if (cd != 0) {
        *(int16_t *)((char *)self + 0xa2) = cd - 1;
        if (cd != 1) return 0;
    }

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
        self[0x21] += 0x5a;     /* gravity */
    } else {
        self[0x21] += 0x5a;
    }
    Object_OrientByAxis(self + 4, self + 4, self + 0x23);
    self[9]  += self[0x20];
    self[10] += self[0x21];
    self[0xb]+= self[0x22];
    if ((_DAT_80065310 - (uint32_t)*((uint8_t *)self + 9)) & 0xf) return 0;
    MatrixNormal(self + 4, self + 4);

damage: {
        uint32_t imp = *arg;
        uint8_t  k   = *(uint8_t *)(imp + 4);
        if (k == 7) {
            if (Damage_VsImpactor(self, (int)*(uint16_t *)(imp + 0xc)) == 0) return 0;
            Damage_AccumulateOrFire(self, 0);
            goto retire;
        }
        if (k == 2) {
            FX_RingFlash_Init(self, (void *)(uintptr_t)imp);
            int64_t inSide = GTE_Dot32x16(self + 0x20, (uint32_t *)(imp + 0x80));
            if (inSide < 0)
                Damage_VsImpactorAlt(imp, -50, &DAT_80100044, 1);
        }
    }
retire:
    Object_RubbleSpawn(&DAT_801012a0, self);
    Object_RetireDeferred(self);
    return 0;
}
