/* splash.c -- boot splash + fatal-error trap.
 *
 * Source: SLUS_005.10
 *   FUN_80015164 -- Splash_InitDisplay
 *   FUN_80015288 -- Splash_DrawLine
 *   FUN_80015368 -- Stream_FatalOom (and friends)
 *
 * Splash_InitDisplay sets up the studio-logo display: low-res 320x240
 * (or 640x480 hi-res when `hires` is non-zero), loads `Misc\Kong.fnt`,
 * allocates an atlas, and clears the screen via SetDefDispEnv /
 * SetDefDrawEnv with dfe=1 (clear-on-flip), tpage=0x20.
 *
 * Splash_DrawLine appends a line of text to the splash screen, tracking
 * line height in iRam00000688/0x68c/0x690 and wrapping at iRam00000694.
 *
 * Stream_FatalOom (FUN_80015368) is the "Out of SPU RAM" / general
 * fatal trap: it sets up the splash display and dies in an infinite
 * loop. Functions in the asset pipeline call it when they can't
 * recover.
 *
 * MED confidence: control flow is clear; field names for the per-line
 * tracker (line height, padding) are inferred.
 */
#include <stdint.h>

extern void  PutDrawEnv(void *drawEnv);
extern void  PutDispEnv(void *dispEnv);
extern void  SetDispMask(int mode);
extern void *SetDefDispEnv(void *de, int x, int y, int w, int h);
extern void *SetDefDrawEnv(void *de, int x, int y, int w, int h);
extern void *Asset_LoadFile(void);            /* FUN_80015948 */
extern uint32_t Font_AllocAtlas(void *fnt, int slot);  /* FUN_80019034 */
extern void  Iso_LatchPathArg(const char *p);          /* sets the next Iso_OpenPath name */
extern void  Splash_AlignNext(void *xy, int mode);     /* FUN_8001a0ac */

extern uint32_t uRam00000684;          /* font atlas handle */
extern int32_t  iRam00000688;          /* current Y */
extern int32_t  iRam0000068c;          /* current X */
extern int32_t  iRam00000690;          /* current line max-height */
extern int32_t  iRam00000694;          /* y limit */

typedef struct { int16_t x, y; int16_t w, h; } DRAWENV_min;
typedef struct { int16_t x, y; int16_t w, h; } DISPENV_min;

void Splash_InitDisplay(int hires)
{
    int16_t w = 0x140, h = 0xf0;
    if (hires) { w = 0x280; h = 0x1e0; }

    extern void Render_SetTexpageMode(int mode);   /* FUN_80017fd4 */
    Render_SetTexpageMode(0);

    uint32_t fnt = (uint32_t)Asset_LoadFile();    /* "Misc\\Kong.fnt" -- caller latches name */
    uRam00000684 = Font_AllocAtlas((void *)fnt, 0);

    uint8_t  disp[0x14];
    uint8_t  draw[0x5c];
    SetDefDispEnv(disp, 0, 0, w, h);
    SetDefDrawEnv(draw, 0, 0, w, h);
    *(int8_t *)(draw + 5) = 1;          /* dfe (clear-on-display-flip) */
    *(int16_t *)(draw + 6) = 0x20;      /* tpage */
    PutDrawEnv(draw);
    PutDispEnv(disp);

    int16_t xy[2] = { 0, 0 };
    Splash_AlignNext(&xy, 0);
    iRam00000694  = h - 0x20;
    SetDispMask(1);
    iRam0000068c  = 0x20;
    iRam00000688  = 0x20;
    iRam00000690  = 0;
}

extern void Font_DrawText(void *font, const char *s, int x, int y);   /* FUN_80019960 */
extern int  Font_MeasureText(void *font, const char *s);              /* FUN_80019138 */
extern uint8_t *piRam00000684;

void Splash_DrawLine(const char *s)
{
    Font_DrawText(piRam00000684, s, iRam00000688, iRam0000068c);
    int line = Font_MeasureText(piRam00000684, s);
    if (line < iRam00000690) line = iRam00000690;
    iRam0000068c += *(uint8_t *)(*(uintptr_t *)piRam00000684 + 6);  /* line height */
    iRam00000690 = line;
    if (iRam0000068c + *(uint8_t *)(*(uintptr_t *)piRam00000684 + 6) > iRam00000694) {
        iRam0000068c = 0x20;
        iRam00000690 = 0;
        iRam00000688 += 8 + line;
    }
}

/* Stream_FatalOom: print the error and freeze. The "do{}while(true)"
 * preserves the binary's intentional spin -- in a release build the
 * user sees the message and the system is hung. */
void Stream_FatalOom(const char *msg)
{
    Splash_InitDisplay(0);
    Splash_DrawLine(msg);
    for (;;) { /* hang */ }
}
