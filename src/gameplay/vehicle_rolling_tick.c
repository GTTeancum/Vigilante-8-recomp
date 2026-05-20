/* vehicle_rolling_tick.c -- the per-frame physics tick for vehicles
 * in "rolling" state (obj+0xd0 == 12).
 *
 * Source: SLUS_005.10  gap_80030f34  (860 B, 215 MIPS instructions).
 *
 * Originally a Ghidra-gap function (the auto-analyser missed the
 * function start because it sits in the gap between Object_GeneralTick
 * at 0x80030c08 and FUN_80031300).  Discovered by tracing callers of
 * Object_GeneralTick + observing that Vehicle_StateTransition installs
 * the literal address 0x80030f34 into obj+0x64 when obj+0xd0 == 12.
 *
 * Raw MIPS extracted to analysis/SLUS_005.10/mips/80030f34.s.
 *
 * Signature: standard per-object callback (self, mode, arg2, arg3).
 *
 * Modes:
 *   0  per-frame tick.  Branches on obj+6 (i16 status):
 *        status >= 0 : "normal" -- skip the damaged-state machinery
 *        status <  0 : "damaged" -- material lookup + audio voice +
 *                       FUN_80030a88 + FUN_8002d494 + counter +
 *                       per-frame SPU pan / pitch update.
 *      BOTH paths converge on Object_GeneralTick(self).  Always
 *      integrates regardless of damage state.
 *
 *   1  init: sets obj+0xd0 = 12 (state "rolling"), or flags 0x4088,
 *      calls FUN_8003e76c + FUN_8002cce8 (state setup), then clears
 *      the high half of obj[0].
 *
 *   3  forward to FUN_8002d82c (heavy event handler).
 *
 *   7  SPAWN sub-projectile (matches the FUN_8002e630 constructor's
 *      pattern -- allocates 0x124 bytes via FUN_8001ac44, installs
 *      this very function as the new sub-projectile's tick callback,
 *      sets dragMass = 0x5000 and other template-driven init).
 *
 *   else  return 0.
 *
 * HIGH confidence: line-by-line MIPS port.
 */
#include <stdint.h>

extern void     Object_GeneralTick(uint32_t *obj);         /* FUN_80030c08 */
extern uint8_t *Terrain_MaterialAt(uint32_t x, uint32_t z); /* FUN_800255f4 */
extern void     FUN_80030a88(uint32_t *obj, void *lut);
extern void     FUN_8002d494(uint32_t *obj, void *lut);
extern void     FUN_8002d44c(uint32_t *obj);
extern void     FUN_8002d82c(uint32_t *obj, int arg);
extern uint32_t FUN_800446dc(const int32_t *xyz);           /* SfxCurve_2P_Stereo */
extern int      FUN_800129e8(int mode, const char *msg);
extern void     FUN_8003e76c(uint32_t *obj);
extern void     FUN_8002cce8(uint32_t *obj, uint8_t arg);
extern void    *FUN_8001ac44(int *bank, uint16_t kind, uint32_t size, uint32_t flags);
extern int      FUN_8001affc(int *bank, uint16_t kind, uint16_t key);
extern void     FUN_8001b2fc(uint32_t *chassis, int joint, void *wheel);
extern int      FUN_8001b270(uint32_t *obj);
extern void    *FUN_8001d470(uint32_t size);
extern void     FUN_8001d4f0(uint32_t *chassis, void *extra);
extern void     FUN_8001d708(void *obj);
extern int      sprintf(char *buf, const char *fmt, ...);

extern void   *iRam00000010;                /* gp[16] display flag */
extern const char DAT_80055738[];            /* sprintf format */
extern const uint8_t DAT_80065c28[];         /* per-status lookup table */
extern int *  DAT_8005ec10;                  /* spawn-mode template @0x80060000-0x13f0 */

/* The original MIPS writes to 0x1f801bf0 -- the PSX SPU voice
 * register block (24 voices * 16 bytes each + a small base offset).
 * On host that address is unmapped, so we redirect to a scratch
 * buffer.  The audio engine is out-of-scope per CLAUDE.md; these
 * writes become deferred-state for a future host audio layer to
 * pick up (or be ignored). */
static uint8_t g_spu_voice_scratch[24 * 16 + 16];
static uint32_t * const SPU_VOICE_BASE =
    (uint32_t *)((uintptr_t)g_spu_voice_scratch);

