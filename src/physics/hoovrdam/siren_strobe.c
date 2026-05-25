/* siren_strobe.c -- Hoover Dam alarm-siren strobe + grab logic.
 *
 * Source: HOOVRDAM.DLL  FUN_80100c30.
 *
 * Strobe-light controller that ALSO grabs vehicles that drive too
 * close (similar to SF conveyor grab). Owns a per-frame timeline
 * buffer of strobe-light cells at self+0x22 (8-cell ring) keyed by
 * self+0x21 (write index) and self+0x20 (countdown).
 *
 * mode dispatch (large fall-through):
 *   0  -- per-tick: drain countdown, on underflow copy next ring slot
 *         into DAT_8008f660 (active light cell). Reseed strobe rate
 *         via Rand255.
 *   1  -- post-spawn: pick siren path waypoint (FUN_8001ffd4@bank
 *         0x80065a50) + bind via func_0x8003d080. _DAT_80065a10 =
 *         (path attached).
 *   2  -- post-init: schedule first 240-frame fire.
 *   0xb -- bind FX channel (Pool_BindFXOnObject).
 *   0x11-- begin per-frame damage handler.
 *   9   -- main "grab vehicle" event:
 *          * if vehicle is too close (Z+0xfb140000 > 0x540000), kick
 *            into capture state: set tick callback FUN_801006cc,
 *            stop FX, set fall-velocity 0x1c980, schedule damage and
 *            spawn a 0x40-byte carrier descriptor on the heap with
 *            preset lift coords (0x39f0000, 0x5470000, 0x20f800, 0x168
 *            + grab-point offsets at +0x12000/-0x50000).
 *          * every 16 frames emit a strobe particle (0x24a/0x24b/0x249
 *            depending on flag 0x80000) at the grabbed object's pos,
 *            yaw = Math_Atan2_Pos(target).
 *   7  -- post-grab maintenance.
 *   4  -- free ring buffer, return.
 *
 * MED. The chained fall-through is preserved verbatim from the
 * original switch-with-no-breaks idiom.
 */
#include <stdint.h>

extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);
extern int  Rand255(void);                                          /* FUN_80017160 */
extern uint32_t Path_Pick(uint32_t bank, int seed);                 /* FUN_8001ffd4 */
extern int  Path_AttachToObj(uint32_t flag, uint32_t path);          /* func_0x8003d080 */
extern void Damage_Apply_AgainstSelf(void *self, void *param);       /* FUN_80020890 */
extern void HD_QueueDamage(void);                                    /* func_0x80023d00 */
extern uint8_t  Pool_AllocSFX(void);                                 /* FUN_8004410c */
extern void Pool_BindFXOnObject(uint32_t h, uint32_t bin, int slot, int aux); /* FUN_800443c8 */
extern void Damage_StandardVehicleAlt(void *self, uint32_t *imp);    /* func_0x80022120 */
extern uint32_t *Object_FetchPrev(void *self);                       /* FUN_8001d5e0 */
extern uint32_t *Object_FetchPrevHead(void *self);                   /* FUN_8001d5a0 */
extern void Damage_VsImpactorAlt(uint32_t *imp, int dmg, void *p, int n); /* FUN_8002c958 */
extern void SFX_Update(int h, int posVoxel);                         /* FUN_80044574 */
extern void *Heap_AllocOrRetry(uint32_t n);                          /* FUN_800116f4 */
extern int  HD_PathLookup(int8_t pathIdx);                           /* func_0x80021888 */
extern void Spawner_Promote(uint32_t target);                        /* func_0x8003dbb0 */
extern int  Math_Atan2_Pos(int xy);                                  /* FUN_80016c88 */
extern uint32_t Object_SpawnFromBank(uint32_t bin, int kind, int prio, int flag); /* FUN_8001ac44 */
extern void Object_Suspend(void);                                    /* FUN_8002036c */
extern void Object_RetireFX(int port);                               /* FUN_8001d470 */
extern void Heap_Free(uint32_t p);                                   /* FUN_80045088 */

extern uint8_t  DAT_8008f660, DAT_80100098;
extern uint32_t DAT_80101b88;
extern uint32_t _DAT_800659fc, _DAT_800737e8, _DAT_80065a10;
extern uint32_t FUN_801006cc;
extern int FUN_8003e80c(int obj, int event, int param3);

