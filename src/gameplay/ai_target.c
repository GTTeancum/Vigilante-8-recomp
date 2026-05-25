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
extern int  FUN_80021a30(int *treeRoot, int param_2, int param_3, int *bbox4); /* TreeAABB_FirstHit */
#define TreeAABB_FirstHit(r,w,h,b) FUN_80021a30((int*)(uintptr_t)(r),(int)(w),(int)(h),(b))
extern int8_t cRam00000015;          /* g_matchMode */
extern int16_t sRam000006f0;         /* per-frame screen X bound */
extern int16_t sRam000007dc;         /* per-frame screen Y bound */
extern uintptr_t uRam000006fc;        /* world kd-tree root */
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);
extern void *Host_HeapBase(void);
extern uint32_t Host_HeapSize(void);

static int host_ai_object_ptr_ok(uintptr_t p)
{
    uintptr_t base = (uintptr_t)Host_HeapBase();
    uintptr_t end = base + Host_HeapSize();

    return p >= base && p + 0x124 <= end;
}

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_sll_i32(int32_t v, int shift)
{
    return (int32_t)((uint32_t)v << shift);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

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
        mips_subu_i32(*(int32_t *)(self + 0x24), AI_TARGET_RADIUS),
        mips_addu_i32(*(int32_t *)(self + 0x24), AI_TARGET_RADIUS),
        mips_subu_i32(*(int32_t *)(self + 0x2c), AI_TARGET_RADIUS),
        mips_addu_i32(*(int32_t *)(self + 0x2c), AI_TARGET_RADIUS),
    };
    int hit = TreeAABB_FirstHit(uRam000006fc, sRam000006f0, sRam000007dc, bbox);
    if (hit == 0) return 0;
    if (!host_ai_object_ptr_ok((uintptr_t)hit)) return 0;

    /* "Smooth-switch" gate: new target must be within ±0xfa000 of
     * current-target's pos in every axis. */
    int curTarget = *(int *)(self + 0xe4);
    for (int axisOff = 0x24; axisOff <= 0x2c; axisOff += 4) {
        int32_t d = mips_subu_i32(*(int32_t *)(hit + axisOff),
                                  *(int32_t *)(curTarget + axisOff));
        if (d < 0) d = mips_subu_i32(0, d);
        if (d >= AI_TARGET_RADIUS) return 0;
    }
    *(int *)(self + 0xe4) = hit;       /* commit new target */

    /* Dispatch event 11 to the new target. */
    typedef int16_t (*TickFn)(int obj, int event, void *arg);
    TickFn fn = (TickFn)Object_CallbackFromPsxSlot((const void *)(uintptr_t)candidate);
    if (fn != NULL) fn(candidate, 0xb, self);

    (void)V8_RandNext();   /* RNG advance (consumer not shown in excerpt) */
    return 1;
}

/* ================================================================
 * FUN_80022e90 -- direct Ghidra port, bit-exact.
 *
 * Differences from Vehicle_TryAcquireTarget above (which is MED):
 *   - self+0xe4 is temporarily aliased to the candidate hit then
 *     restored to the original currentTarget at the end (no permanent
 *     commit).
 *   - The RNG advance updates candidate[+6] (a short field) with
 *     sVar1 + (rand * 0x80 * (2 - cRam00000016) >> 0xf).
 *
 * HIGH confidence: line-for-line from Ghidra ref.
 * ================================================================ */
extern uint32_t FUN_80017160(void);          /* V8_RandNext (hex alias) */
extern int8_t   cRam00000016;                /* difficulty (1..3) */

