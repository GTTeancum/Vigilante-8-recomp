/* siren_strobe.c -- Hoover Dam alarm-siren strobe + grab logic.
 *
 * Source: HOOVRDAM.DLL  FUN_80100c30.
 *
 * Strobe-light controller that ALSO grabs vehicles that drive too
 * close (similar to SF conveyor grab). Owns a per-frame timeline
 * buffer of strobe-light cells at self+0x22 (8-cell ring) keyed by
 * self+0x21 (write index) and self+0x20 (countdown).
 *
 * mode dispatch (large fall-through):
 *   0  -- per-tick: drain countdown, on underflow copy next ring slot
 *         into DAT_8008f660 (active light cell). Reseed strobe rate
 *         via Rand255.
 *   1  -- post-spawn: pick siren path waypoint (FUN_8001ffd4@bank
 *         0x80065a50) + bind via func_0x8003d080. _DAT_80065a10 =
 *         (path attached).
 *   2  -- post-init: schedule first 240-frame fire.
 *   0xb -- bind FX channel (Pool_BindFXOnObject).
 *   0x11-- begin per-frame damage handler.
 *   9   -- main "grab vehicle" event:
 *          * if vehicle is too close (Z+0xfb140000 > 0x540000), kick
 *            into capture state: set tick callback FUN_801006cc,
 *            stop FX, set fall-velocity 0x1c980, schedule damage and
 *            spawn a 0x40-byte carrier descriptor on the heap with
 *            preset lift coords (0x39f0000, 0x5470000, 0x20f800, 0x168
 *            + grab-point offsets at +0x12000/-0x50000).
 *          * every 16 frames emit a strobe particle (0x24a/0x24b/0x249
 *            depending on flag 0x80000) at the grabbed object's pos,
 *            yaw = Math_Atan2_Pos(target).
 *   7  -- post-grab maintenance.
 *   4  -- free ring buffer, return.
 *
 * MED. The chained fall-through is preserved verbatim from the
 * original switch-with-no-breaks idiom.
 */
#include <stdint.h>

extern int  Rand255(void);                                          /* FUN_80017160 */
extern uint32_t Path_Pick(uint32_t bank, int seed);                 /* FUN_8001ffd4 */
extern int  Path_AttachToObj(uint32_t flag, uint32_t path);          /* func_0x8003d080 */
extern void Damage_Apply_AgainstSelf(void *self, void *param);       /* FUN_80020890 */
extern void HD_QueueDamage(void);                                    /* func_0x80023d00 */
extern uint8_t  Pool_AllocSFX(void);                                 /* FUN_8004410c */
extern void Pool_BindFXOnObject(uint32_t h, uint32_t bin, int slot, int aux); /* FUN_800443c8 */
extern void Damage_StandardVehicleAlt(void *self, uint32_t *imp);    /* func_0x80022120 */
extern uint32_t *Object_FetchPrev(void *self);                       /* FUN_8001d5e0 */
extern uint32_t *Object_FetchPrevHead(void *self);                   /* FUN_8001d5a0 */
extern void Damage_VsImpactorAlt(uint32_t *imp, int dmg, void *p, int n); /* FUN_8002c958 */
extern void SFX_Update(int h, int posVoxel);                         /* FUN_80044574 */
extern void *Heap_AllocOrRetry(uint32_t n);                          /* FUN_800116f4 */
extern int  HD_PathLookup(int8_t pathIdx);                           /* func_0x80021888 */
extern void Spawner_Promote(uint32_t target);                        /* func_0x8003dbb0 */
extern int  Math_Atan2_Pos(int xy);                                  /* FUN_80016c88 */
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag); /* FUN_8001ac44 */
extern void Object_Suspend(void);                                    /* FUN_8002036c */
extern void Object_RetireFX(int port);                               /* FUN_8001d470 */
extern void Heap_Free(uint32_t p);                                   /* FUN_80045088 */

extern uint8_t  DAT_8008f660, DAT_80100098;
extern uint32_t DAT_80101b88;
extern uint32_t _DAT_800659fc, _DAT_800737e8, _DAT_80065a10;
extern uint32_t FUN_801006cc, FUN_8003e80c;

