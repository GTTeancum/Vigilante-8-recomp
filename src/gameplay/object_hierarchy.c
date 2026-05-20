/* object_hierarchy.c -- parent/child hierarchy walkers.
 *
 * Source: SLUS_005.10
 *   FUN_8001d564  -- Object_DetachFromParent
 *   FUN_8001d5a0  -- Object_Parent (walks `parent_link` up to the
 *                    first ancestor whose first-child slot is self)
 *
 * Layout in each Object:
 *   +0x34  next-sibling pointer (used while walking child list)
 *   +0x38  first-child pointer
 *   +0x3c  parent pointer
 *
 * Object_DetachFromParent unlinks `self` from its parent's child list
 * (handles both "first child" and "later sibling" cases).
 *
 * Object_Parent walks `self.parent`. The loop continues while the
 * parent's first-child slot is NOT self, walking up the chain --
 * unusual but matches the binary. (Effectively returns the *grand*
 * parent for chained sibling lookups.) Pass 3 should re-examine.
 *
 * HIGH.
 */
#include <stdint.h>

void Object_DetachFromParent(int self)
{
    int parent = *(int *)(self + 0x3c);
    int sibling = *(int *)(self + 0x34);
    if (*(int *)(parent + 0x38) == self) {
        *(int *)(parent + 0x38) = sibling;
    } else {
        *(int *)(parent + 0x34) = sibling;
    }
    if (sibling != 0) {
        *(int *)(sibling + 0x3c) = parent;
    }
    *(int *)(self + 0x34) = 0;
}

int Object_Parent(int self)
{
    int parent = *(int *)(self + 0x3c);
    while (parent != 0 && *(int *)(parent + 0x38) != self) {
        self   = *(int *)(self + 0x3c);
        parent = *(int *)(self + 0x3c);
    }
    return parent;
}
