/* pause_menu.c -- in-match pause menu.
 *
 * Source: SLUS_005.10  FUN_80012a90.
 *
 * Invoked by V8_MainLoop when the active player presses Start. Owns
 * the screen until the user picks an action; returns nonzero to tell
 * the main loop "exit this match".
 *
 * Menu items (single-screen):
 *   row 0  CD Track  <n>           (left/right cycles tracks 1..N)
 *   row 1  Resume / Quit toggle
 * Split-screen mode adds a second row:
 *   row 1  Split    Horiz/Vert     (left/right toggles)
 * Confirmation dialog when quitting: "ARE YOU SURE?" with YES/NO.
 *
 * Also handles two error overlays interleaved with the menu:
 *   - CD COVER OPEN / READING INDEX (poll Disc_CoverOpen + Disc_ReadIndex)
 *   - REPLACE CONTROLLER N (poll port status)
 *
 * The current selection is `cursor` (initial 2 -> Resume/Quit row).
 * Pad bits used (same as main loop): 0x10000000=Up, 0x40000000=Down,
 * 0x80000000=Left, 0x20000000=Right, 0x8000000=X, 0x400000=Start,
 * 0xa0000000=L/R triggers, 0xa8400000=any-of-above.
 *
 * On exit:
 *   - If "quit" confirmed, copies saved per-voice volumes back into
 *     SPU voice regs (we snapshotted them at entry) and re-arms the
 *     viewports if split mode changed, then returns 1 (quitFlag).
 *   - Else returns 0 and restores volumes.
 *
 * MED. The SPU voice plumbing is audio-engine-out-of-scope; the
 * selection state machine is 1:1.
 */
#include <stdint.h>

typedef struct { int16_t x, y, w, h; } RECT;

extern int  sprintf(char *dst, const char *fmt, ...);
extern void VSync(int mode);
extern void ClearImage(RECT *r, uint8_t, uint8_t, uint8_t);
extern int  Pause_Poll(void);                         /* FUN_80043bb4 */
extern int  Disc_ReadIndex(void);                     /* FUN_80043aec */
extern void Audio_SnapshotVoices(void);               /* FUN_80043be4 */
extern void Audio_RestoreVoices(void);                /* FUN_80043c0c */
extern void Pad_Tick(void);                           /* FUN_800120d4 */
extern void Shell_TickDeferred(void);                 /* FUN_800128bc */
extern void Font_MoveCursor(void *xy, int kind);      /* FUN_8001a0ac */
extern void Font_SetColor(void *xy, uint32_t rgb);    /* FUN_80019f9c */
extern void Font_DrawLine2D(int x1, int y1, int x2, int y2, uint32_t rgb); /* FUN_80019f44 */
extern void Font_SelectGlyphTable(int rend, int idx); /* FUN_80019010 */
extern void Font_DrawText(int rend, const char *s, void *xy, int just); /* FUN_80019a58 */
extern void Music_Play(int track);                    /* FUN_80043ce0 */
extern void Vehicle_RebuildViewport(uint32_t modelPtr); /* FUN_8003d898 */
extern void PutDrawEnv(void *de);

extern uint32_t *VOICE_00_LEFT_RIGHT;
extern int16_t   sRam00000184, sRam00000186, sRam0000018a;
extern uint8_t   bRam000008f0, bRam000008f8;
extern int32_t   iRam00000004, iRam00000008, iRam00000010;
extern uint8_t   uRam0000000c, uRam000005ac;
extern uint32_t  uRamffffacb0;
extern int32_t   iRam000007d0, iRam000007d4;
extern uint32_t  DAT_80065930[];
extern int16_t   DAT_80065c28[];
extern char      s_REPLACE_CONTROLLER_x_8005686c[];
extern uint8_t   DAT_80065510, DAT_80065518;
extern uint8_t   DAT_80065488, DAT_80065490, DAT_80065464, DAT_80065468;
extern uint8_t   DAT_800654a8, DAT_800654ac, DAT_800654c8;

