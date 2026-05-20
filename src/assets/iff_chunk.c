/* iff_chunk.c -- read an EA-IFF (FORM) chunk header from the stream.
 *
 * Source: SLUS_005.10  FUN_80022524.
 *
 * Reads an 8-byte IFF chunk header { fourcc; big-endian size }, byte-swaps
 * the size to little-endian, decrements the caller's "bytes remaining"
 * counter by (header + payload). Returns 0xffffffff if the chunk is a
 * "FORM" (in which case it also consumes the trailing 4cc form type),
 * else returns the byte-swapped size.
 *
 * This is the central reader used by every EXP/EXP-derived archive
 * (TERR, XOBF, XLSC, BIN, HEAD subchunks). Pass 2 should layer a
 * proper IFF walker on top of it.
 *
 * HIGH confidence.
 *
 * Layout convention (network/BE on disc, host LE in memory):
 *
 *   on disc:
 *     u8[4]  fourcc
 *     u32_BE size
 *     u8[size] payload
 *     <even-pad byte if size is odd>     -- the +1 then &~1 below
 *
 *   in memory after reading:
 *     u32   fourcc            (LE, so "FORM" reads as 'M''R''O''F' = 0x4d524f46)
 *     u32   size              (LE, swapped)
 */
#include <stdint.h>

extern void Stream_Read(void *dst, uint32_t nBytes);

#define FOURCC_FORM_LE   0x4d524f46u   /* "FORM" with LE byteswap */

uint32_t Iff_ReadChunkHeader(uint32_t *hdrOut, uint32_t *parentRemaining)
{
    Stream_Read(hdrOut, 8);
    uint32_t sz = hdrOut[1];
    sz = (sz >> 24) | ((sz >> 8) & 0xff00u) | ((sz & 0xff00u) << 8) | (sz << 24);
    hdrOut[1] = sz;

    /* Round odd sizes up to even (IFF pads to 16-bit alignment), then
     * subtract from the caller's "bytes remaining" alongside the 8 B
     * we just consumed. */
    uint32_t padded = (sz + 1u) & 0xfffffffeu;
    *parentRemaining = *parentRemaining - 8u - padded;

    if (hdrOut[0] == FOURCC_FORM_LE) {
        Stream_Read(hdrOut, 4);          /* read form type 4cc */
        hdrOut[1] -= 4;                  /* (size in caller's tracking) */
        return 0xffffffffu;              /* sentinel: caller knows it's a FORM */
    }
    return padded;
}
