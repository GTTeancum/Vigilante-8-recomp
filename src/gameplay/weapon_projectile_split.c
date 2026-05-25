/* weapon_projectile_split.c -- split-on-impact projectile tick.
 *
 * Source: SLUS_005.10  LAB_8003cb64  0x8003cb64  (~80 instructions).
 *
 * Installed at +0x64 of projectile objects spawned by WeaponSlot_Spawn
 * (FUN_8002c99c).  Wraps the gravity tick and spawns child split
 * projectiles on impact.
 *
 * Event dispatch: all events forwarded to FUN_8003eab0 (gravity tick).
 * On impact (gravity returns < 0), tries to spawn split children via
 * FUN_80021b80 using LAB_8003c61c as the child callback.
 *
 * Split count by weapon type (obj[0x0a]):
 *   bank mismatch (obj[0x58] != DAT_800737dc) : 14 splits
 *   type 0  ->  2    type 7  ->  6
 *   type 10 ->  9    type 13 ->  8
 *   type 17 ->  4    all other types (< 18) -> no split
 *   type >= 18 -> no split
 *
 * Object field layout:
 *   +0x09 (9)   : random byte (randomised on each split-spawn)
 *   +0x0a (10)  : weapon type (uint16)
 *   +0x0c (12)  : HP (uint16)
 *   +0x10 (16)  : MATRIX (32 bytes: 4 words rotation + 4 words translation)
 *   +0x24 (36)  : world pos X (int32, part of MATRIX translation)
 *   +0x28 (40)  : world pos Y
 *   +0x2c (44)  : world pos Z
 *   +0x48 (72)  : spawn pos X (int32, copied from +0x24 on split)
 *   +0x4c (76)  : spawn pos Y
 *   +0x50 (80)  : spawn pos Z
 *   +0x58 (88)  : bank handle (int32)
 *   +0x64 (100) : tick callback
 *
 * HIGH confidence: line-for-line MIPS port.
 */
#include <stdint.h>
#include <string.h>

/* FUN_8003eab0 -- Projectile_GravityTick: integrate vel, gravity, collision.
 * Returns >= 0 when in-flight, < 0 on impact/death. */
extern int FUN_8003eab0(int obj, int event, int param3);

/* FUN_80021b80 -- Bone_AllocWithCallback: allocate new object with callback. */
extern int FUN_80021b80(int (*cb)(int, int, int), int bank, uint16_t count, uint32_t flags);

/* FUN_8002036c -- Object_PostUpdate2: post-spawn scene registration. */
extern void FUN_8002036c(int obj);

/* FUN_8001b0c4 -- BoneAnim_SetupByWeaponType: initialise bone animations. */
extern void FUN_8001b0c4(int obj);

/* FUN_80020890 -- Object_SchedulePostEvent: queue a post-frame event. */
extern void FUN_80020890(int obj, int timer);

/* FUN_80017160 -- V8_RandNext: PRNG. */
extern uint32_t FUN_80017160(void);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* LAB_8003c61c -- split projectile state-machine tick (child callback). */
extern int LAB_8003c61c(int obj, int event, int param3);

/* DAT_800737dc: vehicle-sound bank handle.  Used to identify same-bank splits. */
extern uint32_t DAT_800737dc;

/* DAT_80065BB8: shared bank pointer written by LAB_8003c61c event-1 (spawn)
 * and read here to pass to FUN_80021b80 when splitting.
 * PSX address: gp+2228 = 0x80065304+2228 = 0x80065BB8. */
extern int32_t DAT_80065BB8;

/* ------------------------------------------------------------------ */

/*
 * split_count -- look up how many child projectiles to spawn.
 * Returns < 0 if no split should occur.
 */
static int split_count(uint16_t wtype, int same_bank)
{
    if (!same_bank)
        return 14;
    if (wtype >= 18)
        return -1;
    switch (wtype) {
    case  0: return 2;
    case  7: return 6;
    case 10: return 9;
    case 13: return 8;
    case 17: return 4;
    default: return -1;
    }
}

/* ------------------------------------------------------------------ */

int LAB_8003cb64(int obj, int event, int param3)
{
    uint8_t *s0 = (uint8_t *)(uintptr_t)(uint32_t)obj;

    /* Forward all events to the gravity tick. */
    int ret = FUN_8003eab0(obj, event, param3);

    /* Still in flight (>= 0): nothing more to do. */
    if (ret >= 0)
        return ret;

    /* ---- Impact: save matrix snapshot (obj[0x10..0x2f] = 32 bytes). ---- */
    uint32_t mat[8];
    memcpy(mat, s0 + 0x10, 32);

    /* Save HP before the old object is potentially freed. */
    uint16_t hp = *(uint16_t *)(s0 + 0x0c);

    /* Determine split count. */
    int32_t bank_old = *(int32_t *)(s0 + 0x58);
    int same_bank = ((uint32_t)bank_old == DAT_800737dc);
    uint16_t wtype = *(uint16_t *)(s0 + 0x0a);
    int count = split_count(wtype, same_bank);

    /* No split: return gravity-tick result unchanged. */
    if (count < 0)
        return ret;

    /* ---- Spawn child split-projectile. ---- */
    int new_obj = FUN_80021b80(&LAB_8003c61c,
                                DAT_80065BB8,
                                (uint16_t)((uint32_t)count & 0xffffu),
                                0);
    uint8_t *p = (uint8_t *)(uintptr_t)(uint32_t)new_obj;

    /* Copy HP. */
    *(uint16_t *)(p + 0x0c) = hp;

    /* Copy full matrix (8 words: rotation + translation). */
    memcpy(p + 0x10, mat, 32);

    /* Copy world position (mat[1..3] = words at +0x24,+0x28,+0x2c)
     * to the spawn-position slot at +0x48..+0x50. */
    *(uint32_t *)(p + 0x48) = mat[5];   /* saved obj[0x24] */
    *(uint32_t *)(p + 0x4c) = mat[6];   /* saved obj[0x28] */
    *(uint32_t *)(p + 0x50) = mat[7];   /* saved obj[0x2c] */

    /* Random byte at +0x09. */
    uint32_t rnd = FUN_80017160();
    p[0x09] = (uint8_t)rnd;

    /* Install child tick callback and post-update. */
    Object_SetCallbackPsxSlot(p, (uintptr_t)&LAB_8003c61c);
    FUN_8002036c(new_obj);

    /* Bone/animation setup. */
    FUN_8001b0c4(new_obj);

    /* Schedule post-frame event (timer=600). */
    FUN_80020890(new_obj, 600);

    return ret;
}

