/* vehicle_impulse.c -- FUN_8002c018 + FUN_8002c6fc.
 *
 * Two tightly-coupled impulse helpers called by the vehicle damage chain:
 *
 *   FUN_8002c018  Vehicle_SoundEntitySetup (127 MIPS instructions)
 *     Creates (or removes) a positional sound entity linked to the vehicle.
 *     First call when self[0] & 0x20000 == 0:
 *       Allocates a type-0x26 object of size 0x80 from the global sound
 *       pool (DAT_800737a0[15]), fills positional / half-extent fields from
 *       the bounding box at self[0x5c], registers it via FUN_8001d544,
 *       sets the 0x20000 flag, optionally calls FUN_800207c4 and then
 *       wires the entity into the audio graph via FUN_8004483c.
 *     Subsequent calls (flag already set):
 *       Walks the linked list at self[0x38] looking for a type-0x26 node
 *       whose ref field matches the pool handle (DAT_800737a0[15]), then
 *       removes it via FUN_8001ac08.
 *
 *   FUN_8002c6fc  Vehicle_ZoneDamage (152 MIPS instructions)
 *     Applies a negative impulse (damage) to a vehicle.
 *     Determines the hit zone (0=top, 1=mid, 2=bottom) from the contact
 *     z-position vs. the bounding box half-extents.
 *     Rolling state (self[0xd0]==12): damage applies directly to vehicle
 *     health (self[0xc]).  If health would reach zero with flag set, calls
 *     FUN_8002be84 (soft-kill) or FUN_8002bd84 (full destroy, returns 1).
 *     Non-rolling: routes to the zone-specific sub-object pointer at
 *     self[0xec / 0xf0 / 0xf4], applies scaled damage, chains to the next
 *     alive zone when a sub-object is destroyed.
 *     Returns 1 if FUN_8002bd84 was called, 0 otherwise.
 *
 * Source: SLUS_005.10  0x8002c018 / 0x8002c6fc
 *
 * HIGH confidence: both structural bodies match Ghidra pseudo-C +
 * MIPS line-for-line.  MED confidence on semantic field names.
 */
#include <stdint.h>

/* ---------- callees ---------- */

/* Object pool allocator / destructor */
extern void *FUN_8001ac44(int *bank, uint16_t kind, uint32_t size, uint32_t flags);
extern void  FUN_8001ac08(void *entity);

/* World-object registration */
extern void  FUN_8001d544(uint32_t *self, void *entity);
extern void  FUN_800207c4(void *entity);

/* Audio voice management */
extern int   FUN_8004410c(void);
extern void  FUN_8004483c(int voice, uint32_t glist, int clip, uint32_t *pos);

/* Vehicle life-cycle */
extern void  FUN_8002bd84(uint32_t *self);   /* full destroy              */
extern void  FUN_8002be84(uint32_t *self);   /* soft-kill (health->0)     */

/* Sub-object damage-state notification */
extern void  FUN_8003fc50(void *sub);

/* Callback installed as the type-0x26 entity's tick function */
extern void  LAB_8002bfb8(void);
extern void  Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

/* ---------- globals ---------- */

/* DAT_800737a0[15] == DAT_800737dc: pool handle for vehicle sound entities.
 * The array is defined in platform/panic_stubs.c; slot 15 is the one
 * that FUN_8002c018 passes to FUN_8001ac44 and searches for in the node list. */
extern uintptr_t DAT_800737a0[32];
#define SOUND_POOL  ((int *)(uintptr_t)DAT_800737a0[15])

/* Kernel scratchpad globals (gp-relative in MIPS, named by PSX address). */
extern uint32_t uRam000005f8;   /* global audio list head; passed to FUN_8004483c */
extern uint32_t uRam00000604;   /* game-mode flags word; bit 3 = friendly-fire off */

/* ---------- helpers ---------- */

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_mult_lo_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)((int64_t)a * (int64_t)b));
}

