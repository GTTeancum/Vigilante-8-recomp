/* gondola_position.c -- Ski Resort gondola path-position interpolate.
 *
 * Source: SKIRESRT.DLL  FUN_801005e0.
 *
 * Sets the gondola's world position based on a packed 16.16 path-t
 * parameter (param_2):
 *   - if t is in the cable section (& 0x7fff < 0x7000), linearly
 *     interpolate between the two adjacent waypoints on the cable.
 *     Each waypoint is rotated into world coords via GTE_RotateLong-
 *     MatTrans against the path-piece's local matrix.
 *   - else (t >= 0xf000), it's at the terminal pulley: compute the
 *     midpoint of the two pulley anchor points and project a circular
 *     offset using the cos/sin LUT at -0x7ff9f84c/4a.
 *
 * Stores world pos at param_1+0x48/0x4c/0x50, yaw at +0x42.
 *
 * MED.
 */
#include <stdint.h>

extern int  GondolaPath_FindPiece(int piece, uint32_t pickFlag);   /* FUN_8001b038 */
extern void GTE_RotateLongMatTrans(uint32_t *mat, int *src, int *dst);
extern void Object_RefitAABB(int self);
extern uint32_t _DAT_800659fc;

void SK_GondolaPosition(int self, uint16_t t)
{
    *(uint16_t *)(self + 0x46) = t;

    if ((t & 0x7fff) < 0x7000) {
        int tu = (int)(int16_t)t;
        if (tu < 0) tu = 0xfffff000 - tu;
        int *prev = *(int **)(_DAT_800659fc + 0x80);
        int *cur  = (int *)**(int **)(_DAT_800659fc + 0x80);
        while (cur != NULL && (uint32_t)cur[3] < (uint32_t)tu) {
            prev = cur;
            cur  = (int *)*cur;
        }
        int p0  = prev[2];
        int yaw = *(int16_t *)(p0 + 0x42);
        int p1  = cur[2];
        uint32_t frac = ((tu - prev[3]) * 0x100) / (uint32_t)(cur[3] - prev[3]);
        if (yaw < 0) yaw = -yaw;
        uint32_t pick0 = (yaw > 0x400) ? 0x8001u : 0x8000u;
        int wp0 = GondolaPath_FindPiece(p0, pick0);
        int a[4]; int b[4];
        GTE_RotateLongMatTrans((uint32_t *)(intptr_t)(p0 + 0x10), (int *)(intptr_t)(wp0 + 4), a);
        int yaw2 = *(int16_t *)(p1 + 0x42);
        if (yaw2 < 0) yaw2 = -yaw2;
        uint32_t pick1 = (yaw2 > 0x400) ? 0x8001u : 0x8000u;
        if ((int)((uint32_t)t << 16) >= 0) pick1 ^= 1u;
        int wp1 = GondolaPath_FindPiece(p1, pick1);
        GTE_RotateLongMatTrans((uint32_t *)(intptr_t)(p1 + 0x10), (int *)(intptr_t)(wp1 + 4), b);
        int dx = (b[0] - a[0]) * frac; if (dx < 0) dx += 0xff;
        *(int *)(self + 0x48) = a[0] + (dx >> 8);
        int dy = (b[1] - a[1]) * frac; if (dy < 0) dy += 0xff;
        *(int *)(self + 0x4c) = a[1] + (dy >> 8);
    }
    /* Pulley anchor section (independent of above): */
    int base = (t < 0xf000u) ? -0x7ffa0000 : *(int *)(_DAT_800659fc + 0x80);
    int pulley = *(int *)(*(int *)(*(int *)(base + 0x59fc) + 0x88) + 8);
    int p0 = GondolaPath_FindPiece(pulley, 0x8000);
    int p1 = GondolaPath_FindPiece(pulley, 0x8001);
    int mid[3];
    mid[0] = (*(int *)(p0 + 4) + *(int *)(p1 + 4)) / 2;
    mid[1] = (*(int *)(p0 + 8) + *(int *)(p1 + 8)) / 2;
    int dx = *(int *)(p1 + 4) - *(int *)(p0 + 4);
    mid[2] = (*(int *)(p0 + 0xc) + *(int *)(p1 + 0xc)) / 2;
    GTE_RotateLongMatTrans((uint32_t *)(intptr_t)(pulley + 0x10), mid, mid);
    uint32_t yaw = (uint32_t)(-(int16_t)t / 2);
    if (t < 0xf000u) yaw += 0x800;
    *(int16_t *)(self + 0x42) = (int16_t)yaw;
    int sinX = *(int16_t *)((yaw & 0xfff) * 4 + -0x7ff9f84a) * dx;
    if (sinX < 0) sinX += 0x1fff;
    *(int *)(self + 0x48) = mid[0] + (sinX >> 13);
    *(int *)(self + 0x4c) = mid[1];
    int cosX = *(int16_t *)((*(uint16_t *)(self + 0x42) & 0xfff) * 4 + -0x7ff9f84c) * dx;
    if (cosX < 0) cosX += 0x1fff;
    *(int *)(self + 0x50) = mid[2] - (cosX >> 13);
    Object_RefitAABB(self);
}
