/* wheel_ticks.c -- per-frame physics tick callbacks for vehicle wheels
 * (and similar sub-objects).
 *
 * Source: SLUS_005.10 -- four Ghidra-gap functions discovered while
 * tracing callers of Object_GeneralTick (FUN_80030c08).  None of these
 * appear in analysis/SLUS_005.10/functions.json; the auto-analyser
 * missed their function starts.  Raw MIPS extracted to
 * analysis/SLUS_005.10/mips/{address}.s by tools/scan_writes.py.
 *
 *   Wheel_Tick_Front  gap_8002bdd0  (176 B, 44 instr)   -- DONE
 *   Wheel_Tick_Rear   gap_8002b98c  (652 B, 162 instr)  -- DONE
 *   Wheel_Tick_Stub   gap_80031294  (104 B, 26 instr)   -- DONE (Vehicle_StateTransition)
 *
 * The "front/rear" naming is provisional -- both have the same mode-
 * 0 structure (obj+0xd0 == 12 -> Object_GeneralTick, else cleanup),
 * but the rear variant has additional state-flag handling.  Final
 * naming requires tracing which wheel slot each callback gets
 * installed into.
 *
 * Mode dispatch:
 *   Wheel_Tick_Front:  0=tick, 2=helper, 3=heavy-event, else return 0
 *   Wheel_Tick_Rear:   0=tick, 2=child-spawn/register, 3=SAT-collide, else return 0
 *   Vehicle_StateTransition: 1-arg callback swapper (not a per-frame tick)
 *
 * HIGH confidence: line-by-line MIPS port of all three bodies.
 */
#include <stdint.h>

extern void Object_GeneralTick(uint32_t *obj);              /* FUN_80030c08 */
extern void FUN_8002f998(uint32_t *obj);                    /* state cleanup */
extern uint32_t FUN_800446dc(const int32_t *xyz);           /* SfxCurve_2P_Stereo */
extern void FUN_80044574(int8_t voice, uint32_t pan);
extern void FUN_8002bd84(uint32_t *obj);                    /* sibling helper */
extern void FUN_8002d82c(uint32_t *obj, intptr_t arg);      /* heavy event */
extern void FUN_80020890(uint32_t *obj, int mode);
extern void FUN_80023940(uint32_t *obj);

/* Wheel_Tick_Rear additional externs. */
extern void FUN_8001787c(intptr_t obj, const int32_t *kdtree_node);/* OBB suspension */
extern void *FUN_8001f5a0(intptr_t obj, intptr_t arg);      /* SAT axis select */
extern void FUN_80017ba8(uint32_t *obj, void *axis_result); /* impulse response */
extern uint32_t FUN_8004410c(void);                         /* SfxChannel_Acquire */
extern void FUN_800447e8(uint32_t ctx, uint32_t sndbank,
                         int sfx_id, const int32_t *xyz);   /* positional sound */
extern void FUN_8003fd24(const int32_t *xyz, int arg);      /* Debris_AttachTrack */
extern void FUN_800205f8(uint32_t *obj);                    /* state reset */
extern uint32_t *FUN_8003ff28(uint32_t *child);             /* child spawn/init */
extern uint32_t *FUN_8003fbc8(uint32_t *obj);               /* bone/child lookup */
extern void FUN_8003f89c(uint32_t *chassis, uint32_t *bone,
                         uint32_t *child, int flag);        /* bone attach */
extern uint32_t *FUN_80040a80(void *ctx, int kind,
                              void *tbl);                   /* world-obj register */
extern void FUN_8001d544(uint32_t *obj, uint32_t *reg);     /* obj register link */
extern void FUN_80020744(uint32_t *reg);                    /* Object_RegisterPostUpdate */
extern void FUN_800207c4(uint32_t *reg);                    /* Object_BindFinalize */

/* Globals accessed gp-relative in gap_8002b98c mode 0. */
extern uint32_t uRam000005f8;      /* sound bank selector (gp+0x5F8) */
/* World-registration table pointer + data (mode 2). */
extern uintptr_t DAT_800737d8;     /* ptr to world-object context (gp-based) */
extern uint8_t  DAT_80065710[];    /* world-object data table */
/* Callback installed into child node objects (mode 2). */
extern int FUN_8003cb64(uint32_t *self, int mode, int arg2, int arg3);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

