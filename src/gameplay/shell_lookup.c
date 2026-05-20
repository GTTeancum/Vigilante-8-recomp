/* shell_lookup.c -- SHELL.DLL lookup-table allocation pair.
 *
 * Source: SHELL.DLL  FUN_8010a614 / FUN_8010a6c4.
 *
 * Allocates two zero-filled scratch tables for shell-screen lookups:
 *   DAT_801133a0: 0x28 * 0x20 = 1280 bytes  (40 entries of 32 bytes)
 *   DAT_801133a4: 0x200 * 0x20 = 16384 bytes (512 entries of 32 bytes)
 *
 * The teardown function (8010a6c4) frees both. Clear-on-allocate +
 * paired free is the standard "shell-screen scratch buffer" idiom.
 *
 * HIGH.
 */
#include <stdint.h>

extern void *Heap_CallocOrRetry(int count, int size);
extern void  Heap_Free(void *p);
extern void *DAT_801133a0;
extern void *DAT_801133a4;
extern uint8_t DAT_801133b0;
extern uint8_t DAT_801133b1;

void Shell_AllocLookupTables(void)
{
    DAT_801133b1 = 0;
    DAT_801133b0 = 0;
    DAT_801133a0 = Heap_CallocOrRetry(0x28, 0x20);
    DAT_801133a4 = Heap_CallocOrRetry(0x200, 0x20);
}

void Shell_FreeLookupTables(void)
{
    Heap_Free(DAT_801133a0);
    Heap_Free(DAT_801133a4);
}
