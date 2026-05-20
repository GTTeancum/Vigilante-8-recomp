/* silo_rotate.c -- Valley Farms silo rotate + AAA-fire target pick.
 *
 * Source: VALLYFRM.DLL  FUN_80100964.
 *
 * Per-tick rotates the silo dish at obj+0x38, +0x44 by +0x44 per
 * frame. On impactor event (kind 3/8), forwards damage; on damage
 * success, picks nearest hostile target from _DAT_80065a18 list,
 * preferring "high-priority" targets (object+6 > 0). Allocates a
 * 0x98-byte projectile (FUN_80045134 realloc), copies the muzzle
 * transform from world space, sets velocity = 2 * facing, binds FX
 * channel 1, marks kind 7 (AAA round) with 100-frame fuse and tick
 * callback FUN_801005e8.
 *
 * mode dispatch:
 *   0  -- tick rotate + (optional) damage propagate
 *   1  -- post-spawn: seed dish angle via Rand255
 *   3  -- damage event from impactor
 *   8  -- forwarded variant
 *
 * MED.
 */
#include <stdint.h>

extern void Object_RefitAABB(int sub);
extern void Damage_FromImpulse(int self, int *imp);
extern int  Damage_StandardVehicle(int self, int *imp);
extern uint32_t Object_LocalToWorldPos(int sub);
extern int  Math_Distance(int xy, int target);
extern uint32_t Object_BuildLocalCoords(int sub);                  /* FUN_8001d564 */
extern void *Heap_Realloc(uint32_t base, uint32_t newSize);
extern void Object_BindLifecycle(uint32_t *self);
extern uint8_t Pool_AllocSFX(void);
extern void Pool_BindFXOnObject(uint32_t h, uint32_t bin, int slot, uint32_t *xyz);
extern int  Rand255(void);
extern int *_DAT_80065a18;
extern uint32_t FUN_801005e8;

uint32_t VF_SiloRotate(int self, int mode, int param3)
{
    if (mode == 1) goto seed_angle;
    if (mode == 0) {
        *(int16_t *)(*(int *)(self + 0x38) + 0x44) += 0x44;
        if (param3 == 0) return 0;
        Object_RefitAABB(*(int *)(self + 0x38));
    }
    if (mode == 3 || mode == 8 || mode == 0) {
        if (mode == 3) Damage_FromImpulse(self, (int *)(intptr_t)param3);
        if (Damage_StandardVehicle((mode == 3 ? 1 : self), (int *)(intptr_t)param3) == 0)
            return 0;
        /* Pick target. */
        uint32_t *muzzle = (uint32_t *)Object_LocalToWorldPos(*(int *)(self + 0x38));
        int    fallback = 0;
        int    primary  = 0;
        uint32_t bestF  = 0xffffffffu;
        uint32_t bestP  = 0xffffffffu;
        int *cur  = (int *)*_DAT_80065a18;
        int *prev = _DAT_80065a18;
        while (cur != NULL) {
            int cand = prev[2];
            if (*(char *)(cand + 4) == 2 && *(int16_t *)(cand + 0xc) != 0) {
                uint32_t d = (uint32_t)Math_Distance(self + 0x48, cand + 0x48);
                if (*(int16_t *)(cand + 6) > 0 && d < bestP) {
                    bestP   = 1;
                    primary = cand;
                }
                if (d < bestF) { bestF = d; fallback = cand; }
            }
            prev = cur;
            cur  = (int *)*cur;
        }
        int target = (primary != 0) ? primary : fallback;
        uint32_t lc = Object_BuildLocalCoords(*(int *)(self + 0x38));
        uint32_t *p = (uint32_t *)Heap_Realloc(lc, 0x98);
        p[0x21] = (uint32_t)target;
        *(int16_t *)((char *)p + 6) = *(int16_t *)(self + 6);
        for (int i = 0; i < 4; i++) p[4 + i]    = muzzle[i];
        for (int i = 0; i < 4; i++) p[8 + i]    = muzzle[4 + i];
        for (int i = 0; i < 3; i++) p[0x12 + i] = muzzle[5 + i];
        *(uint8_t *)(p + 1) = 7;
        *p = 0x80u;
        *(int16_t *)(p + 3) = 100;
        p[0x19] = (uint32_t)(uintptr_t)&FUN_801005e8;
        Object_BindLifecycle(p);
        p[0x22] = (int)*(int16_t *)(p + 5) << 1;
        p[0x23] = (int)*(int16_t *)((char *)p + 0x1a) << 1;
        p[0x24] = (int)*(int16_t *)(p + 8) << 1;
        char h = (char)Pool_AllocSFX();
        *((char *)p + 5) = h;
        Pool_BindFXOnObject(h, *(uint32_t *)(*(int *)(self + 0x58) + 8), 1, p + 9);
    }
seed_angle:
    *(uint16_t *)(*(int *)(self + 0x38) + 0x44) = (uint16_t)Rand255();
    return 0;
}