uint32_t HD_SirenStrobe(uint32_t *self, uint32_t mode, uint32_t *arg)
{
    switch (mode) {
    case 0:
        if (arg != NULL) {
            uint32_t v = self[0x20];
            self[0x20] = v - (uintptr_t)arg;
            if ((int)(v - (uintptr_t)arg) < 0) {
                do {
                    uint32_t idx = self[0x21];
                    uint32_t *src = (uint32_t *)((idx & 7) * 0x80 + self[0x22]);
                    self[0x20] += 6;
                    uint32_t *dst = (uint32_t *)&DAT_8008f660;
                    for (int i = 0; i < 8; i++) {
                        dst[0] = src[0]; dst[1] = src[1];
                        dst[2] = src[2]; dst[3] = src[3];
                        src += 4; dst += 4;
                    }
                    self[0x21] = idx + 1;
                } while ((int)self[0x20] < 0);
            }
        }
        DAT_80101b88 = (uint32_t)Rand255();
        /* fall through */
    case 1: {
        *self = 0x80;
        self[0x22] = 0x80;
        uint32_t path = Path_Pick(0x80065a50, 0x100);
        _DAT_80065a10 = (Path_AttachToObj(0x7f000000u, path) != 0);
    }
    /* fall through */
    case 2:
        HD_QueueDamage();
        Damage_Apply_AgainstSelf(self, (void *)(intptr_t)0xf0);
        self = (uint32_t *)1;
        /* fall through */
    case 0xb: {
        uint8_t h = Pool_AllocSFX();
        Pool_BindFXOnObject(h, *(uint32_t *)(*(int *)(_DAT_800659fc + 0x58) + 8), 1, 0);
        self = (uint32_t *)(uintptr_t)h;
    }
    /* fall through */
    case 0x11:
        Damage_StandardVehicleAlt(self, arg);
        /* fall through */
    case 9: {
        int8_t cmp = 8;
        if (arg && 0x540000 < arg[2] + 0xfb140000u) {
            uint32_t *vic = Object_FetchPrev(self);
            if (*vic & 0x2000000) return 0;
            Damage_VsImpactorAlt(vic, -150, &DAT_80100098, 0);
            uint32_t bin = *(uint32_t *)(_DAT_800659fc + 0x58);
            for (uintptr_t c = vic[0xe]; c != 0; c = *(uint32_t *)(c + 0x34))
                *(uint16_t *)(*(int *)(c + 0x30) + 0x28) = 0x40;
            if (vic[0x1a] != 0) *(uint16_t *)(vic[0x1a] + 0x28) = 0x40;
            char sfx = *((char *)vic + 0xd3);
            if (sfx == 0) {
                sfx = (char)Pool_AllocSFX();
                *((char *)vic + 0xd3) = sfx;
            }
            Pool_BindFXOnObject(sfx, *(uint32_t *)(_DAT_800737e8 + 8), 3, 0);
            SFX_Update((int)*((char *)vic + 5), 0);
            vic[0x19] = (uint32_t)(uintptr_t)&FUN_801006cc;
            *(uint8_t *)(vic + 2) = 0;
            *vic = (*vic & ~2u) | 0x3000020u;
            int r = Rand255();
            *((char *)vic + 0xd2) = (char)((r << 2) >> 15) + 0x3c;
            vic[0x20] = 0;
            vic[0x21] = 0x1c980;
            vic[0x22] = 0;
            Damage_Apply_AgainstSelf(vic, NULL);
            uint32_t bin2 = vic[0x38];
            if (bin2 == 0) return 0;
            uint32_t *desc = (uint32_t *)Heap_AllocOrRetry(0x40);
            int       grab = HD_PathLookup(*((char *)vic + 0xd2));
            desc[0] = *(uint32_t *)(bin2 + 0x48);
            desc[1] = *(uint32_t *)(bin2 + 0x4c);
            desc[2] = *(uint32_t *)(bin2 + 0x50);
            desc[3] = 0x78;
            desc[4] = 0x39f0000;
            desc[5] = 0x20f800;
            desc[6] = 0x5470000;
            desc[7] = 0x168;
            desc[8] = *(uint32_t *)(grab + 0x48);
            desc[9] = *(int *)(grab + 0x4c) - 0x12000;
            desc[10]= *(int *)(grab + 0x50) + 0x50000;
            desc[11]= 0;
            Spawner_Promote(bin2);
            self = desc;
            cmp = 0;
        }
        if ((char)self[1] != cmp || (DAT_80101b88 = DAT_80101b88 + 1, (DAT_80101b88 & 0xf) != 0))
            return 0;
        uint32_t *head = Object_FetchPrevHead(self);
        if (*(int *)(head + 0x8c) < 0xee6) {
            self = (uint32_t *)Object_SpawnFromBank(*(uint32_t *)(_DAT_800659fc + 0x58),
                                                   0x24b, 0x80, 8);
        }
        uint32_t kind = (*self & 0x80000) ? 0x249u : 0x24au;
        self = (uint32_t *)Object_SpawnFromBank(*(uint32_t *)(_DAT_800659fc + 0x58),
                                                kind, 0x80, 8);
        *(uint16_t *)((char *)self + 0x42) = (uint16_t)Math_Atan2_Pos((int)(uintptr_t)(head + 0x10));
        *(uint8_t  *)(self + 1)  = 1;
        *self = 0x24u;
        self[0x12] = arg[0]; self[0x13] = arg[1]; self[0x14] = arg[2];
        self[0x19] = (uint32_t)(uintptr_t)&FUN_8003e80c;
        Object_Suspend();
    }
    /* fall through */
    case 7:
        Object_RetireFX(0x8c);
        /* fall through */
    case 4:
        Heap_Free(self[0x22]);
        return 0;
    default:
        return 0;
    }
    return 0;
}
