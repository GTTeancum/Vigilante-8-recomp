/* snd_parse.c -- SND (sound bank) parser.
 *
 * Source: SLUS_005.10  FUN_800441f8.
 *
 * SND on-disc layout (recovered):
 *
 *   u16  nSamples         -- count of entries
 *   u16  sizeIn8b         -- total sample data length in 8-byte units
 *   u32  sampleOffsets[nSamples]  -- offsets within the bank's SPU
 *                                    region (in 8-byte units)
 *   u8   sampleData[sizeIn8b * 8] -- VAG/ADPCM-encoded sample payload
 *
 * The parser:
 *   1. Reads (nSamples, sizeIn8b) from the stream.
 *   2. SpuMalloc()'s `sizeIn8b*8 + 0x3f` rounded down to 64 bytes for
 *      the on-SPU buffer. OOM trips a fatal trap (Stream_FatalOom).
 *   3. Heap-allocates a small in-RAM handle of (nSamples*4 + 4) bytes:
 *        u16  nSamples;
 *        u16  spuBaseIn8b;
 *        u32  sampleAbsOffsets[nSamples];  -- 8-byte units, post-add-base
 *   4. Reads the offset table (nSamples u32s).
 *   5. Adds spuBaseIn8b to each offset to convert per-sample relative
 *      offsets into absolute SPU 8b-unit positions.
 *   6. Streams sample data into SPU RAM 1 KiB at a time via SpuWrite,
 *      waiting on SpuIsTransferCompleted between chunks.
 *
 * HIGH-MED confidence: layout is unambiguous from the read sequence;
 * the +0x3f / &~0x3f rounding is the standard SPU 64-byte alignment.
 *
 * The original calls the misnomered Stream_Read of the 1024-byte
 * sample chunk into auStack_418 BEFORE setting the SPU transfer
 * address. We preserve that order; it is observable only if SpuWrite
 * is preemptable on real hardware (it isn't), so it's incidental.
 */
#include <stdint.h>

extern void Stream_Read(void *dst, uint32_t nBytes);
extern uint32_t SpuMalloc(uint32_t bytes);
extern void SpuSetTransferMode(int mode);
extern void SpuSetTransferStartAddr(uint32_t addr);
extern int  SpuRead(const void *buf, uint32_t bytes);   /* misnamed: SPU read = host->SPU write */
extern int  SpuIsTransferCompleted(int mode);
extern void Stream_FatalOom(const char *msg);            /* FUN_80015368 trap */
extern void *Heap_AllocOrRetry(uint32_t n);

typedef struct {
    uint16_t nSamples;
    uint16_t spuBaseIn8b;
    uint32_t sampleOff[1];   /* nSamples entries */
} SndBank;

void *Audio_ParseSND(void)
{
    /* Header: 2 u16s. */
    struct { uint16_t nSamples; uint16_t sizeIn8b; } hdr;
    Stream_Read(&hdr, 4);

    uint32_t spuBytes = (uint32_t)hdr.sizeIn8b * 8;
    uint32_t spuAlloc = (spuBytes + 0x3fu) & ~0x3fu;
    uint32_t addr     = SpuMalloc(spuAlloc);
    if (addr == 0) Stream_FatalOom("Out of SPU RAM");

    SndBank *bank = (SndBank *)Heap_AllocOrRetry((uint32_t)hdr.nSamples * 4 + 4);
    bank->nSamples    = hdr.nSamples;
    bank->spuBaseIn8b = (uint16_t)(addr >> 3);

    /* Offset table. */
    Stream_Read(bank->sampleOff, (uint32_t)hdr.nSamples * 4);
    for (int i = 0; i < hdr.nSamples; i++) {
        bank->sampleOff[i] += bank->spuBaseIn8b;
    }

    /* Sample payload -> SPU RAM in 1 KiB chunks. */
    SpuSetTransferMode(0);
    SpuSetTransferStartAddr(addr);
    uint32_t left = spuBytes;
    while (left != 0) {
        uint32_t chunk = (left < 0x400u) ? left : 0x400u;
        uint8_t  buf[0x400];
        Stream_Read(buf, chunk);
        SpuSetTransferStartAddr(addr);
        SpuRead(buf, chunk);
        while (SpuIsTransferCompleted(0) == 0) { /* wait */ }
        addr += chunk;
        left -= chunk;
    }
    return bank;
}
