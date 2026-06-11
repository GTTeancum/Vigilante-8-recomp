/* pickup_population.c -- runtime materialisation of authored pickup slots. */
#include <stdint.h>
#include <stdio.h>

extern int16_t  DAT_800658e8[];
extern int32_t  DAT_800659ec;
extern int      FUN_80020120(int *list, uint32_t flag);
extern uint32_t *FUN_80020190(int *list, uint32_t flag, int n);
extern intptr_t FUN_80021c6c(uint32_t *placeholder);
extern uint32_t FUN_8002036c(uint32_t *obj);
extern uint32_t FUN_80017160(void);
extern uint8_t  DAT_80065a50[];

static int s_pickup_population_done;

static int PickupPopulation_SpawnOne(uint32_t mask)
{
    int count = FUN_80020120((int *)DAT_80065a50, mask);
    if (count <= 0)
        return 0;

    int index = ((int32_t)FUN_80017160() * count) >> 15;
    uint32_t *placeholder = FUN_80020190((int *)DAT_80065a50, mask, index);
    if (placeholder == 0)
        return 0;

    intptr_t pickup = FUN_80021c6c(placeholder);
    if (pickup == 0)
        return 0;

    *placeholder |= 0x8000u;
    if (FUN_8002036c((uint32_t *)(uintptr_t)pickup) == 0)
        return 0;
    return 1;
}

void PickupPopulation_Reset(void)
{
    s_pickup_population_done = 0;
    DAT_800659ec = 0;
}

void PickupPopulation_Tick(void)
{
    int weapon_quota;
    int utility_quota;
    int spawned = 0;
    int wanted = 0;

    if (s_pickup_population_done)
        return;
    s_pickup_population_done = 1;

    weapon_quota = (int)DAT_800658e8[1];
    utility_quota = (int)DAT_800658e8[2];
    if (weapon_quota < 0)
        weapon_quota = 0;
    if (utility_quota < 0)
        utility_quota = 0;
    if (weapon_quota > 64)
        weapon_quota = 64;
    if (utility_quota > 64)
        utility_quota = 64;

    wanted = weapon_quota + utility_quota;

    for (int i = 0; i < weapon_quota; i++) {
        if (PickupPopulation_SpawnOne(0x66000000u))
            spawned++;
    }

    for (int i = 0; i < utility_quota; i++) {
        uint32_t mask = (DAT_800659ec < (int32_t)DAT_800658e8[3]) ?
            0x00780000u : 0x00380000u;
        if (PickupPopulation_SpawnOne(mask))
            spawned++;
        DAT_800659ec++;
    }

    fprintf(stderr,
            "v8: PickupPopulation materialized %d/%d authored pickups "
            "(weapon=%d utility=%d masks=66000000/00780000)\n",
            spawned, wanted, weapon_quota, utility_quota);
}
