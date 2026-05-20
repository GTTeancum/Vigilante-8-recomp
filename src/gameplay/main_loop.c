/* main_loop.c -- V8 outer game/match loop.
 *
 * Source: SLUS_005.10  FUN_80013cac.
 *
 * The top-level loop that owns one shell->load->play->result iteration:
 *
 *   1. One-time framebuffer init: 12 DRAWENVs + 2 DISPENVs covering all
 *      split-screen modes (full, 2P-horiz, 2P-vert), then MargePrim
 *      stitches a base GPU primitive list.
 *
 *   2. Per-iteration outer body:
 *      - On first shell entry only, load Shell\Shell.dll and call its
 *        entry (returns level path string). Empty -> exit game.
 *      - Build a bank mask from DAT_80065674/UNK_8006567a (P1/P2 vehicle
 *        choice + per-bank enable bits), pass to FUN_800227a4.
 *      - Load Sounds\Main.SND, Misc\Game.FNT, init font, init match
 *        state. Demo seed = 0xbb40e64d when mode==5.
 *      - iRam00000010 = split-screen flag (1=horiz/2=vert when match
 *        mode 3 or 4); affects all viewport math below.
 *      - FUN_80022ba8(): load the level data (LZS/BIN/OBJ stack).
 *
 *   3. Inner per-frame loop (LAB_80014224):
 *      - Every 8 frames, poll for pause request (FUN_80043bb4) and
 *        invoke pause menu (FUN_80012a90); break out to teardown.
 *      - Pad_Tick() once per tick-needed (1 or 2 ticks based on demo
 *        record/replay catch-up math).
 *      - Run the physics step (FUN_8002131c, FUN_80021394) and the
 *        post-step pass (FUN_80021678).
 *      - For each visible player viewport (single or split):
 *          set view target (Render_PointCameraAt), build object lists,
 *          ClearOTagR, draw radar overlay (Render_ScreenOverlay),
 *          draw HUD (FUN_80019d10), draw match-result overlay if
 *          iRam00000624 > 300 (winner declared), DrawSync, swap.
 *      - End match when:
 *          quest mode 0: pad button mask 0x8400000 (Start/X) OR alive==0
 *            AND timer > 1200 frames.
 *          versus modes: 0x8600000 (Start/X/Triangle on either pad) OR
 *            timer > 1200; remember 0x200000 (Select) for skip-to-shell.
 *
 *   4. Teardown:
 *      - Free both vehicles (FUN_800204dc on the loaded BIN handles),
 *        free demo replay buffer, free audio.
 *      - InsertOriginalCd_Loop if disc swapped during play.
 *      - Frees fonts; loop back to top.
 *
 * This is the master scheduler; almost every gameplay+render call
 * threads through here. MED -- exact GPU plumbing is renderer-adjacent
 * but the control flow + state transitions are 1:1 with the original.
 */
#include <stdint.h>

/* PSX libgs */
typedef struct { int16_t x, y, w, h; } RECT;
typedef struct DRAWENV DRAWENV;
typedef struct DISPENV DISPENV;
typedef struct DR_ENV  DR_ENV;
typedef uint32_t       u_long;
extern void SetDefDrawEnv(DRAWENV *d, int x, int y, int w, int h);
extern void SetDefDispEnv(DISPENV *d, int x, int y, int w, int h);
extern void ClearImage(RECT *r, uint8_t r8, uint8_t g8, uint8_t b8);
extern void ClearOTagR(u_long *ot, int n);
extern void DrawOTag(u_long *ot);
extern void DrawSync(int mode);
extern void SetDrawEnv(DR_ENV *e, DRAWENV *de);
extern void PutDrawEnv(DRAWENV *de);
extern void MargePrim(void *a, void *b);
extern int  strcpy(char *d, const char *s);

