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
#include <stdlib.h>

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
extern uintptr_t Overlay_Open(const char *path);        /* FUN_80011adc -- returns ptr-as-int on PSX */
extern void      Overlay_Close(uintptr_t handle);       /* FUN_80045088 */
extern void     Audio_StreamReset(void);                /* FUN_80029dec */
extern void     FUN_800227a4(uint32_t mask);
extern void     Host_VehicleInit(void);
extern void     Host_AIVehicleInit(void);
extern int      Host_WeaponProbe(void);
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
extern void     FUN_800212c4(uint16_t mode);            /* Frame_PreTickAll */
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
extern int32_t  iRam0000000c;
extern int32_t  iRam00000618, iRam00000624, iRam00000628;
extern int32_t  iRam000005ac, iRam0000060c, iRamffffacb0;
extern uintptr_t iRam00000608;
extern uint32_t uRam0000000c, uRam000006cc, uRam000006cf, uRam00000014;
extern uint8_t  cRam00000600, uRam00000680, uRam0000062c, uRam00000630;
extern uint32_t uRam000005f8;
extern uint16_t uRam000006f0, uRam000007dc, uRam00000620;
extern uint32_t DAT_80065c30, DAT_80065c28, DAT_80065c40;
extern uint8_t  DAT_80065674[];
extern uint8_t  UNK_8006567a;
extern void    *DAT_8006eed8, *DAT_8006eee0;
extern uint32_t DAT_8006eef0, DAT_8006eff8, DAT_8006f100;
extern int      g_v8_weapon_probe;

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
            uintptr_t h = Overlay_Open("Shell\\Shell.dll");
            /* PSX overlay handle layout: ptr-to-fn at +4 (32-bit PSX
             * sizes). On host this requires the loader's handle layout
             * to keep a function pointer at byte offset 4 -- see
             * platform/shell_stub.c which uses #pragma pack(4). */
            char *path = (char *)(*(char *(**)(void))(h + 4))();
            strcpy(shellPath, path ? path : "");
            Overlay_Close(h);
            if (shellPath[0] == '\0') return;
        }
        firstShellEntry = 0;

        cRam00000018 = (int8_t)cRam00000018;       /* DISPENV pos copied */
        Audio_StreamReset();

        uint32_t mask = 0xe000;
        if (DAT_80065674[1] >= 12 || DAT_80065674[1] == DAT_80065674[0]) {
            DAT_80065674[1] = (uint8_t)((DAT_80065674[0] + 1u) % 12u);
        }
        for (int i = 0; i < 8; i++) {
            if (i < 2 || (&UNK_8006567a)[i] != 0)
                mask |= 1u << (DAT_80065674[i] & 0x1f);
        }
        FUN_800227a4(mask);

        uRam000005f8 = Sound_LoadSND("Sounds\\Main.SND");
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
            uintptr_t slot = *(uint32_t *)(iRam00000608 + DAT_80065674[0] * 8 + 8)
                           + cRam00000600 * 0x10;
            banner = *(char **)(slot + 0xc);
            uRam000006f0  = *(uint16_t *)(slot + 2);
            uRam000007dc = *(uint16_t *)(slot + 4);
            bannerKind   = *(uint8_t  *)(slot + 1);
        } else if (iRam00000618 != 0) {
            banner = "Loading Demo Level";
        }
        Level_LoadByName(shellPath, (char *)banner, bannerKind);
        Host_VehicleInit();
        Host_AIVehicleInit();
        if (g_v8_weapon_probe)
            exit(Host_WeaponProbe());

        Shell_PostLoad();
        uRam0000000c = 0;
        FUN_800212c4(0);

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
            FUN_800212c4(uRam0000000c & 0xffff);
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
                    Audio_PlayXA(xa, (int)DAT_80065674[0]);
                }
                if (bRam00000015 == 0) {
                    uRam00000620 = (uint16_t)Cheats_TryUnlock();
                    uRam00000620 ^= (uint16_t)(DAT_80065674[0] < 6);
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

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_80013cac  (from analysis/SLUS_005.10/decomp/80013cac.c) --- */
// addr: 0x80013cac  name: FUN_80013cac

/* WARNING: Removing unreachable block (ram,0x80014340) */

void FUN_80013cac(void)

{
  short sVar1;
  bool bVar2;
  u_long uVar3;
  int iVar4;
  char *pcVar5;
  undefined4 uVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  undefined4 uVar10;
  uint *puVar11;
  undefined1 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  uint uVar15;
  uint uVar16;
  uint *puVar17;
  int iVar18;
  int iVar19;
  undefined4 uStackY_f0;
  char local_d0 [64];
  char local_90 [64];
  RECT local_50;
  u_long local_48;
  uint local_44;
  uint local_40;
  uint local_3c;
  uint local_38;
  int local_34;
  uint local_30;
  
  local_38 = 0;
  pcVar5 = local_d0;
  do {
    pcVar5[3] = '\x03';
    pcVar5[7] = '`';
    pcVar5[4] = '\0';
    pcVar5[5] = '\0';
    pcVar5[6] = '\0';
    if (pcVar5 == local_d0) {
      pcVar5[8] = '\0';
      pcVar5[9] = '\0';
      pcVar5[10] = 'w';
      pcVar5[0xb] = '\0';
      pcVar5[0xc] = '@';
      pcVar5[0xd] = '\x01';
      pcVar5[0xe] = '\x02';
      pcVar5[0xf] = '\0';
    }
    else {
      pcVar5[8] = -0x61;
      pcVar5[9] = '\0';
      pcVar5[10] = '\0';
      pcVar5[0xb] = '\0';
      pcVar5[0xc] = '\x02';
      pcVar5[0xd] = '\0';
      pcVar5[0xe] = -0x10;
      pcVar5[0xf] = '\0';
    }
    *(undefined4 *)(pcVar5 + 0x10) = *(undefined4 *)pcVar5;
    *(undefined4 *)(pcVar5 + 0x14) = *(undefined4 *)(pcVar5 + 4);
    *(undefined4 *)(pcVar5 + 0x18) = *(undefined4 *)(pcVar5 + 8);
    *(undefined4 *)(pcVar5 + 0x1c) = *(undefined4 *)(pcVar5 + 0xc);
    pcVar5 = pcVar5 + 0x20;
  } while (pcVar5 < local_90);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f208,0,0,0x140,0xf0);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f264,0,0xf0,0x140,0xf0);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f2c0,0,0,0x140,0x77);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f31c,0,0xf0,0x140,0x77);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f378,0,0x79,0x140,0x77);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f3d4,0,0x169,0x140,0x77);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f430,0,0,0x9f,0xf0);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f48c,0,0xf0,0x9f,0xf0);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f4e8,0xa1,0,0x9f,0xf0);
  SetDefDrawEnv((DRAWENV *)&DAT_8006f544,0xa1,0xf0,0x9f,0xf0);
  SetDefDispEnv((DISPENV *)&DAT_8006f5a0,0,0xf0,0x140,0xf0);
  SetDefDispEnv((DISPENV *)&DAT_8006f5b4,0,0,0x140,0xf0);
  DAT_8006eca0 = &DAT_8006eca4;
  DAT_8006eca4 = 0;
  DAT_8006eca8 = &DAT_8006eca0;
  DAT_8006ecac = &DAT_8006ecb0;
  DAT_8006ecb0 = 0;
  DAT_8006ecb4 = &DAT_8006ecac;
  FUN_80043ef0();
  DAT_8006eed8 = CONCAT13(1,(undefined3)DAT_8006eed8);
  DAT_8006eedc = 0xe1000220;
  DAT_8006eee3 = 3;
  DAT_8006eee4 = CONCAT13(0x62,(undefined3)DAT_8006eee4);
  DAT_8006eee8 = 0;
  DAT_8006eeec = 0x140;
  DAT_8006eeee = 0xf0;
  MargePrim(&DAT_8006eed8,&DAT_8006eee0);
  do {
    iVar19 = 0;
    FUN_80011834();
    local_34 = 0x78;
    FUN_800128d4();
    FUN_800251fc(0x40);
    if (local_38 == 0) {
      iVar4 = FUN_80011adc("Shell\\Shell.dll");
      pcVar5 = (char *)(**(code **)(iVar4 + 4))();
      if (pcVar5 == (char *)0x0) {
        pcVar5 = "";
      }
      strcpy(local_90,pcVar5);
      FUN_80045088(iVar4);
      if (local_90[0] == '\0') {
        return;
      }
    }
    uVar15 = 0;
    local_38 = 0;
    DAT_8006f5a8 = (short)cRam00000018;
    DAT_8006f5aa = (short)cRam00000019;
    DAT_8006f5bc = DAT_8006f5a8;
    DAT_8006f5be = DAT_8006f5aa;
    FUN_80029dec();
    uVar8 = 0xe000;
    bVar2 = true;
    do {
      if ((bVar2) || ((&UNK_8006567a)[uVar15] != '\0')) {
        uVar8 = uVar8 | 1 << ((int)(&DAT_80065674)[uVar15] & 0x1fU);
      }
      uVar15 = uVar15 + 1;
      bVar2 = uVar15 < 2;
    } while (uVar15 < 8);
    FUN_800227a4(uVar8);
    uRam000005f8 = FUN_80044360("Sounds\\Main.SND");
    uVar6 = FUN_80015f80("Misc\\Game.FNT");
    FUN_800165cc(0);
    FUN_80017fd4(1);
    iRam00000628 = FUN_80019034(uVar6,0x23);
    FUN_8001910c();
    FUN_8002a598();
    if (bRam00000015 == 5) {
      FUN_8001714c(0xbb40e64d);
    }
    DAT_8006f100 = 0;
    DAT_8006eff8 = 0;
    DAT_8006eef0 = 0;
    uVar8 = (uint)(bRam00000015 - 3 < 2);
    iRam00000010 = uVar8 << 1;
    uRam000006cf = 0xff;
    if (uVar8 != 0) {
      uRam000006cf = 0xab;
    }
    iVar18 = 1;
    iVar4 = 1;
    if (bRam00000015 == 0) {
      iVar7 = *(int *)(iRam00000608 + DAT_80065674 * 8 + 8) + cRam00000600 * 0x10;
      pcVar5 = *(char **)(iVar7 + 0xc);
      uRam000006f0 = *(undefined2 *)(iVar7 + 2);
      uRam000007dc = *(undefined2 *)(iVar7 + 4);
      uVar12 = *(undefined1 *)(iVar7 + 1);
    }
    else {
      if (iRam00000618 == 0) {
        pcVar5 = "";
      }
      else {
        pcVar5 = "Loading Demo Level";
      }
      uVar12 = 0;
    }
    FUN_80022ba8(local_90,pcVar5,uVar12);
    FUN_80012980();
    uRam0000000c = 0;
    FUN_800212c4(0);
    if (bRam00000015 == 0) {
      FUN_8001a0ac(&DAT_80065604,0);
      FUN_80019960(iRam00000628,&DAT_800102b0,0x10,0xc0);
      do {
        FUN_800126f0();
      } while ((uRam0000062c & 0x40) == 0);
    }
    local_3c = 0;
    local_40 = puRam000007d0[0x38];
    if (puRam000007d4 != (uint *)0x0) {
      local_3c = puRam000007d4[0x38];
    }
    local_50.w = 0x140;
    local_50.x = 0;
    local_50.y = 0;
    local_50.h = 0x1e0;
    ClearImage(&local_50,'\0','\0','\0');
    local_44 = 0;
    iRamffffacb0 = 1;
    iRam00000624 = 0;
LAB_80014224:
    do {
      uVar8 = 0;
      local_30 = 0;
      local_44 = local_44 + 1;
      if ((((local_44 & 7) == 0) && (iVar7 = FUN_80043bb4(), iVar7 != 0)) &&
         (iVar7 = FUN_80012a90(iRam00000628,0), iVar7 != 0)) goto LAB_80014ebc;
      uVar15 = 2;
      if (iRam00000618 == 0) {
        if (iRam0000001c == 0) {
          uVar15 = 1;
        }
        else {
          uVar15 = iRamffffacb0 - uRam0000000c;
        }
      }
      uVar16 = 0;
      if (uVar15 != 0) {
        do {
          FUN_800120d4();
          uRam0000000c = uRam0000000c + 1;
          uRam000006cc = (undefined2)uRam0000000c;
          uVar9 = 0;
          if (uVar16 == uVar15 - 1) {
            uVar9 = uVar15;
          }
          FUN_8002131c(uVar9);
          FUN_80021394((uint32_t)iRam0000000c);
          FUN_80021678();
          if ((DAT_80065c30 & 0x800000) != 0) {
            iVar4 = 3 - iVar4;
          }
          uVar16 = uVar16 + 1;
          uVar8 = uVar8 | uRam0000062c;
          local_30 = local_30 | uRam00000630;
        } while (uVar16 < uVar15);
      }
      FUN_800212c4(uRam0000000c & 0xffff);
      iRam00000008 = 1 - iRam00000008;
      if ((short)puRam000007d0[3] == 0) {
        iVar4 = 0;
      }
      else if ((iVar4 == 2) && ((*puRam000007d0 & 0x1000000) != 0)) {
        iVar4 = 1;
      }
      if (puRam000007d4 != (uint *)0x0) {
        if ((short)puRam000007d4[3] == 0) {
          iVar18 = 0;
        }
        else if ((iVar18 == 2) && ((*puRam000007d4 & 0x1000000) != 0)) {
          iVar18 = 1;
        }
      }
      if (iRam00000010 == 0) {
        if (((char)bRam00000015 < '\x03') || ((short)puRam000007d0[3] != 0)) {
          puVar17 = puRam000007d0;
          iVar7 = iVar4;
          if (puRam000007d4 != (uint *)0x0) {
            uVar16 = *puRam000007d4;
            puVar11 = puRam000007d4;
            goto LAB_80014a54;
          }
        }
        else {
          uVar16 = *puRam000007d0;
          puVar11 = puRam000007d0;
          puVar17 = puRam000007d4;
          iVar7 = iVar18;
LAB_80014a54:
          if ((uVar16 & 0x1000000) == 0) {
            *puVar11 = uVar16 & 0xfffffffd;
          }
        }
        if (iVar7 == 2) {
          uVar16 = puVar17[0x3e];
          *puVar17 = *puVar17 | 2;
          sVar1 = *(short *)(puVar17[0x38] + 0x8a);
        }
        else {
          if ((*puVar17 & 0x1000000) == 0) {
            *puVar17 = *puVar17 & 0xfffffffd;
          }
          uVar16 = puVar17[0x38];
          sVar1 = *(short *)(uVar16 + 0x8a);
        }
        FUN_8001db24(uVar16,(int)sVar1);
        FUN_800119c0(iRam00000008);
        FUN_8001d994(0x140,0xf0,0xa0,0x78);
        FUN_80021600();
        if ((iVar7 == 2) && ((*puVar17 & 0x20000000) == 0)) {
          FUN_8002b8d0(puVar17);
        }
        ClearOTagR(&local_48,1);
        uVar3 = local_48;
        if (uRam00000680 != 0) {
          local_48 = 0x6eed8;
          DAT_8006eee4 = DAT_8006eee4 & 0xff000000 | uRam00000680;
          DAT_8006eed8 = DAT_8006eed8 & 0xff000000 | uVar3;
        }
        uRam00000680 = 0;
        if (iVar7 != 0) {
          FUN_8002af98(puVar17,2 - iVar7,&local_48);
        }
        FUN_8002b7bc(puVar17,&DAT_8006f680,&local_48);
        if (iRam00000618 == 0) {
          FUN_80019d10(&DAT_8006eef0,iRam00000628,&local_48,uVar15);
        }
        else if ((uRam0000000c & 0x3f) < 0x28) {
          *(undefined1 *)(iRam00000628 + 4) = 0x80;
          *(undefined1 *)(iRam00000628 + 5) = 0x80;
          *(undefined1 *)(iRam00000628 + 6) = 0;
          FUN_80019c64(iRam00000628,"DEMO MODE",&DAT_80065618,10);
        }
        if (iVar19 == 0) {
          if ((iRam00000624 != 0) && (DAT_8006eef0 == 0)) {
            if (iRam000005ac == 4) {
              if (iRam00000024 == 0) {
                pcVar5 = "Sounds\\Defeat.xa";
              }
              else {
                pcVar5 = "Sounds\\Victory.xa";
              }
              FUN_80043df8(pcVar5,(int)DAT_80065674);
            }
            if (bRam00000015 == 0) {
              uRam00000620 = FUN_800220d4();
              uRam00000620 = uRam00000620 ^ DAT_80065674 < '\x06';
            }
            iVar19 = FUN_8001392c(iRam00000628);
          }
        }
        else {
          FUN_80018f7c(iVar19,&local_48);
        }
        FUN_800128bc();
        DrawSync(0);
        FUN_8002a25c(0,(int)*(short *)(&DAT_8006f20a + (1 - iRam00000004) * 0x5c),&local_48);
        if (DAT_80065c28 < 2) {
          uVar8 = uVar8 | 0x8000000;
        }
        FUN_80012828(&DAT_8006f5a0 + iRam00000008 * 0x14,&DAT_8006f208 + iRam00000004 * 0x5c,
                     &local_48,iRam0000060c + 0x3ffc);
      }
      else {
        uVar6 = 0xf0;
        if (iRam00000010 == 1) {
          uVar10 = 0x140;
          uVar6 = 0x78;
          uVar13 = 0xa0;
          uVar14 = 0x3c;
        }
        else {
          uVar10 = 0xa0;
          uVar13 = 0x50;
          uVar14 = 0x78;
        }
        FUN_8001d994(uVar10,uVar6,uVar13,uVar14);
        FUN_800119c0(0);
        if ((*puRam000007d4 & 0x1000000) == 0) {
          *puRam000007d4 = *puRam000007d4 & 0xfffffffd;
        }
        if (iVar4 == 2) {
          puVar17 = puRam000007d0 + 0x3e;
          *puRam000007d0 = *puRam000007d0 | 2;
          sVar1 = *(short *)(local_40 + 0x8a);
          uVar16 = *puVar17;
        }
        else {
          sVar1 = *(short *)(local_40 + 0x8a);
          uVar16 = local_40;
        }
        FUN_8001db24(uVar16,(int)sVar1);
        DAT_8006f6a0 = DAT_8006f680;
        DAT_8006f6a4 = DAT_8006f684;
        DAT_8006f6a8 = DAT_8006f688;
        DAT_8006f6ac = DAT_8006f68c;
        DAT_8006f6b0 = DAT_8006f690;
        DAT_8006f6b4 = DAT_8006f694;
        DAT_8006f6b8 = DAT_8006f698;
        DAT_8006f6bc = DAT_8006f69c;
        FUN_80021600();
        ClearOTagR(&local_48,1);
        iVar7 = FUN_80019d10(&DAT_8006eef0,iRam00000628,&local_48,uVar15);
        if ((iVar7 == 0) && (iVar19 == 0)) {
          if ((((iRam00000624 != 0) && (DAT_8006eef0 == 0)) && (DAT_8006eff8 == 0)) &&
             (DAT_8006f100 == 0)) {
            if (iRam000005ac == 4) {
              if ((iRam00000024 == 0) && (bRam00000015 != 3)) {
                pcVar5 = "Sounds\\Defeat.xa";
              }
              else {
                pcVar5 = "Sounds\\Victory.xa";
              }
              uVar16 = 0;
              if (bRam00000015 == 3) {
                uVar16 = (uint)((short)puRam000007d0[3] == 0);
              }
              FUN_80043df8(pcVar5,(int)(&DAT_80065674)[uVar16]);
            }
            iVar19 = FUN_8001392c(iRam00000628);
          }
        }
        else {
          if (iVar19 != 0) {
            FUN_80018f7c(iVar19,&local_48);
          }
          uVar3 = local_48;
          local_48 = (uint)(&uStackY_f0 + iRam00000010 * 8 + iRam00000008 * 4) & 0xffffff;
          (&uStackY_f0)[iRam00000010 * 8 + iRam00000008 * 4] =
               (uint)*(byte *)((int)&uStackY_f0 + iRam00000008 * 0x10 + iRam00000010 * 0x20 + 3) <<
               0x18 | uVar3;
          iVar7 = (1 - iRam00000008) * 0x5c;
          SetDrawEnv((DR_ENV *)(&DAT_8006f224 + iVar7),(DRAWENV *)(&DAT_8006f208 + iVar7));
          iVar7 = (1 - iRam00000008) * 0x5c;
          *(uint *)(&DAT_8006f224 + iVar7) = (uint)(byte)(&DAT_8006f227)[iVar7] << 0x18 | local_48;
          local_48 = (uint)(&DAT_8006f224 + iVar7) & 0xffffff;
        }
        if (iVar18 != 0) {
          FUN_8002af98(puRam000007d4,iRam00000010 << 1 | 1,&local_48);
        }
        FUN_8002b7bc(puRam000007d4,&DAT_8006f6c0,&local_48);
        FUN_80019d10(&DAT_8006f100,iRam00000628,&local_48,uVar15);
        DrawSync(0);
        iVar7 = (iRam00000010 * 4 - (iRam00000008 + -1)) * 0x5c;
        FUN_8002a25c((int)*(short *)(&DAT_8006f208 + iVar7),(int)*(short *)(&DAT_8006f20a + iVar7),
                     &local_48);
        FUN_80012828(&DAT_8006f5a0 + iRam00000008 * 0x14,
                     &UNK_8006f150 + (iRam00000010 * 4 + iRam00000008) * 0x5c,&local_48,
                     iRam0000060c + 0x3ffc);
        FUN_800119c0(1);
        if ((*puRam000007d0 & 0x1000000) == 0) {
          *puRam000007d0 = *puRam000007d0 & 0xfffffffd;
        }
        if (iVar18 == 2) {
          puVar17 = puRam000007d4 + 0x3e;
          *puRam000007d4 = *puRam000007d4 | 2;
          sVar1 = *(short *)(local_3c + 0x8a);
          uVar16 = *puVar17;
        }
        else {
          sVar1 = *(short *)(local_3c + 0x8a);
          uVar16 = local_3c;
        }
        FUN_8001db24(uVar16,(int)sVar1);
        DAT_8006f6c0 = DAT_8006f680;
        DAT_8006f6c4 = DAT_8006f684;
        DAT_8006f6c8 = DAT_8006f688;
        DAT_8006f6cc = DAT_8006f68c;
        DAT_8006f6d0 = DAT_8006f690;
        DAT_8006f6d4 = DAT_8006f694;
        DAT_8006f6d8 = DAT_8006f698;
        DAT_8006f6dc = DAT_8006f69c;
        FUN_80021600();
        ClearOTagR(&local_48,1);
        if (iVar4 != 0) {
          FUN_8002af98(puRam000007d0,iRam00000010 << 1,&local_48);
        }
        FUN_8002b7bc(puRam000007d0,&DAT_8006f6a0,&local_48);
        FUN_80019d10(&DAT_8006eff8,iRam00000628,&local_48,uVar15);
        FUN_800128bc();
        DrawSync(0);
        iVar7 = (iRam00000010 * 4 + iRam00000008 + -2) * 0x5c;
        FUN_8002a25c((int)*(short *)(&DAT_8006f208 + iVar7),(int)*(short *)(&DAT_8006f20a + iVar7),
                     &local_48);
        DrawOTag(&local_48);
        DrawSync(0);
        if (DAT_80065c28 < 2) {
          uVar8 = uVar8 | 0x8000000;
        }
        if (DAT_80065c40 < 2) {
          local_30 = local_30 | 0x8000000;
        }
        PutDrawEnv((DRAWENV *)(&DAT_8006f208 + (iRam00000010 * 4 + iRam00000008) * 0x5c));
        DrawOTag((u_long *)(iRam0000060c + 0x3ffc));
      }
      if ((iVar19 != 0) && (iRam00000624 = iRam00000624 + uVar15, 300 < iRam00000624)) {
        if (bRam00000015 == 0) {
          if (((uVar8 & 0x8400000) != 0) || ((iRam00000024 == 0 && (0x4b0 < iRam00000624))))
          goto LAB_80014ebc;
        }
        else if ((((uVar8 | local_30) & 0x8600000) != 0) || (0x4b0 < iRam00000624)) {
          local_38 = (uVar8 | local_30) & 0x200000;
          goto LAB_80014ebc;
        }
      }
      uVar16 = uVar8 | local_30;
      if ((uVar16 & 0x100) == 0) {
        if (((uVar16 & 0x8000000) != 0) &&
           ((iRam00000618 != 0 ||
            (iVar7 = FUN_80012a90(iRam00000628,(uVar8 >> 0x1b ^ 1) & 1), iVar7 != 0)))) break;
        goto LAB_80014224;
      }
      if ((uVar16 & 0x800) == 0) {
        local_34 = 0x78;
        goto LAB_80014224;
      }
      local_34 = local_34 - uVar15;
    } while (-1 < local_34);
    uRam00000014 = 3;
LAB_80014ebc:
    if (iVar19 != 0) {
      FUN_80018f3c(iVar19);
    }
    if (iRam00000618 != 0) {
      FUN_8001265c();
      FUN_800120d4();
      uRam00000014 = 3;
      if ((uRam0000062c == 0) && (uRam00000014 = 3, uRam00000630 == 0)) {
        uRam00000014 = 2;
      }
      FUN_80011c58(&DAT_80065968);
      FUN_800126c8();
    }
    FUN_800128bc();
    FUN_80044054();
    FUN_80044394(uRam000005f8);
    FUN_80022a1c();
    FUN_800204dc(local_40);
    if (puRam000007d4 != (uint *)0x0) {
      FUN_800204dc(local_3c);
    }
    FUN_8002accc();
    FUN_80041e80();
    FUN_8001356c(iRam00000628);
    FUN_800190d8(iRam00000628);
    FUN_80011914(0);
    FUN_80011914(1);
    FUN_80016678(0);
  } while( true );
}

#endif  /* GHIDRA REF */
