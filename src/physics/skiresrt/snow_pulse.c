/* snow_pulse.c -- Ski Resort snow-puff pulse emitter + damage.
 *
 * Source: SKIRESRT.DLL  FUN_801016ac.
 *
 * Per-tick handler that decrements obj+2 countdown; on -1, samples
 * a path waypoint (FUN_8001b038 / FUN_8001b07c), builds a local
 * matrix with CompMatrixLV, and spawns a snow puff (object id 0x20
 * from bank _DAT_800737d8). Puff velocity = waypoint normal * (rand
 * + 0x1000) scaled by 4.12. Puff orientation uses RotMatrixYXZ with
 * a random Y angle. Then reload countdown = 8.
 *
 * Each tick also drifts ALL child puffs in the chain (+0x88..+0x90
 * velocity decay by /64 + gravity +0x38 on Y).
 *
 * mode dispatch:
 *   0  -- tick (above)
 *   1  -- post-spawn init (bind FX channel 3 with 0x40000 minimum
 *         lifetime at obj+0x15)
 *   3  -- impulse propagate + damage check
 *   8  -- damage forwarded variant
 *   4  -- retire (silence FX)
 *
 * MED.
 */
#include <stdint.h>

extern int  Rand255(void);
extern uint32_t PathPiece_Find(uint32_t piece, uint32_t flag);    /* FUN_8001b038 */
extern uint32_t Pathfinder_BuildNormal(uint8_t *out, uint32_t cur); /* FUN_8001b07c */
extern void CompMatrixLV(uint32_t mat, uint32_t a, uint8_t *b);    /* FUN_8004cf04 */
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag);
extern void RotMatrixYXZ_gte(int *yawXyz, uint32_t *outMat);
extern void Object_BumpSubstate_Or_FX(uint32_t *self);
extern void Damage_FromImpulse(uint32_t *self, int *imp);
extern int  Damage_StandardVehicle(uint32_t *self, int *imp);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t a);
extern void SFX_StopWorld(int h);
extern uint8_t Pool_AllocSFX(void);
extern void Pool_BindSnareFx(uint8_t h, uint32_t bin, int slot, int aux);
extern uint8_t SFX_PlayWorld(uint32_t *pos);
extern void SFX_Update(int h, int posVoxel);
extern uint32_t _DAT_800737d8;

uint32_t SK_SnowPulse(uint32_t *self, int mode, int param3)
{
    switch (mode) {
    case 0: {
        int8_t cd = (int8_t)(*(uint8_t *)(self + 2)) - 1;
        *(int8_t *)(self + 2) = cd;
        if (cd == -1) {
            uint32_t ch = self[0xe];
            int speed = ((Rand255() << 8) >> 15) + 0x1000;
            uint32_t wp = PathPiece_Find(ch, 0x8000);
            uint8_t  loc[20];
            uint32_t nrm = Pathfinder_BuildNormal(loc, wp);
            CompMatrixLV(ch + 0x10, nrm, loc);
            uint32_t *puff = (uint32_t *)Object_SpawnFromBank(_DAT_800737d8, 0x20, 0xa0, 8);
            *puff |= 0x410u;
            int16_t nx = *(int16_t *)(loc + 0);
            int16_t ny = *(int16_t *)(loc + 6);
            int16_t nz = *(int16_t *)(loc + 12);
            int vx = nx * speed; if (vx < 0) vx += 0xfff;
            puff[0x22] = vx >> 12;
            int vy = ny * speed; if (vy < 0) vy += 0xfff;
            puff[0x23] = vy >> 12;
            int vz = nz * speed; if (vz < 0) vz += 0xfff;
            puff[0x24] = vz >> 12;
            int rot[3] = { 0, 0, Rand255() };
            RotMatrixYXZ_gte(rot, puff + 4);
            puff[9]  = *(uint32_t *)(loc + 16);  /* spawn pos copy */
            puff[10] = *(uint32_t *)(loc + 20);
            puff[0xb]= *(uint32_t *)(loc + 24);
            puff[0x19] = 0x8004042cu;
            Object_BumpSubstate_Or_FX(self);
            *(uint8_t *)(self + 2) = 8;
        }
        for (int c = *(int *)(self[0xe] + 0x34); c != 0; c = *(int *)(c + 0x34)) {
            int vx = *(int *)(c + 0x88);
            *(int *)(c + 0x24) += vx;
            *(int *)(c + 0x28) += *(int *)(c + 0x8c);
            *(int *)(c + 0x2c) += *(int *)(c + 0x90);
            int t = vx; if (t < 0) t += 0x3f;
            *(int *)(c + 0x88) = vx - (t >> 6);
            int vz = *(int *)(c + 0x90);
            int u = vz; if (u < 0) u += 0x3f;
            *(int *)(c + 0x90) = vz - (u >> 6);
            *(int *)(c + 0x8c) += 0x38;
        }
        if (param3 == 0) return 0;
        SFX_Update((int)*((char *)self + 5), SFX_PlayWorld(self + 9));
        /* fall to case 3 */
    }
    case 3:
        Damage_FromImpulse(self, (int *)(intptr_t)param3);
        /* fall to case 8 */
    case 8:
        if (Damage_StandardVehicle(self, (int *)(intptr_t)param3) != 0) {
            Damage_AccumulateOrFire(self, 0);
            SFX_StopWorld((int)*((char *)self + 5));
            *((char *)self + 5) = 0;
            goto init;
        }
        break;
    case 1:
init: {
        *(uint8_t *)(self + 2) = *((uint8_t *)self + 9) & 7;
        *self |= 0x80u;
        uint32_t life = 0x40000;
        if ((int)self[0x15] > 0x40000) life = self[0x15];
        self[0x15] = life;
        char h = (char)Pool_AllocSFX();
        *((char *)self + 5) = h;
        Pool_BindSnareFx((uint8_t)h, *(uint32_t *)(self[0x16] + 8), 3, 0);
    }
    /* fall to case 4 */
    case 4:
        SFX_StopWorld((int)*((char *)self + 5));
        break;
    default: break;
    }
    return 0;
}
