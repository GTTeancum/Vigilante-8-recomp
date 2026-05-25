/* vehicle_node.c -- Reticle sub-object management and node-slot cycling.
 *
 * Sources:
 *   FUN_8002ce68  Reticle_UpdateLock   (75 instr, ~300 B)
 *   FUN_8002cf90  Vehicle_CycleNodeSlot (51 instr, ~204 B)
 *
 * These two functions maintain the lock-on reticle attached to a vehicle's
 * damageBits-keyed sub-object node.  They are called by the vehicle state-
 * event handler (FUN_8002d494) each tick the player has a target.
 *
 * FUN_8002ce68  Reticle_UpdateLock:
 *   a0 = self (Vehicle *)
 *   a1 = activate (1 = begin locking; 0 = deactivate)
 *
 *   Looks up the reticle sub-object from self+0x110[damageBits].
 *   Activate path: if node health > 0 AND node status == 0:
 *     dispatch mode 0xB to the node (lock-begin event), store return value
 *     in node->status.  Then, if player is alive and node doesn't have the
 *     0x40000 flag, add lock-on progress:
 *       rate = (2 - cRam00000016) << (self->flags & 0x40000 ? 5 : 6)
 *       node->status += rate + (V8_RandNext() * rate) >> 15
 *   Deactivate path: dispatch mode 0 to the node.
 *
 * FUN_8002cf90  Vehicle_CycleNodeSlot:
 *   a0 = self (Vehicle *)
 *   a1 = step (callers pass 1 = advance by one slot)
 *
 *   Advances damageBits (self+0xb3) by `step` mod 3 (using the
 *   0x55555556 multiply-high trick for integer division-by-3), then
 *   searches for the next non-null slot in self+0x110[damageBits].
 *   Tries up to 3 times total (3 mod-3 positions = full cycle).
 *   Returns 1 if a non-null node was found within 2 advances, 0 otherwise
 *   (found on 3rd try = full circle, or not found at all).
 *
 * HIGH confidence: line-by-line MIPS port of both bodies.
 */
#include <stdint.h>

/* V8_RandNext (FUN_80017160) -- 16-bit LCG/xorshift. */
extern uint32_t V8_RandNext(void);

/* gp+0x16: difficulty byte / countdown (0, 1, or 2; used in lock rate). */
extern int8_t cRam00000016;
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_sll_i32(int32_t v, unsigned sh)
{
    return (int32_t)((uint32_t)v << sh);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

/* ============================================================
 * FUN_8002ce68  Reticle_UpdateLock  (75 instr)
 * ============================================================ */
void FUN_8002ce68(uint32_t *self, int activate)
{
    uint8_t *s = (uint8_t *)self;

    /* Locate reticle node: self + 0x110 + damageBits * 4 */
    uint8_t   db   = *(uint8_t *)(s + 0xb3);
    uint32_t *node = (uint32_t *)(uintptr_t)
                     *(uint32_t *)(s + 0x110u + (uint32_t)db * 4u);
    if (!node)
        return;

    typedef int (*TickFn)(uint32_t *, int, uint32_t *);
    TickFn cb = (TickFn)Object_CallbackFromPsxSlot(node);

    /* Deactivate conditions: flag==0, OR node health (i16@+0xc)==0,
     * OR node status (i16@+0x06) already non-zero (already locked). */
    if (!activate
        || *(int16_t *)((uint8_t *)node + 0x0c) == 0
        || *(int16_t *)((uint8_t *)node + 0x06) != 0) {
        /* Deactivate: dispatch mode 0 */
        if (cb)
            cb(node, 0, self);
        return;
    }

    /* Activate: dispatch mode 0xB (lock-begin), store return in node->status */
    if (cb)
        *(int16_t *)((uint8_t *)node + 0x06) =
            (int16_t)cb(node, 0xb, self);
    else
        *(int16_t *)((uint8_t *)node + 0x06) = 0;

    /* Increment lock progress only while player is alive and node is ready */
    if (*(int16_t *)(s + 0x06) <= 0)       /* player status (i16@+6) */
        return;
    if (node[0] & 0x40000u)                /* node->flags disabling bit */
        return;

    /* rate = (2 - cRam00000016) << (self->flags & 0x40000 ? 5 : 6) */
    int32_t val  = mips_subu_i32(2, (int32_t)(int8_t)cRam00000016);
    int32_t rate = (self[0] & 0x40000u) ? mips_sll_i32(val, 5) : mips_sll_i32(val, 6);

    /* node->status += rate + (V8_RandNext() * rate) >> 15 */
    int32_t   rng     = (int32_t)V8_RandNext();
    int32_t   scaled  = mips_mult_lo_i32(rng, rate) >> 15;
    uint32_t  cur     = (uint32_t)*(uint16_t *)((uint8_t *)node + 0x06);
    *(uint16_t *)((uint8_t *)node + 0x06) =
        (uint16_t)mips_addu_i32((int32_t)cur, mips_addu_i32(rate, scaled));
}

/* ============================================================
 * FUN_8002cf90  Vehicle_CycleNodeSlot  (51 instr)
 *
 * Fixed-point mod-3 via 0x55555556 multiply-high trick:
 *   q = mfhi(v * 0x55555556) - (v >> 31)   // = floor(v / 3)
 *   r = v - q*3                             // = v % 3
 * ============================================================ */
int FUN_8002cf90(uint32_t *self, int step)
{
    uint8_t *s   = (uint8_t *)self;
    uint8_t *blk = s + 0xa4;               /* offset block: self+0xa4 */
    int      a2;

    /* MIPS: a1 += 3 (makes negative damageBits wrap correctly mod 3). */
    step = mips_addu_i32(step, 3);

    /* -------- Helper macro: advance damageBits by step mod 3 ---------- */
#define NEXT_SLOT(nb_out) do {                                        \
    uint32_t _v = (uint32_t)mips_addu_i32((int32_t)*(uint8_t *)(blk + 0x0f), step); \
    uint32_t _hi = (uint32_t)(((uint64_t)_v * 0x55555556ull) >> 32); \
    int32_t  _q  = (int32_t)_hi - (int32_t)(_v >> 31);              \
    (nb_out) = (uint8_t)mips_subu_i32((int32_t)_v, mips_mult_lo_i32(_q, 3)); \
    *(uint8_t *)(blk + 0x0f) = (nb_out);                             \
} while (0)

    /* -------- Iteration 1 (before retry loop) -------- */
    {
        uint8_t nb;
        NEXT_SLOT(nb);
        uint32_t *node = (uint32_t *)(uintptr_t)
                         *(uint32_t *)(s + (((uint32_t)nb + 9u) << 2) + 0xecu);
        a2 = 0;
        if (node)
            goto done;
        a2 = 1;
    }

    /* -------- Retry loop (iterations 2 and 3) -------- */
    while (a2 < 3) {
        uint8_t nb;
        NEXT_SLOT(nb);
        uint32_t *node = (uint32_t *)(uintptr_t)
                         *(uint32_t *)(s + (((uint32_t)nb + 9u) << 2) + 0xecu);
        a2 = mips_addu_i32(a2, 1);               /* MIPS: delay slot of beq always fires */
        if (node) {
            a2 = mips_subu_i32(a2, 1);           /* undo: delay-slot incr followed by explicit decr */
            break;
        }
    }

#undef NEXT_SLOT

done:
    /* Return 1 if found within 2 advances (a2 < 2), else 0. */
    return (a2 < 2) ? 1 : 0;
}
