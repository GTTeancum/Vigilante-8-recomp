/* radar_sweep.c -- Secret Base radar dish sweep + target ping.
 *
 * Source: SCRTBASE.DLL  FUN_80100200.
 *
 * Manages the rotating radar dish that pings nearby vehicles. Holds
 * a per-target list head at _DAT_80065a18; every 4 frames bumps the
 * damage arbiter; when the sweep counter at self+8 underflows, walks
 * the target list and triggers a ping FX on any kind==2 vehicle with
 * health!=0 and Z < 0x4b32000.
 *
 * mode dispatch:
 *   2  -- shutdown / retire
 *   1  -- init with bank base -0x7ffa0000 + 0x5a50 = 0x80065a50
 *   default -- init with bank +1 (0x80065a51)
 *   0x11 -- damage event only (skip sweep work)
 *
 * Path attachment via Path_Pick + func_0x8003d080 gates whether
 * dish is "active" (_DAT_80065a10).
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t Path_Pick(int bank, int seed);                          /* FUN_8001ffd4 */
extern int  Path_AttachToObj(uint32_t flag, uint32_t path);             /* func_0x8003d080 */
extern void SB_QueueDamage(void);                                       /* func_0x80023d00 */
extern void FX_QueueAtTarget(uint32_t *head, int kind);                 /* func_0x8002002c */
extern void Damage_StandardVehicleAlt(int self, int *imp);              /* func_0x80022120 */
extern int  Rand255(void);
extern void Damage_Apply_AgainstSelf(void *self, void *param);
extern void Sched_Yield(uint32_t obj, void *cb, int aux);               /* FUN_8002123c */
extern int8_t LAB_801001ac;
extern uint32_t _DAT_80065a10, _DAT_80065a00;
extern int   *_DAT_80065a18;
extern int    DAT_80102bcc;

uint32_t SB_RadarSweep(int self, uint32_t mode, int *arg)
{
    if (mode != 2) {
        int base = -0x7ffa0000;
        if (mode > 2 || mode != 1) {
            if (mode == 0x11) goto fwd;
            base = 1;
        }
        DAT_80102bcc = 0;
        uint32_t path = Path_Pick(base + 0x5a50, 0x100);
        _DAT_80065a10 = (Path_AttachToObj(0x7f000000u, path) != 0);
        *(uint8_t *)(self + 8) = 0x1e;
    }
    uint16_t spin = *(uint16_t *)(self + 0xc);
    *(uint16_t *)(self + 0xc) = (uint16_t)(spin + 1);
    if ((spin & 3) == 0) SB_QueueDamage();
    int8_t cd = *(char *)(self + 8) - 1;
    *(char *)(self + 8) = cd;
    if (cd == 0) { arg = (int *)1; cd = -1; }
    if (cd < 0) {
        int *node = (int *)*_DAT_80065a18;
        int **head = (int **)_DAT_80065a18;
        while (node != NULL) {
            int *cell = (int *)head[2];
            if (*(char *)((intptr_t)cell + 4) == 2
                && *(int16_t *)((intptr_t)cell + 0xc) != 0
                && *(int *)((intptr_t)cell + 0x50) < 0x4b32000)
            {
                FX_QueueAtTarget(_DAT_80065a18, 9);
            }
            head = (int **)node;
            node = (int *)*node;
        }
        if (*(char *)(self + 8) == -4) {
            int r = Rand255();
            *(char *)(self + 8) = (char)((r * 0x1e) >> 15) + 0x1e;
            Sched_Yield(_DAT_80065a00, &LAB_801001ac, 0);
            arg = NULL;
        }
    }
    Damage_Apply_AgainstSelf((void *)(intptr_t)self, (void *)(intptr_t)0x3c);
    self = 1;
fwd:
    Damage_StandardVehicleAlt(self, arg);
    return 0;
}
