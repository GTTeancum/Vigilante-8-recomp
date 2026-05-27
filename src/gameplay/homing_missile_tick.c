/* homing_missile_tick.c -- homing/guided missile projectile tick (LAB_8003403c).
 *
 * Source: SLUS_005.10  LAB_8003403c  0x8003403c  (~180 instructions).
 *
 * Tick callback for the guided/homing missile (wtype-7 or similar).
 * Manages a particle exhaust trail, target-tracking physics, terrain
 * impact, and three armed-state explosion paths on collision.
 *
 * obj[8] (int8) selects armed state:
 *   <0 -- inert: terrain/collision both ignored (return 0).
 *    0 -- unarmed: WeaponHit_Apply on collision.
 *    1 -- armed:   full explosion on collision (BoneObj + debris).
 *    2 -- rocket:  FUN_8003fdcc trail + optional WeaponHit on vehicle.
 *   >=3 -- armed:  target-dequeue only (no damage).
 *
 * Event dispatch:
 *   0  -> spawn rocket exhaust trail while phase-counter < 20;
 *          integrate velocity; apply gravity (vel_y += 56);
 *          steer toward target (obj[0x84]) if locked;
 *          update rotation matrix; terrain check.
 *          Returns -1 on terrain impact, 0 otherwise.
 *   3  -> collision: dispatch on armed state (see above).
 *          Returns -1 (consumed), 0 (terrain / no-op), or
 *          WeaponHit_Apply result.
 *   9  -> target lost: if obj[0x84]==param3, set obj[0x84]=obj[0x80].
 *          Returns 0.
 *   other -> return 0.
 *
 * Object field layout:
 *   +0x00 (0)    : flags (int32; bit 0x20 cleared when missile goes upward)
 *   +0x05 (5)    : player / channel index (int8)
 *   +0x06 (6)    : HP word cleared on upward boost (sh zero,6)
 *   +0x08 (8)    : armed-state byte (int8)
 *   +0x0a (10)   : weapon-type / trail-slot (uint16)
 *   +0x0c (12)   : effect ID (uint16); shifted <<7 or <<8 for FUN_80033db4
 *   +0x24 (36)   : render pos X
 *   +0x28 (40)   : render pos Y
 *   +0x2c (44)   : render pos Z
 *   +0x38 (56)   : trail particle chain head (int32 ptr)
 *   +0x48 (72)   : world pos X (int32, 16.16 fixed)
 *   +0x4c (76)   : world pos Y
 *   +0x50 (80)   : world pos Z
 *   +0x64 (100)  : tick callback
 *   +0x80 (128)  : previous target ptr
 *   +0x84 (132)  : current target ptr
 *   +0x88 (136)  : velocity X
 *   +0x8c (140)  : velocity Y (gravity +56/tick; negative = ascending)
 *   +0x90 (144)  : velocity Z
 *   +0x94 (148)  : phase counter (int16; counts 0→20 during trail spawn)
 *
 * Particle-chain node layout (same object struct; linked via [0x34]):
 *   +0x24 (36)   : direction / delta vector X (written each tick)
 *   +0x28 (40)   : direction / delta vector Y
 *   +0x2c (44)   : direction / delta vector Z
 *   +0x34 (52)   : next-node pointer (int32)
 *
 * HIGH confidence: direct MIPS disassembly port.
 */
#include <stdint.h>

/* FUN_8001ac44 -- BoneObj_BuildTree: allocate and build a bone-tree object. */
extern int FUN_8001ac44(int *bank, uint16_t slot, uint32_t size, uint32_t flags);

/* FUN_8001f9cc -- Object_PreTickRecurse: advance animation state of trail obj. */
extern int FUN_8001f9cc(intptr_t obj, int16_t param2);

/* FUN_8001d544 -- Object_PrependChild: head-insert child into parent child list. */
extern void FUN_8001d544(void *parent, void *child);

/* FUN_8001dc1c -- Object_RecomputeBoundingRadius: rebuild rotation / bounding data. */
extern int FUN_8001dc1c(intptr_t obj);

