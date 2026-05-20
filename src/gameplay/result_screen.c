/* result_screen.c -- post-match result screen text builder.
 *
 * Source: SLUS_005.10  FUN_8001392c.
 *
 * Composes the YOU WIN / YOU LOSE / PLAYER N WINS / PLAYER TEAM WINS
 * text for the result screen. Switches on g_matchMode (uRam00000015):
 *   case 0 (quest)     -> "YOU LOSE!" + (if won) cheat unlock code
 *   case 1, 2 (versus) -> "YOU WIN!" / "YOU LOSE!" + scores
 *   case 3 (split)     -> "PLAYER N WINS!" with N derived from
 *                         which player's health hit zero
 *   case 4 (team)      -> "PLAYER TEAM WINS!" + sum P1+P2 weights/skills
 *
 * Calls MatchScore_AppendLine for the actual score formatting, then
 * concatenates any tail format (DAT_800101c8, 80010238, etc.) before
 * passing the buffer to Font_LayoutWord + Layout/render.
 *
 * Plus: in case 4, sums the two player's weights and skills into P1.
 *
 * MED -- preserves the original case dispatch. Bit-exact: the
 * fprintf format strings come from in-binary constants; the layout
 * call at the tail is renderer-adjacent (out of scope for 1:1).
 *
 * Pass 3 should: rename the unlock-cheat-code-emit branch, document
 * DAT_80065c08 (the 14-byte unlock codes), and resolve DAT_800101c8 /
 * 80010238 format strings.
 */
#include <stdint.h>

extern int  sprintf(char *dst, const char *fmt, ...);
extern int  Util_StrLen(const char *s);
extern char *strcat(char *dst, const char *src);
extern void MatchScore_AppendLine(char *resultStr, int vehicle);
extern uint32_t Font_LayoutWord(int font, char *s, int x, int y);
extern void Font_DispatchCallback(uint32_t handle);

extern int8_t  uRam00000015;     /* g_matchMode */
extern int32_t iRam000007d0;     /* g_player1Vehicle */
extern int32_t iRam000007d4;     /* g_player2Vehicle */
extern int32_t iRam00000024;     /* p1 alive flag */
extern int32_t iRam00000620;     /* cheats earned */
extern int8_t  bRam0000061c;     /* unlock-cheat slot */
extern int8_t  cRam0000067c;     /* current quest */
extern uint8_t  uRam00000674, uRam00000678;
extern const char **PTR_s_Chassey_Blue_800567ec;
extern const char DAT_800100fc[], DAT_80010110[], DAT_80010124[];
extern const char DAT_800655c8[], DAT_8001017c[], DAT_800101c8[];
extern const char DAT_80010238[], DAT_80010274[];
extern const char DAT_80065464[], DAT_80065468[];
extern uint8_t DAT_80065c08[];   /* 14-byte unlock cheat codes */
extern int32_t DAT_80065978[2];   /* P1/P2 win counts */

void ResultScreen_Build(int fontHandle)
{
    char buf[256];
    int vehicle = iRam000007d0;
    char *tail = NULL;
    const char *tailFmt = NULL;

    switch (uRam00000015) {
    case 0:
        if (iRam00000024 == 0) {
            sprintf(buf, "\x01`  \a`\x05YOU LOSE!\n");
        } else {
            sprintf(buf, DAT_800100fc);
            MatchScore_AppendLine(buf, vehicle);
            if (iRam00000620 != 0 && bRam0000061c < 0x20) {
                int n = Util_StrLen(buf);
                sprintf(buf + n, DAT_80010110,
                        (&PTR_s_Chassey_Blue_800567ec)[(uint8_t)bRam0000061c * 2]);
            }
            strcat(buf, DAT_80010124);
            /* Append unlock code (14 bytes, each + 'A' to convert to ASCII). */
            int n = Util_StrLen(buf);
            char *p = buf + n;
            const uint8_t *src = DAT_80065c08 + iRam00000620 * 0xe;
            for (int i = 0; i < 0xe; i++) p[i] = (char)(src[i] + 'A');
            p[0xe] = '\0';
        }
        strcat(buf, DAT_800655c8);
        goto draw;
    case 1:
    case 2: {
        int winnerIs1P = (iRam00000024 != 0);
        DAT_80065978[!winnerIs1P]++;
        if (winnerIs1P) {
            sprintf(buf, "\x01 ` \a`\x05YOU WIN!\n");
            MatchScore_AppendLine(buf, vehicle);
        } else {
            sprintf(buf, "\x01`  \a`\x05YOU LOSE!\n");
        }
        tail   = buf + Util_StrLen(buf);
        tailFmt = DAT_8001017c;
        break;
    }
    case 3: {
        /* PLAYER 1 or 2 wins */
        cRam0000067c++;
        char winner = '1';
        if (*(int16_t *)(iRam000007d0 + 0xc) == 0) {
            winner = '2';
        }
        sprintf(buf, "\x01 ` \a`\x05PLAYER %c WINS!\n", winner);
        int liveV = iRam000007d0;
        if (*(int16_t *)(iRam000007d0 + 0xc) == 0) liveV = iRam000007d4;
        MatchScore_AppendLine(buf, liveV);
        tail   = buf + Util_StrLen(buf);
        tailFmt = DAT_800101c8;
        break;
    }
    case 4: {
        int teamWon = (iRam00000024 != 0);
        DAT_80065978[!teamWon]++;
        if (!teamWon) {
            sprintf(buf, "\x01`  \a`\x05PLAYER TEAM LOSES!\n");
        } else {
            sprintf(buf, "\x01 ` \a`\x05PLAYER TEAM WINS!\n");
            /* Sum P2 weight/skill into P1 for the team score. */
            *(int8_t *)(iRam000007d0 + 0xba) += *(int8_t *)(iRam000007d4 + 0xba);
            *(int8_t *)(iRam000007d0 + 0xbb) += *(int8_t *)(iRam000007d4 + 0xbb);
            MatchScore_AppendLine(buf, iRam000007d0);
        }
        tail   = buf + Util_StrLen(buf);
        tailFmt = DAT_80010238;
        break;
    }
    default:
        goto submitDraw;
    }
    sprintf(tail, tailFmt, (uint32_t)uRam00000674, (uint32_t)uRam00000678);
submitDraw:
    strcat(buf, DAT_80010274);
draw: {
        uint32_t layout = Font_LayoutWord(fontHandle, buf, 0x40, 0x40);
        Font_DispatchCallback(layout);
    }
}
