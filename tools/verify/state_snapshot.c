/* state_snapshot.c -- minimum-viable snapshot serializer.
 *
 * Reads / writes the V8StateSnapshot blob to disk. Field-level diff via
 * a byte-by-byte comparator: any divergence is reported with the byte
 * offset; per-field labels arrive in pass 2 once structs are finalized.
 *
 * Endianness: the blob is little-endian (PSX is LE), so on the host port
 * we write the struct directly. If a future port runs on a BE host the
 * writer/reader must swap explicitly.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "state_snapshot.h"

#define V8_SS_MAGIC    0x53533856u  /* 'V8SS' little-endian */
#define V8_SS_VERSION  2

/* External state references (pass-2 expanded). */
extern uint8_t  g_splitScreenMode;
extern int32_t  g_drawBufIndex;
extern int32_t  g_dispBufIndex;
extern void    *g_player1Vehicle;
extern void    *g_player2Vehicle;
extern uint32_t g_matchConfigFlags;
extern int32_t  g_optionFlag_c28;
extern int32_t  g_optionFlag_c40;

/* Top-level globals the snapshot reads from. Declared extern; the live
 * game provides definitions. */
extern uint32_t g_vsyncCounter;
extern int32_t  g_rngSeed;
extern int32_t  g_rngCounter;
extern uint8_t  g_matchMode;
/* g_vehicles[]: not yet a single array in the binary. Pass 2 will
 * introduce a unified array. For now zero-fill. */

int V8_SnapshotCapture(V8StateSnapshot *out)
{
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    out->magic            = V8_SS_MAGIC;
    out->version          = V8_SS_VERSION;
    out->frameCounter     = g_vsyncCounter;
    out->rngSeed          = g_rngSeed;
    out->rngCounter       = g_rngCounter;
    out->matchMode        = g_matchMode;
    out->splitScreenMode  = (uint8_t)g_splitScreenMode;
    out->drawBufIndex     = (uint8_t)g_drawBufIndex;
    out->dispBufIndex     = (uint8_t)g_dispBufIndex;
    out->matchConfigFlags = g_matchConfigFlags;
    out->optionFlag_c28   = g_optionFlag_c28;
    out->optionFlag_c40   = g_optionFlag_c40;

    /* Player 1 / 2 vehicle snapshots. */
    if (g_player1Vehicle != NULL) {
        out->p1Present = 1;
        memcpy(&out->vehicles[0], g_player1Vehicle, sizeof(Vehicle));
    }
    if (g_player2Vehicle != NULL) {
        out->p2Present = 1;
        memcpy(&out->vehicles[1], g_player2Vehicle, sizeof(Vehicle));
    }
    /* AI vehicles [2..V8_MAX_VEHICLES-1]: pass 3 fills once the pool
     * pointer is named. */
    return 0;
}

int V8_SnapshotWrite(const V8StateSnapshot *snap, const char *path)
{
    FILE *fp = fopen(path, "wb");
    if (!fp) return -1;
    size_t n = fwrite(snap, 1, sizeof(*snap), fp);
    fclose(fp);
    return (n == sizeof(*snap)) ? 0 : -1;
}

int V8_SnapshotRead(V8StateSnapshot *snap, const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;
    size_t n = fread(snap, 1, sizeof(*snap), fp);
    fclose(fp);
    if (n != sizeof(*snap))    return -1;
    if (snap->magic != V8_SS_MAGIC) return -2;
    return 0;
}

int V8_SnapshotCompare(const V8StateSnapshot *a, const V8StateSnapshot *b,
                       void (*emit_diff)(const char *field, uint32_t off))
{
    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
    int diffs = 0;
    for (uint32_t i = 0; i < sizeof(*a); i++) {
        if (pa[i] != pb[i]) {
            diffs++;
            if (emit_diff) emit_diff("?", i);
        }
    }
    return diffs;
}