/* FUN_8001d840 -- Object_AlignToTerrainNormal: probe terrain, build orientation.
 *   param_1: object handle (uint32_t), param_2: pos array ptr, param_3: dest ptr (as int). */
extern void FUN_8001d840(intptr_t param_1, uint32_t *param_2, intptr_t param_3);

/* FUN_8002036c -- Object_PostUpdate2: register object in active list. */
extern uint32_t FUN_8002036c(void *obj);

/* FUN_80020620 -- ColEvent_Dispatch: dispatch a collision event. */
extern void FUN_80020620(intptr_t obj, uint32_t event);

/* FUN_800205f8 -- Object_RetireDeferred: queue object for removal. */
extern void FUN_800205f8(intptr_t obj);

/* FUN_80025400 -- Terrain_HeightAt: return terrain Y at (X, Z). */
extern int FUN_80025400(int x, int z);

/* FUN_80031454 -- WeaponHit_Apply: apply impulse + damage to collider. */
extern int FUN_80031454(intptr_t obj, intptr_t *collider, uint16_t radius, int strength);
extern uintptr_t Collision_QueryHostWord(const void *query, uint32_t index);

/* FUN_80033db4 -- TerrainDamage_Apply: deform terrain in radius. */
extern void FUN_80033db4(int x, int z, int radius, int intensity);

/* FUN_80012068 -- HudFlash_SetEntry: trigger HUD damage flash. */
extern void FUN_80012068(int idx, uint8_t b5, uint8_t b6, uint8_t b7);

/* FUN_800170c8 -- Math_Sqrt64: integer sqrt of a 64-bit value (lo, hi as int). */
extern int FUN_800170c8(uint32_t lo, int hi);

/* FUN_8003fd24 -- Debris_Spawn: spawn impact/explosion particle. */
extern int FUN_8003fd24(const int32_t *xyz, int kind);

/* FUN_8003fdcc -- WeaponSpawn_TrailObject: allocate a typed trail/effect object.
 *   param1: world pos ptr (uint32_t*), param2: bone index (uint16_t), param3: trail seed. */
extern uint32_t *FUN_8003fdcc(uint32_t *param_1, uint16_t param_2, intptr_t param_3);

/* FUN_8003fea8 -- RocketExhaust_Emit: emit rocket-exhaust glow particle. */
extern void FUN_8003fea8(const int32_t *pos, uint32_t flags);

/* FUN_8004410c -- Audio_AllocVoice: allocate a free SPU voice. */
extern int FUN_8004410c(void);

/* FUN_8004483c -- Audio_VoicePlay: (voice, bank, pitch, pos). */
extern void FUN_8004483c(int voice, uint32_t bank, int pitch, const void *pos);

/* FUN_800447e8 -- Audio_PlaySfxChannel: play SFX on a player channel or voice.
 *   param1: player-index (event 0 boost sound) or voice (collision). */
extern void FUN_800447e8(int param1, uint32_t bank, int pitch, const void *pos);

/* LAB_8003e80c -- explosion debris main tick (in effect_death_ticks.c). */
extern intptr_t LAB_8003e80c(intptr_t obj, int event, intptr_t param3);
/* LAB_8003e7b4 -- explosion debris child tick (in effect_death_ticks.c). */
extern intptr_t LAB_8003e7b4(intptr_t obj, int event, intptr_t param3);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* DAT_800658FC -- sound bank (gp+1528). */
extern uint32_t DAT_800658FC;

/* DAT_800659D0 -- trail-animation base halfword (gp+1740, signed). */
extern int16_t DAT_800659D0;

/* DAT_800737d8 -- bone bank pool handle. */
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

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

