/* vehicle_construct.c -- Vehicle struct constructor + spawn dispatcher.
 *
 * Source: SLUS_005.10
 *   FUN_8002e630  Vehicle_Construct   (1124 B)
 *   FUN_8002a350  Vehicle_Dispatch    (mode 7 = spawn -> Construct)
 *
 * This is the ENGINE'S OWN player/AI vehicle initialiser.  The host
 * vehicle shim in platform/host_vehicle.c bypasses this and allocates
 * a 0x200-byte zero-filled buffer; using the constructor instead
 * gives bit-exact Vehicle state at spawn.
 *
 * The constructor:
 *   1. Allocates 0x124 bytes via the object pool (FUN_8001ac44 =
 *      Object_Pool_AllocFromBank).  Vehicle struct size is fixed at
 *      0x124 = 292 bytes.
 *   2. Initialises basic flags and template-derived fields.
 *   3. Sets the per-tick callback to LAB_8002e2bc (the 0x374-byte
 *      player physics tick that lives in a Ghidra gap region).
 *   4. Computes `dragMass = -*(int*)(obj+0x4c)` -- the +0x4c slot is
 *      template-copied by the pool allocator.
 *   5. Walks the obj's child list (+0x38) and registers each child
 *      with bonus 'kind < 4' into puVar6[index + 0x3b] slots
 *      (+0xec..+0xf8 = 4 generic "subobject" pointers).
 *   6. Allocates and initialises 4 WHEEL sub-objects (one per
 *      uVar10 in [0, 4)).  Each wheel is a separately-pool-allocated
 *      0x9c-byte object whose pointer is stored at
 *      puVar6[uVar10 + 0x3f] = obj + (0xfc + uVar10*4).
 *      Each wheel gets:
 *         +0x80 i32 chassisLink       (from main bank entry +0x24)
 *         +0x84 i32 inertia_q12       (= -bank_inertia)
 *         +0x88 u32 invInertiaScale   (= 0x1000000 / (-inertia / 0x1000))
 *         +0x8c u32 randSeed          (from FUN_80017160 -- RNG)
 *         +0x44/0x48 unaligned packed (matches MIPS bit-twiddle pattern)
 *      and is linked to the chassis via FUN_8001b2fc.
 *   7. Allocates a 0x80-byte "extra" object via FUN_8001d470,
 *      stored at obj+0xf8 (puVar6[0x3e]).
 *   8. Memsets vel/angvel (+0x90..+0x9b, 12 bytes) and a 28-byte
 *      block at obj+0xa4..+0xbf to zero.
 *   9. Writes per-character stats from the template:
 *         +0x16  health-low (template+0x1c)
 *         +0xa2  stat[+0x1a]
 *         +0xa8  stat[+0x18]
 *         +0xaa  signed stat[+0x1e]
 *         +0xb4  signed stat[+0x1f]
 *         +0xb2  always 1 (a "valid vehicle" sentinel)
 *
 * Field offsets and values below are byte-for-byte against the MIPS
 * disassembly at analysis/SLUS_005.10/mips/8002e630.s -- no empirical
 * fitting.
 *
 * HIGH confidence on structure + offsets; the bit-packing in steps 6
 * and 9 uses MIPS-style unaligned stores (`-1 << shift | value >> ...`)
 * which we preserve literally below.
 *
 * NOT WIRED to the host runtime yet: this function expects (1) a
 * fully-initialised object pool at `param_1` (the bank, normally
 * DAT_800737d4/player Arms.exp bank), and (2) one of the 0x24-byte
 * vehicle stat templates at 0x8005ea60..0x8005ec10.  The tiny wrappers
 * at 0x8002a3e8..0x8002a58c pass those template addresses to
 * Vehicle_Dispatch; they are now materialised in vehicle_stat_templates.c.
 */
#include <stdint.h>

