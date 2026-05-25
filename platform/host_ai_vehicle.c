/* host_ai_vehicle.c -- spawn the opponent through the original LOAD record path. */
#include <stdint.h>
#include <stdio.h>

extern void     Audio_BankSelect(uint32_t mask);
extern intptr_t Host_TerrainFindPlaceholderById(int spawn_id);
extern uint32_t FUN_800244c4(int pos_x, int pos_z);
extern intptr_t FUN_80021f30(uint8_t *record);
extern uint32_t FUN_8002036c(uint32_t *obj);
extern void     FUN_8002e604(uint32_t *vehicle, int health);
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

extern void    *puRam000007d4;
extern void    *puRam000007d0;
extern uint8_t  DAT_80065674[];
extern uintptr_t DAT_800737a0[];

static int placeholder_is_navigable(int spawn_id)
{
    uint8_t *obj = (uint8_t *)Host_TerrainFindPlaceholderById(spawn_id);
    if (obj == 0)
        return 0;
    return FUN_800244c4(*(int32_t *)(obj + 0x48),
                        *(int32_t *)(obj + 0x50)) != 0;
}

static int choose_ai_spawn_id(void)
{
    static const int ids[] = {
        1, 2, 3, 4, 5, 6, 7, 8,
        -2, -3, -4, -5, -6, -7, -8, -9,
        -10, -11, -12, -13, -14, -15
    };

    for (int i = 0; i < (int)(sizeof(ids) / sizeof(ids[0])); i++) {
        if (placeholder_is_navigable(ids[i]))
            return ids[i];
    }
    return 0;
}

void Host_AIVehicleInit(void)
{
    uint32_t player_slot = DAT_80065674[0];
    if (player_slot >= 12) player_slot = 0;

    uint32_t ai_slot = DAT_80065674[1];
    if (ai_slot >= 12 || ai_slot == player_slot) ai_slot = (player_slot + 1u) % 12u;
    DAT_80065674[1] = (uint8_t)ai_slot;

    int spawn_id = choose_ai_spawn_id();
    if (spawn_id == 0) {
        fprintf(stderr,
                "v8: source AI spawn skipped -- no navigable spawn placeholder\n");
        return;
    }

    if (DAT_800737a0[player_slot] == 0 || DAT_800737a0[ai_slot] == 0 ||
        DAT_800737a0[13] == 0 || DAT_800737a0[14] == 0 || DAT_800737a0[15] == 0) {
        Audio_BankSelect(0x0000e000u | (1u << player_slot) | (1u << ai_slot));
    }

    uint8_t record[6] = {
        (uint8_t)ai_slot,
        0x00,
        (uint8_t)((uint16_t)spawn_id & 0xffu),
        (uint8_t)(((uint16_t)spawn_id >> 8) & 0xffu),
        0x00,
        0x00
    };

    uint8_t *ai = (uint8_t *)FUN_80021f30(record);
    if (ai == 0) {
        fprintf(stderr,
                "v8: source AI spawn failed slot=%u spawn_id=%d\n",
                (unsigned)ai_slot, spawn_id);
        return;
    }

    FUN_8002e604((uint32_t *)ai, *(uint16_t *)(ai + 0x0c) >> 1);
    if (*(int16_t *)(ai + 0x06) < 0)
        *(int16_t *)(ai + 0x06) = 1;
    if (puRam000007d0 != 0)
        *(uint32_t *)(ai + 0xe4) = (uint32_t)(uintptr_t)puRam000007d0;
    *(uint8_t *)(ai + 0x08) = 4;
    FUN_8002036c((uint32_t *)ai);
    puRam000007d4 = ai;

    fprintf(stderr,
            "v8: source AI spawned slot=%u spawn_id=%d obj=%p status=%d state=%u cb=%p\n",
            (unsigned)ai_slot, spawn_id, (void *)ai,
            (int)*(int16_t *)(ai + 0x06),
            (unsigned)*(uint8_t *)(ai + 0xd0),
            (void *)Object_CallbackFromPsxSlot(ai));
}
