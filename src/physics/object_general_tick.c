/* object_general_tick.c -- the engine's universal "movable object"
 * per-frame physics step.
 *
 * Source: SLUS_005.10  FUN_80030c08  (size 812).
 *
 * This is the ONE caller of Object_IntegrateAndOrient in the whole
 * binary -- every dynamic object (AI vehicles, guided projectiles,
 * physics-driven debris) routes its tick through here.  The player
 * vehicle's per-tick path is separate (still being recovered); this
 * function is the contract those siblings share.
 *
 * Per-tick sequence:
 *   1. Probe terrain height under the object's current XYZ
 *      (FUN_8001d748).  Compute (terrainY - posY) and convert it to
 *      a depth-below-ground value scaled by 1/256 ("iVar1").
 *   2. Compute current speed = Vec3_Length(vel) and store
 *      (speed/128) at +0x8c -- the cached speed field used by the
 *      AI / collision pipeline.
 *   3. Pre-bake some ang-velocity components:
 *      - +0x94 (ang_y_pre) = (short at +0xa4) << 6
 *      - +0x90 (ang_x_pre) gets +-0x200 nudges based on a sign
 *        flag in the i16 at +0x1a
 *      - +0x98 (ang_z_pre) gets +-0x200 nudges based on +0x16
 *   4. Drive forward/lateral linear velocity from heading-input
 *      product (i16 at +0x14 * i16 at +0xa6) >> 4, with round-toward-
 *      zero adjustment.  Drag term: 0x1c00 minus a curve dependent on
 *      param_1[0x36] (mass) and the squared terrain-depth (caps at
 *      0x800 to avoid div-by-zero when the object is above ground).
 *   5. Run Object_IntegrateAndOrient (the cleaned, bit-exact
 *      integrator) which writes pos+matrix.
 *   6. Post-integrate damping:
 *      - ang_x/y/z *= 0xf80/0x1000  (0.97 angular damping)
 *      - lin_x/y/z *= 63/64         (~1.5% linear drag per tick)
 *      Both with round-toward-zero adjustments matching the integrator.
 *   7. Decrement 3-element i32 timer array at +0x110.
 *   8. Decrement 3-element i16 timer array at +0x11c.
 *   9. If flag bit 0x800000 is NOT set, maintain the "previous-pos
 *      trail" at +0x48 (used by render and collision):
 *         if (i16 at +0x120) == 0: snap trail to current XYZ
 *         else:                    lerp 1/32 toward current XYZ
 *
 * Per-tick fixed-point format:
 *   - velocity (+0x80,+0x84,+0x88) is q23.8 (engine's "speed unit"
 *     per frame; integrator divides by 128 to convert to position
 *     delta in q16.15).
 *   - angular velocity (+0x90,+0x94,+0x98) is q11.20 (integrator
 *     divides by 128, then GTE small-angle path scales by 1/0x1000).
 *
 * HIGH on structure (matches MIPS line for line); MED on the field
 * NAMES (which i16 is "steerInput" vs "controlPad" requires more
 * cross-function context to nail down).
 */
#include <stdint.h>

extern int     Terrain_HeightAndProbe(intptr_t self, int *posXyz, void *normalOut, uintptr_t *materialOut);  /* FUN_8001d748 */
extern int32_t Vec3_Length(const int32_t *v);                                  /* FUN_80016a20 */
extern void    Object_IntegrateAndOrient(uint8_t *obj);                        /* FUN_80017324 */
extern void   *Host_HeapBase(void);
extern uint32_t Host_HeapSize(void);

/* PSY-Q rounded right shift: matches the engine's idiom
 *   if (x < 0) x += (1<<n)-1;
 *   x >>= n;
 * which is round-toward-zero for negative x (the MIPS arithmetic
 * right-shift would otherwise round toward -infinity). */
static int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static int32_t mips_sll_i32(int32_t v, unsigned sh)
{
    return (int32_t)((uint32_t)v << sh);
}

static int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint64_t)(uint32_t)a * (uint64_t)(uint32_t)b);
}

static int32_t mips_abs_i32(int32_t v)
{
    return v < 0 ? mips_subu_i32(0, v) : v;
}