/* RTZ right-shift: bgez/addiu bias/sra MIPS pattern. */
static inline int32_t rtz_sra(int32_t v, int n)
{
    if (v < 0) v = mips_addu_i32(v, (1 << n) - 1);
    return v >> n;
}

static inline int32_t mips_avg2_i32(int32_t a, int32_t b)
{
    int32_t sum = mips_addu_i32(a, b);
    return mips_addu_i32(sum, (int32_t)((uint32_t)sum >> 31)) >> 1;
}

/* ======================================================================
 * FUN_8002c018 -- Vehicle_SoundEntitySetup
 * Source: SLUS_005.10  0x8002c018
 * ====================================================================== */
void FUN_8002c018(uint32_t *self)
{
    uint8_t *s = (uint8_t *)self;

    if ((*self & 0x20000u) == 0) {
        /* ---- first call: allocate and link a positional sound entity ---- */

        /* self[0x5c] is a pointer to the bounding-box object.
         * Fields at bb+0x04..+0x18 are six int32 coordinates:
         *   +0x04  x_min   +0x08  y_min   +0x0c  z_min
         *   +0x10  x_max   +0x14  y_max   +0x18  z_max  */
        uint8_t *bb = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x5c);

        uint8_t *ent = (uint8_t *)FUN_8001ac44(SOUND_POOL, 0x26, 0x80, 8);

        /* Install the entity's tick callback. */
        Object_SetCallbackPsxSlot(ent, (uintptr_t)&LAB_8002bfb8);

        /* Center position: average of min/max per axis. */
        int32_t x0 = *(int32_t *)(bb + 0x04);
        int32_t x1 = *(int32_t *)(bb + 0x10);
        int32_t y1 = *(int32_t *)(bb + 0x14);
        int32_t z0 = *(int32_t *)(bb + 0x0c);
        int32_t z1 = *(int32_t *)(bb + 0x18);

        *(int32_t *)(ent + 0x24) = mips_avg2_i32(x0, x1);
        *(int32_t *)(ent + 0x28) = y1;                   /* y: copy x_max? */
        *(int32_t *)(ent + 0x2c) = mips_avg2_i32(z0, z1);

        /* Zero 6 int16 fields at ent+0x12..+0x1e (skipping 0x10, 0x18, 0x20). */
        *(int16_t *)(ent + 0x12) = 0;
        *(int16_t *)(ent + 0x14) = 0;
        *(int16_t *)(ent + 0x16) = 0;
        *(int16_t *)(ent + 0x1a) = 0;
        *(int16_t *)(ent + 0x1c) = 0;
        *(int16_t *)(ent + 0x1e) = 0;

        /* Half-extents: RTZ divide of range by power-of-two. */
        int32_t dx = mips_subu_i32(x1, x0);
        *(int16_t *)(ent + 0x10) = (int16_t)rtz_sra(dx, 3);   /* /8 */

        int32_t dy = mips_subu_i32(y1, *(int32_t *)(bb + 0x08));
        *(int16_t *)(ent + 0x18) = (int16_t)rtz_sra(dy, 2);   /* /4 */

        int32_t dz = mips_subu_i32(z1, z0);
        *(int16_t *)(ent + 0x20) = (int16_t)rtz_sra(dz, 3);   /* /8 */

        /* Register entity in the world and set the initialised flag. */
        FUN_8001d544(self, (void *)ent);

        uint32_t flags = *self;
        *self = flags | 0x20000u;

        if ((flags & 4u) == 0)
            FUN_800207c4((void *)ent);

        int voice = FUN_8004410c();
        /* self+9 uint32_t words = self + 0x24 bytes = pos field in vehicle */
        FUN_8004483c(voice, uRam000005f8, 0x2d, self + 9);
    } else {
        /* ---- already initialised: find and remove the sound entity ---- */

        /* self[0x38] is the head of an entity linked list. Walk until we
         * find a node with type==0x26 and whose ref matches the pool handle,
         * then call FUN_8001ac08 to unlink/free it. */
        uintptr_t node = *(uint32_t *)(s + 0x38);
        while (node != 0) {
            int16_t  type = *(int16_t *)(node + 0x0a);
            uint32_t ref  = *(uint32_t *)(node + 0x58);
            if (type == 0x26 && ref == (uint32_t)DAT_800737a0[15])
                break;
            node = *(uint32_t *)(node + 0x34);
        }
        if (node != 0)
            FUN_8001ac08((void *)(uintptr_t)node);
    }
}

