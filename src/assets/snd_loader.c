/* snd_loader.c -- SND sound bank loader (open / load / close pattern).
 *
 * Source: SLUS_005.10
 *   FUN_80044360  -- Audio_LoadSND  -- thin {open, read-payload, close} wrapper.
 *   FUN_800441f8  -- Audio_ParseSND -- the bank parser (in snd_parse.c)
 *   FUN_80044394  -- Audio_FreeSND  -- SpuFree(handle's sample bytes) + Heap_Free(handle)
 *
 * The bank handle layout (recovered from FUN_80044394):
 *   { u16 ???, u16 sampleSizeIn8b, ... }
 *      -- the SPU allocation length is *(u16 *)(handle+2) << 3.
 *
 * HIGH.
 */
#include <stdint.h>

extern void Stream_OpenByName(uint32_t pathHandle);          /* FUN_800159b4 */
extern void Stream_Close(void);                              /* FUN_80015a00 */
extern void *Audio_ParseSND(void);                           /* FUN_800441f8 */
extern void  SpuFree(uint32_t spuAddr);
extern void  Heap_Free(void *p);

void *Audio_LoadSND(uint32_t pathHandle)
{
    Stream_OpenByName(pathHandle);
    void *bank = Audio_ParseSND();
    Stream_Close();
    return bank;
}

void Audio_FreeSND(void *handle)
{
    uint8_t *h = (uint8_t *)handle;
    /* SPU buffer length stored in 8-byte units at +2. */
    SpuFree((uint32_t)*(uint16_t *)(h + 2) << 3);
    Heap_Free(handle);
}
