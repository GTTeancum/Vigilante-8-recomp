/* smoke_emitter_tick.c -- vehicle damage-smoke emitter tick + puff tick.
 *
 * Source: SLUS_005.10
 *   LAB_8002c210  0x8002c210  (smoke emitter per-frame tick, ~76 instr)
 *   LAB_800404c4  0x800404c4  (smoke puff per-frame tick,    ~21 instr)
 *
 * LAB_8002c210 is installed at offset +0x64 of the smoke-emitter object
 * allocated by FUN_8002c3ac (vehicle_damage_smoke.c).  It runs every tick
 * to count down a timer and, when it reaches 0xffff (i.e. wrapped through
 * zero), spawns a new floating puff via FUN_8001ac44, sets its velocity
 * and initial world position, installs LAB_800404c4 as the puff tick, and
 * registers the puff with the scene.  It also calls FUN_8002c6fc to apply
 * a small ongoing damage pulse to the parent vehicle.
 *
 * LAB_800404c4 is installed by LAB_8002c210 onto each spawned puff.
 * Every tick it integrates position += velocity (Euler step).  On event 5
 * (death) it calls FUN_800205f8 to remove the puff from the scene.
 *
 * Object field layout (smoke emitter):
 *   +0x00 (0)   : flags (uint32)
 *   +0x0c (12)  : damage-per-puff (uint16)
 *   +0x10 (16)  : rotation/world matrix (MATRIX)
 *   +0x24 (36)  : world pos X (int32)
 *   +0x28 (40)  : world pos Y (int32)
 *   +0x2c (44)  : world pos Z (int32)
 *   +0x80 (128) : countdown timer (uint16, counts down to 0 then 0xffff triggers spawn)
 *   +0x82 (130) : timer reload value (uint16)
 *   +0x84 (132) : puff lateral speed (int32, Q0)
 *   +0x88 (136) : puff Y-velocity seed (int32, downward = negative)
 *   +0x96 (150) : puff bone-slot index (uint16)
 *   +0x98 (152) : world bank handle (int32)
 *
 * Object field layout (smoke puff, installed by this tick):
 *   +0x24 (36)  : world pos X (int32)
 *   +0x28 (40)  : world pos Y (int32)
 *   +0x2c (44)  : world pos Z (int32)
 *   +0x64 (100) : tick callback pointer
 *   +0x88 (136) : velocity X (int32)
 *   +0x8c (140) : velocity Y (int32)
 *   +0x90 (144) : velocity Z (int32)
 *
 * HIGH confidence: line-for-line MIPS port.
 */
#include <stdint.h>
#include "../../include/structs.h"

/* FUN_8001ac44 -- BoneObj_BuildTree: allocate & initialise a bone object. */
extern int   FUN_8001ac44(int *bank, uint16_t slot, int size, int flags);

/* FUN_80017160 -- V8_RandNext: PRNG, returns pseudo-random uint32. */
extern uint32_t FUN_80017160(void);

/* FUN_8001d624 -- Object_GetWorldMatrix: returns MATRIX * (= &obj[0x10]). */
extern MATRIX *FUN_8001d624(int obj);

/* FUN_800202f4 -- Object_RegisterInScene: add puff to the live object list. */
extern void FUN_800202f4(uint32_t *obj);

/* FUN_8001d5e0 -- Object_FindRoot: walk parent chain, return root vehicle. */
extern int  FUN_8001d5e0(int obj);

/* FUN_8002c6fc -- Vehicle_DistributeDamage: apply impulse damage to vehicle. */
extern int  FUN_8002c6fc(uint32_t *self, int impulse, const int32_t *vec, int flag);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* FUN_8001d564 -- Object_DetachFromParent: unlink self; returns self handle. */
extern int  FUN_8001d564(int self);

/* FUN_800204dc -- Object_FreeAndUnregister: free obj and dequeue from lists. */
extern void FUN_800204dc(int obj);

/* FUN_800205f8 -- Damage_Apply alias (also removes puff from scene). */
extern void FUN_800205f8(int obj);

/* DAT_800607b4: 4096-entry interleaved sin/cos Q12 LUT.
 *   sin[i] = DAT_800607b4[i*2],  cos[i] = DAT_800607b4[i*2+1]. */
extern int16_t DAT_800607b4[];

/* ------------------------------------------------------------------ */

/*
 * LAB_800404c4 -- smoke puff per-frame tick.
 *
 * Installed by LAB_8002c210 at puff[0x64].
 * event 0  -> integrate world position (pos += vel, one Euler step).
 * event 5  -> FUN_800205f8(obj); return -1.
 * other    -> return 0.
 *
 * Source: 0x800404c4..0x8004053c (21 instructions).
 */
int LAB_800404c4(int obj, int event, int param3)
{
    uint8_t *t0 = (uint8_t *)(uintptr_t)(uint32_t)obj;
    (void)param3;

    if (event == 0) {
        /* Euler integrate: pos += vel */
        *(int32_t *)(t0 + 0x24) += *(int32_t *)(t0 + 0x88);
        *(int32_t *)(t0 + 0x28) += *(int32_t *)(t0 + 0x8c);
        *(int32_t *)(t0 + 0x2c) += *(int32_t *)(t0 + 0x90);
        return 0;
    }
    if (event == 5) {
        FUN_800205f8(obj);
        return -1;
    }
    return 0;
}

