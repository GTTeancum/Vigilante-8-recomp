/* control_tower.c -- AirGrave control_tower_1 impact.
 *
 * Source: AIRGRAVE.DLL  FUN_80100c18.
 *
 * Byte-identical body to WW_BonfireImpact (Wild West) -- another
 * instance of inlined-across-DLLs convenience code. On non-zero,
 * non-5 mode, set scratch slot _DAT_00000029 to (obj.posY - 0x5f5)
 * (= "spark emit at this Y"), then unconditionally invoke
 * Damage_Apply.
 *
 * HIGH.
 */
#include <stdint.h>

extern void Damage_Apply(void *obj);
extern int8_t _DAT_00000029;

uint32_t AG_ControlTowerImpact(int obj, int mode)
{
    if (mode != 0) {
        if (mode == 5) goto handle;
        obj = 1;
    }
    _DAT_00000029 = (int8_t)(*(int *)(obj + 0x28) - 0x5f5);
handle:
    Damage_Apply((void *)(intptr_t)obj);
    return 0;
}
