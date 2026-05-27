/* homing_proj_tick.c -- homing projectile tick (LAB_80033290).
 *
 * Source: SLUS_005.10  LAB_80033290  0x80033290  (~90 instructions).
 *
 * Tick callback for homing/tracking projectiles (e.g. Blimp Bomb wtype-6).
 * obj[8] (int8) selects mode:
 *   0 -- unarmed: WeaponHit_Apply on collision.
 *   1 -- armed:   full explosion on collision.
 *
 * Event dispatch:
 *   0  -> integrate velocity; apply gravity (vel_y += 56); terrain check.
 *          Returns -1 on terrain impact, 0 otherwise.
 *   3  -> collision:
 *          obj[8]==0: WeaponHit_Apply(obj, param3, 26, 57).
 *          obj[8]!=0: if collider_type==3 (terrain) return 0; else
 *          spawn explosion debris, play sound, optionally apply impulse
 *          to vehicle collider, retire bullet.  Return -1.
 *   other -> return 0.
 *
 * Object field layout:
 *   +0x00 (0)   : flags / type word
 *   +0x08 (8)   : armed-state byte (int8; 0=WeaponHit mode, else=explode mode)
 *   +0x24 (36)  : render pos X
 *   +0x28 (40)  : render pos Y
 *   +0x2c (44)  : render pos Z
 *   +0x48 (72)  : world pos X (int32, 16.16 fixed)
 *   +0x4c (76)  : world pos Y
 *   +0x50 (80)  : world pos Z
 *   +0x88 (136) : velocity X
 *   +0x8c (140) : velocity Y (gravity added each tick)
 *   +0x90 (144) : velocity Z
 *
 * HIGH confidence: direct MIPS disassembly port.
 */
#include <stdint.h>

/* FUN_80025400 -- Terrain_HeightAt. */
extern int FUN_80025400(int x, int z);

/* FUN_8003fd24 -- Debris_Spawn: spawn impact/explosion particle. */
extern intptr_t FUN_8003fd24(const int32_t *xyz, int kind);

/* FUN_8004410c -- Audio_AllocVoice. */
extern int FUN_8004410c(void);

/* FUN_8004483c -- Audio_VoicePlay: (voice, bank, pitch, pos). */
extern void FUN_8004483c(int voice, uint32_t bank, int pitch, const void *pos);

/* FUN_80020620 -- ColEvent_Dispatch. */
extern void FUN_80020620(intptr_t obj, uint32_t event);

/* FUN_800205f8 -- Object_RetireDeferred. */
extern void FUN_800205f8(intptr_t obj);

/* FUN_80031454 -- WeaponHit_Apply. */
extern int FUN_80031454(intptr_t obj, intptr_t *collider, uint16_t radius, int strength);

/* FUN_8001ac44 -- BoneObj_BuildTree: allocate and build a bone-tree object. */
extern int FUN_8001ac44(int *bank, uint16_t slot, uint32_t size, uint32_t flags);

/* FUN_8002036c -- Object_PostUpdate2: register object in active list. */
extern uint32_t FUN_8002036c(void *obj);

/* FUN_800176f8 -- Object_ApplyImpulseFrom: apply impulse vector at position. */
extern void FUN_800176f8(intptr_t obj, int32_t *vec, int32_t *pos);

/* FUN_80012068 -- HudFlash_SetEntry: trigger HUD damage flash. */
extern void FUN_80012068(int idx, uint8_t b5, uint8_t b6, uint8_t b7);

/* LAB_8003e80c -- explosion debris main tick (in effect_death_ticks.c). */
extern intptr_t LAB_8003e80c(intptr_t obj, int event, intptr_t param3);
/* LAB_8003e7b4 -- explosion debris child tick (in effect_death_ticks.c). */
extern intptr_t LAB_8003e7b4(intptr_t obj, int event, intptr_t param3);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
extern uintptr_t Collision_QueryHostWord(const void *query, uint32_t index);

/* DAT_800658FC -- sound bank (gp+1528). */
extern uint32_t DAT_800658FC;

/* DAT_800737d8 -- bone bank pool handle. */
extern uintptr_t DAT_800737d8;

/* ------------------------------------------------------------------ */

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

