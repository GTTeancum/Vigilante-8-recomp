/* vehicle_tick.c -- the player/AI vehicle tick callback.
 *
 * Source: SLUS_005.10  LAB_8002e2bc  (0x374 B, 221 MIPS instructions).
 *
 * Originally a Ghidra-gap function (the auto-analyser missed the
 * function start).  Discovered via FUN_8002e630's line 34:
 *     puVar6[0x19] = (uint)&LAB_8002e2bc;
 * meaning every Vehicle the constructor builds gets this function
 * installed at obj+0x64 as its per-tick callback.
 *
 * Cleaned from raw MIPS at analysis/SLUS_005.10/mips/8002e2bc.s.
 *
 * Signature (per MIPS): callback receives (a0=self, a1=mode, a2=arg2,
 * a3=arg3).  Modes:
 *
 *   0  -- normal per-frame tick.  Two branches:
 *           obj+6 (i16 status) < 0  -> "damaged" path: material-attr
 *               audio bookkeeping, then FUN_8002efe0 + FUN_8002d494
 *               (the heavy per-tick physics), optionally FUN_8002d054
 *               if `arg2` (catchupFlag) is non-zero.  Falls into the
 *               common tail.
 *           obj+6 >= 0  -> "normal" path: tick down the +0xb0 sound-
 *               event counter; on expiry trigger FUN_800447e8 with
 *               the vehicle's pos.  Falls into the common tail.
 *
 *         Common tail: tick down the secondary +0xb6 counter (s2+0x12);
 *         when it hits zero with +0xb9 set, sprintf a message and call
 *         the message-display path (FUN_800129e8 + FUN_8002d44c).
 *         Catchup-flag remapping of flag bit 0x10000 -> 0x8000.  If
 *         status != 0, call FUN_80020890(obj, 0) before returning.
 *
 *   1  -- (delegated to dispatcher) no-op return.
 *   2  -- call FUN_8002f998(obj)  (state cleanup).
 *   3  -- call FUN_8002d82c(obj, arg2)  (heavy event handler, 2704 B).
 *   4  -- call FUN_80042f5c(obj + 0xc0)  (timer reset).
 *
 * NOTE on physics: the per-frame integrator (Object_IntegrateAndOrient)
 * is NOT called directly from this function in the normal-state path.
 * Either (a) the engine's outer object-list walker calls a SEPARATE
 * tick on the wheels (the 4 sub-objects this Vehicle owns at obj+0xfc..
 * +0x108), each of which runs the integrator, or (b) the integrator
 * IS called from FUN_8002efe0 or FUN_8002d494 in the damaged-state
 * path and the engine sets obj+6 < 0 during normal play.  This is the
 * next investigation surface; for now we faithfully reproduce the
 * MIPS structure.
 *
 * HIGH confidence: every branch + JAL + store matches the MIPS line-
 * for-line.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern uint8_t *Terrain_MaterialAt(uint32_t x, uint32_t z);   /* FUN_800255f4 */
extern void     FUN_800441c8(uint32_t arg);                    /* audio voice-stop */
extern uint32_t FUN_800446dc(const int32_t *xyz);              /* SfxCurve_2P_Stereo */
extern void     FUN_80044574(int8_t voice, uint32_t pan);
extern void     FUN_800447e8(int8_t voiceId, void *bank, int kind, int32_t *pos);
extern void     FUN_80020890(uint32_t *obj, int mode);         /* post-tick */
extern void     FUN_80023940(uint32_t *obj);                   /* alt tick */
extern void     FUN_8002d054(uint32_t *obj, uint32_t caller_s1); /* catchup step */
extern void     FUN_8002d44c(uint32_t *obj);
extern void     FUN_8002d494(uint32_t *obj, void *lut);        /* heavy phys A */
extern void     FUN_8002d82c(uint32_t *obj, intptr_t arg);     /* mode-3 entry */
extern void     FUN_8002efe0(uint32_t *obj, const uint8_t *lut); /* heavy phys B */
extern void     FUN_8002f998(uint32_t *obj);                   /* mode-2 entry */
extern void     FUN_80042f5c(void *p);                          /* mode-4 entry */
extern int      FUN_800129e8(int displayMode, const char *msg);
extern int      sprintf(char *buf, const char *fmt, ...);

/* Globals (gp-relative loads in MIPS -> these names from earlier
 * Ghidra exports). */
extern void   *iRam00000010;            /* gp[16]    (a display-mode flag) */
extern void   *iRam000005f8;            /* gp[1528]  (audio bank handle) */
extern int (*pcRam00000730)(uint32_t obj, int eventId, uint32_t param2);
extern const char DAT_80055738[];        /* sprintf format string @0x80055738 */
extern const uint8_t DAT_80065c28[];     /* per-status lookup table @0x80065c28 */

