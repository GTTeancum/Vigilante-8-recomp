/* blimp_pilot.c -- Casino City blimp roll/yaw pilot.
 *
 * Source: CASNOCTY.DLL  FUN_801006d4.
 *
 * Per-tick handler for the casino blimp. Reads the local player's
 * control bits (puVar14[2] high bits = state byte; 0x800 = roll left,
 * 0x1000 = recover-to-center) and integrates a yaw rate at obj+0x29
 * clamped to [-0x2aa, +0x2aa]. Also copies a 16-word transform block
 * from the carried pilot record so the blimp follows the host's pose.
 *
 * mode:
 *   0  -- per-tick integrate + control sample
 *   2  -- shutdown / retire
 *
 * State machine (pilot.byte+2):
 *   <4  -- normal; if |yaw| > 0x2a, mark hostile (state |= 0xff)
 *   >=4 -- hostile/inverted; player input ignored
 *   2,signed -- on big impact (`*self & 0x1000000`), eject pilot:
 *               alloc projectile, launch from blimp pos, mark blimp
 *               retiring (`*self |= 0x20`, schedule Damage_Apply 120f),
 *               reset to state 0 + downforce Y = 0xee1f.
 *
 * MED.
 */
#include <stdint.h>

extern uint32_t Pool_AllocProjectile(void);                                  /* FUN_8004410c */
extern void     Pool_LaunchProjectile(uint32_t h, uint32_t bin, int kind,
                                      uint32_t *spawnXyz);                   /* FUN_8004483c */
extern void     Damage_Apply_AgainstSelf(void *self, void *param);           /* FUN_80020890 */
extern void     Object_Detach(uint32_t *self);                               /* func_0x80031294 */
extern void     Vehicle_DropBindings(uint32_t bin);                          /* func_0x8003d8c4 */
extern uint32_t _DAT_800658fc;

