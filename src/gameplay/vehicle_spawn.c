/* vehicle_spawn.c -- original vehicle/object spawn helpers.
 *
 * Source: SLUS_005.10 gap 0x80021c20..0x80022040.
 *
 * These functions sit between LOAD.DLL spawn records and the vehicle
 * constructor callback table.  They are intentionally separate from the host
 * fallback vehicle shim: the real runtime path clones a placeholder object,
 * installs the selected vehicle dispatch callback, then lets FUN_80021b80
 * allocate/build the final object.
 */
#include <stdint.h>

extern intptr_t FUN_80021b80(intptr_t (*cb)(intptr_t, int, int),
                             intptr_t bank, uint16_t kind, uint32_t flags);
extern void     FUN_8001dc1c(uint32_t *obj);
extern void     FUN_8001b0c4(uint32_t *obj);
extern intptr_t FUN_8001ffd4(intptr_t listSentinel, int spawnId);
extern int32_t  FUN_80016aac(const int32_t *a, const int32_t *b);

extern uintptr_t (*DAT_8005ec34[12])(uint32_t *, int, uint16_t);
extern uintptr_t DAT_800737a0[32];
extern uint8_t   DAT_80065674[];
extern uint8_t   DAT_80065a50[];
extern void     *puRam000007d0;
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);
extern void      Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* HIGH: spawn selected vehicle slot directly through callback table.
 * This is the tiny 0x80021c20 wrapper used by callers that already know the
 * selected roster slot. */
intptr_t FUN_80021c20(uint16_t slot)
{
    uint32_t idx = slot & 0xffffu;
    if (idx >= 12) idx = 0;
    return FUN_80021b80((intptr_t (*)(intptr_t, int, int))DAT_8005ec34[idx],
                        (intptr_t)DAT_800737a0[idx], 0, 0);
}

/* HIGH: clone a placeholder object into a real runtime object.
 *
 * Original copies flags, callback/bank/kind-derived health fields, spawn
 * transform, and then runs rotation/bone-animation setup helpers.
 */
intptr_t FUN_80021c6c(uint32_t *placeholder)
{
    uint8_t *src = (uint8_t *)placeholder;
    intptr_t (*cb)(intptr_t, int, int) =
        (intptr_t (*)(intptr_t, int, int))Object_CallbackFromPsxSlot(src);
    intptr_t bank = (intptr_t)*(uint32_t *)(src + 0x58);
    uint16_t kind = *(uint16_t *)(src + 0x0a);
    uint32_t flags = (*(uint32_t *)src << 1) & 8u;

    uint8_t *dst = (uint8_t *)FUN_80021b80(cb, bank, kind, flags);

    uint16_t health = *(uint16_t *)(src + 0x0c);
    uint16_t maxHealth = *(uint16_t *)(src + 0x0e);
    Object_SetCallbackPsxSlot(dst, (uintptr_t)cb);
    *(uint32_t *)dst |= *(uint32_t *)src;
    *(uint16_t *)(dst + 0x06) = *(uint16_t *)(src + 0x06);
    *(uint8_t  *)(dst + 0x08) = *(uint8_t  *)(src + 0x08);

    *(uint32_t *)(dst + 0x48) = *(uint32_t *)(src + 0x48);
    *(uint32_t *)(dst + 0x4c) = *(uint32_t *)(src + 0x4c);
    *(uint32_t *)(dst + 0x50) = *(uint32_t *)(src + 0x50);
    *(uint32_t *)(dst + 0x40) = *(uint32_t *)(src + 0x40);
    *(int16_t  *)(dst + 0x44) = *(int16_t  *)(src + 0x44);
    *(uint8_t  *)(dst + 0x09) = *(uint8_t  *)(src + 0x09);

    if (health != 0 || maxHealth != 0) {
        uint32_t child = *(uint32_t *)(dst + 0x38);
        *(uint16_t *)(dst + 0x0c) = health;
        *(uint16_t *)(dst + 0x0e) = maxHealth;
        while (child != 0) {
            *(uint16_t *)((uintptr_t)child + 0x0c) = health;
            child = *(uint32_t *)((uintptr_t)child + 0x34);
        }
    }

    FUN_8001dc1c((uint32_t *)dst);
    FUN_8001b0c4((uint32_t *)dst);
    return (intptr_t)dst;
}

