/* train_init.c -- Wild West train initialization (path attach).
 *
 * Source: WILDWEST.DLL  FUN_80100688.
 *
 * Run once per spawn. Computes the nearest path waypoint to the
 * train's current position via Path_NearestWaypoint (FUN_80042cdc),
 * stores it at obj+0x29, sets forward direction (+0x2b = 1), then
 * samples the path at that waypoint to compute the initial path-t
 * (obj+0x2c) and initial velocity 0xee6 at obj+0x2a. Zeros out
 * various per-instance counters and sets status flags 0x180.
 *
 * Used by all train cars at level start.
 *
 * MED.
 */
#include <stdint.h>

extern int  Path_NearestWaypoint(uint32_t *posXyz, uint32_t excludeId);  /* FUN_80042cdc */
extern int  Path_Sample(int waypoint, int t, uint32_t *posInOut, int32_t *deltaOut);  /* FUN_80042724 */

void WW_TrainInitOnSpawn(uint32_t *self)
{
    int waypoint = Path_NearestWaypoint(self + 0x12, 0xffffffffu);
    self[0x29] = (uint32_t)waypoint;
    self[0x2b] = 1;

    uint8_t scratch[0x20];
    int initT = Path_Sample(waypoint, (int)*(int *)scratch, self + 0x12, (int32_t *)scratch);
    self[0x2c] = (uint32_t)(initT << 16);
    self[0x2a] = 0xee6;
    self[0x1e] = 0;
    self[0x1d] = 0;
    self[0x24] = 0;
    self[0] |= 0x180u;
    self[0x25] = 0;
    self[0x26] = 0;
    *(uint16_t *)(self + 0x27) = 0x10;
    *(uint16_t *)((uint8_t *)self + 0x9e) = 0x20;
    *(uint16_t *)(self + 0x28) = 0x40;
}
