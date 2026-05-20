/* object_state.c -- object state-change broadcast helpers.
 *
 * Source: SLUS_005.10
 *   FUN_80024718 -- Object_SetState
 *   FUN_80021888 -- Object_BroadcastEventWorld
 *
 * Object_SetState routes through FUN_80024520 (still in pass-3 backlog)
 * which is the 6-arg state-transition primitive. The trailing args
 * (uRam000006ec, 0, 0, 0x800) are: { effect pool, flag1=0, flag2=0,
 * effect mask 0x800 (= "destroyed -> spawn debris cloud") }.
 *
 * Object_BroadcastEventWorld is the world-or-tree event broadcaster
 * with arg2=0 baked in (used by the simpler event dispatchers in the
 * level DLLs).
 *
 * HIGH.
 */
#include <stdint.h>

extern void Object_TransitionState(int obj, int state, uintptr_t effectPool,
                                   int flag1, int flag2, int effectMask);  /* FUN_80024520 */
extern int  Object_DispatchList(void *listHead, uint32_t event, uint32_t arg);  /* FUN_80020000 */
extern void Object_DispatchTree2(uintptr_t treeRoot, uint32_t event, uint32_t arg);  /* FUN_8002111c */

extern uintptr_t uRam000006ec;
extern uintptr_t uRam000006fc;
extern uint8_t   DAT_80065a18[];

void Object_SetState(int obj, int newState)
{
    Object_TransitionState(obj, newState, uRam000006ec, 0, 0, 0x800);
}

void Object_BroadcastEventWorld(uint32_t event)
{
    if (Object_DispatchList(DAT_80065a18, event, 0) == 0) {
        Object_DispatchTree2(uRam000006fc, event, 0);
    }
}
