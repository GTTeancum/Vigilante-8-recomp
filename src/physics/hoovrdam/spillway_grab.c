/* spillway_grab.c -- Hoover Dam spillway / floodgate grab.
 *
 * Source: HOOVRDAM.DLL  FUN_80101734.
 *
 * Companion to siren_strobe.c, this is the spillway grab logic that
 * catches vehicles attempting to traverse the dam crest. When a
 * vehicle enters mode==3 (impact) with the "armed" flag self[0]
 * 0x10000 set, the spillway grabs it:
 *   - alloc/reuse FX slot at vehicle+0xd3
 *   - bind grab FX + snare projectile
 *   - rebind vehicle tick to FUN_801006cc, state=8
 *   - mark `*v |= 0x3000020`, clear ~0x2
 *   - pick path grab-point index (rand, retry until != self+6 kind)
 *   - heap-alloc 0x40-byte carrier descriptor with preset coordinates
 *     differentiated by spillway side (Z < 0x5210000 -> +0x50000,
 *     else -0x50000).
 *
 * Pad-trigger path (mode 0/8 + DAT_1f80000c controller record kind==2):
 *   - lever push: re-arms 0x10000 only if controller stick magnitude
 *     beyond 0xf580 and within bounding box (|+0x12000 - posY| < 0x10000
 *     and |posZ delta| < 0xa000). On match, schedule damage.
 *
 * mode dispatch:
 *   2 -- silent retire
 *   3, 8 (default) -- main grab logic
 *
 * MED.
 */
#include <stdint.h>

extern uint8_t  Pool_AllocSFX(void);                                 /* FUN_8004410c */
extern void Pool_BindFXOnObject(uint32_t h, uint32_t bin, int slot, int aux); /* FUN_800443c8 */
extern void Pool_BindSnareToObject(uint32_t h, uint32_t bin, int slot, uint32_t *xyz); /* FUN_800447e8 */
extern void SFX_Update(int h, int posVoxel);
extern int  Rand255(void);
extern void Damage_Apply_AgainstSelf(void *self, void *param);
extern void *Heap_AllocOrRetry(uint32_t n);
extern int  HD_PathLookup(int8_t pathIdx);                           /* func_0x80021888 */
extern void Spawner_Promote(uint32_t target);                        /* func_0x8003dbb0 */
extern void Damage_FromImpulse(uint32_t *self, uint32_t *imp);       /* func_0x8002239c */
extern void Damage_StandardVehicle(void *self, uint32_t *imp);       /* func_0x80022320 */
extern uint32_t _DAT_1f80000c;
extern uint32_t FUN_801006cc;

uint32_t HD_SpillwayGrab(uint32_t *self, uint32_t mode, uint32_t *arg)
{
    if (mode == 2) goto disarm;
    if (mode == 3 || mode == 8 || mode > 3) {
        uint32_t *vic = (uint32_t *)(uintptr_t)*arg;
        if ((char)vic[1] == 2 && (*self & 0x10000)) {
            if (*vic & 0x2000000) return 0;
            char sfx = *((char *)vic + 0xd3);
            if (sfx == 0) {
                sfx = (char)Pool_AllocSFX();
                *((char *)vic + 0xd3) = sfx;
            }
            Pool_BindFXOnObject(sfx, *(uint32_t *)(self[0x16] + 8), 3, 0);
            uint8_t h = Pool_AllocSFX();
            Pool_BindSnareToObject(h, *(uint32_t *)(self[0x16] + 8), 4, vic + 9);
            SFX_Update((int)*((char *)vic + 5), 0);
            vic[0x19] = (uint32_t)(uintptr_t)&FUN_801006cc;
            *(uint8_t *)(vic + 2) = 8;
            *vic = (*vic & ~2u) | 0x3000020u;
            uint8_t pick;
            do {
                int r = Rand255();
                pick  = (uint8_t)(((r << 2) >> 15) + 0x3c);
                *((char *)vic + 0xd2) = pick;
            } while (pick == (uint8_t)*(int16_t *)((char *)self + 6));
            Damage_Apply_AgainstSelf(vic, (void *)(intptr_t)0x3c);
            uint32_t *bin = (uint32_t *)(uintptr_t)vic[0x38];
            if (bin == NULL) return 0;
            uint32_t *desc = (uint32_t *)Heap_AllocOrRetry(0x40);
            int      path = HD_PathLookup(*((char *)vic + 0xd2));
            desc[0] = bin[0x12]; desc[1] = bin[0x13]; desc[2] = bin[0x14];
            desc[3] = 0x78;
            desc[4] = 0x39f0000;
            desc[5] = 0x25f800;
            desc[6] = 0x5210000;
            desc[7] = 0xf0;
            desc[8] = *(uint32_t *)(path + 0x48);
            desc[9] = *(int *)(path + 0x4c) - 0x12000;
            int dz  = *(int *)(path + 0x50) < 0x5210000 ? 0x50000 : -0x50000;
            desc[10] = *(int *)(path + 0x50) + dz;
            desc[11] = 0;
            Spawner_Promote((uint32_t)(uintptr_t)bin);
        }
        Damage_FromImpulse((uint32_t *)(uintptr_t)*arg, arg);
        /* fall through */
    }
    if (*(char *)(_DAT_1f80000c + 4) == 2) {
        int stick = *(int *)(_DAT_1f80000c + 0x80);
        uint32_t side = 0x50000;
        if (*(int16_t *)((char *)self + 0x42) != 0) {
            stick = -stick;
            side = (uint32_t)(0x5f580 < stick);
            if (side == 0) goto disarm_then_pass;
        }
        if ((int)(side | 0xf580u) < stick) {
            int dy = (*(int *)(_DAT_1f80000c + 0x28) + 0x12000) - (int)self[0x13];
            if (dy < 0) dy = -dy;
            if (dy < 0x10000) {
                int dz = *(int *)(_DAT_1f80000c + 0x2c) - (int)self[0x14];
                if (dz < 0) dz = -dz;
                if (dz < 0xa000) {
                    *self |= 0x10000u;
                    Damage_Apply_AgainstSelf(self, NULL);
                }
            }
        }
    }
disarm_then_pass:
    *self &= ~0x10000u;
    Damage_StandardVehicle(self, arg);
disarm:
    *self &= ~0x10000u;
    return 0;
}
