/* particle_emitter_tick.c -- swirling particle effect emitter tick.
 *
 * Source: SLUS_005.10
 *   LAB_80040540  0x80040540  (emitter parent tick, ~100+ instructions)
 *   LAB_8004042c  0x8004042c  (particle child die-only tick, ~12 instructions)
 *
 * LAB_80040540 is installed at +0x64 of particle-emitter objects spawned
 * by FUN_800407b4 (Particle_SpawnFromPool in particle_spawn.c).
 *
 * Event dispatch:
 *   0  -> main tick: optionally sync matrix from parent, decrement timer,
 *         on wrap spawn new child particle, walk children to update orbit.
 *   2  -> timer reset: set timer to 0xffff (begin new countdown cycle).
 *   other -> return 0.
 *
 * LAB_8004042c is installed at +0x64 of each child particle allocated during
 * event 0 processing. The tick itself does nothing (motion is updated by
 * the parent walk loop). On event 5 (death), detach from parent and free.
 *
 * Child particle field layout (offsets from base pointer):
 *   +0x00 (0)   : flags / type word
 *   +0x24 (36)  : world pos X   (int32) -- written by FUN_8001d708
 *   +0x28 (40)  : world pos Y   (int32)
 *   +0x2c (44)  : world pos Z   (int32)
 *   +0x34 (52)  : next-sibling  (int32)
 *   +0x44 (68)  : sub-timer     (uint16, += puff[0x84] each tick)
 *   +0x48 (72)  : orbit pos X   (int32, = RTZ(speed*sin>>12))
 *   +0x4c (76)  : orbit pos Y   (int32, += vel_y each tick)
 *   +0x50 (80)  : orbit pos Z   (int32, = RTZ(speed*cos>>12))
 *   +0x64 (100) : tick callback  (= &LAB_8004042c)
 *   +0x80 (128) : packed rates   (uint32 from obj[0x90]: lo=rate0,hi=angle_rate)
 *   +0x82 (130) : angle rate     (uint16, &= 0xfff range)
 *   +0x84 (132) : sub-timer rate (uint16)
 *   +0x86 (134) : current angle  (uint16, &= 0xfff for table lookup)
 *   +0x88 (136) : vel X seed     (not used directly; see vel_y)
 *   +0x8c (140) : vel Y (drift)  (int32)
 *   +0x98 (152) : orbit speed    (int32, += s0[0x9c] per tick)
 *   +0x9c (156) : speed growth   (int32)
 *
 * Emitter parent field layout:
 *   +0x00 (0)   : flags (bit 17=0x20000: inherit parent matrix; bit 16=0x10000: rand angle)
 *   +0x10 (16)  : rotation matrix (MATRIX)
 *   +0x38 (56)  : first-child pointer
 *   +0x3c (60)  : parent pointer
 *   +0x80 (128) : countdown timer (uint16)
 *   +0x82 (130) : timer reload value (uint16)
 *   +0x84 (132) : speed growth seed (int32)
 *   +0x88 (136) : vel_y drift seed  (int32)
 *   +0x90 (144) : packed angle rates (uint32, copied to child[0x80])
 *   +0x94 (148) : sub-timer rate     (int16, copied to child[0x84])
 *   +0x96 (150) : bone-slot index    (uint16)
 *   +0x98 (152) : bank handle        (int32)
 *
 * Sin/cos table: DAT_800607b4[i*2]=sin[i], DAT_800607b4[i*2+1]=cos[i], i in [0,4095].
 *
 * HIGH confidence: line-for-line MIPS port.
 */
#include <stdint.h>
#include <string.h>
#include "../../include/structs.h"

/* FUN_8001d5a0 -- Object_Parent: walk parent chain. */
extern int     FUN_8001d5a0(int obj);

/* FUN_8001d624 -- Object_GetWorldMatrix: returns MATRIX * (= &obj[0x10]). */
extern MATRIX *FUN_8001d624(int obj);

/* FUN_8004d734 -- TransposeMatrix: dst = transpose(src). */
extern MATRIX *FUN_8004d734(MATRIX *src, MATRIX *dst);

