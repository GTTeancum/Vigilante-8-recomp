/* spatial_tracker.c -- world spatial tracker attachment helpers.
 *
 * Source: SLUS_005.10 FUN_8003e598.
 *
 * MED: walks an object's collision-tree leaf records, computes the maximum
 * world-space X/Z bounds, allocates a tracker record, and stores it at obj+0x70.
 */
#include <stdint.h>

extern void *FUN_8003e254(uint32_t slot, int maxX, int maxZ);

void FUN_8003e598(uint32_t *obj, void *slot)
{
    uint8_t *object = (uint8_t *)obj;
    uint16_t *node = (uint16_t *)(uintptr_t)*(uint32_t *)(object + 0x5c);
    int first = 1;
    int minX = 0;
    int minY = 0;
    int minZ = 0;
    int maxX = 0;
    int maxY = 0;
    int maxZ = 0;

    if (node == 0)
        return;

    int guard = 0;
    while (*node != 0 && guard++ < 256) {
        if (*node == 1) {
            int *box = (int *)((uint8_t *)node + 4);

            if (first) {
                minX = box[0];
                minY = box[1];
                minZ = box[2];
                maxX = box[3];
                maxY = box[4];
                maxZ = box[5];
                first = 0;
            } else {
                if (box[0] < minX)
                    minX = box[0];
                if (box[1] < minY)
                    minY = box[1];
                if (box[2] < minZ)
                    minZ = box[2];
                if (box[3] > maxX)
                    maxX = box[3];
                if (box[4] > maxY)
                    maxY = box[4];
                if (box[5] > maxZ)
                    maxZ = box[5];
            }
            node = (uint16_t *)((uint8_t *)node + 0x1c);
        } else if (*node == 2) {
            node = (uint16_t *)((uint8_t *)node + 4 + (uint32_t)node[1] * 12u);
        } else {
            break;
        }
    }

    (void)minX;
    (void)minY;
    (void)minZ;
    (void)maxY;

    if (!first) {
        *(uint32_t *)(object + 0x70) =
            (uint32_t)(uintptr_t)FUN_8003e254((uint32_t)(uintptr_t)slot, maxX, maxZ);
    }
}