/* ======================================================================
 * FUN_8002c6fc -- Vehicle_ZoneDamage
 * Source: SLUS_005.10  0x8002c6fc
 * ====================================================================== */
int FUN_8002c6fc(uint32_t *self, int impulse, const int32_t *vec, int flag)
{
    uint8_t *s = (uint8_t *)self;

    /* --- zone determination from contact z vs bounding box --- */
    uint8_t *bb   = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0x5c);
    int32_t upper = *(int32_t *)(bb + 0x18) / 2;   /* z_max / 2 */
    int32_t lower = *(int32_t *)(bb + 0x0c) / 2;   /* z_min / 2 */
    int32_t cz    = vec[2];

    int zone;
    if (upper < cz) {
        zone = 0;   /* above upper bound */
    } else {
        zone = 1;   /* middle (default)  */
        if (cz < lower)
            zone = 2;   /* below lower bound */
    }

    /* --- common guards --- */
    uint16_t health = *(uint16_t *)(s + 0x0c);

    /* No action for non-negative impulse or already-dead vehicle. */
    if (impulse >= 0 || health == 0)
        return 0;

    /* Friendly-fire check: when bit 3 of game-mode flags is set, vehicles
     * with negative self[0x6] (same team?) are immune.                   */
    if ((uRam00000604 & 8u) != 0 && *(int16_t *)(s + 0x6) < 0)
        return 0;

    int new_hp = mips_addu_i32((int)(uint32_t)health, impulse);  /* impulse is negative */

    if (*(int8_t *)(s + 0xd0) == 12) {
        /* ---- rolling state: damage the vehicle directly ---- */
        if (new_hp < 1) {
            if (flag != 0) {
                /* LAB_8002c8fc: choose between soft-kill and full destroy. */
LAB_weak_kill:
                if (new_hp > -0x15) {   /* -21 < new_hp <= 0 */
                    FUN_8002be84(self);
                    return 0;
                }
                FUN_8002bd84(self);
                return 1;
            }
            new_hp = 1;   /* clamp: flag==0, never go below 1 */
        }
        *(int16_t *)(s + 0x0c) = (int16_t)new_hp;
    } else {
        /* ---- non-rolling: damage the zone-specific sub-object ---- */
        int off = mips_mult_lo_i32(zone, 4);
        uint32_t half_hp = (uint32_t)health >> 1;

        /* Return 0 immediately if no sub-object exists for this zone. */
        if (*(uint32_t *)(s + 0xec + off) == 0)
            return 0;

        int acc = impulse;   /* accumulated (chain) damage */

        /* Loop: process sub at the current zone; if it dies, pick the next
         * alive zone and repeat until no sub-object remains to absorb. */
        do {
            uint8_t  *sub    = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0xec + off);
            uint16_t sub_hp  = *(uint16_t *)(sub + 0x0c);
            int8_t   factor  = *(int8_t  *)(sub + 0x08);
            int new_sub      = mips_addu_i32((int)(uint32_t)sub_hp, acc);

            /* Notify caller if the scaled-damage bracket changed.
             * Scaled = (sub_hp * factor + half_health) / health. */
            int32_t old_sc = mips_addu_i32(mips_mult_lo_i32((int32_t)sub_hp, factor),
                                           (int32_t)half_hp) / (int32_t)health;
            int32_t new_sc = mips_addu_i32(mips_mult_lo_i32(new_sub, factor),
                                           (int32_t)half_hp) / (int32_t)health;
            if (old_sc != new_sc)
                FUN_8003fc50(sub);

            if (new_sub >= 0) {
                /* Sub-object survives: store reduced health, return. */
                *(int16_t *)(sub + 0x0c) = (int16_t)new_sub;
                return 0;
            }

            /* Sub-object destroyed: zero its health, carry damage forward. */
            *(uint16_t *)(sub + 0x0c) = 0;
            acc = new_sub;   /* negative residual */

            /* Select which zone to hit next:
             *   if we just hit zone 1 (mid): switch based on whether zone 0 is dead.
             *   otherwise: switch to zone 1.                                          */
            if (zone == 1) {
                uint8_t *z0 = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0xec);
                zone = (*(uint16_t *)(z0 + 0x0c) == 0) ? 2 : 0;
            } else {
                zone = 1;
            }

            /* Check if ALL three sub-objects are dead. */
            {
                uint8_t *z0 = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0xec);
                uint8_t *z1 = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0xf0);
                uint8_t *z2 = (uint8_t *)(uintptr_t)*(uint32_t *)(s + 0xf4);
                if (*(uint16_t *)(z0 + 0x0c) == 0 &&
                    *(uint16_t *)(z1 + 0x0c) == 0 &&
                    *(uint16_t *)(z2 + 0x0c) == 0) {
                    if (flag == 0) return 0;
                    /* Re-use new_sub as new_hp for the weak-kill label. */
                    new_hp = acc;
                    goto LAB_weak_kill;
                }
            }

            off = mips_mult_lo_i32(zone, 4);
        } while (*(uint32_t *)(s + 0xec + off) != 0);
    }
    return 0;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth -- DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8002c018  (from analysis/SLUS_005.10/decomp/8002c018.c) --- */
