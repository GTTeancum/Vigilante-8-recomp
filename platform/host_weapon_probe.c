/* host_weapon_probe.c -- verification-only source weapon path audit.
 *
 * This is not gameplay logic.  The --weapon-probe CLI flag runs after the
 * normal source vehicle/AI construction and exercises decoded pickup,
 * weapon-slot, attached-state, and projectile callbacks through their real
 * source entry points.  It exits immediately after reporting pass/fail.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern uint32_t *puRam000007d0;
extern uint32_t *puRam000007d4;
extern uintptr_t DAT_800737dc;
extern int g_v8_weapon_probe_state;

extern void *FUN_8001d470(uint32_t size);
extern void FUN_8002cce8(uint32_t *self, uint8_t cap_mask);
extern intptr_t LAB_8003c61c(intptr_t obj, int event, intptr_t param3);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

typedef intptr_t (*ObjCb)(intptr_t obj, int event, intptr_t arg);

static int bank_entry_spawnable(uintptr_t bankPtr, uint16_t kind)
{
    uint8_t *bank = (uint8_t *)bankPtr;
    uintptr_t table;
    int16_t *entry;

    if (bank == NULL)
        return 0;
    table = (uintptr_t)*(uint32_t *)bank;
    if (table == 0)
        return 0;

    entry = (int16_t *)(table + (uint32_t)kind * 0x1c + 0x1c);
    return !(entry[0] < 0 && (entry[0] != -1));
}

static int state_fire_bank_requirements_met(uint8_t *slot, unsigned state)
{
    uintptr_t bank = (uintptr_t)*(uint32_t *)(slot + 0x58);
    uintptr_t pivot = (uintptr_t)*(uint32_t *)(slot + 0x38);

    switch (state) {
    case 1:
        return bank_entry_spawnable(bank, 0x3f);
    case 2:
        return pivot >= 0x10000000u && bank_entry_spawnable(bank, 0x36);
    case 4:
        return pivot >= 0x10000000u &&
               bank_entry_spawnable(bank, 0x3c) &&
               bank_entry_spawnable(bank, 0x3d);
    default:
        return 1;
    }
}

static void clear_weapon_slots(uint8_t *vehicle)
{
    *(uint32_t *)(vehicle + 0x110) = 0;
    *(uint32_t *)(vehicle + 0x114) = 0;
    *(uint32_t *)(vehicle + 0x118) = 0;
    vehicle[0xb3] = 0;
}

static int slot_count(uint8_t *vehicle)
{
    int n = 0;
    if (*(uint32_t *)(vehicle + 0x110) != 0) n++;
    if (*(uint32_t *)(vehicle + 0x114) != 0) n++;
    if (*(uint32_t *)(vehicle + 0x118) != 0) n++;
    return n;
}

static int pickup_attaches(uint16_t wtype)
{
    switch (wtype) {
    case 2:
    case 4:
    case 6:
    case 8:
    case 9:
    case 14:
        return 1;
    default:
        return 0;
    }
}

static uint8_t *make_probe_pickup(uint8_t *vehicle, uint16_t wtype, int idx)
{
    uint8_t *obj = (uint8_t *)FUN_8001d470(0x80);
    if (obj == NULL)
        return NULL;

    memset(obj, 0, 0x80);
    Object_SetCallbackPsxSlot(obj, (uintptr_t)&LAB_8003c61c);
    *(uint32_t *)(obj + 0x58) = (uint32_t)(uintptr_t)DAT_800737dc;
    *(uint16_t *)(obj + 0x06) = (uint16_t)(0x700 + idx);
    *(uint16_t *)(obj + 0x0a) = wtype;
    *(uint16_t *)(obj + 0x0c) = 5;
    *(uint16_t *)(obj + 0x0e) = 5;
    *(uint32_t *)(obj + 0x48) = *(uint32_t *)(vehicle + 0x48);
    *(uint32_t *)(obj + 0x4c) = *(uint32_t *)(vehicle + 0x4c);
    *(uint32_t *)(obj + 0x50) = *(uint32_t *)(vehicle + 0x50);
    LAB_8003c61c((intptr_t)obj, 1, 0);
    return obj;
}

static int probe_pickup_paths(uint8_t *vehicle)
{
    static const uint16_t wtypes[] = {0, 2, 4, 6, 8, 9, 10, 11, 12, 14};
    int failures = 0;
    uint8_t saved_type = vehicle[4];
    uint8_t saved_d0 = vehicle[0xd0];

    vehicle[4] = 2;
    for (unsigned i = 0; i < sizeof(wtypes) / sizeof(wtypes[0]); i++) {
        uint16_t wtype = wtypes[i];
        uint8_t *pickup;
        uint32_t desc[1];
        int before, after, ret;

        clear_weapon_slots(vehicle);
        vehicle[0xd0] = (uint8_t)(wtype == 14 ? 0 : saved_d0);
        pickup = make_probe_pickup(vehicle, wtype, (int)i);
        if (pickup == NULL) {
            fprintf(stderr, "v8: weapon_probe pickup wtype=%u FAIL alloc\n",
                    (unsigned)wtype);
            failures++;
            continue;
        }

        before = slot_count(vehicle);
        desc[0] = (uint32_t)(uintptr_t)vehicle;
        ret = (int)LAB_8003c61c((intptr_t)pickup, 3, (intptr_t)desc);
        after = slot_count(vehicle);

        fprintf(stderr,
                "v8: weapon_probe pickup wtype=%u ret=%d slots %d->%d "
                "timers=(%u,%u,%u)\n",
                (unsigned)wtype, ret, before, after,
                (unsigned)*(uint16_t *)(vehicle + 0x11c),
                (unsigned)*(uint16_t *)(vehicle + 0x11e),
                (unsigned)*(uint16_t *)(vehicle + 0x120));

        if (pickup_attaches(wtype) && after <= before) {
            fprintf(stderr,
                    "v8: weapon_probe pickup wtype=%u FAIL no attached slot\n",
                    (unsigned)wtype);
            failures++;
        }
        if (wtype == 10 && *(uint16_t *)(vehicle + 0x120) != 900) failures++;
        if (wtype == 11 && *(uint16_t *)(vehicle + 0x11c) != 900) failures++;
        if (wtype == 12 && *(uint16_t *)(vehicle + 0x11e) != 900) failures++;
    }

    clear_weapon_slots(vehicle);
    vehicle[4] = saved_type;
    vehicle[0xd0] = saved_d0;
    return failures;
}

static int probe_one_state(uint8_t *vehicle, uint8_t *target,
                           unsigned state, int fire_enabled)
{
    int failures = 0;
    uint8_t *subject = (state == 1 && target != NULL) ? target : vehicle;
    uint8_t *slot;
    uintptr_t cbp;
    ObjCb cb;
    intptr_t gate = 0, key = 0, joint = 0, fire = 0;

    clear_weapon_slots(subject);
    subject[0xd0] = (uint8_t)state;
    FUN_8002cce8((uint32_t *)subject, 0x40);

    slot = (uint8_t *)(uintptr_t)*(uint32_t *)(subject + 0x110);
    if (slot == NULL) {
        fprintf(stderr, "v8: weapon_probe state=%u FAIL no slot\n", state);
        return 1;
    }

    cbp = Object_CallbackFromPsxSlot(slot);
    cb = (ObjCb)cbp;
    if (cb == NULL) {
        fprintf(stderr, "v8: weapon_probe state=%u FAIL no callback\n", state);
        return 1;
    }

    *(uint32_t *)(subject + 0xe4) = (uint32_t)(uintptr_t)(target ? target : vehicle);
    gate = cb((intptr_t)slot, 12, (intptr_t)subject);
    key = cb((intptr_t)slot, 13, 0);
    joint = cb((intptr_t)slot, 14, 0);
    int valid_combo = state_fire_bank_requirements_met(slot, state);
    if (valid_combo && state != 4)
        cb((intptr_t)slot, 0, (intptr_t)subject);
    else if (valid_combo)
        fprintf(stderr,
                "v8: weapon_probe state=%u skip direct tick source-special combo\n",
                state);
    else
        fprintf(stderr,
                "v8: weapon_probe state=%u skip tick invalid source bank combo\n",
                state);
    if (fire_enabled && gate != 0 && valid_combo && state != 4)
        fire = cb((intptr_t)slot, 11, (intptr_t)subject);
    else if (fire_enabled && state == 4)
        fprintf(stderr,
                "v8: weapon_probe state=%u skip direct fire source-special combo\n",
                state);
    else if (fire_enabled && gate == 0)
        fprintf(stderr,
                "v8: weapon_probe state=%u skip direct fire source gate closed\n",
                state);
    else if (fire_enabled)
        fprintf(stderr,
                "v8: weapon_probe state=%u skip direct fire invalid source bank combo\n",
                state);

    fprintf(stderr,
            "v8: weapon_probe state=%u slot=%p cb=%p gate=%ld key=0x%lx "
            "joint=0x%lx fire=%ld fired=%d ammo=%u flags=0x%x\n",
            state, (void *)slot, (void *)cbp, (long)gate,
            (unsigned long)key, (unsigned long)joint, (long)fire,
            fire_enabled, (unsigned)*(uint16_t *)(slot + 0x0c),
            (unsigned)*(uint32_t *)slot);

    if (key == 0 || joint == 0) {
        fprintf(stderr,
                "v8: weapon_probe state=%u FAIL missing source key/joint\n",
                state);
        failures++;
    }
    if (fire_enabled && fire == 0 && *(uint16_t *)(slot + 0x0c) == 0) {
        fprintf(stderr,
                "v8: weapon_probe state=%u FAIL fire produced zero cooldown and no ammo\n",
                state);
        failures++;
    }

    return failures;
}

static int probe_state_callbacks(uint8_t *vehicle, uint8_t *target)
{
    int failures = 0;
    uint8_t saved_d0 = vehicle[0xd0];

    if (g_v8_weapon_probe_state >= 0) {
        if (g_v8_weapon_probe_state >= 13)
            return 1;
        failures += probe_one_state(vehicle, target,
                                    (unsigned)g_v8_weapon_probe_state, 1);
        clear_weapon_slots(vehicle);
        vehicle[0xd0] = saved_d0;
        return failures;
    }

    for (unsigned state = 0; state < 13; state++)
        failures += probe_one_state(vehicle, target, state, 0);

    clear_weapon_slots(vehicle);
    vehicle[0xd0] = saved_d0;
    return failures;
}

int Host_WeaponProbe(void)
{
    uint8_t *vehicle = (uint8_t *)puRam000007d0;
    uint8_t *target = (uint8_t *)puRam000007d4;
    int failures = 0;

    if (vehicle == NULL) {
        fprintf(stderr, "v8: weapon_probe FAIL no player vehicle\n");
        return 2;
    }

    fprintf(stderr,
            "v8: weapon_probe begin player=%p target=%p bank=%p\n",
            (void *)vehicle, (void *)target, (void *)(uintptr_t)DAT_800737dc);

    if (g_v8_weapon_probe_state < 0)
        failures += probe_pickup_paths(vehicle);
    failures += probe_state_callbacks(vehicle, target);

    fprintf(stderr, "v8: weapon_probe %s failures=%d\n",
            failures == 0 ? "PASS" : "FAIL", failures);
    return failures == 0 ? 0 : 1;
}
