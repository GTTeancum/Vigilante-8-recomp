#ifndef V8_STRUCTS_H
#define V8_STRUCTS_H

#include <stdint.h>
#include "fixed.h"

/* Recovered struct layouts for Vigilante 8 (PSX SLUS_005.10).
 * Layouts derive from cross-function offset clustering. Fields are
 * tagged with confidence per DECOMP_RULES.md.
 *
 * STATUS: Pass 1. Most struct layouts are partial -- only offsets that
 * appear in already-decompiled functions are listed. Pass 2 will close
 * the gaps by clustering every memory access against allocation sites.
 */

/* PSX standard libgs / libgte types. Match PSY-Q layout exactly. */
typedef struct { int16_t vx, vy, vz, pad; } SVECTOR;
typedef struct { int32_t vx, vy, vz;       } VECTOR;
typedef struct {
    int16_t m[3][3];
    int16_t pad;
    int32_t t[3];
} MATRIX;
typedef struct { uint8_t r, g, b, cd; } CVECTOR;

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

/* Vehicle struct -- STRUCT_PASS recovered ~16 fields. Total size still
 * unknown (highest seen offset is +0x120). Allocation site not yet
 * located -- pass 2 will close that gap.
 *
 * Evidence: cross-function offset clustering through puRam000007d0 (P1)
 * and puRam000007d4 (P2) via call-site argument propagation (the
 * STRUCT_PASS sweep below; see notes/struct_evidence.md).
 *
 *   +0x00  u32       inputFlags    HIGH  bit 0x2 = button held; 0x1000000 = held-latch; 0x20000000 = flag for late-bind; 0x4000 = "active" flag
 *   +0x0c  i16       statusFlags   HIGH  compared against 0; LSB selects between alt-model frame
 *   +0x0e  i16       statusPad
 *   +0x54  void*     listLink      MED   appears 24-bit-masked (Ghidra OT chain artifact); reused as next-in-pool ptr
 *   +0xb3  u8        damageBits    MED   set after collision; 6 reads
 *   +0xbc  i16       healthMaybe   MED   compared against 0 in collision response
 *   +0xd0  i8        controlFlags  LOW
 *   +0xe0  void*     modelData     HIGH  used by Render_VehicleSetTrans (out-of-scope renderer reads it too)
 *   +0xe4  i32       drawCallback  MED   indexed once per draw frame
 *   +0xec  i32       posX_q1715    HIGH  17.15 packed world coord
 *   +0xf0  i32       posY_q1715    HIGH
 *   +0xf4  i32       posZ_q1715    HIGH
 *   +0xf8  void*     altModelData  HIGH  (puRam000007d0[0x3e])
 *   +0x120 i16       angleMaybe    MED   compared/incremented per tick
 *
 * Tag legend:
 *   HIGH = field role unambiguous across multiple call sites
 *   MED  = clear *type*, role inferred from a few sites
 *   LOW  = best guess
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
