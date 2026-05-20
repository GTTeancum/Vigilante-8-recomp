/* boulder_roll.c -- Canyonlands rolling-boulder physics+impact.
 *
 * Source: CANYNLND.DLL  FUN_80100244.
 *
 * Per-tick handler for a rolling-boulder destructible: integrates
 * velocity against terrain with surface-normal bounce response,
 * applies +0x5a gravity to Y, computes spin axis from horizontal
 * velocity, retires on hard impact or after first vehicle hit.
 *
 * mode dispatch:
 *   0  -- per-tick: terrain query + bounce + integrate + spin axis
 *   2  -- final retire (spawn rubble + free)
 *   3  -- impactor collision (param_3 = impactor):
 *         kind 7 = vehicle hit; scale Y vel by 0x93c/4096, recompute
 *                  spin period = 0x1000000 / (Y * 0x3243); retire if
 *                  not already breaking.
 *
 * Offsets: self+9..b = world pos (mirrors +0x12..14);
 *          self+0x15 gravity Y offset; self+0x1c = debris cell ptr;
 *          self+0x20..22 vel; self+0x23..24 spin axis;
 *          self+0x25 inv spin radius (4.12).
 *
 * MED.
 */
#include <stdint.h>

extern int  Terrain_QueryAt(uint32_t *self, uint32_t *pos, int16_t *nOut, int flag);
extern void Object_OrientByAxis(uint32_t *mat, uint32_t *out, uint32_t *axis);
extern void MatrixNormal(uint32_t *m, uint32_t *out);
extern int  Damage_VsImpactor(void *self, int kind);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t a);
extern void Damage_RetireSelf(uint32_t *self);
extern int64_t GTE_Dot32x16(uint32_t *v, uint32_t *axis);
extern void FX_RingFlash(int imp, void *p, uint32_t *xyz);
extern void Damage_VsImpactorAlt(int imp, int kind, void *p, int n);
extern void FX_RingFlash_Init(uint32_t *self, void *p);
extern void Object_RubbleSpawn(uint8_t *bp, uint32_t *self);
extern void Object_RetireDeferred(uint32_t *self);
extern uint32_t Pool_AllocProjectile(uint32_t kind);
extern void Pool_LaunchProjectile(uint32_t h, uint32_t bin, int u, void *xyz);
extern uint8_t  DAT_80100044, DAT_801012a0;
extern int8_t   DAT_801012ac, _DAT_000012ad;
extern uint32_t _DAT_80065310;