/* ============================================================
 * Wheel_Tick_Front  (gap_8002bdd0, 176 B)
 * Per-frame: if obj+0xd0 (state byte) == 12, run Object_GeneralTick
 * (the integrator).  Otherwise run state-cleanup.  In both cases
 * refresh the SPU pan via SfxCurve_2P_Stereo on the current pos.
 * ============================================================ */
int Wheel_Tick_Front(uint32_t *self, int mode, intptr_t arg2, intptr_t arg3)
{
    (void)arg3;
    uint8_t *s = (uint8_t *)self;

    if (mode == 2) {
        FUN_8002bd84(self);
        return 0;
    }
    if (mode == 3) {
        FUN_8002d82c(self, arg2);
        return 0;
    }
    if (mode != 0) return 0;

    /* Mode 0: physics or cleanup. */
    uint8_t state = *(uint8_t *)(s + 0xd0);
    if (state == 12) {
        Object_GeneralTick(self);
    } else {
        FUN_8002f998(self);
    }
    /* Refresh 3D-positional SPU pan for this object's audio voice. */
    uint32_t pan = FUN_800446dc((const int32_t *)(s + 0x24));
    int8_t   voice = *(int8_t *)(s + 5);
    FUN_80044574(voice, pan);
    return 0;
}

/* Bare-FUN_ alias (the constructor installs this address as the
 * wheel's tick callback). */
int gap_8002bdd0(uint32_t *self, int mode, intptr_t arg2, intptr_t arg3)
{
    return Wheel_Tick_Front(self, mode, arg2, arg3);
}

/* ============================================================
 * Wheel_Tick_Rear  (gap_8002b98c, 652 B, 162 instr)
 *
 * More complex per-frame tick with two mode-0 paths controlled by
 * self[0] bit 0x8000 ("fast-path" flag):
 *
 *   0x8000 SET  -- fast: state check; Object_GeneralTick if state==12,
 *                  else FUN_8002f998 (cleanup).  Return immediately.
 *
 *   0x8000 CLR  -- full: OBB suspension probe (FUN_8001787c) against the
 *                  kdtree node at obj+0x5c + 4, then copy pos (obj+0x24..
 *                  0x2c) to prevPos (obj+0x48..0x50).  Increment per-frame
 *                  counter at obj+0xBE (u16).  If status (i16 at obj+6) < 0
 *                  (damaged state) return.  Otherwise, if counter has just
 *                  hit 540 fire a positional sound event via FUN_800447e8
 *                  with sound-ID 57 and the object's world position, then
 *                  call FUN_8003fd24 + FUN_800205f8 and return -1.
 *
 * Mode 2 -- child-spawn and world-registration:
 *   Iterates 8 child-pointer slots (obj+0xFC..0x118, accessed as
 *   *(s1+0xEC) where s1 walks obj+0x10..0x2C by 4).  For each non-null
 *   slot, calls FUN_8003ff28(child).  For the last 3 slots (node[0..2]
 *   at obj+0x110..0x118, loop index 9..11), if the returned sub-object
 *   has health > 0 and kind != 6, installs FUN_8003cb64 as that
 *   sub-object's tick callback (+0x64).
 *   After the loop, zeroes all 12 u32s from obj+0xEC to obj+0x118
 *   (= the entire child-pointer block).  Calls FUN_8003fbc8 for a bone
 *   lookup and FUN_8003f89c to attach the result.  Then restores
 *   kdtreeRoot, clears obj[0] bits 0x8020, and runs the engine's
 *   world-registration sequence: FUN_80040a80 + FUN_8001d544 +
 *   FUN_80020744 + FUN_800207c4 + FUN_80020890(reg, 480).
 *
 * Mode 3 -- SAT collision:
 *   FUN_8001f5a0(self, arg2) -> axis, then FUN_80017ba8(self, axis).
 *
 * HIGH confidence: line-by-line MIPS port.
 * ============================================================ */