extern void *FUN_8001ac44(int *bank, uint16_t kind, uint32_t size, uint32_t flags);
extern intptr_t FUN_8001affc(int *bank, uint16_t kind, uint16_t key);
extern void  FUN_8001b2fc(uint32_t *chassis, const void *boneTemplate, uint32_t *wheel);
extern void  FUN_8001d4f0(uint32_t *chassis, uint32_t extra);
extern void *FUN_8001d470(uint32_t size);
extern void  FUN_8001d708(uint32_t *obj);
extern int   FUN_8001b270(uint32_t *obj);
extern char  FUN_8003fc94(uint32_t childObj);
extern uint32_t FUN_80017160(void);          /* RNG */
extern void *FUN_80044efc(void *dst, int val, uint32_t n);   /* memset */

/* The tick callback installed into every constructed Vehicle.  Body
 * lives at SLUS:0x8002e2bc in a Ghidra gap region (analysis/.../mips/
 * 8002e2bc.s).  Cleaned C decomp pending. */
extern void LAB_8002e2bc(void);

extern uint32_t uRam00000604;
extern int *    DAT_800737d4;       /* global object pool/bank */
extern void     Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

/* HIGH: construct a Vehicle (player or AI car) from a template.
 *
 *   bank      -- normally DAT_800737d4 (the global pool)
 *   kind      -- u16 kind id (selects template within bank)
 *   template  -- vehicle stats data (one of DAT_8005ea60..DAT_8005ec10).
 *                  +0x0c (u8)  : per-wheel-presence bitmask (4 bits used)
 *                  +0x0d (u8)  : template stat (-> obj +0xd0 controlFlags)
 *                  +0x0e (u8)  : team / variant flags (-> obj +0xac)
 *                  +0x0f (u8)  : another stat (-> obj +0xb4)
 *                  +0x10 (u32) : template-data ptr (-> obj +0xdc)
 *                  +0x14 (u32) : unaligned block for obj+0x9f..+0x9c
 *                  +0x18 (u16) : -> obj +0xa0
 *                  +0x1a (u16) : stat (-> obj +0xa2)
 *                  +0x1c (u16) : health (-> obj +0x0c)
 *                  +0x1e (i8)  : signed stat (-> obj +0xaa)
 *                  +0x1f (i8)  : signed stat (-> obj +0xb4)
 *                  +0x00/02/04/06 (per-wheel kinds, 4 x u16)
 */
