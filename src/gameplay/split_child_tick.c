/* split_child_tick.c -- split-projectile child tick (LAB_8003c61c).
 *
 * Source: SLUS_005.10  LAB_8003c61c  0x8003c61c  (~130 instructions).
 *
 * Installed at +0x64 of child objects spawned by LAB_8003cb64
 * (weapon_projectile_split.c).  Drives the second-generation
 * projectile state machine: orbit/timer tick, spawn/death events,
 * and weapon-type-specific impact behaviour.
 *
 * Event dispatch:
 *   0  -> tick: increment sub-timer at +0x42 by 68; if param3 != 0
 *          call Object_InitBoneMatrix.  Return 0.
 *   1  -> spawn: store bank handle to DAT_80065BB8, set flags, type
 *          byte, lifetime counter.  If weapon type == 13, randomise
 *          weapon-sound assignment.  Return 0.
 *   2  -> die: if type == 1, spawn trail + Damage_Apply, return -1.
 *          Else clear flag 0x8000, return -1.
 *   3  -> collision: source_type == 2 -> weapon-type damage dispatch
 *          (see split_dispatch below).  source_type == 7 -> return 1.
 *          Other source types -> return 0.
 *
 * Weapon-type dispatch (event 3, source_type == 2):
 *   wtype 0  : heal adjacent HP slots, play SFX 39, common tail.
 *   wtype 2  : spawn child (LAB_80031fa0, bone 0,  kind 0x8011).
 *   wtype 4  : spawn child (LAB_8003302c, bone 17, kind 0x8012).
 *   wtype 6  : spawn child (LAB_800336fc, bone 7,  kind 0x8013).
 *   wtype 8  : spawn child (LAB_8003565c, bone 13, kind 0x8015).
 *   wtype 9  : spawn child (LAB_80034920, bone 10, kind 0x8014).
 *   wtype 10 : set source[0x120]=900, play SFX 38, common tail.
 *   wtype 11 : set source[0x11c]=900, play SFX 38, common tail.
 *   wtype 12 : set source[0x11e]=900, play SFX 38, common tail.
 *   wtype 14 : find bone 0x801f on source, spawn child (wtype lookup).
 *   others   : return 0.
 *
 * HIGH confidence: line-for-line MIPS port with direct disassembly.
 */
#include <stdint.h>
#include <string.h>

/* -- External helpers -- */
extern void     FUN_8001d708(int obj);              /* Object_InitBoneMatrix */
extern intptr_t FUN_8001b038(int obj, uint16_t kind);/* Bone_FindByKindOnObj */
extern int      FUN_8001ac44(int bank, uint16_t slot, uint32_t size, uint32_t flags); /* BoneObj_BuildTree */
extern void     FUN_800205f8(int obj);              /* Damage_Apply */
extern int      FUN_8003fea8(uint32_t pos, uint32_t colour); /* WeaponSpawn_SparkEffect */
extern void     FUN_80020890(int obj, int timer);   /* Object_SchedulePostEvent */
extern void     FUN_80020744(int obj);              /* Object_BindToWorld */
extern int      FUN_8001ffd4(int param_1, int param_2); /* ObjList_FindPayloadBySpawnId */
extern void     FUN_8002cbe8(int source, int child); /* WeaponSlot_Attach / Object_LinkToVehicle */
extern void     FUN_8003fd24(const int32_t *xyz, int arg); /* Debris_AttachTrack */
extern void    *FUN_8003d1e8(uint8_t state_code);   /* Vehicle_StateToCallback (vehicle_init_joints.c) */
extern uint32_t FUN_8003ce24(uint32_t param_1);     /* WeaponTarget_PickRandom */

/* Audio stubs (engine out of scope) */
extern int  FUN_8004410c(void);                    /* Audio_AllocVoice */
extern int  FUN_8004483c(int voice, int bank, int sfx, int *pos); /* Audio_PlaySfx3D */

