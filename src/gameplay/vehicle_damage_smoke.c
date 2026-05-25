/* vehicle_damage_smoke.c -- spawn a smoke effect on a damaged vehicle.
 *
 * Source: SLUS_005.10  FUN_8002c3ac.
 *
 * Called from the damage path when the vehicle's smoke flag (0x4000000)
 * is clear and the smoke-emitter object slot at +0x11e is empty.
 *
 *   1. Allocate a 0x9c-byte effect object.
 *   2. Install LAB_8002c210 as the per-frame tick at +0x64.
 *   3. Set scale (+0x54) = 0x10000 (1.0 in 16.16).
 *   4. Zero +0x24..0x2c (world pos cache).
 *   5. Set flags (+0x00) |= 0xa4 (smoke object type + render hint).
 *   6. Initialise per-instance fields:
 *        +0x96 (i16) = 0x22       (smoke variant byte + size)
 *        +0x82 (i16) = 4          (palette index)
 *        +0x0c (i16) = 2          (smoke kind)
 *        +0x84 (i32) = 0x200      (initial alpha)
 *        +0x98 (i32) = world bank (DAT_800737d8)
 *        +0x88 (i32) = -0x600     (initial Y velocity, downward in screen space?)
 *        +0x8c (i32) = 0
 *   7. Parent-link via Object_PrependChild (FUN_8001d544).
 *   8. Mark world-bind (FUN_80020744).
 *   9. Schedule end-of-life event 600 frames (FUN_80020890).
 *  10. Set smoke flag on parent.
 *  11. Play smoke SFX via the audio engine (FUN_8004483c is stubbed).
 *
 * HIGH confidence (direct Ghidra port).  LAB_8002c210 is a panic stub
 * pending pass-3 decomp of the smoke tick.
 */
#include <stdint.h>

extern void    *FUN_8001d470(uint32_t size);
extern void     FUN_8001d544(uint32_t *parent, uint32_t *child);
extern void     FUN_80020744(uint32_t *obj);
extern void     FUN_80020890(uint32_t *obj, int frames);

extern int      FUN_8004410c(void);
extern void     FUN_8004483c(int, uint32_t, int, uint32_t *);

extern uint32_t DAT_800737d8;
extern uint32_t uRam000005f8;

extern int LAB_8002c210(int obj, int event, int param3);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

void FUN_8002c3ac(uint32_t *param_1)
{
    uint32_t  uVar1, uVar3;
    uint32_t *puVar2;

    if (((*param_1 & 0x4000000u) == 0) &&
        (*(int16_t *)((uint8_t *)param_1 + 0x11e) == 0))
    {
        puVar2 = (uint32_t *)FUN_8001d470(0x9c);
        Object_SetCallbackPsxSlot(puVar2, (uintptr_t)&LAB_8002c210);
        puVar2[0x15] = 0x10000;
        puVar2[0xb]  = 0;
        puVar2[10]   = 0;
        puVar2[9]    = 0;
        *puVar2 |= 0xa4u;

        uVar1 = DAT_800737d8;
        *(uint16_t *)((uint8_t *)puVar2 + 0x96) = 0x22;
        *(uint16_t *)((uint8_t *)puVar2 + 0x82) = 4;
        *(uint16_t *)(puVar2 + 3) = 2;
        puVar2[0x21] = 0x200;
        puVar2[0x26] = uVar1;
        puVar2[0x22] = 0xfffffa00;
        puVar2[0x23] = 0;

        FUN_8001d544(param_1, puVar2);
        FUN_80020744(puVar2);
        FUN_80020890(puVar2, 600);

        *param_1 |= 0x4000000u;

        uVar3 = (uint32_t)FUN_8004410c();
        FUN_8004483c((int)uVar3, uRam000005f8, 0x3c, param_1 + 9);
    }
}