uint32_t *Vehicle_Construct(int *bank, uint16_t kind, intptr_t template_p)
{
    uint8_t *tpl = (uint8_t *)(uintptr_t)template_p;
    uint32_t *obj = (uint32_t *)FUN_8001ac44(
        bank, kind, 0x124,
        (uint32_t)(bank[1] != 0) << 3);

    /* Basic flags + template-copied scalars. */
    *(uint16_t *)((uint8_t *)obj + 6) = 0;
    *(uint8_t  *)(obj + 1) = 2;                  /* obj+4 = obj kind tag */
    obj[0]  |= 0x6000u;                          /* flags */
    *(uint16_t *)(obj + 3) = *(uint16_t *)(tpl + 0x1c);          /* obj+0xc = health */
    *(uint16_t *)(obj + 0x35) = 0x400;           /* obj+0xd4 = 0x400 */
    *(uint8_t  *)(obj + 0x34) = *(uint8_t  *)(tpl + 0xd);        /* obj+0xd0 controlFlags */
    obj[0x37] = *(uint32_t *)(tpl + 0x10);                       /* obj+0xdc = template ptr */

    if (bank[1] != 0) obj[0] |= 4u;

    /* Install the tick callback in the PSX-width callback slot. */
    Object_SetCallbackPsxSlot(obj, (uintptr_t)&LAB_8002e2bc);

    /* dragMass (+0xd8) = -*(int*)(obj + 0x4c).  +0x4c is template-
     * copied by the pool allocator above (one of the first words
     * the bank entry copies into the new object). */
    obj[0x36] = (uint32_t)mips_subu_i32(0, (int32_t)obj[0x13]);

    /* Zero 12 i32 slots from +0x118 down to +0xec.
     * MIPS:
     *   a0 = s3 + 0x2c; s2 = 0xb;
     *   loop: sw zero,0xec(a0); s2--; bgez s2,loop; a0 -= 4
     */
    for (int i = 0; i <= 0xb; i++) {
        obj[0x46 - i] = 0;
    }

    /* Walk child list at obj+0x38 (puVar6[0xe]).  Each child whose
     * +6 kind-tag is < 4 gets registered into the per-kind slot at
     * obj[+0xec + kind*4]. */
    for (uintptr_t child = obj[0xe]; child != 0;
         child = *(uint32_t *)(child + 0x34)) {
        if (*(uint16_t *)(child + 6) < 4) {
            char inc = FUN_8003fc94((uint32_t)child);
            *(int8_t  *)(child + 8) = (int8_t)mips_addu_i32((int32_t)inc, 1);
            *(uint16_t *)(child + 0xc) = *(uint16_t *)(tpl + 0x1c);
            obj[*(int16_t *)(child + 6) + 0x3b] = (uint32_t)child;
        }
    }

    /* Per-wheel sub-allocation (4 wheels). */
    uint32_t rng_last = 0;
    for (uint32_t w = 0; w < 4; w++) {
        uint32_t wheel_kind = 9;
        if ((uRam00000604 & 1) == 0) {
            wheel_kind = (uint32_t)*(uint16_t *)(tpl + ((int)w >> 1) * 2);
        }
        uint32_t *wheel = (uint32_t *)FUN_8001ac44(DAT_800737d4, (uint16_t)wheel_kind, 0x9c, 0);
        *(uint8_t  *)(wheel + 1) = 8;

        int *gbank = DAT_800737d4;
        intptr_t joint = FUN_8001affc(bank, kind, (uint16_t)(w - 0x8000));
        FUN_8001b2fc(obj, (const void *)(uintptr_t)joint, wheel);
        obj[w + 0x3f] = (uint32_t)(uintptr_t)wheel;

        uint32_t chassisLink;
        if (*(uint16_t *)(joint + 0x1a) == 0xffff) {
            chassisLink = 0;
        } else {
            chassisLink = *(uint32_t *)((uintptr_t)*bank + (uint32_t)*(uint16_t *)(joint + 0x1a) * 0x1c + 0x24);
        }
        wheel[0x20] = chassisLink;                       /* wheel +0x80 */
        wheel[0x22] = wheel[0x13];                       /* wheel +0x88 */
        wheel[0x21] = wheel[0x13];                       /* wheel +0x84 */

        /* Per-wheel kind id at template+(w/2)*2 +4/+8 */
        uint8_t *tp = tpl + ((int)w >> 1) * 2;
        *(uint16_t *)(wheel + 0x23) = *(uint16_t *)(tp + 4);
        *(uint16_t *)((uint8_t *)wheel + 0x8e) = *(uint16_t *)(tp + 8);

        int iVar8  = *(int32_t *)((uintptr_t)*gbank + wheel_kind * 0x1c + 0x24);
        int iVar12 = mips_mult_lo_i32(iVar8, -0x6486);
        wheel[0x24] = (uint32_t)mips_subu_i32(0, iVar8);
        if (iVar12 < 0) iVar12 = mips_addu_i32(iVar12, 0xfff);
        wheel[0x25] = 0x1000000 / (iVar12 >> 12);

        uint16_t rng16 = (uint16_t)FUN_80017160();
        rng_last = (uint32_t)rng16;

        /* Unaligned stores at wheel+0x43 and wheel+0x40 -- the MIPS
         * uses bit-shifts to splice across word boundaries. */
        {
            uint32_t  off = ((uintptr_t)wheel + 0x43) & 3;
            uint32_t *p   = (uint32_t *)((uintptr_t)wheel + 0x43 - off);
            uint32_t  mask = (uint32_t)0xffffffffu << (off + 1) * 8;
            *p = (*p & mask) | (rng_last >> (3 - off) * 8);
        }
        {
            uint32_t  off = ((uintptr_t)(wheel + 0x10)) & 3;
            uint32_t *p   = (uint32_t *)((uintptr_t)(wheel + 0x10) - off);
            uint32_t  mask = (uint32_t)0xffffffffu >> (4 - off) * 8;
            *p = (*p & mask) | (rng_last << off * 8);
        }
        *(int16_t *)(wheel + 0x11) = (int16_t)((w & 1) << 0xb);

        uint32_t flags = w << 0x13;
        if ((int)w < 2) flags |= 0x20000;
        wheel[0] |= ((uint32_t)*(uint8_t *)(tpl + 0xc) >> (w & 0x1f) & 1u) << 0x10
                  | flags;
        FUN_8001d708(wheel);
    }

    /* The "extra" 0x80-byte sub-object stored at obj+0xf8 (puVar6[0x3e]). */
    uintptr_t extra = (uintptr_t)FUN_8001d470(0x80);
    obj[0x3e] = (uint32_t)extra;
    intptr_t extraJoint = FUN_8001affc(bank, kind, 0x8100);
    if (extraJoint == 0) {
        *(uint32_t *)(extra + 0x4c) = 0xffffaaabu;
        FUN_8001d708((uint32_t *)(uintptr_t)extra);
        FUN_8001d4f0(obj, (uint32_t)extra);
    } else {
        FUN_8001b2fc(obj, (const void *)(uintptr_t)extraJoint, (uint32_t *)(uintptr_t)extra);
    }

    /* Template-block copy at template+0x14..0x17 into obj+0x9c..+0x9f.
     * SLUS uses lwl/lwr on tpl+0x17/+0x14 followed by swl/swr on
     * obj+0x9f/+0x9c, which is an exact unaligned 4-byte copy on PSX.
     * The following sh copies tpl+0x18..+0x19 into obj+0xa0. */
    {
        uint8_t *dst = (uint8_t *)obj;
        dst[0x9c] = tpl[0x14];
        dst[0x9d] = tpl[0x15];
        dst[0x9e] = tpl[0x16];
        dst[0x9f] = tpl[0x17];
        *(uint16_t *)(obj + 0x28) = *(uint16_t *)(tpl + 0x18);
        *(uint16_t *)((uint8_t *)obj + 0xa2) = *(uint16_t *)(tpl + 0x1a);
    }

    /* Zero angular velocity + linear velocity (12 bytes at obj+0x90),
     * then mirror into obj+0x80 (vel).  MIPS does `memset(obj+0x90, 0, 12)`
     * then copies obj+0x90..+0x98 to obj+0x80..+0x88. */
    FUN_80044efc(obj + 0x24, 0, 0xc);
    obj[0x20] = obj[0x24];
    obj[0x21] = obj[0x25];
    obj[0x22] = obj[0x26];
    obj[0x1e] = 0;       /* obj +0x78 */
    obj[0x1d] = 0;       /* obj +0x74 */

    /* 28-byte zero block at obj+0xa4. */
    FUN_80044efc(obj + 0x29, 0, 0x1c);

    /* Final per-character stat copies. */
    *(uint8_t  *)((uint8_t *)obj + 0xb2) = 1;
    *(uint16_t *)(obj + 0x2b) = (uint16_t)*(uint8_t *)(tpl + 0xe);
    *(int16_t  *)(obj + 0x2a) = (int16_t)*(int8_t  *)(tpl + 0x1e);
    *(int16_t  *)((uint8_t *)obj + 0xaa) = (int16_t)*(int8_t *)(tpl + 0x1f);
    *(uint8_t  *)(obj + 0x2d) = *(uint8_t *)(tpl + 0xf);

    /* Finalise: register the object in some global table.  The return
     * goes into obj+0x7c (puVar6[0x1f]). */
    obj[0x1f] = (uint32_t)FUN_8001b270(obj);

    return obj;
}

