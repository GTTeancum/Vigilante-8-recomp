/* security_door.c -- Secret Base security-door arm/disarm.
 *
 * Source: SCRTBASE.DLL  FUN_80101904.
 *
 * Manages a sliding security door whose state is held at self+8
 * (0=disarmed, 1=armed). When a vehicle (kind==2) or shrapnel
 * (kind==1) hits the trigger pad with matching player-side, marks
 * the trigger object 0x20 (consumed) and:
 *   - tracks down door panel
 *   - calls FUN_8001f9cc with combined (self+0x46) + (*self+0x60) as
 *     the slide-distance signal
 *   - spawns a payload at impactor+0x24 (kind 0x14)
 *
 * mode dispatch:
 *   1   -- post-spawn init (clear child kind==0 list)
 *   2   -- retire/disarm (cycle linked panels via +0x78, clear flag)
 *   3   -- impactor kind==2 trigger
 *   8   -- impactor kind==1 trigger
 *   default -- forward to Damage_StandardVehicle
 *
 * MED.
 */
#include <stdint.h>

extern void Door_SlideTo(int self, uint32_t target);                    /* FUN_8001f9cc */
extern uint32_t Pool_AllocPayload(void);                                /* FUN_8004410c */
extern void Pool_BindPayloadToObject(uint32_t h, uint32_t bin, int kind,
                                     uint32_t *spawnXyz);                /* FUN_800447e8 */
extern void Damage_FromImpulse(int self, int *imp);                      /* func_0x8002239c */
extern int  Damage_StandardVehicle(int self, int *imp);                  /* func_0x80022320 */
extern void Damage_RetireSelfAlt(int self);                              /* func_0x80020844 */
extern void Door_FreeChildren(int self);                                 /* func_0x8001abd0 */
extern uint32_t _DAT_800658fc;

uint32_t SB_SecurityDoor(int self, uint32_t mode, int *arg)
{
    int8_t want = 2;
    if (mode == 2) goto disarm;
    if (mode < 3) {
        if (mode == 1) goto reinit_chain;
        want = 1;
    }
    if (mode == 3 || mode == 8 || (mode != 8 && (mode == 3 ? 1 : want = 1, 1))) {
        uint32_t *trig = (uint32_t *)(uintptr_t)arg[3];
        uint8_t   tk   = (uint8_t)trig[1];
        int8_t    impK = *(char *)(*arg + 4);
        if (tk == mode && impK == want) {
            *trig |= 0x20u;
            if (*(char *)(self + 8) != 1) return 0;
            *(char *)(self + 8) = impK;
            Door_SlideTo(self, (uint32_t)*(uint16_t *)(self + 0x46)
                              + (uint32_t)**(uint16_t **)(self + 0x60));
            uint32_t h = Pool_AllocPayload();
            Pool_BindPayloadToObject(h, _DAT_800658fc, 0x14, (uint32_t *)(uintptr_t)(*arg + 0x24));
        }
        if (mode == 3) Damage_FromImpulse(self, arg);
        if (Damage_StandardVehicle((mode == 3 ? 1 : self), arg) == 0) return 0;
        Damage_RetireSelfAlt(self);
    }
disarm: {
        uint32_t *chain = *(uint32_t **)(self + 0x78);
        uint32_t v     = *chain & ~0x20u;
        *chain = v;
        *(char *)(self + 8) = 0;
        while (1) {
            *(uint32_t *)(self + 0x78) = v;
            *(char *)(v + 4) = (char)(uintptr_t)chain;
reinit_chain:;
            uint32_t c = *(uint32_t *)(self + 0x38);
            while (1) {
                if (c == 0) {
                    Door_FreeChildren(self);
                    Door_SlideTo(self, 0);
                    return 0;
                }
                chain = (uint32_t *)3;
                if (*(int16_t *)(c + 6) == 0) break;
                c = *(uint32_t *)(c + 0x34);
            }
        }
    }
}
