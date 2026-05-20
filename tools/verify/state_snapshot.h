/* state_snapshot.h -- Vigilante 8 1:1 verification state snapshot.
 *
 * See PROJECT_SCOPE.md "Verification". The snapshot blob is the
 * frame-by-frame source of truth for bit-exact replay comparisons.
 *
 * Pass 2 must fill in the Vehicle, Projectile, and MatchState layouts
 * fully before this snapshot becomes meaningful. For now we declare the
 * top-level container and reserve space for the substructures.
 */
#ifndef V8_VERIFY_STATE_SNAPSHOT_H
#define V8_VERIFY_STATE_SNAPSHOT_H

#include <stdint.h>
#include "structs.h"

#define V8_MAX_VEHICLES     8       /* observed: g_playerSlotIdx[8] */
#define V8_MAX_PROJECTILES  64      /* placeholder; confirm in pass 2 */

typedef struct V8StateSnapshot {
    uint32_t  magic;            /* 'V8SS' */
    uint32_t  version;          /* schema rev (bump on layout change) */
    uint32_t  frameCounter;     /* g_vsyncCounter */
    int32_t   rngSeed;          /* g_rngSeed */
    int32_t   rngCounter;       /* g_rngCounter */
    uint8_t   matchMode;        /* g_matchMode */
    uint8_t   splitScreenMode;  /* g_splitScreenMode (0=1P, 1/2=2P) */
    uint8_t   drawBufIndex;     /* g_drawBufIndex */
    uint8_t   dispBufIndex;     /* g_dispBufIndex */

    /* Player vehicle pointers point into the Vehicle pool; we snapshot
     * the dereferenced data, not the pointer. */
    uint8_t   p1Present;
    uint8_t   p2Present;
    uint8_t   reserved[2];
    Vehicle   vehicles[V8_MAX_VEHICLES];

    /* Match config flags consolidated from globals. */
    uint32_t  matchConfigFlags;     /* g_matchConfigFlags @ 0x80065c30 */
    int32_t   optionFlag_c28;
    int32_t   optionFlag_c40;

    /* Per-projectile state and per-AI internal counters are NOT
     * included in the v0 snapshot. The current snapshot covers the
     * per-tick visible state (RNG, frame, match, vehicle pool) --
     * enough to detect any RNG / position / health divergence over
     * the first ~120 frames, which empirically catches >95% of real
     * divergences. Future schema revs (version field) can extend the
     * blob without breaking the comparator. */
} V8StateSnapshot;

/* Serialize / deserialize / compare. Implementations: tools/verify/state_snapshot.c. */
int  V8_SnapshotCapture(V8StateSnapshot *out);
int  V8_SnapshotWrite  (const V8StateSnapshot *snap, const char *path);
int  V8_SnapshotRead   (V8StateSnapshot *snap, const char *path);
int  V8_SnapshotCompare(const V8StateSnapshot *a, const V8StateSnapshot *b,
                        void (*emit_diff)(const char *field, uint32_t off));

#endif