static inline int32_t rsa(int32_t x, int n)
{
    if (x < 0) x = mips_addu_i32(x, (1 << n) - 1);
    return x >> n;
}

static int object_general_is_host_object_ptr(uint32_t ptr)
{
    uintptr_t base = (uintptr_t)Host_HeapBase();
    uintptr_t end = base + (uintptr_t)Host_HeapSize();
    uintptr_t addr = (uintptr_t)ptr;

    return addr >= base && addr + 8u <= end && ((addr & 3u) == 0);
}

/* HIGH: per-tick step for any movable object that routes through the
 * universal integrator. */
void Object_GeneralTick(uint32_t *obj)
{
    uint8_t *b = (uint8_t *)obj;

    /* --- 1. Terrain probe + depth-below-ground (scaled by 1/256). --- */
    int32_t terrainY = Terrain_HeightAndProbe((intptr_t)obj, (int *)(b + 0x24), NULL, NULL);
    int32_t depth256 = rsa(mips_subu_i32(terrainY, *(int32_t *)(b + 0x28)), 8);

    /* --- 2. Cache speed = |vel| / 128 at +0x8c. --- */
    int32_t speed = Vec3_Length((const int32_t *)(b + 0x80));
    *(int32_t *)(b + 0x8c) = rsa(speed, 7);

    /* --- 3. Pre-bake ang_y from cache field at +0xa4. --- */
    *(int32_t *)(b + 0x94) = mips_sll_i32(*(int16_t *)(b + 0xa4), 6);

    /* Nudge ang_x by +-0x200 based on sign at +0x1a. */
    int16_t steer1a = *(int16_t *)(b + 0x1a);
    *(int32_t *)(b + 0x90) =
        mips_addu_i32(*(int32_t *)(b + 0x90), (steer1a < 1) ? -0x200 : 0x200);

    /* Nudge ang_z by +-0x200 based on sign at +0x16. */
    int16_t steer16 = *(int16_t *)(b + 0x16);
    *(int32_t *)(b + 0x98) =
        mips_addu_i32(*(int32_t *)(b + 0x98), (steer16 < 0) ? 0x200 : -0x200);

    /* --- 4. Lateral input drive: (short@0x14 * short@0xa6) / 16, RTZ. --- */
    int32_t latProd = mips_mult_lo_i32((int16_t)*(uint16_t *)(b + 0x14),
                                       *(int16_t *)(b + 0xa6));
    *(int32_t *)(b + 0x80) = mips_addu_i32(*(int32_t *)(b + 0x80), rsa(latProd, 4));

    /* Drag-style velocity contribution along Y: 0x1c00 minus a curve
     * proportional to (object mass at +0xd8) / (depth*|depth|),
     * clamped to a min divisor of 0x800.  The mult uses i32-wrap
     * semantics (matches MIPS mult-then-mflo). */
    int32_t depthAbs  = mips_abs_i32(depth256);
    int32_t denom     = mips_mult_lo_i32(depth256, depthAbs);
    if (denom < 0x800) denom = 0x800;
    int32_t mass      = *(int32_t *)(b + 0xd8);
    int32_t numerator = mips_mult_lo_i32(mass, 0x1c00);
    *(int32_t *)(b + 0x84) =
        mips_subu_i32(mips_addu_i32(*(int32_t *)(b + 0x84), 0x1c00), numerator / denom);

    /* Long-input drive along the +0x20 input pair: (short@0x20 * short@0xa6) / 16. */
    int32_t longProd = mips_mult_lo_i32((int16_t)*(uint16_t *)(b + 0x20),
                                        *(int16_t *)(b + 0xa6));
    *(int32_t *)(b + 0x88) = mips_addu_i32(*(int32_t *)(b + 0x88), rsa(longProd, 4));

    /* --- 5. Run the universal integrator (bit-exact GTE). --- */
    Object_IntegrateAndOrient(b);

    /* --- 6. Post-integrate damping. --- */
    /* Angular: scale by 0xf80/0x1000 ~ 0.9687, MIPS-equivalent i32 wrap then
     * arithmetic-shift-right by 12.  Original idiom: (v * 0xf80) >> 12. */
    int32_t ax = *(int32_t *)(b + 0x90);
    int32_t ay = *(int32_t *)(b + 0x94);
    int32_t az = *(int32_t *)(b + 0x98);
    *(int32_t *)(b + 0x90) = mips_mult_lo_i32(ax, 0xf80) >> 12;
    *(int32_t *)(b + 0x94) = mips_mult_lo_i32(ay, 0xf80) >> 12;
    *(int32_t *)(b + 0x98) = mips_mult_lo_i32(az, 0xf80) >> 12;

    /* Linear: subtract 1/64 of the velocity with rounded right-shift. */
    int32_t vx = *(int32_t *)(b + 0x80);
    int32_t vy = *(int32_t *)(b + 0x84);
    int32_t vz = *(int32_t *)(b + 0x88);
    *(int32_t *)(b + 0x80) = mips_subu_i32(vx, rsa(vx, 6));
    *(int32_t *)(b + 0x84) = mips_subu_i32(vy, rsa(vy, 6));
    *(int32_t *)(b + 0x88) = mips_subu_i32(vz, rsa(vz, 6));

    /* --- 7. Three-slot i32-keyed timer array at +0x110 (entries are
     * pointers; deref to access the i16 at +6 inside the target). --- */
    for (int i = 0; i < 3; i++) {
        uint32_t nodeWord = *(uint32_t *)(b + 0x110 + i * 4);
        uint8_t *node = object_general_is_host_object_ptr(nodeWord) ?
                        (uint8_t *)(uintptr_t)nodeWord : 0;
        if (node && *(int16_t *)(node + 6) != 0) {
            *(int16_t *)(node + 6) = (int16_t)mips_addu_i32(*(int16_t *)(node + 6), -1);
        }
    }

    /* --- 8. Three-slot i16 timer array at +0x11c (3x i16 in place). --- */
    for (int i = 0; i < 3; i++) {
        int16_t *t = (int16_t *)(b + 0x11c + i * 2);
        if (*t != 0) *t = (int16_t)mips_addu_i32(*t, -1);
    }

    /* --- 9. Previous-pos trail at +0x48..+0x50 unless flag 0x800000. --- */
    if ((obj[0] & 0x800000u) == 0) {
        int32_t px = *(int32_t *)(b + 0x24);
        int32_t py = *(int32_t *)(b + 0x28);
        int32_t pz = *(int32_t *)(b + 0x2c);
        if (*(int16_t *)(b + 0x120) == 0) {
            /* Snap. */
            *(int32_t *)(b + 0x48) = px;
            *(int32_t *)(b + 0x4c) = py;
            *(int32_t *)(b + 0x50) = pz;
        } else {
            /* Lerp by 1/32, RTZ. */
            int32_t tx = *(int32_t *)(b + 0x48);
            int32_t ty = *(int32_t *)(b + 0x4c);
            int32_t tz = *(int32_t *)(b + 0x50);
            *(int32_t *)(b + 0x48) =
                mips_addu_i32(tx, rsa(mips_subu_i32(px, tx), 5));
            *(int32_t *)(b + 0x4c) =
                mips_addu_i32(ty, rsa(mips_subu_i32(py, ty), 5));
            *(int32_t *)(b + 0x50) =
                mips_addu_i32(tz, rsa(mips_subu_i32(pz, tz), 5));
        }
    }
}

