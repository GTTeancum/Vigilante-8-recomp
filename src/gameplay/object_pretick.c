/* object_pretick.c -- Per-frame object animation/state-machine tick.
 *
 * Source: SLUS_005.10
 *   FUN_8001f9cc  -- Object_PreTickRecurse: advance the animation state
 *                    machine for one object.  Reads keyframe data from the
 *                    animation data pointer at obj+0x60, advances the frame
 *                    counter at obj+0x46, updates rotation/translation/scale
 *                    and bone channels.
 *   FUN_8001fc38  -- Object_PreTickChildren: walk the sibling chain (via
 *                    obj+0x34) calling Object_PreTickRecurse on each, then
 *                    recursing into children (obj+0x38).
 *
 * Object layout offsets referenced here:
 *   +0x30 : int  -- bone bank ptr
 *   +0x34 : int  -- next sibling ptr
 *   +0x38 : int  -- first child ptr
 *   +0x40 : 6 bytes -- packed rotation state (3 bytes rot + 2 bytes rot-param)
 *   +0x46 : int16 -- last frame counter
 *   +0x48 : int[3] -- translation xyz
 *   +0x54 : (padding)
 *   +0x58 : int  -- bone bank handle (used by bone channel ops)
 *   +0x60 : short * -- animation data cursor (keyframe data pointer)
 *   +0x64 : 4 bytes -- (flags etc.)
 *   +0x22 : uint16 -- scale selector field
 *
 * Animation data format (per keyframe at *anim_ptr):
 *   [0]  int16 -- frame delta to next keyframe (negative = end-of-sequence)
 *   [1]  int16 -- flags (bits 0x01/0x02/0x08/0x10/0x20/0x40)
 *   [2+] variable length depending on flags
 *
 * HIGH confidence: direct Ghidra port with 64-bit pointer casts.
 */
#include <stdint.h>
#include <stdbool.h>

extern void FUN_8001d708(int param_1);                              /* Object_InitBoneMatrix */
extern int *FUN_8001b36c(int *param_1, uint32_t param_2);          /* BoneAnim_InitSlot */
extern int *FUN_80043864(int *param_1, int16_t *param_2, uint32_t *param_3); /* Matrix_ScaleByVec */
extern void *Host_HeapBase(void);
extern uint32_t Host_HeapSize(void);
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

static inline uintptr_t obj_ptr32(uintptr_t base, uint32_t off)
{
    return (uintptr_t)*(uint32_t *)(base + off);
}

static inline void obj_write32(uintptr_t base, uint32_t off, uintptr_t value)
{
    *(uint32_t *)(base + off) = (uint32_t)value;
}

static int host_ptr_range_ok(uintptr_t p, uint32_t need)
{
    uintptr_t base = (uintptr_t)Host_HeapBase();
    uintptr_t end = base + Host_HeapSize();

    if (p < base || p > end)
        return 0;
    return need <= (uint32_t)(end - p);
}

/* ================================================================
 * FUN_8001f9cc  -- Object_PreTickRecurse
 *
 * Advance the animation state machine for object at param_1.
 * param_2: current frame counter (int16, from the global clock).
 *
 * Returns: 0  on normal advance (no further action needed)
 *          -1 if the tick callback returned a negative value (abort)
 * ================================================================ */
