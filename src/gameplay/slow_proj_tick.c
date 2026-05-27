/* slow_proj_tick.c -- slow/sticky projectile tick (LAB_80031bbc).
 *
 * Source: SLUS_005.10  LAB_80031bbc  0x80031bbc  (~84 instructions).
 *
 * Tick callback for slow projectiles (e.g. Spider Bomb wtype-2).  If the
 * object-flags bit 0x10000 is set the projectile can stick to vehicles.
 *
 * Event dispatch:
 *   0  -> integrate velocity; update rotation matrix; emit smoke trail
 *          every 32 ticks; decrement lifetime; retire on terrain hit or
 *          lifetime expiry.  Returns -1 on retire, 0 otherwise.
 *   3  -> collision:
 *          flags & 0x10000 AND vehicle collider (type 2): compute delta
 *          vector relative to collider, run GTE transforms, detach from
 *          scene list, install LAB_80031AFC (attached-bomb tick) and append
 *          as child of the vehicle.  Return 1.
 *          Otherwise: WeaponHit_Apply(obj, param3, 12, 54).
 *          Return the call result (-1 or 0).
 *   other -> return 0.
 *
 * Object field layout:
 *   +0x00 (0)   : flags (int32; bit 0x10000 = sticky-bomb variant)
 *   +0x10 (16)  : 3×3 rotation matrix (input to FUN_800439b8 / FUN_8004c934)
 *   +0x24 (36)  : render pos X (mirrored from physics each tick)
 *   +0x28 (40)  : render pos Y
 *   +0x2c (44)  : render pos Z
 *   +0x48 (72)  : world pos X (int32, 16.16 fixed)
 *   +0x4c (76)  : world pos Y
 *   +0x50 (80)  : world pos Z
 *   +0x64 (100) : tick callback (replaced with LAB_80031AFC on vehicle hit)
 *   +0x88 (136) : velocity X
 *   +0x8c (140) : velocity Y
 *   +0x90 (144) : velocity Z
 *   +0x94 (148) : lifetime countdown (uint16, also stores anim/smoke phase)
 *
 * HIGH confidence: direct MIPS disassembly port.
 */
#include <stdint.h>

/* FUN_800439b8 -- RotMatrix_ApplyAngVel: update rotation matrix. */
extern void FUN_800439b8(int *matrix, int ax, int ay, int az);

/* FUN_8004c934 -- MatrixNormal: compute surface normal from matrix. */
extern void FUN_8004c934(int *mat_a, int *mat_b);

/* FUN_80025400 -- Terrain_HeightAt: return terrain Y at (X, Z). */
extern int FUN_80025400(int x, int z);

/* FUN_8004410c -- Audio_AllocVoice. */
extern int FUN_8004410c(void);

/* FUN_8004483c -- Audio_VoicePlay: (voice, bank, pitch, pos). */
extern void FUN_8004483c(int voice, uint32_t bank, int pitch, const void *pos);

/* FUN_8003fd24 -- Debris_Spawn: spawn impact particle; returns object handle. */
extern int FUN_8003fd24(const int32_t *xyz, int kind);

/* FUN_80020620 -- ColEvent_Dispatch: dispatch a collision event. */
extern void FUN_80020620(uint32_t obj, uint32_t event);

/* FUN_800205f8 -- Object_RetireDeferred: queue object for removal. */
extern void FUN_800205f8(int obj);

/* FUN_80031454 -- WeaponHit_Apply. */
extern int FUN_80031454(intptr_t obj, intptr_t *collider, uint16_t radius, int strength);

/* FUN_80043248 -- GTE_RotateLong: rotate a 3-element int vector by
 * the current GTE matrix.  (a0=src, a1=dst) */
extern int *FUN_80043248(int *src, int *dst);

/* FUN_8004366c -- GTE_LoadTransposed: load transposed matrix from collider. */
extern void FUN_8004366c(int *delta);

