/* turret_track.c -- Secret Base turret target tracking + fire.
 *
 * Source: SCRTBASE.DLL  FUN_80100970.
 *
 * Per-tick handler for SB defense turrets:
 *   case 0 (tick): rotate turret toward currently-locked target (held
 *       at self+0x78). Atan2(localX/localZ) -> yaw delta /4; pitch
 *       similarly with -1.0 scale, clamped to [-0x155, +0x155]. If
 *       target lost (Z>=0xfa001 or HP==0) clear lock.
 *   case 2: silent retire.
 *   case 3, 8: damage event, decrement counter (Damage_AccumulateOr-
 *       Fire) and reschedule damage timer.
 *   case 1: post-spawn init.
 *
 * Lock acquisition (when self+0x78 == 0): walk the global target
 * list at _DAT_80065a18; first kind==2 with HP!=0 within radius
 * 0xc7fff becomes the new lock; spawns the lock-on FX (object id
 * 0x1e3, attached via FUN_80031300 with sub-id 0x1e1, kind 0x98,
 * tick callback FUN_8010076c).
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t Object_LocalToWorldPos(int sub);                  /* FUN_8001d624 */
extern void Util_TransposeMatRotate(uint32_t mat, int p, int *out); /* FUN_800435c0 */
extern int  ratan2(int dz, int dx);
extern void Object_RefitAABB(void);                                /* FUN_8001d708 */
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag); /* FUN_8001ac44 */
extern uint32_t Object_SpawnAttached(int parent, int subBin, int kindA, int kindB, uint32_t *parentSlot); /* FUN_80031300 */
extern uint32_t Pool_AllocSFX(void);
extern void Pool_BindFXOnObject(uint32_t h, uint32_t bin, int slot, void *xyz); /* FUN_8004483c */
extern void Pool_BindSnareToObject(uint32_t h, uint32_t bin, int slot, int xyz); /* FUN_800447e8 */
extern void Object_BindLifecycle(uint32_t *self);                  /* FUN_800202f4 */
extern void Object_BindFinalize(void);                              /* FUN_800207c4 */
extern void Damage_RetireSelf(int self);                            /* FUN_80020744 */
extern int  Math_Distance(int xy1, uint32_t *xy2);                  /* FUN_80016aac */
extern int  Damage_StandardVehicle(int self, int *imp);             /* func_0x80022320 */
extern void Damage_RetireSelfAlt(int self);                         /* func_0x80020844 */
extern int  Damage_AccumulateOrFire(uint32_t *self, uint16_t a);    /* FUN_80020778 */
extern void Damage_Apply_AgainstSelf(void *self, void *param);
extern int  *_DAT_80065a18;
extern uint32_t FUN_8010076c;

uint32_t SB_TurretTrack(int self, uint32_t mode, int *imp)
{
    int sub = *(int *)(self + 0x38);
    int trg = *(int *)(self + 0x78);

    switch (mode) {
    case 0: {
        uint32_t wm = Object_LocalToWorldPos(sub);
        int local[3] = { 0, 0, 0 };
        Util_TransposeMatRotate(wm, trg + 0x48, local);
        int yaw = ratan2(local[0], local[2]) << 20 >> 20;
        int pit = ratan2(local[1], local[2]) * -0x100000 >> 20;
        if (yaw < 0) yaw += 3;
        *(int16_t *)(sub + 0x42) += (int16_t)(yaw >> 2);
        if (pit < 0) pit += 3;
        int next = (int)*(int16_t *)(sub + 0x40) + (pit >> 2);
        if (next < -0x155) next = -0x155;
        if (next >  0x155) next =  0x155;
        *(int16_t *)(sub + 0x40) = (int16_t)next;
        Object_RefitAABB();
        if (local[2] < 0xfa001 && *(int16_t *)(trg + 0xc) != 0) return 0;
        *(int *)(self + 0x78) = 0;
        break;
    }
    case 1: goto schedule;
    case 2: break;
    case 3: goto damage;
    case 8: goto forward;
    default: return 0;
    }

    /* Reacquire lock. */
    if (*(int *)(self + 0x78) == 0) goto search;

    {
        uint32_t parent_fx = Object_SpawnFromBank(*(uint32_t *)(self + 0x58), 0x1e3, 0x80, 8);
        uint32_t fx       = Object_SpawnAttached(self, *(int *)(self + 0x38), 0x1e1, 0x98,
                                                 (uint32_t *)(uintptr_t)parent_fx);
        uint32_t *fp = (uint32_t *)(uintptr_t)fx;
        *fp = 0x84u;
        *(int16_t *)(fp + 3) = 0x32;
        fp[0x19] = (uint32_t)(uintptr_t)&FUN_8010076c;
        *(int16_t *)((char *)fp + 0x96) = 4;
        *(int16_t *)(fp + 0x25) = 8;
        Object_BindLifecycle(fp);
        *(uint32_t *)(uintptr_t)parent_fx = 0x10u;
        *(uint32_t *)(uintptr_t)(parent_fx + 0x19 * 4) = 0x8003e80cu;
        Object_BindFinalize();
        uint32_t h = Pool_AllocSFX();
        Pool_BindFXOnObject(h, *(uint32_t *)(*(int *)(self + 0x58) + 8), 0, fp + 9);
        for (;;) {
            *(uint32_t **)(self + 0x78) = fp;
            Damage_RetireSelf(self);
            uint32_t h2 = Pool_AllocSFX();
            Pool_BindSnareToObject(h2, *(uint32_t *)(*(int *)(self + 0x58) + 8), 2, trg);
search:
            if ((int *)*_DAT_80065a18 == NULL) break;
            int  scan = self + 0x48;
            int *prev = _DAT_80065a18;
            int *cur  = (int *)*_DAT_80065a18;
            while (cur != NULL) {
                uint32_t *cand = (uint32_t *)prev[2];
                int d;
                if (*(char *)(cand + 1) == 2 && *(int16_t *)(cand + 3) != 0 &&
                    (d = Math_Distance(scan, cand + 0x12)) <= 0xc7fff)
                    break;
                prev = cur;
                cur  = (int *)*cur;
            }
            if (cur == NULL) goto post_search;
        }
    }
post_search:
    imp = (int *)0x3c;
damage:
    if (*(char *)(*imp + 4) == 7) {
        imp = (int *)(uintptr_t)*(uint16_t *)(*imp + 0xc);
forward:
        if (Damage_StandardVehicle(self, imp) != 0) {
            Damage_RetireSelfAlt(self);
            Damage_AccumulateOrFire((uint32_t *)(uintptr_t)self, 0);
schedule:
            Damage_Apply_AgainstSelf((void *)(intptr_t)self,
                                     (void *)(intptr_t)(*(uint8_t *)(self + 9) + 0x1e));
        }
    }
    return 0;
}