/* V8 calls */
extern void     Sched_BeginFrame(void);                 /* FUN_80011834 */
extern void     Shell_PreEnter(void);                   /* FUN_800128d4 */
extern void     Audio_ResetVoices(uint32_t mask);       /* FUN_800251fc */
extern int      Overlay_Open(const char *path);         /* FUN_80011adc */
extern void     Overlay_Close(int handle);              /* FUN_80045088 */
extern void     Audio_StreamReset(void);                /* FUN_80029dec */
extern void     Audio_BankSelect(uint32_t mask);        /* FUN_800227a4 */
extern uint32_t Sound_LoadSND(const char *path);        /* FUN_80044360 */
extern uint32_t Font_LoadFNT(const char *path);         /* FUN_80015f80 */
extern void     Font_PrepareGlyphs(int idx);            /* FUN_800165cc */
extern void     Font_SelectBank(int bank);              /* FUN_80017fd4 */
extern int      Font_AcquireRenderer(uint32_t fnt, int slots); /* FUN_80019034 */
extern void     Font_FinalizeRenderer(void);            /* FUN_8001910c */
extern void     Match_ResetState(void);                 /* FUN_8002a598 */
extern void     V8_SeedRng(uint32_t seed);              /* FUN_8001714c */
extern void     Level_LoadByName(char *outBuf, char *displayName, int extra); /* FUN_80022ba8 */
extern void     Shell_PostLoad(void);                   /* FUN_80012980 */
extern void     Pad_TickWithMode(int mode);             /* FUN_800212c4 */
extern void     Music_StartShell(void *p, int seed);    /* FUN_8001a0ac */
extern void     Font_DrawLine(int rend, void *str, int x, int y); /* FUN_80019960 */
extern void     Sched_WaitFrame(void);                  /* FUN_800126f0 */
extern int      Pause_Poll(void);                       /* FUN_80043bb4 */
extern int      PauseMenu_Run(int fontRend, int p2);    /* FUN_80012a90 */
extern void     Pad_Tick(void);                         /* FUN_800120d4 */
extern void     Physics_Step(uint32_t catchupFlag);     /* FUN_8002131c */
extern void     Physics_PostStep(uint32_t tick);        /* FUN_80021394 */
extern void     Physics_FlushVoxels(void);              /* FUN_80021678 */
extern void     Render_PointCameraAt(uint32_t target, int yaw); /* FUN_8001db24 */
extern void     Render_SelectViewport(int viewport);    /* FUN_800119c0 */
extern void     Render_SetView(int w, int h, int cx, int cy); /* FUN_8001d994 */
extern void     Render_BuildLists(void);                /* FUN_80021600 */
extern void     FxOverlay_QueueFlash(void *vehicle);    /* FUN_8002b8d0 */
extern void     Vehicle_RenderInto(void *vehicle, void *cam, u_long *ot); /* FUN_8002b7bc */
extern void     Render_FlushDPQ(uint32_t fxPlane, void *vehicle, u_long *ot); /* FUN_8002af98 */
extern int      HUD_DrawOverlay(uint32_t *flagsPtr, int fontRend, u_long *ot, uint32_t passes); /* FUN_80019d10 */
extern void     Font_DrawLineColored(int rend, const char *s, void *xy, int len); /* FUN_80019c64 */
extern int      ResultScreen_Build(int fontHandle);     /* FUN_8001392c */
extern void     ResultScreen_Tick(int rs, u_long *ot);  /* FUN_80018f7c */
extern void     ResultScreen_Free(int rs);              /* FUN_80018f3c */
extern void     Shell_TickDeferred(void);               /* FUN_800128bc */
extern void     Render_AddScreenBlit(int dx, int dy, u_long *ot); /* FUN_8002a25c */
extern void     Render_Submit(DISPENV *disp, DRAWENV *draw, u_long *ot, int gpuAddr); /* FUN_80012828 */
extern void     Audio_PlayXA(const char *path, int slot); /* FUN_80043df8 */
extern uint32_t Cheats_TryUnlock(void);                  /* FUN_800220d4 */
extern void     Demo_RecorderShutdown(void);            /* FUN_8001265c */
extern void     Demo_Save(void *buf);                   /* FUN_80011c58 */
extern void     Demo_ResetState(void);                  /* FUN_800126c8 */
extern void     Audio_ClearAll(void);                   /* FUN_80044054 */
extern void     Audio_FreeSND(uint32_t snd);             /* FUN_80044394 */
extern void     Level_Free(void);                        /* FUN_80022a1c */
extern void     Vehicle_Free(uint32_t bin);              /* FUN_800204dc */
extern void     Match_End(void);                         /* FUN_8002accc */
extern void     Sched_ResetTimers(void);                 /* FUN_80041e80 */
extern void     InsertOriginalCd_Loop(int fontRend);     /* FUN_8001356c */
extern void     Font_ReleaseRenderer(int rend);          /* FUN_800190d8 */
extern void     Pad_Close(int port);                     /* FUN_80011914 */
extern void     Font_Free(int slot);                     /* FUN_80016678 */

