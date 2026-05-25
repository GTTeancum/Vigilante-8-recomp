/* object_spawn_effect.c -- Spawn a smoke/spark particle-effect object.
 *
 * Source: SLUS_005.10
 *   FUN_8003fea8  Object_SpawnEffect(xyz, color)  (~15 instr)
 *
 * Allocates a type-0xa0 effect object if the game mode (iRam00000010) is 0
 * AND the position is within view frustum (FUN_8001db54).  If spawned, the
 * object gets:
 *   [0]    = 0xa0          (object type)
 *   [0xb]  = 0x80          (effect subtype / display-list index)
 *   [9]    = color         (packed RGBA color word, passed in param_2)
 *   [0x19] = LAB_8003ee88  (per-frame tick for the smoke/spark animation)
 * then registered in all active-object lists via FUN_800202f4.
 *
 * Called by: vehicle_collision.c (spark on impact), boulder_chain.c,
 *            bunker_door.c, missile_chain.c, windmill.c.
 *
 * HIGH: direct port from Ghidra pseudoC; control flow matches MIPS exactly.
 */
#include <stdint.h>
#include <stddef.h>

extern int32_t   iRam00000010;           /* game mode: 0 = in-game */
extern int       FUN_8001db54(int32_t *pos, int radius);  /* frustum cull check */
extern uint32_t *FUN_8001d470(uint32_t size);             /* Object_AllocSmall */
extern void      FUN_800202f4(uint32_t *obj);             /* Object_RegisterInAllLists */
extern void      LAB_8003ee88(void);     /* effect-object tick callback */
extern void      Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/*
 * HIGH: spawn a type-0xa0 effect at world position xyz with colour word color.
 * Returns pointer to the new object, or NULL if out of view or mode != 0.
 */
uint32_t *FUN_8003fea8(int32_t *param_1, uint32_t param_2)
{
    int       iVar1;
    uint32_t *puVar2;

    if ((iRam00000010 == 0) &&
        (iVar1 = FUN_8001db54(param_1, 0), iVar1 != 0)) {
        puVar2 = FUN_8001d470(0x80);
        puVar2[0]    = 0xa0u;
        puVar2[0xb]  = 0x80u;
        puVar2[9]    = param_2;
        Object_SetCallbackPsxSlot(puVar2, (uintptr_t)&LAB_8003ee88);
        FUN_800202f4(puVar2);
    } else {
        puVar2 = (uint32_t *)NULL;
    }
    return puVar2;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8003fea8  (from analysis/SLUS_005.10/decomp/8003fea8.c) ---
 * 8003fea8: lw   v0, iRam00000010
 * 8003feac: bne  v0, 0, LAB_8003fedc   ; if mode != 0, skip
 * 8003feb4: jal  FUN_8001db54(a0, 0)
 * 8003febc: beq  v0, 0, LAB_8003fedc   ; if not in view, skip
 * 8003fec0: jal  FUN_8001d470(0x80)
 * 8003fec8: li   v1, 0xa0; sw v1, 0(v0)
 * 8003fecc: li   v1, 0x80; sw v1, 0x2c(v0)
 * 8003fed0: sw   a1, 0x24(v0)
 * 8003fed4: li   v1, LAB_8003ee88; sw v1, 0x64(v0)
 * 8003fed8: jal  FUN_800202f4(v0)
 * 8003fedc: jr   ra ; return puVar2 (NULL if skipped)
 */

#endif  /* GHIDRA REF */
