/* weapon_slot.c -- Weapon slot management and sub-object lifecycle.
 *
 * Source: SLUS_005.10
 *   FUN_8001d5a0  -- Object_FindFirstChild (13 instr)
 *   FUN_8002c99c  -- WeaponSlot_Spawn  (spawn a weapon sub-object from slot, ~35 instr)
 *   FUN_8002ca94  -- WeaponSlot_Drop   (remove slot, shift table, update damageBits, ~40 instr)
 *   FUN_8002cb7c  -- WeaponSlot_OnChildDestroy (called when weapon sub-object dies, ~25 instr)
 *   FUN_8002cbe8  -- WeaponSlot_Attach (attach new weapon to vehicle, eject oldest if full, ~50 instr)
 *
 * Struct layout constants (byte offsets into vehicle object):
 *   +6          uint16   spawnId
 *   +8          int8     kind byte (weapon type)
 *   +0xb3       uint8    damageBits (current active weapon slot index)
 *   +0x38       int32*   parent back-pointer
 *   +0x3c       int32*   first child / next sibling
 *   +0x64       void(*fn)(...)  tick callback function pointer
 *   +0x80..0x88 int32[3] parent velocity (X, Y, Z) in Q7 fixed-point
 *   +0x88..0x90 int32[3] weapon sub-object velocity
 *   +0x110..0x118 int32[3] weapon status object pointers (3 active slots)
 *   +0xec+36..44 uint32[3] weapon descriptor table (slots 9,10,11 × 4 bytes)
 *
 * Dropped-arg corrections vs Ghidra pseudoC (confirmed from MIPS):
 *   FUN_8001d5a0(param_1)          -- was FUN_8001d5a0()
 *   FUN_8002c99c(param_1,param_2)  -- was FUN_8002c99c() in ca94/cb7c
 *   FUN_8002ca94(iVar1,iVar3)      -- was FUN_8002ca94() in cb7c
 *   FUN_8001d4f0(param_1,param_2)  -- was FUN_8001d4f0() in cbe8
 *
 * HIGH confidence: all 5 functions verified instruction-for-instruction against MIPS.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Implemented elsewhere */
extern void  FUN_8001d4f0(uint32_t *parent, uint32_t *child);  /* Object_AppendChild */
extern void *FUN_8003ff28(uint32_t *weaponDesc);               /* Weapon_SpawnFromDesc */
extern void  FUN_80044c44(void *dst, void *src, int bytes);    /* PSX_Memmove */
extern int   FUN_8004410c(void);                               /* Audio_AllocVoice */
extern int   FUN_8004483c(int voice, uint32_t bank, int sfxId, void *pos3d); /* Audio_PlaySfx3DPos */
extern void  LAB_8003cb64(void);                               /* weapon projectile tick callback */
extern uint32_t uRam000005f8;                                  /* audio bank / SFX config ptr */
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
extern uintptr_t Object_CallbackFromPsxSlot(const void *obj);

/* FUN_8001d5a0 -- Object_FindFirstChild.
 * Canonical definition is in object_hierarchy.c. */
extern intptr_t FUN_8001d5a0(intptr_t param_1);

static int weapon_slot_trace_enabled(void)
{
    static int checked = 0;
    static int enabled = 0;
    if (!checked) {
        enabled = getenv("V8_TRACE_WEAPON_SLOTS") != NULL ||
                  getenv("V8_TRACE_WEAPONS") != NULL;
        checked = 1;
    }
    return enabled;
}

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

/* ================================================================
 * FUN_8002c99c  -- WeaponSlot_Spawn
 *
 * Spawn a new weapon sub-object for slot param_2 of vehicle param_1:
 *   1. Look up the weapon descriptor at param_1[0xec + (param_2+9)*4].
 *   2. Spawn the sub-object via FUN_8003ff28(descriptor).
 *   3. Copy the vehicle's spawnId to the sub-object (s0+6 = spawnId).
 *   4. Add parent velocity / 128 (arithmetic right-shift 7, with PSX
 *      rounding for negative values) to the sub-object's velocity.
 *   5. Acquire a voice channel and play sound 0x29 at the sub-object's
 *      world position (+0x24).
 *   6. Install the projectile tick callback at +0x64 if the sub-object
 *      has a non-zero spawnId AND its kind byte is not 6.
 *
 * MIPS confirmed args: a0=param_1, a1=param_2 (saved as s1 in delay slot).
 * HIGH confidence.
 * ================================================================ */