/* FUN_8004cdc4 -- GTE_MatrixIdentity(?): identity/normalize matrix op. */
extern void FUN_8004cdc4(int *mat_a, int *mat_b);

/* FUN_8001fe8c -- ObjectList_Unlink: remove obj from a linked list. */
extern int FUN_8001fe8c(void *listHead, uint32_t *obj);

/* FUN_8001d4f0 -- Object_AppendChild: tail-insert child into parent. */
extern void FUN_8001d4f0(uint32_t *parent, uint32_t *child);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* LAB_80031AFC -- Bomb_AttachedTick: tick callback for a bomb stuck to
 * a vehicle.  Installed at obj[0x64] when the projectile lands on a vehicle. */
extern intptr_t LAB_80031AFC(intptr_t obj, int event, intptr_t param3);
extern uintptr_t Collision_QueryHostWord(const void *query, uint32_t index);

/* DAT_800658FC -- sound bank (gp+1528). */
extern uint32_t DAT_800658FC;

/* DAT_80065a18 -- active-object list sentinel used by ObjectList_Unlink. */
extern uint8_t DAT_80065a18[];

/* ------------------------------------------------------------------ */

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

intptr_t LAB_80031bbc(intptr_t obj, int event, intptr_t param3)
{
    uint8_t *s1 = (uint8_t *)(uintptr_t)obj;

    if (event == 0) {
        /* ----- Tick ----- */
        /* Integrate velocity into position (X, Y, Z). */
        *(int32_t *)(s1 + 0x48) =
            mips_addu_i32(*(int32_t *)(s1 + 0x48), *(int32_t *)(s1 + 0x88));
        *(int32_t *)(s1 + 0x4c) =
            mips_addu_i32(*(int32_t *)(s1 + 0x4c), *(int32_t *)(s1 + 0x8c));
        *(int32_t *)(s1 + 0x50) =
            mips_addu_i32(*(int32_t *)(s1 + 0x50), *(int32_t *)(s1 + 0x90));

        /* Mirror to render position. */
        *(int32_t *)(s1 + 0x24) = *(int32_t *)(s1 + 0x48);
        *(int32_t *)(s1 + 0x28) = *(int32_t *)(s1 + 0x4c);
        *(int32_t *)(s1 + 0x2c) = *(int32_t *)(s1 + 0x50);

        /* Update rotation matrix (+0x10 area). */
        FUN_800439b8((int *)(s1 + 0x10), 0, 0, 512);

        /* Emit smoke trail every 32 ticks. */
        uint16_t life = *(uint16_t *)(s1 + 0x94);
        if ((life & 0x1f) == 0) {
            FUN_8004c934((int *)(s1 + 0x10), (int *)(s1 + 0x10));
        }

        /* Decrement lifetime; retire if zero. */
        life = (uint16_t)mips_subu_i32((int32_t)life, 1);
        *(uint16_t *)(s1 + 0x94) = life;
        if (life == 0) {
            FUN_800205f8(obj);
            return -1;
        }

        /* Terrain collision check. */
        int pos_x = *(int32_t *)(s1 + 0x48);
        int pos_z = *(int32_t *)(s1 + 0x50);
        int pos_y = *(int32_t *)(s1 + 0x4c);
        int terrain_y = FUN_80025400(pos_x, pos_z);

        if (pos_y >= terrain_y) {
            /* Bullet at or below terrain: terrain impact. */
            const int32_t *pos_field = (const int32_t *)(s1 + 0x48);
            int voice = FUN_8004410c();
            FUN_8004483c(voice, DAT_800658FC, 54, pos_field);
            FUN_8003fd24(pos_field, 12);
            FUN_80020620((uint32_t)obj, 1u);
            return -1;
        }

        return 0;
    }

    if (event == 3) {
        /* ----- Collision ----- */
        int flags = *(int32_t *)s1;
        uint8_t *s2 = (uint8_t *)Collision_QueryHostWord((void *)(uintptr_t)param3, 0);
        uint8_t collider_type = s2[4];

        if ((flags & 0x10000) && collider_type == 2) {
            /* Sticky: attach to vehicle. */

            /* Compute delta = obj.render_pos - collider.render_pos. */
            int32_t delta[3];
            delta[0] = mips_subu_i32(*(int32_t *)(s1 + 0x24), *(int32_t *)(s2 + 0x24));
            delta[1] = mips_subu_i32(*(int32_t *)(s1 + 0x28), *(int32_t *)(s2 + 0x28));
            delta[2] = mips_subu_i32(*(int32_t *)(s1 + 0x2c), *(int32_t *)(s2 + 0x2c));

            /* Load collider's transposed rotation matrix, normalize delta. */
            FUN_8004366c(delta);

            /* GTE transforms on velocity and position. */
            FUN_80043248((int *)(s1 + 0x88), (int *)(s1 + 0x88));
            FUN_80043248(delta, (int *)(s1 + 0x24));

            /* Normalize/orient obj rotation matrix. */
            FUN_8004cdc4((int *)(s1 + 0x10), (int *)(s1 + 0x10));

            /* Detach from scene list, install attached-bomb tick. */
            FUN_8001fe8c(DAT_80065a18, (uint32_t *)s1);
            Object_SetCallbackPsxSlot((void *)(uintptr_t)s1, (uintptr_t)&LAB_80031AFC);
            *(uint16_t *)(s1 + 0x94) = 360;

            /* Append as child of the vehicle. */
            FUN_8001d4f0((uint32_t *)s2, (uint32_t *)s1);
            return 1;
        }

        /* Non-vehicle or non-sticky: apply weapon hit. */
        return FUN_80031454(obj, (intptr_t *)(uintptr_t)param3,
                            12u, 54);
    }

    return 0;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0
/* --- SLUS_005.10  LAB_80031bbc  (manual disassembly reference) ---
 *
 * 80031bbc: addiu sp,sp,-48
 * 80031bc4: addu s1,a0,zero        ; s1 = obj
 * 80031bd0: beq a1,zero,0x80031bec ; event==0 -> tick
 * 80031bdc: beq a1,3,0x80031cfc   ; event==3 -> collision
 * 80031be4: j 0x80031de4           ; else return 0
 *
 * -- event 0 (0x80031bec) --
 *   pos_x += vel_x; pos_y += vel_y; pos_z += vel_z
 *   render_{xyz} = pos_{xyz}
 *   FUN_800439b8(&obj[0x10], 0, 0, 512)
 *   if (life & 0x1f)==0: FUN_8004c934(&obj[0x10], &obj[0x10])
 *   life--; if==0: FUN_800205f8(obj) + return -1
 *   terrain_y = FUN_80025400(x,z)
 *   if pos_y >= terrain_y:
 *     SoundAlloc+play(54) + FUN_8003fd24(&pos,12) + FUN_80020620(obj,1)
 *     return -1
 *   return 0
 *
 * -- event 3 (0x80031cfc) --
 *   flags = obj[0]; s2 = *param3 (collider)
 *   if flags&0x10000 AND collider_type==2:
 *     delta = render_pos - s2.render_pos
 *     FUN_8004366c(delta)
 *     FUN_80043248(&vel,&vel); FUN_80043248(delta,&render_pos)
 *     FUN_8004cdc4(&obj[0x10],&obj[0x10])
 *     FUN_8001fe8c(DAT_80065a18, obj)
 *     obj[0x64] = LAB_80031AFC (0x80031AFC)
 *     obj[0x94] = 360
 *     FUN_8001d4f0(collider, obj)
 *     return 1
 *   else:
 *     WeaponHit_Apply(obj, param3, 12, 54)
 *
 * 80031de0: addu v0,zero,zero  ; return 0
 */
#endif /* GHIDRA REF */
