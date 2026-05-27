/* mine_dead_ticks.c -- rolling mine post-explosion tick callbacks.
 *
 * Source: SLUS_005.10
 *   LAB_800321C0  0x800321C0  (~280 instructions) -- normal mine variant
 *   LAB_8003277C  0x8003277C  (~190 instructions) -- large mine variant
 *
 * Both functions are tick callbacks installed after a mine transitions from
 * its initial armed state to a post-detonation rolling/tracking state.
 *
 * LAB_800321C0 (normal mine / Spider-Bomb wtype-2 armed rolling stage):
 *   Rolls toward a homing target using rotation-matrix column vectors to drive
 *   velocity in each axis (X,Y,Z).  Steers via terrain-ahead lookahead and
 *   angular velocity.  When lifetime expires: explosion.
 *
 * LAB_8003277C (large mine / wtype-3 area proxy):
 *   Similar rolling physics but no homing or Y-axis velocity correction.
 *   Maintains a "proxy" collision-sphere object at obj[0x84] whose position is
 *   kept synchronised with self each tick.  When lifetime expires: explosion.
 *
 * Event dispatch (LAB_800321C0):
 *   0  -> roll toward target; terrain evasion; smoke/spark effects; lifetime
 *          countdown.  Returns -1 on expire/terrain, 0 otherwise.
 *   3  -> collision: if DAT_80065908 & 0x400: spawn trail obj + sound + retire.
 *          else: WeaponHit_Apply(obj, param3, 26, 55).
 *   4  -> die: if obj[0] & 0x10000, clear bit 0x800000 from obj[0x84][0].
 *   9  -> target unlock: if obj[0x84]==param3, set obj[0x84]=obj[0x80].
 *
 * Event dispatch (LAB_8003277C):
 *   0  -> roll (no homing); sync proxy position; spark every 4 ticks;
 *          lifetime countdown.  Returns -1 on expire/terrain, 0 otherwise.
 *   3  -> collision: if collider_type==7 return 0; else WeaponHit_Apply(26,55).
 *   4  -> die: clear bit 0x800000 from obj[0x84][0].
 *
 * Object field layout (shared):
 *   +0x00 (0)   : flags word (bit 0x10000 = die-event clear-flag)
 *   +0x09 (9)   : frame phase byte (low byte; compared vs global frame counter)
 *   +0x10 (16)  : 3×3 rotation matrix (int16 × 9 = 18 bytes)
 *   +0x14 (20)  : matrix col[0][2] → X velocity target
 *   +0x1a (26)  : matrix col[1][2] → Y velocity target (normal only)
 *   +0x20 (32)  : matrix col[2][2] → Z velocity target
 *   +0x24 (36)  : render pos X
 *   +0x28 (40)  : render pos Y
 *   +0x2c (44)  : render pos Z
 *   +0x48 (72)  : world pos X
 *   +0x4c (76)  : world pos Y
 *   +0x50 (80)  : world pos Z
 *   +0x80 (128) : prev_target object handle
 *   +0x84 (132) : curr_target / proxy-sphere object handle
 *   +0x88 (136) : velocity X
 *   +0x8c (140) : velocity Y
 *   +0x90 (144) : velocity Z
 *   +0x94 (148) : lifetime countdown (uint16)
 *
 * HIGH confidence: direct MIPS disassembly port.
 */
#include <stdint.h>

/* FUN_80025400 -- Terrain_HeightAt: return terrain Y at (X, Z). */
extern int FUN_80025400(int x, int z);

/* FUN_8004410c -- Audio_AllocVoice. */
extern int FUN_8004410c(void);

/* FUN_8004483c -- Audio_VoicePlay: (voice, bank, pitch, pos). */
extern void FUN_8004483c(int voice, uint32_t bank, int pitch, const void *pos);

/* FUN_8003fd24 -- Debris_Spawn: spawn impact/explosion particle. */
extern int FUN_8003fd24(const int32_t *xyz, int kind);

/* FUN_80020620 -- ColEvent_Dispatch. */
extern void FUN_80020620(intptr_t obj, uint32_t event);

/* FUN_800205f8 -- Object_RetireDeferred. */
extern void FUN_800205f8(intptr_t obj);