intptr_t LAB_8003403c(intptr_t obj, int event, intptr_t param3)
{
    uint8_t *s1 = (uint8_t *)(uintptr_t)obj;

    /* ----- Event dispatch ----- */
    if (event == 3)
        goto ev_collision;
    if (event >= 4) {
        if (event == 9)
            goto ev_target_unlock;
        return 0;
    }
    if (event != 0)
        return 0;

    /* ===== Event 0: Tick ===== */
    {
        int32_t *pos = (int32_t *)(s1 + 0x48);
        int32_t *vel = (int32_t *)(s1 + 0x88);

        /* -- Trail spawn phase (first 20 ticks) -- */
        int16_t phase = *(int16_t *)(s1 + 0x94);
        if (phase < 20) {
            /* Allocate a bone-tree trail object. */
            intptr_t trail = (intptr_t)(uint32_t)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8,
                                                              *(uint16_t *)(s1 + 10),
                                                              152u, 8u);
            uint8_t *tobj = (uint8_t *)(uintptr_t)trail;
            *(int32_t *)tobj |= 0x410;

            /* Set animation phase relative to global counter. */
            int16_t base = (int16_t)mips_subu_i32(
                mips_mult_lo_i32((int32_t)DAT_800659D0, 2),
                (int32_t)(uint32_t)*(uint16_t *)(s1 + 70));
            FUN_8001f9cc(trail, base);

            /* Attach trail object as child of self. */
            FUN_8001d544(s1, tobj);

            /* Advance phase counter. */
            *(int16_t *)(s1 + 0x94) = (int16_t)mips_addu_i32((int32_t)phase, 1);
        }

        /* -- Update trail particle chain direction vectors -- */
        {
            uint8_t *node = (uint8_t *)(uintptr_t)(uint32_t)
                            (*(int32_t *)(s1 + 0x38));
            uint8_t *next = (uint8_t *)(uintptr_t)(uint32_t)
                            (*(int32_t *)(node + 0x34));

            if (next != NULL) {
                /* Walk pairwise: node[0x24..] = next.render_pos - vel */
                do {
                    *(int32_t *)(node + 0x24) =
                        mips_subu_i32(*(int32_t *)(next + 0x24), vel[0]);
                    *(int32_t *)(node + 0x28) =
                        mips_subu_i32(*(int32_t *)(next + 0x28), vel[1]);
                    *(int32_t *)(node + 0x2c) =
                        mips_subu_i32(*(int32_t *)(next + 0x2c), vel[2]);
                    node = next;
                    next = (uint8_t *)(uintptr_t)(uint32_t)
                           (*(int32_t *)(node + 0x34));
                } while (next != NULL);
            }
            /* Last (or sole) node gets reversed velocity direction. */
            *(int32_t *)(node + 0x24) = mips_subu_i32(0, vel[0]);
            *(int32_t *)(node + 0x28) = mips_subu_i32(0, vel[1]);
            *(int32_t *)(node + 0x2c) = mips_subu_i32(0, vel[2]);
        }

        /* -- Integrate velocity into position -- */
        pos[0] = mips_addu_i32(pos[0], vel[0]);
        pos[1] = mips_addu_i32(pos[1], vel[1]);
        pos[2] = mips_addu_i32(pos[2], vel[2]);

        /* Mirror to render position. */
        *(int32_t *)(s1 + 0x24) = pos[0];
        *(int32_t *)(s1 + 0x28) = pos[1];
        *(int32_t *)(s1 + 0x2c) = pos[2];

        /* -- Boost sound when ascending (vel_y < 0 before gravity) --
         * sltiu checks: (vel_y + 56) < 56u  ↔  vel_y < 0 (upward). */
        int32_t vy = vel[1];
        if ((uint32_t)mips_addu_i32(vy, 56) < 56u) {
            FUN_800447e8((int)(int8_t)s1[5], DAT_800658FC, 53,
                         (const void *)pos);
            *(int32_t *)s1 &= ~0x20;
            *(int16_t *)(s1 + 6) = 0;
        }

        /* Apply gravity. */
        vel[1] = mips_addu_i32(vy, 56);

        /* -- Homing steering toward target -- */
        uint8_t *target_obj = (uint8_t *)(uintptr_t)(uint32_t)
                              (*(int32_t *)(s1 + 0x84));
        if (target_obj != NULL) {
            /* Delta: target world pos – self world pos. */
            int32_t *tpos = (int32_t *)(target_obj + 0x48);
            int32_t dx = mips_subu_i32(tpos[0], pos[0]);
            int32_t dy = mips_subu_i32(tpos[1], pos[1]);
            int32_t dz = mips_subu_i32(tpos[2], pos[2]);

            /* XZ distance = sqrt(dx^2 + dz^2) using 64-bit accumulator. */
            int64_t dx2 = (int64_t)dx * dx;
            int64_t dz2 = (int64_t)dz * dz;
            uint64_t xz64 = (uint64_t)dx2 + (uint64_t)dz2;
            /* xz_dist = */ FUN_800170c8((uint32_t)xz64, (int)(uint32_t)(xz64 >> 32));

            /* Vertical approach term = sqrt(dy*112 + vel_y^2). */
            int32_t vy_now = vel[1]; /* post-gravity */
            int64_t vy2   = (int64_t)vy_now * vy_now;
            int64_t dy112 = (int64_t)dy * 112;   /* dy*(8-1)*16 = dy*112 */
            uint64_t yterm64 = (uint64_t)(dy112 + vy2);
            int dist_y = FUN_800170c8((uint32_t)yterm64,
                                      (int)(uint32_t)(yterm64 >> 32));

            /* Steering correction: steer_delta = dist_y - vel_y */
            int32_t steer = mips_subu_i32(dist_y, vy_now);
            if (steer != 0) {
                /* Steer X: vel_x += (dx*56/steer - vel_x) >> 5 (rounded) */
                int32_t target_vx = (int32_t)((int64_t)dx * 56 / steer);
                int32_t corr_x = mips_subu_i32(target_vx, vel[0]);
                if (corr_x < 0) corr_x = mips_addu_i32(corr_x, 31);
                vel[0] = mips_addu_i32(vel[0], corr_x >> 5);

                /* Steer Z: vel_z += (dz*56/steer - vel_z) >> 5 (rounded) */
                int32_t target_vz = (int32_t)((int64_t)dz * 56 / steer);
                int32_t corr_z = mips_subu_i32(target_vz, vel[2]);
                if (corr_z < 0) corr_z = mips_addu_i32(corr_z, 31);
                vel[2] = mips_addu_i32(vel[2], corr_z >> 5);
            }
        }

        /* Rebuild bounding/rotation data. */
        FUN_8001dc1c(obj);

        /* -- Terrain collision check -- */
        int terrain_y = FUN_80025400(pos[0], pos[2]);
        if (!(terrain_y < pos[1]))
            return 0; /* above terrain: normal tick */

        /* Below terrain: impact — dispatch on armed state. */
        int8_t state = *(int8_t *)(s1 + 8);
        if (state < 0)
            return 0;

        if (state < 2) {
            /* State 0 or 1: dust particle + sound + dispatch retire. */
            FUN_8003fd24((const int32_t *)pos, 0);
            FUN_8004483c((int)(int8_t)s1[5], DAT_800658FC, 57,
                         (const void *)pos);
            FUN_80020620((uint32_t)obj, 1u);
            return -1;
        }

        if (state == 2) {
            /* State 2: exhaust + terrain deformation + retire. */
            uint16_t eid = *(uint16_t *)(s1 + 12);
            uint32_t *eff = FUN_8003fdcc((uint32_t *)pos, 39u, (int)eid);
            *eff |= 0x10u;
            FUN_8004483c((int)(int8_t)s1[5], DAT_800658FC, 57,
                         (const void *)pos);
            FUN_80033db4(pos[0], pos[2], 0x20000, (int)eid << 7);
            FUN_8003fd24((const int32_t *)pos, 0);
            FUN_8003fea8((const int32_t *)pos, 0x08000080u);
            FUN_80020620((uint32_t)obj, 1u);
            return -1;
        }

        /* state >= 3 or unmapped: no terrain response. */
        return 0;
    }

    /* ===== Event 3: Collision ===== */
