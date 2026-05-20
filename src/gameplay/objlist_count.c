/* objlist_count.c -- count list length.
 *
 * Source: SLUS_005.10  FUN_800166dc.
 *
 * Walks a singly-linked list (next-pointer at +0) starting at
 * `*headPtr`, returns the count.
 *
 * HIGH.
 */
#include <stdint.h>

int ObjList_Length(int **headPtr)
{
    int count = 0;
    int *node = (int *)*headPtr;
    while (node != NULL) {
        count++;
        node = (int *)*node;
    }
    return count;
}
