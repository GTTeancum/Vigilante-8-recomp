/* vehicle_kill.c -- Vehicle teardown / death handling.
 *
 * Sources:
 *   FUN_8002bc18  Vehicle_Kill          (508 B, 88 instr)
 *   FUN_8002bd84  Vehicle_WheelDestroy  ( 80 B, 14 instr)
 *
 * Vehicle_Kill is the canonical vehicle death handler called by both
 * Vehicle_StateTransition (gap_80031294) when health reaches 0 and
 * Vehicle_WheelDestroy when a wheel-mode-2 event fires.
 *
 * Sequence performed by Vehicle_Kill:
 *   1.  FUN_8003fbc8 bone-index lookup (returns 0 if no bone linkage).
 *       If non-zero, calculate bone-table entry at:
 *           **(self+0x58) + bone_idx * 28 + 0x1C
 *       then call FUN_8001d68c + FUN_8003f4a0 to tear down the link.
 *       The surface tag at bone_entry+0x16 is passed to FUN_80020890 as
 *       the mode (0 if tag == 0xAAAA or no bone).
 *   2.  FUN_80020890(self, mode) -- post-tick event with death mode.
 *   3.  Install gap_8002b98c (Wheel_Tick_Rear) as the new tick callback
 *       -- the "dead vehicle" ticker (0x8002B98C computed via
 *       lui 0x8003; addiu -0x4674).
 *   4.  Zero health (+0xC), inputMul (+0xA6), audio counter (+0xBE).
 *   5.  Set flag 0x8000 in self[0] (fast-path for dead tick), clear
 *       flag 0x4000.  (MIPS: ori 0x8000 then and 0xFFFFBFFF.)
 *   6.  Release two SPU voices: FUN_800441c8(obj+5) + (obj+0xD3).
 *   7.  FUN_80042f5c(self+0xC0) -- death particle / visual effect.
 *   8.  Score / player-death update:
 *       - If status > 0 (enemy kill): zero status, increment
 *         cRam0000067c kill counter; notify FUN_8002ea94(player, 1)
 *         for each player whose +0xE4 == self.
 *       - If status <= 0: set iRam00000624 = 1, flag owner
 *         *(self+0xE0)[0] |= 0x20000, clear self+0xE4.
 *
 * Vehicle_WheelDestroy (FUN_8002bd84) is Wheel_Tick_Front mode-2's
 * sibling helper: retrieves wheel state, formats a debug string via
 * FUN_80053004 + FUN_800129e8, then calls Vehicle_Kill.
 *
 * HIGH confidence: line-by-line MIPS port of both bodies.
 */
#include <stdint.h>

extern uint32_t  FUN_8003fbc8(uint32_t *obj);        /* bone-index lookup */
extern void     *FUN_8001d68c(void *buf, uint32_t *obj,
                               void *bone_entry);     /* bone setup */
extern void      FUN_8003f4a0(uint32_t *bone_tbl, uint16_t idx,
                               void *setup_result);   /* bone tear-down */
extern void      FUN_80020890(uint32_t *obj, int mode);
extern void      FUN_800441c8(int voice);             /* release SPU voice */
extern void      FUN_80042f5c(uint8_t *obj_c0);       /* death particle effect */
extern void      FUN_8002ea94(uint32_t *player_obj, int flag);
extern void      Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* Globals referenced gp-relative in Vehicle_Kill. */
extern int8_t    cRam0000067c;          /* global kill counter (gp+0x67C) */
extern int32_t   iRam00000624;          /* match score flag   (gp+0x624)  */
extern void     *puRam000007d0;         /* player 1 vehicle ptr (gp+0x7D0)*/
extern void     *puRam000007d4;         /* player 2 vehicle ptr (gp+0x7D4)*/
extern void     *Host_HeapBase(void);
extern uint32_t  Host_HeapSize(void);

static int host_heap_contains32(uint32_t p)
{
    uintptr_t base = (uintptr_t)Host_HeapBase();
    uintptr_t addr = (uintptr_t)p;
    return addr >= base && addr < base + Host_HeapSize();
}