int FUN_8003cb64(int obj, int event, int param3)
{
    return LAB_8003cb64(obj, event, param3);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10  LAB_8003cb64  (manual disassembly reference) ---
 *
 * 8003cb64: addiu sp,sp,-72
 * 8003cb6c: addu t0,a0,zero          ; t0=obj
 * 8003cb7c: jal 8003eab0             ; FUN_8003eab0(obj,event,param3)
 * 8003cb84: addu t3,v0,zero          ; t3=ret
 * 8003cb88: bgez t3,8003ccec         ; if ret>=0 -> exit
 * 8003cb90: lw s0..s3, 16..28(t0)   ; save obj[0x10..0x1c]
 * 8003cba0: sw s0..s3, 16..28(sp)
 * 8003cbb0: lw s0..s3, 32..44(t0)   ; save obj[0x20..0x2c]
 * 8003cbc0: sw s0..s3, 32..44(sp)
 * 8003cbd0: lw v1,88(t0)            ; bank_old = obj[0x58]
 * 8003cbd4: lw v0,14300(v0)         ; DAT_800737dc (v0 has 0x8007xxxx from delay)
 * 8003cbd8: lhu t2,12(t0)           ; hp = u16(obj[0x0c])
 * 8003cbdc: bne v1,v0,8003cc3c      ; bank mismatch -> a2=14
 * 8003cbe0: addiu a2,zero,-1        ; delay: a2=-1 default
 * 8003cbe4: lhu v1,10(t0)           ; weapon type
 * 8003cbec: sltiu v0,v1,18          ; type<18?
 * 8003cbf0: beq v0,zero,8003cc40    ; type>=18 -> exit (a2=-1)
 * 8003cbf8: addiu v0,v0,2832        ; jump table = 0x80010B10
 * ... jump table dispatch ...
 * 8003cc14: j 8003cc40 / addiu a2,4  ; type 17
 * 8003cc1c: j 8003cc40 / addiu a2,2  ; type 0
 * 8003cc24: j 8003cc40 / addiu a2,6  ; type 7
 * 8003cc2c: j 8003cc40 / addiu a2,9  ; type 10
 * 8003cc34: j 8003cc40 / addiu a2,8  ; type 13
 * 8003cc3c: addiu a2,14              ; bank mismatch
 * 8003cc40: bltz a2,8003ccec         ; if a2<0 -> no split
 * 8003cc48: lw a1,2228(gp)           ; a1 = DAT_80065BB8
 * 8003cc4c: addiu t1,0x8004,-14820   ; t1 = 0x8003C61C = &LAB_8003c61c
 * 8003cc54: andi a2,a2,0xffff
 * 8003cc58: jal 80021b80             ; FUN_80021b80(LAB_8003c61c,bank,count,0)
 * 8003cc60: addu t0,v0,zero          ; t0 = new_obj
 * 8003cc64: sh t2,12(t0)             ; new[0x0c] = hp
 * 8003cc68: lw s0..s3,16..28(sp)
 * 8003cc78: sw s0..s3,16..28(t0)    ; copy matrix words 0-3
 * 8003cc88: lw s0..s3,32..44(sp)
 * 8003cc98: sw s0..s3,32..44(t0)    ; copy matrix words 4-7
 * 8003cca8: lw s0,36(sp)            ; s0=old[0x24]
 * 8003ccac: lw s1,40(sp)            ; s1=old[0x28]
 * 8003ccb0: lw s2,44(sp)            ; s2=old[0x2c]
 * 8003ccb4: sw s0,72(t0)            ; new[0x48]=old[0x24]
 * 8003ccb8: sw s1,76(t0)            ; new[0x4c]=old[0x28]
 * 8003ccbc: sw s2,80(t0)            ; new[0x50]=old[0x2c]
 * 8003ccc0: jal 80017160            ; rand
 * 8003cccc: sb v0,9(t0)             ; new[0x09] = rand
 * 8003ccd0: jal 8002036c            ; FUN_8002036c(new)
 * 8003ccd4: sw t1,100(t0)           ; delay: new[0x64]=LAB_8003c61c
 * 8003ccd8: jal 8001b0c4            ; FUN_8001b0c4(new)
 * 8003cce4: jal 80020890            ; FUN_80020890(new,600)
 * 8003ccec: addu v0,t3,zero         ; return t3 (gravity tick result)
 */

#endif  /* GHIDRA REF */
