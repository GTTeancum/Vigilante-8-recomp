/* vehicle_event.c -- Vehicle per-tick event dispatch (state-event handler).
 *
 * Source: SLUS_005.10  FUN_8002d494  (231 instr, ~924 B)
 *
 * Called each physics tick from the vehicle roller / vehicle tick chain with
 * a pointer to the current event-node.  The event-node carries a 32-bit
 * flag word at [+8] whose bits select which state-change actions to perform.
 *
 * a0 = self        (Vehicle *)
 * a1 = event_node  (u32[3]: [0]=??, [4]=hit_data, [8]=event_flags)
 *
 * Event-flag bits in event_node[+8]:
 *   bit  6  (0x000040): power toggle (set/clear 0x20000000 in self->flags)
 *   bits 19-20 (0x180000): cycle node slot (0x80000=fwd, 0x100000=rev alone)
 *   bit 17 (0x020000): projectile-hit -- dispatch mode 9 to matching node
 *   bit 21 (0x200000): target-cycle -- call FindNextTarget, store new target
 *   bit 18 (0x040000): weapon-fired -- play fire SFX, update lock-on
 *   bit  2 (0x000004): attached-weapon activate (forwarded to Reticle_UpdateLock)
 *   bit  1 (0x000002): built-in gun activate (mode 0xB to chassis sub-object)
 *
 * Supporting sub-functions (all defined in this translation unit's deps):
 *   FUN_8002cf90  Vehicle_CycleNodeSlot
 *   FUN_8002ea94  Vehicle_Retarget
 *   FUN_8002ed34  FindNextTarget
 *   FUN_8002ce68  Reticle_UpdateLock
 *   FUN_8001d708  Object_SyncAnchorPos
 *   FUN_8004410c  Audio_AllocVoice
 *   FUN_8004445c  Audio_PlaySfxVol
 *   FUN_800447e8  Audio_PlaySfx3D
 *
 * HIGH confidence: line-by-line MIPS port; cross-checked against Ghidra.
 */
#include <stdint.h>

extern int     FUN_8002cf90(uint32_t *self, int step);
extern void    FUN_8002ea94(uint32_t *self, int flag);
extern uint32_t *FUN_8002ed34(uint32_t *self, uint32_t *hint);
extern void    FUN_8002ce68(uint32_t *self, int activate);
extern void    FUN_8001d708(uint32_t *obj);

/* Audio helpers */
extern uint32_t FUN_8004410c(void);                     /* allocate SPU voice      */
extern void     FUN_8004445c(int ch, uint32_t bank, int sfxId);   /* play SFX vol  */
extern void     FUN_800447e8(int ch, uint32_t bank, int sfxId,
                              const int32_t *pos);               /* play SFX 3D    */

/* Global sound bank identifier (gp+0x5f8). */
extern uint32_t uRam000005f8;

/* Frame counter used for per-vehicle retarget phase-stagger (gp+0xc). */
extern int32_t  iRam0000000c;
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