// addr: 0x8002c018  name: FUN_8002c018

void FUN_8002c018(uint *param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;

  if ((*param_1 & 0x20000) == 0) {
    uVar5 = param_1[0x17];
    iVar1 = FUN_8001ac44(DAT_800737dc,0x26,0x80,8);
    *(undefined1 **)(iVar1 + 100) = &LAB_8002bfb8;
    *(int *)(iVar1 + 0x24) = (*(int *)(uVar5 + 4) + *(int *)(uVar5 + 0x10)) / 2;
    *(undefined4 *)(iVar1 + 0x28) = *(undefined4 *)(uVar5 + 0x14);
    iVar2 = *(int *)(uVar5 + 0xc);
    iVar4 = *(int *)(uVar5 + 0x18);
    *(undefined2 *)(iVar1 + 0x1e) = 0;
    *(undefined2 *)(iVar1 + 0x1c) = 0;
    *(undefined2 *)(iVar1 + 0x1a) = 0;
    *(undefined2 *)(iVar1 + 0x16) = 0;
    *(undefined2 *)(iVar1 + 0x14) = 0;
    *(undefined2 *)(iVar1 + 0x12) = 0;
    *(int *)(iVar1 + 0x2c) = (iVar2 + iVar4) / 2;
    iVar2 = *(int *)(uVar5 + 0x10) - *(int *)(uVar5 + 4);
    if (iVar2 < 0) {
      iVar2 = iVar2 + 7;
    }
    *(short *)(iVar1 + 0x10) = (short)(iVar2 >> 3);
    iVar2 = *(int *)(uVar5 + 0x14) - *(int *)(uVar5 + 8);
    if (iVar2 < 0) {
      iVar2 = iVar2 + 3;
    }
    *(short *)(iVar1 + 0x18) = (short)(iVar2 >> 2);
    iVar2 = *(int *)(uVar5 + 0x18) - *(int *)(uVar5 + 0xc);
    if (iVar2 < 0) {
      iVar2 = iVar2 + 7;
    }
    *(short *)(iVar1 + 0x20) = (short)(iVar2 >> 3);
    FUN_8001d544(param_1,iVar1);
    uVar5 = *param_1;
    *param_1 = uVar5 | 0x20000;
    if ((uVar5 & 4) == 0) {
      FUN_800207c4(iVar1);
    }
    uVar3 = FUN_8004410c();
    FUN_8004483c(uVar3,uRam000005f8,0x2d,param_1 + 9);
  }
  else {
    uVar5 = param_1[0xe];
    if (uVar5 != 0) {
      do {
        if ((*(short *)(uVar5 + 10) == 0x26) && (*(int *)(uVar5 + 0x58) == DAT_800737dc)) break;
        uVar5 = *(uint *)(uVar5 + 0x34);
      } while (uVar5 != 0);
      if (uVar5 != 0) {
        FUN_8001ac08(uVar5);
      }
    }
  }
  return;
}

