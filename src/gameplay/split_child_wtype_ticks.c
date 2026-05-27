/* split_child_wtype_ticks.c -- per-wtype split-child tick/spawn callbacks.
 *
 * Source: SLUS_005.10
 *
 *  FUN_80031e34 / LAB_80031fa0  -- wtype-2 child tick (bouncing debris)
 *  FUN_80032f7c / LAB_8003302c  -- wtype-4 child tick (spring-guided projectile)
 *  FUN_80033648 / LAB_800336fc  -- wtype-6 missile-slot event dispatcher
 *
 * These are invoked by split_child_tick.c event-3 dispatch for the
 * respective weapon types.  LAB_80031fa0 and LAB_8003302c are installed
 * as per-tick obj[+0x64] callbacks on child objects.  LAB_800336fc is the
 * event callback stored for the wtype-6 attached slot; its keyed fire path
 * can spawn homing missile children.
 *
 * MED confidence: direct MIPS disassembly port.
 * NOTE: event-0 SFX uses PSX SPU direct-register writes in the original
 * (skipped on host; voice alloc + rand still called to advance state).
 * Collision-path FUN_8004483c bank arg uses DAT_80065BB8 (MED -- the
 * parent split-child tick stores its bank there on spawn, unverified).
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* -- External helpers -- */

/* FUN_80025400 -- Terrain_HeightAt: return terrain Y at world (X, Z). */
extern int FUN_80025400(int x, int z);

/* FUN_8001d708 -- Object_InitBoneMatrix: initialise/reset bone transform. */
extern void FUN_8001d708(intptr_t obj);

/* FUN_8003fd24 -- Debris_Spawn: spawn impact particle, kind=arg2. Returns int handle. */
extern intptr_t FUN_8003fd24(const int32_t *xyz, int kind);

/* FUN_8004410c -- Audio_AllocVoice: allocate a free SPU voice channel. */
extern int FUN_8004410c(void);

/* FUN_8004483c -- Audio_PlaySfx3D: play SFX on voice at 3D position. */
extern int FUN_8004483c(int voice, int bank, int sfx, const int32_t *pos);

/* FUN_800205f8 -- Damage_Apply / Object_Retire: retire object from scene. */
extern void FUN_800205f8(intptr_t obj);

/* FUN_80030c08 -- Object_GeneralTick: run general rolling/physics tick. */
extern void FUN_80030c08(uint32_t *obj);

/* FUN_80031454 -- WeaponHit_Apply: apply impulse + damage to collider.
 *   Returns -1 (event consumed) or 0 (pass-through). */
extern int FUN_80031454(intptr_t param_1, intptr_t *param_2, uint16_t param_3, int param_4);
extern uintptr_t Collision_QueryHostWord(const void *query, uint32_t index);

/* FUN_80017160 -- V8_RandNext: advance LCG, return next random uint32. */
extern uint32_t FUN_80017160(void);

/* FUN_800439b8 -- RotMatrix_ApplyAngVel: apply angular velocity to rotation matrix. */
extern void FUN_800439b8(uint32_t *obj);

/* FUN_8004c934 -- MatrixNormal / smoke emitter. */
extern void FUN_8004c934(uint32_t *obj);

/* FUN_800202f4 -- Object_RegisterInScene: register new object in the world. */
extern void FUN_800202f4(intptr_t obj);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* FUN_80031300 -- Object_SpawnAttached: spawn a bone child object.
 *   (parent, subBin, kind, kindB=0x98, flags) -> child object handle */
extern intptr_t FUN_80031300(intptr_t parent, intptr_t subBin, int kind, uint32_t kindB, intptr_t flags);

/* FUN_800176f8 -- Object_ApplyImpulseAtPoint: add impulse vec to linear+angular vel. */
extern void FUN_800176f8(intptr_t obj, const int32_t *vec, const int32_t *pos);

/* FUN_8002cb7c -- WeaponSlot_Deactivate: deactivate/despawn weapon slot on vehicle. */
extern void FUN_8002cb7c(intptr_t vehicle);
extern int FUN_8003c538(uint32_t *obj, intptr_t arg);
extern void *FUN_8003351c(intptr_t self, uint32_t *owner, int16_t kind,
                          uint16_t timer);
