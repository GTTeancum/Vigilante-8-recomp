/* run_compare.c -- driver: read two snapshot blobs, diff them, print
 * the first 16 mismatched offsets (with the field name when possible).
 *
 * Usage:
 *   run_compare baseline.bin port.bin
 *
 * Field-name resolution: we use the byte offset within V8StateSnapshot
 * to map to a logical field. The table below is updated as struct fields
 * are recovered in subsequent passes.
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "state_snapshot.h"

typedef struct { uint32_t off; uint32_t size; const char *name; } FieldDesc;

#define F(field) { (uint32_t)offsetof(V8StateSnapshot, field), \
                   (uint32_t)sizeof(((V8StateSnapshot*)0)->field), #field }

static const FieldDesc kFields[] = {
    F(magic), F(version), F(frameCounter), F(rngSeed), F(rngCounter),
    F(matchMode), F(splitScreenMode), F(drawBufIndex), F(dispBufIndex),
    F(p1Present), F(p2Present),
    F(vehicles),
    F(matchConfigFlags), F(optionFlag_c28), F(optionFlag_c40),
};

static const char *field_for(uint32_t off)
{
    const char *best = "?";
    for (size_t i = 0; i < sizeof(kFields) / sizeof(kFields[0]); i++) {
        if (off >= kFields[i].off && off < kFields[i].off + kFields[i].size) {
            best = kFields[i].name;
        }
    }
    return best;
}

static int diffs_emitted = 0;
static void emit_diff(const char *_unused, uint32_t off)
{
    (void)_unused;
    if (diffs_emitted++ < 16) {
        printf("  diff @ +0x%04x  field=%s\n", off, field_for(off));
    }
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s baseline.bin port.bin\n", argv[0]);
        return 2;
    }
    V8StateSnapshot a, b;
    if (V8_SnapshotRead(&a, argv[1]) != 0) { fprintf(stderr, "bad baseline\n"); return 3; }
    if (V8_SnapshotRead(&b, argv[2]) != 0) { fprintf(stderr, "bad port\n");     return 3; }
    int n = V8_SnapshotCompare(&a, &b, emit_diff);
    printf("total mismatched bytes: %d / %u\n",
           n, (unsigned)sizeof(V8StateSnapshot));
    return (n == 0) ? 0 : 1;
}