/* --- SLUS_005.10 FUN_8002c6fc  (from analysis/SLUS_005.10/decomp/8002c6fc.c) --- */
// addr: 0x8002c6fc  name: FUN_8002c6fc

undefined4 FUN_8002c6fc(int param_1,int param_2,int param_3,int param_4)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;

  if (*(int *)(*(int *)(param_1 + 0x5c) + 0x18) / 2 < *(int *)(param_3 + 8)) {
    iVar5 = 0;
  }
  else {
    iVar5 = 1;
    if (*(int *)(param_3 + 8) < *(int *)(*(int *)(param_1 + 0x5c) + 0xc) / 2) {
      iVar5 = 2;
    }
  }
  uVar4 = (uint)*(ushort *)(param_1 + 0xc);
  if (((param_2 < 0) && (uVar4 != 0)) &&
     (((uRam00000604 & 8) == 0 || (-1 < *(short *)(param_1 + 6))))) {
    iVar1 = uVar4 + param_2;
    if (*(char *)(param_1 + 0xd0) == '\f') {
      if (iVar1 < 1) {
        if (param_4 != 0) {
LAB_8002c8fc:
          if (-0x15 < iVar1) {
            FUN_8002be84(param_1);
            return 0;
          }
          FUN_8002bd84(param_1);
          return 1;
        }
        iVar1 = 1;
      }
      *(short *)(param_1 + 0xc) = (short)iVar1;
    }
    else {
      iVar3 = iVar5 * 4;
      uVar6 = (uint)(*(ushort *)(param_1 + 0xc) >> 1);
      iVar2 = *(int *)(param_1 + iVar3 + 0xec);
      iVar1 = param_2;
      while (iVar2 != 0) {
        iVar3 = *(int *)(param_1 + iVar3 + 0xec);
        iVar1 = (uint)*(ushort *)(iVar3 + 0xc) + iVar1;
        if ((int)((uint)*(ushort *)(iVar3 + 0xc) * (int)*(char *)(iVar3 + 8) + uVar6) / (int)uVar4
            != (int)(iVar1 * *(char *)(iVar3 + 8) + uVar6) / (int)uVar4) {
          FUN_8003fc50(iVar3);
        }
        if (-1 < iVar1) {
          *(short *)(iVar3 + 0xc) = (short)iVar1;
          return 0;
        }
        *(undefined2 *)(iVar3 + 0xc) = 0;
        if (iVar5 == 1) {
          iVar5 = (uint)(*(short *)(*(int *)(param_1 + 0xec) + 0xc) == 0) << 1;
        }
        else {
          iVar5 = 1;
        }
        if (((*(short *)(*(int *)(param_1 + 0xec) + 0xc) == 0) &&
            (*(short *)(*(int *)(param_1 + 0xf0) + 0xc) == 0)) &&
           (*(short *)(*(int *)(param_1 + 0xf4) + 0xc) == 0)) {
          if (param_4 == 0) {
            return 0;
          }
          goto LAB_8002c8fc;
        }
        iVar3 = iVar5 * 4;
        iVar2 = *(int *)(param_1 + iVar3 + 0xec);
      }
    }
  }
  return 0;
}

#endif  /* GHIDRA REF */
