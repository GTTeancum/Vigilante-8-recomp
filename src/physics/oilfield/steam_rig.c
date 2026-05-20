/* steam_rig.c -- Oil Fields steam-vent rig periodic emit.
 *
 * Source: OILFIELD.DLL  FUN_80100668.
 *
 * Sibling to rig_emit.c (FUN_801002b0) but uses a different child-
 * particle layout. Per-tick:
 *   - countdown at self+0x80 decrements; on -1 spawn a steam puff:
 *       * pool-alloc puff via func_0x80040378(self+0x98 bank)
 *       * orientation taken from a 0x1000-entry sin/cos table at
 *         -0x7ff9f84c.. (4.12-fixed unit circle) indexed by 12 bits
 *         from the RNG
 *       * velocity X/Z scaled by self+0x84 (per-rig strength)
 *       * Y velocity = self+0x88 + (rand() * self+0x88 >> 15) jitter
 *       * tick callback puff[0x19] = 0x80040470 (puff-tick)
 *       * play world FX via FUN_8001d4f0
 *     reload countdown from self+0x82
 *   - Each child @+0x38 -> +0x34 chain: pos += vel (offsets +0x24..2c
 *     and +0x88..90)
 *   - Object_RetireDeferred(self) if mode==0
 *
 * mode dispatch:
 *   0  -- per-tick (above)
 *   2  -- damage-applied (sets cooldown sentinel 0xffff)
 *   3  -- impactor collision; if impactor.kind == 2 ring-flash and
 *         apply damage at &DAT_80100090
 *
 * MED.
 */
#include <stdint.h>

extern void *Generic_PoolAlloc(uint32_t bank);                   /* func_0x80040378 */
extern int   Rand255(void);                                      /* FUN_80017160 */
extern void  Object_BumpSubstate_Or_FX(int obj);                 /* FUN_8001d4f0 */
extern void  Object_RetireDeferred(int self);                    /* FUN_800205f8 */
extern void  FX_RingFlash(int imp, void *p, int spawnXyz);       /* FUN_800176f8 */
extern void  Damage_TouchImpactor(int imp);                      /* FUN_8002c3ac */
extern uint8_t DAT_80100090;

uint32_t OF_SteamRig(int self, uint32_t mode, int *arg)
{
    uint32_t hit = 2;
    if (mode == 2) goto seal;
    if (mode == 0 || mode == 3) {
        if (mode == 0) {
            int16_t cd = *(int16_t *)(self + 0x80) - 1;
            *(int16_t *)(self + 0x80) = cd;
            if (cd == -1) {
                uint32_t *puff = (uint32_t *)Generic_PoolAlloc(*(uint32_t *)(self + 0x98));
                if (puff != NULL) {
                    int r = (Rand255() & 0xfff) * 4;
                    *puff |= 0x410u;
                    int sx = *(int *)(self + 0x84) * (int)*(int16_t *)(r - 0x7ff9f84c);
                    if (sx < 0) sx += 0xfff;
                    puff[0x22] = sx >> 12;
                    int sz = *(int *)(self + 0x84) * (int)*(int16_t *)(r - 0x7ff9f84a);
                    if (sz < 0) sz += 0xfff;
                    puff[0x24] = sz >> 12;
                    int r2 = Rand255();
                    int v  = *(int *)(self + 0x88);
                    puff[9] = puff[10] = puff[0xb] = 0;
                    puff[0x19] = 0x80040470u;
                    puff[0x23] = v + ((r2 * v) >> 15);
                    Object_BumpSubstate_Or_FX(self);
                }
                *(int16_t *)(self + 0x80) = *(int16_t *)(self + 0x82);
            }
            for (int c = *(int *)(self + 0x38); c != 0; c = *(int *)(c + 0x34)) {
                *(int *)(c + 0x24) += *(int *)(c + 0x88);
                *(int *)(c + 0x28) += *(int *)(c + 0x8c);
                *(int *)(c + 0x2c) += *(int *)(c + 0x90);
            }
            Object_RetireDeferred(self);
            hit = 0;
        }
        int imp = *arg;
        if (*(uint8_t *)(imp + 4) != hit) return 0;
        FX_RingFlash(imp, &DAT_80100090, self + 0x48);
        Damage_TouchImpactor(imp);
    }
seal:
    *(uint16_t *)(self + 0x80) = 0xffff;
    return 0;
}
