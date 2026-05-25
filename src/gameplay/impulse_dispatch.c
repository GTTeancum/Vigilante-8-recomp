/* impulse_dispatch.c -- FUN_8002c958 (19 MIPS instructions).
 *
 * Impulse-application dispatcher: checks whether the vehicle's abort
 * flag (*(uint16_t *)(self + 0x11e)) is set and routes accordingly.
 *
 *   if self[0x11e] != 0:
 *       FUN_8002c018(self, impulse, vec, flag)   -- vehicle abort/cleanup
 *       return 0
 *   else:
 *       return FUN_8002c6fc(self, impulse, vec, flag)  -- linear impulse
 *
 * All four register arguments (a0..a3) are forwarded unchanged to each
 * callee, so the signature here is the same as both callees.
 *
 * Source: SLUS_005.10  0x8002c958
 * HIGH confidence: trivial branch dispatch.
 */
#include <stdint.h>

extern int FUN_8002c018(uint32_t *self, int impulse, const int32_t *vec, int flag);
extern int FUN_8002c6fc(uint32_t *self, int impulse, const int32_t *vec, int flag);

int FUN_8002c958(uint32_t *self, int impulse, const int32_t *vec, int flag)
{
    /* MIPS 0x8002c960: lhu v0,0x11e(a0) -- unsigned 16-bit load */
    if (*(uint16_t *)((uint8_t *)self + 0x11e) != 0u) {
        /* 0x8002c980: jal FUN_8002c018 (abort path) */
        FUN_8002c018(self, impulse, vec, flag);
        /* 0x8002c988: clear v0  -- force return 0 */
        return 0;
    }
    /* 0x8002c970: jal FUN_8002c6fc (impulse path); returns its result */
    return FUN_8002c6fc(self, impulse, vec, flag);
}
