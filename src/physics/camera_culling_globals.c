/* camera_culling_globals.c -- camera state used by source frustum culling.
 *
 * These symbols live in the main EXE BSS around 0x8006f6f4 and 0x8006f780.
 * Camera_ApplyFrame writes the position values, while FUN_8001d898 builds the
 * frustum projection matrix at DAT_8006f780.
 */
#include <stdint.h>
#include "structs.h"

/* MED: current camera world-space origin, consumed by FUN_8001db54. */
int32_t DAT_8006f6f4;
int32_t DAT_8006f6f8;
int32_t DAT_8006f6fc;

/* MED: view-frustum projection matrix used by source culling. */
MATRIX DAT_8006f780;

/* MED: current and alternate viewport camera matrices. */
MATRIX DAT_8006f680;
MATRIX DAT_8006f6a0;
