/* shell_aliases.c -- map decomp-friendly names to their real impls.
 *
 * Several functions got two names during the decomp: a high-level
 * one (Quest_Load) used in src/assets/, and a context-specific one
 * (Shell_PreEnter) used in src/gameplay/main_loop.c. Both point at
 * FUN_800128d4. We resolve them by aliasing here so the panic-stub
 * generator doesn't have to know about every duplicate.
 */
#include <stdint.h>

extern void Quest_Load(void);

void Shell_PreEnter(void) { Quest_Load(); }
