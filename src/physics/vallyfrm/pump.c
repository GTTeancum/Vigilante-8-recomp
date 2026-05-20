/* pump.c -- Valley Farms pump_1 / silo_1 collision + countdown.
 *
 * Source: VALLYFRM.DLL  FUN_80101220.
 *
 * On collision (mode 3, 1, 8) tests against terrain + circular hit;
 * on success, queues a snow effect (Effects_QueueSnow) and stores
 * the count into +0x8. Each tick decrements +0x8 by 1; on hitting 1,
 * if a global concurrency counter DAT_80101308 is < 2, increments it
 * and broadcasts event 1000 via func_0x80021d6c (Object_BroadcastTreeEvent).
 *
 * Used by the pump_1 and silo_1 destructibles -- limits the number
 * of simultaneous "explosion sequence in progress" to 2.
 *
 * MED confidence.
 */
#include <stdint.h>

extern int  Damage_FromImpulse(uint32_t *self, int *impulse);
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t amount);
extern uint8_t Effects_QueueSnow(int obj);    /* FUN_8003fc94 */
extern void Object_BroadcastTreeEvent(uint32_t event);  /* func_0x80021d6c */
extern int32_t DAT_80101308;

uint32_t VF_PumpTick(int obj, uint32_t mode, void *impact)
{
    int hit = 0;
    if (mode == 3 || mode == 1 || mode == 8) {
        if (mode == 8) goto circularOnly;
        if ((hit = Damage_FromImpulse((uint32_t *)(intptr_t)obj, (int *)impact)) != 0) goto fired;
        circularOnly:
        if ((hit = Damage_AccumulateOrFire((uint32_t *)(intptr_t)obj, (uint16_t)(uintptr_t)impact)) != 0) goto fired;
    }
    *(uint8_t *)(obj + 8) = Effects_QueueSnow(obj);
fired:
    {
        int8_t counter = *(int8_t *)(obj + 8);
        *(int8_t *)(obj + 8) = (int8_t)(counter - 1);
        if (counter == 1 && DAT_80101308 < 2) {
            DAT_80101308++;
            Object_BroadcastTreeEvent(1000);
        }
    }
    return 0;
}