void FUN_8002c99c(intptr_t param_1, int param_2)
{
    intptr_t iVar1;   /* spawned weapon sub-object ptr */
    int      iVar3;   /* velocity component (sign-extended) */
    int      uVar2;   /* voice channel */

    /* Look up descriptor (32-bit PSX ptr stored at slot table) and spawn.
     * PSX: a1 = param_1 + (param_2+9)*4; a0 = lw [a1 + 0xec]; jal FUN_8003ff28 */
    iVar1 = (intptr_t)FUN_8003ff28((uint32_t *)(uintptr_t)
                *(int32_t *)(uintptr_t)(param_1 + mips_mult_lo_i32(mips_addu_i32(param_2, 9), 4) + 0xec));

    /* Copy parent's spawnId → sub-object. */
    *(uint16_t *)(uintptr_t)(iVar1 + 6) = *(uint16_t *)(uintptr_t)(param_1 + 6);

    /* Vel X (param_1+0x80) → sub-object +0x88, added with PSX rounding >>7. */
    iVar3 = *(int *)(uintptr_t)(param_1 + 0x80);
    if (iVar3 < 0) iVar3 = mips_addu_i32(iVar3, 0x7f);
    *(int *)(uintptr_t)(iVar1 + 0x88) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(iVar1 + 0x88), iVar3 >> 7);

    /* Vel Y (param_1+0x84) → sub-object +0x8c. */
    iVar3 = *(int *)(uintptr_t)(param_1 + 0x84);
    if (iVar3 < 0) iVar3 = mips_addu_i32(iVar3, 0x7f);
    *(int *)(uintptr_t)(iVar1 + 0x8c) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(iVar1 + 0x8c), iVar3 >> 7);

    /* Vel Z (param_1+0x88) → sub-object +0x90. */
    iVar3 = *(int *)(uintptr_t)(param_1 + 0x88);
    if (iVar3 < 0) iVar3 = mips_addu_i32(iVar3, 0x7f);
    *(int *)(uintptr_t)(iVar1 + 0x90) =
        mips_addu_i32(*(int32_t *)(uintptr_t)(iVar1 + 0x90), iVar3 >> 7);

    /* Acquire voice, play SFX at sub-object world position (+0x24). */
    uVar2 = FUN_8004410c();
    FUN_8004483c(uVar2, uRam000005f8, 0x29, (void *)(uintptr_t)(iVar1 + 0x24));

    /* Install tick callback unless spawnId==0 or kind==6. */
    if ((*(uint16_t *)(uintptr_t)(iVar1 + 0xc) != 0) &&
        (*(int8_t  *)(uintptr_t)(iVar1 + 8) != 6))
    {
        Object_SetCallbackPsxSlot((void *)(uintptr_t)iVar1, (uintptr_t)LAB_8003cb64);
    }
}

/* ================================================================
 * FUN_8002ca94  -- WeaponSlot_Drop
 *
 * Fire the spawn animation for slot param_2, then slide the weapon
 * status table down to close the gap, and update the damageBits
 * (active-weapon index) to stay valid.
 *
 * Layout of vehicle weapon status table:
 *   param_1 + 0x110 + slot*4  →  ptr to status object (or 0 if empty)
 *
 * MIPS confirmed args: a0=s4=param_1, a1→s1=param_2 (in delay slot).
 * HIGH confidence.
 * ================================================================ */
void FUN_8002ca94(intptr_t param_1, uint32_t param_2)
{
    uint8_t  bVar1;   /* original damageBits */
    int16_t  sVar2;   /* clamp temp */
    intptr_t iVar3;   /* param_1 + param_2*4 */
    uint32_t uVar4;   /* damageBits as uint */

    bVar1 = *(uint8_t *)(uintptr_t)(param_1 + 0xb3);
    uVar4 = (uint32_t)bVar1;

    /* Spawn the sub-object from this slot. */
    FUN_8002c99c(param_1, (int)param_2);

    /* Shift status table: memmove(dst, src, bytes) to close removed slot. */
    iVar3 = mips_addu_i32(mips_mult_lo_i32((int32_t)param_2, 4), param_1);
    FUN_80044c44((void *)(uintptr_t)(iVar3 + 0x110),
                 (void *)(uintptr_t)(iVar3 + 0x114),
                 (int)((2 - param_2) * 4));

    /* Zero the now-vacated last slot. */
    *(uint32_t *)(uintptr_t)(param_1 + 0x118) = 0;

    /* If damageBits <= slot, may need to decrement. */
    if ((int)uVar4 <= (int)param_2) {
        if (param_2 == 0) goto LAB_8002cb18;
        if (uVar4 != param_2) return;
        if (*(int *)(uintptr_t)(iVar3 + 0x110) != 0) goto LAB_8002cb18;
    }
    /* Decrement damageBits. */
    *(uint8_t *)(uintptr_t)(param_1 + 0xb3) =
        (uint8_t)mips_addu_i32((int32_t)(uint32_t)bVar1, -1);

LAB_8002cb18:
    if (uVar4 == param_2) {
        /* Clamp the new active slot's status spawnId to at least 0x1e. */
        iVar3 = *(int *)(uintptr_t)(param_1 + (uint32_t)*(uint8_t *)(uintptr_t)(param_1 + 0xb3) * 4 + 0x110);
        if (iVar3 != 0) {
            sVar2 = 0x1e;
            if (0x1e < *(int16_t *)(uintptr_t)(iVar3 + 6))
                sVar2 = *(int16_t *)(uintptr_t)(iVar3 + 6);
            *(int16_t *)(uintptr_t)(iVar3 + 6) = sVar2;
        }
    }
}

