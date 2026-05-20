/* shell_misc.c -- tiny SHELL.DLL helpers cleaned in pass 2.
 *
 * Source: SHELL.DLL
 *   FUN_80102174  -- ShellEvent_Empty (placeholder stub).
 *   FUN_801021a0  -- ShellEvent_Empty2 (another stub).
 *   FUN_801021d4  -- Shell_WaitForStartPress -- spin-poll Pad_Tick
 *                    until the global event bitmask DAT_80065c28
 *                    crosses 2 (the "Start pressed" sentinel).
 *   FUN_8010223c  -- Shell_RealignCursor (calls Layout_AlignNext on
 *                    DAT_8010043c, a fixed UI screen-coord pair).
 *   FUN_8010754c  -- ShellEvent_Empty3 (third empty placeholder).
 *
 * The empty functions are deliberately empty in the original -- V8
 * uses a "function table of event handlers" pattern, and the empties
 * are no-op slots reserved for events that didn't ship with handlers.
 *
 * HIGH confidence on the no-ops, MED on the spin (`DAT_80065c28 < 2`
 * threshold meaning).
 */
#include <stdint.h>

extern void  Pad_Tick(void);                  /* FUN_800120d4 (main EXE) */
extern void  Layout_AlignNext(void *xy, int mode);   /* FUN_8001a0ac */
extern int16_t DAT_80065c28;
extern int16_t DAT_8010043c[2];

void ShellEvent_Empty (void) { /* no-op slot */ }
void ShellEvent_Empty2(void) { /* no-op slot */ }
void ShellEvent_Empty3(void) { /* no-op slot */ }
void ShellEvent_Empty4(void) { /* no-op slot @ 0x80101d6c */ }

void Shell_WaitForStartPress(void)
{
    Pad_Tick();
    while (DAT_80065c28 < 2) {
        Pad_Tick();
    }
}

void Shell_RealignCursor(void)
{
    Layout_AlignNext(DAT_8010043c, 0);
}