uint32_t HD_SirenStrobe(uint32_t *self, uint32_t mode, uint32_t *arg)
{
    switch (mode) {
    case 0:
        if (arg != NULL) {
            uint32_t v = self[0x20];
            self[0x20] = v - (uintptr_t)arg;
            if ((int)(v - (uintptr_t)arg) < 0) {
                do {
                    uint32_t idx = self[0x21];
                    uint32_t *src = (uint32_t *)((idx & 7) * 0x80 + self[0x22]);
                    self[0x20] += 6;
                    uint32_t *dst = (uint32_t *)&DAT_8008f660;
                    for (int i = 0; i < 8; i++) {
                        dst[0] = src[0]; dst[1] = src[1];
                        dst[2] = src[2]; dst[3] = src[3];
                        src += 4; dst += 4;
                    }
                    self[0x21] = idx + 1;
                } while ((int)self[0x20] < 0);
            }
        }
        DAT_80101b88 = (uint32_t)Rand255();
        /* fall through */
    case 1: {
        *self = 0x80;
        self[0x22] = 0x80;
        uint32_t path = Path_Pick(0x80065a50, 0x100);
        _DAT_80065a10 = (Path_AttachToObj(0x7f000000u, path) != 0);
    }
    /* fall through */
    case 2:
        HD_QueueDamage();
        Damage_Apply_AgainstSelf(self, (void *)(intptr_t)0xf0);
        self = (uint32_t *)1;
        /* fall through */
    case 0xb: {
        uint8_t h = Pool_AllocSFX();
        Pool_BindFXOnObject(h, *(uint32_t *)(*(int *)(_DAT_800659fc + 0x58) + 8), 1, 0);
        self = (uint32_t *)(uintptr_t)h;
    }
    /* fall through */
    case 0x11:
        Damage_StandardVehicleAlt(self, arg);
        /* fall through */
    case 9: {
        int8_t cmp = 8;
        if (arg && 0x540000 < arg[2] + 0xfb140000u) {
            uint32_t *vic = Object_FetchPrev(self);
            if (*vic & 0x2000000) return 0;
            Damage_VsImpactorAlt(vic, -150, &DAT_80100098, 0);
            uint32_t bin = *(uint32_t *)(_DAT_800659fc + 0x58);
            for (uintptr_t c = vic[0xe]; c != 0; c = *(uint32_t *)(c + 0x34))
                *(uint16_t *)(*(int *)(c + 0x30) + 0x28) = 0x40;
            if (vic[0x1a] != 0) *(uint16_t *)(vic[0x1a] + 0x28) = 0x40;
            char sfx = *((char *)vic + 0xd3);
            if (sfx == 0) {
                sfx = (char)Pool_AllocSFX();
                *((char *)vic + 0xd3) = sfx;
            }
            Pool_BindFXOnObject(sfx, *(uint32_t *)(_DAT_800737e8 + 8), 3, 0);
            SFX_Update((int)*((char *)vic + 5), 0);
            Object_SetCallbackPsxSlot(vic, (uintptr_t)&FUN_801006cc);
            *(uint8_t *)(vic + 2) = 0;
            *vic = (*vic & ~2u) | 0x3000020u;
            int r = Rand255();
            *((char *)vic + 0xd2) = (char)((r << 2) >> 15) + 0x3c;
            vic[0x20] = 0;
            vic[0x21] = 0x1c980;
            vic[0x22] = 0;
            Damage_Apply_AgainstSelf(vic, NULL);
            uint32_t bin2 = vic[0x38];
            if (bin2 == 0) return 0;
            uint32_t *desc = (uint32_t *)Heap_AllocOrRetry(0x40);
            int       grab = HD_PathLookup(*((char *)vic + 0xd2));
            desc[0] = *(uint32_t *)(bin2 + 0x48);
            desc[1] = *(uint32_t *)(bin2 + 0x4c);
            desc[2] = *(uint32_t *)(bin2 + 0x50);
            desc[3] = 0x78;
            desc[4] = 0x39f0000;
            desc[5] = 0x20f800;
            desc[6] = 0x5470000;
            desc[7] = 0x168;
            desc[8] = *(uint32_t *)(grab + 0x48);
            desc[9] = *(int *)(grab + 0x4c) - 0x12000;
            desc[10]= *(int *)(grab + 0x50) + 0x50000;
            desc[11]= 0;
            Spawner_Promote(bin2);
            self = desc;
            cmp = 0;
        }
        if ((char)self[1] != cmp || (DAT_80101b88 = DAT_80101b88 + 1, (DAT_80101b88 & 0xf) != 0))
            return 0;
        uint32_t *head = Object_FetchPrevHead(self);
        if (*(int *)(head + 0x8c) < 0xee6) {
            self = (uint32_t *)Object_SpawnFromBank(*(uint32_t *)(_DAT_800659fc + 0x58),
                                                   0x24b, 0x80, 8);
        }
        uint32_t kind = (*self & 0x80000) ? 0x249u : 0x24au;
        self = (uint32_t *)Object_SpawnFromBank(*(uint32_t *)(_DAT_800659fc + 0x58),
                                                kind, 0x80, 8);
        *(uint16_t *)((char *)self + 0x42) = (uint16_t)Math_Atan2_Pos((int)(uintptr_t)(head + 0x10));
        *(uint8_t  *)(self + 1)  = 1;
        *self = 0x24u;
        self[0x12] = arg[0]; self[0x13] = arg[1]; self[0x14] = arg[2];
        Object_SetCallbackPsxSlot(self, (uintptr_t)&FUN_8003e80c);
        Object_Suspend();
    }
    /* fall through */
    case 7:
        Object_RetireFX(0x8c);
        /* fall through */
    case 4:
        Heap_Free(self[0x22]);
        return 0;
    default:
        return 0;
    }
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

/* --- HOOVRDAM.DLL FUN_80100c30  (from analysis/dll/HOOVRDAM/decomp/80100c30.c) --- */
// addr: 0x80100c30  name: FUN_80100c30

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100c30(uint *param_1,undefined4 param_2,uint *param_3)

{
  char cVar1;
  undefined2 uVar2;
  undefined4 *puVar3;
  uint *puVar4;
  int iVar5;
  uint uVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  undefined4 uVar9;
  uint uVar10;
  undefined4 uVar11;
  uint uVar12;
  undefined4 uVar13;
  
  switch(param_2) {
  case 0:
    if ((param_3 != (uint *)0x0) &&
       (uVar6 = param_1[0x20], param_1[0x20] = uVar6 - (int)param_3, (int)(uVar6 - (int)param_3) < 0
       )) {
      do {
        uVar6 = param_1[0x21];
        puVar3 = (undefined4 *)((uVar6 & 7) * 0x80 + param_1[0x22]);
        puVar7 = puVar3 + 0x20;
        param_1[0x20] = param_1[0x20] + 6;
        puVar8 = (undefined4 *)&DAT_8008f660;
        do {
          uVar9 = puVar3[1];
          uVar11 = puVar3[2];
          uVar13 = puVar3[3];
          *puVar8 = *puVar3;
          puVar8[1] = uVar9;
          puVar8[2] = uVar11;
          puVar8[3] = uVar13;
          puVar3 = puVar3 + 4;
          puVar8 = puVar8 + 4;
        } while (puVar3 != puVar7);
        param_1[0x21] = uVar6 + 1;
      } while ((int)param_1[0x20] < 0);
    }
    DAT_80101b88 = FUN_80017160/*0x80017160*/();
  case 1:
    *param_1 = 0x80;
    param_1[0x22] = 0x80;
    uVar9 = FUN_8001ffd4/*0x8001ffd4*/(0x80065a50,0x100);
    iVar5 = func_0x8003d080(0x7f000000,uVar9);
    _DAT_80065a10 = (uint)(iVar5 != 0);
  case 2:
    func_0x80023d00();
    FUN_80020890/*0x80020890*/(param_1,0xf0);
    param_1 = (uint *)0x1;
  case 0xb:
    param_1 = (uint *)FUN_8004410c/*0x8004410c*/(param_1);
    FUN_800443c8/*0x800443c8*/(param_1,*(undefined4 *)(*(int *)(_DAT_800659fc + 0x58) + 8),1,0);
  case 0x11:
    func_0x80022120(param_1,param_3);
  case 9:
    cVar1 = '\b';
    if (0x540000 < param_3[2] + 0xfb140000) {
      puVar4 = (uint *)FUN_8001d5e0/*0x8001d5e0*/(param_1);
      if ((*puVar4 & 0x2000000) != 0) {
        return 0;
      }
      FUN_8002c958/*0x8002c958*/(puVar4,0xffffff6a,&DAT_80100098,0);
      uVar9 = *(undefined4 *)(_DAT_800659fc + 0x58);
      for (uVar6 = puVar4[0xe]; uVar6 != 0; uVar6 = *(uint *)(uVar6 + 0x34)) {
        *(undefined2 *)(*(int *)(uVar6 + 0x30) + 0x28) = 0x40;
      }
      if (puVar4[0x1a] != 0) {
        *(undefined2 *)(puVar4[0x1a] + 0x28) = 0x40;
      }
      cVar1 = *(char *)((int)puVar4 + 0xd3);
      if (cVar1 == '\0') {
        cVar1 = FUN_8004410c/*0x8004410c*/(0,uVar9,0x24d,0x24c,4,0x3c);
        *(char *)((int)puVar4 + 0xd3) = cVar1;
      }
      FUN_800443c8/*0x800443c8*/(cVar1,*(undefined4 *)(_DAT_800737e8 + 8),3,0);
      FUN_80044574/*0x80044574*/((int)*(char *)((int)puVar4 + 5),0);
      puVar4[0x19] = (uint)FUN_801006cc;
      *(undefined1 *)(puVar4 + 2) = 0;
      *puVar4 = *puVar4 & 0xfffffffd | 0x3000020;
      iVar5 = FUN_80017160/*0x80017160*/();
      *(char *)((int)puVar4 + 0xd2) = (char)((iVar5 << 2) >> 0xf) + '<';
      puVar4[0x20] = 0;
      puVar4[0x21] = 0x1c980;
      puVar4[0x22] = 0;
      FUN_80020890/*0x80020890*/(puVar4);
      uVar6 = puVar4[0x38];
      if (uVar6 == 0) {
        return 0;
      }
      param_1 = (uint *)Heap_AllocOrRetry/*0x800116f4*/(0x40);
      iVar5 = FUN_80021888/*0x80021888*/(*(undefined1 *)((int)puVar4 + 0xd2));
      uVar10 = *(uint *)(uVar6 + 0x4c);
      uVar12 = *(uint *)(uVar6 + 0x50);
      *param_1 = *(uint *)(uVar6 + 0x48);
      param_1[1] = uVar10;
      param_1[2] = uVar12;
      param_1[3] = 0x78;
      param_1[4] = 0x39f0000;
      param_1[6] = 0x5470000;
      param_1[5] = 0x20f800;
      param_1[7] = 0x168;
      param_1[8] = *(uint *)(iVar5 + 0x48);
      param_1[9] = *(int *)(iVar5 + 0x4c) - 0x12000;
      param_1[10] = *(int *)(iVar5 + 0x50) + 0x50000;
      param_1[0xb] = 0;
      func_0x8003dbb0(uVar6);
      cVar1 = '\0';
    }
    if (((char)param_1[1] != cVar1) || (DAT_80101b88 = DAT_80101b88 + 1, (DAT_80101b88 & 0xf) != 0))
    {
      return 0;
    }
    iVar5 = FUN_8001d5a0/*0x8001d5a0*/(param_1);
    if (*(int *)(iVar5 + 0x8c) < 0xee6) {
      param_1 = (uint *)FUN_8001ac44/*0x8001ac44*/(*(undefined4 *)(_DAT_800659fc + 0x58),0x24b,0x80,8);
    }
    uVar9 = 0x24a;
    if ((*param_1 & 0x80000) != 0) {
      uVar9 = 0x249;
    }
    param_1 = (uint *)FUN_8001ac44/*0x8001ac44*/(*(undefined4 *)(_DAT_800659fc + 0x58),uVar9,0x80,8);
    uVar2 = Math_Atan2_Pos/*0x80016c88*/(iVar5 + 0x10);
    *(undefined2 *)((int)param_1 + 0x42) = uVar2;
    *(undefined1 *)(param_1 + 1) = 1;
    *param_1 = 0x24;
    uVar6 = param_3[1];
    uVar10 = param_3[2];
    param_1[0x12] = *param_3;
    param_1[0x13] = uVar6;
    param_1[0x14] = uVar10;
    param_1[0x19] = 0x8003e80c;
    FUN_8002036c/*0x8002036c*/();
  case 7:
    FUN_8001d470/*0x8001d470*/(0x8c);
  case 4:
    Heap_Free/*0x80045088*/(param_1[0x22]);
    return 0;
  default:
    return 0;
  }
}

#endif  /* GHIDRA REF */
