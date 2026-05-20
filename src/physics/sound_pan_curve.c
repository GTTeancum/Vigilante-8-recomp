/* sound_pan_curve.c -- 3D-position -> SPU pan/volume curve helpers.
 *
 * Source: SLUS_005.10
 *   FUN_800445b8  -- SfxCurve_2P_Mono  (72 B)
 *   FUN_80044890  -- SfxCurve_1P_Mono  (92 B)
 *   FUN_80044600  -- SfxCurve_2P_Stereo (220 B)
 *   FUN_800448ec  -- SfxCurve_1P_Stereo (208 B)
 *
 * Each consumes a camera-space i32 xyz vector and the global master
 * SFX volume (sRam000008e4 = g_masterSfxVol, i16 in q12) and returns
 * either a single i16 (mono) or a packed (right<<16 | left) i16 pair
 * (stereo) suitable for SPU voice-volume registers.
 *
 * Volume curves (the constants are bit-identical to PSY-Q's hardware-
 * register encoding):
 *
 *   1P-mono: vol scales linearly with (0x200000 - dist), clamped to
 *            zero at dist >= 0x200000 (about 32 m world).  The final
 *            shift `(vol << 7) >> 16` matches PSY-Q's range
 *            normalisation to SPU's 14-bit unsigned domain.
 *
 *   2P-mono: vol = masterVol * (0x200 / (dist >> 12 + 0x200)) -- an
 *            inverse-square-ish fall-off.  Larger 2P player count
 *            requires the level-fall-off to be steeper.
 *
 *   *-Stereo: same magnitude as the mono variant, then a per-channel
 *             multiplier of (0x1000 +/- sin(panAngle)) where
 *             sin(panAngle) = (x / |xyz|) clamped to q12 range.
 *
 * The 2P-stereo variant uses LZCR-based shift normalisation to
 * preserve precision when the distance is small (Vec3_Length can
 * return tiny values where x/dist would otherwise overflow).
 *
 * All four are bit-exact ports of the MIPS bodies in analysis/.
 * Each invocation forwards its `xyz` argument unchanged through to
 * Vec3_Length (the MIPS makes this implicit via the calling convention;
 * we make it explicit).
 *
 * HIGH confidence.
 */
#include <stdint.h>

extern int32_t Vec3_Length(const int32_t *v);   /* FUN_80016a20 */
extern int16_t sRam000008e4;                     /* g_masterSfxVol, i16 q12 */
extern void    gte_ldLZCS(int32_t v);
extern int32_t gte_stLZCR(void);

/* PSY-Q's MIPS uses the pattern
 *      if (x < 0) x += (1<<n)-1;
 *      x >>= n;
 * to get round-toward-zero arithmetic right-shift for signed integers.
 * The compiler emits this for every (signed >> n) the engine wants
 * to be RTZ instead of MIPS's native floor-shift. */
static inline int32_t rsa(int32_t x, int n)
{
    if (x < 0) x += (1 << n) - 1;
    return x >> n;
}

/* HIGH: 2P-mono curve. */
int FUN_800445b8(const int32_t *xyz)
{
    int32_t dist = Vec3_Length(xyz);
    /* Inverse-distance scaled by master vol.  The +0x200000 bias is the
     * "near-field" floor, ~32 m world units.  The division by dist>>12
     * yields a q4-scaled volume; shift+sext to i16. */
    int32_t denom = rsa(dist + 0x200000, 12);
    int32_t v     = ((int32_t)sRam000008e4 << 9) / denom;
    return (int16_t)v;
}

/* HIGH: 1P-mono curve. */
int FUN_80044890(const int32_t *xyz)
{
    int32_t dist = Vec3_Length(xyz);
    int32_t headroom = 0x200000 - dist;
    if (headroom < 0) return 0;
    /* Linear fall-off in [0, 0x200000].  Multiply by master vol,
     * RTZ-shift to land in SPU's 14-bit unsigned vol range. */
    int32_t v = (headroom >> 12) * (int32_t)sRam000008e4;
    v = rsa(v, 9);
    return (v << 7) >> 16;
}

/* HIGH: 1P-stereo curve.  Returns (R << 16) | L i16 pair. */
uint32_t FUN_800448ec(const int32_t *xyz)
{
    int32_t dist = Vec3_Length(xyz);
    if (dist >= 0x200000) return 0;

    /* Magnitude: same linear fall-off as 1P-mono but kept at full
     * precision to fold into LR pan. */
    int32_t mag = rsa(0x200000 - dist, 12) * (int32_t)sRam000008e4;
    mag = rsa(mag, 9);

    /* Pan: sin(panAngle) = x / dist, clamped to q12. */
    int32_t panSin = (dist != 0) ? ((xyz[0] << 12) / dist) : 0;

    /* L = mag * (0x1000 - panSin),  R = mag * (0x1000 + panSin),
     * each RTZ-shifted >>13 (=>>12 from the q12 * q12 product, >>1
     * for the half-pair normalisation). */
    int32_t L = rsa((0x1000 - panSin) * (mag >> 9), 13);
    int32_t R = rsa((0x1000 + panSin) * (mag >> 9), 13);
    return (uint32_t)L | ((uint32_t)R << 16);
}

