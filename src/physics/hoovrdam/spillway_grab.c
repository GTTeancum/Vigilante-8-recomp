/* spillway_grab.c -- Hoover Dam spillway / floodgate grab.
 *
 * Source: HOOVRDAM.DLL  FUN_80101734.
 *
 * Companion to siren_strobe.c, this is the spillway grab logic that
 * catches vehicles attempting to traverse the dam crest. When a
 * vehicle enters mode==3 (impact) with the "armed" flag self[0]
 * 0x10000 set, the spillway grabs it:
 *   - alloc/reuse FX slot at vehicle+0xd3
 *   - bind grab FX + snare projectile
 *   - rebind vehicle tick to FUN_801006cc, state=8
 *   - mark `*v |= 0x3000020`, clear ~0x2
 *   - pick path grab-point index (rand, retry until != self+6 kind)
 *   - heap-alloc 0x40-byte carrier descriptor with preset coordinates
 *     differentiated by spillway side (Z < 0x5210000 -> +0x50000,
 *     else -0x50000).
 *
 * Pad-trigger path (mode 0/8 + DAT_1f80000c controller record kind==2):
 *   - lever push: re-arms 0x10000 only if controller stick magnitude
 *     beyond 0xf580 and within bounding box (|+0x12000 - posY| < 0x10000
 *     and |posZ delta| < 0xa000). On match, schedule damage.
 *
 * mode dispatch:
 *   2 -- silent retire
 *   3, 8 (default) -- main grab logic
 *
 * MED.
 */
#include <stdint.h>

extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
extern uint8_t  Pool_AllocSFX(void);                                 /* FUN_8004410c */
extern void Pool_BindFXOnObject(uint32_t h, uint32_t bin, int slot, int aux); /* FUN_800443c8 */
extern void Pool_BindSnareToObject(uint32_t h, uint32_t bin, int slot, uint32_t *xyz); /* FUN_800447e8 */
extern void SFX_Update(int h, int posVoxel);
extern int  Rand255(void);
extern void Damage_Apply_AgainstSelf(void *self, void *param);
extern void *Heap_AllocOrRetry(uint32_t n);
extern int  HD_PathLookup(int8_t pathIdx);                           /* func_0x80021888 */
extern void Spawner_Promote(uint32_t target);                        /* func_0x8003dbb0 */
extern void Damage_FromImpulse(uint32_t *self, uint32_t *imp);       /* func_0x8002239c */
extern void Damage_StandardVehicle(void *self, uint32_t *imp);       /* func_0x80022320 */
extern uint32_t _DAT_1f80000c;
extern uint32_t FUN_801006cc;

static inline int32_t mips_addu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a + (uint32_t)b);
}

static inline int32_t mips_subu_i32(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a - (uint32_t)b);
}

static inline int32_t mips_sll_i32(int32_t v, unsigned sh)
{
    return (int32_t)((uint32_t)v << sh);
}

static inline int32_t mips_abs_i32(int32_t v)
{
    return (v < 0) ? mips_subu_i32(0, v) : v;
}