intptr_t LAB_80033290(intptr_t obj, int event, intptr_t param3)
{
    uint8_t *s1 = (uint8_t *)(uintptr_t)obj;
    /* s3 = param3 (collider descriptor pointer on event 3) */

    if (event == 0) {
        /* ----- Tick ----- */
        /* Integrate velocity into position. */
        int32_t *pos = (int32_t *)(s1 + 0x48);
        int32_t *vel = (int32_t *)(s1 + 0x88);
        pos[0] = mips_addu_i32(pos[0], vel[0]);
        pos[1] = mips_addu_i32(pos[1], vel[1]);
        pos[2] = mips_addu_i32(pos[2], vel[2]);

        /* Mirror to render position. */
        *(int32_t *)(s1 + 0x24) = pos[0];
        *(int32_t *)(s1 + 0x28) = pos[1];
        *(int32_t *)(s1 + 0x2c) = pos[2];

        /* Apply gravity to Y velocity. */
        vel[1] = mips_addu_i32(vel[1], 56);

        /* Terrain collision check. */
        int terrain_y = FUN_80025400(pos[0], pos[2]);
        if (!(terrain_y < pos[1])) {
            /* At or below terrain: impact. */
            FUN_8003fd24((const int32_t *)pos, 0);

            int voice = FUN_8004410c();
            FUN_8004483c(voice, DAT_800658FC, 57, pos);
            FUN_80020620((uint32_t)obj, 1u);
            return -1;
        }
        return 0;
    }

    if (event == 3) {
        /* ----- Collision ----- */
        int8_t armed = *(int8_t *)(s1 + 8);

        if (armed == 0) {
            /* Unarmed: standard hit. */
            return FUN_80031454(obj, (intptr_t *)(uintptr_t)param3,
                                26u, 57);
        }

        /* Armed: full explosion. */
        uint8_t *s0_collider = (uint8_t *)Collision_QueryHostWord((void *)(uintptr_t)param3, 0);
        uint8_t collider_type = s0_collider[4];

        /* Terrain: no-op. */
        if (collider_type == (uint8_t)event)
            return 0;

        /* Spawn explosion debris object. */
        intptr_t new_obj = (intptr_t)(uint32_t)FUN_8001ac44((int *)(uintptr_t)DAT_800737d8,
                                                            43u, 128u, 8u);
        uint8_t *nobj = (uint8_t *)(uintptr_t)new_obj;
        *(int32_t *)nobj = 52;
        /* Copy position from self. */
        *(int32_t *)(nobj + 0x48) = *(int32_t *)(s1 + 0x48);
        *(int32_t *)(nobj + 0x4c) = *(int32_t *)(s1 + 0x4c);
        *(int32_t *)(nobj + 0x50) = *(int32_t *)(s1 + 0x50);
        /* Install tick callbacks. */
        intptr_t child = (intptr_t)(uint32_t)*(uint32_t *)(nobj + 0x38);
        Object_SetCallbackPsxSlot((void *)(uintptr_t)nobj, (uintptr_t)&LAB_8003e80c);
        if (child)
            Object_SetCallbackPsxSlot((void *)(uintptr_t)child, (uintptr_t)&LAB_8003e7b4);
        FUN_8002036c((uint32_t *)nobj);

        /* Spawn explosion visual. */
        int32_t *pos = (int32_t *)(s1 + 0x48);
        FUN_8003fd24((const int32_t *)pos, 22);

        /* Play explosion sound. */
        int32_t *render_pos = (int32_t *)(s1 + 0x24);
        int voice = FUN_8004410c();
        FUN_8004483c(voice, DAT_800658FC, 55, render_pos);

        /* Apply impulse to vehicle collider if applicable. */
        if (collider_type == 2) {
            int32_t impulse[3];
            int32_t *vel = (int32_t *)(s1 + 0x88);
            impulse[0] = mips_mult_lo_i32(vel[0], 96);
            impulse[1] = mips_mult_lo_i32(vel[1], 96);
            impulse[2] = mips_mult_lo_i32(vel[2], 96);

            intptr_t collider_obj = (intptr_t)Collision_QueryHostWord((void *)(uintptr_t)param3, 0);
            FUN_800176f8(collider_obj, impulse, render_pos);

            /* Damage flash for negative HP targets. */
            uint8_t *cobj = s0_collider;
            *(int32_t *)(cobj + 0x84) = (int32_t)0xFFFB8800u;
            *(int32_t *)(cobj + 0x98) = (int32_t)((uint32_t)*(int32_t *)(cobj + 0x98) << 4);
            *(int32_t *)(cobj + 0x90) = (int32_t)((uint32_t)*(int32_t *)(cobj + 0x90) << 4);
            int16_t hp = *(int16_t *)(cobj + 6);
            if (hp < 0) {
                FUN_80012068((int)(uint8_t)(~hp), 255u, 8u, 32u);
            }
        }

        FUN_800205f8(obj);
        return -1;
    }

    return 0;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0
/* --- SLUS_005.10  LAB_80033290  (manual disassembly reference) ---
 *
 * 80033290: addiu sp,sp,-56
 * 80033298: addu s1,a0,zero       ; s1 = obj
 * 800332a0: addu s3,a2,zero       ; s3 = param3
 * 800332ac: beq a1,zero,0x800332c8 ; event==0 -> tick
 * 800332b8: beq a1,3,0x80033380  ; event==3 -> collision
 * 800332c0: j return0             ; else return 0
 *
 * -- event 0 (tick) --
 *   pos_{xyz} += vel_{xyz}; render_pos = pos; vel_y += 56
 *   terrain_y = Terrain_HeightAt(x,z)
 *   if terrain_y >= pos_y: FUN_8003fd24(&pos,0)+sound(57)+FUN_80020620+return -1
 *   return 0
 *
 * -- event 3 (collision) --
 *   lb v0,8(s1) ; armed = obj[8]
 *   if armed==0: WeaponHit_Apply(obj, param3, 26, 57)
 *   else:
 *     s0 = *param3 = collider
 *     if collider_type==3: return 0
 *     new_obj = BoneObj_BuildTree(DAT_800737d8, 43, 128, 8)
 *     new_obj[0]=52; copy pos; install LAB_8003e80c/LAB_8003e7b4; PostUpdate2
 *     FUN_8003fd24(&pos, 22); SoundAlloc+play(55)
 *     if collider_type==2:
 *       impulse = vel*96; FUN_800176f8(collider, &impulse, &render_pos)
 *       set collider[0x84]; scale [0x90],[0x98]
 *       if HP<0: HudFlash_SetEntry
 *     FUN_800205f8(obj); return -1
 */
#endif /* GHIDRA REF */