/* Forward decl. */
int Vehicle_RollingTick(uint32_t *self, int mode, int arg2, int arg3);

/* HIGH: rolling per-frame tick.  Always integrates via
 * Object_GeneralTick; in damaged state also does material/audio/
 * counter work. */
int Vehicle_RollingTick(uint32_t *self, int mode, int arg2, int arg3)
{
    (void)arg3;
    uint8_t *s = (uint8_t *)self;

    /* ---- Mode dispatch ---- */
    if (mode == 1) {
        /* INIT: enter "rolling" state. */
        uint32_t f = self[0];
        *(uint8_t *)(s + 0xd0) = 12;       /* state = rolling */
        f |= 0x4088u;
        self[0] = f;
        FUN_8003e76c(self);
        FUN_8002cce8(self, (uint8_t)(*(uint8_t *)(s + 3) | 1u));
        self[0] = (uint32_t)(uint16_t)self[0];
        return 0;
    }
    if (mode == 3) {
        FUN_8002d82c(self, arg2);
        return 0;
    }
    if (mode == 7) {
        /* SPAWN sub-projectile.  Matches FUN_8002e630 (Vehicle_Construct)
         * pattern: alloc 0x124 bytes, install THIS function as the new
         * sub-projectile's tick callback, copy a slice of template data. */
        int *bank = (int *)self;       /* a0 (which the caller passed as "obj") */
        uint16_t kind = (uint16_t)(arg2 & 0xffff);
        int *s3_template = (int *)((uintptr_t)0x80060000 - 0x13f0);  /* &DAT_8005ec10 */

        uint32_t *p = (uint32_t *)FUN_8001ac44(bank, kind, 292, 0);
        *(uint16_t *)((uint8_t *)p + 6) = 0;
        *(uint8_t  *)((uint8_t *)p + 4) = 2;
        *(uint16_t *)((uint8_t *)p + 0xc) =
            *(uint16_t *)((uint8_t *)s3_template + 0x1c);
        int br = FUN_8001b270((uint32_t *)bank);
        *(uint16_t *)((uint8_t *)p + 0xe) = (uint16_t)br;
        *(uint32_t *)((uint8_t *)p + 0x7c) = (uint32_t)br;

        /* Install this very function as the new object's tick callback. */
        *(uint32_t *)((uint8_t *)p + 0x64) =
            (uint32_t)(uintptr_t)&Vehicle_RollingTick;

        void *extra = FUN_8001d470(128);
        *(void **)((uint8_t *)p + 0xf8) = extra;
        int joint = FUN_8001affc(bank, kind, 0x8100);
        if (joint != 0) {
            FUN_8001b2fc(p, joint, extra);
        } else {
            *(int32_t *)((uintptr_t)extra + 0x4c) = -21845;   /* 0xffffaaab */
            FUN_8001d708(extra);
            FUN_8001d4f0(p, extra);
        }

        /* Unaligned 4-byte slice from template+0x14 to obj+0x9c (writes
         * inertia + related fields). */
        uint32_t inertia_blob =
            (uint32_t)*(uint8_t *)((uintptr_t)s3_template + 0x14)        |
            (uint32_t)*(uint8_t *)((uintptr_t)s3_template + 0x15) <<  8  |
            (uint32_t)*(uint8_t *)((uintptr_t)s3_template + 0x16) << 16  |
            (uint32_t)*(uint8_t *)((uintptr_t)s3_template + 0x17) << 24;
        *(uint8_t *)((uintptr_t)p + 0x9c) = (uint8_t)(inertia_blob >>  0);
        *(uint8_t *)((uintptr_t)p + 0x9d) = (uint8_t)(inertia_blob >>  8);
        *(uint8_t *)((uintptr_t)p + 0x9e) = (uint8_t)(inertia_blob >> 16);
        *(uint8_t *)((uintptr_t)p + 0x9f) = (uint8_t)(inertia_blob >> 24);

        *(int16_t  *)((uintptr_t)p + 0xa0) = *(int16_t  *)((uintptr_t)s3_template + 0x18);
        *(uint16_t *)((uintptr_t)p + 0xa2) = *(uint16_t *)((uintptr_t)s3_template + 0x1a);

        /* dragMass = 20480 = 0x5000.  This is the engine's literal
         * value for sub-projectile spawned via this path. */
        *(int32_t *)((uintptr_t)p + 0xd8) = 20480;

        return 0;
    }
    if (mode != 0) return 0;

    /* ==================== Mode 0: per-frame tick ==================== */
    int16_t status = *(int16_t *)(s + 6);
    if (status < 0) {
        /* ---- Damaged-state extra work ---- */
        uint8_t *material = Terrain_MaterialAt(
            (uint32_t) *(int32_t *)(s + 0x24),
            (uint32_t) *(int32_t *)(s + 0x2c));
        int16_t reread = *(int16_t *)(s + 6);
        int idx = (~(int32_t)reread) * 3;
        const uint8_t *lut = &DAT_80065c28[idx * 8];

        /* Material attr 7 -> bounce: invert all 3 vel components. */
        if (*(int16_t *)(material + 0x16) == 7) {
            *(int32_t *)(s + 0x80) = -*(int32_t *)(s + 0x80);
            *(int32_t *)(s + 0x84) = -*(int32_t *)(s + 0x84);
            *(int32_t *)(s + 0x88) = -*(int32_t *)(s + 0x88);
        }

        FUN_80030a88(self, (void *)(uintptr_t)lut);
        FUN_8002d494(self, (void *)(uintptr_t)lut);

        /* Secondary counter at obj+0xb6 (= s + 0xa4 + 0x12). */
        uint8_t *s2 = s + 0xa4;
        uint8_t cnt = *(uint8_t *)(s2 + 0x12);
        if (cnt != 0) {
            uint8_t new = cnt - 1;
            *(uint8_t *)(s2 + 0x12) = new;
            if (new == 0) {
                uint8_t b9 = *(uint8_t *)(s2 + 0x15);
                if (b9 != 0) {
                    int16_t st2 = *(int16_t *)(s + 6);
                    if (st2 < 0) {
                        char msgbuf[32];
                        sprintf(msgbuf, (const char *)DAT_80055738,
                                (unsigned)*(uint8_t *)(s2 + 0x15));
                        int dmode = (iRam00000010 != 0) ? -st2 : 0;
                        FUN_800129e8(dmode, msgbuf);
                        FUN_8002d44c(self);
                    }
                    uint8_t prev_b9 = *(uint8_t *)(s2 + 0x15);
                    uint8_t prev_ba = *(uint8_t *)(s2 + 0x16);
                    *(uint8_t *)(s2 + 0x12) = 30;
                    *(uint8_t *)(s2 + 0x15) = 0;
                    *(uint8_t *)(s2 + 0x16) = prev_ba + prev_b9;
                }
            }
        }

        /* If catchupFlag set, refresh this object's SPU voice volume +
         * pitch from current position and velocity. */
        if (arg2 != 0) {
            int8_t voice = *(int8_t *)(s + 5);
            uint32_t pan = FUN_800446dc((const int32_t *)(s + 0x24));
            uint32_t *vreg =
                (uint32_t *)((uintptr_t)SPU_VOICE_BASE
                             + ((uintptr_t)(uint8_t)voice << 4));
            vreg[0] = pan;
            /* Pitch from velY (obj+0x8c), RTZ-shifted /8, clamped to
             * [768, 1536].  Default pitch = 1536 when out of range. */
            int32_t vy = *(int32_t *)(s + 0x8c);
            int32_t pitch = (vy < 0) ? ((vy + 7) >> 3) : (vy >> 3);
            int32_t pitch_clamped;
            if (pitch < 768) {
                pitch_clamped = 768;
            } else if (pitch > 1536) {
                pitch_clamped = 1536;
            } else {
                pitch_clamped = pitch;
            }
            *(int16_t *)((uintptr_t)vreg + 4) = (int16_t)pitch_clamped;
        }
    }
    /* ---- Both paths fall through to here ---- */

    /* Catchup-flag bit-remap: clear 0x18000, set 0x8000 if 0x10000 set. */
    if (arg2 != 0) {
        uint32_t f0 = self[0];
        uint32_t f1 = f0 & 0xfffe7fffu;
        if ((f0 & 0x10000u) != 0) f1 |= 0x8000u;
        self[0] = f1;
    }

    /* The actual physics integration. */
    Object_GeneralTick(self);
    return 0;
}

/* Legacy gap alias. */
int gap_80030f34(uint32_t *self, int mode, int arg2, int arg3)
{
    return Vehicle_RollingTick(self, mode, arg2, arg3);
}
