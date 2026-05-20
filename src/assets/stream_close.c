/* stream_close.c -- close the active streaming CD reader.
 *
 * Source: SLUS_005.10  FUN_80015798.
 *
 * Issues CdControl(CdlPause), unhooks the CdReadyCallback, and frees
 * the 2 KiB sector ring buffer previously allocated by Stream_Open.
 *
 * HIGH confidence: standard PSY-Q CD streamer teardown.
 */
#include <stdint.h>

extern void  CdControl(unsigned char com, unsigned char *param, unsigned char *result);
extern void  CdReadyCallback(void (*cb)(int, unsigned char *));
extern void  Heap_Free(void *p);
extern void *uRam000006a0;   /* sector buffer base set by Stream_Open */

#define CdlPause  0x09

void Stream_Close(void)
{
    CdControl(CdlPause, (unsigned char *)0, (unsigned char *)0);
    CdReadyCallback((void (*)(int, unsigned char *))0);
    Heap_Free(uRam000006a0);
}
