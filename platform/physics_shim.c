/* physics_shim.c -- one-vehicle Physics_Step host shim.
 *
 * The engine's real Physics_Step (FUN_8002131c) walks a linked list
 * at piRam0000075c and dispatches per-object tick callbacks. That
 * list has a 32-bit-pointer layout (node[2] = obj) that won't
 * round-trip cleanly on x64 without packed structs.
 *
 * For the single-player-driving milestone we don't need a multi-
 * object world walker. This shim does the minimum: invoke the
 * player vehicle's +0x64 tick callback directly. Multi-vehicle
 * support comes later (proper packed ObjNode in host_vehicle.c).
 *
 * Despite the shortcut on the walker, the per-object physics IS
 * the engine's: each vehicle's tick callback runs
 * Object_IntegrateAndOrient (cleaned, src/physics/object_integrate.c)
 * which exercises the real GTE-driven matrix update + integration.
 */
#include <stdio.h>
#include <stdint.h>

extern void *puRam000007d0;    /* player 1 vehicle */
extern void *puRam000007d4;    /* player 2 vehicle (NULL in solo) */
extern uint8_t DAT_80065a60[]; /* source Physics_Step list head */
extern uint8_t DAT_80065a18[]; /* source active collision list */
extern uint32_t *piRam00000714;
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

/* Match timer: g_v8_match_timer > 0 → set iRam00000624=1 when Physics_Step
 * call count reaches that value, simulating "timer expired" match end. */
extern int     g_v8_match_timer;
extern int     g_v8_frame_count;
extern int32_t iRam0000000c;
extern int32_t DAT_80065310;
extern uint32_t _DAT_80065310;
extern int32_t iRam00000624;   /* match-end flag → triggers ResultScreen */
extern uint8_t DAT_80065c28[];

typedef void (*TickFn)(uint8_t *self, int mode, int catchupFlag);

static void tick_one(uint8_t *obj, int catchupFlag)
{
    if (!obj) return;
    TickFn cb = (TickFn)Object_CallbackFromPsxSlot(obj);
    if (!cb) return;
    cb(obj, 0, catchupFlag);
}

static void snapshot_motion(uint8_t *obj, uint32_t out[13])
{
    if (!obj) {
        for (int i = 0; i < 13; i++) out[i] = 0;
        return;
    }
    out[0]  = *(uint32_t *)(obj + 0x00);
    out[1]  = *(uint32_t *)(obj + 0x24);
    out[2]  = *(uint32_t *)(obj + 0x28);
    out[3]  = *(uint32_t *)(obj + 0x2c);
    out[4]  = *(uint32_t *)(obj + 0x80);
    out[5]  = *(uint32_t *)(obj + 0x84);
    out[6]  = *(uint32_t *)(obj + 0x88);
    out[7]  = *(uint32_t *)(obj + 0x90);
    out[8]  = *(uint32_t *)(obj + 0x94);
    out[9]  = *(uint32_t *)(obj + 0x98);
    out[10] = *(uint32_t *)(obj + 0xa4);
    out[11] = *(uint32_t *)(obj + 0xa8);
    out[12] = *(uint32_t *)(obj + 0xd0);
}

static int snapshot_equal(const uint32_t a[13], const uint32_t b[13])
{
    for (int i = 0; i < 13; i++) {
        if (a[i] != b[i])
            return 0;
    }
    return 1;
}

