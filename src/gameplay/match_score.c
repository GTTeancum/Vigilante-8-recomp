/* match_score.c -- compute and format the post-match score line.
 *
 * Source: SLUS_005.10  FUN_800136c4.
 *
 * Computes a final score for the just-finished match and sprintf's
 * it onto the end of the result-screen string.
 *
 * Score formula:
 *   timeFactor = min(g_matchElapsedTicks, vehicle.weight * 300
 *                                          + g_quest * 0xa8c
 *                                          + vehicle.skill * 600)
 *   score = (g_matchMode == 0)  // quest mode
 *           ? (timeFactor * 0x55) / g_matchElapsedTicks   // /128 base
 *             + (cheats_enabled ? 0xf : 0)
 *           : (timeFactor * 100) / g_matchElapsedTicks    // %  base
 *
 * Then appends the formatted line:
 *   "<minutes>:<seconds>  <vehicle.weight>:<vehicle.skill>  <quest>"
 * via sprintf with format DAT_80010084.
 *
 * Vehicle struct fields used:
 *   +0xba  u8  weight
 *   +0xbb  u8  skill
 *
 * Promotes 2 more Vehicle fields. Pass 3 should add these to structs.h.
 *
 * MED-HIGH confidence.
 */
#include <stdint.h>

extern int  Util_StrLen(const char *s);   /* FUN_80052544 */
extern int  sprintf(char *dst, const char *fmt, ...);
extern char DAT_80010084[];      /* "%d:%02d  %d:%d  %d" format string */
extern int8_t  cRam00000015;     /* g_matchMode */
extern int8_t  cRam0000067c;     /* g_currentQuest */
extern int32_t iRam0000000c;     /* g_matchElapsedTicks */
extern int32_t iRam00000620;     /* g_cheatsActive flag */

void MatchScore_AppendLine(char *resultStr, int vehicle)
{
    uint8_t weight = *(uint8_t *)(vehicle + 0xba);
    uint8_t skill  = *(uint8_t *)(vehicle + 0xbb);
    int     elapsed = iRam0000000c;
    int     cap     = (int)weight * 300
                    + (int)cRam0000067c * 0xa8c
                    + (int)skill  * 600;
    int     timeFactor = (cap < elapsed) ? cap : elapsed;

    int score;
    if (cRam00000015 == 0) {
        score = (timeFactor * 0x55) / elapsed;
        if (iRam00000620 != 0) score += 0xf;
    } else {
        score = (timeFactor * 100) / elapsed;
    }
    (void)score;

    int len = Util_StrLen(resultStr);
    int minutes = elapsed / 0xe10;
    int seconds = (elapsed / 0x3c) - minutes * 0x3c;
    sprintf(resultStr + len, DAT_80010084,
            minutes, seconds, (uint32_t)weight, (uint32_t)skill, (int)cRam0000067c);
    /* The original continues appending more fields based on g_matchMode --
     * pass 3 finishes the tail formatting. */
}
