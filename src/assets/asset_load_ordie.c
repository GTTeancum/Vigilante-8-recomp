/* asset_load_ordie.c -- Asset_LoadFile + fatal trap on failure.
 *
 * Source: SLUS_005.10  FUN_80015f80.
 *
 * The "named" variant of Asset_LoadFile. Callers latch the path name
 * via Iso_LatchPathArg(name) (or pass the name in as the first arg --
 * Ghidra loses argument visibility for thin one-liner wrappers); this
 * function calls the no-arg Asset_LoadFile and triggers the splash
 * fatal trap if the load returns NULL.
 *
 * HIGH confidence.
 */
#include <stdint.h>

extern void *Asset_LoadFile(void);
extern void  Stream_FatalOom(const char *msg);

void *Asset_LoadFileOrDie(const char *pathArg)
{
    void *p = Asset_LoadFile();
    if (p == NULL) Stream_FatalOom(pathArg);
    return p;
}
