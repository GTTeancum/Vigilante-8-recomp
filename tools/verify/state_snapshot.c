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
#include "state_snapshot.h"

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
    if (snap->version != V8_SS_VERSION) return -3;
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
