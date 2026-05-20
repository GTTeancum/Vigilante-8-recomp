/* conveyor_grab.c -- Sand Factory conveyor "grab vehicle" event.
 *
 * Source: SANDFACT.DLL  FUN_80100e54.
 *
 * When a vehicle drives over an active conveyor segment with the
 * "snag" flag (impactor.flags[+0]==1 && impactor.flags[+2]!=0), the
 * conveyor takes ownership of the vehicle: rebinds its tick to the
 * conveyor's drag callback (FUN_80100c6c), zeros its X velocity,
 * forces Y velocity to 0x1c980 (downward into rollers), zeros Z.
 * If the vehicle has a sub-model, also forces its yaw spin (+0x86)
 * and Y velocity (+0x22) to fixed scary-rotation values.
 *
 * Position-snap: target the midpoint of the conveyor segment bounds
 * (avg of psVar6[2..6] and [8..0xc] for X,Z; psVar6[10] for Y),
 * rotate into the conveyor's local frame, then set velocity to 2x
 * the delta needed to reach it (snap-arrives in two frames).
 *
 * Plays the snare FX, schedules a 64-frame damage event.
 *
 * mode dispatch:
 *   3, 8  -- impact event with kind==2 (vehicle) -> snare
 *   kind==7 (shrapnel) -> forward to standard damage
 *
 * MED.
 */
#include <stdint.h>

extern int  Conveyor_FindGrabSlot(int self, int radius);                /* FUN_8001b038 */
extern uint32_t Pool_AllocSnare(void);                                   /* FUN_8004410c */
extern void Pool_BindSnareToObject(uint32_t h, uint32_t bin, int slot,
                                   uint32_t *spawnXyz);                  /* FUN_800447e8 */
extern void SFX_Update(int handle, int posVoxel);                        /* FUN_80044574 */
extern void GTE_RotateLongMatTrans(uint32_t *mat, int *src, int *dst);   /* FUN_80043408 */
extern void Conveyor_RebuildLocalFrame(uint8_t out[20], int self, int slot); /* FUN_8001d68c */
extern void Damage_Apply_AgainstSelf(void *self, void *param);           /* FUN_80020890 */
extern void Damage_StandardVehicle(int self, int *imp);                  /* func_0x80022320 */
extern uint32_t FUN_80100c6c;  /* per-segment drag tick */

uint32_t SF_ConveyorGrab(int self, int mode, int *arg)
{
    if (mode != 3 && mode != 8) goto fwd;

    int16_t *flags = (int16_t *)(uintptr_t)arg[1];
    uint32_t *imp  = (uint32_t *)(uintptr_t)*arg;
    if (flags[0] == 1 && flags[1] != 0 && (char)imp[1] == 2) {
        uint32_t *sub = (uint32_t *)(uintptr_t)imp[0x38];
        int slot   = Conveyor_FindGrabSlot(self, 0x8000);
        uint32_t h = Pool_AllocSnare();
        Pool_BindSnareToObject(h, *(uint32_t *)(*(int *)(self + 0x58) + 8), 5, imp + 9);
        SFX_Update((int)*((char *)imp + 5), 0);
        imp[0x19] = (uint32_t)(uintptr_t)&FUN_80100c6c;
        imp[0x20] = 0;
        *imp |= 0x3000022u;
        imp[0x21] = 0x1c980;
        imp[0x22] = 0;
        if (sub != NULL) {
            *(uint16_t *)(sub + 0x21) = 0;
            *sub |= 0xc0000u;
            *(uint16_t *)((char *)sub + 0x86) = 0xfe37u;
            *(uint16_t *)(sub + 0x22)         = 0xf415u;
        }
        int mid[3];
        mid[0] = (*(int *)(flags + 2) + *(int *)(flags + 8)) / 2;
        mid[1] = *(int *)(flags + 10);
        mid[2] = (*(int *)(flags + 6) + *(int *)(flags + 0xc)) / 2;
        GTE_RotateLongMatTrans((uint32_t *)(uintptr_t)(self + 0x10), mid, mid);
        imp[0x20] = (mid[0] - imp[9])  * 2;
        imp[0x21] = (mid[1] - imp[10]) * 2;
        imp[0x22] = (mid[2] - imp[0xb]) * 2;
        uint8_t stash[20];
        struct { uint32_t a, b, c; } lp;
        Conveyor_RebuildLocalFrame(stash, self, slot);
        imp[0x12] = lp.a; imp[0x13] = lp.b; imp[0x14] = lp.c;
        imp[0x10] = *(uint32_t *)(slot + 0x10);
        *(uint16_t *)(imp + 0x11) = *(uint16_t *)(slot + 0x14);
        Damage_Apply_AgainstSelf(imp, (void *)(intptr_t)0x40);
        arg = (int *)1;
    }
    if (*(char *)(*arg + 4) != 7) return 0;
    arg = (int *)(uintptr_t)*(uint16_t *)(*arg + 0xc);
fwd:
    Damage_StandardVehicle(self, arg);
    return 0;
}
