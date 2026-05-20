/* bridge_collapse.c -- Wild West bridge plank collapse handler.
 *
 * Source: WILDWEST.DLL  FUN_801010a8.
 *
 * Per-plank impact handler. On impact (kind==7) and damage tier
 * exceeded (self+3 lower 7 bits < imp+0xc damage), drop the plank's
 * physics constraint: allocate a free-rigid body via FUN_8003ff28,
 * zero its impulse/gravity except Y=-0xbeb (3.0 in 4.12 down), spawn
 * a debris projectile (kind 0x29) tied to the bridge bin, set its
 * post-tick callback to 0x8003cb64.
 *
 * State transitions:
 *   case 0  -- per-tick: if HP exhausted (self+0xc == 0) retire.
 *   case 3  -- impact event.
 *   case 2  -- after impact, if self+0x29 (chain link counter) == 0
 *              detach the submodel via SubModel_Detach.
 *   case 6, 1, default -- no-op (handled elsewhere).
 *   case 8, 9 -- routed targets.
 *
 * MED.
 */
#include <stdint.h>

extern void Object_RetireDeferred(uint32_t *self);
extern int  RigidBody_Alloc(void);                                /* FUN_8003ff28 */
extern uint32_t Pool_AllocProjectile(void);
extern void Pool_LaunchProjectile(uint32_t h, uint32_t bin, int kind, void *xyz);
extern void Damage_StandardVehicle(uint32_t *self, void *imp);    /* func_0x80022320 */
extern void Damage_Apply_AgainstSelf(void *self, void *param);
extern void SubModel_Detach(uint32_t *self);
extern uint32_t _DAT_800658fc;

uint32_t WW_BridgeCollapse(uint32_t *self, uint32_t mode, int *arg)
{
    int *imp = arg;
    switch (mode) {
    case 0:
        if (self[0xc] == 0) Object_RetireDeferred(self);
        /* fall through */
    case 3:
        if (*(char *)(*imp + 4) == 7) {
            if (self[0xe] != 0 &&
                ((uint16_t)self[3] & 0x7f) < *(uint16_t *)(*imp + 0xc))
            {
                int rb = RigidBody_Alloc();
                *(uint16_t *)(rb + 0xc) = 0;
                *(uint32_t *)(rb + 0x88) = 0;
                *(uint32_t *)(rb + 0x8c) = 0xfffff415u;  /* -0xbeb */
                *(uint32_t *)(rb + 0x90) = 0;
                uint32_t h = Pool_AllocProjectile();
                Pool_LaunchProjectile(h, _DAT_800658fc, 0x29, (void *)(intptr_t)(rb + 0x24));
                *(uint32_t *)(rb + 100) = 0x8003cb64u;
            }
            Damage_StandardVehicle(self, (void *)(intptr_t)*(uint16_t *)(*imp + 0xc));
        }
        if ((*self & 1) == 0 && (uint32_t *)imp[3] != self) {
            int rb = RigidBody_Alloc();
            *(uint32_t *)(rb + 100) = 0x8003cb64u;
            *(uint16_t *)(rb + 0xc)  = 0;
            Damage_Apply_AgainstSelf(self, (void *)(intptr_t)300);
            /* fall to case 8 */
        case 8:
            Damage_StandardVehicle(self, imp);
            /* fall to case 2 */
        case 2:
            if (self[0x29] == 0) {
                SubModel_Detach(self);
        case 9:
                if (*(int16_t *)((char *)imp + 6) == *(int16_t *)((char *)self + 6))
                    *self |= 0x10000u;
            }
        }
        break;
    case 6: case 1: default: break;
    }
    return 0;
}