int Menu_Pause(int fontRend, int padIx)
{
    uint8_t  options[3];      /* {cdTrack, splitMode, quitFlag} */
    uint8_t  optMax [3];      /* {numTracks, 2, 2}              */
    int      voiceSnap[24];   /* per-voice L/R volumes */
    int      cursor   = 2;
    uint32_t spinTick = 0;

    /* Initial state. */
    options[1] = (uint8_t)(iRam00000010 - 1);
    options[0] = bRam000008f0;
    options[2] = 1;                /* default highlight = "Resume" */
    optMax [0] = bRam000008f8;
    optMax [1] = 2;
    optMax [2] = 2;

    Audio_SnapshotVoices();
    for (int i = 0; i < 24; i++) {
        voiceSnap[i] = (int)((uint32_t *)&VOICE_00_LEFT_RIGHT)[i*4 + 1];
        ((uint16_t *)&VOICE_00_LEFT_RIGHT)[(i*4 + 1)*2] = 0;
    }
    Shell_TickDeferred();
    PutDrawEnv((void *)((1 - iRam00000008) * 0x5c));   /* finalize last frame */
    sRam0000018a = (iRam00000010 != 0) ? 0x4c : 0x3c;
    Font_MoveCursor(&DAT_80065488, 0);
    Font_SetColor(&DAT_80065488, 0xffffff);
    Font_DrawLine2D(0x58, 0x66, 0xe8, 0x66, 0x808080);
    Font_SelectGlyphTable(fontRend, 1);
    Font_DrawText(fontRend, "PAUSED", &DAT_80065490, 2);

    const uint32_t HI = 0x808080, LO = 0x404040;
    int padBase = padIx * 0x30;

    for (;;) {
        /* --- Layout + read input -------------------------------- */
        char numBuf[8];
        sprintf(numBuf, "%i", options[0] + 1);
        Font_DrawText(fontRend, "CD Track", NULL, 0);
        Font_DrawText(fontRend, numBuf, NULL, 2);
        if (iRam00000010 != 0) {
            const char *s = (options[1] == 0) ? "Horiz" : "Vert";
            Font_DrawText(fontRend, "Split", NULL, 0);
            Font_DrawText(fontRend, s, NULL, 2);
        }
        Font_DrawText(fontRend, (const char *)&DAT_800654c8, NULL, 0);
        Font_DrawText(fontRend, "Resume", NULL, 1);

        uint32_t pad;
        for (;;) {
            VSync(0);
            Pad_Tick();
            pad = DAT_80065930[padBase / 4];
            if (DAT_80065c28[padIx * 0xc] >= 2) {
                /* Port unplugged. */
                Font_DrawText(fontRend, "REPLACE CONTROLLER x", &DAT_80065490, 2);
                do { Pad_Tick(); } while (DAT_80065c28[padIx * 0xc] >= 2);
                continue;
            }
            spinTick++;
            if ((spinTick & 0xf) == 0 && Pause_Poll()) {
                /* CD cover opened. */
                do {
                    Font_DrawText(fontRend, "CD COVER OPEN", &DAT_80065490, 2);
                    while (Pause_Poll()) { }
                    Font_DrawText(fontRend, "READING INDEX", &DAT_80065490, 2);
                } while (Disc_ReadIndex() == 0);
                Font_DrawText(fontRend, (uRam000005ac == 1) ? "NO CD" : "PAUSED",
                              &DAT_80065490, 2);
                options[0] = 0;
                optMax [0] = bRam000008f8;
                Music_Play(0);
                pad = 0x8400000;        /* synthesize quit-confirm */
                goto handled;
            }
            if ((pad & 0xf8400000) != 0) break;
        }
handled:

        if (pad & 0x8000000) goto confirmQuit;

        if ((pad & 0x10000000) && cursor != 0) {
            cursor--;
            if (iRam00000010 == 0) cursor = 0;
        }
        if ((pad & 0x40000000) && cursor < 2) {
            cursor++;
            if (iRam00000010 == 0) cursor = 2;
        }
        if (pad & 0x80000000) {
            uint8_t v = options[cursor];
            uint8_t n = (uint8_t)(v - 1);
            if (v == 0) { n = (uint8_t)(optMax[cursor] - 1);
                          if (optMax[cursor] == 0) n = 0; }
            options[cursor] = n;
        }
        if (pad & 0x20000000) {
            uint8_t v = options[cursor];
            uint8_t n = (uint8_t)(v + 1);
            if ((int)(optMax[cursor] - 1) <= v) n = 0;
            options[cursor] = n;
        }
        if (cursor == 0 && uRam000005ac > 1 && (pad & 0xa0000000))
            Music_Play(options[0]);
        if ((pad & 0x400000) && cursor == 2) goto confirmQuit;
    }

confirmQuit: {
        int yes = 1;
        Font_DrawText(fontRend, "ARE YOU SURE?", &DAT_80065490, 2);
        for (;;) {
            uint32_t pad;
            Font_DrawText(fontRend, (const char *)&DAT_80065464, NULL, 8);
            Font_DrawText(fontRend, (const char *)&DAT_80065468, NULL, 9);
            do { Pad_Tick(); pad = DAT_80065930[padBase / 4]; }
            while ((pad & 0xa8400000) == 0);
            if (pad & 0x80000000) yes = 0;
            if (pad & 0x20000000) yes = 1;
            if (pad & 0x8400000) {
                if (yes && !(pad & 0x8000000)) {
                    Font_SelectGlyphTable(fontRend, 0x23);
                    options[2] = 1;
                    /* restore voices */
                    for (int i = 0; i < 24; i++)
                        ((uint16_t *)&VOICE_00_LEFT_RIGHT)[(i*4 + 1)*2] = (uint16_t)voiceSnap[i];
                    Audio_RestoreVoices();
                    if (iRam00000010 != 0) {
                        iRam00000010 = options[1] + 1;
                        RECT r;
                        if (iRam00000010 == 1) {
                            ClearImage((RECT *)&DAT_80065510, 0, 0, 0);
                            ClearImage((RECT *)&DAT_80065518, 0, 0, 0);
                        } else {
                            r.x = 0x9f; r.y = 0; r.w = 2; r.h = 0x1e0;
                            ClearImage(&r, 0, 0, 0);
                        }
                        Vehicle_RebuildViewport(*(uint32_t *)(iRam000007d0 + 0xe0));
                        Vehicle_RebuildViewport(*(uint32_t *)(iRam000007d4 + 0xe0));
                    }
                    uRamffffacb0 = uRam0000000c;
                    return 1;   /* quit the match */
                }
                break;
            }
        }
        Font_DrawText(fontRend, "PAUSED", &DAT_80065490, 2);
        goto handled;
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

/* --- SLUS_005.10 FUN_80012a90  (from analysis/SLUS_005.10/decomp/80012a90.c) --- */
// addr: 0x80012a90  name: FUN_80012a90

bool FUN_80012a90(int param_1,int param_2)

{
  byte bVar1;
  undefined2 uVar2;
  bool bVar3;
  byte bVar4;
  short sVar5;
  uint uVar6;
  dword *pdVar7;
  undefined2 *puVar8;
  RECT *rect;
  char *pcVar9;
  uint *puVar10;
  uint uVar11;
  byte bVar12;
  uint uVar13;
  int iVar14;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  char acStack_98 [8];
  undefined2 local_90 [24];
  byte local_60 [8];
  byte local_58 [8];
  RECT local_50;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  uint local_38;
  uint local_34;
  int local_30;
  
  bVar12 = 2;
  local_a0 = (uint)(ushort)(sRam00000184 + 8);
  local_a8 = local_a0;
  local_a4 = 0xf0048;
  local_a0 = 0xa0;
  local_9c = 0xf0048;
  local_60[1] = (char)iRam00000010 - 1;
  local_60[0] = bRam000008f0;
  local_60[2] = '\x01';
  local_50.x = CONCAT11(2,bRam000008f8);
  local_50.y = CONCAT11(local_50.y._1_1_,2);
  local_58[0] = bRam000008f8;
  local_58[1] = 2;
  local_58[2] = 2;
  uVar13 = 0;
  FUN_80043be4();
  puVar8 = local_90;
  pdVar7 = &VOICE_00_LEFT_RIGHT;
  do {
    uVar13 = uVar13 + 1;
    *puVar8 = (short)pdVar7[1];
    puVar8 = puVar8 + 1;
    *(undefined2 *)(pdVar7 + 1) = 0;
    pdVar7 = pdVar7 + 4;
  } while ((int)uVar13 < 0x18);
  FUN_800128bc();
  PutDrawEnv((DRAWENV *)(&DAT_8006f208 + (1 - iRam00000008) * 0x5c));
  sRam0000018a = 0x3c;
  if (iRam00000010 != 0) {
    sRam0000018a = 0x4c;
  }
  FUN_8001a0ac(&DAT_80065488,0);
  FUN_80019f9c(&DAT_80065488,0xffffff);
  FUN_80019f44(0x58,0x66,0xe8,0x66,0x808080);
  *(undefined1 *)(param_1 + 4) = 0x80;
  *(undefined1 *)(param_1 + 5) = 0x80;
  *(undefined1 *)(param_1 + 6) = 0;
  FUN_80019010(param_1,1);
  FUN_80019a58(param_1,"PAUSED",&DAT_80065490,2);
  local_38 = 0x808080;
  local_34 = 0x404040;
  local_30 = param_2 << 2;
  do {
    do {
      sVar5 = sRam00000186 + 0x18;
      local_a8 = CONCAT22(sVar5,(undefined2)local_a8);
      uVar11 = local_34;
      if (bVar12 == 0) {
        uVar11 = local_38;
      }
      *(uint *)(param_1 + 4) = *(uint *)(param_1 + 4) & 0xff000000 | uVar11;
      local_a0._2_2_ = sVar5;
      sprintf(acStack_98,"%i",local_60[0] + 1);
      FUN_80019a58(param_1,"CD Track",&local_a8,0);
      FUN_8001a0ac(&local_a0,0);
      FUN_80019a58(param_1,acStack_98,&local_a0,2);
      *(undefined1 *)(param_1 + 4) = 0;
      *(undefined1 *)(param_1 + 5) = 0x80;
      *(undefined1 *)(param_1 + 6) = 0;
      FUN_80019a58(param_1,&DAT_800654a8,&local_a0,0);
      FUN_80019a58(param_1,&DAT_800654ac,&local_a0,1);
      if (iRam00000010 != 0) {
        local_a0._2_2_ = local_a0._2_2_ + 0x10;
        local_a8 = CONCAT22(local_a0._2_2_,(undefined2)local_a8);
        uVar11 = local_34;
        if (bVar12 == 1) {
          uVar11 = local_38;
        }
        *(uint *)(param_1 + 4) = *(uint *)(param_1 + 4) & 0xff000000 | uVar11;
        FUN_80019a58(param_1,"Split",&local_a8,0);
        FUN_8001a0ac(&local_a0,0);
        if (local_60[1] == 0) {
          pcVar9 = "Horiz";
        }
        else {
          pcVar9 = "Vert";
        }
        FUN_80019a58(param_1,pcVar9,&local_a0,2);
        *(undefined1 *)(param_1 + 4) = 0;
        *(undefined1 *)(param_1 + 5) = 0x80;
        *(undefined1 *)(param_1 + 6) = 0;
        FUN_80019a58(param_1,&DAT_800654a8,&local_a0,0);
        FUN_80019a58(param_1,&DAT_800654ac,&local_a0,1);
      }
      local_a0._2_2_ = local_a0._2_2_ + 0x10;
      local_a8 = CONCAT22(local_a0._2_2_,(undefined2)local_a8);
      uVar11 = local_34;
      if ((bVar12 == 2) && (local_60[2] == '\0')) {
        uVar11 = local_38;
      }
      *(uint *)(param_1 + 4) = *(uint *)(param_1 + 4) & 0xff000000 | uVar11;
      FUN_80019a58(param_1,&DAT_800654c8,&local_a8,0);
      uVar11 = local_34;
      if ((bVar12 == 2) && (local_60[2] != '\0')) {
        uVar11 = local_38;
      }
      *(uint *)(param_1 + 4) = *(uint *)(param_1 + 4) & 0xff000000 | uVar11;
      FUN_80019a58(param_1,"Resume",&local_a0,1);
      while( true ) {
        VSync(0);
        FUN_800120d4();
        uVar11 = *(uint *)((int)&DAT_80065930 + local_30);
        if ((short)(&DAT_80065c28)[param_2 * 0xc] < 2) break;
        uVar6 = uVar13 & 0xf;
        uVar13 = uVar13 + 1;
        if ((uVar6 == 0) && (iVar14 = FUN_80043bb4(), iVar14 != 0)) {
          *(undefined1 *)(param_1 + 4) = 0x80;
          *(undefined1 *)(param_1 + 5) = 0x80;
          *(undefined1 *)(param_1 + 6) = 0;
          do {
            FUN_8001a0ac(&DAT_80065490,0);
            FUN_80019a58(param_1,"CD COVER OPEN",&DAT_80065490,2);
            do {
              iVar14 = FUN_80043bb4();
            } while (iVar14 != 0);
            FUN_8001a0ac(&DAT_80065490,0);
            FUN_80019a58(param_1,"READING INDEX",&DAT_80065490,2);
            iVar14 = FUN_80043aec();
          } while (iVar14 == 0);
          FUN_8001a0ac(&DAT_80065490,0);
          if (uRam000005ac == 1) {
            pcVar9 = "NO CD";
          }
          else {
            pcVar9 = "PAUSED";
          }
          FUN_80019a58(param_1,pcVar9,&DAT_80065490,2);
          local_60[0] = 0;
          local_58[0] = bRam000008f8;
          FUN_80043ce0(0);
          goto LAB_80013120;
        }
        if ((uVar11 & 0xf8400000) != 0) goto LAB_80013120;
      }
                    /* WARNING: Read-only address (ram,0x8005687f) is written */
      s_REPLACE_CONTROLLER_x_8005686c[0x13] = (char)param_2 + '1';
      *(undefined1 *)(param_1 + 4) = 0x80;
      *(undefined1 *)(param_1 + 5) = 0x80;
      *(undefined1 *)(param_1 + 6) = 0;
      FUN_8001a0ac(&DAT_80065490);
      FUN_80019a58(param_1,"REPLACE CONTROLLER x",&DAT_80065490,2);
      do {
        FUN_800120d4();
      } while ((short)(&DAT_80065c28)[param_2 * 0xc] < 2);
      FUN_8001a0ac(&DAT_80065490,0);
      FUN_80019a58(param_1,"PAUSED",&DAT_80065490,2);
LAB_80013120:
      if ((uVar11 & 0x8000000) != 0) goto LAB_80013420;
      if ((((uVar11 & 0x10000000) != 0) && (bVar12 != 0)) &&
         (bVar12 = bVar12 - 1, iRam00000010 == 0)) {
        bVar12 = 0;
      }
      if ((((uVar11 & 0x40000000) != 0) && (bVar12 < 2)) && (bVar12 = bVar12 + 1, iRam00000010 == 0)
         ) {
        bVar12 = 2;
      }
      if ((int)uVar11 < 0) {
        bVar1 = local_60[bVar12];
        bVar4 = bVar1 - 1;
        if ((bVar1 == 0) && (bVar4 = local_58[bVar12] - 1, local_58[bVar12] == 0)) {
          bVar4 = 0;
        }
        local_60[bVar12] = bVar4;
      }
      if ((uVar11 & 0x20000000) != 0) {
        bVar1 = local_60[bVar12];
        bVar4 = bVar1 + 1;
        if ((int)(local_58[bVar12] - 1) <= (int)(uint)bVar1) {
          bVar4 = 0;
        }
        local_60[bVar12] = bVar4;
      }
      if (((bVar12 == 0) && (1 < uRam000005ac)) && ((uVar11 & 0xa0000000) != 0)) {
        FUN_80043ce0(local_60[0]);
      }
    } while (((uVar11 & 0x400000) == 0) || (bVar12 != 2));
    local_40 = 0x670058;
    local_3c = CONCAT22(sRam0000018a + -0x16,0x90);
    local_48 = 0x670058;
    local_44 = local_3c;
    bVar3 = true;
    if (local_60[2] != '\0') {
LAB_80013420:
      FUN_80019010(param_1,0x23);
      if ((uVar11 & 0x8000000) != 0) {
        local_60[2] = '\x01';
      }
      iVar14 = 0;
      if (local_60[2] != '\0') {
        puVar8 = local_90;
        pdVar7 = &VOICE_00_LEFT_RIGHT;
        do {
          uVar2 = *puVar8;
          puVar8 = puVar8 + 1;
          iVar14 = iVar14 + 1;
          *(undefined2 *)(pdVar7 + 1) = uVar2;
          pdVar7 = pdVar7 + 4;
        } while (iVar14 < 0x18);
        FUN_80043c0c();
        if (iRam00000010 != 0) {
          iRam00000010 = local_60[1] + 1;
          rect = &local_50;
          if (iRam00000010 == 1) {
            ClearImage((RECT *)&DAT_80065510,'\0','\0','\0');
            rect = (RECT *)&DAT_80065518;
          }
          else {
            local_50.x = 0x9f;
            local_50.w = 2;
            local_50.y = 0;
            local_50.h = 0x1e0;
          }
          ClearImage(rect,'\0','\0','\0');
          FUN_8003d898(*(undefined4 *)(iRam000007d0 + 0xe0));
          FUN_8003d898(*(undefined4 *)(iRam000007d4 + 0xe0));
        }
      }
      uRamffffacb0 = uRam0000000c;
      return local_60[2] == '\0';
    }
    *(undefined1 *)(param_1 + 4) = 0x80;
    *(undefined1 *)(param_1 + 5) = 0x80;
    *(undefined1 *)(param_1 + 6) = 0;
    FUN_8001a0ac(&DAT_80065490,0);
    FUN_8001a0ac(&local_48,0);
    FUN_80019a58(param_1,"ARE YOU SURE?",&DAT_80065490,2);
    do {
      uVar11 = local_34;
      if (!bVar3) {
        uVar11 = local_38;
      }
      *(uint *)(param_1 + 4) = *(uint *)(param_1 + 4) & 0xff000000 | uVar11;
      FUN_80019a58(param_1,&DAT_80065464,&local_48,8);
      uVar11 = local_34;
      if (bVar3) {
        uVar11 = local_38;
      }
      *(uint *)(param_1 + 4) = *(uint *)(param_1 + 4) & 0xff000000 | uVar11;
      FUN_80019a58(param_1,&DAT_80065468,&local_48,9);
      puVar10 = (uint *)((int)&DAT_80065930 + local_30);
      do {
        FUN_800120d4();
        uVar11 = *puVar10;
      } while ((uVar11 & 0xa8400000) == 0);
      if ((int)uVar11 < 0) {
        bVar3 = false;
      }
      if ((uVar11 & 0x20000000) != 0) {
        bVar3 = true;
      }
    } while ((uVar11 & 0x8400000) == 0);
    if ((!bVar3) || ((uVar11 & 0x8000000) != 0)) goto LAB_80013420;
    FUN_8001a0ac(&DAT_80065490,0);
    FUN_8001a0ac(&local_48,0);
    *(undefined1 *)(param_1 + 4) = 0x80;
    *(undefined1 *)(param_1 + 5) = 0x80;
    *(undefined1 *)(param_1 + 6) = 0;
    FUN_80019a58(param_1,"PAUSED",&DAT_80065490,2);
  } while( true );
}

#endif  /* GHIDRA REF */
