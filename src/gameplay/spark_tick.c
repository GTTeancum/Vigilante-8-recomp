/* spark_tick.c -- LAB_8003ee88: per-frame tick for spark/glow particle objects.
 *
 * Object layout (fields accessed here):
 *   obj+0x24  uint8_t[4]  {r, g, b, vel}  base colour and advance rate
 *   obj+0x2c  int32_t     progress         cycle position [0..255]
 *
 * DAT_80065984 is the packed background colour word (XBGR byte order in PSX
 * VRAM -- byte 0=R, 1=G, 2=B).  Spark adds a triangle-wave-scaled fraction of
 * the object colour on top each frame.
 *
 * CONFIDENCE: HIGH -- bit-exact MIPS reconstruction including RTZ >>7 and the
 * MIPS branch-delay-slot red-channel clamping artefact.
 */

#include <stdint.h>

/* Background colour word; individual bytes accessed as R/G/B channels. */
extern uint32_t DAT_80065984;

/* Object_FreeAndUnregister -- remove a spark object from the scene. */
extern void FUN_800205f8(int obj);

/* RTZ (round-toward-zero) arithmetic right shift by n bits.
 * MIPS emits: if (x < 0) x += (1 << n) - 1; x >>= n;
 * For n=7: bias = 127. */
static int rtz_sra7(int x)
{
    if (x < 0)
        x += 127;
    return x >> 7;
}

int LAB_8003ee88(int obj, int event, int a2)
{
    uint8_t  *t0;          /* obj+0x24 */
    int32_t   progress;
    int        delta, blend;
    int        obj_r, obj_g, obj_b, obj_vel;
    uint8_t   *bg;
    int        bg_r, bg_g, bg_b;
    int        val, new_r, new_g, new_b;
    uint32_t   packed;

    if (event != 0 || a2 == 0)
        return 0;

    t0       = (uint8_t *)(uintptr_t)(obj + 0x24);
    progress = *(int32_t *)(uintptr_t)(obj + 0x2c);
    obj_b    = t0[2];   /* obj+0x26 */

    /* Triangle wave: peak at progress==128, zero at 0 and 256. */
    delta = progress - 128;
    if (delta < 0)
        delta = -delta;
    blend = 128 - delta;   /* [0..128] */

    bg = (uint8_t *)&DAT_80065984;

    /* --- Blue channel --- */
    bg_b   = bg[2];
    val    = blend * obj_b;
    new_b  = bg_b + rtz_sra7(val);
    if (new_b > 255) new_b = 255;
    packed = (uint32_t)new_b << 16;

    /* --- Green channel --- */
    obj_g  = t0[1];   /* obj+0x25 */
    bg_g   = bg[1];
    val    = blend * obj_g;
    new_g  = bg_g + rtz_sra7(val);
    if (new_g > 255) new_g = 255;
    packed |= (uint32_t)new_g << 8;

    /* --- Red channel --- */
    obj_r  = t0[0];   /* obj+0x24 */
    bg_r   = bg[0];
    val    = blend * obj_r;
    new_r  = bg_r + rtz_sra7(val);
    if (new_r > 255) new_r = 255;
    packed |= (uint32_t)new_r;

    DAT_80065984 = packed;

    /* Advance progress counter. */
    obj_vel   = t0[3];   /* obj+0x27 */
    progress += obj_vel * a2;
    *(int32_t *)(uintptr_t)(obj + 0x2c) = progress;

    if (progress < 256)
        return 0;

    /* Particle lifetime expired -- free the object. */
    FUN_800205f8(obj);
    return -1;
}
