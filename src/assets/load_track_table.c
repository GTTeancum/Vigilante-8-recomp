/* load_track_table.c -- read CD audio track-position table from disc.
 *
 * Source: LOAD.DLL  FUN_801002ac.
 *
 * The Shell\Load.tbl payload contains a header followed by an array of
 * i16 entries representing CD-DA track positions used by the music
 * sequencer. The first five i16s are header values (track count,
 * format hint, options...), and from the sixth i16 onward each entry
 * is a per-track sector offset.
 *
 * The function reads the header into five locals (only one of which
 * is currently used), then loops over `(size - 10) / 2` entries
 * (i.e. all remaining i16s after the 10-byte header) into the global
 * track table at DAT_800658e8.
 *
 * HIGH confidence on shape; MED on the meaning of the 5 header fields.
 */
#include <stdint.h>

extern int16_t XobfStream_ReadI16(void *st);   /* func_0x800224b4 */
extern int16_t DAT_800658e8[];                  /* track-position table */

void Audio_LoadTrackTable(void *stream, int payloadSize)
{
    /* 10-byte header (5 i16s) -- parse-and-discard for now. */
    int16_t h0 = XobfStream_ReadI16(stream);
    int16_t h1 = XobfStream_ReadI16(stream);
    int16_t h2 = XobfStream_ReadI16(stream);
    int16_t h3 = XobfStream_ReadI16(stream);
    int16_t h4 = XobfStream_ReadI16(stream);
    (void)h0; (void)h1; (void)h2; (void)h3; (void)h4;

    int n = (payloadSize - 10) / 2;
    for (int i = 0; i < n; i++) {
        DAT_800658e8[i] = XobfStream_ReadI16(stream);
    }
}
