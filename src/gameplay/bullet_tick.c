/* bullet_tick.c -- standard bullet projectile tick (LAB_80031634).
 *
 * Source: SLUS_005.10  LAB_80031634  0x80031634  (~58 instructions).
 *
 * Installed at +0x64 of bullet objects by the bullet spawner.
 *
 * Event dispatch:
 *   0  -> integrate velocity into position; if bullet is below terrain
 *          (Terrain_HeightAt < pos_y) fire terrain-impact effect and retire.
 *          Decrement lifetime each tick; retire when it reaches zero.
 *          Returns -1 on retire/impact, 0 otherwise.
 *   3  -> collision: if collider is terrain (type 3) return 0; otherwise
 *          fire impact effect, play a ricochet sound with pitch chosen by
 *          collider HP, then retire the bullet.
 *          Returns -1 (consumed) or 0 (terrain collider only).
 *   other -> return 0.
 *
 * Object field layout:
 *   +0x24 (36)  : render pos X (mirrored from physics pos each tick)
 *   +0x28 (40)  : render pos Y
 *   +0x2c (44)  : render pos Z
 *   +0x48 (72)  : world pos X (int32, 16.16 fixed)
 *   +0x4c (76)  : world pos Y
 *   +0x50 (80)  : world pos Z
 *   +0x88 (136) : velocity X (int32, 16.16 fixed)
 *   +0x8c (140) : velocity Y
 *   +0x90 (144) : velocity Z
 *   +0x94 (148) : lifetime countdown (uint16, decremented each tick)
 *
 * Sound pitch selection on collision (non-vehicle collider, rand&3 != 0):
 *   HP in [96,127]:  pitch 62
 *   HP in [64,95]:   pitch 63
 *   HP in [129,159]: pitch 63
 *   HP other:        pitch 61
 *   If rand&3==0 or collider is vehicle (type 2): pitch 64.
 *
 * HIGH confidence: direct MIPS disassembly port.
 */
#include <stdint.h>

/* FUN_80025400 -- Terrain_HeightAt: return terrain Y at world (X, Z). */
extern int FUN_80025400(int x, int z);

/* FUN_8003fd24 -- Debris_Spawn: allocate and place a debris/impact particle.
 * Returns the spawned object handle.  Declared void elsewhere but the
 * return value is used by this caller (confirmed via Ghidra analysis). */
extern intptr_t FUN_8003fd24(const int32_t *xyz, int kind);

/* FUN_8001d708 -- Object_ScheduleEvent: post an event to an object. */
extern void FUN_8001d708(intptr_t obj);

/* FUN_80017160 -- Rand16: return a 16-bit pseudo-random value. */
extern int FUN_80017160(void);

/* FUN_8004410c -- Audio_AllocVoice: allocate a sound voice. */
extern int FUN_8004410c(void);

/* FUN_8004483c -- Audio_VoicePlay: start playback on an allocated voice.
 *   voice, sound_bank, pitch, position_xyz */
extern void FUN_8004483c(int voice, uint32_t sound_bank, int pitch, const void *pos);

/* FUN_800205f8 -- Object_RetireDeferred: queue object for deletion. */
extern void FUN_800205f8(intptr_t obj);

/* DAT_800658FC -- bullet ricochet sound bank (gp+1528). */
extern uint32_t DAT_800658FC;
extern uintptr_t Collision_QueryHostWord(const void *query, uint32_t index);

/* ------------------------------------------------------------------ */

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

/* Choose ricochet pitch from HP bucket (non-vehicle, rand&3 != 0). */
static int bullet_pitch_from_hp(uint16_t hp)
{
    if ((uint16_t)(hp - 96u) < 32u)  return 62;
    if ((uint16_t)(hp - 64u) < 32u)  return 63;
    if ((uint16_t)(hp - 129u) < 31u) return 63;
    return 61;
}

/* ------------------------------------------------------------------ */