/* FUN_80031454 -- WeaponHit_Apply. */
extern int FUN_80031454(intptr_t obj, intptr_t *collider, uint16_t radius, int strength);
extern uintptr_t Collision_QueryHostWord(const void *query, uint32_t index);

/* FUN_800439b8 -- RotMatrix_ApplyAngVel: update rotation matrix with angular vel. */
extern void FUN_800439b8(int *matrix, int ax, int ay, int az);

/* FUN_8004c934 -- MatrixNormal: renormalise a rotation matrix. */
extern void FUN_8004c934(int *mat_a, int *mat_b);

/* FUN_8003fdcc -- WeaponSpawn_TrailObject: allocate weapon trail particle object. */
extern uint32_t *FUN_8003fdcc(uint32_t *param_1, uint16_t param_2, int param_3);

/* FUN_8001d840 -- Object_AlignToTerrainNormal: align object to terrain at pos. */
extern void FUN_8001d840(uint32_t param_1, uint32_t *param_2, int param_3);

/* FUN_8001d708 -- Object_InitBoneMatrix: init bone animation for particle. */
extern void FUN_8001d708(int obj);

/* FUN_4352c -- GTE_RotateVecTranspose: rotate 3-element vector by transposed GTE
 * matrix (loaded by FUN_8004779c).  (src_matrix, src_vec, dst_vec) */
extern int *FUN_8004352c(int *mat, int *src, int *dst);

/* FUN_8004779c -- Mine_NormaliseDelta: normalise a 64-bit squared-distance
 * product to produce a steering scale factor for terrain evasion.
 * (lo, hi, numerator, sign) → normalised int result. */
extern int FUN_8004779c(uint32_t lo, int hi, int numerator, int sign);

/* FUN_8004ecd4 -- Mine_SteerAngle: compute angular-velocity component from
 * normalised delta and perpendicular component.
 * (delta_neg, delta_perp) → clamped angle. */
extern int FUN_8004ecd4(int delta_neg, int delta_perp);

/* FUN_8004d154 -- Matrix_Copy: copy 18-byte (3×3 int16) rotation matrix.
 * (src_matrix, dst_matrix) */
extern void FUN_8004d154(int *src, int *dst);

/* DAT_800658FC -- sound bank (gp+1528). */
extern uint32_t DAT_800658FC;

/* DAT_80065908 -- collision-flags word (gp+0x604 = gp+1540).
 * Bit 0x400: mine hit creates a trail object instead of normal weapon hit. */
extern uint32_t DAT_80065908;

/* gp+12 (offset 12 from the GP register = global frame counter low word).
 * Used to phase smoke/spark emission: every 32 ticks (0x1f mask),
 * every 4 ticks (0x3 mask). */
extern int32_t DAT_80065310;   /* gp+12 = frame counter (PSX GP=0x80065304, +12=0x80065310) */

/* ------------------------------------------------------------------ */

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
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

static int32_t mips_neg_i32(int32_t v)
{
    return mips_subu_i32(0, v);
}

/* Compute (int16_matrix_element * 15258) >> 12  with RTZ rounding.
 * This maps a Q12 unit-rotation component to a velocity target. */
static int32_t mine_vel_target(int16_t mat_elem)
{
    int32_t v = (int32_t)mat_elem;
    /* Multiply by 15258 using PSX shift-add sequence:
     * v * 15258 = v * (2*(5*(32*1-1)*16-1)*2+1)*... == v * 15258. */
    int32_t v1 = mips_addu_i32(mips_sll_i32(v, 2), v);
    v1 = mips_subu_i32(mips_sll_i32(v1, 5), v);        /* * 159 */
    v1 = mips_subu_i32(mips_sll_i32(v1, 4), v);        /* * 2543 */
    int32_t v0 = mips_sll_i32(mips_subu_i32(mips_sll_i32(v1, 2), v1), 1);
    /* RTZ arithmetic right-shift by 12. */
    if (v0 < 0) v0 = mips_addu_i32(v0, 4095);
    return v0 >> 12;
}

/* RTZ arithmetic right-shift by 4. */
static int32_t rtz_sra4(int32_t v)
{
    if (v < 0) v = mips_addu_i32(v, 15);
    return v >> 4;
}

/* RTZ arithmetic right-shift by 3. */
static int32_t rtz_sra3(int32_t v)
{
    if (v < 0) v = mips_addu_i32(v, 7);
    return v >> 3;
}