/* FUN_8001ac44 -- BoneObj_BuildTree: allocate & initialise a bone object. */
extern int     FUN_8001ac44(int *bank, uint16_t slot, int size, int flags);

/* FUN_80017160 -- V8_RandNext: PRNG. */
extern uint32_t FUN_80017160(void);

/* FUN_80044efc -- memset (byte-fill with u32 unroll). */
extern void   *FUN_80044efc(void *dst, uint32_t fill, int count);

/* FUN_8001d4f0 -- Object_AppendChild: tail-insert child. */
extern void    FUN_8001d4f0(uint32_t *parent, uint32_t *child);

/* FUN_8001d708 -- Object_InitBoneMatrix: update world pos + rot from local. */
extern void    FUN_8001d708(int obj);

/* FUN_8001d564 -- Object_DetachFromParent: unlink self; returns self. */
extern int     FUN_8001d564(int self);

/* FUN_8001af48 -- Object_HeapFreeRecursiveQuiet: free tree quietly. */
extern void    FUN_8001af48(int obj);
extern void    Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* FUN_800204dc -- Object_FreeAndUnregister: free + remove from lists. */
extern void    FUN_800204dc(int obj);

/* FUN_800205f8 -- Damage_Apply alias. */
extern void    FUN_800205f8(int obj);

/* DAT_800607b4: 4096-entry interleaved sin/cos Q12 LUT. */
extern int16_t DAT_800607b4[];

/* RTZ arithmetic right shift by n bits. */
static inline int32_t rtz_sra(int32_t x, int n)
{
    if (x < 0) x += (1 << n) - 1;
    return x >> n;
}

/* ------------------------------------------------------------------ */

/*
 * LAB_8004042c -- child particle die-only tick.
 * event 5: detach from parent and free, return -1.
 * else:     return 0.
 */