int Wheel_Tick_Rear(uint32_t *self, int mode, intptr_t arg2, intptr_t arg3)
{
    (void)arg3;
    uint8_t *s = (uint8_t *)self;

    /* ---- Mode dispatch ---- */
    if (mode == 2) goto mode2;
    if ((unsigned)mode < 3u) {
        if (mode == 0) goto mode0;
        return 0;   /* mode 1 */
    }
    if (mode == 3) goto mode3;
    return 0;

mode0:;
    /* Fast path: bit 0x8000 in self[0] indicates "passive/suspended". */
    if (self[0] & 0x8000u) {
        uint8_t state = *(uint8_t *)(s + 0xd0);
        if (state == 12) {
            Object_GeneralTick(self);
        } else {
            FUN_8002f998(self);
        }
        return 0;
    }

    /* Full path: OBB suspension probe. */
    {
        uint32_t *kdtree = (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x5c);
        FUN_8001787c((intptr_t)self, (const int32_t *)(kdtree + 1));   /* kdtree node at +4 */

        /* prevPos = pos */
        *(int32_t *)(s + 0x48) = *(int32_t *)(s + 0x24);
        *(int32_t *)(s + 0x4c) = *(int32_t *)(s + 0x28);
        *(int32_t *)(s + 0x50) = *(int32_t *)(s + 0x2c);

        /* Increment audio event counter at obj+0xBE. */
        uint16_t cnt    = *(uint16_t *)(s + 0xbe);
        int16_t  status = *(int16_t  *)(s + 6);
        cnt = (uint16_t)mips_addu_i32((int32_t)(uint32_t)cnt, 1);
        *(uint16_t *)(s + 0xbe) = cnt;

        if (status < 0) return 0;   /* damaged: skip event check */

        /* Counter threshold: 540 (MIPS sign-extends u16 then compares). */
        if ((int16_t)cnt != 540) return 0;

        /* Counter hit 540 -- fire positional sound event. */
        uint32_t sfx_ctx   = FUN_8004410c();
        int32_t *pos_xyz   = (int32_t *)(s + 0x24);
        FUN_800447e8(sfx_ctx, uRam000005f8, 57, pos_xyz);
        FUN_8003fd24(pos_xyz, 0);
        FUN_800205f8(self);
        return -1;
    }

mode2:;
    {
        /* Save kdtreeRoot for restoration after loop. */
        uint32_t saved_kd = *(uint32_t *)(s + 0x5c);

        /* Iterate 8 child-ptr slots: s1 starts at obj+0x10, reads
         * *(s1 + 0xEC) which hits obj+0xFC..0x118 as s1 advances. */
        uint8_t *s1 = s + 0x10;
        for (int i = 4; i < 12; i++, s1 += 4) {
            uint32_t *child = (uint32_t *)(uintptr_t)
                              *(uint32_t *)(s1 + 0xec);
            if (!child) continue;

            uint32_t *v1 = FUN_8003ff28(child);

            /* For the first 5 slots (i=4..8): no conditional install. */
            if (i < 9) continue;

            /* For node slots (i=9..11, obj+0x110..0x118): install
             * callback if health > 0 and kind != 6. */
            if (!v1)                                              continue;
            if (*(uint16_t *)((uint8_t *)v1 + 0x0c) == 0)        continue; /* health */
            if (*(int8_t   *)((uint8_t *)v1 + 0x08) == 6)        continue; /* kind */
            Object_SetCallbackPsxSlot(v1, (uintptr_t)&FUN_8003cb64);
        }

        /* Zero the entire child-pointer block: obj+0xEC..0x118 (12 u32s).
         * MIPS loop: v0 starts at obj+0x2C, decrements by 4, writes
         * *(v0+0xEC)=0 for 12 iterations (s0=11..0). */
        for (int j = 11; j >= 0; j--) {
            *(uint32_t *)(s + 0xec + (uint32_t)j * 4) = 0;
        }

        /* Bone lookup + optional attach. */
        uint32_t *bone = FUN_8003fbc8(self);
        if (bone) {
            FUN_8003f89c(self,
                         (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x58),
                         bone, 0);
        }

        /* Restore kdtreeRoot; clear state bits 0x8020 in obj[0]. */
        *(uint32_t *)(s + 0x5c) = saved_kd;
        self[0] &= ~0x00008020u;    /* mask = 0xffff7fdf */

        /* World-object registration sequence. */
        uint32_t *reg = FUN_80040a80(
            (void *)DAT_800737d8,
            18,
            (void *)DAT_80065710);
        reg[0] |= 0x00020000u;
        FUN_8001d544(self, reg);
        FUN_80020744(reg);
        FUN_800207c4(reg);
        FUN_80020890(reg, 480);
        return 0;
    }

mode3:;
    {
        /* SAT collision: axis select + impulse response. */
        void *axis = FUN_8001f5a0((intptr_t)self, arg2);
        FUN_80017ba8(self, axis);
        return 0;
    }
}

