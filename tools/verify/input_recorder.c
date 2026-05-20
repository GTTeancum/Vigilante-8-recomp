/* input_recorder.c -- TAS-style frame-by-frame input log.
 *
 * The contract: the abstract input struct (the seam between the new
 * control layer and the original gameplay code) is recorded per tick
 * and replayed verbatim. Once the input struct is recovered in pass 2,
 * this writer/reader becomes round-trip exact.
 *
 * File format (little-endian):
 *   magic       4    'V8IL'
 *   version     4    1
 *   nFrames     4
 *   inputSize   4    bytes per frame (sizeof V8InputCommand)
 *   followed by  nFrames * inputSize bytes
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define V8_IL_MAGIC    0x4c493856u   /* 'V8IL' */
#define V8_IL_VERSION  1

typedef struct {
    uint8_t  buttons[4];   /* placeholder: pad bits */
    int16_t  steerX;
    int16_t  throttle;
    int16_t  weaponSlot;
    int16_t  reserved;
} V8InputCommand;

typedef struct {
    uint32_t        magic;
    uint32_t        version;
    uint32_t        nFrames;
    uint32_t        inputSize;
    V8InputCommand *frames;
} V8InputLog;

int V8_InputLogWrite(const V8InputLog *log, const char *path)
{
    FILE *fp = fopen(path, "wb");
    if (!fp) return -1;
    uint32_t hdr[4] = { V8_IL_MAGIC, V8_IL_VERSION, log->nFrames, log->inputSize };
    fwrite(hdr, sizeof(uint32_t), 4, fp);
    fwrite(log->frames, log->inputSize, log->nFrames, fp);
    fclose(fp);
    return 0;
}

int V8_InputLogRead(V8InputLog *log, const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;
    uint32_t hdr[4];
    if (fread(hdr, sizeof(uint32_t), 4, fp) != 4) { fclose(fp); return -1; }
    if (hdr[0] != V8_IL_MAGIC) { fclose(fp); return -2; }
    log->magic     = hdr[0];
    log->version   = hdr[1];
    log->nFrames   = hdr[2];
    log->inputSize = hdr[3];
    log->frames    = (V8InputCommand *)malloc((size_t)log->nFrames * log->inputSize);
    if (!log->frames) { fclose(fp); return -3; }
    fread(log->frames, log->inputSize, log->nFrames, fp);
    fclose(fp);
    return 0;
}