uint32_t FUN_80022e90(int param_1)
{
    int16_t sVar1;
    int     iVar2;
    int     iVar3;
    int     iVar4;
    int     iVar5;
    int32_t local_28, local_24, local_20, local_1c;

    if (cRam00000015 == '\0') {
        if (*(uint8_t *)(uintptr_t)(param_1 + 0xd0) < 6) {
            return 0;
        }
        iVar5 = *(int *)(uintptr_t)(
                  param_1 + (uint32_t)*(uint8_t *)(uintptr_t)(param_1 + 0xb3) * 4 + 0x110);
        if (iVar5 != 0) {
            if (*(int8_t *)(uintptr_t)(iVar5 + 8) != 4) return 0;
            local_28 = mips_subu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x24), 0xfa000);
            local_24 = mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x24), 0xfa000);
            local_20 = mips_subu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x2c), 0xfa000);
            local_1c = mips_addu_i32(*(int32_t *)(uintptr_t)(param_1 + 0x2c), 0xfa000);
            iVar2 = FUN_80021a30((int *)(uintptr_t)uRam000006fc,
                                 (int)sRam000006f0,
                                 (int)sRam000007dc,
                                 &local_28);
            if (iVar2 != 0) {
                if (!host_ai_object_ptr_ok((uintptr_t)iVar2))
                    return 0;
                iVar4 = *(int *)(uintptr_t)(param_1 + 0xe4);
                if (!host_ai_object_ptr_ok((uintptr_t)iVar4))
                    return 0;
                iVar3 = mips_subu_i32(*(int32_t *)(uintptr_t)(iVar2 + 0x24),
                                      *(int32_t *)(uintptr_t)(iVar4 + 0x24));
                if (iVar3 < 0) iVar3 = mips_subu_i32(0, iVar3);
                if (0xf9fff < iVar3) return 0;
                iVar3 = mips_subu_i32(*(int32_t *)(uintptr_t)(iVar2 + 0x28),
                                      *(int32_t *)(uintptr_t)(iVar4 + 0x28));
                if (iVar3 < 0) iVar3 = mips_subu_i32(0, iVar3);
                if (0xf9fff < iVar3) return 0;
                iVar3 = mips_subu_i32(*(int32_t *)(uintptr_t)(iVar2 + 0x2c),
                                      *(int32_t *)(uintptr_t)(iVar4 + 0x2c));
                if (iVar3 < 0) iVar3 = mips_subu_i32(0, iVar3);
                if (0xf9fff < iVar3) return 0;
                *(int *)(uintptr_t)(param_1 + 0xe4) = iVar2;
                if (Object_CallbackFromPsxSlot((const void *)(uintptr_t)iVar5) == 0) {
                    sVar1 = 0;
                } else {
                    typedef int16_t (*TickFn)(int, int, int);
                    sVar1 = ((TickFn)Object_CallbackFromPsxSlot((const void *)(uintptr_t)iVar5))
                        (iVar5, 0xb, param_1);
                }
                iVar2 = (int)FUN_80017160();
                *(int16_t *)(uintptr_t)(iVar5 + 6) =
                    (int16_t)mips_addu_i32((int32_t)sVar1,
                                           mips_mult_lo_i32(mips_sll_i32(iVar2, 7),
                                                           mips_subu_i32(2, cRam00000016)) >> 0xf);
                *(int *)(uintptr_t)(param_1 + 0xe4) = iVar4;
                return 1;
            }
        }
    }
    return 0;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80022e90  (from analysis/SLUS_005.10/decomp/80022e90.c) --- */
// addr: 0x80022e90  name: FUN_80022e90

undefined4 FUN_80022e90(int param_1)

{
  short sVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  
  if (cRam00000015 == '\0') {
    if (*(byte *)(param_1 + 0xd0) < 6) {
      return 0;
    }
    iVar5 = *(int *)(param_1 + (uint)*(byte *)(param_1 + 0xb3) * 4 + 0x110);
    if (iVar5 != 0) {
      if (*(char *)(iVar5 + 8) != '\x04') {
        return 0;
      }
      local_28 = *(int *)(param_1 + 0x24) + -0xfa000;
      local_24 = *(int *)(param_1 + 0x24) + 0xfa000;
      local_20 = *(int *)(param_1 + 0x2c) + -0xfa000;
      local_1c = *(int *)(param_1 + 0x2c) + 0xfa000;
      iVar2 = FUN_80021a30(uRam000006fc,(int)sRam000006f0,(int)sRam000007dc,&local_28);
      if (iVar2 != 0) {
        iVar4 = *(int *)(param_1 + 0xe4);
        iVar3 = *(int *)(iVar2 + 0x24) - *(int *)(iVar4 + 0x24);
        if (iVar3 < 0) {
          iVar3 = -iVar3;
        }
        if (0xf9fff < iVar3) {
          return 0;
        }
        iVar3 = *(int *)(iVar2 + 0x28) - *(int *)(iVar4 + 0x28);
        if (iVar3 < 0) {
          iVar3 = -iVar3;
        }
        if (0xf9fff < iVar3) {
          return 0;
        }
        iVar3 = *(int *)(iVar2 + 0x2c) - *(int *)(iVar4 + 0x2c);
        if (iVar3 < 0) {
          iVar3 = -iVar3;
        }
        if (0xf9fff < iVar3) {
          return 0;
        }
        *(int *)(param_1 + 0xe4) = iVar2;
        if (*(code **)(iVar5 + 100) == (code *)0x0) {
          sVar1 = 0;
        }
        else {
          sVar1 = (**(code **)(iVar5 + 100))(iVar5,0xb,param_1);
        }
        iVar2 = FUN_80017160();
        *(short *)(iVar5 + 6) = sVar1 + (short)(iVar2 * 0x80 * (2 - cRam00000016) >> 0xf);
        *(int *)(param_1 + 0xe4) = iVar4;
        return 1;
      }
    }
  }
  return 0;
}

#endif  /* GHIDRA REF */