/* ================================================================
 * FUN_8002cb7c  -- WeaponSlot_OnChildDestroy
 *
 * Called when a weapon sub-object (param_1) is destroyed.
 * Walks up to the owning vehicle via Object_FindFirstChild on
 * param_1, then iterates the 3 weapon slots looking for the
 * matching sub-object pointer.  When found, calls WeaponSlot_Drop.
 *
 * MIPS: a0=param_1 (s0), FUN_8001d5a0(a0=param_1) → iVar1,
 *       FUN_8002ca94(a0=iVar1, a1=iVar3) (slot index).
 * HIGH confidence.
 * ================================================================ */
void FUN_8002cb7c(intptr_t param_1)
{
    intptr_t iVar1;   /* owning vehicle ptr */
    int iVar2;   /* offset within slot table */
    int iVar3;   /* slot index */

    iVar1 = FUN_8001d5a0(param_1);
    if (iVar1 != 0) {
        iVar3 = 0;
        iVar2 = 0x24;
        do {
            if (*(int *)(uintptr_t)(iVar1 + iVar2 + 0xec) == param_1) {
                FUN_8002ca94(iVar1, (uint32_t)iVar3);
                return;
            }
            iVar3 = mips_addu_i32(iVar3, 1);
            iVar2 = mips_addu_i32(iVar2, 4);
        } while (iVar3 < 3);
    }
}

/* ================================================================
 * FUN_8002cbe8  -- WeaponSlot_Attach
 *
 * Attach weapon sub-object param_2 to vehicle param_1.
 *
 *   1. Append param_2 as a child of param_1 (FUN_8001d4f0) and
 *      zero its spawnId (done in the delay slot in MIPS).
 *   2. Walk the 3 weapon slots.  If an existing slot has the same
 *      kind byte as param_2, negate param_2's kind byte and return
 *      (toggle: same weapon type already held → pick up cancels it).
 *   3. If all 3 slots are occupied (uVar3==3), determine which slot
 *      to eject:
 *        - default: eject current damageBits slot
 *        - if hp > 0 AND current slot's kind == 6 (homing): eject
 *          damageBits-1 (prefer to keep the smart weapon), or slot 2
 *          if damageBits was 0.
 *      Then call WeaponSlot_Spawn to fire the ejected slot's ammo.
 *   4. Store param_2 in slot uVar3 of the extended descriptor table.
 *
 * MIPS confirmed args: FUN_8001d4f0(a0=param_1, a1=param_2),
 *   FUN_8002c99c(a0=param_1, a1=s0).
 * HIGH confidence.
 * ================================================================ */