/* ============================================================
 * FUN_8002b940  Vehicle_GetStateName  (14 instr)
 *
 * Returns a string pointer describing the vehicle's state for the
 * death debug log.  Routes by status (i16 at obj+6):
 *   -1  -> string at 0x80065704 (special "killed" message)
 *   -2  -> string at 0x800656F8 (special "OOB" message)
 *   else -> *(PTR_s_Chassey_Blue_800567ec + state*8 + 4)
 *           (8-byte entries: [name_ptr, callback_ptr]; returns name)
 *
 * HIGH confidence: 14 instructions decoded in full.
 * ============================================================ */
extern const uint32_t PTR_s_Chassey_Blue_800567ec[];   /* vehicle name table */

int32_t FUN_8002b940(uint32_t *obj)
{
    uint8_t *s = (uint8_t *)obj;
    int16_t  status = *(int16_t *)(s + 6);

    if (status == -1)
        return (int32_t)(intptr_t)(void *)(uintptr_t)0x80065704u;
    if (status == -2)
        return (int32_t)(intptr_t)(void *)(uintptr_t)0x800656f8u;

    /* Normal: look up vehicle name from 8-byte state entry table. */
    uint8_t  state_code = *(uint8_t *)(s + 0xd0);
    /* PTR_s_Chassey_Blue_800567ec[state*2 + 1] = name string ptr */
    return (int32_t)PTR_s_Chassey_Blue_800567ec[state_code * 2u + 1u];
}

/* Vehicle_WheelDestroy additional externs. */
extern int        FUN_80053004(char *buf, const char *fmt, int arg);
extern int        FUN_800129e8(int mode, const char *msg);
extern const char DAT_8006571c[];       /* death debug message format */

/* Forward declaration: gap_8002b98c is the dead-vehicle tick. */
extern int gap_8002b98c(uint32_t *self, int mode, int arg2, int arg3);

/* ============================================================
 * Vehicle_Kill  (FUN_8002bc18, 88 instr, 508 B)
 *
 * Tears down a vehicle and transitions it to the dead-vehicle tick.
 * ============================================================ */
void Vehicle_Kill(uint32_t *self)
{
    uint8_t *s = (uint8_t *)self;
    uint8_t  sp_buf[48];   /* local scratch for FUN_8001d68c */
    int      a1_mode;

    /* Bone-index lookup.  FUN_8003fbc8 returns an index (0 = no bone). */
    uint32_t bone_idx = FUN_8003fbc8(self);
    if (bone_idx) {
        /* Navigate the bone table: entry = base + bone_idx*28 + 0x1C */
        uint32_t *bone_tbl = (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x58);
        uint32_t *bone_arr = (uint32_t *)(uintptr_t)bone_tbl[0];
        uint32_t  off      = bone_idx * 28u + 0x1cu;
        void     *bone_entry = (void *)((uint8_t *)bone_arr + off);

        void *setup = FUN_8001d68c(sp_buf, self, bone_entry);
        FUN_8003f4a0(bone_tbl,
                     *(uint16_t *)((uint8_t *)bone_entry + 0x1a),
                     setup);

        uint16_t surf = *(uint16_t *)((uint8_t *)bone_entry + 0x16);
        a1_mode = (surf == 0xAAAAu) ? 0 : (int)surf;
    } else {
        a1_mode = 0;
    }

    /* Post-tick event; pass surface tag or 0. */
    FUN_80020890(self, a1_mode);

    /* Install dead-vehicle tick callback (0x8002B98C = gap_8002b98c). */
    Object_SetCallbackPsxSlot(s, (uintptr_t)&gap_8002b98c);

    /* Wipe life-cycle fields. */
    *(uint16_t *)(s + 0x0c) = 0;    /* health = 0      */
    *(int16_t  *)(s + 0xa6) = 0;    /* inputMul = 0    */
    *(uint16_t *)(s + 0xbe) = 0;    /* audio counter   */

    /* Set flag 0x8000 (fast-path), clear flag 0x4000. */
    self[0] = (self[0] | 0x8000u) & ~0x4000u;

    /* Release two SPU audio voices. */
    FUN_800441c8((int)(int8_t)*(uint8_t *)(s + 0x05));
    FUN_800441c8((int)(uint8_t)*(uint8_t *)(s + 0xd3));

    /* Trigger death particle / visual effect. */
    FUN_80042f5c(s + 0xc0);

    /* Score and player-death bookkeeping. */
    int16_t status = *(int16_t *)(s + 6);
    if (status > 0) {
        /* Enemy kill: update scoreboard. */
        *(int16_t *)(s + 6) = 0;
        cRam0000067c++;

        uint32_t *p1 = (uint32_t *)puRam000007d0;
        if (p1 && *(uint32_t *)((uint8_t *)p1 + 0xe4) == (uint32_t)(uintptr_t)self)
            FUN_8002ea94(p1, 1);

        uint32_t *p2 = (uint32_t *)puRam000007d4;
        if (p2 && *(uint32_t *)((uint8_t *)p2 + 0xe4) == (uint32_t)(uintptr_t)self)
            FUN_8002ea94(p2, 1);
    } else {
        /* Non-kill (own death or neutral): flag owner, clear target. */
        uint32_t owner32 = *(uint32_t *)(s + 0xe0);
        uint32_t *owner = host_heap_contains32(owner32)
                        ? (uint32_t *)(uintptr_t)owner32
                        : (uint32_t *)0;
        iRam00000624 = 1;
        if (owner)
            owner[0] |= 0x00020000u;
        *(uint32_t *)(s + 0xe4) = 0;
    }
}