intptr_t func_0x80021c6c(uint32_t *placeholder)
{
    return FUN_80021c6c(placeholder);
}

/* HIGH: find nearest spawn placeholder with id in [1, 31]. */
intptr_t FUN_80021db0(int32_t *xyz)
{
    uint32_t *node = *(uint32_t **)(DAT_80065a50);
    int32_t bestDist = -1;
    uint32_t *best = 0;

    while (node != 0) {
        uint8_t *obj = (uint8_t *)(uintptr_t)node[2];
        uint16_t id = *(uint16_t *)(obj + 0x06);
        if ((uint16_t)(id - 1) < 0x1fu) {
            int32_t dist = FUN_80016aac(xyz, (const int32_t *)(obj + 0x48));
            if ((uint32_t)dist < (uint32_t)bestDist) {
                bestDist = dist;
                best = (uint32_t *)obj;
            }
        }
        node = (uint32_t *)(uintptr_t)node[0];
    }
    return (intptr_t)best;
}

/* HIGH: install selected vehicle callback/bank on placeholder and clone it. */
intptr_t FUN_80021e5c(uint32_t *placeholder, int slot)
{
    uint8_t *obj = (uint8_t *)placeholder;
    uint32_t idx = (uint32_t)slot & 0xffffu;
    if (idx >= 12) idx = 0;

    Object_SetCallbackPsxSlot(obj, (uintptr_t)DAT_8005ec34[idx]);
    *(uint16_t *)(obj + 0x0a) = 0;
    *(uint32_t *)(obj + 0x58) = (uint32_t)DAT_800737a0[idx];

    uint8_t *spawned = (uint8_t *)FUN_80021c6c(placeholder);
    int16_t hp = *(int16_t *)(obj + 0x06);
    if (hp > 0) {
        int16_t v1 = *(int16_t *)(spawned + 0x14);
        int32_t val = (v1 * 0x11f1) >> 5;
        *(int32_t *)(spawned + 0x80) = val;
        *(int32_t *)(spawned + 0x84) = 0;
        v1 = *(int16_t *)(spawned + 0x20);
        val = (v1 * 0x11f1) >> 5;
        *(int32_t *)(spawned + 0x88) = val;
    }
    return (intptr_t)spawned;
}

/* HIGH: spawn from a 6-byte LOAD.DLL spawn record. */
intptr_t FUN_80021f30(uint8_t *record)
{
    uint32_t *placeholder;
    int16_t spawnId = *(int16_t *)(record + 2);
    if (spawnId != 0) {
        placeholder = (uint32_t *)FUN_8001ffd4((intptr_t)DAT_80065a50, spawnId);
    } else {
        placeholder = (uint32_t *)FUN_80021db0((int32_t *)((uint8_t *)puRam000007d0 + 0x24));
    }
    if (placeholder == 0)
        return 0;

    *(uint16_t *)((uint8_t *)placeholder + 0x0c) =
        *(uint16_t *)(record + 4);

    int slot = *(int8_t *)record;
    if (slot < 0)
        slot = (int8_t)DAT_80065674[(uint8_t)~slot];

    uint8_t *spawned = (uint8_t *)FUN_80021e5c(placeholder, slot);
    *(uint32_t *)spawned |= (uint32_t)*(uint8_t *)(record + 1) << 24;
    return (intptr_t)spawned;
}

/* HIGH: fallback spawn by spawn-id, with negative ids indexing player slots. */
intptr_t FUN_80021fd8(int spawnId)
{
    uint32_t *placeholder =
        (uint32_t *)FUN_8001ffd4((intptr_t)DAT_80065a50, spawnId);
    if (placeholder == 0)
        return 0;

    int slot;
    if (spawnId < 0)
        slot = (int8_t)DAT_80065674[(uint32_t)~spawnId];
    else
        slot = (int8_t)DAT_80065674[(uint32_t)spawnId + 1u];
    return FUN_80021e5c(placeholder, slot);
}