/* Clamp to [-256, 256]. */
static int32_t clamp256(int32_t v)
{
    if (v < -256) return -256;
    if (v >  256) return  256;
    return v;
}

/* ================================================================== */
/* LAB_800321C0 -- MineDead_Tick (normal mine variant)               */
/* ================================================================== */

intptr_t LAB_800321C0(intptr_t obj, int event, intptr_t param3)
{
    uint8_t *s2 = (uint8_t *)(uintptr_t)obj;

    /* ---- event dispatch ---- */
    if (event == 3) goto event3;
    if ((uint32_t)event < 4u) {
        if (event == 0) goto event0;
        /* event 1,2: no-op */
        goto return0;
    }
    if (event == 4) goto event4;
    if (event == 9) goto event9;
    goto return0;

event0: {
    /* ----- Tick ----- */
    int32_t *mat = (int32_t *)(s2 + 0x10);
    int32_t *vel = (int32_t *)(s2 + 0x88);
    int32_t *pos = (int32_t *)(s2 + 0x48);

    /* X velocity steering from matrix column element obj+0x14. */
    int16_t mat14 = *(int16_t *)(s2 + 0x14);
    int32_t target_x = mine_vel_target(mat14);
    int32_t delta_x  = rtz_sra4(mips_subu_i32(target_x, vel[0]));
    delta_x = clamp256(delta_x);

    /* Y velocity steering from matrix column element obj+0x1a (/8 divisor). */
    int16_t mat1a = *(int16_t *)(s2 + 0x1a);
    int32_t target_y = mine_vel_target(mat1a);
    int32_t raw_dy   = mips_subu_i32(target_y, vel[1]);
    int32_t delta_y  = rtz_sra3(raw_dy);

    /* Z velocity steering from matrix column element obj+0x20. */
    int16_t mat20 = *(int16_t *)(s2 + 0x20);
    int32_t target_z = mine_vel_target(mat20);
    int32_t delta_z  = rtz_sra4(mips_subu_i32(target_z, vel[2]));
    delta_z = clamp256(delta_z);

    /* Apply velocity deltas and integrate into position. */
    vel[0] = mips_addu_i32(vel[0], delta_x);
    vel[1] = mips_addu_i32(vel[1], delta_y);
    vel[2] = mips_addu_i32(vel[2], delta_z);

    pos[0] = mips_addu_i32(pos[0], vel[0]);
    pos[1] = mips_addu_i32(pos[1], vel[1]);
    pos[2] = mips_addu_i32(pos[2], vel[2]);

    /* Terrain ahead: check 16 ticks forward. */
    int terrain_ahead = FUN_80025400(mips_addu_i32(pos[0], mips_sll_i32(vel[0], 4)),
                                     mips_addu_i32(pos[2], mips_sll_i32(vel[2], 4)));
    /* Local stack storage for delta vector to target. */
    int32_t delta_vec[3]; /* sp+16 = dx, sp+20 = dy, sp+24 = dz */

    /* Terrain evasion vs. target homing. */
    int32_t pos_y_margin = mips_addu_i32(pos[1], 20480);
    int32_t terrain_excess = mips_subu_i32(terrain_ahead, pos_y_margin);
    /* sp+20 = target dy (needed in both branches; load now). */
    int32_t *tgt_obj = (int32_t *)(uintptr_t)(uint32_t)(*(int32_t *)(s2 + 0x84));
    int32_t dx = mips_subu_i32(tgt_obj[0x48/4], pos[0]);           /* target.x - self.x */
    /* Target Y with half-velocity prediction. */
    int32_t *tgt_child = (int32_t *)(uintptr_t)(uint32_t)(*(int32_t *)((uint8_t *)tgt_obj + 0x5c));
    int32_t tgt_child_vy = *(int32_t *)((uint8_t *)tgt_child + 8);
    /* round-toward-zero divide by 2 */
    tgt_child_vy = mips_addu_i32(tgt_child_vy, (int32_t)((uint32_t)tgt_child_vy >> 31)) >> 1;
    int32_t dy = mips_addu_i32(mips_subu_i32(tgt_obj[0x4c/4], pos[1]), tgt_child_vy);
    int32_t dz = mips_subu_i32(tgt_obj[0x50/4], pos[2]);
    delta_vec[0] = dx;
    delta_vec[1] = dy;
    delta_vec[2] = dz;

    /* ang_y, ang_x for RotMatrix_ApplyAngVel. */
    int32_t ang_y = 0, ang_x = 0;

    if (terrain_excess >= dy) {
        /* Terrain NOT blocking: homing toward target. */
    } else {
        /* Terrain-evasion: pick the smaller-magnitude horizontal axis and
         * produce a perpendicular steering torque via FUN_8004779c. */
        int32_t abs_dz = dz < 0 ? mips_neg_i32(dz) : dz;
        int32_t abs_dx = dx < 0 ? mips_neg_i32(dx) : dx;
        int32_t mult_result, norm_factor, norm_sign;
        if (abs_dx < abs_dz) {
            /* Use dx as perpendicular weight. */
            int64_t prod = (int64_t)terrain_excess * dx;
            mult_result = (int32_t)((uint64_t)prod);
            int hi       = (int)((uint64_t)prod >> 32);
            norm_factor  = dz;
            norm_sign    = (int32_t)((uint32_t)dz >> 31);
            ang_y = FUN_8004779c((uint32_t)mult_result, hi, norm_factor, norm_sign);
        } else {
            /* Use dz as perpendicular weight. */
            int64_t prod = (int64_t)terrain_excess * dz;
            mult_result = (int32_t)((uint64_t)prod);
            int hi       = (int)((uint64_t)prod >> 32);
            norm_factor  = dz;
            norm_sign    = (int32_t)((uint32_t)dz >> 31);
            ang_y = FUN_8004779c((uint32_t)mult_result, hi, norm_factor, norm_sign);
        }
        /* GTE: rotate delta vector by transposed matrix into body frame. */
        FUN_8004352c(mat, delta_vec, delta_vec);

        /* Compute angular velocity component from perpendicular. */
        ang_y = FUN_8004ecd4(mips_neg_i32(ang_y), delta_vec[2]);
        ang_y = clamp256(ang_y);
    }

    /* ang_x: ±256 based on sign of dx (roll forward/backward). */
    ang_x = (delta_vec[0] > 0) ? 256 : -256;

    /* Update rotation matrix. */
    FUN_800439b8(mat, ang_y, ang_x, 0);

    /* Mirror world pos to render pos. */
    *(int32_t *)(s2 + 0x24) = pos[0];
    *(int32_t *)(s2 + 0x28) = pos[1];
    *(int32_t *)(s2 + 0x2c) = pos[2];

    /* Smoke emission: renormalise matrix every 32 ticks. */
    uint8_t frame_phase = s2[9];
    int32_t frame_ctr   = DAT_80065310;
    if (((uint32_t)(frame_ctr - frame_phase) & 0x1fu) == 0u) {
        FUN_8004c934(mat, mat);
    }

    /* Spark/dust emission every 4 ticks. */
    uint16_t life = *(uint16_t *)(s2 + 0x94);
    if ((life & 0x3u) == 0u) {
        int particle = FUN_8003fd24((const int32_t *)pos, 9);
        /* Set particle flag and animation frame. */
        *(int32_t *)(uintptr_t)(uint32_t)particle |= 0x400;
        int16_t life_s = *(int16_t *)(s2 + 0x94);
        *(int16_t *)(uintptr_t)(uint32_t)(particle + 0x44) =
            (int16_t)mips_mult_lo_i32((int32_t)life_s, 96);
        FUN_8001d708(particle);
    }

    /* Decrement lifetime. */
    life = (uint16_t)mips_subu_i32((uint16_t)*(uint16_t *)(s2 + 0x94), 1);
    *(uint16_t *)(s2 + 0x94) = life;
    if ((uint32_t)life << 16) goto return0;

    /* Lifetime zero: explosion + retire. */
    {
        int32_t *spos = (int32_t *)(s2 + 0x48);
        int voice = FUN_8004410c();
        FUN_8004483c(voice, DAT_800658FC, 55, spos);
        FUN_8003fd24(spos, 26);
        FUN_800205f8(obj);
        return -1;
    }
}

event3: {
    /* ----- Collision ----- */
    int32_t *spos = (int32_t *)(s2 + 0x48);

    if (DAT_80065908 & 0x400u) {
        /* Armed trail path: spawn trail object at pos, retire self. */
        int32_t *pos72 = (int32_t *)(s2 + 0x48);
        uint32_t *trail = FUN_8003fdcc((uint32_t *)pos72, 46u, 40);
        uint8_t *trp = (uint8_t *)(uintptr_t)(uint32_t)(uintptr_t)trail;

        /* Initialise trail object fields. */
        *(int16_t *)(trp + 0x40) = 2048;
        *(int16_t *)(trp + 0x42) = 0;
        *(int16_t *)(trp + 0x44) = 0;  /* (in delay slot before FUN_8001d708) */
        FUN_8001d708((int)(uintptr_t)trail);

        /* Copy self's rotation matrix to the trail object. */
        FUN_8004d154((int *)(s2 + 0x10), (int *)(trp + 0x10));

        int voice = FUN_8004410c();
        FUN_8004483c(voice, DAT_800658FC, 54, spos);

        FUN_800205f8(obj);
        return -1;
    }

    /* Standard weapon hit. */
    return FUN_80031454(obj, (intptr_t *)(uintptr_t)param3, 26u, 55);
}

event4: {
    /* ----- Die ----- */
    /* If obj has flag 0x10000, clear bit 0x800000 from the current target. */
    if (*(int32_t *)s2 & 0x10000) {
        uint8_t *tgt = (uint8_t *)(uintptr_t)(uint32_t)(*(int32_t *)(s2 + 0x84));
        *(int32_t *)tgt &= (int32_t)0xff7fffffu;
    }
    goto return0;
}

event9: {
    /* ----- Target unlock ----- */
    /* If obj[0x84] == param3, set obj[0x84] = obj[0x80]. */
    if ((intptr_t)*(int32_t *)(s2 + 0x84) == param3) {
        *(int32_t *)(s2 + 0x84) = *(int32_t *)(s2 + 0x80);
    }
    /* fall through */
}

return0:
    return 0;
}


