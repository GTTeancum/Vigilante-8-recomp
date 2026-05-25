/* split_child_wtype_ticks.c -- per-wtype split-child tick/spawn callbacks.
 *
 * Source: SLUS_005.10
 *
 *  FUN_80031e34 / LAB_80031fa0  -- wtype-2 child tick (bouncing debris)
 *  FUN_80032f7c / LAB_8003302c  -- wtype-4 child tick (spring-guided projectile)
 *  FUN_80033648 / LAB_800336fc  -- wtype-6 child spawn helper (spring launcher)
 *
 * These are invoked by split_child_tick.c event-3 dispatch for the
 * respective weapon types.  LAB_80031fa0 and LAB_8003302c are installed
 * as per-tick obj[+0x64] callbacks on child objects.  LAB_800336fc is a
 * one-shot spawn helper whose address is also stored as the wtype-6
 * callback; it spawns a wtype-2 child, applies impulse, and returns.
 *
 * MED confidence: direct MIPS disassembly port.
 * NOTE: event-0 SFX uses PSX SPU direct-register writes in the original
 * (skipped on host; voice alloc + rand still called to advance state).
 * Collision-path FUN_8004483c bank arg uses DAT_80065BB8 (MED -- the
 * parent split-child tick stores its bank there on spawn, unverified).
 */

#include <stdint.h>

/* -- External helpers -- */

/* FUN_80025400 -- Terrain_HeightAt: return terrain Y at world (X, Z). */
extern int FUN_80025400(int x, int z);

/* FUN_8001d708 -- Object_InitBoneMatrix: initialise/reset bone transform. */
extern void FUN_8001d708(int obj);

/* FUN_8003fd24 -- Debris_Spawn: spawn impact particle, kind=arg2. Returns int handle. */
extern int FUN_8003fd24(const int32_t *xyz, int kind);

/* FUN_8004410c -- Audio_AllocVoice: allocate a free SPU voice channel. */
extern int FUN_8004410c(void);

/* FUN_8004483c -- Audio_PlaySfx3D: play SFX on voice at 3D position. */
extern int FUN_8004483c(int voice, int bank, int sfx, const int32_t *pos);

/* FUN_800205f8 -- Damage_Apply / Object_Retire: retire object from scene. */
extern void FUN_800205f8(int obj);

/* FUN_80030c08 -- Object_GeneralTick: run general rolling/physics tick. */
extern void FUN_80030c08(uint32_t *obj);

/* FUN_80031454 -- WeaponHit_Apply: apply impulse + damage to collider.
 *   Returns -1 (event consumed) or 0 (pass-through). */
extern int FUN_80031454(int param_1, int *param_2, uint16_t param_3, int param_4);

/* FUN_80017160 -- V8_RandNext: advance LCG, return next random uint32. */
extern uint32_t FUN_80017160(void);

/* FUN_800439b8 -- RotMatrix_ApplyAngVel: apply angular velocity to rotation matrix. */
extern void FUN_800439b8(uint32_t *obj);

/* FUN_8004c934 -- MatrixNormal / smoke emitter. */
extern void FUN_8004c934(uint32_t *obj);

/* FUN_800202f4 -- Object_RegisterInScene: register new object in the world. */
extern void FUN_800202f4(int obj);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* FUN_80031300 -- Object_SpawnAttached: spawn a bone child object.
 *   (parent, subBin, kind, kindB=0x98, flags) -> child object handle */
extern int FUN_80031300(int parent, int subBin, int kind, uint32_t kindB, int flags);

/* FUN_800176f8 -- Object_ApplyImpulseAtPoint: add impulse vec to linear+angular vel. */
extern void FUN_800176f8(int obj, const int32_t *vec, const int32_t *pos);

/* FUN_8002cb7c -- WeaponSlot_Deactivate: deactivate/despawn weapon slot on vehicle. */
extern void FUN_8002cb7c(int vehicle);

/* Globals */
extern int32_t DAT_80065310; /* frame tick counter (low word) */
/* DAT_80065BB8: shared sound-bank handle written by split_child_tick event-1 spawn.
 * This is the most likely candidate for the 'bank' arg to FUN_8004483c in the
 * child tick SFX paths.  FIXME: unverified (original uses PSX SPU direct-writes
 * in some paths; FUN_8004483c is only called in event-3 collision paths). */