ev_collision:
    {
        uint8_t *collider = (uint8_t *)Collision_QueryHostWord((void *)(uintptr_t)param3, 0);
        uint8_t collider_type = collider[4];

        /* Terrain collider: ignore. */
        if (collider_type == 3)
            return 0;

        int8_t armed = *(int8_t *)(s1 + 8);
        if (armed < 0)
            return 0;

        int32_t *pos = (int32_t *)(s1 + 0x48);

        if (armed == 1) {
            /* ----- Full explosion ----- */
            intptr_t new_obj_h = (intptr_t)(uint32_t)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8,
                                                                  43u, 128u, 8u);
            uint8_t *nobj = (uint8_t *)(uintptr_t)new_obj_h;
            *(int32_t *)nobj = 52;

            /* Copy world position from self. */
            *(int32_t *)(nobj + 0x48) = pos[0];
            *(int32_t *)(nobj + 0x4c) = pos[1];
            *(int32_t *)(nobj + 0x50) = pos[2];

            /* Install tick callbacks. */
            intptr_t child = (intptr_t)(uint32_t)*(uint32_t *)(nobj + 0x38);
            Object_SetCallbackPsxSlot((void *)(uintptr_t)nobj, (uintptr_t)&LAB_8003e80c);
            if (child)
                Object_SetCallbackPsxSlot((void *)(uintptr_t)child, (uintptr_t)&LAB_8003e7b4);
            FUN_8002036c((uint32_t *)nobj);

            /* Explosion visual + sound. */
            FUN_8003fd24((const int32_t *)pos, 22);
            int voice = FUN_8004410c();
            FUN_8004483c(voice, DAT_800658FC, 55,
                         (const void *)(s1 + 0x24));

            /* Vehicle-specific damage. */
            if (collider_type == 2) {
                *(int32_t *)(collider + 0x84) = (int32_t)0xFFF70F80u;
                *(int32_t *)(collider + 0x98) = 0xCB20; /* always (delay slot) */
                int16_t hp = *(int16_t *)(collider + 6);
                if (hp < 0) {
                    FUN_80012068((int)(~hp), 255u, 2u, 128u);
                }
            }

            FUN_800205f8(obj);
            return -1;
        }

        if (armed == 2) {
            /* ----- Rocket impact ----- */
            int32_t *rpos = (int32_t *)(s1 + 0x48); /* s2 = s1+72 */
            FUN_8003fea8((const int32_t *)rpos, 0x08000080u);

            if (collider_type == 2 &&
                (*(int32_t *)collider & 0x100000)) {
                /* Vehicle with special flag: spawn trail effect + WeaponHit. */
                uint32_t *tep = FUN_8003fdcc((uint32_t *)rpos, 39u, 0);
                FUN_8001d840((intptr_t)tep,
                             (uint32_t *)((uint8_t *)tep + 0x48),
                             (intptr_t)((uint8_t *)tep + 0x10));
                *tep |= 0x10u;
                FUN_80033db4(pos[0], pos[2], 0x20000,
                             (int)*(uint16_t *)(s1 + 12) << 8);
                /* Fall through to WeaponHit. */
            } else {
                /* Non-vehicle or no special flag: exhaust effect + retire. */
                FUN_8003fdcc((uint32_t *)rpos, 48u, 200);
                int voice2 = FUN_8004410c();
                FUN_800447e8(voice2, DAT_800658FC, 57,
                             (const void *)rpos);
                FUN_800205f8(obj);
                return -1;
            }
            /* Shared armed==2 weapon-hit path. */
            return FUN_80031454(obj, (intptr_t *)(uintptr_t)param3,
                                26u, 57);
        }

        if (armed == 0) {
            /* ----- Unarmed: standard hit ----- */
            return FUN_80031454(obj, (intptr_t *)(uintptr_t)param3,
                                26u, 57);
        }

        /* armed >= 3: fall through to target-dequeue. */
    }

    /* ===== Event 9: Target unlock ===== */
