/* bomb_attached_tick.c -- sticky bomb attached-to-vehicle tick (LAB_80031AFC).
 *
 * Source: SLUS_005.10  LAB_80031AFC  0x80031AFC  (~40 instructions).
 *
 * Installed at obj+0x64 by LAB_80031bbc (slow_proj_tick.c) when a slow/sticky
 * projectile lands on a vehicle collider (type 2).  The bomb child is appended
 * to the vehicle's child list so FUN_80017594 (angular_impulse.c) keeps the
 * render position locked to the vehicle frame each tick.
 *
 * Event dispatch:
 *   0  -> update render position via angular-impulse follow; decrement lifetime;
 *          when lifetime reaches zero: play sound, spawn debris kind 16,
 *          apply damage (-40 hp) to parent vehicle, detach and free self.
 *          Returns -1 on detonation, 0 while counting down.
 *   other -> return 0.
 *
 * Object field layout (as sticky-bomb child):
 *   +0x24 (36)  : render pos X  (used as 3D sound position, updated by follow)
 *   +0x28 (40)  : render pos Y
 *   +0x2c (44)  : render pos Z
 *   +0x88 (136) : velocity X  (passed to FUN_80017594 as transform src)
 *   +0x94 (148) : lifetime countdown (uint16, set to 360 by slow_proj_tick)
 *
 * HIGH confidence: direct MIPS disassembly port.
 */
#include <stdint.h>

/* FUN_8001d5a0 -- Object_FindFirstChild: return first child of obj. */
extern int FUN_8001d5a0(int obj);

/* FUN_80017594 -- Object_ApplyAngularFollowParent: update child render position
 * by following parent's angular velocity and local offset.
 * (parent, &vel, &render_pos) */
extern void FUN_80017594(uint32_t *parent, int32_t *vel, int32_t *render_pos);

/* FUN_8004410c -- Audio_AllocVoice. */
extern int FUN_8004410c(void);

/* FUN_8004483c -- Audio_VoicePlay: (voice, bank, pitch, pos). */
extern void FUN_8004483c(int voice, uint32_t bank, int pitch, const void *pos);

/* FUN_8003fd24 -- Debris_Spawn: spawn impact particle; returns handle. */
extern int FUN_8003fd24(const int32_t *xyz, int kind);

/* FUN_8002c958 -- Impulse_Dispatch: apply damage/impulse to object.
 * (obj, damage_amount, pos, flags) */
extern int FUN_8002c958(uint32_t *obj, int damage, const int32_t *pos, int flags);

/* FUN_8001d564 -- Object_DetachFromParent: remove self from parent child list.
 * Returns the detached object handle. */
extern int FUN_8001d564(int obj);

/* FUN_800204dc -- Object_HeapFreeRecursive: free object and all children. */
extern void FUN_800204dc(int obj);

/* DAT_800658FC -- sound bank (gp+1528). */
extern uint32_t DAT_800658FC;

/* ------------------------------------------------------------------ */

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

int LAB_80031AFC(int obj, int event, int param3)
{
    uint8_t *s1 = (uint8_t *)(uintptr_t)(uint32_t)obj;
    (void)param3;

    if (event != 0)
        return 0;

    /* ----- Tick ----- */
    int32_t *render_pos = (int32_t *)(s1 + 0x24);  /* s0 = s1+36 */

    /* Follow parent vehicle: keep render position locked to vehicle frame. */
    int parent = FUN_8001d5a0(obj);
    FUN_80017594((uint32_t *)(uintptr_t)parent, (int32_t *)(s1 + 0x88), render_pos);

    /* Decrement lifetime countdown. */
    uint16_t life = *(uint16_t *)(s1 + 0x94);
    life = (uint16_t)mips_subu_i32((int32_t)life, 1);
    *(uint16_t *)(s1 + 0x94) = life;

    /* Test if now zero (PSX: sll v0,v0,16; bne v0,zero → if (life<<16)!=0). */
    if ((uint32_t)life << 16)
        return 0;

    /* Lifetime expired: detonate. */
    int voice = FUN_8004410c();
    FUN_8004483c(voice, DAT_800658FC, 54, render_pos);

    FUN_8003fd24((const int32_t *)render_pos, 16);

    /* Apply -40 HP damage to parent vehicle at render position. */
    parent = FUN_8001d5a0(obj);
    FUN_8002c958((uint32_t *)(uintptr_t)parent, -40, (const int32_t *)render_pos, 1);

    /* Detach from parent child list and free. */
    int detached = FUN_8001d564(obj);
    FUN_800204dc(detached);

    return -1;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0
/* --- SLUS_005.10  LAB_80031AFC  (manual disassembly reference) ---
 *
 * 80031afc: addiu sp,sp,-32
 * 80031b00: sw s1,20(sp)
 * 80031b04: addu s1,a0,zero        ; s1 = obj
 * 80031b08: sw ra,24(sp)
 * 80031b0c: bne a1,zero,0x80031ba4 ; event!=0 → return 0
 * 80031b10: sw s0,16(sp)
 * 80031b14: jal 0x8001d5a0          ; Object_FindFirstChild(obj)
 * 80031b18: addiu s0,s1,36          ; delay: s0 = &render_pos
 * 80031b1c: addu a0,v0,zero         ; parent
 * 80031b20: addiu a1,s1,136         ; &vel
 * 80031b24: jal 0x80017594          ; FUN_80017594(parent, &vel, &render_pos)
 * 80031b28: addu a2,s0,zero
 * 80031b2c: lhu v0,148(s1)          ; life
 * 80031b34: addiu v0,v0,-1
 * 80031b38: sh v0,148(s1)
 * 80031b3c: sll v0,v0,16
 * 80031b40: bne v0,zero,0x80031ba4  ; if life != 0 → return 0
 * 80031b48: jal 0x8004410c          ; AllocVoice
 * 80031b50: lw a1,1528(gp)          ; DAT_800658FC
 * 80031b54: addu a0,v0,zero         ; voice
 * 80031b58: addiu a2,zero,54
 * 80031b5c: jal 0x8004483c          ; VoicePlay(voice, bank, 54, &render_pos)
 * 80031b60: addu a3,s0,zero
 * 80031b64: addu a0,s0,zero         ; &render_pos
 * 80031b68: jal 0x8003fd24          ; Debris_Spawn(&render_pos, 16)
 * 80031b6c: addiu a1,zero,16
 * 80031b70: jal 0x8001d5a0          ; parent = FindFirstChild(obj)
 * 80031b74: addu a0,s1,zero
 * 80031b78: addu a0,v0,zero
 * 80031b7c: addiu a1,zero,-40       ; damage = -40
 * 80031b80: addu a2,s0,zero         ; &render_pos
 * 80031b84: jal 0x8002c958          ; Impulse_Dispatch(parent,-40,&pos,1)
 * 80031b88: addiu a3,zero,1
 * 80031b8c: jal 0x8001d564          ; obj = DetachFromParent(obj)
 * 80031b90: addu a0,s1,zero
 * 80031b94: jal 0x800204dc          ; HeapFreeRecursive(obj)
 * 80031b98: addu a0,v0,zero
 * 80031b9c: j 0x80031ba8
 * 80031ba0: addiu v0,zero,-1        ; return -1
 * 80031ba4: addu v0,zero,zero       ; return 0
 */
#endif /* GHIDRA REF */