/* HIGH: vehicle lifecycle dispatcher.  Modes:
 *    1 = init  (set up obj flags + read player vehicle global)
 *    7 = spawn (call Vehicle_Construct)
 * Source: FUN_8002a350 in analysis/. */
extern void *   puRam000007d0;
extern void     FUN_8002cce8(uint32_t *obj, uint8_t arg);

uintptr_t Vehicle_Dispatch(uint32_t *obj, int mode, uint16_t kind, intptr_t template_p)
{
    if (mode == 1) {
        uint32_t saved13 = obj[0x13];
        obj[0]    |= 0x88u;
        Object_SetCallbackPsxSlot(obj, (uintptr_t)&LAB_8002e2bc);
        obj[0x13]  = (uint32_t)mips_subu_i32((int32_t)saved13, 0x8000);
        obj[10]    = (uint32_t)mips_subu_i32((int32_t)saved13, 0x8000);
        FUN_8002cce8(obj, (uint8_t)(*(uint8_t *)((uint8_t *)obj + 3) | 1u));
        uint32_t pv = (uint32_t)(uintptr_t)puRam000007d0;
        obj[0] = (uint32_t)(uint16_t)obj[0];
        obj[0x39] = pv;     /* obj +0xe4 stores the player Vehicle ptr */
        return 0;
    }
    if (mode == 7) {
        return (uintptr_t)Vehicle_Construct((int *)(intptr_t)obj, kind, template_p);
    }
    return 0;
}

