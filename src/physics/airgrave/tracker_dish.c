/* tracker_dish.c -- AirGrave radar/AA dish destructible tracker.
 *
 * Source: AIRGRAVE.DLL  FUN_80100228.
 *
 * Multi-stage destructible: the dish rotates to track the local
 * player, and fires when armed. Each damage tick advances through
 * health stages (the +8 byte at obj+0x38 = current stage, decremented
 * by the damage path); when it reaches 1 the model swaps and the rate
 * of fire jumps. Final hit explodes.
 *
 * mode dispatch:
 *   0  -- per-tick rotate + fire-cycle update + AABB refit
 *   1  -- post-spawn AABB/ground init
 *   3  -- damage event (param_3 = impactor)
 *   8  -- explicit damage amount (param_3 = damage scalar)
 *
 * The 0x17d7 / nFrames constant is the projectile reload divisor
 * (4.12 fixed) -- larger nFrames slows reload.
 *
 * MED.
 */
#include <stdint.h>

extern int      Terrain_HeightAt(int32_t x, int32_t z);              /* FUN_80025400 */
extern int      Object_LocalToWorldPos(void);                        /* FUN_8001d624 -- returns ptr */
extern void     Object_RefitAABB(void *self);                        /* FUN_8001d708 */
extern int      ratan2(int dz, int dx);                              /* FUN_8004ecd4 */
extern void     SubModel_Detach(uint32_t modelHandle);               /* FUN_8003fc50 */
extern char     SubModel_PickInitialStage(uint32_t modelHandle);     /* FUN_8003fc94 */
extern int      SubModel_AddrAt(int worldXyz);                       /* FUN_8003fd24 */
extern uint32_t Pool_AllocProjectile(void);                          /* FUN_8004410c */
extern void     Pool_LaunchProjectile(uint32_t handle, uint32_t bin,
                                      int unused, int spawnXyz);     /* FUN_8004483c */
extern int      Damage_LookupImpactor(void *self, int kind,
                                      void *params, int n);          /* FUN_8002c958 */
extern int      Damage_AccumulateOrFire(uint32_t *self, uint16_t a); /* FUN_80020778 */
extern void     FX_RingFlash(void *self, int dy, void *params, int n); /* FUN_800176f8 */
extern uint32_t _DAT_80065ad4;   /* live target vehicle */
extern uint8_t  DAT_80100044, DAT_80100050;

