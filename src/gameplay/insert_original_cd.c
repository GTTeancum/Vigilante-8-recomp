/* insert_original_cd.c -- the "Insert Original CD" loop.
 *
 * Source: SLUS_005.10  FUN_8001356c.
 *
 * Called by the main loop whenever the disc-state global is anything
 * other than 4 (= "PSX disc with audio tracks"). Displays:
 *
 *   "Insert Original CD" until the CD cover opens (CdControlB poll
 *      via FUN_80043bb4 returns bit set)
 *   "CD COVER OPEN" until the cover closes again
 *   "CHECKING CD" once and re-detects via CD_DetectDiskType
 *
 * The loop exits once the correct disc is identified (uRam000005ac == 4).
 *
 * Bit-exact: the busy-spin on VSync + cover-open polling is preserved
 * verbatim. The fixed UI position at DAT_80065520/8 keeps the message
 * box layout identical to the original.
 *
 * Renderer-adjacent: the Font_DrawText / Layout calls are on the
 * out-of-scope renderer seam. The new audio/UI layer must reproduce
 * the same "block until correct disc" flow.
 *
 * HIGH (control flow); MED (UI offsets unverified).
 */
#include <stdint.h>

extern void  PutDrawEnv(void *e);
extern void  VSync(int mode);
extern void  Layout_AlignNext(void *xy, int mode);    /* FUN_8001a0ac */
extern void  Font_SetTint(void *font, uint32_t rgb);  /* FUN_80019f9c */
extern void  Font_SetColor(int font, uint16_t mode);  /* FUN_80019010 */
extern void  Font_DrawText(int font, const char *s, void *xy, int align);  /* FUN_80019a58 */
extern uint8_t CD_IsCoverOpen(void);                  /* FUN_80043bb4 */
extern uint8_t CD_DetectDiskType(void);                /* FUN_80043aec */
extern int32_t  iRam00000008;
extern uint8_t  uRam000005ac;
extern uint8_t  DAT_8006f208[];
extern uint8_t  DAT_80065520[];
extern uint8_t  DAT_80065528[];

void InsertOriginalCd_Loop(int fontHandle)
{
    while (uRam000005ac != 4) {
        PutDrawEnv(DAT_8006f208 + (1 - iRam00000008) * 0x5c);
        Layout_AlignNext(DAT_80065520, 0);
        Font_SetTint(DAT_80065520, 0xffffff);
        *(uint8_t *)(fontHandle + 4) = 0x80;
        *(uint8_t *)(fontHandle + 5) = 0x80;
        *(uint8_t *)(fontHandle + 6) = 0;
        Font_SetColor(fontHandle, 1);
        Font_DrawText(fontHandle, "Insert Original CD", DAT_80065528, 10);
        do { VSync(0); } while (CD_IsCoverOpen() == 0);

        Layout_AlignNext(DAT_80065528, 0);
        Font_DrawText(fontHandle, "CD COVER OPEN", DAT_80065528, 10);
        while (CD_IsCoverOpen() != 0) { /* wait for close */ }

        Layout_AlignNext(DAT_80065528, 0);
        Font_DrawText(fontHandle, "CHECKING CD", DAT_80065528, 10);
        CD_DetectDiskType();
    }
}