extern intptr_t LAB_80033290(intptr_t obj, int event, intptr_t param3);
extern int FUN_80016aac(const int32_t *a, const int32_t *b);
extern void *Matrix_ComposeParentChain(intptr_t obj);
extern long ratan2(int y, int x);
extern intptr_t FUN_8001d5a0(intptr_t obj);

/* Globals */
extern int32_t DAT_80065310; /* frame tick counter (low word) */
/* DAT_80065BB8: shared sound-bank handle written by split_child_tick event-1 spawn.
 * This is the most likely candidate for the 'bank' arg to FUN_8004483c in the
 * child tick SFX paths.  FIXME: unverified (original uses PSX SPU direct-writes
 * in some paths; FUN_8004483c is only called in event-3 collision paths). */
extern uintptr_t DAT_80065BB8;

/* ------------------------------------------------------------------ */
/* Helper: round-toward-zero arithmetic right shift. */
static int32_t rtz_sra(int32_t v, int shift)
{
    if (v < 0)
        v = (int32_t)((uint32_t)v + (uint32_t)((1 << shift) - 1));
    return v >> shift;
}

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_sll_i32(int32_t v, int shift)
{
    return (int32_t)((uint32_t)v << shift);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

static int weapon_trace_enabled(void)
{
    static int checked = 0;
    static int enabled = 0;
    if (!checked) {
        enabled = getenv("V8_TRACE_WEAPONS") != NULL;
        checked = 1;
    }
    return enabled;
}

/* ------------------------------------------------------------------ */
/*
 * LAB_80031fa0 / FUN_80031e34 -- wtype-2 split child tick.
 *
 * Spawned from split_child_tick.c event-3 / wtype-2 path.
 * Installed at obj[+0x64] (offset 100) on the child object.
 *
 * Object field layout (relevant):
 *   +0x00 / +0    flags (int32)
 *   +0x24 / +36   render pos X
 *   +0x28 / +40   render pos Y
 *   +0x2c / +44   render pos Z
 *   +0x48 / +72   world pos X (int32)
 *   +0x4c / +76   world pos Y
 *   +0x50 / +80   world pos Z
 *   +0x88 / +136  velocity X (int32)
 *   +0x8c / +140  velocity Y
 *   +0x90 / +144  velocity Z
 *   +0x94 / +148  lifetime countdown (uint16)
 *
 * Event dispatch:
 *   0  -> integrate vel into pos; if underground: spawn debris, alloc voice,
 *          play SFX (PSX SPU direct), transition to rolling tick.
 *   3  -> collision: check target type, spawn debris, play SFX, retire self.
 *   other -> return 0.
 */
intptr_t LAB_80031fa0(intptr_t obj, int event, intptr_t param3)
{
    uint8_t  *s0  = (uint8_t  *)(uintptr_t)obj;
    int32_t  *pos = (int32_t  *)(s0 + 72);

    if (event == 0) {
        int terrain_h = FUN_80025400(pos[0], pos[2]);

        if (terrain_h >= pos[1]) {
            /* Underground: spawn impact debris. */
            int debris = FUN_8003fd24(pos, 1);
            if (debris) {
                *(uint32_t *)((uint8_t *)(uintptr_t)(uint32_t)debris + 68)
                    = FUN_80017160();
                FUN_8001d708(debris);
            }

            /* Allocate SPU voice, select SFX based on RNG (61 or 64). */
            /* MIPS: alloc voice → s3; rand&3 → sfx; PSX SPU direct writes (skipped). */
            FUN_8004410c();
            FUN_80017160();

            /* Transition child to general rolling tick. */
            FUN_80030c08((uint32_t *)(uintptr_t)obj);
            return 0;
        }

        /* Above terrain: integrate velocity into position. */
        *(int32_t *)(s0 + 72) = mips_addu_i32(*(int32_t *)(s0 + 72), *(int32_t *)(s0 + 136));
        *(int32_t *)(s0 + 76) = mips_addu_i32(*(int32_t *)(s0 + 76), *(int32_t *)(s0 + 140));
        *(int32_t *)(s0 + 80) = mips_addu_i32(*(int32_t *)(s0 + 80), *(int32_t *)(s0 + 144));

        /* Mirror physics position to render position. */
        *(int32_t *)(s0 + 36) = *(int32_t *)(s0 + 72);
        *(int32_t *)(s0 + 40) = *(int32_t *)(s0 + 76);
        *(int32_t *)(s0 + 44) = *(int32_t *)(s0 + 80);

        /* Decrement lifetime; transition to rolling when expired. */
        {
            uint16_t life = *(uint16_t *)(s0 + 148);
            --life;
            *(uint16_t *)(s0 + 148) = life;
            if (life == 0)
                FUN_80030c08((uint32_t *)(uintptr_t)obj);
        }
        return 0;
    }

    if (event == 3) {
        /* Collision: dereference param3 to get target object. */
        intptr_t target_h = (intptr_t)Collision_QueryHostWord((void *)(uintptr_t)param3, 0);
        uint8_t  *tgt      = (uint8_t *)(uintptr_t)target_h;

        /* Skip if target is terrain (type byte at +4 == 3). */
        if (tgt[4] == 3)
            return 0;

        /* Spawn impact debris at projectile position. */
        {
            int debris = FUN_8003fd24(pos, 1);
            if (debris) {
                uint8_t *dp = (uint8_t *)(uintptr_t)(uint32_t)debris;
                *(uint32_t *)dp |= 0x400u;      /* set bit 0x400 in flags */
                *(uint32_t *)(dp + 68) = FUN_80017160();
                FUN_8001d708(debris);
            }
        }

        /* Allocate voice, advance RNG, select SFX from target kind (lhu +6). */
        {
            int      voice = FUN_8004410c();
            uint16_t tval  = *(uint16_t *)(tgt + 6);
            int      sfx;
            FUN_80017160(); /* advance RNG (MIPS uses prior result, not needed here) */
            if      (tval < 64u)  sfx = 64;
            else if (tval < 96u)  sfx = 63;
            else if (tval < 128u) sfx = 62;
            else if (tval < 160u) sfx = 61;
            else                  sfx = 64;
            FUN_8004483c(voice, (int)DAT_80065BB8 /* MED: shared split-child bank ptr */, sfx, pos);
        }

        /* Retire self. */
        FUN_800205f8(obj);
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/*
 * LAB_8003302c / FUN_80032f7c -- wtype-4 split child tick.
 *
 * Spring-guided projectile: two spring channels correct X and vertical(Y)
 * velocity each tick toward target offsets stored in the object.
 *
 * Object field layout (relevant):
 *   +0x00 / +0    flags (int32)
 *   +0x14 / +20   spring target X (int16)
 *   +0x20 / +32   spring target Y/vertical (int16)
 *   +0x48 / +72   world pos X (int32)
 *   +0x4c / +76   world pos Y (vertical, up)
 *   +0x50 / +80   world pos Z
 *   +0x84 / +132  proxy object handle (int32)
 *   +0x88 / +136  velocity X (int32, spring-corrected)
 *   +0x8c / +140  velocity Y (int32, uncorrected middle channel)
 *   +0x90 / +144  velocity Z (int32, spring-corrected for vertical)
 *   +0x94 / +148  lifetime countdown (uint16)
 *
 * NOTE: +144 drives pos.y (+76) in this object layout -- the second
 * spring channel controls vertical height, not Z.  FIXME: verify.
 *
 * Event dispatch:
 *   0  -> spring-damper integration; terrain test; frame-phased effects.
 *   3  -> collision: if target type == 7, pass; else WeaponHit_Apply.
 *   4  -> die: clear flag 0x800000 on proxy object.
 *   other -> return 0.
 */
intptr_t LAB_8003302c(intptr_t obj, int event, intptr_t param3)
{
    uint8_t  *s1  = (uint8_t  *)(uintptr_t)obj;
    int32_t  *pos = (int32_t  *)(s1 + 72);

    if (event == 3) {
        /* Collision: load target, skip if type == 7, else apply hit. */
        intptr_t t_h = (intptr_t)Collision_QueryHostWord((void *)(uintptr_t)param3, 0);
        uint8_t  *tgt = (uint8_t *)(uintptr_t)t_h;
        if (tgt[4] == 7)
            return 0;
        return FUN_80031454(obj, (intptr_t *)(uintptr_t)param3, 26u, 55);
    }

    if (event == 4) {
        /* Die: clear 0x800000 in proxy object flags. */
        int32_t proxy_h = *(int32_t *)(s1 + 132);
        if (proxy_h) {
            uint8_t *proxy = (uint8_t *)(uintptr_t)(uint32_t)proxy_h;
            *(uint32_t *)proxy &= ~0x800000u;
        }
        return 0;
    }

    if (event != 0)
        return 0;

    /* -- Event 0: spring-damper tick -- */

    /* X spring (source int16 at +20) → corrects velocity at +136 → pos at +72. */
    {
        int32_t tgt_vel = rtz_sra(mips_mult_lo_i32((int16_t)*(int16_t *)(s1 + 20), 15258), 12);
        int32_t cur_vel = *(int32_t *)(s1 + 136);
        int32_t delta   = rtz_sra(mips_subu_i32(tgt_vel, cur_vel), 4);
        if (delta >  256) delta =  256;
        if (delta < -256) delta = -256;
        cur_vel = mips_addu_i32(cur_vel, delta);
        *(int32_t *)(s1 + 136) = cur_vel;
        *(int32_t *)(s1 + 72) = mips_addu_i32(*(int32_t *)(s1 + 72), cur_vel);
    }

    /* Vertical spring (source int16 at +32) → corrects velocity at +144 → pos at +76. */
    {
        int32_t tgt_vel = rtz_sra(mips_mult_lo_i32((int16_t)*(int16_t *)(s1 + 32), 15258), 12);
        int32_t cur_vel = *(int32_t *)(s1 + 144);
        int32_t delta   = rtz_sra(mips_subu_i32(tgt_vel, cur_vel), 4);
        if (delta >  256) delta =  256;
        if (delta < -256) delta = -256;
        cur_vel = mips_addu_i32(cur_vel, delta);
        *(int32_t *)(s1 + 144) = cur_vel;
        *(int32_t *)(s1 + 76) = mips_addu_i32(*(int32_t *)(s1 + 76), cur_vel);  /* pos.y (vertical height) */
    }

    /* Terrain floor test. */
    {
        int terrain_h = FUN_80025400(pos[0], pos[2]);
        if (terrain_h >= pos[1]) {
            /* Underground: impact SFX, debris, free proxy, return -1. */
            int voice = FUN_8004410c();
            FUN_8004483c(voice, (int)DAT_80065BB8 /* MED: shared split-child bank ptr */, 55, pos);
            FUN_8003fd24(pos, 12);
            {
                int proxy_h = *(int32_t *)(s1 + 132);
                FUN_800205f8(proxy_h);
            }
            return -1;
        }
    }

    /* Sync proxy object world position. */
    {
        int32_t proxy_h = *(int32_t *)(s1 + 132);
        if (proxy_h) {
            uint8_t *proxy = (uint8_t *)(uintptr_t)(uint32_t)proxy_h;
            *(int32_t *)(proxy + 72) = pos[0];
            *(int32_t *)(proxy + 76) = pos[1];
            *(int32_t *)(proxy + 80) = pos[2];
        }
    }

    /* Decrement lifetime; expire if zero. */
    {
        uint16_t life = *(uint16_t *)(s1 + 148);
        --life;
        *(uint16_t *)(s1 + 148) = life;
        if (life == 0) {
            int voice = FUN_8004410c();
            FUN_8004483c(voice, (int)DAT_80065BB8 /* MED: shared split-child bank ptr */, 55, pos);
            FUN_8003fd24(pos, 26);
            FUN_800205f8(obj);
            return 0;
        }
    }

    /* Every 4 frames: spawn spark effect. */
    if ((DAT_80065310 & 3) == 0) {
        int spark = FUN_8003fd24(pos, 18);
        if (spark)
            FUN_8001d708(spark);
    }

    /* Apply angular velocity to rotation matrix. */
    FUN_800439b8((uint32_t *)(uintptr_t)obj);

    /* Every 32 frames: smoke effect. */
    if ((DAT_80065310 & 31) == 0)
        FUN_8004c934((uint32_t *)(uintptr_t)obj);

    return 0;
}

/* ------------------------------------------------------------------ */
/*
 * LAB_800336fc -- wtype-6 / missile-slot event dispatcher.
 *
 * This is the callback stored in DAT_80010534 for vehicle sub-object kind 7.
 * It was previously mistranscribed as a one-shot spawn helper; the original
 * MIPS has a 15-entry event jump table at 0x80010680.
 */
intptr_t LAB_800336fc(intptr_t obj, intptr_t event, int kind)
{
    uint8_t *self = (uint8_t *)(uintptr_t)obj;
    intptr_t arg = (intptr_t)kind;

    if (weapon_trace_enabled() && (event == 0xb || event == 1 || event == 0)) {
        fprintf(stderr,
                "v8: missile weapon dispatch obj=%p event=%d arg=%p hp=%u status=%d flags=0x%x\n",
                (void *)(uintptr_t)obj, (int)event, (void *)(uintptr_t)arg,
                (unsigned)*(uint16_t *)(self + 0x0c),
                (int)*(int16_t *)(self + 0x06),
                (unsigned)*(uint32_t *)self);
    }

    switch ((int)event) {
    case 0:
        if (!FUN_8003c538((uint32_t *)(uintptr_t)obj, arg))
            return 0;
        if (*(uint32_t *)(uintptr_t)(arg + 0xe4) == 0)
            return 0;

        {
            uint8_t *joint = (uint8_t *)(uintptr_t)*(uint32_t *)(self + 0x38);
            int32_t *target_pos = (int32_t *)(uintptr_t)(*(uint32_t *)(uintptr_t)(arg + 0xe4) + 0x48);
            int32_t *self_pos = (int32_t *)(self + 0x48);
            int32_t dx = mips_subu_i32(target_pos[0], self_pos[0]);
            int32_t dy = mips_subu_i32(target_pos[1], self_pos[1]);
            int32_t dz = mips_subu_i32(target_pos[2], self_pos[2]);
            int32_t dist = FUN_80016aac(self_pos, target_pos);
            int32_t yaw = (int32_t)((ratan2(dx, dz) << 20) >> 20);
            int32_t pitch = (int32_t)((-ratan2(dy, dist)) << 20) >> 20;
            if (pitch > 256) pitch = 256;
            if (pitch < -128) pitch = -128;
            if (joint) {
                int32_t cur_yaw = *(int16_t *)(joint + 0x42);
                int32_t delta_yaw = mips_subu_i32(yaw, cur_yaw);
                if (delta_yaw < 0) delta_yaw = mips_addu_i32(delta_yaw, 3);
                *(uint16_t *)(joint + 0x42) =
                    (uint16_t)mips_addu_i32((int32_t)*(uint16_t *)(joint + 0x42), delta_yaw >> 2);

                int32_t cur_pitch = *(int16_t *)(joint + 0x40);
                int32_t delta_pitch = mips_subu_i32(pitch, cur_pitch);
                if (delta_pitch < 0) delta_pitch = mips_addu_i32(delta_pitch, 3);
                *(uint16_t *)(joint + 0x40) =
                    (uint16_t)mips_addu_i32((int32_t)*(uint16_t *)(joint + 0x40), delta_pitch >> 2);
                FUN_8001d708((intptr_t)joint);
            }
        }
        return 0;

    case 1:
        *(uint16_t *)(self + 0x0c) = 12;
        self[0x08] = 3;
        *(uint32_t *)self |= 0x4000u;
        return 0;

    case 9: {
        uint32_t fire_code = (uint32_t)arg & 0xfffu;
        if (fire_code == 0x242u) {
            uint16_t ammo = *(uint16_t *)(self + 0x0c);
            if (ammo < 2)
                return -1;
            *(uint16_t *)(self + 0x0c) = (uint16_t)mips_addu_i32(ammo, -1);
            intptr_t owner = FUN_8001d5a0(obj);
            uint32_t *shot = (uint32_t *)FUN_8003351c(obj, (uint32_t *)(uintptr_t)owner, 0x23, 0x28);
            *(uint8_t *)((uint8_t *)shot + 0x08) = 1;
            *shot |= 0x01000000u;
            return 0x78;
        }
        if (fire_code == 0x244u) {
            uint16_t ammo = *(uint16_t *)(self + 0x0c);
            if (ammo < 2)
                return -1;
            intptr_t owner = FUN_8001d5a0(obj);
            uint8_t *joint = (uint8_t *)(uintptr_t)*(uint32_t *)(self + 0x38);
            int16_t save_pitch = joint ? *(int16_t *)(joint + 0x40) : 0;
            int16_t save_yaw = joint ? *(int16_t *)(joint + 0x42) : 0;
            uint8_t *first = (uint8_t *)FUN_8003351c(obj, (uint32_t *)(uintptr_t)owner, 6, 0x4b);
            if (first != NULL)
                *(uint32_t *)first |= 0x01000000u;
            for (uint32_t i = 1; i < 6 && *(uint16_t *)(self + 0x0c) != 0; i++) {
                uint8_t *shot;
                if (joint) {
                    int32_t rand_pitch = (mips_mult_lo_i32((int32_t)FUN_80017160(), 3) << 6) >> 15;
                    int32_t rand_yaw = (mips_mult_lo_i32((int32_t)FUN_80017160(), 3) << 6) >> 15;
                    *(uint16_t *)(joint + 0x40) =
                        (uint16_t)mips_addu_i32(mips_addu_i32(save_pitch, rand_pitch), -96);
                    *(uint16_t *)(joint + 0x42) =
                        (uint16_t)mips_addu_i32(mips_addu_i32(save_yaw, rand_yaw), -96);
                    FUN_8001d708((intptr_t)joint);
                }
                shot = (uint8_t *)FUN_80031300(owner, (intptr_t)joint, 6, 0x98, 0);
                if (shot != NULL) {
                    *(uint32_t *)shot = 0x01800094u;
                    *(uint16_t *)(shot + 0x0c) = 75;
                    Object_SetCallbackPsxSlot(shot, (uintptr_t)&LAB_80033290);
                    FUN_800202f4((intptr_t)shot);
                    *(uint16_t *)(shot + 0x94) = 60;
                    *(int32_t *)(shot + 0x88) =
                        rtz_sra(*(int32_t *)((uint8_t *)(uintptr_t)owner + 0x80), 7) +
                        (int16_t)*(uint16_t *)(shot + 0x14) * 6;
                    *(int32_t *)(shot + 0x8c) =
                        rtz_sra(*(int32_t *)((uint8_t *)(uintptr_t)owner + 0x84), 7) +
                        (int16_t)*(uint16_t *)(shot + 0x1a) * 6;
                    *(int32_t *)(shot + 0x90) =
                        rtz_sra(*(int32_t *)((uint8_t *)(uintptr_t)owner + 0x88), 7) +
                        (int16_t)*(uint16_t *)(shot + 0x20) * 6;
                }
                *(uint16_t *)(self + 0x0c) =
                    (uint16_t)mips_addu_i32(*(uint16_t *)(self + 0x0c), -1);
            }
            if (joint) {
                *(int16_t *)(joint + 0x40) = save_pitch;
                *(int16_t *)(joint + 0x42) = save_yaw;
                FUN_8001d708((intptr_t)joint);
            }
            if (*(uint16_t *)(self + 0x0c) == 0)
                FUN_8002cb7c(obj);
            return 0x78;
        }
        return 0;
    }

    case 10:
        return 0;

    case 11: {
        uint16_t timer = (*(int16_t *)(uintptr_t)(arg + 0x11c) != 0) ? 150u : 75u;
        void *shot = FUN_8003351c(obj, (uint32_t *)(uintptr_t)arg, 6, timer);
        if (weapon_trace_enabled()) {
            fprintf(stderr,
                    "v8: missile weapon fired weapon=%p owner=%p shot=%p timer=%u ammo=%u\n",
                    (void *)(uintptr_t)obj, (void *)(uintptr_t)arg, shot,
                    (unsigned)timer, (unsigned)*(uint16_t *)(self + 0x0c));
        }
        return 60;
    }

    case 12: {
        intptr_t target = *(uint32_t *)(uintptr_t)(arg + 0xe4);
        if (target == 0)
            return 0;
        int dist = FUN_80016aac((const int32_t *)(self + 0x48),
                                (const int32_t *)(uintptr_t)(target + 0x48));
        if (dist > 0x3e7fff)
            return 0;
        return (*(int32_t *)(uintptr_t)(arg + 0x8c) < 0x11e1) ? 1 : 0;
    }

    case 13:
        return 3;

    case 14:
        return 0x8013;

    default:
        return 0;
    }
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0
/* --- SLUS_005.10 FUN_80031e34 (LAB_80031fa0) manual disassembly reference ---
 *
 * Entry: 80031e34  (LAB_80031fa0 = 0x80031fa0 is inside the body; engine
 *         dispatches to it as the obj[+0x64] callback entry point.)
 *
 * s2 = obj; s1 = set during event-specific paths.
 * Event dispatch: event==0 → 80031e68; event==3 → 80031f54; else → j 80031848 (shared ret-0)
 *
 * Event 0 underground path (terrain_h >= pos.y):
 *   FUN_8003fd24(&pos, 1) → debris; rand→debris+68; FUN_8001d708(debris)
 *   s3=FUN_8004410c(); rand&3→sfx (61 or 64); PSX SPU direct writes; FUN_80030c08(obj)
 *
 * Event 0 above-terrain path:
 *   pos += vel (X/Y/Z at +72,+76,+80 from vel at +136,+140,+144)
 *   render_pos (at +36,+40,+44) = pos; lhu +148 -- dec; if zero: FUN_80030c08
 *
 * Event 3 (collision, entry at 80031f54):
 *   s1 = *(int*)param3 = target obj
 *   if target[4]==3: return 0
 *   FUN_8003fd24(&pos,1)→debris; debris[0]|=0x400; rand→debris+68; FUN_8001d708(debris)
 *   s3=FUN_8004410c(); FUN_80017160(); sfx from target lhu+6:
 *     [0-63]=64, [64-95]=63, [96-127]=62, [128-159]=61, [160+]=64
 *   FUN_8004483c(s3, bank, sfx, obj+72); FUN_800205f8(obj); return -1
 */

/* --- SLUS_005.10 FUN_80032f7c (LAB_8003302c) manual disassembly reference ---
 *
 * s1 = obj.
 * Event dispatch: event==3→8003323c; event==4→8003326c; event==0→80032fc8;
 *                 else → j 80032a8c (shared ret-0)
 *
 * Event 0 tick:
 *   X spring: lh+20(s1)*15258 rtz>>12 = tgt_vx; rtz_sra4(tgt_vx-vel+136); clamp±256; vel+136+=delta; pos+72+=vel+136
 *   Vert spring: lh+32(s1)*15258 rtz>>12 = tgt_vy; rtz_sra4(tgt_vy-vel+144); clamp±256; vel+144+=delta; pos+76+=vel+144
 *   FUN_80025400(pos.x,pos.z) vs pos.y:
 *     if underground: FUN_8004410c→voice; FUN_8004483c(voice,bank,55,&pos);
 *                     FUN_8003fd24(&pos,12); FUN_800205f8(obj[+132]); return -1
 *   else: proxy[+72/76/80]=pos; lhu+148 -- dec; if 0: voice,sfx55,debris26,free
 *   every 4 frames (DAT_80065310&3==0): FUN_8003fd24(&pos,18); FUN_8001d708(spark)
 *   FUN_800439b8(obj); every 32 frames: FUN_8004c934(obj)
 *
 * Event 3 (entry 8003323c):
 *   lw v0,0(a2)=target; if target[4]==7: return 0
 *   FUN_80031454(obj, a2, 26, 55)
 *
 * Event 4 (entry 8003326c):
 *   lw v0,132(s1); lw v1,0(v0); v1 &= ~0x800000; sw v1,0(v0)
 */

/* --- SLUS_005.10 FUN_80033648 (LAB_800336fc) manual disassembly reference ---
 *
 * a0=target_vehicle (s3), a1=proj_self (s1), a2=kind
 * FUN_80031300(s1, s3, kind, 152=0x98, 0) → s0=child
 * spring_vec[0]=lh+18(s0)<<5; [1]=lh+24(s0)<<5; [2]=lh+30(s0)<<5  (at sp+24)
 * child[0]=0x800084; child[+100]=FUN_80031e34; FUN_800202f4(child)
 * base_vel = rtz_sra(s1[+128], 7)
 * child[+136]=base_vel - rtz_sra12(lh+18(s0)*15258)
 * child[+140]=base_vel - rtz_sra12(lh+24(s0)*15258)
 * child[+144]=base_vel - rtz_sra12(lh+30(s0)*15258)
 * vel_src = s1[+228]; if 0 → vel_src=s1
 * FUN_800176f8(vel_src, &spring_vec, child+72)
 * child[+132]=vel_src
 * lhu+12(s3)--; if 0: FUN_8002cb7c(s3)
 * return child
 */
#endif /* GHIDRA REF */