int gap_8002b98c(uint32_t *self, int mode, intptr_t arg2, intptr_t arg3)
{
    return Wheel_Tick_Rear(self, mode, arg2, arg3);
}

/* ============================================================
 * Vehicle_StateTransition  (gap_80031294, 26 instr)
 *
 * NOT a per-frame tick -- this is the callback SWAPPER.  Called when
 * a vehicle changes state to remap obj+0x64 (the tick-callback slot)
 * between LAB_8002e2bc (the audio/dispatch tick) and gap_80030f34
 * (the "rolling" tick that includes Object_GeneralTick).
 *
 * Action:
 *   1. Clear flag 0x2000000 in obj[0].
 *   2. If health (u16 at obj+0xc) == 0:
 *        call FUN_8002bc18(obj)  -- the destroy/cleanup tick.
 *      Else if obj+0xd0 (state byte) == 12:
 *        install gap_80030f34 (rolling tick: calls Object_GeneralTick)
 *      Else:
 *        install LAB_8002e2bc (audio/state tick: no integration)
 *
 * This finally explains the routing: VEHICLES IN STATE 12 ARE THE
 * ONES THAT RUN PER-FRAME PHYSICS.  Newly-constructed vehicles have
 * obj+0xd0 = template[+0xd]; for the player Vehicle the template
 * sets this so they spawn in state 12 directly.  When a vehicle gets
 * damaged or transitions to a non-physical state, this function
 * swaps in LAB_8002e2bc (audio-only) and physics stops.
 *
 * NOTE on signature: the MIPS reads a0 only (no other args).  This
 * is a 1-arg function, not a mode-dispatcher.  The caller invokes it
 * explicitly when a state change is needed; it is not the per-frame
 * callback itself.
 *
 * HIGH confidence: 26 instructions are fully decoded.  The two
 * `lui v0, 0x8003; addiu v0, +0xf34/-0x1d44` pairs encode the
 * absolute addresses 0x80030f34 and 0x8002e2bc (verified by
 * arithmetic).
 */
void Vehicle_StateTransition(uint32_t *self)
{
    extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
    self[0] &= ~0x02000000u;
    if (*(uint16_t *)((uint8_t *)self + 0xc) == 0) {
        extern void FUN_8002bc18(uint32_t *obj);
        FUN_8002bc18(self);
        return;
    }
    extern int Wheel_Tick_Front(uint32_t *self, int mode, intptr_t arg2, intptr_t arg3);
    extern int LAB_8002e2bc(uint32_t *self, int mode, intptr_t arg2, intptr_t arg3);
    /* `gap_80030f34` is at SLUS:0x80030f34 -- the rolling tick (yet
     * to be cleaned in full).  Bridged below via a forward declaration. */
    extern int gap_80030f34(uint32_t *self, int mode, intptr_t arg2, intptr_t arg3);

    uint8_t state = *(uint8_t *)((uint8_t *)self + 0xd0);
    if (state == 12) {
        Object_SetCallbackPsxSlot(self, (uintptr_t)&gap_80030f34);
    } else {
        Object_SetCallbackPsxSlot(self, (uintptr_t)&LAB_8002e2bc);
    }
}

void gap_80031294(uint32_t *self)
{
    Vehicle_StateTransition(self);
}