uint32_t HD_SpillwayGrab(uint32_t *self, uint32_t mode, uint32_t *arg)
{
    if (mode == 2) goto disarm;
    if (mode == 3 || mode == 8 || mode > 3) {
        uint32_t *vic = (uint32_t *)(uintptr_t)*arg;
        if ((char)vic[1] == 2 && (*self & 0x10000)) {
            if (*vic & 0x2000000) return 0;
            char sfx = *((char *)vic + 0xd3);
            if (sfx == 0) {
                sfx = (char)Pool_AllocSFX();
                *((char *)vic + 0xd3) = sfx;
            }
            Pool_BindFXOnObject(sfx, *(uint32_t *)(self[0x16] + 8), 3, 0);
            uint8_t h = Pool_AllocSFX();
            Pool_BindSnareToObject(h, *(uint32_t *)(self[0x16] + 8), 4, vic + 9);
            SFX_Update((int)*((char *)vic + 5), 0);
            Object_SetCallbackPsxSlot(vic, (uintptr_t)&FUN_801006cc);
            *(uint8_t *)(vic + 2) = 8;
            *vic = (*vic & ~2u) | 0x3000020u;
            uint8_t pick;
            do {
                int r = Rand255();
                pick  = (uint8_t)mips_addu_i32(mips_sll_i32(r, 2) >> 15, 0x3c);
                *((char *)vic + 0xd2) = pick;
            } while (pick == (uint8_t)*(int16_t *)((char *)self + 6));
            Damage_Apply_AgainstSelf(vic, (void *)(intptr_t)0x3c);
            uint32_t *bin = (uint32_t *)(uintptr_t)vic[0x38];
            if (bin == NULL) return 0;
            uint32_t *desc = (uint32_t *)Heap_AllocOrRetry(0x40);
            int      path = HD_PathLookup(*((char *)vic + 0xd2));
            desc[0] = bin[0x12]; desc[1] = bin[0x13]; desc[2] = bin[0x14];
            desc[3] = 0x78;
            desc[4] = 0x39f0000;
            desc[5] = 0x25f800;
            desc[6] = 0x5210000;
            desc[7] = 0xf0;
            desc[8] = *(uint32_t *)(path + 0x48);
            desc[9] = (uint32_t)mips_subu_i32(*(int *)(path + 0x4c), 0x12000);
            int dz  = *(int *)(path + 0x50) < 0x5210000 ? 0x50000 : -0x50000;
            desc[10] = (uint32_t)mips_addu_i32(*(int *)(path + 0x50), dz);
            desc[11] = 0;
            Spawner_Promote((uint32_t)(uintptr_t)bin);
        }
        Damage_FromImpulse((uint32_t *)(uintptr_t)*arg, arg);
        /* fall through */
    }
    if (*(char *)(_DAT_1f80000c + 4) == 2) {
        int stick = *(int *)(_DAT_1f80000c + 0x80);
        uint32_t side = 0x50000;
        if (*(int16_t *)((char *)self + 0x42) != 0) {
            stick = mips_subu_i32(0, stick);
            side = (uint32_t)(0x5f580 < stick);
            if (side == 0) goto disarm_then_pass;
        }
        if ((int)(side | 0xf580u) < stick) {
            int dy = mips_subu_i32(mips_addu_i32(*(int *)(_DAT_1f80000c + 0x28),
                                                 0x12000),
                                   (int)self[0x13]);
            dy = mips_abs_i32(dy);
            if (dy < 0x10000) {
                int dz = mips_subu_i32(*(int *)(_DAT_1f80000c + 0x2c),
                                       (int)self[0x14]);
                dz = mips_abs_i32(dz);
                if (dz < 0xa000) {
                    *self |= 0x10000u;
                    Damage_Apply_AgainstSelf(self, NULL);
                }
            }
        }
    }
disarm_then_pass:
    *self &= ~0x10000u;
    Damage_StandardVehicle(self, arg);
disarm:
    *self &= ~0x10000u;
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

/* --- HOOVRDAM.DLL FUN_80101734  (from analysis/dll/HOOVRDAM/decomp/80101734.c) --- */
// addr: 0x80101734  name: FUN_80101734

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101734(uint *param_1,uint param_2,uint *param_3)

{
  char cVar1;
  undefined4 uVar2;
  int iVar3;
  uint uVar4;
  uint *puVar5;
  int iVar6;
  uint *puVar7;
  uint uVar8;
  
  if (param_2 == 3) {
LAB_80101780:
    puVar7 = (uint *)*param_3;
    if (((char)puVar7[1] == '\x02') && ((*param_1 & 0x10000) != 0)) {
      if ((*puVar7 & 0x2000000) != 0) {
        return 0;
      }
      cVar1 = *(char *)((int)puVar7 + 0xd3);
      if (cVar1 == '\0') {
        cVar1 = FUN_8004410c/*0x8004410c*/();
        *(char *)((int)puVar7 + 0xd3) = cVar1;
      }
      FUN_800443c8/*0x800443c8*/(cVar1,*(undefined4 *)(param_1[0x16] + 8),3,0);
      uVar2 = FUN_8004410c/*0x8004410c*/();
      FUN_800447e8/*0x800447e8*/(uVar2,*(undefined4 *)(param_1[0x16] + 8),4,puVar7 + 9);
      FUN_80044574/*0x80044574*/((int)*(char *)((int)puVar7 + 5),0);
      puVar7[0x19] = (uint)FUN_801006cc;
      *(undefined1 *)(puVar7 + 2) = 8;
      *puVar7 = *puVar7 & 0xfffffffd | 0x3000020;
      puVar5 = (uint *)0x3000020;
      do {
        iVar3 = FUN_80017160/*0x80017160*/(puVar5);
        uVar4 = ((iVar3 << 2) >> 0xf) + 0x3c;
        *(char *)((int)puVar7 + 0xd2) = (char)uVar4;
        puVar5 = puVar7;
      } while ((uVar4 & 0xff) == (int)*(short *)((int)param_1 + 6));
      FUN_80020890/*0x80020890*/(puVar7,0x3c);
      param_1 = (uint *)puVar7[0x38];
      if (param_1 == (uint *)0x0) {
        return 0;
      }
      puVar5 = (uint *)Heap_AllocOrRetry/*0x800116f4*/(0x40);
      iVar3 = FUN_80021888/*0x80021888*/(*(undefined1 *)((int)puVar7 + 0xd2));
      uVar4 = param_1[0x13];
      uVar8 = param_1[0x14];
      *puVar5 = param_1[0x12];
      puVar5[1] = uVar4;
      puVar5[2] = uVar8;
      puVar5[3] = 0x78;
      puVar5[4] = 0x39f0000;
      puVar5[6] = 0x5210000;
      puVar5[5] = 0x25f800;
      puVar5[7] = 0xf0;
      puVar5[8] = *(uint *)(iVar3 + 0x48);
      param_3 = puVar5 + 8;
      puVar5[9] = *(int *)(iVar3 + 0x4c) - 0x12000;
      iVar6 = -0x50000;
      if (*(int *)(iVar3 + 0x50) < 0x5210000) {
        iVar6 = 0x50000;
      }
      puVar5[10] = *(int *)(iVar3 + 0x50) + iVar6;
      puVar5[0xb] = 0;
      func_0x8003dbb0(param_1);
    }
    FUN_8002239c/*0x8002239c*/(param_1,param_3);
  }
  else {
    if ((param_2 < 4) && (param_2 == 2)) goto LAB_80101a6c;
    if (param_2 != 8) goto LAB_80101780;
  }
  if (*(char *)(_DAT_1f80000c + 4) == '\x02') {
    iVar3 = *(int *)(_DAT_1f80000c + 0x80);
    uVar4 = 0x50000;
    if (*(short *)((int)param_1 + 0x42) != 0) {
      iVar3 = -iVar3;
      uVar4 = (uint)(0x5f580 < iVar3);
      if (uVar4 == 0) goto LAB_80101a44;
    }
    if ((int)(uVar4 | 0xf580) < iVar3) {
      iVar3 = (*(int *)(_DAT_1f80000c + 0x28) + 0x12000) - param_1[0x13];
      if (iVar3 < 0) {
        iVar3 = -iVar3;
      }
      if (iVar3 < 0x10000) {
        iVar3 = *(int *)(_DAT_1f80000c + 0x2c) - param_1[0x14];
        if (iVar3 < 0) {
          iVar3 = -iVar3;
        }
        if (iVar3 < 0xa000) {
          *param_1 = *param_1 | 0x10000;
          FUN_80020890/*0x80020890*/(param_1,0);
        }
      }
    }
  }
LAB_80101a44:
  *param_1 = *param_1 & 0xfffeffff;
  FUN_80022320/*0x80022320*/(param_1,param_3);
LAB_80101a6c:
  *param_1 = *param_1 & 0xfffeffff;
  return 0;
}

#endif  /* GHIDRA REF */