/* HIGH: 2P-stereo curve.  Same shape as 2P-mono on the magnitude
 * side, with the LZCR-driven shift normalisation that lets the
 * x/dist division retain precision when dist is small. */
uint32_t FUN_80044600(const int32_t *xyz)
{
    int32_t dist  = Vec3_Length(xyz);
    int32_t denom = rsa(dist + 0x200000, 12);
    int32_t mag   = ((int32_t)sRam000008e4 << 9) / denom;

    /* LZCR-based normalisation for the (x << k) / (dist >> (12 - k))
     * division so we don't lose low bits when dist is small. */
    gte_ldLZCS(dist);
    int32_t  lz = gte_stLZCR();
    uint32_t k  = (uint32_t)(lz - 1);
    if (k >= 0xc) k = 0xc;

    int32_t panSin;
    if (dist == 0) {
        panSin = 0;
    } else {
        int32_t num   = xyz[0] << (k & 0x1f);
        int32_t dnorm = dist >> ((0xc - k) & 0x1f);
        panSin = num / dnorm;
    }

    int32_t L = rsa((0x1000 - panSin) * mag, 13);
    int32_t R = rsa((0x1000 + panSin) * mag, 13);
    return (uint32_t)L | ((uint32_t)R << 16);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth -- DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_800445b8 --- */
int FUN_800445b8(void) {
    int iVar1 = FUN_80016a20();
    int iVar2 = iVar1 + 0x200000;
    if (iVar2 < 0) iVar2 = iVar1 + 0x200fff;
    return (int)(short)(((int)sRam000008e4 << 9) / (iVar2 >> 0xc));
}

/* --- SLUS_005.10 FUN_80044890 --- */
int FUN_80044890(void) {
    int iVar1 = FUN_80016a20();
    if (0x200000 - iVar1 < 0) iVar1 = 0;
    else {
        iVar1 = (0x200000 - iVar1 >> 0xc) * (int)sRam000008e4;
        if (iVar1 < 0) iVar1 = iVar1 + 0x1ff;
        iVar1 = (iVar1 << 7) >> 0x10;
    }
    return iVar1;
}

/* --- SLUS_005.10 FUN_800448ec --- */
uint FUN_800448ec(int *param_1) {
    int iVar1 = FUN_80016a20();
    uint uVar2 = 0;
    if (iVar1 < 0x200000) {
        int iVar3 = -iVar1 + 0x200000;
        if (iVar3 < 0) iVar3 = -iVar1 + 0x200fff;
        iVar3 = (iVar3 >> 0xc) * (int)sRam000008e4;
        if (iVar3 < 0) iVar3 = iVar3 + 0x1ff;
        if (iVar1 == 0) iVar1 = 0;
        else            iVar1 = (*param_1 << 0xc) / iVar1;
        int iVar4 = (0x1000 - iVar1) * (iVar3 >> 9);
        if (iVar4 < 0) iVar4 = iVar4 + 0x1fff;
        iVar1 = (iVar1 + 0x1000) * (iVar3 >> 9);
        if (iVar1 < 0) iVar1 = iVar1 + 0x1fff;
        uVar2 = iVar4 >> 0xd | (iVar1 >> 0xd) << 0x10;
    }
    return uVar2;
}

/* --- SLUS_005.10 FUN_80044600 --- */
uint FUN_80044600(int *param_1) {
    int iVar1 = FUN_80016a20();
    int iVar2 = iVar1 + 0x200000;
    if (iVar2 < 0) iVar2 = iVar1 + 0x200fff;
    iVar2 = ((int)sRam000008e4 << 9) / (iVar2 >> 0xc);
    gte_ldLZCS(iVar1);
    int iVar3 = gte_stLZCR();
    uint uVar4 = 0xc;
    if ((int)(iVar3 - 1U) < 0xc) uVar4 = iVar3 - 1U;
    if (iVar1 == 0) iVar1 = 0;
    else            iVar1 = (*param_1 << (uVar4 & 0x1f)) / (iVar1 >> (0xc - uVar4 & 0x1f));
    iVar3 = (0x1000 - iVar1) * iVar2;
    if (iVar3 < 0) iVar3 = iVar3 + 0x1fff;
    iVar2 = (iVar1 + 0x1000) * iVar2;
    if (iVar2 < 0) iVar2 = iVar2 + 0x1fff;
    return iVar3 >> 0xd | (iVar2 >> 0xd) << 0x10;
}

#endif