void FUN_8002bc18(uint32_t *self) { Vehicle_Kill(self); }

/* ============================================================
 * Vehicle_WheelDestroy  (FUN_8002bd84, 14 instr, 80 B)
 *
 * Wheel_Tick_Front mode-2 helper: get wheel state, print debug msg,
 * then kill the vehicle.
 * ============================================================ */
void Vehicle_WheelDestroy(uint32_t *self)
{
    char    msg[64];
    int32_t state = FUN_8002b940(self);
    FUN_80053004(msg, DAT_8006571c, state);
    FUN_800129e8(0, msg);
    Vehicle_Kill(self);
}

void FUN_8002bd84(uint32_t *self) { Vehicle_WheelDestroy(self); }

/* ============================================================
 * FUN_8002be84  Vehicle_SoftKill  (~40 instr)
 *
 * Called when a vehicle gets "totally" knocked out by a collision or by
 * a heavy-weapons hit while the match is still active. Sequence:
 *   1. FUN_80020890(param_1, 300) -- schedule deferred post-tick event in 300 ticks
 *   2. Install LAB_8002bdd0 as the new tick callback (+0x19 = word offset 25)
 *   3. Zero health (+0xc / word 3), inputMul (+0xa6 / short), flags (+0xb2 / byte)
 *   4. FUN_800441c8(voice at +0xd3) -- stop looping audio voice
 *   5. If voice slot +5 is empty, alloc a new voice; play SFX 0x22 (explosion)
 *   6. If health > 0 OR difficulty == 3: display "XXX wrecked / Total it!" message
 *   7. Spawn particle effect at position DAT_80065710 from pool DAT_800737d8 (type 6)
 *   8. Attach the particle as a child bone via FUN_8001b038 / FUN_8001b2fc
 *   9. Activate particle via FUN_80020744 + FUN_800207c4
 *
 * HIGH: direct port from Ghidra pseudoC (8002be84.c); all field offsets
 * verified against Vehicle_Kill and surrounding vehicle object layout.
 * ============================================================ */

/* Additional externs needed by Vehicle_SoftKill
 * (FUN_8002b940 and FUN_800129e8 already declared earlier in this file) */
extern intptr_t FUN_8001b038(uint32_t *obj, uint16_t key); /* bone table query; retval from FUN_8001afa0 */
extern uint32_t *FUN_800407b4(uint32_t pool, uint16_t type, uint32_t *pos_tbl);
extern void    FUN_8001b2fc(uint32_t *parent, const void *child, uint32_t *sibling);
extern void    FUN_80020744(uint32_t *obj);    /* Object_RegisterPostUpdate */
extern void    FUN_800207c4(uint32_t *obj);    /* Object_BindFinalize */
extern uint32_t FUN_8004410c(void);             /* Audio_AllocVoice */
extern int     FUN_8004483c(uint32_t voice, uint32_t bank, int clip, void *pos);
extern void    LAB_8002bdd0(void);              /* wrecked-vehicle animation tick */
extern void    Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