ev_target_unlock:
    {
        if ((intptr_t)*(int32_t *)(s1 + 0x84) == param3)
            *(int32_t *)(s1 + 0x84) = *(int32_t *)(s1 + 0x80);
        return 0;
    }
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0
/* --- SLUS_005.10  LAB_8003403c  (manual disassembly reference) ---
 *
 * 8003403c: addiu sp,sp,-64
 * 80034044: addu s1,a0,zero       ; s1 = obj
 * 8003404c: addu s3,a2,zero       ; s3 = param3
 * 8003405c: beq a1,3,0x80034490  ; event==3 → collision
 * 80034064: sltiu v0,a1,4
 * 80034068: beq v0,zero,0x80034080 ; a1>=4
 * 80034070: beq a1,zero,0x80034090 ; event==0 → tick
 * 80034078: j 0x800346b0           ; else return 0
 * 80034080: beq a1,9,0x80034690   ; event==9 → unlock
 * 80034088: j 0x800346b0           ; else return 0
 *
 * -- event 0 (0x80034090) --
 *   lh v0,148(s1)                 ; phase = obj[0x94]
 *   slti v0,v0,20 → if phase<20 spawn trail:
 *     BoneObj_BuildTree(DAT_800737d8, obj[10], 152, 8) → trail
 *     trail[0] |= 0x410
 *     FUN_8001f9cc(trail, DAT_800659D0*2 - obj[70])
 *     FUN_8001d544(obj, trail)
 *     obj[0x94]++
 *   Walk obj[0x38] chain via [0x34]: each node[0x24..] = next.render-vel
 *   Last node[0x24..] = -vel
 *   pos += vel; render = pos
 *   if (vel_y+56)<56u: FUN_800447e8(obj[5],bank,53,pos);
 *                       obj[0]&=~0x20; obj[6]=0
 *   vel_y += 56 (gravity)
 *   if (target=obj[0x84]) != 0:
 *     dx/dy/dz = target.pos - self.pos
 *     xz_dist = FUN_800170c8(dx^2+dz^2)
 *     dist_y_term = FUN_800170c8(dy*112 + vel_y^2)
 *     steer = dist_y_term - vel_y
 *     if steer!=0: vel_x += (dx*56/steer - vel_x)>>5 (rounded)
 *                  vel_z += (dz*56/steer - vel_z)>>5 (rounded)
 *   FUN_8001dc1c(obj)
 *   terrain_y = FUN_80025400(pos_x, pos_z)
 *   if terrain < pos_y:
 *     state = obj[8]
 *     if state<0: return 0
 *     if state<2: Debris_Spawn(&pos,0) + sound(57) + dispatch(1) + return -1
 *     if state==2: FUN_8003fdcc(&pos,39,eid)|=0x10 + sound(57)
 *                  + FUN_80033db4(x,z,0x20000,eid<<7)
 *                  + Debris_Spawn(&pos,0) + FUN_8003fea8(&pos,0x8000080)
 *                  + dispatch(1) + return -1
 *
 * -- event 3 (0x80034490) --
 *   collider = *param3; if type==3 return 0
 *   armed = obj[8]
 *   if armed<0: return 0
 *   if armed==1: BoneObj_BuildTree(737d8,43,128,8)+new[0]=52+copy pos
 *                install LAB_8003e80c/LAB_8003e7b4; PostUpdate2
 *                Debris_Spawn(&pos,22); sound(55,render_pos)
 *                if type==2: col[0x84]=0xFFF70F80; col[0x98]=0xCB20(delay)
 *                            if HP<0: HudFlash(~HP,255,2,128)
 *                RetireDeferred; return -1
 *   if armed==2: FUN_8003fea8(&pos,0x8000080)
 *                if type==2 && flags&0x100000:
 *                  FUN_8003fdcc(&pos,39,0)→eff; FUN_8001d840(eff,&eff[72],&eff[16])
 *                  eff[0]|=0x10; FUN_80033db4(x,z,0x20000,eid<<8)
 *                  → WeaponHit_Apply(obj,param3,26,57)
 *                else: FUN_8003fdcc(&pos,48,200); sound(57); RetireDeferred; return -1
 *   if armed==0: WeaponHit_Apply(obj, param3, 26, 57)
 *   armed>=3: fall to event-9 path
 *
 * -- event 9 (0x80034690) --
 *   if obj[0x84]==param3: obj[0x84]=obj[0x80]
 *   return 0
 */
#endif /* GHIDRA REF */
