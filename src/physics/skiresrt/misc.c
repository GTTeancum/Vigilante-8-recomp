/* misc.c -- Ski Resort small object handlers.
 *
 * Source: SKIRESRT.DLL
 *   FUN_80101424 -- SR_InstallDefaultTick: on mode-1 (init), bind the
 *                   per-tick callback to the generic SUB_800223dc
 *                   (the default "respond to children" behaviour).
 *   FUN_80101390 -- SR_InsertSortedByY: insert an object into a
 *                   Y-sorted linked list (so the lift station can
 *                   process objects from lowest to highest).
 *   FUN_80102094 -- SR_SnowMachineHit: collision-based event broadcast
 *                   that flips an event id between 0x200 and 0x201
 *                   based on whether the object is "above" the
 *                   level's midpoint -- this is the snowfall trigger.
 *
 * HIGH-MED across all three.
 */
#include <stdint.h>

extern void SUB_800223dc(void);
extern int  Collision_AgainstTerrain(int obj, int impact);
extern int  Collision_Circular(int obj, int impact);
extern int  Effects_QueueSnow(int obj);    /* FUN_8003fc94 */
extern void *Object_Pool_Alloc2(void);     /* func_0x80022c54 */
extern int   Object_BroadcastEvent2(int eventId, int eventArg);  /* func_0x80021924 */

extern int8_t  *_DAT_800659fc;
extern uint32_t _DAT_80065b34, _DAT_80065b38;
extern int32_t  DAT_80102154;

void SR_InstallDefaultTick(int obj, int mode)
{
    if (mode == 1) *(void **)(obj + 100) = SUB_800223dc;
}

void SR_InsertSortedByY(int obj)
{
    uint32_t *newNode = Object_Pool_Alloc2();
    int **list = *(int ***)(_DAT_800659fc + 0x80);
    int *tail = list;
    int *cur  = (int *)list[0];
    if (cur != NULL) {
        int *prev;
        do {
            prev = cur;
            if (*(int *)(obj + 0x50) <= *(int *)(tail[2] + 0x50)) break;
            cur = (int *)*prev;
            tail = prev;
        } while ((int *)*prev != NULL);
    }
    int *next = (int *)tail[1];
    *next = (int)(uintptr_t)newNode;
    tail[1] = (int)(uintptr_t)newNode;
    newNode[0] = (uintptr_t)tail;
    newNode[1] = (uintptr_t)next;
}

uint32_t SR_SnowMachineHit(int obj, uint32_t mode, void *impact)
{
    if (mode == 3 || mode == 8) Collision_AgainstTerrain(obj, (int)(intptr_t)impact);
    int hit = Collision_Circular(obj, (int)(intptr_t)impact);
    if (hit == 0) return 0;
    if (Effects_QueueSnow(obj) != 0) return 0;
    DAT_80102154++;
    if ((DAT_80102154 & 1) == 0) return 0;
    int evt = ((_DAT_80065b34 + _DAT_80065b38) >> 1u < (uint32_t)*(int32_t *)(obj + 0x48))
              ? 0x201 : 0x200;
    return (uint32_t)Object_BroadcastEvent2(9, evt);
}
