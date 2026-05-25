/* weapon_trail_tick.c -- LAB_8003efc8: per-frame event handler for weapon trail
 * projectile objects (spawned by FUN_8003fdcc / WeaponSpawn_TrailObject).
 *
 * Events handled:
 *   5  -- object removal: free the trail object.
 *   3  -- collision: apply impulse to struck object; flash HUD if target is dead.
 *   *  -- all other events: return 0 (no-op).
 *
 * CONFIDENCE: HIGH -- reconstructed from MIPS; field offsets and scale factors
 * verified against analysis/SLUS_005.10/decomp/8003efc8.c.
 */

#include <stdint.h>

/* Vec3_Delta: compute int16_t[3] direction from 'from' to 'to' world positions. */
extern void FUN_80016bd8(int16_t *out, int32_t *from, int32_t *to);

/* Object_ApplyImpulseAtPoint: add force vector at world position to obj dynamics. */
extern void FUN_800176f8(int obj, int32_t *force, int32_t *pos);

/* HudFlash_SetEntry: trigger a damage flash entry in the HUD. */
extern void FUN_80012068(int idx, int a, int b, int c);

/* Object_FreeAndUnregister: remove and free an object from the scene. */
extern void FUN_800205f8(int obj);

int LAB_8003efc8(int obj, int event, int *event_data)
{
    int       collide_struct;
    uint8_t   type;
    int16_t   delta[3];
    int32_t   force[3];
    int16_t   status;

    if (event == 5) {
        FUN_800205f8(obj);
        return -1;
    }

    if (event != 3)
        return 0;

    /* Event 3: collision.
     * event_data[0] points to the collision info struct. */
    collide_struct = *event_data;

    /* Byte [4] of the collision struct is the collision type;
     * only type 2 (vehicle body) triggers the impulse. */
    type = ((uint8_t *)(uintptr_t)collide_struct)[4];
    if (type != 2)
        return 0;

    /* Compute the normalised direction from our position to the collision point. */
    FUN_80016bd8(delta,
                 (int32_t *)(uintptr_t)(obj + 0x24),
                 (int32_t *)(uintptr_t)(collide_struct + 0x24));

    /* Scale: x and z *= 24 (3*8), y *= 12 (3*4). */
    force[0] = (int32_t)delta[0] * 24;
    force[1] = (int32_t)delta[1] * 12;
    force[2] = (int32_t)delta[2] * 24;

    /* Apply the impulse to the struck object at our spawn position. */
    FUN_800176f8(collide_struct, force, (int32_t *)(uintptr_t)(obj + 0x24));

    /* If the struck object already has a negative status word (i.e. it is
     * dying / dead), flash the HUD.  The bitwise NOT of the status value
     * is used as the HUD slot index (matches nor a0,zero,a0 in MIPS). */
    status = *(int16_t *)(uintptr_t)(collide_struct + 6);
    if (status < 0)
        FUN_80012068(~(int)(unsigned short)status, 255, 2, 128);

    return 0;
}
