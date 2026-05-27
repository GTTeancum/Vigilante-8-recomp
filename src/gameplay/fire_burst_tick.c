/* fire_burst_tick.c -- fire-burst effect object per-frame tick.
 *
 * Source: SLUS_005.10  LAB_8004007c  0x8004007c  (~69 instructions).
 *
 * Installed at +0x64 of the fire-burst object allocated by
 * FUN_80040234 (WeaponSpawn_FireBurst in weapon_spawn_extra.c).
 *
 * Event dispatch:
 *   0  -> tick: walk children, integrate pos+=vel, apply gravity to vel_y,
 *               free expired particles (lifetime==0).
 *   1  -> spawn: emit 8 child particles with random velocities.
 *   5  -> die:   FUN_800205f8(obj), return -1.
 *   other -> return 0.
 *
 * Particle child object field layout:
 *   +0x00 (0)   : flags/type word (set to 16 on spawn)
 *   +0x24 (36)  : world pos X  (int32)
 *   +0x28 (40)  : world pos Y  (int32)
 *   +0x2c (44)  : world pos Z  (int32)
 *   +0x34 (52)  : next-sibling pointer (int32)
 *   +0x86 (134) : remaining lifetime (uint8, ticks)
 *   +0x88 (136) : velocity X (int32)
 *   +0x8c (140) : velocity Y (int32)
 *   +0x90 (144) : velocity Z (int32)
 *
 * Parent fire-burst object field layout:
 *   +0x38 (56)  : first-child pointer (int32)
 *
 * Gravity constant: vel_y += 56 per tick (PSX Y-down; particles initially
 * move upward with negative vel_y, gravity decelerates and reverses them).
 *
 * HIGH confidence: line-for-line MIPS port.
 */
#include <stdint.h>

/* FUN_8001ac44 -- BoneObj_BuildTree: allocate & initialise a bone object. */
extern int  FUN_8001ac44(int *bank, uint16_t slot, int size, int flags);

/* FUN_80017160 -- V8_RandNext: PRNG, returns pseudo-random uint32. */
extern uint32_t FUN_80017160(void);

/* FUN_8001d564 -- Object_DetachFromParent: unlink self from parent list. */
extern int  FUN_8001d564(int self);

/* FUN_8001af48 -- Object_HeapFreeRecursiveQuiet: free particle tree. */
extern void FUN_8001af48(int obj);

/* FUN_8001d4f0 -- Object_AppendChild: tail-insert child into parent's list. */
extern void FUN_8001d4f0(uint32_t *parent, uint32_t *child);

/* FUN_800205f8 -- Damage_Apply alias: remove obj from scene. */
extern void FUN_800205f8(int obj);

/* DAT_800737d8: world bank handle (int32 value used as bone bank pointer). */
extern uintptr_t DAT_800737d8;

/* ------------------------------------------------------------------ */

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_sll_i32(int32_t v, unsigned sh)
{
    return (int32_t)((uint32_t)v << sh);
}

int LAB_8004007c(int obj, int event, int param3)
{
    uint8_t *s3 = (uint8_t *)(uintptr_t)(uint32_t)obj;
    (void)param3;

    /* ---- event dispatch ---- */
    if (event == 1) goto spawn;
    if (event == 0) goto tick;
    if (event == 5) {
        FUN_800205f8(obj);
        return -1;
    }
    return 0;

    /* ----------------------------------------------------------------
     * Event 0: tick -- walk child particle list, integrate, apply gravity,
     *                  free expired particles.
     * ---------------------------------------------------------------- */
tick:
    {
        int s0 = *(int32_t *)(s3 + 0x38);   /* first child */
        if (s0 == 0) return 0;

        do {
            uint8_t *p = (uint8_t *)(uintptr_t)(uint32_t)s0;
            int s1 = *(int32_t *)(p + 0x34);    /* next sibling (pre-load) */

            /* Decrement lifetime counter. */
            uint8_t life = (uint8_t)mips_subu_i32(p[0x86], 1);
            p[0x86] = life;

            if (life == 0) {
                /* Lifetime expired: detach and free this particle. */
                FUN_8001d564(s0);
                FUN_8001af48(s0);
            } else {
                /* Integrate position. */
                *(int32_t *)(p + 0x24) =
                    mips_addu_i32(*(int32_t *)(p + 0x24), *(int32_t *)(p + 0x88));
                *(int32_t *)(p + 0x28) =
                    mips_addu_i32(*(int32_t *)(p + 0x28), *(int32_t *)(p + 0x8c));
                *(int32_t *)(p + 0x2c) =
                    mips_addu_i32(*(int32_t *)(p + 0x2c), *(int32_t *)(p + 0x90));
                /* Apply gravity: vel_y += 56 per tick. */
                *(int32_t *)(p + 0x8c) = mips_addu_i32(*(int32_t *)(p + 0x8c), 56);
            }

            s0 = s1;
        } while (s0 != 0);

        return 0;
    }

    /* ----------------------------------------------------------------
     * Event 1: spawn -- emit 8 child particles with random velocities.
     *
     * Particle velocity ranges (approximate, Q12 / Q11 arithmetic):
     *   vel_x = (rand1 << 11 >> 15) - 1024  ~ [-2048, -1024+511] lateral
     *   vel_y = -(rand2 << 12 >> 15)          ~ upward burst
     *   vel_z = (rand3 << 11 >> 15) - 1024  ~ lateral (same form as vel_x)
     * Lifetime  = (rand4 << 3 >> 15) + 24     ~ 24..31 ticks
     * ---------------------------------------------------------------- */
spawn:
    {
        int counter = 0;

        do {
            int s0 = FUN_8001ac44((int *)(uintptr_t)DAT_800737d8, 21u, 148, 8);
            uint8_t *p = (uint8_t *)(uintptr_t)(uint32_t)s0;
            counter = mips_addu_i32(counter, 1);

            /* Zero initial world position. */
            *(int32_t *)(p + 0x24) = 0;
            *(int32_t *)(p + 0x28) = 0;
            *(int32_t *)(p + 0x2c) = 0;

            /* Velocity X: (rand1 << 11 >> 15) - 1024 */
            uint32_t rand1 = FUN_80017160();
            int32_t vel_x = mips_subu_i32(mips_sll_i32((int32_t)rand1, 11) >> 15, 1024);
            *(int32_t *)(p + 0x88) = vel_x;

            /* Velocity Y: -(rand2 << 12 >> 15)  (upward burst) */
            uint32_t rand2 = FUN_80017160();
            int32_t vel_y = mips_subu_i32(0, mips_sll_i32((int32_t)rand2, 12) >> 15);
            *(int32_t *)(p + 0x8c) = vel_y;

            /* Velocity Z: (rand3 << 11 >> 15) - 1024 */
            uint32_t rand3 = FUN_80017160();
            int32_t vel_z = mips_subu_i32(mips_sll_i32((int32_t)rand3, 11) >> 15, 1024);
            *(int32_t *)(p + 0x90) = vel_z;

            /* Flags word. */
            *(int32_t *)(p + 0x00) = 16;

            /* Lifetime: (rand4 << 3 >> 15) + 24 */
            uint32_t rand4 = FUN_80017160();
            uint8_t life = (uint8_t)mips_addu_i32(mips_sll_i32((int32_t)rand4, 3) >> 15,
                                                  24);

            /* Attach particle as child of fire-burst object. */
            FUN_8001d4f0((uint32_t *)(uintptr_t)(uint32_t)obj,
                         (uint32_t *)(uintptr_t)(uint32_t)s0);
            p[0x86] = life;

        } while (counter < 8);

        return 0;
    }
}
