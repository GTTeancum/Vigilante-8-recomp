/* match_config.c -- load the 12-byte match-config record into globals.
 *
 * Source: SLUS_005.10  FUN_80011c58.
 *
 * The match config record is a 12-byte structure produced by the shell
 * UI (during character / level select) and consumed by the main loop
 * just before a match starts. Recovered layout:
 *
 *   u8  matchMode         -> g_matchMode (@ uRam00000015)
 *   u8  flags             -> uRam000005f4
 *   u8  playerSlot[6]     -> g_playerSlotIdx (DAT_80065674..)
 *   u8  flags2[4]         -> DAT_8006567c..7f
 *
 * The byte-by-byte copy with the indirect addressing is the canonical
 * PSY-Q open-coded memcpy of a packed structure -- a compiler artifact
 * not a deliberate design.
 *
 * HIGH confidence on layout (offsets verified across multiple readers).
 */
#include <stdint.h>

extern uint8_t uRam00000015;     /* g_matchMode */
extern uint8_t uRam000005f4;
extern uint8_t DAT_80065674[];   /* base of g_playerSlotIdx[6] + g_matchFlags2[4] */

#define g_playerSlotIdx  (DAT_80065674)
#define g_matchFlags2    (DAT_80065674 + 8)

void Match_LoadConfig(const uint8_t *cfg)
{
    uRam00000015  = cfg[0];          /* match mode */
    uRam000005f4  = cfg[1];          /* flag byte */
    for (int i = 0; i < 6; i++) g_playerSlotIdx[i] = cfg[2 + i];
    for (int i = 0; i < 4; i++) g_matchFlags2[i]   = cfg[8 + i];
}