uint32_t CC_BlimpPilot(uint32_t *self, int mode)
{
    if (mode == 2) goto retire;

    uint32_t *pilot = (uint32_t *)(uintptr_t)self[0x1e];
    int  hostBase   = ~(int)*(int16_t *)((char *)self + 6) * 0x18;
    int16_t *kindP  = (int16_t *)(hostBase + -0x7ff9a3d8);
    uint32_t pad    = *(uint32_t *)(hostBase + -0x7ff9a3d0);
    uint32_t *yawP  = self + 0x29;

    if ((*self & 1) != 0) {
        int32_t vx = (int32_t)self[0x20]; if (vx < 0) vx += 0x7f;
        self[9]  += vx >> 7;
        int32_t vy = (int32_t)self[0x21]; if (vy < 0) vy += 0x7f;
        self[10] += vy >> 7;
        int32_t vz = (int32_t)self[0x22]; if (vz < 0) vz += 0x7f;
        self[0xb]+= vz >> 7;
    }

    int16_t kind = *kindP;
    int     applyDamp = 0;
    if (kind == 3 || (kind >= 4 && kind < 6) || kind == 2) {
        if (kind == 3) goto integrate_axis;
        uint32_t rollMask = (kind == 2) ? (pad & 0x800) : 0;
        if (rollMask) {
            int32_t y = (int16_t)self[0x29] - 0x10;
            if (y < -0x2aa) y = -0x2aa;
            *(int16_t *)(self + 0x29) = (int16_t)y;
        }
        if (pad & 0x1000) *(uint16_t *)(self + 0x29) = 1;
        int32_t y = (int16_t)self[0x29];
        if (y < 0) y += 0xf;
        *(uint16_t *)(self + 0x29) = (uint16_t)self[0x29] - (int16_t)(y >> 4);
integrate_axis: {
            uint8_t  b   = *(uint8_t *)(yawP + 8);
            *(uint16_t *)(self + 0x29) = (uint16_t)((b - 0x80) * 5);
            uint32_t u   = (uint8_t)(b - 0x70);
            int      iv  = u - 0x80; if (iv < 0) iv = u - 0x7d;
            int      target = (int16_t)*yawP + (iv >> 2);
            int      clamp  = (target < -0x2aa) ? -0x2aa
                             : (target >  0x2aa) ?  0x2aa : target;
            int      cur    = (int16_t)*yawP; if (cur < 0) cur += 0xf;
            *(int16_t *)yawP = (int16_t)(clamp - (cur >> 4));
        }
    }

    /* Hostile-state transition. */
    if ((char)pilot[2] < 4) {
        int y = (int16_t)*yawP; if (y < 0) y = -y;
        if (y > 0x2a) *(uint8_t *)(pilot + 2) = 0xff;
    }
    if ((char)pilot[2] < 0) {
        int v = (int16_t)*yawP; if (v < 0) v += 0x1f;
        *(int16_t *)((char *)pilot + 0x42) += (int16_t)(v >> 5);
    }

    /* Copy transform from pilot record. */
    self[4]  = pilot[4];  self[5]  = pilot[5];  self[6]  = pilot[6];  self[7]  = pilot[7];
    self[8]  = pilot[8];  self[9]  = pilot[9];  self[10] = pilot[10]; self[0xb]= pilot[0xb];
    self[0x12] = pilot[0x12]; self[0x13] = pilot[0x13]; self[0x14] = pilot[0x14];

    char st = (char)pilot[2];
    if (st < 5) {
        if (*self & 1) return 0;
        if (st == 4)   return 0;
        if (st != 2 && (pad & 0x1000000) == 0) return 0;
    }
    /* Ejection: spawn pilot projectile, mark blimp retiring. */
    uint32_t h = Pool_AllocProjectile();
    Pool_LaunchProjectile(h, _DAT_800658fc, 0x14, pilot + 0x12);
    pilot[0x24] = 0;
    *pilot |= 0x20u;
    Damage_Apply_AgainstSelf(pilot, (void *)(intptr_t)0x78);
    Object_Detach(self);
    Vehicle_DropBindings(self[0x38]);
    *self = (*self & 0xfeffffdd) | 8;
    st = (char)pilot[2];
    int armed = (st == 2);
    if (st == 2 || st < 0) { *(uint8_t *)(pilot + 2) = 0; armed = 1; }
    else                    armed = (st < 5);
    if (armed) return 0;
    self[0x21] = 0xffffee1fu;
retire:
    *self |= 2u;
    return 0;
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 *
 * These are the raw Ghidra pseudo-C exports for the function(s)
 * this file cleans up. Use them to audit any MED-confidence
 * rewrite line-by-line. Regenerated by tools/restore_ghidra_refs.py.
 * ============================================================ */
#if 0

/* --- CASNOCTY.DLL FUN_801006d4  (from analysis/dll/CASNOCTY/decomp/801006d4.c) --- */
// addr: 0x801006d4  name: FUN_801006d4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801006d4(uint *param_1,int param_2)

{
  char cVar1;
  byte bVar2;
  char cVar3;
  undefined4 uVar4;
  int iVar5;
  short *psVar6;
  uint uVar7;
  short sVar8;
  int iVar9;
  uint *puVar10;
  uint uVar11;
  uint uVar12;
  uint uVar13;
  uint *puVar14;
  
  if ((param_2 != 0) && (param_2 == 2)) goto LAB_80100a74;
  puVar14 = (uint *)param_1[0x1e];
  iVar5 = ~(int)*(short *)((int)param_1 + 6) * 0x18;
  psVar6 = (short *)(iVar5 + -0x7ff9a3d8);
  uVar11 = *(uint *)(iVar5 + -0x7ff9a3d0);
  puVar10 = param_1 + 0x29;
  if ((*param_1 & 1) != 0) {
    uVar7 = param_1[0x20];
    if ((int)uVar7 < 0) {
      uVar7 = uVar7 + 0x7f;
    }
    puVar10 = (uint *)param_1[0x21];
    param_1[9] = param_1[9] + ((int)uVar7 >> 7);
    if ((int)puVar10 < 0) {
      puVar10 = (uint *)((int)puVar10 + 0x7f);
    }
    uVar7 = param_1[0x22];
    param_1[10] = param_1[10] + ((int)puVar10 >> 7);
    if ((int)uVar7 < 0) {
      uVar7 = uVar7 + 0x7f;
    }
    psVar6 = (short *)((int)uVar7 >> 7);
    param_1[0xb] = param_1[0xb] + (int)psVar6;
  }
  sVar8 = *psVar6;
  if (sVar8 == 3) {
LAB_80100854:
    bVar2 = *(byte *)(psVar6 + 8);
    *(ushort *)(param_1 + 0x29) = (bVar2 - 0x80) * 5;
    uVar7 = (uint)*(byte *)(bVar2 - 0x70);
    iVar5 = uVar7 - 0x80;
    if (iVar5 < 0) {
      iVar5 = uVar7 - 0x7d;
    }
    iVar9 = (int)(short)*puVar10 + (iVar5 >> 2);
    iVar5 = -0x2aa;
    if ((-0x2ab < iVar9) && (iVar5 = 0x2aa, iVar9 < 0x2ab)) {
      iVar5 = iVar9;
    }
    iVar9 = (int)(short)*puVar10;
    if (iVar9 < 0) {
      iVar9 = iVar9 + 0xf;
    }
    *(short *)puVar10 = (short)iVar5 - (short)(iVar9 >> 4);
  }
  else {
    if (sVar8 < 4) {
      uVar7 = uVar11 & 0x800;
      if (sVar8 != 2) {
        sVar8 = 1;
        goto LAB_801007c0;
      }
LAB_801007d4:
      if (uVar7 != 0) {
        iVar9 = (short)param_1[0x29] + -0x10;
        iVar5 = -0x2aa;
        if (-0x2aa < iVar9) {
          iVar5 = iVar9;
        }
        *(short *)(param_1 + 0x29) = (short)iVar5;
      }
      if ((uVar11 & 0x1000) != 0) {
        *(undefined2 *)(param_1 + 0x29) = 1;
      }
      iVar5 = (int)(short)param_1[0x29];
      psVar6 = (short *)(uint)(ushort)param_1[0x29];
      if (iVar5 < 0) {
        iVar5 = iVar5 + 0xf;
      }
      *(ushort *)(param_1 + 0x29) = (ushort)param_1[0x29] - (short)(iVar5 >> 4);
      goto LAB_80100854;
    }
LAB_801007c0:
    uVar7 = (uint)(sVar8 < 6);
    if (uVar7 != 0) goto LAB_801007d4;
  }
  if ((char)puVar14[2] < '\x04') {
    iVar5 = (int)(short)*puVar10;
    if (iVar5 < 0) {
      iVar5 = -iVar5;
    }
    if (0x2a < iVar5) {
      *(undefined1 *)(puVar14 + 2) = 0xff;
    }
  }
  if ((char)puVar14[2] < '\0') {
    iVar5 = (int)(short)*puVar10;
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0x1f;
    }
    *(short *)((int)puVar14 + 0x42) = *(short *)((int)puVar14 + 0x42) + (short)(iVar5 >> 5);
  }
  uVar7 = puVar14[5];
  uVar12 = puVar14[6];
  uVar13 = puVar14[7];
  param_1[4] = puVar14[4];
  param_1[5] = uVar7;
  param_1[6] = uVar12;
  param_1[7] = uVar13;
  uVar7 = puVar14[9];
  uVar12 = puVar14[10];
  uVar13 = puVar14[0xb];
  param_1[8] = puVar14[8];
  param_1[9] = uVar7;
  param_1[10] = uVar12;
  param_1[0xb] = uVar13;
  uVar7 = puVar14[0x13];
  uVar12 = puVar14[0x14];
  param_1[0x12] = puVar14[0x12];
  param_1[0x13] = uVar7;
  param_1[0x14] = uVar12;
  cVar1 = (char)puVar14[2];
  if (cVar1 < '\x05') {
    if ((*param_1 & 1) != 0) {
      return 0;
    }
    if (cVar1 == '\x04') {
      return 0;
    }
    if ((cVar1 != '\x02') && ((uVar11 & 0x1000000) == 0)) {
      return 0;
    }
  }
  uVar4 = FUN_8004410c/*0x8004410c*/();
  FUN_8004483c/*0x8004483c*/(uVar4,_DAT_800658fc,0x14,puVar14 + 0x12);
  puVar14[0x24] = 0;
  *puVar14 = *puVar14 | 0x20;
  FUN_80020890/*0x80020890*/(puVar14,0x78);
  func_0x80031294(param_1);
  func_0x8003d8c4(param_1[0x38]);
  *param_1 = *param_1 & 0xfeffffdd | 8;
  cVar1 = (char)puVar14[2];
  cVar3 = '\x02';
  if ((cVar1 == '\x02') || (cVar3 = cVar1 < '\x05', cVar1 < '\0')) {
    *(undefined1 *)(puVar14 + 2) = 0;
  }
  if (cVar3 != '\0') {
    return 0;
  }
  param_1[0x21] = 0xffffee1f;
LAB_80100a74:
  *param_1 = *param_1 | 2;
  return 0;
}

#endif  /* GHIDRA REF */