void FUN_8002cbe8(intptr_t param_1, intptr_t param_2)
{
    intptr_t iVar1;   /* slot weapon object ptr */
    int      iVar2;   /* slot descriptor offset (0x24..0x2c) */
    uint32_t uVar3;   /* slot iterator / eject index */
    uint32_t uVar4;   /* damageBits */

    /* Append child and clear its spawnId. */
    *(int16_t *)(uintptr_t)(param_2 + 6) = 0;
    FUN_8001d4f0((uint32_t *)(uintptr_t)param_1, (uint32_t *)(uintptr_t)param_2);
    if (weapon_slot_trace_enabled()) {
        fprintf(stderr,
                "v8: weapon slot attach vehicle=%p child=%p kind=%d active=%u "
                "cb=%p raw=0x%08x slots=(%p,%p,%p)\n",
                (void *)(uintptr_t)param_1, (void *)(uintptr_t)param_2,
                (int)*(int8_t *)(uintptr_t)(param_2 + 8),
                (unsigned)*(uint8_t *)(uintptr_t)(param_1 + 0xb3),
                (void *)Object_CallbackFromPsxSlot((const void *)(uintptr_t)param_2),
                (unsigned)*(uint32_t *)(uintptr_t)(param_2 + 0x64),
                (void *)(uintptr_t)*(uint32_t *)(uintptr_t)(param_1 + 0x110),
                (void *)(uintptr_t)*(uint32_t *)(uintptr_t)(param_1 + 0x114),
                (void *)(uintptr_t)*(uint32_t *)(uintptr_t)(param_1 + 0x118));
    }

    uVar3 = 0;
    if (*(int *)(uintptr_t)(param_1 + 0x110) != 0) {
        iVar2 = 0x24;
        iVar1 = param_1;
        do {
            if (2 < (int)uVar3) break;
            iVar2 = iVar2 + 4;
            /* Same kind as incoming weapon → toggle (negate kind byte) and return. */
            if (*(int8_t *)(uintptr_t)(*(int *)(uintptr_t)(iVar1 + 0x110) + 8)
                == *(int8_t *)(uintptr_t)(param_2 + 8))
            {
                *(int8_t *)(uintptr_t)(param_2 + 8) =
                    (int8_t)mips_subu_i32(0, (int32_t)(uint32_t)*(uint8_t *)(uintptr_t)(param_2 + 8));
                if (weapon_slot_trace_enabled()) {
                    fprintf(stderr,
                            "v8: weapon slot attach duplicate-toggle vehicle=%p "
                            "child=%p new_kind=%d\n",
                            (void *)(uintptr_t)param_1,
                            (void *)(uintptr_t)param_2,
                            (int)*(int8_t *)(uintptr_t)(param_2 + 8));
                }
                return;
            }
            iVar1  = mips_addu_i32(iVar1, 4);
            uVar3  = (uint32_t)mips_addu_i32((int32_t)uVar3, 1);
        } while (*(int *)(uintptr_t)(param_1 + iVar2 + 0xec) != 0);
    }

    if (uVar3 == 3) {
        /* All slots occupied: select which to eject. */
        uVar4 = (uint32_t)*(uint8_t *)(uintptr_t)(param_1 + 0xb3);
        uVar3 = uVar4;
        if ((0 < *(int16_t *)(uintptr_t)(param_1 + 6)) &&
            (*(int8_t *)(uintptr_t)(*(int *)(uintptr_t)(param_1 + uVar4 * 4 + 0x110) + 8) == 6))
        {
            uVar3 = 2;  /* default: eject slot 2 (homing case) */
            if (uVar4 != 0)
                uVar3 = (uint32_t)mips_addu_i32((int32_t)uVar4, -1);
        }
        FUN_8002c99c(param_1, (int)uVar3);
    }

    /* Write new weapon into the chosen (or first available) slot. */
    *(int *)(uintptr_t)(param_1 + mips_mult_lo_i32(mips_addu_i32((int32_t)uVar3, 9), 4) + 0xec) = param_2;
    if (weapon_slot_trace_enabled()) {
        fprintf(stderr,
                "v8: weapon slot attach stored vehicle=%p slot=%u child=%p "
                "slots=(%p,%p,%p)\n",
                (void *)(uintptr_t)param_1, (unsigned)uVar3,
                (void *)(uintptr_t)param_2,
                (void *)(uintptr_t)*(uint32_t *)(uintptr_t)(param_1 + 0x110),
                (void *)(uintptr_t)*(uint32_t *)(uintptr_t)(param_1 + 0x114),
                (void *)(uintptr_t)*(uint32_t *)(uintptr_t)(param_1 + 0x118));
    }
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* --- SLUS_005.10 FUN_8001d5a0 --- */
int FUN_8001d5a0(int param_1)
{
  int iVar1;
  iVar1 = *(int *)(param_1 + 0x3c);
  while ((iVar1 != 0 && (*(int *)(iVar1 + 0x38) != param_1))) {
    param_1 = *(int *)(param_1 + 0x3c);
    iVar1 = *(int *)(param_1 + 0x3c);
  }
  return iVar1;
}

/* --- SLUS_005.10 FUN_8002c99c --- */
void FUN_8002c99c(int param_1,int param_2)
{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  iVar1 = FUN_8003ff28(*(undefined4 *)(param_1 + (param_2 + 9) * 4 + 0xec));
  *(undefined2 *)(iVar1 + 6) = *(undefined2 *)(param_1 + 6);
  iVar3 = *(int *)(param_1 + 0x80);
  if (iVar3 < 0) { iVar3 = iVar3 + 0x7f; }
  *(int *)(iVar1 + 0x88) = *(int *)(iVar1 + 0x88) + (iVar3 >> 7);
  iVar3 = *(int *)(param_1 + 0x84);
  if (iVar3 < 0) { iVar3 = iVar3 + 0x7f; }
  *(int *)(iVar1 + 0x8c) = *(int *)(iVar1 + 0x8c) + (iVar3 >> 7);
  iVar3 = *(int *)(param_1 + 0x88);
  if (iVar3 < 0) { iVar3 = iVar3 + 0x7f; }
  *(int *)(iVar1 + 0x90) = *(int *)(iVar1 + 0x90) + (iVar3 >> 7);
  uVar2 = FUN_8004410c();
  FUN_8004483c(uVar2,uRam000005f8,0x29,iVar1 + 0x24);
  if ((*(short *)(iVar1 + 0xc) != 0) && (*(char *)(iVar1 + 8) != '\x06')) {
    *(undefined1 **)(iVar1 + 100) = &LAB_8003cb64;
  }
  return;
}

/* --- SLUS_005.10 FUN_8002ca94 --- */
void FUN_8002ca94(int param_1,uint param_2)
{
  byte bVar1;
  short sVar2;
  int iVar3;
  uint uVar4;
  bVar1 = *(byte *)(param_1 + 0xb3);
  uVar4 = (uint)bVar1;
  FUN_8002c99c();       /* dropped args: (param_1, param_2) */
  iVar3 = param_2 * 4 + param_1;
  FUN_80044c44(iVar3 + 0x110,iVar3 + 0x114,(2 - param_2) * 4);
  *(undefined4 *)(param_1 + 0x118) = 0;
  if ((int)uVar4 <= (int)param_2) {
    if (param_2 == 0) goto LAB_8002cb18;
    if (uVar4 != param_2) { return; }
    if (*(int *)(iVar3 + 0x110) != 0) goto LAB_8002cb18;
  }
  *(byte *)(param_1 + 0xb3) = bVar1 - 1;
LAB_8002cb18:
  if (uVar4 == param_2) {
    iVar3 = *(int *)(param_1 + (uint)*(byte *)(param_1 + 0xb3) * 4 + 0x110);
    if (iVar3 != 0) {
      sVar2 = 0x1e;
      if (0x1e < *(short *)(iVar3 + 6)) { sVar2 = *(short *)(iVar3 + 6); }
      *(short *)(iVar3 + 6) = sVar2;
    }
  }
  return;
}

/* --- SLUS_005.10 FUN_8002cb7c --- */
void FUN_8002cb7c(int param_1)
{
  int iVar1;
  int iVar2;
  int iVar3;
  iVar1 = FUN_8001d5a0();   /* dropped arg: param_1 */
  if (iVar1 != 0) {
    iVar3 = 0;
    iVar2 = 0x24;
    do {
      if (*(int *)(iVar1 + iVar2 + 0xec) == param_1) {
        FUN_8002ca94();     /* dropped args: (iVar1, iVar3) */
        return;
      }
      iVar3 = iVar3 + 1;
      iVar2 = iVar2 + 4;
    } while (iVar3 < 3);
  }
  return;
}

/* --- SLUS_005.10 FUN_8002cbe8 --- */
void FUN_8002cbe8(int param_1,int param_2)
{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  *(undefined2 *)(param_2 + 6) = 0;
  FUN_8001d4f0();     /* dropped args: (param_1, param_2) */
  uVar3 = 0;
  if (*(int *)(param_1 + 0x110) != 0) {
    iVar2 = 0x24;
    iVar1 = param_1;
    do {
      if (2 < (int)uVar3) break;
      iVar2 = iVar2 + 4;
      if (*(char *)(*(int *)(iVar1 + 0x110) + 8) == *(char *)(param_2 + 8)) {
        *(char *)(param_2 + 8) = -*(char *)(param_2 + 8);
        return;
      }
      iVar1 = iVar1 + 4;
      uVar3 = uVar3 + 1;
    } while (*(int *)(param_1 + iVar2 + 0xec) != 0);
  }
  if (uVar3 == 3) {
    uVar4 = (uint)*(byte *)(param_1 + 0xb3);
    uVar3 = uVar4;
    if (((0 < *(short *)(param_1 + 6)) &&
        (*(char *)(*(int *)(param_1 + uVar4 * 4 + 0x110) + 8) == '\x06')) && (uVar3 = 2, uVar4 != 0))
    {
      uVar3 = uVar4 - 1;
    }
    FUN_8002c99c(param_1,uVar3);
  }
  *(int *)(param_1 + (uVar3 + 9) * 4 + 0xec) = param_2;
  return;
}

#endif  /* GHIDRA REF */