/* Legacy FUN_ aliases for direct callers. */
uint32_t *FUN_8002e630(int *bank, uint16_t kind, intptr_t template_p)
{
    return Vehicle_Construct(bank, kind, template_p);
}
uintptr_t FUN_8002a350(uint32_t *obj, int mode, uint16_t kind, intptr_t template_p)
{
    return Vehicle_Dispatch(obj, mode, kind, template_p);
}

/* HIGH: tiny vehicle-character dispatch wrappers from the previously
 * unlabelled code range 0x8002a3e8..0x8002a594.  Each wrapper only loads
 * its matching 0x24-byte stat record into a3 and jumps to FUN_8002a350.
 * DAT_8005ec34 is the runtime callback table used by FUN_80021c20 and
 * FUN_80021e5c when spawning the selected vehicle.
 */
extern const uint8_t DAT_8005ea60[0x24];
extern const uint8_t DAT_8005ea84[0x24];
extern const uint8_t DAT_8005eaa8[0x24];
extern const uint8_t DAT_8005eacc[0x24];
extern const uint8_t DAT_8005eaf0[0x24];
extern const uint8_t DAT_8005eb14[0x24];
extern const uint8_t DAT_8005eb38[0x24];
extern const uint8_t DAT_8005eb5c[0x24];
extern const uint8_t DAT_8005eb80[0x24];
extern const uint8_t DAT_8005eba4[0x24];
extern const uint8_t DAT_8005ebc8[0x24];
extern const uint8_t DAT_8005ebec[0x24];

#define VEH_WRAPPER(name, tpl) \
    uintptr_t name(uint32_t *obj, int mode, uint16_t kind) \
    { return Vehicle_Dispatch(obj, mode, kind, (intptr_t)(tpl)); }

VEH_WRAPPER(FUN_8002a3e8, DAT_8005ea60)
VEH_WRAPPER(FUN_8002a40c, DAT_8005ea84)
VEH_WRAPPER(FUN_8002a430, DAT_8005eaa8)
VEH_WRAPPER(FUN_8002a454, DAT_8005eacc)
VEH_WRAPPER(FUN_8002a478, DAT_8005eaf0)
VEH_WRAPPER(FUN_8002a49c, DAT_8005eb14)
VEH_WRAPPER(FUN_8002a4c0, DAT_8005eb38)
VEH_WRAPPER(FUN_8002a4e4, DAT_8005eb5c)
VEH_WRAPPER(FUN_8002a508, DAT_8005eb80)
VEH_WRAPPER(FUN_8002a52c, DAT_8005eba4)
VEH_WRAPPER(FUN_8002a574, DAT_8005ebc8)
VEH_WRAPPER(FUN_8002a550, DAT_8005ebec)

#undef VEH_WRAPPER

uintptr_t (*DAT_8005ec34[12])(uint32_t *, int, uint16_t) = {
    FUN_8002a3e8, FUN_8002a40c, FUN_8002a430, FUN_8002a454,
    FUN_8002a478, FUN_8002a49c, FUN_8002a4c0, FUN_8002a4e4,
    FUN_8002a508, FUN_8002a52c, FUN_8002a574, FUN_8002a550,
};

uintptr_t (*Vehicle_GetDispatchForSlot(uint32_t slot))(uint32_t *, int, uint16_t)
{
    if (slot >= 12) slot = 0;
    return DAT_8005ec34[slot];
}
