/* slogan_screen.c -- show a random slogan via Shell\Slogan.fnt.
 *
 * Source: SHELL.DLL  FUN_8010238c.
 *
 * The slogan screen displayed during boot / between matches. Steps:
 *   1. SetDispMask(0)            -- blank the screen
 *   2. Vram_BinPacker_Reset      -- clear the VRAM atlas
 *   3. Asset_LoadFileOrDie       -- load Shell\Slogan.fnt
 *   4. Font_AllocAtlas(fnt, 0)
 *   5. V8_RandNext()             -- random slogan index
 *   6. lookup at PTR_DAT_80111d6c[(rand * 11) >> 15]  -- one of 11 slogans
 *   7. Font_DrawText
 *   8. SetDispMask(1,...) with a window box for the text region
 *   9. Wait for any button (poll Pad_Tick until input mask hits 0x840)
 *  10. Font_FreeAtlas
 *
 * The `* 11` then `>> 15` is the standard V8 "scale a 15-bit rand
 * sample into a small enum index" idiom -- since V8_RandNext returns
 * 0..0x7fff, `(r * 11) >> 15` evenly distributes across 11 slogans.
 *
 * HIGH-MED confidence.
 */
#include <stdint.h>

extern void  SetDispMask(int mode, ...);
extern void  Vram_BinPacker_Reset(void);                /* func_0x80019e7c */
extern void *Asset_LoadFileOrDie(const char *path);     /* FUN_80015f80 */
extern uint32_t Font_AllocAtlas(void *fnt, int slot);   /* FUN_80019034 */
extern uint32_t V8_RandNext(void);
extern void  Font_DrawText(uint32_t font, const char *s, void *xy, int align);  /* FUN_80019a58 */
extern void  Pad_Tick(void);
extern void  Font_FreeAtlas(uint32_t font);             /* FUN_800190d8 */

extern const char *PTR_DAT_80111d6c[];   /* 11-slot slogan strings */
extern uint8_t     DAT_8010059c[];        /* layout xy for slogan text */
extern uint32_t    _DAT_80065930;
extern uint32_t    _DAT_80065934;

#define ANY_BUTTON_MASK   0x840u  /* X | Start */

void Slogan_Show(void)
{
    SetDispMask(0);
    Vram_BinPacker_Reset();

    void *fnt = Asset_LoadFileOrDie("Shell\\Slogan.fnt");
    uint32_t atlas = Font_AllocAtlas(fnt, 0);

    uint32_t r = V8_RandNext();
    const char *line = PTR_DAT_80111d6c[(r * 11u) >> 15];
    Font_DrawText(atlas, line, DAT_8010059c, 2);

    SetDispMask(1, 0, 0x60, 0x70, 0x60, 0x70, 0x1c0, 0xa0);

    do {
        Pad_Tick();
    } while (((_DAT_80065930 | _DAT_80065934) & ANY_BUTTON_MASK) == 0);

    Font_FreeAtlas(atlas);
}