void FUN_8002d494(uint32_t *self, uint32_t *event_node)
{
    uint8_t  *s         = (uint8_t *)self;
    uint32_t  ev_flags  = event_node[2];           /* event_node[+8] */
    uint8_t  *blk       = s + 0xa4;               /* vehicle stat block */

    /* ----------------------------------------------------------------
     * 1. Power toggle: if bit 6 (0x40) changes, sync the 0x20000000
     *    flag and update two linked sub-object fields accordingly.
     * ---------------------------------------------------------------- */
    uint32_t want_pow  = (ev_flags & 0x40u) ? 0x20000000u : 0u;
    if ((self[0] & 0x20000000u) != want_pow) {
        /* Toggle bit 29 in self->flags. */
        self[0] ^= 0x20000000u;

        /* ptr_f8: sub-object pointer stored at self+0xf8 */
        uint32_t *ptr_f8 = (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0xf8);
        *(int16_t *)((uint8_t *)ptr_f8 + 0x42) = want_pow ? (int16_t)0x800 : 0;

        /* ptr_e0: sub-object pointer stored at self+0xe0 */
        uint32_t *ptr_e0 = (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0xe0);
        *(int16_t *)((uint8_t *)ptr_e0 + 0x8e) = want_pow ? (int16_t)0x800 : 0;

        /* Negate the angular velocity component at ptr_e0+0x98. */
        *(int32_t *)((uint8_t *)ptr_e0 + 0x98) =
            mips_subu_i32(0, *(int32_t *)((uint8_t *)ptr_e0 + 0x98));

        /* Sync the anchor position for the sub-object at self+0xf8. */
        FUN_8001d708(ptr_f8);
    }

    /* ----------------------------------------------------------------
     * 2. Node-slot cycling: bits 0x80000 / 0x100000 trigger
     *    Vehicle_CycleNodeSlot with step +1 or -1.
     *    Clears bit 28 of self->flags after cycling.
     *    Plays a click SFX; if cycle succeeded, notifies the new
     *    reticle sub-object with mode 0xA (target-change).
     * ---------------------------------------------------------------- */
    if (ev_flags & 0x180000u) {
        int step = (ev_flags & 0x80000u) ? 1 : -1;
        int found = FUN_8002cf90(self, step);

        /* Clear "targeting-locked" flag (bit 28). */
        self[0] &= ~0x10000000u;

        /* Play cycle click: clip 0x16 if found, 0x15 if not. */
        uint32_t voice = FUN_8004410c();
        int      clip  = found ? 0x16 : 0x15;
        FUN_8004445c((int)voice, uRam000005f8, clip);

        /* If cycle found a valid slot, dispatch mode 0xA to new reticle. */
        if (found) {
            uint8_t  db  = *(uint8_t *)(blk + 0x0f);   /* updated damageBits */
            uint32_t off = ((uint32_t)db + 9u) << 2;
            uint32_t *node = (uint32_t *)(uintptr_t)*(uint32_t *)(s + off + 0xec);
            if (node) {
                typedef int (*TickFn)(intptr_t, int, intptr_t);
                TickFn cb = (TickFn)Object_CallbackFromPsxSlot(node);
                if (cb)
                    cb((intptr_t)node, 0xa, (intptr_t)self);
            }
        }
    }

    /* ----------------------------------------------------------------
     * 3. Periodic retarget: when bit 28 is clear and the frame-counter
     *    phase matches this vehicle's slot byte (self+9), call
     *    Vehicle_Retarget(self, 0) to find a new closest enemy.
     * ---------------------------------------------------------------- */
    if (!(self[0] & 0x10000000u)) {
        uint8_t phase = *(uint8_t *)(s + 9);               /* per-vehicle phase */
        if (((uint32_t)mips_subu_i32(iRam0000000c, (int32_t)phase) & 0x3fu) == 0)
            FUN_8002ea94(self, 0);
    }

    /* ----------------------------------------------------------------
     * 4. Target cycling: bit 21 (0x200000).
     *    Finds the next target beyond the current one; updates
     *    self->currentTarget and resets the lock timer.
     * ---------------------------------------------------------------- */
    if (ev_flags & 0x200000u) {
        /* Use current target as hint if bit 28 is set (targeting active)
         * and the lock timer < 0x100. */
        uint32_t *hint = NULL;
        if (self[0] & 0x10000000u) {
            int16_t timer = *(int16_t *)(blk + 0x18);   /* self+0xbc */
            if (timer < 0x100)
                hint = (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0xe4);
        }

        uint32_t *new_tgt = FUN_8002ed34(self, hint);
        *(uint32_t *)(s + 0xe4)   = (uint32_t)(uintptr_t)new_tgt;
        *(int16_t  *)(blk + 0x18) = 0;               /* reset lock timer */
        self[0] |= 0x10000000u;                       /* set targeting-active */

        /* Play target-acquired click. */
        uint32_t voice = FUN_8004410c();
        FUN_8004445c((int)voice, uRam000005f8, 0x16);
    }

    /* ----------------------------------------------------------------
     * 5. Fire event: bit 18 (0x40000).
     *    Plays fire SFX unless lock is in-progress (node exists but
     *    node->status == 0).
     * ---------------------------------------------------------------- */
    if (ev_flags & 0x40000u) {
        uint8_t   db   = *(uint8_t *)(blk + 0x0f);
        uint32_t *node = (uint32_t *)(uintptr_t)
                         *(uint32_t *)(s + 0x110u + (uint32_t)db * 4u);
        /* Play clip 0x15 unless node exists AND node is not yet locked. */
        if (!node || *(int16_t *)((uint8_t *)node + 0x06) != 0) {
            uint32_t voice = FUN_8004410c();
            FUN_8004445c((int)voice, uRam000005f8, 0x15);
        }
    }

    /* ----------------------------------------------------------------
     * 6. Always: update attached-weapon/lock-on node (activate if bit 2 set).
     * ---------------------------------------------------------------- */
    FUN_8002ce68(self, (int)(ev_flags & 0x4u));

    /* ----------------------------------------------------------------
     * 7. Immunity-frames counter: if counter > 0, decrement and skip
     *    the projectile-hit block (block 8).
     * ---------------------------------------------------------------- */
    uint16_t ctr = *(uint16_t *)(blk + 0x0a);   /* self+0xae */
    if (ctr != 0) {
        *(uint16_t *)(blk + 0x0a) = (uint16_t)(ctr - 1u);
        goto chassis_notify;                     /* skip hit block */
    }

    /* ----------------------------------------------------------------
     * 8. Projectile-hit: bit 17 (0x20000), event_node[+4] > 0x100.
     *    Scan all three node slots; dispatch mode 9 (hit) to first
     *    healthy one that acknowledges.  Play 3D SFX, set counter.
     * ---------------------------------------------------------------- */
    if ((ev_flags & 0x20000u) && ((int32_t)event_node[1] > 0x100)) {
        int     s2  = 0;
        int32_t s1b = 0x24;   /* byte offset: 0x24, 0x28, 0x2c */
        do {
            uint32_t *node = (uint32_t *)(uintptr_t)
                             *(uint32_t *)(s + (uint32_t)s1b + 0xecu);
            if (node && *(uint16_t *)((uint8_t *)node + 0x0c) != 0) {
                /* Call node tick with mode 9 (take-hit). */
                int s0 = 0;
                typedef int (*TickFn)(intptr_t, int, intptr_t);
                TickFn cb = (TickFn)Object_CallbackFromPsxSlot(node);
                if (cb)
                    s0 = cb((intptr_t)node, 9, (intptr_t)event_node[1]);

                if (s0 != 0) {
                    /* Node responded: play 3D SFX at player position. */
                    uint32_t voice = FUN_8004410c();
                    int clip = (s0 >= 0) ? 0x2a : 0x15;
                    FUN_800447e8((int)voice, uRam000005f8, clip,
                                 (const int32_t *)(s + 0x24));

                    /* Set immunity counter if hit was positive (normal hit). */
                    if (s0 > 0)
                        *(int16_t *)(blk + 0x0a) = (int16_t)s0;

                    /* Consume the event data. */
                    event_node[1] = 0;
                    goto chassis_notify;
                }
            }
            s2++;
            s1b = mips_addu_i32(s1b, 4);
        } while (s2 < 3);
    }

chassis_notify:
    /* ----------------------------------------------------------------
     * 9. Chassis sub-object notification: dispatch mode 4 (idle) or
     *    0xB (built-in gun fire) to the chassis node at self+0x10c.
     * ---------------------------------------------------------------- */
    {
        int mode = (ev_flags & 0x2u) ? 0xb : 4;
        uint32_t *chassis = (uint32_t *)(uintptr_t)*(uint32_t *)(s + 0x10c);
        if (chassis) {
            typedef int (*TickFn)(intptr_t, int, intptr_t);
            TickFn cb = (TickFn)Object_CallbackFromPsxSlot(chassis);
            if (cb)
                cb((intptr_t)chassis, mode, (intptr_t)self);
        }
    }

    /* ----------------------------------------------------------------
     * 10. Lock-timer advance: increment self+0xbc by 8 while < 0x100.
     * ---------------------------------------------------------------- */
    {
        int16_t  timer_s = *(int16_t  *)(blk + 0x18);  /* signed compare */
        uint16_t timer_u = *(uint16_t *)(blk + 0x18);  /* unsigned add   */
        if (timer_s < 0x100)
            *(uint16_t *)(blk + 0x18) = (uint16_t)mips_addu_i32((int32_t)timer_u, 8);
    }
}