void FUN_80030c08(uint32_t *obj)
{
    Object_GeneralTick(obj);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth -- DO NOT EDIT MANUALLY)
 * (full body: analysis/SLUS_005.10/decomp/80030c08.c)
 * ============================================================ */
#if 0

void FUN_80030c08(uint *param_1) {
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint *puVar6;
  uint uVar7;

  iVar1 = FUN_8001d748(param_1, param_1 + 9, 0, 0);
  iVar1 = iVar1 - param_1[10];
  if (iVar1 < 0) iVar1 = iVar1 + 0xff;
  iVar1 = iVar1 >> 8;
  iVar2 = FUN_80016a20(param_1 + 0x20);
  if (iVar2 < 0) iVar2 = iVar2 + 0x7f;
  param_1[0x23] = iVar2 >> 7;
  param_1[0x25] = (int)(short)param_1[0x29] << 6;
  uVar4 = param_1[0x24] + 0x200;
  if (*(short *)((int)param_1 + 0x1a) < 1) uVar4 = param_1[0x24] - 0x200;
  param_1[0x24] = uVar4;
  uVar4 = param_1[0x26] - 0x200;
  if (*(short *)((int)param_1 + 0x16) < 0) uVar4 = param_1[0x26] + 0x200;
  param_1[0x26] = uVar4;
  iVar2 = (int)(short)param_1[5] * (int)*(short *)((int)param_1 + 0xa6);
  if (iVar2 < 0) iVar2 = iVar2 + 0xf;
  iVar3 = iVar1;
  if (iVar1 < 0) iVar3 = -iVar1;
  param_1[0x20] = param_1[0x20] + (iVar2 >> 4);
  iVar2 = 0x800;
  if (0x800 < iVar1 * iVar3) iVar2 = iVar1 * iVar3;
  param_1[0x21] = (param_1[0x21] + 0x1c00) - (int)(param_1[0x36] * 0x1c00) / iVar2;
  iVar1 = (int)(short)param_1[8] * (int)*(short *)((int)param_1 + 0xa6);
  if (iVar1 < 0) iVar1 = iVar1 + 0xf;
  param_1[0x22] = param_1[0x22] + (iVar1 >> 4);
  FUN_80017324(param_1);
  param_1[0x24] = (int)(param_1[0x24] * 0xf80) >> 0xc;
  param_1[0x25] = (int)(param_1[0x25] * 0xf80) >> 0xc;
  param_1[0x26] = (int)(param_1[0x26] * 0xf80) >> 0xc;
  uVar5 = param_1[0x20];
  uVar4 = uVar5;
  if ((int)uVar5 < 0) uVar4 = uVar5 + 0x3f;
  uVar7 = param_1[0x21];
  param_1[0x20] = uVar5 - ((int)uVar4 >> 6);
  uVar4 = uVar7;
  if ((int)uVar7 < 0) uVar4 = uVar7 + 0x3f;
  uVar5 = param_1[0x22];
  param_1[0x21] = uVar7 - ((int)uVar4 >> 6);
  uVar4 = uVar5;
  if ((int)uVar5 < 0) uVar4 = uVar5 + 0x3f;
  param_1[0x22] = uVar5 - ((int)uVar4 >> 6);
  iVar1 = 0;
  puVar6 = param_1;
  do {
    uVar4 = puVar6[0x44];
    if ((uVar4 != 0) && (*(short *)(uVar4 + 6) != 0))
      *(short *)(uVar4 + 6) = *(short *)(uVar4 + 6) + -1;
    iVar1 = iVar1 + 1;
    puVar6 = puVar6 + 1;
  } while (iVar1 < 3);
  iVar1 = 0;
  puVar6 = param_1;
  do {
    if ((short)puVar6[0x47] != 0)
      *(short *)(puVar6 + 0x47) = (short)puVar6[0x47] + -1;
    iVar1 = iVar1 + 1;
    puVar6 = (uint *)((int)puVar6 + 2);
  } while (iVar1 < 3);
  if ((*param_1 & 0x800000) == 0) {
    if ((short)param_1[0x48] == 0) {
      param_1[0x12] = param_1[9];
      param_1[0x13] = param_1[10];
      param_1[0x14] = param_1[0xb];
    } else {
      iVar1 = param_1[9] - param_1[0x12];
      if (iVar1 < 0) iVar1 = iVar1 + 0x1f;
      param_1[0x12] = param_1[0x12] + (iVar1 >> 5);
      iVar1 = param_1[10] - param_1[0x13];
      if (iVar1 < 0) iVar1 = iVar1 + 0x1f;
      param_1[0x13] = param_1[0x13] + (iVar1 >> 5);
      iVar1 = param_1[0xb] - param_1[0x14];
      if (iVar1 < 0) iVar1 = iVar1 + 0x1f;
      param_1[0x14] = param_1[0x14] + (iVar1 >> 5);
    }
  }
}

#endif