uint32_t CL_RollingBoulder(uint32_t *self, uint32_t mode, uint32_t *arg)
{
    uint32_t hit = 2;
    int16_t  n[3] = { 0, 0, 0 };

    if (mode == 2) goto retire;
    if (mode == 0) {
        uint32_t pos[3] = { self[9], self[10] + self[0x15], self[0xb] };
        int gy = Terrain_QueryAt(self, pos, n, 0);
        if (gy < (int)pos[1] + 0x800) {
            int32_t vdot = (int32_t)self[0x20] * n[0]
                         + (int32_t)self[0x21] * n[1]
                         + (int32_t)self[0x22] * n[2];
            if (vdot < 0) vdot += 0x7ff;
            vdot >>= 11;
            int32_t vy2 = (int32_t)self[0x21] * 2;
            if (vdot < 0) {
                int32_t ax = vdot * n[0]; if (ax < 0) ax += 0xfff;
                int32_t ay = vdot * n[1]; if (ay < 0) ay += 0xfff;
                int32_t az = vdot * n[2]; if (az < 0) az += 0xfff;
                self[0x20] -= ax >> 12;
                self[0x21] -= ay >> 12;
                self[0x22] -= az >> 12;
                vy2          = (int32_t)self[0x21] / 2;
                self[0x21]   = vy2;
                self[10]     = gy - self[0x15];
            }
            int32_t rollX = (vy2 + (int32_t)self[0x20]) * 0x1e;
            if (rollX < 0) rollX += 0xfff;
            self[0x20] += rollX >> 12;
            int32_t fz = n[2] * 0x5a; if (fz < 0) fz += 0xfff;
            self[0x22] += fz >> 12;
            int32_t sx = -(int32_t)self[0x22] * (uint16_t)self[0x25];
            if (sx < 0) sx += 0xfff;
            int32_t sz =  (int32_t)self[0x20] * (uint16_t)self[0x25];
            *(int16_t *)(self + 0x23) = (int16_t)(sx >> 12);
            if (sz < 0) sz += 0xfff;
            *(int16_t *)(self + 0x24) = (int16_t)(sz >> 12);
            uint32_t hard = (vdot < -0x1c9);
            if (n[1] < -0xe66) {
                uint32_t ax = (int32_t)self[0x21] < 0 ? -self[0x21] : self[0x21];
                uint32_t bx = (int32_t)self[0x20] < 0 ? -self[0x20] : self[0x20];
                if (ax < bx) ax = bx;
                uint32_t cx = (int32_t)self[0x22] < 0 ? -self[0x22] : self[0x22];
                if (cx < ax) cx = ax;
                if (cx < 0x42c) { hard = (uint32_t)Damage_AccumulateOrFire(self, 0); hit = 1; }
            }
            if (hard) {
                uint32_t h = Pool_AllocProjectile(hit);
                Pool_LaunchProjectile(h, *(uint32_t *)(self[0x16] + 8), 0, self + 9);
                goto post_gravity;
            }
        } else {
post_gravity:
            self[0x21] += 0x5a;
        }
        arg = self + 4;
        Object_OrientByAxis(arg, arg, self + 0x23);
        self[9]  += self[0x20];
        self[10] += self[0x21];
        self[0xb]+= self[0x22];
        if ((_DAT_80065310 - (uint32_t)*((uint8_t *)self + 9)) & 0xf) return 0;
        MatrixNormal(arg, arg);
    }
    /* Damage / impactor handling. */
    uint32_t imp  = *arg;
    uint8_t  kind = *(uint8_t *)(imp + 4);
    if (kind == 7) {
        if (Damage_VsImpactor(self, (int)*(uint16_t *)(imp + 0xc)) == 0) return 0;
        uint32_t g = self[0x15];
        if (g == 0) goto retire;
        uint32_t debris = self[0x1c];
        if ((int)g < 0) g += 0xf;
        *(int32_t *)(debris + 0x28) = (int32_t)g >> 4;
        *(int32_t *)(debris + 0x24) = (int32_t)g >> 4;
        int32_t sc = (int)self[0x15] * 0x93c; if (sc < 0) sc += 0xfff;
        self[0x15] = sc >> 12;
        int32_t inv = (sc >> 12) * 0x3243; if (inv < 0) inv += 0xfff;
        *(int16_t *)(self + 0x25) = (int16_t)(0x1000000 / (inv >> 12));
        if ((*self & 0x80) != 0) return 0;
        Damage_RetireSelf(self);
    }
    if ((*self & 0x80) == 0) return 0;
    if (kind == 7) {
        uint32_t scale = 0x10000 / *(uint16_t *)(imp + 0xa2);
        int64_t inSide = GTE_Dot32x16(self + 0x20, (uint32_t *)(imp + 0x80));
        if (inSide > 0) return 0;
        int32_t ix = (int32_t)(self[0x20] * scale);
        int32_t iy = (int32_t)(self[0x21] * scale);
        int32_t iz = (int32_t)(self[0x22] * scale);
        int32_t c[3];
        c[0] = (ix < -0x100000) ? -0x100000 : (ix > 0x100000) ? 0x100000 : ix;
        c[1] = (iy < -0x100000) ? -0x100000 : (iy > 0x100000) ? 0x100000 : iy;
        c[2] = (iz < -0x100000) ? -0x100000 : (iz > 0x100000) ? 0x100000 : iz;
        FX_RingFlash((int)imp, c, self + 9);
        Damage_VsImpactorAlt(*arg, -100, &DAT_80100044, 1);
    }
    FX_RingFlash_Init(self, (void *)(uintptr_t)imp);
    int32_t vdot = (int32_t)self[0x20] * (int16_t)*(uint32_t *)(imp + 0x20)
                 + (int32_t)self[0x21] * *(int16_t *)(imp + 0x22)
                 + (int32_t)self[0x22] * (int16_t)*(uint32_t *)(imp + 0x24);
    if (vdot < 0) vdot += 0x7ff;
    vdot >>= 11;
    if (vdot >= 0) return 0;
    int32_t bx = vdot * (int16_t)*(uint32_t *)(imp + 0x20); if (bx < 0) bx += 0xfff;
    int32_t by = vdot * *(int16_t *)(imp + 0x22);            if (by < 0) by += 0xfff;
    int32_t bz = vdot * (int16_t)*(uint32_t *)(imp + 0x24); if (bz < 0) bz += 0xfff;
    self[0x20] -= bx >> 12;
    self[0x21] -= by >> 12;
    self[0x22] -= bz >> 12;
retire:
    Object_RubbleSpawn(&DAT_801012a0, self);
    Object_RetireDeferred(self);
    _DAT_000012ad = (int8_t)(DAT_801012ac - 1);
    return 0;
}
