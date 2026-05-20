/* bomb_list.c -- Casino City bomb-attached objects linked list.
 *
 * Source: CASNOCTY.DLL  FUN_80101d00.
 *
 * Owns a per-target list of "attached" detonators (each a
 * {next, prev, ownerObj, flags+expiry} record at self+0x20..). On
 * collision (mode==0 or fallthrough), checks impactor flag 0x4000,
 * attaches a new detonator with countdown = _DAT_80065310 + 300,
 * applies a -20 damage tick, and queues a per-target hit notification
 * (func_0x8002002c with the casino bomb FX table at 0x80065a18).
 *
 * Each tick walks the list; entries whose deadline lapsed get a
 * second damage application (Damage_VsImpactorAlt -20) and FX play.
 * Live targets get re-tagged (0x4000) and re-registered. If any
 * entry actually fired, plays the world SFX (FUN_800446dc).
 *
 * mode dispatch:
 *   1, 6  -- init/clear: head of list -> empty (self+0x20/21/22 link
 *            to self), state byte (self+1) = 3.
 *   2     -- silent retire.
 *   default (0/3): main tick (above).
 *
 * MED.
 */
#include <stdint.h>

extern int  Bomb_AttachNew(uint32_t *head, uint32_t *target);          /* FUN_8001fe50 */
extern int  Bomb_AttachOrRecycle(uint32_t *head, uint32_t *target);    /* FUN_8001ff0c */
extern void Bomb_UnlinkOne(int *node);                                  /* func_0x80022c78 */
extern void Damage_Apply_AgainstSelf(void *self, void *param);          /* FUN_80020890 */
extern void Damage_VsImpactorAlt(int imp, int dmg, void *p, int n);     /* FUN_8002c6fc */
extern void FX_QueueAtTarget(uint32_t bank, int kind, uint32_t *obj, ...); /* func_0x8002002c */
extern uint8_t Pool_AllocSFX(void);                                     /* FUN_8004410c */
extern void Pool_BindFXOnObject(int h, uint32_t bin, int slot, int aux);/* FUN_800443c8 */
extern void SFX_StopWorld(int h);                                       /* FUN_800441c8 */
extern uint8_t SFX_PlayWorldXY(uint32_t *posXyz);                       /* FUN_800446dc */
extern void SFX_Update(int handle, int posVoxel);                       /* FUN_80044574 */
extern uint8_t  DAT_80100110;
extern uint32_t DAT_80065a18;
extern uint32_t _DAT_80065310;

uint32_t CC_BombList(uint32_t *self, uint32_t mode, uint32_t **arg)
{
    int fired = 0;

    if (mode == 1 || mode == 6) goto init_head;
    if (mode != 2) {
        uint32_t *target = arg ? *arg : NULL;
        if (target == NULL || (char)target[1] != 2) return 0;
        uint32_t *head = self + 0x20;
        if (*target & 0x4000) {
            int node = Bomb_AttachNew(head, target);
            *(uint32_t *)(node + 0xc) = (_DAT_80065310 + 300) | 1;
            if ((*self & 1) == 0) Damage_Apply_AgainstSelf(self, (void *)(intptr_t)0x3c);
            *target &= ~0x4000u;
            FX_QueueAtTarget(DAT_80065a18, 9, target, 0x58a, 4, 0x3c);
            head = (uint32_t *)1;
        }
        int node = Bomb_AttachOrRecycle(head, target);
        *(uint32_t *)(node + 0xc) |= 1;
    }
    /* Per-tick list walk. */
    int *cur  = (int *)self[0x20];
    int *next;
    uint32_t now = _DAT_80065310;
    for (int *node = (int *)*(int *)self[0x20]; node != NULL; node = next) {
        next = (int *)*node;
        if ((cur[3] & 1) != 0 && *(int16_t *)(cur[2] + 0xc) != 0) {
            uint32_t f = cur[3] & ~1u;
            cur[3] = f;
            if (f < now) {
                Damage_VsImpactorAlt(cur[2], -20, &DAT_80100110, 1);
                fired = 1;
                if (*((char *)self + 5) == 0) {
                    char h = (char)Pool_AllocSFX();
                    *((char *)self + 5) = h;
                    Pool_BindFXOnObject(h, *(uint32_t *)(self[0x16] + 8), 2, 0);
                }
            }
            FX_QueueAtTarget(DAT_80065a18, 9, (uint32_t *)cur[2]);
            cur = next;
        }
        *(uint32_t *)cur[2] |= 0x4000u;
        Bomb_UnlinkOne(cur);
        cur = next;
        now = _DAT_80065310;
    }
    if (fired) goto sfx_update;
    if (*((char *)self + 5) != 0) {
        SFX_StopWorld((int)*((char *)self + 5));
        *((char *)self + 5) = 0;
sfx_update:
        if (*((char *)self + 5) != 0)
            SFX_Update((int)*((char *)self + 5), SFX_PlayWorldXY(self + 0x12));
    }
    if ((uint32_t *)self[0x22] == self + 0x20) return 0;
    Damage_Apply_AgainstSelf(self, (void *)(intptr_t)0x3c);
init_head:
    *(uint8_t *)(self + 1) = 3;
    self[0x20] = (uint32_t)(uintptr_t)(self + 0x21);
    self[0x21] = 0;
    self[0x22] = (uint32_t)(uintptr_t)(self + 0x20);
    return 0;
}