extern int8_t   cRam00000015;           /* difficulty byte */
extern uint32_t uRam000005f8;           /* default audio bank handle */
extern uint32_t DAT_800737d8;           /* world particle pool handle */
extern uint8_t  DAT_80065710[];         /* world particle position table */

void FUN_8002be84(intptr_t param_1)
{
    char     cVar1;
    uint32_t uVar2;
    uint32_t *puVar3;
    int      iVar4;
    char     acStack_50[64];
    uint8_t *s = (uint8_t *)(uintptr_t)param_1;

    FUN_80020890((uint32_t *)s, 300);
    /* Install wrecked-vehicle tick callback at +0x19 (byte offset 100) */
    Object_SetCallbackPsxSlot(s, (uintptr_t)&LAB_8002bdd0);
    *(uint16_t *)(s + 0x0c) = 0;   /* zero health */
    *(uint16_t *)(s + 0xa6) = 0;   /* zero inputMul */
    *(uint8_t  *)(s + 0xb2) = 0;   /* zero flags */
    FUN_800441c8(*(uint8_t *)(s + 0xd3)); /* stop loop voice */
    iVar4 = (int)*(int8_t *)(s + 5);
    if (iVar4 == 0) {
        cVar1 = (char)FUN_8004410c();
        *(int8_t *)(s + 5) = cVar1;
        iVar4 = (int)cVar1;
    }
    FUN_8004483c((uint32_t)iVar4, uRam000005f8, 0x22,
                 (void *)(s + 0x24));

    if ((*(int16_t *)(s + 6) > 0) || (cRam00000015 == 3)) {
        uVar2 = (uint32_t)(uintptr_t)FUN_8002b940((uint32_t *)s);
        FUN_80053004(acStack_50, "%s wrecked\nTotal it!", uVar2);
        iVar4 = 0;
        if (*(int16_t *)(s + 6) < 1)
            iVar4 = (int)*(int16_t *)(s + 6) + 3;
        FUN_800129e8(iVar4, acStack_50);
    }

    puVar3 = (uint32_t *)FUN_800407b4(DAT_800737d8, 6, (uint32_t *)DAT_80065710);
    puVar3[0] |= 0x20000u;
    intptr_t joint = FUN_8001b038((uint32_t *)s, 0x8101);
    FUN_8001b2fc((uint32_t *)s, (const void *)(uintptr_t)joint, puVar3);
    FUN_80020744(puVar3);
    FUN_800207c4(puVar3);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8002be84  (from analysis/SLUS_005.10/decomp/8002be84.c) ---
 * 8002be84: ...  FUN_80020890(param_1, 300)
 * 8002be8c: ...  *(ptr*)(param_1+100) = &LAB_8002bdd0
 * 8002be94: ...  *(u16*)(param_1+0xc) = 0;  *(u16*)(param_1+0xa6) = 0;
 * 8002be9c: ...  *(u8*)(param_1+0xb2) = 0
 * 8002bea4: ...  FUN_800441c8(*(u8*)(param_1+0xd3))
 * 8002beb0: ...  if (*(char*)(param_1+5)==0) { voice=FUN_8004410c(); *(char*)(+5)=voice; }
 * 8002bec0: ...  FUN_8004483c(iVar4, uRam000005f8, 0x22, param_1+0x24)
 * 8002bec8: ...  if ((*(short*)(param_1+6)>0)||(cRam15==3)) { sprintf msg; FUN_800129e8; }
 * 8002bed4: ...  puVar3 = FUN_800407b4(DAT_800737d8, 6, &DAT_80065710)
 * 8002bee0: ...  *puVar3 |= 0x20000; uVar2=FUN_8001b038(param_1,0x8101);
 * 8002bef0: ...  FUN_8001b2fc(param_1,uVar2,puVar3); FUN_80020744(puVar3); FUN_800207c4(puVar3)
 */

#endif  /* GHIDRA REF */