/* Forward decl for clarity. */
int Vehicle_Tick(uint32_t *self, int mode, intptr_t arg2, intptr_t arg3);

static int vehicle_tick_trace_enabled(void)
{
    static int cached = -1;
    if (cached < 0) {
        const char *env = getenv("V8_TRACE_VEHICLE_TICK");
        cached = (env != 0 && env[0] != 0 && env[0] != '0');
    }
    return cached;
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

int Vehicle_Tick(uint32_t *self, int mode, intptr_t arg2, intptr_t arg3)
{
    (void)arg3;
    uint8_t *s = (uint8_t *)self;

    /* ---- Mode dispatch ---- */
    if (mode == 2) {
        FUN_8002f998(self);
        return 0;
    }
    if (mode == 3) {
        FUN_8002d82c(self, arg2);
        return 0;
    }
    if (mode == 4) {
        FUN_80042f5c(s + 0xc0);
        return 0;
    }
    if (mode != 0) return 0;

    /* ---- Mode 0 main tick ---- */
    uint8_t *s2 = s + 0xa4;          /* per-state block base */
    int16_t status = *(int16_t *)(s + 6);

    if (status < 0) {
        /* ---- Damaged-state path ---- */
        int32_t posX = *(int32_t *)(s + 0x24);
        int32_t posZ = *(int32_t *)(s + 0x2c);
        uint8_t *material = Terrain_MaterialAt((uint32_t)posX, (uint32_t)posZ);

        int16_t reread_status = *(int16_t *)(s + 6);
        /* Per-status lookup index: ((~status) * 3) * 8 = (~status) * 24. */
        int idx = (~(int32_t)reread_status) * 3;
        const uint8_t *s3 = &DAT_80065c28[idx * 8];

        /* Material attribute 7 -> bounce: invert all 3 vel components. */
        int16_t mat_attr_lo = *(int16_t *)(material + 0x16);
        if (mat_attr_lo == 7) {
            *(int32_t *)(s + 0x80) = mips_subu_i32(0, *(int32_t *)(s + 0x80));
            *(int32_t *)(s + 0x84) = mips_subu_i32(0, *(int32_t *)(s + 0x84));
            *(int32_t *)(s + 0x88) = mips_subu_i32(0, *(int32_t *)(s + 0x88));
        }

        /* Audio state-tracking via material attribute switch. */
        uint32_t flags = self[0];
        if ((flags & 0x200000) != 0) {
            int16_t mat_attr_hi = *(int16_t *)(material + 0x18);
            uint8_t cur_attr   = *(uint8_t *)(s + 0xd2);
            if ((int16_t)cur_attr != mat_attr_hi) {
                uint8_t prev_voice = *(uint8_t *)(s + 0xd3);
                FUN_800441c8(prev_voice);
                uint8_t new_attr = *(uint8_t *)(material + 0x18);
                *(uint8_t *)(s + 0xd2) = new_attr;
                if (new_attr != 0) {
                    int voice = 0;
                    if (pcRam00000730)
                        voice = pcRam00000730((uint32_t)(uintptr_t)self, 11,
                                              (uint32_t)(uintptr_t)material);
                    *(uint8_t *)(s + 0xd3) = (uint8_t)voice;
                } else {
                    *(uint8_t *)(s + 0xd3) = 0;
                }
            }
        } else {
            /* Flag 0x200000 NOT set -- if a voice is active, stop it. */
            uint8_t cur_voice = *(uint8_t *)(s + 0xd3);
            if (cur_voice != 0) {
                FUN_800441c8(cur_voice);
                *(uint8_t *)(s + 0xd2) = 0;
                *(uint8_t *)(s + 0xd3) = 0;
            }
        }

        if (vehicle_tick_trace_enabled()) {
            fprintf(stderr,
                    "v8: vehicle_tick pre self=%p status=%d flags=0x%x lut=0x%x "
                    "vel=(%d,%d,%d) ang=(%d,%d,%d) m18=%d\n",
                    (void *)self, (int)status, (unsigned)self[0],
                    (unsigned)*(uint32_t *)(s3 + 8),
                    *(int32_t *)(s + 0x80), *(int32_t *)(s + 0x84),
                    *(int32_t *)(s + 0x88), *(int32_t *)(s + 0x90),
                    *(int32_t *)(s + 0x94), *(int32_t *)(s + 0x98),
                    (int)*(int16_t *)(s + 0x18));
        }

        /* Heavy per-tick physics for damaged-state objects. */
        FUN_8002efe0(self, s3);
        if (vehicle_tick_trace_enabled()) {
            fprintf(stderr,
                    "v8: vehicle_tick after efe0 self=%p vel=(%d,%d,%d) "
                    "ang=(%d,%d,%d) sub=(%d,%d,%u,%d) m18=%d\n",
                    (void *)self,
                    *(int32_t *)(s + 0x80), *(int32_t *)(s + 0x84),
                    *(int32_t *)(s + 0x88), *(int32_t *)(s + 0x90),
                    *(int32_t *)(s + 0x94), *(int32_t *)(s + 0x98),
                    (int)*(int16_t *)(s + 0xa4),
                    (int)*(int16_t *)(s + 0xa6),
                    (unsigned)*(uint16_t *)(s + 0xac),
                    (int)*(int8_t *)(s + 0xb2),
                    (int)*(int16_t *)(s + 0x18));
        }
        FUN_8002d494(self, (void *)(uintptr_t)s3);
        if (vehicle_tick_trace_enabled()) {
            fprintf(stderr,
                    "v8: vehicle_tick after d494 self=%p vel=(%d,%d,%d) "
                    "ang=(%d,%d,%d) flags=0x%x m18=%d\n",
                    (void *)self,
                    *(int32_t *)(s + 0x80), *(int32_t *)(s + 0x84),
                    *(int32_t *)(s + 0x88), *(int32_t *)(s + 0x90),
                    *(int32_t *)(s + 0x94), *(int32_t *)(s + 0x98),
                    (unsigned)self[0], (int)*(int16_t *)(s + 0x18));
        }
        if (arg2 != 0) FUN_8002d054(self, (uint32_t)(uintptr_t)material);
    } else {
        /* ---- Normal-state path: tick down +0xb0 sound-event counter. ---- */
        uint16_t cnt = *(uint16_t *)(s + 0xb0);
        if (cnt != 0) {
            uint16_t newcnt = cnt - 1;
            *(uint16_t *)(s + 0xb0) = newcnt;
            if (newcnt == 0) {
                /* Counter just expired -> trigger sound. */
                int8_t voice = *(int8_t *)(s + 5);
                FUN_800447e8(voice, (void *)&iRam000005f8, 31, (int32_t *)(s + 0x24));
                *(int8_t *)(s + 5) = 0;
                self[0] &= ~0x08000000u;
            } else if (arg2 != 0) {
                /* Still counting; refresh the 3D pan curve. */
                uint32_t pan = FUN_800446dc((const int32_t *)(s + 0x24));
                int8_t voice = *(int8_t *)(s + 5);
                FUN_80044574(voice, pan);
            }
        } else {
            FUN_80023940(self);
        }
    }

    /* ---- Common tail ---- */
    /* Secondary counter at obj+0xb6 (= s2+0x12). */
    uint8_t cnt_b6 = *(uint8_t *)(s2 + 0x12);
    if (cnt_b6 != 0) {
        uint8_t newcnt = cnt_b6 - 1;
        *(uint8_t *)(s2 + 0x12) = newcnt;
        if (newcnt == 0) {
            uint8_t b9_val = *(uint8_t *)(s2 + 0x15);
            if (b9_val != 0) {
                int16_t status2 = *(int16_t *)(s + 6);
                if (status2 < 0) {
                    char msgbuf[32];
                    sprintf(msgbuf, (const char *)DAT_80055738,
                            (unsigned int)*(uint8_t *)(s2 + 0x15));
                    int displayMode = (iRam00000010 != 0) ? -status2 : 0;
                    FUN_800129e8(displayMode, msgbuf);
                    FUN_8002d44c(self);
                }
                /* Reset counter + accumulate. */
                uint8_t prev_b9 = *(uint8_t *)(s2 + 0x15);
                uint8_t prev_ba = *(uint8_t *)(s2 + 0x16);
                *(uint8_t *)(s2 + 0x12) = 30;
                *(uint8_t *)(s2 + 0x15) = 0;
                *(uint8_t *)(s2 + 0x16) = prev_ba + prev_b9;
            }
        }
    }

    /* Catchup-flag bit remap: clear 0x18000, set 0x8000 if 0x10000 was set. */
    if (arg2 != 0) {
        uint32_t f0 = self[0];
        uint32_t f1 = f0 & 0xfffe7fffu;
        if ((f0 & 0x10000u) != 0) f1 |= 0x8000u;
        self[0] = f1;
    }

    /* Damaged-state post-tick hook. */
    if (*(int16_t *)(s + 6) != 0) {
        FUN_80020890(self, 0);
    }
    return 0;
}

/* Legacy alias for the JAL site. */
int LAB_8002e2bc(uint32_t *self, int mode, intptr_t arg2, intptr_t arg3)
{
    return Vehicle_Tick(self, mode, arg2, arg3);
}
