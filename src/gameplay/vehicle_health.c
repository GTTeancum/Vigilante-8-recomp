/* vehicle_health.c -- decoded vehicle/slot health seeding helper.
 *
 * Source: SLUS_005.10 gap 0x8002e604.
 */
#include <stdint.h>

void FUN_8002e604(uint32_t *vehicle, int health)
{
    *(uint16_t *)((uint8_t *)vehicle + 0x0e) = (uint16_t)health;
    *(uint16_t *)((uint8_t *)vehicle + 0x0c) = (uint16_t)health;

    for (int i = 0; i < 3; i++) {
        uint32_t *slot = (uint32_t *)(uintptr_t)
            *(uint32_t *)((uint8_t *)vehicle + 0xec + i * 4);
        if (slot) {
            *(uint16_t *)((uint8_t *)slot + 0x0c) = (uint16_t)health;
        }
    }
}
