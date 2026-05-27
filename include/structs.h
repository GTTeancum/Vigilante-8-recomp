#ifndef V8_STRUCTS_H
#define V8_STRUCTS_H

#include <stdint.h>
#include "fixed.h"
#include "gte.h"   /* SVECTOR, VECTOR, MATRIX, CVECTOR live here -- one canonical home */

/* Recovered struct layouts for Vigilante 8 (PSX SLUS_005.10).
 * Layouts derive from cross-function offset clustering. Fields are
 * tagged with confidence per DECOMP_RULES.md.
 *
 * STATUS: Pass 2 complete. Vehicle padded to 0x200 in Phase 1 of the
 * running-game plan; remaining UNKNOWN is the alloc site that would
 * confirm the real size.
 */

/* CD-ROM disc location (BCD-encoded MM:SS:FF). PSY-Q definition. */
typedef struct {
    uint8_t minute, second, sector;
    uint8_t track;
} CdlLOC;

typedef struct { int16_t x, y; int16_t w, h; } RECT;

/* --- Vigilante 8 game structures ------------------------------------------ */

/* Heap free-list node. Used by FUN_80044fbc (InitHeap), FUN_80045004 (malloc3),
 * FUN_80045088 (free3). PSY-Q-style 8-byte header per block:
 *   [0]: next free block pointer
 *   [1]: free-block size in 8-byte units
 * HIGH confidence: matches the standard PSY-Q K&R-style heap exactly.
 */
typedef struct V8HeapBlock {
    struct V8HeapBlock *next;   /* HIGH: linked-list next-free pointer */
    uint32_t           size;    /* HIGH: size in 8-byte units */
} V8HeapBlock;

/* Match-state enum value lives in bRam00000015. Observed values:
 *   0: regular match / shell flow
 *   3: probably mid-match end-screen
 *   5: probably credits / special seed mode
 * Values 3..5 are still LOW.
 */
typedef enum {
    MATCH_MODE_SHELL    = 0,   /* MED: shell / pre-match */
    MATCH_MODE_PLAY_A   = 1,   /* LOW */
    MATCH_MODE_PLAY_B   = 2,   /* LOW */
    MATCH_MODE_END      = 3,   /* LOW */
    MATCH_MODE_SPECIAL5 = 5    /* LOW: triggers FUN_8001714c(0xbb40e64d), looks like fixed seed */
} V8MatchMode;