static int position_equal(const uint32_t a[13], const uint32_t b[13])
{
    return a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static int has_nonzero_motion(const uint32_t a[13])
{
    return a[4] != 0 || a[5] != 0 || a[6] != 0 ||
           a[7] != 0 || a[8] != 0 || a[9] != 0;
}

static int motion_stalled_after_tick(const uint32_t before[13],
                                     const uint32_t after[13])
{
    return position_equal(before, after) && has_nonzero_motion(after);
}

typedef struct HostObjListNode {
    struct HostObjListNode *next;
    struct HostObjListNode *prev;
    uintptr_t payload;
    uint32_t deadline;
} HostObjListNode;

extern void FUN_8002131c(uint32_t catchupFlag);
extern int Vehicle_RollingTick(uint32_t *self, int mode, intptr_t arg2, intptr_t arg3);

static int host_list_contains(uint8_t *listHead, uint8_t *needle)
{
    HostObjListNode *sentinel = (HostObjListNode *)listHead;
    HostObjListNode *node;

    if (sentinel->prev == NULL)
        return 0;

    for (node = sentinel->next; node != NULL; node = node->next) {
        if ((uint8_t *)node->payload == needle)
            return 1;
    }
    return 0;
}

static int host_list_populated(uint8_t *listHead)
{
    HostObjListNode *sentinel = (HostObjListNode *)listHead;
    return sentinel != NULL && sentinel->prev != NULL && sentinel->next != NULL;
}

void Physics_Step(uint32_t catchupFlag)
{
    static int call_count = 0;
    static int log_first = 1;
    static int player_stall_frames = 0;
    static int opponent_stall_frames = 0;
    if (log_first) {
        fprintf(stderr, "v8: Physics_Step first call (puRam000007d0=%p)\n",
                puRam000007d0);
        log_first = 0;
    }
    call_count++;
    iRam0000000c = call_count;
    DAT_80065310 = call_count;
    _DAT_80065310 = (uint32_t)call_count;
    uint8_t *player = (uint8_t *)puRam000007d0;
    uint8_t *opponent = (uint8_t *)puRam000007d4;
    int list_has_player = host_list_contains(DAT_80065a60, player);
    int list_has_opponent = host_list_contains(DAT_80065a60, opponent);
    int ticked = host_list_populated(DAT_80065a60);
    uint32_t player_before[13], player_after[13];
    uint32_t opponent_before[13], opponent_after[13];

    snapshot_motion(player, player_before);
    snapshot_motion(opponent, opponent_before);

    if (ticked) {
        FUN_8002131c(catchupFlag);
    } else {
        tick_one(player, (int)catchupFlag);
        tick_one(opponent, (int)catchupFlag);
    }
    snapshot_motion(player, player_after);
    snapshot_motion(opponent, opponent_after);
    if (motion_stalled_after_tick(player_before, player_after))
        player_stall_frames++;
    else
        player_stall_frames = 0;
    if (motion_stalled_after_tick(opponent_before, opponent_after))
        opponent_stall_frames++;
    else
        opponent_stall_frames = 0;

    if (ticked && player && Object_CallbackFromPsxSlot(player) != 0 &&
        ((!list_has_player && call_count > 60) ||
         (player_stall_frames >= 60 && call_count > 300))) {
        static int logged_player_fallback = 0;
        uint32_t direct_before[13], direct_after[13];
        if (!logged_player_fallback) {
            fprintf(stderr,
                    "v8: Physics_Step sustained player stall candidate (%d frames) at frame %d; direct fallback disabled\n",
                    player_stall_frames, call_count);
            logged_player_fallback = 1;
        }
#if 0
        /* Disabled for playable builds: this host-list seam guard can double
         * integrate vehicles because the source list path may update visible
         * motion outside this before/after snapshot window. */
        snapshot_motion(player, direct_before);
        Vehicle_RollingTick((uint32_t *)player, 0, (intptr_t)catchupFlag, 0);
        snapshot_motion(player, direct_after);
        if (snapshot_equal(direct_before, direct_after)) {
            static int logged_player_still = 0;
            if (!logged_player_still) {
                fprintf(stderr,
                        "v8: Physics_Step direct Vehicle_RollingTick left player unchanged at frame %d\n",
                        call_count);
                logged_player_still = 1;
            }
        }
#endif
    }
    if (ticked && opponent && Object_CallbackFromPsxSlot(opponent) != 0 &&
        ((!list_has_opponent && call_count > 60) ||
         (opponent_stall_frames >= 60 && call_count > 300))) {
        static int logged_opponent_fallback = 0;
        uint32_t direct_before[13], direct_after[13];
        if (!logged_opponent_fallback) {
            fprintf(stderr,
                    "v8: Physics_Step sustained opponent stall candidate (%d frames) at frame %d; direct fallback disabled\n",
                    opponent_stall_frames, call_count);
            logged_opponent_fallback = 1;
        }
#if 0
        /* See player path above. */
        snapshot_motion(opponent, direct_before);
        Vehicle_RollingTick((uint32_t *)opponent, 0, (intptr_t)catchupFlag, 0);
        snapshot_motion(opponent, direct_after);
        if (snapshot_equal(direct_before, direct_after)) {
            static int logged_opponent_still = 0;
            if (!logged_opponent_still) {
                fprintf(stderr,
                        "v8: Physics_Step direct Vehicle_RollingTick left opponent unchanged at frame %d\n",
                        call_count);
                logged_opponent_still = 1;
            }
        }
#endif
    }

    /* Match timer: fire match-end exactly once when timer expires. */
    if (g_v8_match_timer > 0 && call_count == g_v8_match_timer && iRam00000624 == 0) {
        fprintf(stderr, "v8: match timer expired at frame %d -- triggering ResultScreen\n",
                call_count);
        iRam00000624 = 1;
    }
    if (call_count == 60 || call_count == 120 || call_count == 240 ||
        call_count == 480 || call_count == 600 || call_count == 720 ||
        call_count == 900 || call_count == 1200 ||
        (call_count > 1200 && (call_count % 600) == 0)) {
        uint8_t *v = (uint8_t *)puRam000007d0;
        if (v) {
            int32_t px24 = *(int32_t *)(v + 0x24);
            int32_t py24 = *(int32_t *)(v + 0x28);
            int32_t pz24 = *(int32_t *)(v + 0x2c);
            int32_t pxec = *(int32_t *)(v + 0xec);
            int32_t pyec = *(int32_t *)(v + 0xf0);
            int32_t pzec = *(int32_t *)(v + 0xf4);
            int16_t status = *(int16_t *)(v + 0x06);
            uint32_t idx = (uint32_t)~(int32_t)status * 24u;
            uint32_t lut_flags = *(uint32_t *)(DAT_80065c28 + idx + 0x08);
            int16_t steer = *(int16_t *)(v + 0xa4);
            int16_t input = *(int16_t *)(v + 0xa6);
            fprintf(stderr, "v8: Physics_Step @%d ticked=%d saw=(%d,%d) cb=%p flags=0x%x  +24=(0x%x,0x%x,0x%x)  +ec=(0x%x,0x%x,0x%x) vel=(%d,%d,%d) ang=(%d,%d,%d) branch18=%d status=%d state=%u lut_flags=0x%x steer=%d input=%d\n",
                    call_count, ticked, list_has_player, list_has_opponent,
                    (void *)Object_CallbackFromPsxSlot(v),
                    (unsigned)*(uint32_t *)(v + 0x00),
                    px24, py24, pz24, pxec, pyec, pzec,
                    *(int32_t *)(v + 0x80), *(int32_t *)(v + 0x84),
                    *(int32_t *)(v + 0x88), *(int32_t *)(v + 0x90),
                    *(int32_t *)(v + 0x94), *(int32_t *)(v + 0x98),
                    (int)*(int16_t *)(v + 0x18),
                    (int)status, (unsigned)*(uint8_t *)(v + 0xd0),
                    (unsigned)lut_flags, (int)steer, (int)input);
            if (call_count == 60) {
                uint32_t child10c = *(uint32_t *)(v + 0x10c);
                uint32_t slot110 = *(uint32_t *)(v + 0x110);
                uint32_t slot114 = *(uint32_t *)(v + 0x114);
                uint32_t slot118 = *(uint32_t *)(v + 0x118);
                fprintf(stderr,
                        "v8: player source slots +10c=%p +110=%p +114=%p +118=%p active=%u\n",
                        (void *)(uintptr_t)child10c,
                        (void *)(uintptr_t)slot110,
                        (void *)(uintptr_t)slot114,
                        (void *)(uintptr_t)slot118,
                        (unsigned)*(uint8_t *)(v + 0xb3));
                for (int i = 0; i < 3; i++) {
                    uint8_t *slot = (uint8_t *)(uintptr_t)*(uint32_t *)(v + 0x110 + i * 4);
                    if (slot) {
                        fprintf(stderr,
                                "v8:   weapon slot[%d] obj=%p kind=%d hp=%d cb=%p flags=0x%x\n",
                                i, (void *)slot,
                                (int)*(int8_t *)(slot + 0x08),
                                (int)*(int16_t *)(slot + 0x0c),
                                (void *)Object_CallbackFromPsxSlot(slot),
                                *(unsigned *)(slot + 0x00));
                    }
                }
            }
        }
        if (call_count == 60 || call_count == 600) {
            uint8_t *ai = (uint8_t *)puRam000007d4;
            if (ai) {
                fprintf(stderr,
                        "v8: AI source @%d +24=(0x%x,0x%x,0x%x) status=%d kind=%u state=%u path=%d steer=%d input=%d cb=%p\n",
                        call_count,
                        (unsigned)*(uint32_t *)(ai + 0x24),
                        (unsigned)*(uint32_t *)(ai + 0x28),
                        (unsigned)*(uint32_t *)(ai + 0x2c),
                        (int)*(int16_t *)(ai + 0x06),
                        (unsigned)*(uint8_t *)(ai + 0x08),
                        (unsigned)*(uint8_t *)(ai + 0xd0),
                        (int)*(int16_t *)(ai + 0xc0),
                        (int)*(int16_t *)(ai + 0xa4),
                        (int)*(int16_t *)(ai + 0xa6),
                        (void *)Object_CallbackFromPsxSlot(ai));
            }
        }
    }
}

/* Sibling stubs that main_loop.c calls but which are no-ops at this
 * scope: Physics_PostStep tidies per-frame state (HUD overlays,
 * trail effects); Physics_FlushVoxels flushes any deferred terrain
 * voxel grid invalidations. Neither matters until we have multi-
 * object + terrain collision. */
void Physics_PostStep(uint32_t tick)
{
    extern void FUN_80021394(uint32_t tick);
    FUN_80021394(tick);
}
void Physics_FlushVoxels(void)
{
    extern void FUN_80021678(void);
    if (piRam00000714 == NULL)
        piRam00000714 = (uint32_t *)DAT_80065a18;
    FUN_80021678();
}