/* ================================================================== */
/* LAB_8003277C -- MineDead_Tick (large mine / area-proxy variant)   */
/* ================================================================== */

intptr_t LAB_8003277C(intptr_t obj, int event, intptr_t param3)
{
    uint8_t *s1 = (uint8_t *)(uintptr_t)obj;

    /* ---- event dispatch ---- */
    if (event == 3) goto large_event3;
    if ((uint32_t)event < 4u) {
        if (event == 0) goto large_event0;
        goto large_return0;
    }
    if (event == 4) goto large_event4;
    goto large_return0;

large_event0: {
    /* ----- Tick ----- */
    int32_t *mat = (int32_t *)(s1 + 0x10);
    int32_t *vel = (int32_t *)(s1 + 0x88);
    int32_t *pos = (int32_t *)(s1 + 0x48);

    /* X velocity steering from matrix column element obj+0x14.
     * Delta is divided by 16 (sra 4) for gradual steering. */
    int16_t mat14 = *(int16_t *)(s1 + 0x14);
    int32_t target_x = mine_vel_target(mat14);
    /* raw_delta >>= 4 with rounding depending on sign. */
    int32_t raw_dx = mips_subu_i32(target_x, vel[0]);
    int32_t delta_x;
    if (raw_dx >= 0) {
        /* Clamp to 256 first, then shift. */
        delta_x = raw_dx >> 4;
        if (delta_x > 256) delta_x = 256;
        if (delta_x < -256) delta_x = -256;
    } else {
        int32_t rounded = mips_addu_i32(raw_dx, 15);
        delta_x = rounded >> 4;
        if (delta_x > 256) delta_x = 256;
        if (delta_x < -256) delta_x = -256;
    }
    vel[0] = mips_addu_i32(vel[0], delta_x);
    /* Integrate pos.x += vel.x now (large mine integrates in-line). */
    pos[0] = mips_addu_i32(pos[0], vel[0]);

    /* Z velocity steering from matrix column element obj+0x20. */
    int16_t mat20 = *(int16_t *)(s1 + 0x20);
    int32_t target_z = mine_vel_target(mat20);
    int32_t raw_dz = mips_subu_i32(target_z, vel[2]);
    int32_t delta_z;
    if (raw_dz >= 0) {
        delta_z = raw_dz >> 4;
        if (delta_z > 256) delta_z = 256;
        if (delta_z < -256) delta_z = -256;
    } else {
        int32_t rounded = mips_addu_i32(raw_dz, 15);
        delta_z = rounded >> 4;
        if (delta_z > 256) delta_z = 256;
        if (delta_z < -256) delta_z = -256;
    }
    vel[2] = mips_addu_i32(vel[2], delta_z);
    /* Integrate pos.z += vel.z. */
    pos[2] = mips_addu_i32(pos[2], vel[2]);

    /* Terrain collision check (pos already updated). */
    int terrain_y = FUN_80025400(pos[0], pos[2]);
    int32_t *s0_mat = (int32_t *)(s1 + 0x16); /* s0 = s1+16 (matrix base+2) */
    (void)s0_mat;

    if (terrain_y < pos[1]) {
        /* Below terrain: impact. */
        int voice = FUN_8004410c();
        int32_t *spos = (int32_t *)(s1 + 0x48);  /* s0 reassigned in delay slot */
        FUN_8004483c(voice, DAT_800658FC, 55, spos);
        FUN_8003fd24(spos, 12);
        FUN_80020620(obj, 1u);
        return -1;
    }

    /* No terrain impact: constant rolling animation (256 units around X axis).
     * Mirror positions to render slots first. */
    *(int32_t *)(s1 + 0x24) = pos[0];
    *(int32_t *)(s1 + 0x28) = pos[1];
    *(int32_t *)(s1 + 0x2c) = pos[2];

    /* RotMatrix_ApplyAngVel(matrix, 0, 256, 0) -- rolling. */
    FUN_800439b8((int *)(s1 + 0x10), 0, 256, 0);

    /* Smoke: renormalise matrix every 32 ticks. */
    uint8_t frame_phase = s1[9];
    int32_t frame_ctr   = DAT_80065310;
    if (((uint32_t)(frame_ctr - frame_phase) & 0x1fu) == 0u) {
        FUN_8004c934((int *)(s1 + 0x10), (int *)(s1 + 0x10));
    }

    /* Spark emission every 4 ticks (kind 18 for large mine). */
    uint16_t life = *(uint16_t *)(s1 + 0x94);
    if ((life & 0x3u) == 0u) {
        int particle = FUN_8003fd24((const int32_t *)(s1 + 0x48), 18);
        *(int32_t *)(uintptr_t)(uint32_t)particle |= 0x400;
        int16_t life_s = *(int16_t *)(s1 + 0x94);
        *(int16_t *)(uintptr_t)(uint32_t)(particle + 0x44) =
            (int16_t)mips_mult_lo_i32((int32_t)life_s, 96);
        FUN_8001d708(particle);
    }

    /* Synchronise proxy collision sphere to self's world position. */
    {
        int32_t *proxy = (int32_t *)(uintptr_t)(uint32_t)(*(int32_t *)(s1 + 0x84));
        proxy[0x48/4] = pos[0];
        proxy[0x4c/4] = pos[1];
        proxy[0x50/4] = pos[2];
    }

    /* Decrement lifetime. */
    life = (uint16_t)mips_subu_i32((uint16_t)*(uint16_t *)(s1 + 0x94), 1);
    *(uint16_t *)(s1 + 0x94) = life;
    if ((uint32_t)life << 16) goto large_return0;

    /* Lifetime zero: explosion + retire. */
    {
        int32_t *spos2 = (int32_t *)(s1 + 0x48);
        int voice2 = FUN_8004410c();
        FUN_8004483c(voice2, DAT_800658FC, 55, spos2);
        FUN_8003fd24(spos2, 26);
        FUN_800205f8(obj);
        return -1;
    }
}

large_event3: {
    /* ----- Collision ----- */
    uint8_t *collider = (uint8_t *)Collision_QueryHostWord((void *)(uintptr_t)param3, 0);
    uint8_t collider_type = collider[4];

    /* Type 7 (mine or proxy): ignore to prevent self-collision. */
    if (collider_type == 7u)
        goto large_return0;

    return FUN_80031454(obj, (intptr_t *)(uintptr_t)param3, 26u, 55);
}

large_event4: {
    /* ----- Die ----- */
    /* Clear bit 0x800000 from the proxy sphere's flags unconditionally. */
    {
        int32_t *proxy = (int32_t *)(uintptr_t)(uint32_t)(*(int32_t *)(s1 + 0x84));
        *(int32_t *)proxy &= (int32_t)0xff7fffffu;
    }
    /* fall through */
}

large_return0:
    return 0;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0
/* --- SLUS_005.10  LAB_800321C0  (normal mine, manual disassembly reference) ---
 *
 * event dispatch:
 *   800321d8: beq a1,v0(=3),event3
 *   800321e4: sltiu v0,a1,4; beq→check 0/4
 *   800321ec: beq a1,zero,event0
 *   800321f4: j return0
 *   800321fc: beq a1,v0(=4),event4
 *   80032204: beq a1,v0(=9),event9
 *
 * event0 (velocity steering):
 *   lh v0,20(s2)        ; mat[0][2] = obj+0x14 (X)
 *   multiply × 15258; RTZ >> 12 → target_vel_x
 *   lw v1,136(s2)       ; vel.x
 *   subu v0,v0,v1       ; raw_delta_x = target - vel.x
 *   RTZ >> 4; clamp [-256,256] → a1 = delta_x
 *
 *   lh v1,26(s2)        ; mat[1][2] = obj+0x1a (Y)
 *   multiply × 15258; RTZ >> 12 → target_vel_y
 *   lw a0,140(s2)       ; vel.y
 *   addu a0,a0,a1       ; vel.x += delta_x (SW in delay of bgez)
 *   sw a0,136(s2)
 *   subu v0,v0,a0       ; raw_delta_y
 *   RTZ >> 3 → delta_y (NOTE: /8 not /16)
 *
 *   lh v1,32(s2)        ; mat[2][2] = obj+0x20 (Z)
 *   vel.y += delta_y; sw vel.y
 *   multiply × 15258; RTZ >> 12 → target_vel_z
 *   lw v1,144(s2)       ; vel.z
 *   RTZ >> 4; clamp [-256,256] → a1 = delta_z
 *
 *   integrate pos += vel; mirror to render_pos
 *   predictive terrain check 16 ticks ahead
 *   if terrain blocking: FUN_8004779c normalise + FUN_8004352c rotate + FUN_8004ecd4 steer
 *   else: homing toward target (read from obj+0x84)
 *   RotMatrix_ApplyAngVel(matrix, ang_y, ang_x, 0)
 *   smoke every 32 / spark every 4
 *   life--; if==0: sound(55)+debris(26)+retire
 *
 * event3: check DAT_80065908&0x400
 *   bit set: FUN_8003fdcc(pos,46,40)+FUN_8004d154(self_mat,trail_mat)+sound(54)+retire
 *   bit clear: WeaponHit_Apply(obj,param3,26,55)
 *
 * event4: if obj[0]&0x10000: obj[0x84][0] &= ~0x800000
 * event9: if obj[0x84]==param3: obj[0x84]=obj[0x80]
 */

/* --- SLUS_005.10  LAB_8003277C  (large mine, manual disassembly reference) ---
 *
 * event dispatch: events 0, 3, 4 only (no event 9)
 *
 * event0:
 *   lh v0,20(s1)        ; mat[0][2] → X steering
 *   multiply × 15258; RTZ >> 12 → target_vel_x
 *   subu v0,v0,a1       ; raw_delta_x = target - vel.x
 *   if(v0<0) addiu v0,v0,15
 *   sra a0,v0,4         ; delta_x >>= 4
 *   clamp [-256,256]
 *   vel.x += delta_x; pos.x += vel.x  (integrated inline)
 *
 *   lh v1,32(s1)        ; mat[2][2] → Z steering (skips Y!)
 *   multiply × 15258; >> 12; >> 4; clamp → delta_z
 *   vel.z += delta_z; pos.z += vel.z + sw (delay slot)
 *
 *   Terrain_HeightAt(pos.x, pos.z)
 *   if terrain<pos.y: sound(55)+Debris(12)+ColEvent(1)+return -1
 *
 *   copy pos→render_pos
 *   RotMatrix_ApplyAngVel(matrix, 0, 256, 0)  ; constant X-roll
 *   every 32 ticks: MatrixNormal
 *   every 4 ticks: Debris_Spawn(&pos, 18) + flag 0x400 + init anim
 *   sync obj[0x84].pos = self.pos
 *   life--; if==0: sound(55)+Debris(26)+retire; return -1
 *
 * event3: if collider_type==7 return 0; else WeaponHit_Apply(26,55)
 * event4: obj[0x84][0] &= ~0x800000
 */
#endif /* GHIDRA REF */
