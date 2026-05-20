/* vehicle_stub.c -- placeholder Vehicle and pad-state globals.
 *
 * V8_MainLoop dereferences puRam000007d0/puRam000007d4 (player vehicle
 * pointers) heavily, but the real Vehicle gets allocated during level
 * load -- which is Level_LoadByName, currently a panic stub. To boot
 * through main loop without crashing, we provide a zero-initialized
 * 0x200-byte stub Vehicle and point P1 at it.
 *
 * This shadows the panic_stub.c definitions of puRam000007d0 etc.
 * The panic-stub generator's hand-written-impls detector picks up
 * these definitions and skips them.
 */
#include <stdint.h>

/* 0x200 bytes -- matches the Plan's "pad to 0x200" Vehicle size. */
static uint8_t g_stub_vehicle_p1[0x200];

/* P1 always present, P2 NULL (single-player). */
void *puRam000007d0 = g_stub_vehicle_p1;
void *puRam000007d4 = 0;

/* Match-state and pad-state globals that the wait loops poll. */
uint32_t uRam0000062c = 0;     /* P1 pad bits */
uint32_t uRam00000630 = 0;     /* P2 pad bits */
uint8_t  bRam00000015 = 0;     /* g_matchMode -- 0 = quest mode */