int FUN_8001f9cc(int param_1, int16_t param_2)
{
    int16_t   sVar1;
    bool      bVar2;
    int       iVar3;
    int32_t   uVar4;
    int16_t  *psVar5;
    uint32_t  in_t0;        /* PSX lwl/lwr scratch; init 0 (bits filled by load) */
    uint32_t  uVar6;
    int32_t   flags;
    uint32_t  uVar10;
    (void)uVar6;
    uint32_t *puVar8;
    uint32_t *puVar9;
    uint32_t *puVar11;

    if (!host_ptr_range_ok((uintptr_t)(uint32_t)param_1, 0x68))
        return 0;

    uintptr_t anim = obj_ptr32((uintptr_t)param_1, 0x60);
    if (anim == 0) {
        return 0;
    }
    if (!host_ptr_range_ok(anim, 4))
        return 0;
    /* Check if enough frames have elapsed since last update. */
    if ((uint16_t)(param_2 - *(int16_t *)(uintptr_t)(param_1 + 0x46)) <
        (uint16_t)*(int16_t *)anim) {
        return 0;
    }

    bVar2  = false;
    puVar11 = (uint32_t *)0;
    in_t0  = 0;

    do {
        psVar5 = (int16_t *)obj_ptr32((uintptr_t)param_1, 0x60);
        if (!host_ptr_range_ok((uintptr_t)psVar5, 4))
            return 0;
        flags = (int32_t)psVar5[1];
        uVar10 = (uint32_t)flags;
        puVar8 = (uint32_t *)(uintptr_t)(psVar5 + 2);

        if (flags < 0) {
            /* Negative flags word: sequence jump / frame-delta advance. */
            *(int16_t *)(uintptr_t)(param_1 + 0x46) =
                *(int16_t *)(uintptr_t)(param_1 + 0x46) + *psVar5;
            obj_write32((uintptr_t)param_1, 0x60,
                obj_ptr32((uintptr_t)param_1, 0x60) + flags);

            /* Dispatch event 5 to tick callback. */
            typedef int (*EventFn)(int, int, int);
            EventFn cb = (EventFn)Object_CallbackFromPsxSlot((const void *)(uintptr_t)param_1);
            if (cb == NULL) {
                iVar3 = 0;
            } else {
                iVar3 = cb(param_1, 5, 0);
            }
            if (iVar3 < 0) {
                return -1;
            }
        } else {
            /* Process keyframe flag bits. */
            if ((uVar10 & ~0x7bu) != 0)
                return 0;

            if ((uVar10 & 1u) != 0) {
                /* Bit 1: rotation update.
                 * PSX lwl/lwr reads 4 bytes from psVar5+4 (aligned assumption). */
                if (!host_ptr_range_ok((uintptr_t)psVar5, 12))
                    return 0;
                in_t0 = *(uint32_t *)(uintptr_t)((intptr_t)psVar5 + 4);
                sVar1 = psVar5[4];
                /* Store rotation bytes to param_1+0x40 (4-byte packed). */
                *(uint32_t *)(uintptr_t)(param_1 + 0x40) = in_t0;
                /* Store rotation parameter at +0x44. */
                *(int16_t *)(uintptr_t)(param_1 + 0x44) = sVar1;
                puVar8 = (uint32_t *)(uintptr_t)(psVar5 + 6);
                bVar2 = true;
            }

            if ((uVar10 & 2u) != 0) {
                /* Bit 2: translation absolute. */
                if (!host_ptr_range_ok((uintptr_t)puVar8, 12))
                    return 0;
                in_t0 = *puVar8;
                *(uint32_t *)(uintptr_t)(param_1 + 0x48) = in_t0;
                *(uint32_t *)(uintptr_t)(param_1 + 0x4c) = puVar8[1];
                *(uint32_t *)(uintptr_t)(param_1 + 0x50) = puVar8[2];
                puVar8 += 3;
                bVar2 = true;
            }

            if ((uVar10 & 8u) != 0) {
                /* Bit 8: translation delta (packed int16 pairs per uint32). */
                if (!host_ptr_range_ok((uintptr_t)puVar8, 8))
                    return 0;
                *(int32_t *)(uintptr_t)(param_1 + 0x48) +=
                    (int32_t)(int16_t)(uint16_t)(*puVar8 & 0xffffu);
                *(int32_t *)(uintptr_t)(param_1 + 0x4c) +=
                    (int32_t)(int16_t)(uint16_t)(*(uint16_t *)((uint8_t *)puVar8 + 2));
                puVar9 = puVar8 + 1;
                bVar2 = true;
                puVar8 += 2;
                *(int32_t *)(uintptr_t)(param_1 + 0x50) +=
                    (int32_t)(int16_t)(uint16_t)(*puVar9 & 0xffffu);
            }

            if ((uVar10 & 0x10u) != 0) {
                /* Bit 0x10: bone channel updates. High bit of uVar6 = stop flag. */
                if (!host_ptr_range_ok(obj_ptr32((uintptr_t)param_1, 0x30), 0x2c))
                    return 0;
                if (!host_ptr_range_ok(obj_ptr32((uintptr_t)param_1, 0x58), 4))
                    return 0;
                do {
                    if (!host_ptr_range_ok((uintptr_t)puVar8, 4))
                        return 0;
                    uVar6 = *puVar8;
                    uVar4 = (int32_t)(uintptr_t)FUN_8001b36c(
                        (int *)obj_ptr32((uintptr_t)param_1, 0x58),
                        *(int16_t *)(uintptr_t)(obj_ptr32((uintptr_t)param_1, 0x30) + 0x2a) +
                        (int32_t)(uint16_t)(*(uint16_t *)((uint8_t *)puVar8 + 2)));
                    puVar8 += 1;
                    *(uint32_t *)(obj_ptr32((uintptr_t)param_1, 0x30) +
                        (int32_t)((uint16_t)uVar6 & 0x7fffu) * 4 + 0x2c) = (uint32_t)uVar4;
                } while ((int)((uint32_t)(uint16_t)uVar6 << 16) >= 0);
                /* Loop while bit 15 of (uint16_t)uVar6 is clear. */
            }

            puVar9 = puVar8;
            if ((uVar10 & 0x20u) != 0) {
                /* Bit 0x20: scale vector follows. Save pointer for later. */
                if (!host_ptr_range_ok((uintptr_t)puVar8, 8))
                    return 0;
                puVar9 = puVar8 + 2;
                bVar2 = true;
                puVar11 = puVar8;
            }

            if ((uVar10 & 0x40u) != 0) {
                /* Bit 0x40: skip mesh data block (at bone bank's mesh ptr). */
                if (!host_ptr_range_ok((uintptr_t)puVar9, 4))
                    return 0;
                if (!host_ptr_range_ok(obj_ptr32((uintptr_t)param_1, 0x30), 12))
                    return 0;
                obj_write32(obj_ptr32((uintptr_t)param_1, 0x30), 8, (uintptr_t)(puVar9 + 1));
                puVar9 = puVar9 + *puVar9 * 2 + 1;
            }

            obj_write32((uintptr_t)param_1, 0x60, (uintptr_t)puVar9);
        }

        anim = obj_ptr32((uintptr_t)param_1, 0x60);
        if (!host_ptr_range_ok(anim, 2))
            return 0;
    } while ((uint16_t)(*(int16_t *)anim) <=
             (uint16_t)(param_2 - *(int16_t *)(uintptr_t)(param_1 + 0x46)));

    if (!bVar2) {
        return 0;
    }

    /* Re-build the bone matrix from updated rotation/translation. */
    FUN_8001d708(param_1);

    /* If a scale vector was recorded, apply it. */
    if (puVar11 != (uint32_t *)0) {
        FUN_80043864((int *)(uintptr_t)(param_1 + 0x10), (int16_t *)puVar11,
                     (uint32_t *)(uintptr_t)(param_1 + 0x10));
        *(uint16_t *)(uintptr_t)(param_1 + 0x22) = (uint16_t)*puVar11;
    }
    return 0;
}

