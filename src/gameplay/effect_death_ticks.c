/* effect_death_ticks.c -- per-frame tick callbacks for particle/effect/missile death.
 *
 * Four tick label functions decompiled from PSX addresses:
 *   LAB_8003e7b4  propagate event up to parent tick
 *   LAB_8003e80c  particle death: detach+free or remove from scene
 *   LAB_8003e868  effect death: unbind child at obj+0x3c from world
 *   LAB_800372b0  missile countdown: integrate position, decrement timer, detonate
 */
#include <stdint.h>

extern int  FUN_8001d564(int self);       /* Object_DetachFromParent -- returns self */
extern void FUN_800204dc(int obj);        /* Vehicle_Free (bone-tree free) */
extern void FUN_800205f8(int obj);        /* Damage_Apply (apply to obj then remove) */
extern void FUN_800207f8(int *obj);       /* Object_UnbindFromWorldBind */
extern int  FUN_8001d5a0(int self);       /* Object_Parent -- returns parent int ptr */
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

/* LAB_8003e7b4 -- Propagate event to parent's tick callback.
 * Reads the parent's tick function pointer from parent+0x64, then calls it. */
int LAB_8003e7b4(int obj, int event, int param3)
{
    int parent = FUN_8001d5a0(obj);
    int (*tick)(int,int,int) =
        (int (*)(int,int,int))Object_CallbackFromPsxSlot((const void *)(uintptr_t)parent);
    if (tick) return tick(parent, event, param3);
    return 0;
}

/* LAB_8003e80c -- Particle death: detach+free or remove from scene.
 * Only acts on event 5 (die now).
 * If obj+0x3c link is non-zero: detach from parent then free the bone tree.
 * Otherwise: apply damage (removes from scene). */
int LAB_8003e80c(int obj, int event, int param3)
{
    (void)param3;
    if (event != 5) return 0;
    int32_t link = *(int32_t *)((uint8_t *)(uintptr_t)obj + 60);
    if (link != 0) {
        FUN_8001d564(obj);      /* detach from parent (returns obj) */
        FUN_800204dc(obj);      /* free: delay slot used v0=obj from detach */
    } else {
        FUN_800205f8(obj);
    }
    return -1;
}

/* LAB_8003e868 -- Effect death: unbind child at obj+0x3c from world.
 * Only acts on event 5 (die now). */
int LAB_8003e868(int obj, int event, int param3)
{
    (void)param3;
    if (event == 5) {
        int child = *(int32_t *)((uint8_t *)(uintptr_t)obj + 60);
        FUN_800207f8((int *)child);
        return -1;
    }
    return 0;
}

/* LAB_800372b0 -- Missile countdown tick: integrate position, decrement timer.
 * Only acts on event 0 (normal tick).
 * pos (obj+36/40/44) += vel (obj+136/140/144) each tick.
 * Decrements byte countdown at obj+134; when it hits 0, calls Damage_Apply. */
int LAB_800372b0(int obj, int event, int param3)
{
    (void)param3;
    if (event != 0) return 0;
    uint8_t *p = (uint8_t *)(uintptr_t)obj;
    *(int32_t *)(p+36)  = mips_addu_i32(*(int32_t *)(p+36), *(int32_t *)(p+136));
    *(int32_t *)(p+40)  = mips_addu_i32(*(int32_t *)(p+40), *(int32_t *)(p+140));
    *(int32_t *)(p+44)  = mips_addu_i32(*(int32_t *)(p+44), *(int32_t *)(p+144));
    uint8_t count = *(uint8_t *)(p+134);
    count = (uint8_t)mips_subu_i32(count, 1);
    *(uint8_t *)(p+134) = count;
    if (count != 0) return 0;
    FUN_800205f8(obj);
    return -1;
}