/* RTZ (round-toward-zero) arithmetic shift right by n bits. */
static inline int32_t rtz_sra(int32_t x, int n)
{
    if (x < 0) x += (1 << n) - 1;
    return x >> n;
}

/*
 * LAB_8002c210 -- smoke emitter per-frame tick.
 *
 * Installed at +0x64 of the smoke-emitter object by FUN_8002c3ac.
 * event 0  -> countdown timer; on wrap spawn puff + apply damage.
 * event 2  -> destroy: clear parent smoke flag, detach, free self.
 * other    -> return 0.
 *
 * Source: 0x8002c210..0x8002c3a8 (~76 instructions).
 */
int LAB_8002c210(int obj, int event, int param3)
{
    uint8_t *s0 = (uint8_t *)(uintptr_t)(uint32_t)obj;
    (void)param3;

    /* ---- event dispatch ---- */
    if (event != 0) {
        if (event == 2) {
            /* Destroy: find root vehicle and clear its "has smoke" flag. */
            int root = FUN_8001d5e0(obj);
            *(uint32_t *)(uintptr_t)(uint32_t)root &= ~0x04000000u;
            /* Detach from parent (returns self), then free. */
            int self = FUN_8001d564(obj);
            FUN_800204dc(self);
            return -1;
        }
        return 0;
    }

    /* ---- event 0: tick ---- */

    /* Decrement uint16 timer; trigger on wrap (0 -> 0xffff). */
    uint16_t timer = *(uint16_t *)(s0 + 0x80);
    timer = (uint16_t)(timer - 1u);
    *(uint16_t *)(s0 + 0x80) = timer;
    if (timer != 0xffffu) return 0;

    /* ---- spawn a new smoke puff ---- */
    int bank   = *(int32_t *)(s0 + 0x98);
    uint16_t slot = *(uint16_t *)(s0 + 0x96);
    int puff = FUN_8001ac44((int *)(uintptr_t)(uint32_t)bank, slot, 160, 8);

    /* First random number: determines puff direction angle. */
    uint32_t rand1 = FUN_80017160();

    /* Set puff flags. */
    *(uint32_t *)(uintptr_t)(uint32_t)puff |= 0x4b4u;

    /* Look up (sin, cos) pair from 4096-entry Q12 LUT. */
    int angle = (int)(rand1 & 0xfffu);           /* 0..4095            */
    int16_t sin_val = DAT_800607b4[angle * 2];
    int16_t cos_val = DAT_800607b4[angle * 2 + 1];

    /* Lateral speed scale. */
    int32_t speed = *(int32_t *)(s0 + 0x84);

    /* vel_x = RTZ(speed * sin >> 12) */
    int32_t vel_x = rtz_sra(speed * (int32_t)sin_val, 12);
    *(int32_t *)(puff + 0x88) = vel_x;

    /* vel_z = RTZ(speed * cos >> 12) */
    int32_t vel_z = rtz_sra(speed * (int32_t)cos_val, 12);

    /* Second random number: perturbs Y velocity. */
    uint32_t rand2 = FUN_80017160();
    /* delay-slot equivalent: store vel_z */
    *(int32_t *)(puff + 0x90) = vel_z;

    /* vel_y = drift + (rand2 * drift) >> 15  (plain sra, not RTZ). */
    int32_t drift = *(int32_t *)(s0 + 0x88);
    int32_t vel_y = drift + (int32_t)(((int32_t)rand2 * drift) >> 15);
    /* delay-slot equivalent: store vel_y (set up a0=obj before FUN_8001d624) */
    *(int32_t *)(puff + 0x8c) = vel_y;

    /* Copy emitter world position to puff. */
    MATRIX *worldmat = FUN_8001d624(obj);
    *(int32_t *)(puff + 0x24) = worldmat->t[0];
    *(int32_t *)(puff + 0x28) = worldmat->t[1];
    *(int32_t *)(puff + 0x2c) = worldmat->t[2];

    /* Install puff tick callback and register puff in the scene.
     * (MIPS: delay slot stores callback ptr, then jal FUN_800202f4.) */
    Object_SetCallbackPsxSlot((void *)(uintptr_t)puff, (uintptr_t)&LAB_800404c4);
    FUN_800202f4((uint32_t *)(uintptr_t)(uint32_t)puff);

    /* Reset countdown timer.
     * (MIPS: delay slot of jal Object_FindRoot stores reload value.) */
    *(uint16_t *)(s0 + 0x80) = *(uint16_t *)(s0 + 0x82);

    /* Find root vehicle and apply per-puff damage pulse. */
    int root = FUN_8001d5e0(obj);
    int damage_val = (int)(uint16_t)*(uint16_t *)(s0 + 0x0c);
    FUN_8002c6fc((uint32_t *)(uintptr_t)(uint32_t)root,
                 -damage_val,
                 (const int32_t *)(s0 + 0x24),
                 1);

    return 0;
}