/* Vehicle / movable-object struct.
 *
 * The engine reuses ONE common base layout for every "movable object"
 * that participates in physics (vehicles, AI cars, guided projectiles,
 * physics debris, destructibles).  The base layout is what's documented
 * below; some object types reinterpret bytes in private windows (e.g.
 * projectiles use +0x80/82/84 as i16 angvel rather than i32 vel; see
 * `Projectile_GravityTick` in src/gameplay/object_post_update.c).
 *
 * Field evidence below comes from a line-by-line MIPS audit of these
 * cleaned functions:
 *
 *   Object_IntegrateAndOrient        src/physics/object_integrate.c
 *   Object_GeneralTick               src/physics/object_general_tick.c
 *   Object_ApplyImpulseAndIntegrate  src/physics/object_impulse.c
 *   Object_OBBSuspension             src/physics/object_obb_suspension.c
 *   Object_FindObstacleAt            src/physics/obstacle_probe.c
 *   SAT_ProjectAxis                  src/physics/sat_projection.c
 *
 * Tag legend:
 *   HIGH = field role unambiguous across multiple call sites
 *   MED  = clear *type*, role inferred from a few sites
 *   LOW  = unresolved/inferred from partial evidence
 *
 *   +0x00  u32       inputFlags        HIGH  bit 0x2 = button held; 0x1000000 = held-latch;
 *                                            0x20000000 = late-bind; 0x4000 = active;
 *                                            0x800 = "has collidable kd-tree";
 *                                            0x800000 = "skip prev-pos trail"
 *   +0x0c  i16       health            HIGH  damaged via Damage_AccumulateOrFire
 *   +0x0e  i16       maxHealth         HIGH
 *   +0x10..+0x27     MATRIX            HIGH  3x3 i16 rotation + i16 pad + 3 i32 translation.
 *                                            t[0..2] OVERLAPS pos at +0x24/+0x28/+0x2c.
 *   +0x12  i16       reachExtent.x     MED   used by ObstacleChain_Walk visibility test
 *   +0x18  i16       reachExtent.y     MED
 *   +0x1e  i16       reachExtent.z     MED
 *   +0x14  i16       inputLat          HIGH  Object_GeneralTick reads as lateral-input scalar
 *   +0x16  i16       steerNudgeZ       HIGH  sign-flag for per-tick ang_z +-0x200 nudge
 *   +0x1a  i16       steerNudgeX       HIGH  sign-flag for per-tick ang_x +-0x200 nudge
 *   +0x20  i16       inputLong         HIGH  longitudinal-input scalar
 *   +0x24  i32       posX              HIGH  q15.16 world coord (aliases matrix.t[0])
 *   +0x28  i32       posY              HIGH  q15.16 (PSX convention: +Y is DOWN)
 *   +0x2c  i32       posZ              HIGH
 *   +0x34  void*     listNext          HIGH  next-in-chain pointer for obstacle-tree walker
 *   +0x38  void*     childListHead     HIGH  head of child-object list
 *   +0x48  i32       prevPosX          HIGH  smoothed/lerped position trail
 *   +0x4c  i32       prevPosY          HIGH
 *   +0x50  i32       prevPosZ          HIGH
 *   +0x54  void*     listLink          MED   appears 24-bit-masked (Ghidra OT artifact)
 *   +0x5c  void*     kdtreeRoot        HIGH  collision leaf-stream root; NULL = no obstacles
 *   +0x64  TickFn    tickCallback      HIGH  per-frame physics dispatch (Physics_Step calls)
 *   +0x74  void*     obstacleChainA    HIGH  collision: primary obstacle list
 *   +0x78  void*     obstacleChainB    HIGH  collision: secondary obstacle list
 *   +0x80  i32       velX              HIGH  q23.8-ish vel; integrator div /128
 *   +0x84  i32       velY              HIGH
 *   +0x88  i32       velZ              HIGH
 *   +0x8c  i32       speedCached       HIGH  Vec3_Length(vel) / 128 -- updated per tick
 *   +0x90  i32       angVelX           HIGH  q11.20-ish angvel; integrator div /128
 *   +0x94  i32       angVelY           HIGH
 *   +0x98  i32       angVelZ           HIGH
 *   +0x9c  i16       invInertiaX       HIGH  per-axis inverse-inertia for angular impulses
 *   +0x9e  i16       invInertiaY       HIGH
 *   +0xa0  i16       invInertiaZ       HIGH
 *   +0xa4  i16       angYPreBake       HIGH  Object_GeneralTick writes (this << 6) into angVelY
 *   +0xa6  i16       inputMul          HIGH  multiplier for lat/long input products
 *   +0xb3  u8        damageBits        MED   set after collision; 6 reads
 *   +0xba  u8        weight            MED   MatchScore_AppendLine evidence
 *   +0xbb  u8        skill             MED   same evidence
 *   +0xbc  i16       healthSecondary   MED   secondary damage check
 *   +0xd0  i8        controlFlags      LOW
 *   +0xd8  i32       dragMass          HIGH  drag-Y coefficient (Object_GeneralTick mass-curve)
 *   +0xe0  void*     modelData         HIGH  used by Render_VehicleSetTrans (renderer reads)
 *   +0xe4  i32       currentTarget     HIGH  AI's current target (Vehicle_TryAcquireTarget)
 *   +0xec  i32       posX_alt          HIGH  ALTERNATE posX field (renderer mirror)
 *   +0xf0  i32       posY_alt          HIGH
 *   +0xf4  i32       posZ_alt          HIGH
 *   +0xf8  void*     altModelData      HIGH
 *   +0x110..+0x118   void* node[3]     HIGH  3 ptr slots for timer/list nodes
 *   +0x11c i16       timer0            HIGH  Object_GeneralTick decrements while > 0
 *   +0x11e i16       timer1            HIGH
 *   +0x120 i16       timer2_snapFlag   HIGH  doubles as snap-vs-lerp flag for prev-pos trail
 *
 * Size: 0x124 bytes.  Confirmed by Vehicle_Construct (SLUS 0x8002e630),
 * which allocates exactly 0x124 bytes through FUN_8001ac44.
 */
typedef struct Vehicle {
    uint32_t inputFlags;            /* HIGH @0x00 */
    uint32_t reserved_0x04[2];      /* +0x04, +0x08 -- unused / private */
    uint16_t health;                /* HIGH @0x0c -- pass 2 promoted from statusFlags after Damage_AccumulateOrFire decomp; arbiter at src/gameplay/collision_apply.c */
    uint16_t maxHealth;             /* HIGH @0x0e -- reset target when destroyed */
    uint8_t  reserved_0x10[0x54 - 0x10];
    void    *listLink;              /* MED  @0x54 */
    uint8_t  reserved_0x58[0xb3 - 0x58];
    uint8_t  damageBits;            /* MED  @0xb3 */
    uint8_t  reserved_0xb4[0xbc - 0xb4];
    int16_t  healthMaybe;           /* MED  @0xbc */
    uint8_t  reserved_0xbe[0xd0 - 0xbe];
    int8_t   controlFlags;          /* LOW  @0xd0 */
    uint8_t  reserved_0xd1[0xe0 - 0xd1];
    void    *modelData;             /* HIGH @0xe0 */
    int32_t  currentTarget;         /* HIGH @0xe4 -- ptr to AI's current target (Vehicle_TryAcquireTarget); previously 'drawCallback' */
    uint8_t  reserved_0xe8[0xec - 0xe8];
    int32_t  posX_q1715;            /* HIGH @0xec  17.15 packed */
    int32_t  posY_q1715;            /* HIGH @0xf0 */
    int32_t  posZ_q1715;            /* HIGH @0xf4 */
    void    *altModelData;          /* HIGH @0xf8 */
    uint8_t  reserved_0xfc[0x120 - 0xfc];
    int16_t  angleMaybe;            /* MED  @0x120 */
    /* size unknown -- callers reach at least +0x120; allocation site TBD */
} Vehicle;

#endif /* V8_STRUCTS_H */