uint32_t AG_TrackerDish(uint32_t *self, int mode, int *arg)
{
    uint32_t modelH = self[0xe];
    int      sub    = *(int *)(modelH + 0x38);

    if (mode == 1) goto ground_align;
    if (mode == 0 || mode == 8) {
        if (mode == 0) {
            /* Advance reload-cycle: pos += rate; if not yet firing, */
            /* tick rate by +0x10e per frame.                        */
            int pos  = *(int *)(sub + 0x28);
            int rate = *(int *)(sub + 0x48);
            pos += rate;
            *(int *)(sub + 0x28) = pos;
            if (*(char *)(sub + 8) == 0) {
                *(int *)(sub + 0x48) = rate + 0x10e;
                if (*(int *)(sub + 0x50) < pos) {
                    /* Reached fire-position: shoot. */
                    *(int *)(sub + 0x28) = *(int *)(sub + 0x50);
                    int8_t  curStage = *(char *)(modelH + 8);
                    uint16_t hp      = *(uint16_t *)((char *)self + 0xe);
                    uint8_t  stageMax= (uint8_t)self[2];
                    uint8_t  nFrames = (uint8_t)self[3];
                    *(char *)(sub + 8) = 1;
                    *(int *)(sub + 0x48) =
                        (int)(((uint32_t)hp * (curStage - 1) + nFrames) * -0x17d7)
                        / (int)((uint32_t)hp * (stageMax - 1));
                    int spawnAddr   = Object_LocalToWorldPos();
                    uint32_t projH  = Pool_AllocProjectile();
                    arg = NULL;
                    Pool_LaunchProjectile(projH,
                                          *(uint32_t *)(self[0x16] + 8),
                                          0, spawnAddr + 0x14);
                    SubModel_AddrAt(spawnAddr + 0x14);
                }
            }
            int pos2 = *(int *)(sub + 0x28);
            if (pos2 < *(int *)(sub + 0x4c)) {
                *(int *)(sub + 0x28) = *(int *)(sub + 0x4c);
                *(int *)(sub + 0x48) = 0;
                *(char *)(sub + 8)   = 0;
            }
            if ((int16_t)self[3] == 0) return 0;
            if ((*self & 0x4000) == 0)  return 0;
            int aimYaw = ratan2(*(int *)(_DAT_80065ad4 + 0x48) - (int)self[0x12],
                                *(int *)(_DAT_80065ad4 + 0x50) - (int)self[0x14]);
            uint16_t *yawCell = (uint16_t *)(modelH + 0x42);
            int delta = aimYaw - (int)*(uint16_t *)((char *)self + 0x42)
                              - (*yawCell - 0x800);
            *yawCell += (int16_t)(int8_t)((delta * 0x100000) >> 24);
            Object_RefitAABB(self);
        }
        /* fall-through into damage handling for mode==0 wasn't taken;
         * mode==8 lands here directly.                              */
    }
    if (mode == 3 || mode == 8 || mode == 0) {
        if (mode == 3 || mode == 0) {
            int    imp = *arg;
            if (*(uint8_t *)(imp + 4) == 2) {
                if (((int *)arg)[3] != *(int *)(self[0xe] + 0x38)) return 0;
                int wp = Object_LocalToWorldPos();
                FX_RingFlash((void *)(intptr_t)imp, &DAT_80100044, NULL, wp + 0x14);
                if (Damage_LookupImpactor((void *)(intptr_t)imp, -12,
                                          &DAT_80100050, 1) != 0) return 0;
            }
            arg = (int *)(uintptr_t)*(uint16_t *)(imp + 0xc);
        }
        int16_t newHp = (int16_t)((uint16_t)self[3] - (int)(intptr_t)arg);
        if ((int)(uint16_t)self[3] < (int)(intptr_t)arg) {
            /* Stage transition or final break. */
            int subAddr = *(int *)(self[0xe] + 0x38);
            if (*(char *)(self[0xe] + 8) == 0) return 0;
            uint8_t  state = *(uint8_t  *)(subAddr + 8);
            uint32_t pos   = *(uint32_t *)(subAddr + 0x28);
            uint32_t rate  = *(uint32_t *)(subAddr + 0x48);
            uint32_t firePt= *(uint32_t *)(subAddr + 0x50);
            SubModel_Detach(self[0xe]);
            int next = *(int *)(self[0xe] + 0x38);
            int8_t   stage = *(char *)(self[0xe] + 8);
            *(char *)(self[0xe] + 8) = (char)(stage - 1);
            if (next != 0 && stage != 1) {
                *(uint8_t  *)(next + 8)    = state;
                *(uint32_t *)(next + 0x28) = pos;
                *(uint32_t *)(next + 0x48) = rate;
                *(uint32_t *)(next + 0x50) = firePt;
                *(uint16_t *)(self + 3)    = *(uint16_t *)((char *)self + 0xe);
            }
            newHp = (int16_t)Damage_AccumulateOrFire(self, 0);
            *(uint16_t *)(self + 3) = 0;
        }
        *(int16_t *)(self + 3) = newHp;
    }
ground_align: {
        uint32_t mh    = self[0xe];
        int      sub2  = *(int *)(mh + 0x38);
        int      wp    = Object_LocalToWorldPos();
        int      grd   = Terrain_HeightAt(*(int32_t *)(wp + 0x14),
                                          *(int32_t *)(wp + 0x1c));
        *(int *)(sub2 + 0x50) = (grd - *(int *)(wp + 0x18)) + *(int *)(sub2 + 0x4c);
        char     stage = SubModel_PickInitialStage(mh);
        *(char *)(mh + 8)        = stage;
        *(char *)((char *)self + 8) = stage + 1;
        *self = (*self & 0xfffffffb) | 0x80;
    }
    return 0;
}