/* Public aliases so list_walkers.c can call by readable name. */
int Object_PreTickRecurse(int payload, uint16_t arg)
    { return FUN_8001f9cc(payload, (int16_t)arg); }

/* ================================================================
 * FUN_8001fc38  -- Object_PreTickChildren
 *
 * Walk the sibling chain starting at param_1 (via param_1[0xd] =
 * +0x34 = next-sibling), applying Object_PreTickRecurse to each.
 * If any object has children and pretick succeeds, recurse.
 * ================================================================ */
void FUN_8001fc38(uint32_t *param_1, uint16_t param_2)
{
    int       iVar1;
    uint32_t *puVar2;

    do {
        puVar2 = (uint32_t *)(uintptr_t)*(uint32_t *)((uint8_t *)param_1 + 0x34);
        if ((*param_1 & 4u) == 0) {
            iVar1 = FUN_8001f9cc((int)(uintptr_t)param_1, (int16_t)param_2);
            uintptr_t child = *(uint32_t *)((uint8_t *)param_1 + 0x38);
            if (iVar1 >= 0 && child != 0) {
                FUN_8001fc38((uint32_t *)child, param_2);
            }
        }
        param_1 = puVar2;
    } while (puVar2 != (uint32_t *)0);
}

/* Public alias. */
void Object_PreTickChildren(int chainHead, uint16_t arg)
    { FUN_8001fc38((uint32_t *)(uintptr_t)chainHead, arg); }
