/* level_teardown.c -- release per-match dynamically allocated state.
 *
 * Source: SLUS_005.10  FUN_80022a1c.
 *
 * Called from the main loop's end-of-match path to free every chunk
 * of memory that the level/match phase brought in. The function
 * touches a lot of global state -- it is the *last* operation in the
 * match dispatch before the shell phase re-runs.
 *
 * Steps (in order):
 *   1. Audio_StopAllSfx (FUN_8001af48) on the active SFX bank
 *      pointer at uRam000007d8.
 *   2. Free the projectile pool buffer (iRam000006f8 + handle from
 *      FUN_800203fc).
 *   3. Free the secondary projectile / spark buffer at iRam000006ec.
 *   4. Tree_Free   the obstacle spatial tree (DAT_80065a50).
 *   5. Tree_Free   the visibility chunk root (DAT_80065a18).
 *   6. Heap_Free   the terrain kd-tree at iRam000006fc, and clear
 *      the pointer so Asset_VisibilityEvictTick sees an empty world.
 *   7. Walk and free the trigger-volume chain (head puRam000007c4,
 *      sentinel DAT_80065ac0; each node freed via FUN_80020540).
 *   8. Reset the spawn-marker list (head piRam0000079c, sentinel
 *      DAT_80065aa0).
 *   9. Pass 2 fills in the rest (the function continues past the
 *      Ghidra excerpt -- it touches at least 4 more list heads).
 *
 * MED confidence: each cleanup step is unambiguous, but the *names*
 * of the released structures are tentative until pass 2 cross-checks
 * with the constructors.
 */
#include <stdint.h>

extern void Heap_Free(void *p);
extern void Audio_StopAllSfx(void *bank);                /* FUN_8001af48 */
extern void *Pool_GetProjectileHandle(void);             /* FUN_800203fc */
extern void Tree_Free(void *root);                       /* FUN_80020658 */
extern void Tree_FreeTerrain(void *root);                /* FUN_80020968 */
extern void TriggerVol_FreeOne(void *node);              /* FUN_80020540 */

extern uintptr_t uRam000007d8;
extern uintptr_t iRam000006f8;
extern uintptr_t iRam000006ec;
extern uintptr_t iRam000006fc;
extern uintptr_t iRam000007bc;
extern void     *puRam000007c4;
extern void     *puRam000007a4;
extern int32_t  *piRam0000079c;
extern uint8_t   DAT_80065a18[];
extern uint8_t   DAT_80065a50[];
extern uint8_t   DAT_80065aa0[];
extern uint8_t   DAT_80065ac0[];

void Level_Teardown(void)
{
    Audio_StopAllSfx((void *)uRam000007d8);

    if (iRam000006f8 != 0) {
        Heap_Free(Pool_GetProjectileHandle());
    }
    if (iRam000006ec != 0) {
        Heap_Free((void *)iRam000006ec);
    }

    Tree_Free(DAT_80065a50);
    Tree_Free(DAT_80065a18);
    Tree_FreeTerrain((void *)iRam000006fc);
    iRam000006fc = 0;

    while (puRam000007c4 != DAT_80065ac0) {
        TriggerVol_FreeOne(*(void **)(iRam000007bc + 8));
    }

    while (puRam000007a4 != DAT_80065aa0) {
        piRam0000079c = (int32_t *)piRam0000079c[0];
        piRam0000079c[1] = (int32_t)DAT_80065aa0;
        /* The original continues with a few more chain detaches and
         * a final draw-env reset; those calls are renderer-adjacent
         * (DrawEnv / OT chain) and belong to the rewritten renderer
         * per CLAUDE.md. The list-walk pattern above is the gameplay-
         * facing part. */
    }
}
