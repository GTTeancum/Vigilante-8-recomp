/* barrel_roll.c -- Oil Fields rolling-barrel physics+impact.
 *
 * Source: OILFIELD.DLL  FUN_80100a30.
 *
 * Same rolling-object idiom as Canyonlands boulder_roll.c, applied
 * to an oil barrel. Different offsets (self+0x24..2c position vs
 * +9..b on Canyonlands) because the Oil Fields object layout omits
 * the cached short-position alias.
 *
 * Velocity:   self+0x80..88 (X,Y,Z)
 * Spin axis:  self+0x8c, +0x90 (X,Z); inv-radius self+0x94
 * Bounce:     v -= 2 * (v.n) * n; Y bounce halved (Y *= 0.5)
 * Gravity:    +0x5a on Y per frame
 * Friction:   horizontal roll-back +0x1e via (vy + vx)*0x1e/4096
 *
 * Damage path:
 *   - impactor.kind == 7 (vehicle): standard Damage_VsImpactor;
 *     compute small relative-velocity impulse scaled 8x; on damage
 *     death, detach submodel and retire.
 *   - impactor.kind == 2 (shrapnel): face-normal bounce only.
 *
 * MED.
 */
#include <stdint.h>

extern int  Terrain_QueryAt(uint32_t *self, uint32_t *pos, int16_t *nOut, int flag);
extern void Object_OrientByAxis(uint32_t *mat, uint32_t *out, uint32_t *axis); /* func_0x800172b4 */
extern void MatrixNormal(uint32_t *m, uint32_t *out);
extern int  Damage_VsImpactor(int self, int kind);
extern void SubModel_Detach(int self);
extern void Object_RetireDeferred(int self);
extern void FX_RingFlash_Init(int self, int *imp);
extern uint32_t _DAT_80065310;

uint32_t OF_BarrelRoll(int self, int mode, int *arg)
{
    int16_t n[3] = { 0, 0, 0 };
    if (mode == 0 || mode != 3) {
        uint32_t pos[3];
        pos[0] = *(uint32_t *)(self + 0x24);
        pos[1] = *(uint32_t *)(self + 0x28) + *(uint32_t *)(self + 0x54);
        pos[2] = *(uint32_t *)(self + 0x2c);
        int gy = Terrain_QueryAt((uint32_t *)(intptr_t)self, pos, n, 0);
        if (gy < (int)pos[1] + 0x800) {
            int vx = *(int *)(self + 0x80);
            int vy = *(int *)(self + 0x84);
            int vz = *(int *)(self + 0x88);
            int vdot = vx * n[0] + vy * n[1] + vz * n[2];
            if (vdot < 0) vdot += 0x7ff;
            vdot >>= 11;
            int dy = n[0] * 2;
            if (vdot < 0) {
                int ax = vdot * n[0]; if (ax < 0) ax += 0xfff;
                int ay = vdot * n[1]; if (ay < 0) ay += 0xfff;
                int az = vdot * n[2]; if (az < 0) az += 0xfff;
                *(int *)(self + 0x80) = vx - (ax >> 12);
                *(int *)(self + 0x84) = ((*(int *)(self + 0x84) - (ay >> 12)) / 2);
                *(int *)(self + 0x88) = vz - (az >> 12);
                *(int *)(self + 0x28) = 1;
                dy = az >> 12;
            }
            int rollX = (dy + n[0]) * 0x1e;
            if (rollX < 0) rollX += 0xfff;
            *(int *)(self + 0x80) = vx + (rollX >> 12);
            int fz = n[2] * 0x5a; if (fz < 0) fz += 0xfff;
            *(int *)(self + 0x88) += fz >> 12;
            int sx = -*(int *)(self + 0x88) * (uint16_t)*(uint16_t *)(self + 0x94);
            if (sx < 0) sx += 0xfff;
            int sz =  *(int *)(self + 0x80) * (uint16_t)*(uint16_t *)(self + 0x94);
            *(int16_t *)(self + 0x8c) = (int16_t)(sx >> 12);
            if (sz < 0) sz += 0xfff;
            *(int16_t *)(self + 0x90) = (int16_t)(sz >> 12);
        }
        *(int *)(self + 0x84) += 0x5a;
        int *m = (int *)(self + 0x10);
        Object_OrientByAxis((uint32_t *)m, (uint32_t *)m, (uint32_t *)(self + 0x8c));
        *(int *)(self + 0x24) += *(int *)(self + 0x80);
        *(int *)(self + 0x28) += *(int *)(self + 0x84);
        *(int *)(self + 0x2c) += *(int *)(self + 0x88);
        if ((_DAT_80065310 - (uint32_t)*(uint8_t *)(self + 9)) & 0xf) return 0;
        MatrixNormal((uint32_t *)m, (uint32_t *)m);
    }
    int imp  = *arg;
    uint8_t k = *(uint8_t *)(imp + 4);
    uint8_t want = 2;
    if (k == 7) {
        if (Damage_VsImpactor(self, (int)*(int16_t *)(imp + 0xc)) == 0) return 0;
        want = 0;
        imp = 1;
    }
    if (k == want) {
        SubModel_Detach(self);
        Object_RetireDeferred(self);
    }
    FX_RingFlash_Init(self, arg);
    int vdot = *(int *)(self + 0x80) * (int16_t)arg[8]
             + *(int *)(self + 0x84) * *(int16_t *)((char *)arg + 0x22)
             + *(int *)(self + 0x88) * (int16_t)arg[9];
    if (vdot < 0) vdot += 0x7ff;
    vdot >>= 11;
    if (vdot >= 0) return 0;
    int bx = vdot * (int16_t)arg[8];                     if (bx < 0) bx += 0xfff;
    int by = vdot * *(int16_t *)((char *)arg + 0x22);     if (by < 0) by += 0xfff;
    int bz = vdot * (int16_t)arg[9];                     if (bz < 0) bz += 0xfff;
    *(int *)(self + 0x80) -= bx >> 12;
    *(int *)(self + 0x84) -= by >> 12;
    *(int *)(self + 0x88) -= bz >> 12;
    return 0;
}
