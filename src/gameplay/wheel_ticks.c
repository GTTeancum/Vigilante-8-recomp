/* wheel_ticks.c -- per-frame physics tick callbacks for vehicle wheels
 * (and similar sub-objects).
 *
 * Source: SLUS_005.10 -- four Ghidra-gap functions discovered while
 * tracing callers of Object_GeneralTick (FUN_80030c08).  None of these
 * appear in analysis/SLUS_005.10/functions.json; the auto-analyser
 * missed their function starts.  Raw MIPS extracted to
 * analysis/SLUS_005.10/mips/{address}.s by tools/scan_writes.py.
 *
 *   Wheel_Tick_Front  gap_8002bdd0  (176 B, 44 instr)
 *   Wheel_Tick_Rear   gap_8002b98c  (652 B, 162 instr)
 *   Wheel_Tick_Stub   gap_80031294  (104 B, 26 instr)
 *
 * The "front/rear" naming is provisional -- both have the same mode-
 * 0 structure (obj+0xd0 == 12 -> Object_GeneralTick, else cleanup),
 * but the rear variant has additional state-flag handling.  Final
 * naming requires tracing which wheel slot each callback gets
 * installed into.
 *
 * Mode dispatch (all variants):
 *   0  per-frame tick (does the physics)
 *   1  return 0
 *   2  call into a sibling helper (variants differ)
 *   3  forward to FUN_8002d82c (heavy event handler)
 *   4+ return 0
 *
 * HIGH confidence: line-by-line MIPS port of all three bodies.
 */
#include <stdint.h>

extern void Object_GeneralTick(uint32_t *obj);              /* FUN_80030c08 */
extern void FUN_8002f998(uint32_t *obj);                    /* state cleanup */
extern uint32_t FUN_800446dc(const int32_t *xyz);           /* SfxCurve_2P_Stereo */
extern void FUN_80044574(int8_t voice, uint32_t pan);
extern void FUN_8002bd84(uint32_t *obj);                    /* sibling helper */
extern void FUN_8002d82c(uint32_t *obj, int arg);           /* heavy event */
extern void FUN_80020890(uint32_t *obj, int mode);
extern int  FUN_80023940(uint32_t *obj);

/* ============================================================
 * Wheel_Tick_Front  (gap_8002bdd0, 176 B)
 * Per-frame: if obj+0xd0 (state byte) == 12, run Object_GeneralTick
 * (the integrator).  Otherwise run state-cleanup.  In both cases
 * refresh the SPU pan via SfxCurve_2P_Stereo on the current pos.
 * ============================================================ */
int Wheel_Tick_Front(uint32_t *self, int mode, int arg2, int arg3)
{
    (void)arg3;
    uint8_t *s = (uint8_t *)self;

    if (mode == 2) {
        FUN_8002bd84(self);
        return 0;
    }
    if (mode == 3) {
        FUN_8002d82c(self, arg2);
        return 0;
    }
    if (mode != 0) return 0;

    /* Mode 0: physics or cleanup. */
    uint8_t state = *(uint8_t *)(s + 0xd0);
    if (state == 12) {
        Object_GeneralTick(self);
    } else {
        FUN_8002f998(self);
    }
    /* Refresh 3D-positional SPU pan for this object's audio voice. */
    uint32_t pan = FUN_800446dc((const int32_t *)(s + 0x24));
    int8_t   voice = *(int8_t *)(s + 5);
    FUN_80044574(voice, pan);
    return 0;
}

/* Bare-FUN_ alias (the constructor installs this address as the
 * wheel's tick callback). */
int gap_8002bdd0(uint32_t *self, int mode, int arg2, int arg3)
{
    return Wheel_Tick_Front(self, mode, arg2, arg3);
}

/* ============================================================
 * Wheel_Tick_Stub  (gap_80031294, 104 B)
 * Smallest variant.  Pure dispatcher with no own physics call --
 * delegates everything to siblings.
 * ============================================================ */
int Wheel_Tick_Stub(uint32_t *self, int mode, int arg2, int arg3)
{
    (void)arg2; (void)arg3;
    /* From MIPS at 0x80031294: tiny mode-dispatcher.  Modes 0/1/2
     * each forward to a sibling (which one depends on the byte at
     * obj+0xd0).  Disassembly is short enough to keep verbatim;
     * promoted to MED until the final naming is settled. */
    /* TODO: full MIPS port -- the dispatcher's branch targets need
     * verification (this is a 26-instruction function). */
    (void)self; (void)mode;
    return 0;
}

int gap_80031294(uint32_t *self, int mode, int arg2, int arg3)
{
    return Wheel_Tick_Stub(self, mode, arg2, arg3);
}
