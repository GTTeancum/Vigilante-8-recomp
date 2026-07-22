/* state_capture.c -- native-game binding for V8StateSnapshot.
 *
 * Kept separate from state_snapshot.c so the standalone comparator can link
 * without definitions for the live game globals. The native gameplay target
 * links this file when its frame loop is available.
 */
#include <stdint.h>
#include <string.h>
#include "state_snapshot.h"

extern uint8_t  g_splitScreenMode;
extern int32_t  g_drawBufIndex;
extern int32_t  g_dispBufIndex;
extern void    *g_player1Vehicle;
extern void    *g_player2Vehicle;
extern uint32_t g_matchConfigFlags;
extern int32_t  g_optionFlag_c28;
extern int32_t  g_optionFlag_c40;
extern uint32_t g_vsyncCounter;
extern int32_t  g_rngSeed;
extern uint8_t  g_rngCounter;
extern uint8_t  g_matchMode;

int V8_SnapshotCapture(V8StateSnapshot *out)
{
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    out->magic            = V8_SS_MAGIC;
    out->version          = V8_SS_VERSION;
    out->frameCounter     = g_vsyncCounter;
    out->rngSeed          = g_rngSeed;
    out->rngCounter       = g_rngCounter;
    out->matchMode        = g_matchMode;
    out->splitScreenMode  = (uint8_t)g_splitScreenMode;
    out->drawBufIndex     = (uint8_t)g_drawBufIndex;
    out->dispBufIndex     = (uint8_t)g_dispBufIndex;
    out->matchConfigFlags = g_matchConfigFlags;
    out->optionFlag_c28   = g_optionFlag_c28;
    out->optionFlag_c40   = g_optionFlag_c40;

    if (g_player1Vehicle != NULL) {
        out->p1Present = 1;
        memcpy(&out->vehicles[0], g_player1Vehicle, sizeof(Vehicle));
    }
    if (g_player2Vehicle != NULL) {
        out->p2Present = 1;
        memcpy(&out->vehicles[1], g_player2Vehicle, sizeof(Vehicle));
    }
    return 0;
}
