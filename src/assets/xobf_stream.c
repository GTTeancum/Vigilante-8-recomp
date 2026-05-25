/* xobf_stream.c -- in-memory big-endian stream readers.
 *
 * Source: SLUS_005.10
 *   FUN_8002249c -- XobfStream_ReadU8
 *   FUN_800224b4 -- XobfStream_ReadI16
 *   FUN_800224ec -- XobfStream_ReadI32
 *
 * The caller passes a pointer to its cursor variable.  Each helper reads
 * from *cursor, advances it, and returns a big-endian value.
 *
 * HIGH confidence: exact leaf functions.
 */
#include <stdint.h>

uint8_t XobfStream_ReadU8(void *streamRef)
{
    uint8_t **cursor = (uint8_t **)streamRef;
    uint8_t *p = *cursor;
    uint8_t v = *p;
    *cursor = p + 1;
    return v;
}

int16_t XobfStream_ReadI16(void *streamRef)
{
    uint16_t hi = XobfStream_ReadU8(streamRef);
    uint16_t lo = XobfStream_ReadU8(streamRef);
    return (int16_t)((hi << 8) | lo);
}

int32_t XobfStream_ReadI32(void *streamRef)
{
    uint32_t hi = (uint16_t)XobfStream_ReadI16(streamRef);
    uint32_t lo = (uint16_t)XobfStream_ReadI16(streamRef);
    return (int32_t)((hi << 16) | lo);
}

/* Hex-address aliases for generated references. */
uint8_t FUN_8002249c(void *streamRef) { return XobfStream_ReadU8(streamRef); }
int16_t FUN_800224b4(void *streamRef) { return XobfStream_ReadI16(streamRef); }
int32_t FUN_800224ec(void *streamRef) { return XobfStream_ReadI32(streamRef); }
