/* ai_target.c -- AI target acquisition.
 *
 * Source: SLUS_005.10  FUN_80022e90.
 *
 * Each in-game tick, every AI-driven vehicle that meets the "experience
 * >= 6" gate (vehicle.controlFlags @ +0xd0) attempts to acquire a new
 * target within a ~1024-world-unit (0xfa000) cube around its current
 * position. Steps:
 *
 *   1. Gate on g_matchMode != 0 (must be in active match).
 *   2. Gate on self.controlFlags @ +0xd0 >= 6 (AI skill threshold).
 *   3. Pick a candidate type from a per-damage-state table at self+0x110
 *      keyed by self.damageBits (+0xb3). The 4-byte stride suggests a
 *      table of u32 type-id pointers.
 *   4. If the candidate's state byte is 0x04 (= "alive AI-able"), do
 *      an AABB-sweep against the world kd-tree (uRam000006fc) within
 *      ±0xfa000 cube, with screen-projection bounds sRam000006f0 /
 *      sRam000007dc as the frustum predicate.
 *   5. If the closest hit is itself within 0xfa000 in every axis of
 *      self's current target (so target switch is "smooth"), store the
 *      new target into self+0xe4 (currentTarget) and dispatch event
 *      11 (= "I'm-targeting-you") via the candidate's per-tick callback.
 *   6. Then a V8_RandNext() roll for further behavior (state machine
 *      branches continue past this excerpt).
 *
 * MED confidence (top half; tail-state machine is pass-3 work).
 *
 * NEW STRUCT FIELD CONFIRMATION:
 *   - Vehicle.+0xb3 (damageBits): used as index into the per-damage
 *     candidate table at +0x110.
 *   - Vehicle.+0xd0 (controlFlags): the AI skill / experience byte.
 *   - Vehicle.+0xe4 (currentTarget): the acquired target pointer.
 *
 * These promote 3 previously-LOW fields to HIGH.
 */
#include <stdint.h>
#include "structs.h"

extern uint32_t V8_RandNext(void);
extern int  TreeAABB_FirstHit(uintptr_t treeRoot, int16_t screenW, int16_t screenH, int32_t *bbox4);  /* FUN_80021a30 */
extern int8_t cRam00000015;          /* g_matchMode */
extern int16_t sRam000006f0;         /* per-frame screen X bound */
extern int16_t sRam000007dc;         /* per-frame screen Y bound */
extern uintptr_t uRam000006fc;        /* world kd-tree root */

#define VEHICLE_AI_GATE   6
#define AI_TARGET_RADIUS  0xfa000

int Vehicle_TryAcquireTarget(uint8_t *self)
{
    if (cRam00000015 == 0)                return 0;
    if (((uint8_t *)self)[0xd0] < VEHICLE_AI_GATE) return 0;

    /* candidate = self.candidateTable[self.damageBits] */
    int candidate = *(int *)(self + ((uint8_t *)self)[0xb3] * 4 + 0x110);
    if (candidate == 0)                   return 0;
    if (*(int8_t *)(candidate + 8) != 4)  return 0;

    /* AABB sweep within ±0xfa000 of self.pos. */
    int32_t bbox[4] = {
        *(int32_t *)(self + 0x24) - AI_TARGET_RADIUS,
        *(int32_t *)(self + 0x24) + AI_TARGET_RADIUS,
        *(int32_t *)(self + 0x2c) - AI_TARGET_RADIUS,
        *(int32_t *)(self + 0x2c) + AI_TARGET_RADIUS,
    };
    int hit = TreeAABB_FirstHit(uRam000006fc, sRam000006f0, sRam000007dc, bbox);
    if (hit == 0) return 0;

    /* "Smooth-switch" gate: new target must be within ±0xfa000 of
     * current-target's pos in every axis. */
    int curTarget = *(int *)(self + 0xe4);
    for (int axisOff = 0x24; axisOff <= 0x2c; axisOff += 4) {
        int32_t d = *(int32_t *)(hit + axisOff) - *(int32_t *)(curTarget + axisOff);
        if (d < 0) d = -d;
        if (d >= AI_TARGET_RADIUS) return 0;
    }
    *(int *)(self + 0xe4) = hit;       /* commit new target */

    /* Dispatch event 11 to the new target. */
    typedef int16_t (*TickFn)(int obj, int event, void *arg);
    TickFn fn = *(TickFn *)(candidate + 100);
    if (fn != NULL) fn(candidate, 0xb, self);

    (void)V8_RandNext();   /* RNG advance (consumer not shown in excerpt) */
    return 1;
}
