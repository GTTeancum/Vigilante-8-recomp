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

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001392c  (from analysis/SLUS_005.10/decomp/8001392c.c) --- */
// addr: 0x8001392c  name: FUN_8001392c

void FUN_8001392c(undefined4 param_1)

{
  bool bVar1;
  char cVar2;
  int iVar3;
  undefined4 uVar4;
  char *pcVar5;
  char *pcVar6;
  int iVar7;
  char acStack_110 [256];
  
  iVar3 = iRam000007d0;
  switch(uRam00000015) {
  case 0:
    if (iRam00000024 == 0) {
      sprintf(acStack_110,"\x01`  \a`\x05YOU LOSE!\n");
    }
    else {
      sprintf(acStack_110,&DAT_800100fc);
      FUN_800136c4(acStack_110,iRam000007d0);
      if ((iRam00000620 != 0) && (bRam0000061c < 0x20)) {
        iVar3 = FUN_80052544(acStack_110);
        sprintf(acStack_110 + iVar3,&DAT_80010110,
                (&PTR_s_Chassey_Blue_800567ec)[(uint)bRam0000061c * 2]);
      }
      strcat(acStack_110,&DAT_80010124);
      iVar3 = FUN_80052544(acStack_110);
      iVar7 = 0;
      pcVar6 = acStack_110 + iVar3;
      pcVar5 = &DAT_80065c08 + iRam00000620 * 0xe;
      do {
        cVar2 = *pcVar5;
        pcVar5 = pcVar5 + 1;
        iVar7 = iVar7 + 1;
        *pcVar6 = cVar2 + 'A';
        pcVar6 = pcVar6 + 1;
      } while (iVar7 < 0xe);
      *pcVar6 = '\0';
    }
    strcat(acStack_110,&DAT_800655c8);
    goto LAB_80013c80;
  case 1:
  case 2:
    bVar1 = iRam00000024 == 0;
    *(int *)(&DAT_80065978 + (uint)(iRam00000024 == 0) * 4) =
         *(int *)(&DAT_80065978 + (uint)(iRam00000024 == 0) * 4) + 1;
    if (bVar1) {
      sprintf(acStack_110,"\x01`  \a`\x05YOU LOSE!\n");
    }
    else {
      sprintf(acStack_110,"\x01 ` \a`\x05YOU WIN!\n");
      FUN_800136c4(acStack_110,iRam000007d0);
    }
    iVar3 = FUN_80052544(acStack_110);
    pcVar5 = acStack_110 + iVar3;
    pcVar6 = &DAT_8001017c;
    break;
  case 3:
    *(int *)(&DAT_80065978 + (uint)(*(short *)(iRam000007d0 + 0xc) == 0) * 4) =
         *(int *)(&DAT_80065978 + (uint)(*(short *)(iRam000007d0 + 0xc) == 0) * 4) + 1;
    uVar4 = 0x31;
    cRam0000067c = cRam0000067c + '\x01';
    if (*(short *)(iVar3 + 0xc) == 0) {
      uVar4 = 0x32;
    }
    sprintf(acStack_110,"\x01 ` \a`\x05PLAYER %c WINS!\n",uVar4);
    iVar3 = iRam000007d0;
    if (*(short *)(iRam000007d0 + 0xc) == 0) {
      iVar3 = iRam000007d4;
    }
    FUN_800136c4(acStack_110,iVar3);
    iVar3 = FUN_80052544(acStack_110);
    pcVar5 = acStack_110 + iVar3;
    pcVar6 = &DAT_800101c8;
    break;
  case 4:
    bVar1 = iRam00000024 == 0;
    *(int *)(&DAT_80065978 + (uint)(iRam00000024 == 0) * 4) =
         *(int *)(&DAT_80065978 + (uint)(iRam00000024 == 0) * 4) + 1;
    if (bVar1) {
      sprintf(acStack_110,"\x01`  \a`\x05PLAYER TEAM LOSES!\n");
    }
    else {
      sprintf(acStack_110,"\x01 ` \a`\x05PLAYER TEAM WINS!\n");
      *(char *)(iRam000007d0 + 0xba) =
           *(char *)(iRam000007d0 + 0xba) + *(char *)(iRam000007d4 + 0xba);
      *(char *)(iRam000007d0 + 0xbb) =
           *(char *)(iRam000007d0 + 0xbb) + *(char *)(iRam000007d4 + 0xbb);
      FUN_800136c4(acStack_110,iRam000007d0);
    }
    iVar3 = FUN_80052544(acStack_110);
    pcVar5 = acStack_110 + iVar3;
    pcVar6 = &DAT_80010238;
    break;
  default:
    goto switchD_80013964_default;
  }
  sprintf(pcVar5,pcVar6,uRam00000674,uRam00000678);
switchD_80013964_default:
  strcat(acStack_110,&DAT_80010274);
LAB_80013c80:
  uVar4 = FUN_80019458(param_1,acStack_110,0x40,0x40);
  FUN_80018efc(uVar4);
  return;
}

#endif  /* GHIDRA REF */