/* Globals */
extern uint32_t DAT_800737dc;   /* vehicle-sound bank handle */
extern int32_t  DAT_80065BB8;   /* gp+2228: shared bank ptr (written on spawn) */
extern uint8_t  uRam000005f8;   /* audio bank handle */
extern int32_t  DAT_80065aac;   /* gp+1960: wtype-0 hit counter */
extern int32_t  DAT_80065a10;   /* gp+1804: wtype-2..14 hit counter */
extern int16_t  DAT_8005ec84[]; /* weapon slot → bone-bank index table */
extern uint8_t  DAT_80065a50[]; /* 0x8006_5A50 re-spawn / obj-list table */
extern void     Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* Sub-tick callback pointers for newly spawned children (pass-3 stubs) */
extern int LAB_80031fa0(int, int, int); /* wtype 2 child tick */
extern int LAB_8003302c(int, int, int); /* wtype 4 child tick */
extern int LAB_800336fc(int, int, int); /* wtype 6 child tick */
extern int LAB_80034920(int, int, int); /* wtype 9 child tick */
extern int LAB_8003565c(int, int, int); /* wtype 8 child tick */

/* ------------------------------------------------------------------ */

int LAB_8003565c(int obj, int event, int param3)
{
    (void)obj;
    (void)event;
    (void)param3;
    return 0;
}

/*
 * common_spawn_child -- shared logic for wtypes 2,4,6,8,9,14.
 *
 *   obj       : self (the split child)
 *   source    : vehicle/object that collided
 *   bank      : bone-bank to allocate child from
 *   slot      : bone-slot index within bank
 *   child_cb  : tick callback to install on new child (may be NULL)
 *   kind      : bone kind to search for on source (for position copy)
 *
 * Returns -2 on success, 0 if s0<0 was the entry path (skip spawn).
 */
static int common_spawn_child(int obj, int source, int bank,
                              int16_t slot, int (*child_cb)(int,int,int),
                              uint16_t kind)
{
    uint8_t *s2 = (uint8_t *)(uintptr_t)(uint32_t)obj;
    uint8_t *s3 = (uint8_t *)(uintptr_t)(uint32_t)source;

    /* Play impact SFX 37 */
    {
        int voice = FUN_8004410c();
        FUN_8004483c(voice, (int)(uintptr_t)(uint32_t)uRam000005f8, 37, (int *)(s2 + 0x24));
    }

    /* Spark effect if source[6] == -1 */
    if (*(int16_t *)(s3 + 0x06) == -1) {
        FUN_8003fea8((uint32_t)(uintptr_t)(s3 + 0x24), 0x84044040u);
    }

    /* Allocate child bone object from bank */
    int child = FUN_8001ac44(bank, (uint16_t)(uint32_t)slot, 0x84, 8);
    uint8_t *cp = (uint8_t *)(uintptr_t)(uint32_t)child;

    /* Flags */
    *(uint32_t *)(cp + 0x00) = 0x00010000u;

    /* Install tick callback and fire spawn event (event 1) */
    if (child_cb) {
        Object_SetCallbackPsxSlot(cp, (uintptr_t)child_cb);
        child_cb(child, 1, 0);
    }

    /* maxHP = HP from newly spawned object */
    *(uint16_t *)(cp + 0x0e) = *(uint16_t *)(cp + 0x0c);

    /* Copy parent HP if non-zero */
    uint16_t parent_hp = *(uint16_t *)(s2 + 0x0c);
    if (parent_hp != 0)
        *(uint16_t *)(cp + 0x0c) = parent_hp;

    /* Find bone of 'kind' on source for position data */
    intptr_t bone = FUN_8001b038(source, kind);
    *(uint32_t *)(cp + 0x80) = (uint32_t)(uintptr_t)bone;

    if (bone) {
        uint8_t *bp = (uint8_t *)bone;
        /* Unaligned copy: bone[0x10..0x13] (word) + bone[0x14..0x15] (half) */
        uint32_t bw; uint16_t bh;
        memcpy(&bw, bp + 0x10, 4);
        memcpy(&bh, bp + 0x14, 2);
        memcpy(cp + 0x40, &bw, 4);
        memcpy(cp + 0x44, &bh, 2);
    }

    /* Relative position: obj[0x48..0x50] - source[0x48..0x50] */
    {
        int32_t *op = (int32_t *)(s2 + 0x48);
        int32_t *sp2 = (int32_t *)(s3 + 0x48);
        int32_t *cp2 = (int32_t *)(cp + 0x48);
        cp2[0] = op[0] - sp2[0];
        cp2[1] = op[1] - sp2[1];
        cp2[2] = op[2] - sp2[2];
    }

    FUN_8001d708(child);               /* Object_InitBoneMatrix */
    FUN_80020744(child);               /* Object_BindToWorld */
    FUN_8002cbe8(source, child);       /* Object_LinkToVehicle */

    /* Fall through to Damage_Apply + re-spawn */
    FUN_800205f8(obj);                 /* self destruct */

    /* Re-spawn next gen from table */
    int16_t idx = *(int16_t *)(s2 + 0x06);
    int next = FUN_8001ffd4((int)(uintptr_t)DAT_80065a50, (int)idx);
    if (next) {
        FUN_80020890(next, 600);
        DAT_80065a10 -= 1;
    }
    return -2;
}