int LAB_8004042c(int obj, int event, int param3)
{
    (void)param3;
    if (event == 5) {
        FUN_8001d564(obj);
        FUN_8001af48(obj);
        return -1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */

/*
 * LAB_80040540 -- particle emitter per-frame tick.
 * Source: 0x80040540 .. 0x800407b0.
 */
int LAB_80040540(int obj, int event, int param3)
{
    uint8_t *s1 = (uint8_t *)(uintptr_t)(uint32_t)obj;

    if (event == 2) {
        /* Timer reset: set to 0xffff (begin new countdown cycle). */
        *(uint16_t *)(s1 + 0x80) = 0xffffu;
        return 0;
    }
    if (event != 0) {
        return 0;
    }

    /* ---- Event 0: tick ---- */

    /* If flag 0x20000 (inherit parent orientation): copy parent matrix. */
    if (*(uint32_t *)(s1 + 0x00) & 0x00020000u) {
        int parent = FUN_8001d5a0(obj);
        MATRIX *worldmat = FUN_8001d624(parent);
        FUN_8004d734(worldmat, (MATRIX *)(s1 + 0x10));
    }

    /* Decrement timer; spawn on wrap (0 → 0xffff). */
    uint16_t timer = *(uint16_t *)(s1 + 0x80);
    timer = (uint16_t)(timer - 1u);
    *(uint16_t *)(s1 + 0x80) = timer;

    if (timer == 0xffffu) {
        /* ---- Spawn new child particle ---- */
        int bank = *(int32_t *)(s1 + 0x98);
        uint16_t slot = *(uint16_t *)(s1 + 0x96);
        int s0 = FUN_8001ac44((int *)(uintptr_t)(uint32_t)bank, slot, 160, 8);
        uint8_t *p = (uint8_t *)(uintptr_t)(uint32_t)s0;

        /* Set particle flags. */
        *(uint32_t *)(p + 0x00) |= 0x410u;

        /* Copy packed angle rates and sub-timer rate from emitter. */
        *(uint32_t *)(p + 0x80) = *(uint32_t *)(s1 + 0x90);   /* LWL/LWR pair */
        *(int16_t  *)(p + 0x84) = *(int16_t  *)(s1 + 0x94);

        /* Radius growth rate. */
        *(int32_t *)(p + 0x9c) = *(int32_t *)(s1 + 0x84);

        /* vel_y: drift + (rand * drift) >> 15. */
        uint32_t rand1 = FUN_80017160();
        int32_t drift = *(int32_t *)(s1 + 0x88);
        int32_t vy = drift + (int32_t)(((int32_t)rand1 * drift) >> 15);
        *(int32_t *)(p + 0x8c) = vy;

        /* Clear local orbit position buffer (p[0x48..0x53] = 12 bytes). */
        FUN_80044efc(p + 0x48, 0u, 12);

        /* Install child tick callback. */
        Object_SetCallbackPsxSlot((void *)(uintptr_t)p, (uintptr_t)&LAB_8004042c);

        /* If flag 0x10000: randomise initial angle. */
        if (*(uint32_t *)(s1 + 0x00) & 0x00010000u) {
            uint32_t rand2 = FUN_80017160();
            *(uint16_t *)(p + 0x86) = (uint16_t)rand2;
        }

        /* Attach particle as child of emitter object. */
        FUN_8001d4f0((uint32_t *)(uintptr_t)(uint32_t)obj,
                     (uint32_t *)(uintptr_t)(uint32_t)s0);

        /* Reset emitter timer. */
        *(uint16_t *)(s1 + 0x80) = *(uint16_t *)(s1 + 0x82);
    }

    /* ---- Walk child particles and update orbit ---- */
    {
        int child = *(int32_t *)(s1 + 0x38);

        if (child == 0) {
            /* No children -- self-destruct. */
            if (*(int32_t *)(s1 + 0x3c) != 0) {
                /* Has parent: detach and free. */
                int ret = FUN_8001d564(obj);
                FUN_800204dc(ret);
            } else {
                /* No parent: Damage_Apply. */
                FUN_800205f8(obj);
            }
            return -1;
        }

        /* Reuse s1 pointer as the sin/cos table base (saves a register in MIPS). */
        int16_t *table = DAT_800607b4;

        do {
            uint8_t *cp = (uint8_t *)(uintptr_t)(uint32_t)child;
            int next = *(int32_t *)(cp + 0x34);    /* pre-load next sibling */

            /* Update drift: orbit_y += vel_y. */
            *(int32_t *)(cp + 0x4c) += *(int32_t *)(cp + 0x8c);

            /* Update sub-timer: counter += rate. */
            uint16_t sub = (uint16_t)(*(uint16_t *)(cp + 0x44)
                                     + *(uint16_t *)(cp + 0x84));
            *(uint16_t *)(cp + 0x44) = sub;

            /* Advance angle: angle = (angle + angle_rate) & 0xfff. */
            uint16_t angle = (uint16_t)(*(uint16_t *)(cp + 0x86)
                                       + *(uint16_t *)(cp + 0x82));
            *(uint16_t *)(cp + 0x86) = angle;
            angle &= 0x0fffu;

            /* Orbit X = RTZ(speed * sin(angle) >> 12). */
            int32_t speed = *(int32_t *)(cp + 0x98);
            int32_t vx = rtz_sra(speed * (int32_t)table[angle * 2], 12);
            *(int32_t *)(cp + 0x48) = vx;

            /* Re-read angle for Z (same angle, same mask). */
            angle = *(uint16_t *)(cp + 0x86) & 0x0fffu;

            /* Orbit Z = RTZ(speed * cos(angle) >> 12). */
            int32_t vz = rtz_sra(speed * (int32_t)table[angle * 2 + 1], 12);
            *(int32_t *)(cp + 0x50) = vz;

            /* Grow orbit speed. */
            int32_t growth = *(int32_t *)(cp + 0x9c);
            *(int32_t *)(cp + 0x98) = speed + growth;

            /* If param3 != 0: update world-space position from local orbit. */
            if (param3 != 0) {
                FUN_8001d708(child);
            }

            child = next;
        } while (child != 0);
    }

    return 0;
}
