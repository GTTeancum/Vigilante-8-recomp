/* object_list_extra.c -- more object list utilities.
 *
 * Source: SLUS_005.10
 *   FUN_80020000  -- ObjList_FindBySpawnIdValue (returns payload, not node)
 *   FUN_8002002c  -- ObjList_TickListWith3Args
 *   FUN_80020120  -- ObjList_CountWithFlag
 *   FUN_80020190  -- ObjList_NthWithFlag
 *   FUN_800202f4  -- Object_RegisterInScene
 *
 * The lists at DAT_80065a18 (world), DAT_80065a60 (back-buf pending),
 * DAT_80065a80 (active draw chain) all share the 12-byte node layout:
 *   +0 next, +4 back, +8 payload.
 *
 * Object_RegisterInScene fast-clear-then-insert into up to three of
 * those chains based on the object's flag bits (4 -> draw chain, 0x80
 * -> back-buf, always -> world).
 *
 * "WithFlag" variants filter by: spawnId > 0x1f AND obj.flags has the
 * requested bit AND obj.flags has none of (0x8002 = invulnerable +
 * dead). The Nth helper additionally counts down to pick the Nth.
 *
 * HIGH.
 */
#include <stdint.h>
#include <stddef.h>

extern int *ObjList_FindBySpawnId(int **listHead, int spawnId, int excludePayload);   /* FUN_8001ff58 */
extern void ObjList_FastInsert(void *listHead, uint32_t *obj);                       /* FUN_8001fe50 */
extern uint8_t DAT_80065a18[];
extern uint8_t DAT_80065a60[];
extern uint8_t DAT_80065a80[];

int ObjList_FindBySpawnIdValue(int **listHead, int spawnId, int exclude)
{
    int *node = ObjList_FindBySpawnId(listHead, spawnId, exclude);
    return (node != NULL) ? node[2] : 0;
}

void ObjList_TickListWith3Args(int **listHead, uint32_t arg2, uint32_t arg3)
{
    int *node = (int *)listHead[0];
    int **prev = listHead;
    while (node != NULL) {
        typedef int (*TickFn)(int, uint32_t, uint32_t);
        TickFn fn = *(TickFn *)((uintptr_t)prev[2] + 100);
        if (fn != NULL) fn((int)(uintptr_t)prev[2], arg2, arg3);
        prev = (int **)node;
        node = (int *)*node;
    }
}

int ObjList_CountWithFlag(int **listHead, uint32_t flag)
{
    int *node = (int *)listHead[0];
    int **prev = listHead;
    int count = 0;
    while (node != NULL) {
        uint32_t *payload = (uint32_t *)(uintptr_t)prev[2];
        if (*(int16_t *)((uintptr_t)payload + 6) > 0x1f
            && (payload[0] & flag) != 0
            && (payload[0] & 0x8002u) == 0)
        {
            count++;
        }
        prev = (int **)node;
        node = (int *)*node;
    }
    return count;
}

uint32_t *ObjList_NthWithFlag(int **listHead, uint32_t flag, int n)
{
    int *node = (int *)listHead[0];
    int **prev = listHead;
    while (node != NULL) {
        uint32_t *payload = (uint32_t *)(uintptr_t)prev[2];
        if (*(int16_t *)((uintptr_t)payload + 6) > 0x1f
            && (payload[0] & flag) != 0
            && (payload[0] & 0x8002u) == 0)
        {
            if (--n == -1) return payload;
        }
        prev = (int **)node;
        node = (int *)*node;
    }
    return NULL;
}

void Object_RegisterInScene(uint32_t *obj)
{
    if ((obj[0] & 4u)    != 0) ObjList_FastInsert(DAT_80065a80, obj);
    if ((obj[0] & 0x80u) != 0) ObjList_FastInsert(DAT_80065a60, obj);
    ObjList_FastInsert(DAT_80065a18, obj);
}