/* ------------------------------------------------------------------ */

int LAB_8003c61c(int obj, int event, int param3)
{
    uint8_t *s2 = (uint8_t *)(uintptr_t)(uint32_t)obj;

    switch (event) {

    /* ---- Event 0: per-frame tick ---- */
    case 0: {
        uint16_t sub = *(uint16_t *)(s2 + 0x42);
        sub = (uint16_t)(sub + 68u);
        *(uint16_t *)(s2 + 0x42) = sub;
        if (param3 != 0)
            FUN_8001d708(obj);          /* Object_InitBoneMatrix */
        return 0;
    }

    /* ---- Event 1: spawn init ---- */
    case 1: {
        /* Save bank handle globally for parent's next FUN_80021b80 call */
        DAT_80065BB8 = *(int32_t *)(s2 + 0x58);

        /* Update flags: set bits 7,8,9; clear bit 3 */
        uint32_t fl = *(uint32_t *)(s2 + 0x00);
        fl = (fl | 0x380u) & ~0x8u;
        *(uint32_t *)(s2 + 0x00) = fl;

        /* type byte */
        s2[0x04] = 3;

        /* Lifetime counter: initialise if zero */
        if (*(int32_t *)(s2 + 0x6c) == 0)
            *(int32_t *)(s2 + 0x6c) = 0x1f4000;

        /* Weapon type 13: randomise sound index */
        if (*(uint16_t *)(s2 + 0x0a) == 13) {
            uint idx = FUN_8003ce24(fl);
            /* Update weapon type from sound-slot table */
            *(uint16_t *)(s2 + 0x0a) = (uint16_t)DAT_8005ec84[idx];
        }
        return 0;
    }

    /* ---- Event 2: die ---- */
    case 2: {
        if (s2[0x04] == 1) {
            /* Spawn trail then self-destruct */
            FUN_8003fd24((int32_t *)(s2 + 0x48), 23);
            FUN_800205f8(obj);
            return -1;
        }
        /* Clear flag 0x8000 */
        *(uint32_t *)(s2 + 0x00) &= ~0x8000u;
        return -1;
    }

    /* ---- Event 3: collision ---- */
    case 3: {
        /* param3 is a descriptor; first word is the source object handle */
        int *desc = (int *)(uintptr_t)(uint32_t)param3;
        int source = desc[0];
        uint8_t *s3 = (uint8_t *)(uintptr_t)(uint32_t)source;
        uint8_t source_type = s3[0x04];

        /* Only handle source type 2 (projectile/weapon impact) */
        if (source_type != 2) {
            return (source_type == 7) ? 1 : 0;
        }

        /* Determine effective weapon type index:
         * if obj[8] != 0 and source[6] > 0: use index 14 (generic)
         * else: use obj[0x0a] weapon type */
        int8_t obj_byte8 = *(int8_t *)(s2 + 0x08);
        int16_t src6     = *(int16_t *)(s3 + 0x06);
        uint16_t wtype;
        if (obj_byte8 != 0 && src6 > 0)
            wtype = 14;
        else
            wtype = *(uint16_t *)(s2 + 0x0a);

        if (wtype >= 15)
            return 0;

        /* DAT_800737dc: vehicle bank loaded at function entry (s1 in MIPS) */
        int vehicle_bank = (int)(uintptr_t)(uint32_t)DAT_800737dc;

        switch (wtype) {

        /* wtype 0: heal HP slots of source's 3 linked neighbours */
        case 0: {
            if (s3[0xd0] == 12) {
                /* Heal source directly */
                uint16_t hp    = *(uint16_t *)(s3 + 0x0c);
                uint16_t maxhp = *(uint16_t *)(s3 + 0x0e);
                uint16_t healed = (uint16_t)(hp + 375u);
                if (healed < maxhp) healed = maxhp; /* keep min(new, maxhp) -- note: slt is < so keeps the smaller */
                /* MIPS: slt v0,a0,v1 (a0=healed,v1=maxhp) -> if healed < maxhp keep healed else keep maxhp */
                if ((uint16_t)(hp + 375u) < maxhp)
                    healed = (uint16_t)(hp + 375u);
                else
                    healed = maxhp;
                *(uint16_t *)(s3 + 0x0c) = healed;
            } else {
                /* Distribute 500 HP across 3 linked slot objects at +0xec,+0xf0,+0xf4 */
                uint16_t self_hp = *(uint16_t *)(s3 + 0x0c);
                uint16_t budget  = 500;

                int32_t *slot_ec = (int32_t *)(s3 + 0xec);
                uint8_t *t0 = (uint8_t *)(uintptr_t)(uint32_t)slot_ec[0];
                if (t0) {
                    uint16_t t_hp = *(uint16_t *)(t0 + 0x0c);
                    uint16_t delta = (uint16_t)(self_hp - t_hp);
                    if (delta > budget) delta = budget;
                    *(uint16_t *)(t0 + 0x0c) = (uint16_t)(t_hp + delta);
                    budget = (uint16_t)(budget - delta);
                }
                uint8_t *t1 = (uint8_t *)(uintptr_t)(uint32_t)slot_ec[1];
                if (t1) {
                    uint16_t t_hp = *(uint16_t *)(t1 + 0x0c);
                    uint16_t delta = (uint16_t)(self_hp - t_hp);
                    if (delta > budget) delta = budget;
                    *(uint16_t *)(t1 + 0x0c) = (uint16_t)(t_hp + delta);
                    budget = (uint16_t)(budget - delta);
                }
                uint8_t *t2 = (uint8_t *)(uintptr_t)(uint32_t)slot_ec[2];
                if (t2) {
                    uint16_t t_hp = *(uint16_t *)(t2 + 0x0c);
                    uint16_t delta = (uint16_t)(self_hp - t_hp);
                    /* last slot: cap by self_hp */
                    if ((uint16_t)(t_hp + delta) > self_hp) delta = (uint16_t)(self_hp - t_hp);
                    *(uint16_t *)(t2 + 0x0c) = (uint16_t)(t_hp + delta);
                    (void)budget;
                }
            }
            /* Play SFX 39 */
            {
                int voice = FUN_8004410c();
                FUN_8004483c(voice, (int)(uintptr_t)(uint32_t)uRam000005f8, 39, (int *)(s2 + 0x24));
            }
            /* Spark effect if source[6] == -1 */
            if (*(int16_t *)(s3 + 0x06) == -1)
                FUN_8003fea8((uint32_t)(uintptr_t)(s3 + 0x24), 0x84044040u);

            /* Damage_Apply self, then re-spawn next gen */
            FUN_800205f8(obj);
            {
                int16_t idx = *(int16_t *)(s2 + 0x06);
                int next = FUN_8001ffd4((int)(uintptr_t)DAT_80065a50, (int)idx);
                if (next) {
                    FUN_80020890(next, 600);
                    DAT_80065aac -= 1;
                }
            }
            return -2;
        }

        /* wtype 10: set source[0x120]=900, SFX 38, common tail */
        case 10:
            *(uint16_t *)(s3 + 0x120) = 900;
            goto sfx38_tail;
        /* wtype 11: set source[0x11c]=900 */
        case 11:
            *(uint16_t *)(s3 + 0x11c) = 900;
            goto sfx38_tail;
        /* wtype 12: set source[0x11e]=900 */
        case 12:
            *(uint16_t *)(s3 + 0x11e) = 900;
        sfx38_tail: {
            int voice = FUN_8004410c();
            FUN_8004483c(voice, (int)(uintptr_t)(uint32_t)uRam000005f8, 38, (int *)(s2 + 0x24));
            if (*(int16_t *)(s3 + 0x06) == -1)
                FUN_8003fea8((uint32_t)(uintptr_t)(s3 + 0x24), 0x84044040u);
            FUN_800205f8(obj);
            int16_t idx = *(int16_t *)(s2 + 0x06);
            int next = FUN_8001ffd4((int)(uintptr_t)DAT_80065a50, (int)idx);
            if (next) {
                FUN_80020890(next, 600);
                DAT_80065aac -= 1;
            }
            return -2;
        }

        /* wtype 2: LAB_80031fa0 child, slot 0, kind 0x8011 */
        case 2:
            return common_spawn_child(obj, source, vehicle_bank, 0,
                                      LAB_80031fa0, 0x8011);
        /* wtype 4: LAB_8003302c child, slot 17, kind 0x8012 */
        case 4:
            return common_spawn_child(obj, source, vehicle_bank, 17,
                                      LAB_8003302c, 0x8012);
        /* wtype 6: LAB_800336fc child, slot 7, kind 0x8013 */
        case 6:
            return common_spawn_child(obj, source, vehicle_bank, 7,
                                      LAB_800336fc, 0x8013);
        /* wtype 9: LAB_80034920 child, slot 10, kind 0x8014 */
        case 9:
            return common_spawn_child(obj, source, vehicle_bank, 10,
                                      LAB_80034920, 0x8014);
        /* wtype 8: LAB_8003565c child, slot 13, kind 0x8015 */
        case 8:
            return common_spawn_child(obj, source, vehicle_bank, 13,
                                      LAB_8003565c, 0x8015);

        /* wtype 14: look up callback and slot from bone kind 0x801f */
        case 14: {
            int src_bank = *(int32_t *)(s3 + 0x58);
            uint8_t src_d0 = s3[0xd0];
            intptr_t bone = FUN_8001b038(source, 0x801fu);
            void *cb_ptr = FUN_8003d1e8(src_d0);
            int16_t slot14 = -1;
            if (bone && cb_ptr)
                slot14 = *(int16_t *)((uint8_t *)bone + 0x1a);
            if (slot14 < 0) {
                /* No bone found: just apply damage + respawn */
                FUN_800205f8(obj);
                int16_t idx = *(int16_t *)(s2 + 0x06);
                int next = FUN_8001ffd4((int)(uintptr_t)DAT_80065a50, (int)idx);
                if (next) {
                    FUN_80020890(next, 600);
                    DAT_80065a10 -= 1;
                }
                return -2;
            }
            /* Install the looked-up callback and spawn */
            typedef int (*cb_t)(int,int,int);
            return common_spawn_child(obj, source, src_bank, slot14,
                                      (cb_t)cb_ptr, 0x801fu);
        }

        default:
            return 0;
        }
    } /* case 3 */

    default:
        return 0;
    }
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0
/* --- SLUS_005.10  LAB_8003c61c  (manual disassembly reference) ---
 *
 * 8003c61c: addiu sp,sp,-48
 * 8003c620: addu s2,a0,zero          ; s2=obj
 * 8003c628: addiu v0,zero,1
 * 8003c640: beq a1,v0,0x8003cacc     ; event==1 -> spawn
 * 8003c648: beq a1,zero,0x8003c668   ; event==0 -> tick
 * 8003c650: beq a1,a3,0x8003ca88     ; event==2 -> die  (a3=2)
 * 8003c658: beq a1,v0,0x8003c68c     ; event==3 -> collision  (v0=3)
 * 8003c660: j 0x8003cb40             ; default: return 0
 *
 * Event 0 (tick) @ 8003c668:
 *   lhu v0,66(s2)  ; sub = obj[0x42]
 *   addiu v0,v0,68
 *   beq a2,zero,0x8003cb3c  ; if param3==0 skip InitBoneMatrix
 *   sh v0,66(s2)
 *   jal 8001d708 / move a0,s2 ; Object_InitBoneMatrix(obj)
 *   j 0x8003cb40 / clear v0   ; return 0
 *
 * Event 1 (spawn) @ 8003cacc:
 *   lw v0,88(s2)             ; bank handle
 *   lw v1,0(s2); lw a0,108(s2)
 *   sw v0,2228(gp)           ; DAT_80065BB8 = bank
 *   addiu v0,3 / ori v1,v1,0x380 / sb v0,4(s2) (type=3)
 *   and v1,v1,-9             ; clear bit 3
 *   bne a0,0,skip_init / sw v1,0(s2) / ori v0,0x1f4000 / sw v0,108(s2)
 *   lhu v1,10(s2)            ; weapon type
 *   bne v1,13,exit           ; only type 13 does sound randomisation
 *   lw a0,0(s2) / jal FUN_8003ce24 ; Weapon_RandomSoundIdx(flags)
 *   (table lookup at 0x8005ec84 and sh result to obj[0x0a])
 *
 * Event 2 (die) @ 8003ca88:
 *   lbu v1,4(s2)
 *   bne v1,1,0x8003cab4
 *     jal FUN_8003fd24 (&obj[0x48], 23)
 *     jal Damage_Apply(obj) / return -1
 *   clear flag 0x8000 / return -1
 *
 * Event 3 (collision) @ 8003c68c:
 *   lw a0,0(a2)              ; a0 = source = param3[0]
 *   lbu v1,4(a0)             ; source_type
 *   bne v1,2,0x8003ca78      ; non-2 -> (type==7?1:0)
 *   lb v1,8(s2); lw s1,DAT_800737dc
 *   beq v1,0,get_wtype
 *     addu s3,a0,zero
 *     lh v0,6(s3); bgtz v0,use_14
 *     (fall-through) get_wtype: lhu v1,10(s2) -> wtype
 *     use_14: v1=14
 *   sltiu v0,v1,15; beq v0,0,return0
 *   jump-table dispatch at 0x80010AD0[wtype]
 *
 * Jump table (0x80010AD0):
 *   wtype 0: 0x8003c6fc, 1: cb3c, 2: c880, 3: cb3c, 4: c86c,
 *   5: cb3c, 6: c894, 7: cb3c, 8: c8bc, 9: c8a8,
 *   10: c7d8, 11: c7cc, 12: c7e4, 13: cb3c, 14: c8d0.
 */
#endif /* GHIDRA REF */
