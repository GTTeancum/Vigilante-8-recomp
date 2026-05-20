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

typedef void (*TickFn)(uint8_t *self, int mode, int catchupFlag);

static void tick_one(uint8_t *obj, int catchupFlag)
{
    if (!obj) return;
    TickFn cb = *(TickFn *)(obj + 0x64);
    if (!cb) return;
    cb(obj, 0, catchupFlag);
}

void Physics_Step(uint32_t catchupFlag)
{
    static int call_count = 0;
    static int log_first = 1;
    if (log_first) {
        fprintf(stderr, "v8: Physics_Step first call (puRam000007d0=%p)\n",
                puRam000007d0);
        log_first = 0;
    }
    call_count++;
    tick_one((uint8_t *)puRam000007d0, (int)catchupFlag);
    tick_one((uint8_t *)puRam000007d4, (int)catchupFlag);
    if (call_count == 60) {
        uint8_t *v = (uint8_t *)puRam000007d0;
        if (v) {
            int32_t px = *(int32_t *)(v + 0x24);
            int32_t py = *(int32_t *)(v + 0x28);
            int32_t pz = *(int32_t *)(v + 0x2c);
            fprintf(stderr, "v8: Physics_Step @60 -- vehicle pos = (0x%x, 0x%x, 0x%x)\n",
                    px, py, pz);
        }
    }
}

/* Sibling stubs that main_loop.c calls but which are no-ops at this
 * scope: Physics_PostStep tidies per-frame state (HUD overlays,
 * trail effects); Physics_FlushVoxels flushes any deferred terrain
 * voxel grid invalidations. Neither matters until we have multi-
 * object + terrain collision. */
void Physics_PostStep(uint32_t tick) { (void)tick; }
void Physics_FlushVoxels(void)       { }
