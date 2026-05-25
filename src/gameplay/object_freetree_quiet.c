/* object_freetree_quiet.c -- recursive free of a sibling/child object tree
 *                            WITHOUT unregistering from scene lists.
 *
 * Source: SLUS_005.10  FUN_8001af48.
 *
 * AUDIT NOTE: Ghidra mislabeled this as "Audio_StopAllSfx" because the
 * level-teardown call site passes the SFX bank-list root (uRam000007d8).
 * The function is generic: walks +0x34 (next-sibling) iteratively, frees
 * the bone-bank slot at +0x30 of each node via FUN_8001bddc, recurses
 * into +0x38 (first-child), and heap-frees the node itself.
 *
 * Twin of FUN_800204dc (Object_HeapFreeRecursive) but skips the scene
 * unregister step -- safe when the tree has never been registered (e.g.
 * the orphan SFX bank list that lives outside the scene graph).
 *
 * HIGH confidence (direct Ghidra port).
 */
#include <stdint.h>

extern void Heap_Free(void *p);                /* FUN_80045088 */
extern void FUN_8001bddc(int boneBank);        /* BoneBank_Free */

void FUN_8001af48(int param_1)
{
    int iVar1;

    while (param_1 != 0) {
        FUN_8001bddc(*(int *)(uintptr_t)(param_1 + 0x30));
        FUN_8001af48(*(int *)(uintptr_t)(param_1 + 0x38));
        iVar1 = *(int *)(uintptr_t)(param_1 + 0x34);
        Heap_Free((void *)(uintptr_t)param_1);
        param_1 = iVar1;
    }
}

/* Public alias (level_teardown.c, vallyfrm/misc.c). */
void Audio_StopAllSfx(void *bank) { FUN_8001af48((int)(intptr_t)bank); }