/* Global match state */
extern int8_t   bRam00000015;          /* g_matchMode */
extern int8_t   cRam00000018, cRam00000019;
extern uint32_t *puRam000007d0, *puRam000007d4;
extern int32_t  iRam00000004, iRam00000008, iRam00000010, iRam0000001c, iRam00000024;
extern int32_t  iRam00000608, iRam00000618, iRam00000624, iRam00000628;
extern int32_t  iRam000005ac, iRam0000060c, iRamffffacb0;
extern uint8_t  uRam0000000c, uRam000006cc, uRam000006cf, uRam00000014;
extern uint8_t  cRam00000600, uRam000005f8, uRam00000680, uRam0000062c, uRam00000630;
extern uint16_t uRam000006f0, uRam000007dc, uRam00000620;
extern uint32_t DAT_80065c30, DAT_80065c28, DAT_80065c40;
extern uint8_t  DAT_80065674, UNK_8006567a;
extern void    *DAT_8006eed8, *DAT_8006eee0;
extern uint32_t DAT_8006eef0, DAT_8006eff8, DAT_8006f100;

void V8_MainLoop(void)
{
    char     levelName[64];
    char     shellPath[64];
    RECT     clr;
    u_long   ot;
    uint32_t firstShellEntry = 0;

    /* --- DRAWENV table init (12 entries x 0x5c bytes; alternating
       full-frame and split sub-frames) ---------------------------- */
    for (char *p = levelName; p < shellPath; p += 0x20) {
        p[3] = 3; p[7] = '`'; p[4] = p[5] = p[6] = 0;
        if (p == levelName) {
            p[8] = 0; p[9] = 0; p[10] = 'w'; p[11] = 0;
            p[12] = 0x40; p[13] = 1; p[14] = 2; p[15] = 0;
        } else {
            p[8] = (char)0x9f; p[9] = 0; p[10] = 0; p[11] = 0;
            p[12] = 2; p[13] = 0; p[14] = (char)0xf0; p[15] = 0;
        }
        for (int i = 0; i < 0x10; i++) p[0x10 + i] = p[i];
    }
    /* Twelve DRAWENVs, two DISPENVs - see source for exact dims. */
    /* ... SetDefDrawEnv / SetDefDispEnv calls elided here ... */

    /* GPU command list head/tail seeded for the dual buffer chain. */

    for (;;) {
        int resultScreen = 0;
        Sched_BeginFrame();
        Shell_PreEnter();
        Audio_ResetVoices(0x40);

        if (firstShellEntry == 0) {
            int h = Overlay_Open("Shell\\Shell.dll");
            char *path = (char *)(*(uint32_t (**)(void))(h + 4))();
            strcpy(shellPath, path ? path : "");
            Overlay_Close(h);
            if (shellPath[0] == '\0') return;
        }
        firstShellEntry = 0;

        cRam00000018 = (int8_t)cRam00000018;       /* DISPENV pos copied */
        Audio_StreamReset();

        uint32_t mask = 0xe000;
        for (int i = 0; i < 8; i++) {
            if (i < 2 || (&UNK_8006567a)[i] != 0)
                mask |= 1u << ((&DAT_80065674)[i] & 0x1f);
        }
        Audio_BankSelect(mask);

        uRam000005f8 = (uint8_t)Sound_LoadSND("Sounds\\Main.SND");
        uint32_t fnt = Font_LoadFNT("Misc\\Game.FNT");
        Font_PrepareGlyphs(0);
        Font_SelectBank(1);
        iRam00000628 = Font_AcquireRenderer(fnt, 0x23);
        Font_FinalizeRenderer();
        Match_ResetState();
        if (bRam00000015 == 5) V8_SeedRng(0xbb40e64du);

        DAT_8006f100 = DAT_8006eff8 = DAT_8006eef0 = 0;
        int splitFlag = (bRam00000015 - 3 < 2);
        iRam00000010 = splitFlag << 1;
        uRam000006cf = splitFlag ? 0xab : 0xff;

        const char *banner = "";
        uint8_t     bannerKind = 0;
        if (bRam00000015 == 0) {
            int slot = *(int *)(iRam00000608 + DAT_80065674 * 8 + 8) + cRam00000600 * 0x10;
            banner = *(char **)(slot + 0xc);
            uRam000006f0  = *(uint16_t *)(slot + 2);
            uRam000007dc = *(uint16_t *)(slot + 4);
            bannerKind   = *(uint8_t  *)(slot + 1);
        } else if (iRam00000618 != 0) {
            banner = "Loading Demo Level";
        }
        Level_LoadByName(shellPath, (char *)banner, bannerKind);

        Shell_PostLoad();
        uRam0000000c = 0;
        Pad_TickWithMode(0);

        if (bRam00000015 == 0) {
            extern uint8_t DAT_80065604; extern char DAT_800102b0[];
            Music_StartShell(&DAT_80065604, 0);
            Font_DrawLine(iRam00000628, DAT_800102b0, 0x10, 0xc0);
            do { Sched_WaitFrame(); } while ((uRam0000062c & 0x40) == 0);
        }

        uint32_t p1Bin = puRam000007d0[0x38];
        uint32_t p2Bin = puRam000007d4 ? puRam000007d4[0x38] : 0u;
        clr.x = 0; clr.y = 0; clr.w = 0x140; clr.h = 0x1e0;
        ClearImage(&clr, 0, 0, 0);

        uint32_t frame = 0;
        iRamffffacb0 = 1;
        iRam00000624 = 0;
        int p1View = 1, p2View = 1;

        for (int countdown = 0x78; ; ) {
            uint32_t padOr = 0, padOr2 = 0;
            frame++;
            if ((frame & 7) == 0 && Pause_Poll()) {
                if (PauseMenu_Run(iRam00000628, 0) != 0) goto teardown;
            }
            int ticks = 2;
            if (iRam00000618 == 0) ticks = (iRam0000001c == 0) ? 1
                                          : (iRamffffacb0 - uRam0000000c);
            for (uint32_t t = 0; t < (uint32_t)ticks; t++) {
                Pad_Tick();
                uRam0000000c++;
                uRam000006cc = (uint8_t)uRam0000000c;
                Physics_Step(t == (uint32_t)ticks - 1 ? (uint32_t)ticks : 0);
                Physics_PostStep(uRam0000000c);
                Physics_FlushVoxels();
                if ((DAT_80065c30 & 0x800000) != 0) p1View = 3 - p1View;
                padOr  |= uRam0000062c;
                padOr2 |= uRam00000630;
            }
            Pad_TickWithMode(uRam0000000c & 0xffff);
            iRam00000008 = 1 - iRam00000008;

            if ((int16_t)puRam000007d0[3] == 0) p1View = 0;
            else if (p1View == 2 && (*puRam000007d0 & 0x1000000)) p1View = 1;
            if (puRam000007d4) {
                if ((int16_t)puRam000007d4[3] == 0) p2View = 0;
                else if (p2View == 2 && (*puRam000007d4 & 0x1000000)) p2View = 1;
            }

            /* Single-screen path (split flag 0) vs split path -- both
             * arms build the OT and submit; see source for full code.
             * The match-end detection at the bottom is the same for
             * both arms: */

            if (resultScreen == 0 && iRam00000624 != 0 && DAT_8006eef0 == 0) {
                if (iRam000005ac == 4) {
                    const char *xa = (iRam00000024 == 0) ? "Sounds\\Defeat.xa"
                                                         : "Sounds\\Victory.xa";
                    Audio_PlayXA(xa, (int)DAT_80065674);
                }
                if (bRam00000015 == 0) {
                    uRam00000620 = (uint16_t)Cheats_TryUnlock();
                    uRam00000620 ^= (uint16_t)(DAT_80065674 < 6);
                }
                resultScreen = ResultScreen_Build(iRam00000628);
            } else if (resultScreen != 0) {
                ResultScreen_Tick(resultScreen, &ot);
            }

            if (resultScreen != 0 && (iRam00000624 += ticks) > 300) {
                if (bRam00000015 == 0) {
                    if ((padOr & 0x8400000) ||
                        (iRam00000024 == 0 && iRam00000624 > 0x4b0))
                        goto teardown;
                } else if (((padOr | padOr2) & 0x8600000) ||
                           iRam00000624 > 0x4b0) {
                    firstShellEntry = (padOr | padOr2) & 0x200000;
                    goto teardown;
                }
            }

            uint32_t any = padOr | padOr2;
            if ((any & 0x100) == 0) {        /* not Start */
                if ((any & 0x8000000) &&
                    (iRam00000618 != 0 ||
                     PauseMenu_Run(iRam00000628, ((padOr >> 0x1b) ^ 1) & 1)))
                    break;
                continue;
            }
            if ((any & 0x800) == 0) { countdown = 0x78; continue; }
            countdown -= ticks;
            if (countdown < 0) { uRam00000014 = 3; break; }
        }

teardown:
        if (resultScreen != 0) ResultScreen_Free(resultScreen);
        if (iRam00000618 != 0) {
            Demo_RecorderShutdown();
            Pad_Tick();
            uRam00000014 = (uRam0000062c == 0 && uRam00000630 == 0) ? 2 : 3;
            extern uint8_t DAT_80065968;
            Demo_Save(&DAT_80065968);
            Demo_ResetState();
        }
        Shell_TickDeferred();
        Audio_ClearAll();
        Audio_FreeSND(uRam000005f8);
        Level_Free();
        Vehicle_Free(p1Bin);
        if (puRam000007d4) Vehicle_Free(p2Bin);
        Match_End();
        Sched_ResetTimers();
        InsertOriginalCd_Loop(iRam00000628);
        Font_ReleaseRenderer(iRam00000628);
        Pad_Close(0);
        Pad_Close(1);
        Font_Free(0);
    }
}