intptr_t LAB_80031634(intptr_t obj, int event, intptr_t param3)
{
    uint8_t *s2 = (uint8_t *)(uintptr_t)obj;

    if (event == 0) {
        /* ----- Tick ----- */
        int pos_x = *(int32_t *)(s2 + 0x48);
        int pos_z = *(int32_t *)(s2 + 0x50);
        int terrain_y = FUN_80025400(pos_x, pos_z);
        int pos_y     = *(int32_t *)(s2 + 0x4c);

        if (terrain_y < pos_y) {
            /* Bullet has gone below terrain: terrain impact. */
            const int32_t *pos_field = (const int32_t *)(s2 + 0x48);
            intptr_t particle = FUN_8003fd24(pos_field, 1);
            int rnd      = FUN_80017160();
            *(int16_t *)(uintptr_t)(particle + 0x44) = (int16_t)rnd;
            FUN_8001d708(particle);

            int voice = FUN_8004410c();
            rnd       = FUN_80017160();
            int pitch = ((rnd & 3) == 0) ? 61 : 64;
            FUN_8004483c(voice, DAT_800658FC, pitch, pos_field);
            FUN_800205f8(obj);
            return -1;
        }

        /* Normal: integrate velocity into position. */
        *(int32_t *)(s2 + 0x48) =
            mips_addu_i32(*(int32_t *)(s2 + 0x48), *(int32_t *)(s2 + 0x88));
        *(int32_t *)(s2 + 0x4c) =
            mips_addu_i32(*(int32_t *)(s2 + 0x4c), *(int32_t *)(s2 + 0x8c));
        *(int32_t *)(s2 + 0x50) =
            mips_addu_i32(*(int32_t *)(s2 + 0x50), *(int32_t *)(s2 + 0x90));

        /* Mirror to render position. */
        *(int32_t *)(s2 + 0x24) = *(int32_t *)(s2 + 0x48);
        *(int32_t *)(s2 + 0x28) = *(int32_t *)(s2 + 0x4c);
        *(int32_t *)(s2 + 0x2c) = *(int32_t *)(s2 + 0x50);

        /* Decrement lifetime. */
        uint16_t life = (uint16_t)mips_subu_i32(
            (int32_t)(uint32_t)*(uint16_t *)(s2 + 0x94), 1);
        *(uint16_t *)(s2 + 0x94) = life;
        if (life == 0) {
            FUN_800205f8(obj);
            return -1;
        }
        return 0;
    }

    if (event == 3) {
        /* ----- Collision ----- */
        /* s1 = *param3 = collider object */
        uint8_t *s1 = (uint8_t *)Collision_QueryHostWord((void *)(uintptr_t)param3, 0);
        uint8_t collider_type = s1[4];

        /* Terrain colliders (type 3): ignore. */
        if (collider_type == 3)
            return 0;

        /* Spawn impact particle at bullet position. */
        const int32_t *pos_field = (const int32_t *)(s2 + 0x48);
        intptr_t particle = FUN_8003fd24(pos_field, 1);
        *(int32_t *)(uintptr_t)particle |= 0x400; /* set particle flag */
        int rnd = FUN_80017160();
        *(int16_t *)(uintptr_t)(particle + 0x44) = (int16_t)rnd;
        FUN_8001d708(particle);

        /* If vehicle with special flag: retire without sound. */
        if (collider_type == 2 && *(uint16_t *)(s1 + 0x11e) != 0) {
            FUN_800205f8(obj);
            return -1;
        }

        /* Allocate voice and select pitch. */
        int voice = FUN_8004410c();
        rnd = FUN_80017160();

        int pitch;
        if ((rnd & 3) == 0 || collider_type == 2) {
            pitch = 64;
        } else {
            pitch = bullet_pitch_from_hp(*(uint16_t *)(s1 + 6));
        }

        FUN_8004483c(voice, DAT_800658FC, pitch, pos_field);
        FUN_800205f8(obj);
        return -1;
    }

    return 0;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0
/* --- SLUS_005.10  LAB_80031634  (manual disassembly reference) ---
 *
 * 80031634: addiu sp,sp,-40
 * 8003163c: addu s2,a0,zero        ; s2 = obj
 * 80031644: beq a1,zero,0x80031668 ; event==0 -> tick
 * 80031654: beq a1,v0,0x80031754   ; event==3 -> collision (v0=3)
 * 80031660: j 0x80031848           ; else return 0
 *
 * -- event 0 --
 * 80031668: lw a0,72(s2)            ; pos_x
 * 8003166c: lw a1,80(s2)            ; pos_z
 * 80031670: jal 0x80025400          ; Terrain_HeightAt(x,z)
 * 80031678: lw v1,76(s2)            ; pos_y
 * 80031680: slt v0,v0,v1            ; terrain < pos_y
 * 80031684: beq v0,zero,0x800316dc  ; not below → integrate
 *   -- impact path --
 *   80031688: addiu s1,s2,72        ; s1 = &pos (delay slot)
 *   8003168c: addu a0,s1,zero       ; a0 = &pos
 *   80031690: jal FUN_8003fd24(&pos,1) → particle returned in v0
 *   80031698: jal FUN_80017160(); delay: s0=particle
 *   800316a0: sh rand,68(s0)
 *   800316a4: jal FUN_8001d708(particle)
 *   800316ac: jal FUN_8004410c() → voice; delay: s0=voice
 *   800316b4: jal FUN_80017160(); delay: s0=voice (already)
 *   800316bc: andi v0,v0,3; beq→pitch=61; else pitch=64
 *   800316d4: j 0x8003182c; delay: a3=&pos
 *
 * -- integrate path (0x800316dc) --
 *   pos_x += vel_x; pos_y += vel_y; pos_z += vel_z
 *   render_x/y/z = pos_x/y/z
 *   lifetime--; if==0: FUN_800205f8 + return -1
 *   else return 0 (0x80031848)
 *
 * -- event 3 (0x80031754) --
 *   80031754: lw s1,0(a2)           ; s1 = *param3
 *   8003175c: lbu v0,4(s1)          ; collider_type
 *   80031764: beq v0,3,0x80031844  ; terrain → return 0
 *   8003176c: jal FUN_8003fd24(&pos,1) → particle
 *   80031778: lw v0,0(s0); ori v0,v0,0x400; sw v0,0(s0)
 *   80031784: jal rand; delay: store flags
 *   8003178c: sh rand,68(particle)
 *   80031790: jal FUN_8001d708(particle)
 *   800317a0: bne collider_type,2,0x800317b8
 *   800317a8: lhu v0,0x11e(s1); bne v0,0 → FUN_800205f8+return -1
 *   800317b8: jal FUN_8004410c → voice (s3)
 *   800317c4: jal rand; delay: s3=voice
 *   800317c8: andi v0,v0,3; beq→a2=64; vehicle→a2=64
 *   800317e4: lhu a2,6(s1)  ; HP
 *   HP∈[96-127] → 62; HP∈[64-95] → 63; HP∈[129-159] → 63; else → 61
 *   8003182c: jal FUN_8004483c(voice,bank,pitch,&pos)
 *   80031834: jal FUN_800205f8(obj)
 *   80031840: addiu v0,-1
 * 80031844: addu v0,zero,zero  ; return 0
 */
#endif /* GHIDRA REF */
