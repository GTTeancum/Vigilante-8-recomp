/* mine_rolling_tick.c -- rolling mine physics/trigger tick (LAB_80032aa4).
 *
 * Source: SLUS_005.10  LAB_80032aa4  0x80032aa4  (~70 instructions).
 *
 * Installed at +0x64 of rolling-mine objects by the mine spawner.
 *
 * Event dispatch:
 *   0  -> integrate velocity into position (gravity + inertia); if flag
 *          0x10000 is set, apply damping to all velocity components.
 *          Return 0.
 *   3  -> FUN_80031454(obj, param3, 26, 55) -- proximity damage.
 *          Return its result (-1 = consumed, 0 = pass-through).
 *   5  -> free bone bank + sub-object; spawn explosion debris;
 *          install next-phase tick callback; arm post-die timer.
 *          Return 0.
 *   other -> return 0.
 *
 * Object field layout:
 *   +0x00 (0)   : flags (int32; bit 0x10000 = large/bouncy variant)
 *   +0x24 (36)  : render pos X (mirrored from physics X each tick)
 *   +0x28 (40)  : render pos Y
 *   +0x2c (44)  : render pos Z
 *   +0x30 (48)  : bone-bank handle (freed on die via BoneBank_Free)
 *   +0x38 (56)  : sub-object handle (freed on die via Object_HeapFreeRecursiveQuiet)
 *   +0x48 (72)  : world pos X (int32, 16.16 fixed)
 *   +0x4c (76)  : world pos Y
 *   +0x50 (80)  : world pos Z
 *   +0x58 (88)  : explosion template pointer (int *, passed to FUN_8001add0)
 *   +0x64 (100) : tick callback function pointer (replaced after die)
 *   +0x88 (136) : velocity X (int32, 16.16 fixed)
 *   +0x8c (140) : velocity Y (positive = downward; gravity added each tick)
 *   +0x90 (144) : velocity Z
 *   +0x94 (148) : post-die countdown timer (uint16)
 *
 * Gravity: +56 units/tick added to vel_Y each tick.
 * Damping: vel *= 3968/4096 (~0.9688) per tick when flag 0x10000 is set.
 *
 * HIGH confidence: direct MIPS disassembly port.
 */
#include <stdint.h>

/* FUN_8001bddc -- BoneBank_Free: release a display-list bone bank. */
extern void FUN_8001bddc(int boneBank);

/* FUN_8001af48 -- Object_HeapFreeRecursiveQuiet: free particle subtree. */
extern void FUN_8001af48(int obj);

/* FUN_8001add0 -- BoneFill_Template: spawn explosion debris from template.
 *   param_1: mine object
 *   param_2: explosion template pointer
 *   param_3: explosion radius (19 = normal, 31 = large)
 *   param_4: flags (8 = fill anim slot)
 */
extern int FUN_8001add0(int param_1, int *param_2, uint16_t param_3, uint32_t param_4);

/* FUN_80031454 -- WeaponHit_Apply: apply impulse + damage to collider.
 *   Returns -1 (event consumed) or 0 (pass-through). */
extern int FUN_80031454(intptr_t param_1, intptr_t *param_2, uint16_t param_3, int param_4);

/* Post-die tick callbacks installed after explosion. */
extern intptr_t LAB_800321C0(intptr_t obj, int event, intptr_t param3); /* normal variant */
extern intptr_t LAB_8003277C(intptr_t obj, int event, intptr_t param3); /* large variant  */
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* ------------------------------------------------------------------ */

static int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

