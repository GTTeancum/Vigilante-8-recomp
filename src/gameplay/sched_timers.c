/* sched_timers.c -- Timer/schedule subsystem teardown.
 *
 * Source: SLUS_005.10
 *   FUN_80041da0  -- Sched_FreeTimerEntry: unlink and free one timer object.
 *   FUN_80041e80  -- Sched_ResetTimers: drain the active timer queue and
 *                    free all timer and animation-channel slots.
 *
 * The PSX game uses a software scheduler (timer queue) for timed game
 * events (weapon delay, AI retrigger, etc.).  The queue is a doubly-
 * linked ring with a sentinel at UNK_80065bc8.  Animation channels are
 * a separate flat array at iRam000008d0 with iRam000008bc entries.
 *
 * HIGH confidence: direct Ghidra port.
 */
#include <stdint.h>
#include "structs.h"

extern void FUN_80045088(int ptr);           /* Heap_Free */
extern void FUN_8001bddc(int bank);          /* BoneBank_Free */
extern void FUN_80040e18(void *p);           /* Heap_Free wrapper preserving MIPS a0 */

/* ---- scheduler globals ---- */
/* puRam000008cc: active timer queue head pointer (points to sentinel when empty) */
extern void     *puRam000008cc;
/* UNK_80065bc8: timer queue sentinel / ring tail */
extern uint8_t   UNK_80065bc8[];
/* piRam000008c4: current iterator pointer in the timer ring */
extern uintptr_t *piRam000008c4;
/* iRam000008c0: count of allocated timer objects in iRam000008d4 array */
extern int32_t   iRam000008c0;
/* iRam000008d4: pointer to timer object array */
extern uintptr_t iRam000008d4;
/* iRam000008bc: count of animation channel slots in iRam000008d0 */
extern int32_t   iRam000008bc;
/* iRam000008d0: pointer to animation channel array (entries 0x34 bytes each) */
extern uintptr_t iRam000008d0;

/* ================================================================
 * FUN_80041da0  -- Sched_FreeTimerEntry
 *
 * Unlinks all joint edges referencing param_1, frees any attached
 * data buffer, then frees param_1 itself.
 *
 * Layout of timer entry (param_1 base):
 *   +0x12 : int16   -- number of joint edges
 *   +0x18 : int32 * -- pointer to data buffer header
 *                        header+8 = the data block
 *   +0x1c : int32[] -- array of joint-edge pointers (one per joint)
 *
 * Each joint-edge struct:
 *   +0x00 : int32   -- side-A object ptr
 *   +0x04 : int32   -- side-B object ptr
 * ================================================================ */
void FUN_80041da0(intptr_t param_1)
{
    uint32_t uVar1;
    intptr_t iVar2;
    int      iVar3;
    intptr_t iVar4;

    iVar3 = 0;
    iVar4 = param_1;
    if (0 < *(int16_t *)(param_1 + 0x12)) {
        do {
            iVar2 = *(int32_t *)(iVar4 + 0x1c);
            /* Which side of the edge are we?  If side-B pointer == param_1,
             * uVar1 = 1 (we are side B); otherwise 0 (we are side A). */
            uVar1 = (uint32_t)(*(int32_t *)(iVar2 + 4) == param_1);
            if (*(int32_t *)(iVar2 + (1 - (int)uVar1) * 4) == 0) {
                /* Other side is already null -- free the edge struct. */
                FUN_80045088((int)(uintptr_t)iVar2);
            } else {
                /* Null our side; other side still holds the edge. */
                *(int32_t *)(iVar2 + (int)uVar1 * 4) = 0;
            }
            iVar3 = iVar3 + 1;
            iVar4 = iVar4 + 4;
        } while (iVar3 < *(int16_t *)(param_1 + 0x12));
    }

    if (*(int32_t *)(param_1 + 0x18) != 0) {
        /* Free the data block inside the buffer header (+8). */
        FUN_80045088(*(int32_t *)(uintptr_t)(*(uint32_t *)(uintptr_t)(param_1 + 0x18) + 8));
        /* Free the buffer header itself. */
        FUN_8001bddc(*(int32_t *)(uintptr_t)(param_1 + 0x18));
    }

    FUN_80045088((int)(uintptr_t)param_1);
}

/* ================================================================
 * FUN_80041e80  -- Sched_ResetTimers
 *
 * Drain the active timer ring, free all timer objects, and free the
 * animation channel array.
 * ================================================================ */
void Sched_ResetTimers(void)
{
    uintptr_t iVar1;
    uintptr_t *piVar2;
    int32_t  iVar3;

    /* 1. Drain the active timer queue ring until it points to sentinel. */
    if (puRam000008cc != NULL &&
        puRam000008cc != (void *)UNK_80065bc8 &&
        piRam000008c4 != NULL) {
        do {
            piVar2   = (uintptr_t *)piRam000008c4[1]; /* next ptr */
            iVar1    = *piRam000008c4;              /* prev ptr */
            if (piVar2 == NULL || iVar1 == 0)
                break;
            *(uintptr_t **)(iVar1 + 4) = piVar2;   /* next->prev = prev */
            *piVar2  = iVar1;                       /* prev->next = next */
            FUN_80040e18(piRam000008c4);
        } while (puRam000008cc != (void *)UNK_80065bc8);
    }

    /* 2. Free all timer objects in the timer array. */
    iVar1 = 0;
    if (0 < iRam000008c0) {
        do {
            iVar3 = iVar1 * 4;
            iVar1 = iVar1 + 1;
            FUN_80041da0(*(uint32_t *)(iVar3 + iRam000008d4));
        } while (iVar1 < iRam000008c0);
    }
    if (iRam000008d4 != 0) {
        FUN_80045088((int)(uintptr_t)iRam000008d4);
    }

    /* 3. Free animation channel sub-objects, then the channel array. */
    iVar1 = 0;
    if (0 < iRam000008bc) {
        iVar3 = 0;
        do {
            if (*(int16_t *)(iVar3 + iRam000008d0) != 0) {
                if (*(int32_t *)((int16_t *)(iVar3 + iRam000008d0) + 6) != 0) {
                    FUN_80045088(*(int32_t *)(uintptr_t)((iVar3 + iRam000008d0) + 12));
                }
                if (*(int32_t *)(iVar3 + iRam000008d0 + 0x10) != 0) {
                    FUN_80045088(*(int32_t *)(uintptr_t)(iVar3 + iRam000008d0 + 0x10));
                }
            }
            iVar1 = iVar1 + 1;
            iVar3 = iVar3 + 0x34;
        } while (iVar1 < iRam000008bc);
    }
    if (iRam000008d0 != 0) {
        FUN_80045088((int)(uintptr_t)iRam000008d0);
    }
}