extern int32_t DAT_80065BB8;

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
int LAB_80031fa0(int obj, int event, int param3)
{
    uint8_t  *s0  = (uint8_t  *)(uintptr_t)(uint32_t)obj;
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
            FUN_80030c08((uint32_t *)(uintptr_t)(uint32_t)obj);
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
                FUN_80030c08((uint32_t *)(uintptr_t)(uint32_t)obj);
        }
        return 0;
    }

    if (event == 3) {
        /* Collision: dereference param3 to get target object. */
        int       target_h = *(int *)(uintptr_t)(uint32_t)param3;
        uint8_t  *tgt      = (uint8_t *)(uintptr_t)(uint32_t)target_h;

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
int LAB_8003302c(int obj, int event, int param3)
{
    uint8_t  *s1  = (uint8_t  *)(uintptr_t)(uint32_t)obj;
    int32_t  *pos = (int32_t  *)(s1 + 72);

    if (event == 3) {
        /* Collision: load target, skip if type == 7, else apply hit. */
        int       t_h = *(int *)(uintptr_t)(uint32_t)param3;
        uint8_t  *tgt = (uint8_t *)(uintptr_t)(uint32_t)t_h;
        if (tgt[4] == 7)
            return 0;
        return FUN_80031454(obj, (int *)(uintptr_t)(uint32_t)param3, 26u, 55);
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
    FUN_800439b8((uint32_t *)(uintptr_t)(uint32_t)obj);

    /* Every 32 frames: smoke effect. */
    if ((DAT_80065310 & 31) == 0)
        FUN_8004c934((uint32_t *)(uintptr_t)(uint32_t)obj);

    return 0;
}

/* ------------------------------------------------------------------ */
/*
 * LAB_800336fc / FUN_80033648 -- wtype-6 child spawn helper.
 *
 * Called from split_child_tick.c event-3 dispatch as a one-shot handler.
 * Prototype matches (target_vehicle, proj_self, kind).
 *
 * Spawns a wtype-2 child attached to target_vehicle via FUN_80031300,
 * initialises spring velocities, applies an impulse, stores the proxy
 * reference, and decrements the vehicle's ammo counter.
 *
 * Arguments:
 *   target_vehicle (a0) : vehicle that the split child has hit
 *   proj_self      (a1) : the split child projectile (self)
 *   kind           (a2) : weapon kind (passed through to child spawner)
 *
 * Returns: child object handle.
 *
 * Note: FUN_80031300 is called with swapped args (a0↔a1), i.e.
 *   FUN_80031300(proj_self, target_vehicle, kind, 0x98, 0).
 */
int LAB_800336fc(int target_vehicle, int proj_self, int kind)
{
    uint8_t *vs = (uint8_t *)(uintptr_t)(uint32_t)target_vehicle;
    uint8_t *ps = (uint8_t *)(uintptr_t)(uint32_t)proj_self;

    /* Spawn bone child; note swapped parent/subBin relative to call site. */
    int      child = FUN_80031300(proj_self, target_vehicle, kind, 0x98u, 0);
    uint8_t *cs    = (uint8_t *)(uintptr_t)(uint32_t)child;

    /* Read spring offsets from child's bone attachment (int16 at +18, +24, +30). */
    int32_t spring_vec[3];
    spring_vec[0] = mips_sll_i32((int16_t)*(int16_t *)(cs + 18), 5);
    spring_vec[1] = mips_sll_i32((int16_t)*(int16_t *)(cs + 24), 5);
    spring_vec[2] = mips_sll_i32((int16_t)*(int16_t *)(cs + 30), 5);

    /* Set child flags and install wtype-2 tick callback. */
    *(int32_t *)cs         = 0x800084;
    Object_SetCallbackPsxSlot(cs, (uintptr_t)&LAB_80031fa0);
    FUN_800202f4(child);

    /* Compute initial spring velocities from proj_self ammo/base value (+128). */
    {
        int32_t base_vel = rtz_sra(*(int32_t *)(ps + 128), 7);
        int16_t sv0 = *(int16_t *)(cs + 18);
        int16_t sv1 = *(int16_t *)(cs + 24);
        int16_t sv2 = *(int16_t *)(cs + 30);
        *(int32_t *)(cs + 136) = mips_subu_i32(base_vel, rtz_sra(mips_mult_lo_i32(sv0, 15258), 12));
        *(int32_t *)(cs + 140) = mips_subu_i32(base_vel, rtz_sra(mips_mult_lo_i32(sv1, 15258), 12));
        *(int32_t *)(cs + 144) = mips_subu_i32(base_vel, rtz_sra(mips_mult_lo_i32(sv2, 15258), 12));
    }

    /* Determine velocity source: proj_self[+228] if non-zero, else proj_self. */
    int vel_src = *(int32_t *)(ps + 228);
    if (vel_src == 0)
        vel_src = proj_self;

    /* Apply spring impulse to velocity source at child's world position. */
    FUN_800176f8(vel_src, spring_vec, (const int32_t *)(cs + 72));

    /* Store velocity source as proxy reference for child's rolling tick. */
    *(int32_t *)(cs + 132) = vel_src;

    /* Decrement vehicle ammo (uint16 at +12); deactivate weapon if empty. */
    {
        uint16_t ammo = *(uint16_t *)(vs + 12);
        if (--ammo == 0) {
            *(uint16_t *)(vs + 12) = 0;
            FUN_8002cb7c(target_vehicle);
        } else {
            *(uint16_t *)(vs + 12) = ammo;
        }
    }

    return child;
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