intptr_t LAB_80032aa4(intptr_t obj, int event, intptr_t param3)
{
    uint8_t *s0 = (uint8_t *)(uintptr_t)obj;

    if (event == 3) {
        /* Collision: detonate — apply proximity damage to collider. */
        return FUN_80031454(obj, (intptr_t *)(uintptr_t)param3, 26u, 55);
    }

    if (event >= 4) {
        if (event == 5) {
            /* Die: free resources, spawn explosion, transition to next tick. */
            int   flags     = *(int32_t *)s0;
            int   bone_bank = *(int32_t *)(s0 + 0x30);
            int   sub_obj   = *(int32_t *)(s0 + 0x38);
            int  *tmpl      = (int *)(uintptr_t)*(uint32_t *)(s0 + 0x58);

            FUN_8001bddc(bone_bank);
            FUN_8001af48(sub_obj);

            /* Larger mine -> bigger explosion radius. */
            uint16_t radius = (flags & 0x10000) ? 31u : 19u;
            FUN_8001add0(obj, tmpl, radius, 8);

            /* Mark object as dying and queued for removal. */
            *(int32_t *)s0 = flags | 0x84;

            /* Install next-phase tick callback. */
            if (flags & 0x10000) {
                Object_SetCallbackPsxSlot(s0, (uintptr_t)&LAB_8003277C);
            } else {
                Object_SetCallbackPsxSlot(s0, (uintptr_t)&LAB_800321C0);
            }

            /* Arm post-die timer. */
            *(uint16_t *)(s0 + 0x94) = 480;
            return 0;
        }
        /* event 4 or >5: unhandled */
        return 0;
    }

    if (event == 0) {
        /* Tick: physics integration. */

        /* Apply gravity to Y velocity. */
        *(int32_t *)(s0 + 0x8c) = mips_addu_i32(*(int32_t *)(s0 + 0x8c), 56);

        /* Integrate velocity into position (X, Y, Z). */
        *(int32_t *)(s0 + 0x48) =
            mips_addu_i32(*(int32_t *)(s0 + 0x48), *(int32_t *)(s0 + 0x88));
        *(int32_t *)(s0 + 0x4c) =
            mips_addu_i32(*(int32_t *)(s0 + 0x4c), *(int32_t *)(s0 + 0x8c));
        *(int32_t *)(s0 + 0x50) =
            mips_addu_i32(*(int32_t *)(s0 + 0x50), *(int32_t *)(s0 + 0x90));

        /* Mirror physics position to render position. */
        *(int32_t *)(s0 + 0x24) = *(int32_t *)(s0 + 0x48);
        *(int32_t *)(s0 + 0x28) = *(int32_t *)(s0 + 0x4c);
        *(int32_t *)(s0 + 0x2c) = *(int32_t *)(s0 + 0x50);

        /* Apply velocity damping when the large-mine flag is set. */
        if (*(int32_t *)s0 & 0x10000) {
            /* vel *= 3968/4096  (≈ 0.9688 per axis) */
            *(int32_t *)(s0 + 0x88) =
                mips_mult_lo_i32(*(int32_t *)(s0 + 0x88), 3968) >> 12;
            *(int32_t *)(s0 + 0x8c) =
                mips_mult_lo_i32(*(int32_t *)(s0 + 0x8c), 3968) >> 12;
            *(int32_t *)(s0 + 0x90) =
                mips_mult_lo_i32(*(int32_t *)(s0 + 0x90), 3968) >> 12;
        }
        return 0;
    }

    /* event == 1, 2, or other: unhandled */
    return 0;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0
/* --- SLUS_005.10  LAB_80032aa4  (manual disassembly reference) ---
 *
 * 80032aa4: addiu sp,sp,-32
 * 80032aa8: sw s0,16(sp)
 * 80032aac: addu s0,a0,zero       ; s0 = obj
 * 80032ab0: addiu v0,zero,3
 * 80032ab4: sw ra,24(sp)
 * 80032ab8: beq a1,v0,0x80032c2c  ; if event==3 -> collision
 * 80032abc: sw s1,20(sp)
 * 80032ac0: sltiu v0,a1,4
 * 80032ac4: beq v0,zero,0x80032adc ; a1>=4 -> check event 5
 * 80032ac8: addiu v0,zero,5
 * 80032acc: beq a1,zero,0x80032aec ; if event==0 -> tick
 * 80032ad0: addu v0,zero,zero
 * 80032ad4: j 0x80032c4c           ; else return 0
 * 80032adc: beq a1,v0,0x80032bb0  ; if event==5 -> die
 * 80032ae0: addu v0,zero,zero
 * 80032ae4: j 0x80032c4c           ; else return 0
 *
 * -- event 0 (tick) --
 * 80032aec: lw v0,140(s0)          ; vel_y
 * 80032af0: lw v1,72(s0)           ; pos_x
 * 80032af4: lw a0,136(s0)          ; vel_x
 * 80032af8: addiu v0,v0,56         ; vel_y += gravity
 * 80032afc: addu v1,v1,a0          ; pos_x += vel_x
 * 80032b00: addiu a0,s0,72         ; a0 = &pos
 * 80032b04: sw v0,140(s0)          ; store vel_y
 * 80032b08: sw v1,72(s0)           ; store pos_x
 * 80032b0c: lw v0,4(a0)            ; pos_y
 * 80032b10: addiu a2,s0,136        ; a2 = &vel
 * 80032b14: lw a1,4(a2)            ; vel_y
 * 80032b18: lw v1,8(a0)            ; pos_z
 * 80032b1c: addu v0,v0,a1          ; pos_y += vel_y
 * 80032b20: sw v0,4(a0)            ; store pos_y
 * 80032b24: lw v0,8(a2)            ; vel_z
 * 80032b2c: addu v1,v1,v0          ; pos_z += vel_z
 * 80032b30: sw v1,8(a0)            ; store pos_z
 * 80032b34: lw t0,72(s0)           ; pos_x (for render mirror)
 * 80032b38: lw t1,76(s0)           ; pos_y
 * 80032b3c: lw t2,80(s0)           ; pos_z
 * 80032b40: sw t0,36(s0)           ; render_x = pos_x
 * 80032b44: sw t1,40(s0)           ; render_y = pos_y
 * 80032b48: sw t2,44(s0)           ; render_z = pos_z
 * 80032b4c: lw v0,0(s0)            ; flags
 * 80032b50: lui v1,0x1              ; 0x10000
 * 80032b54: and v0,v0,v1            ; flags & 0x10000
 * 80032b58: beq v0,zero,0x80032c48 ; not set -> return 0
 * 80032b5c: addiu v1,zero,3968
 * 80032b60-80032bac: apply damping vel *= 3968>>12 for X, Y, Z
 * 80032ba8: j 0x80032c48            ; return 0
 *
 * -- event 5 (die) --
 * 80032bb0: lw a0,48(s0)            ; obj[0x30] = bone bank
 * 80032bb4: jal 0x8001bddc          ; BoneBank_Free
 * 80032bb8: lui s1,0x1              ; s1 = 0x10000
 * 80032bbc: lw a0,56(s0)            ; obj[0x38] = sub-object
 * 80032bc0: jal 0x8001af48          ; Object_HeapFreeRecursiveQuiet
 * 80032bc8: lw v0,0(s0)             ; flags
 * 80032bd0: and v0,v0,s1            ; flags & 0x10000
 * 80032bd4: beq v0,zero,0x80032be0 ; not set -> radius=19
 * 80032bd8: addiu a2,zero,19        ; (delay slot)
 * 80032bdc: addiu a2,zero,31        ; set radius=31
 * 80032be0: lw a1,88(s0)            ; obj[0x58] = template ptr
 * 80032be4: addu a0,s0,zero
 * 80032be8: jal 0x8001add0          ; FUN_8001add0(obj, tmpl, radius, 8)
 * 80032bec: addiu a3,zero,8
 * 80032bf0: lw v0,0(s0)             ; flags
 * 80032bf8: ori v0,v0,0x84          ; flags |= 0x84
 * 80032bfc: sw v0,0(s0)
 * 80032c00: and v0,v0,s1            ; flags & 0x10000
 * 80032c04: beq v0,zero,0x80032c14 ; not set -> LAB_800321C0
 * 80032c08: lui v0,0x8003
 * 80032c0c: j 0x80032c1c
 * 80032c10: addiu v0,v0,10108       ; v0 = LAB_8003277C
 * 80032c14: lui v0,0x8003
 * 80032c18: addiu v0,v0,8640        ; v0 = LAB_800321C0
 * 80032c1c: sw v0,100(s0)           ; obj[0x64] = new callback
 * 80032c20: addiu v0,zero,480
 * 80032c28: sh v0,148(s0)           ; obj[0x94] = 480
 * 80032c24: j 0x80032c48            ; return 0
 *
 * -- event 3 (collision) --
 * 80032c2c: addu a0,s0,zero
 * 80032c30: addu a1,a2,zero         ; a1 = param3 (collider ptr)
 * 80032c34: addiu a2,zero,26        ; radius=26
 * 80032c38: jal 0x80031454          ; FUN_80031454(obj, collider, 26, 55)
 * 80032c3c: addiu a3,zero,55
 * 80032c40: j 0x80032c4c            ; return v0
 */
#endif /* GHIDRA REF */
