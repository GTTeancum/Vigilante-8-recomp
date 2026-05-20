// addr: 0x8010266c  name: FUN_8010266c

/* WARNING: Removing unreachable block (ram,0x80102854) */
/* WARNING: Removing unreachable block (ram,0x80102868) */
/* WARNING: Removing unreachable block (ram,0x8010286c) */
/* WARNING: Removing unreachable block (ram,0x80102aec) */
/* WARNING: Removing unreachable block (ram,0x80102af8) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_8010266c(void)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined **ppuVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  undefined2 local_68;
  short local_66;
  undefined2 local_64;
  short local_62;
  undefined2 local_60;
  short local_5e;
  undefined2 local_5c;
  undefined2 local_5a;
  undefined1 auStack_58 [32];
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  int local_2c;
  
  local_68 = 0x5a;
  local_66 = 0x128;
  local_64 = 0x140;
  local_62 = 0x7c;
  local_60 = 0x1a;
  local_5e = 0;
  local_5c = 0x40;
  local_5a = 0x40;
  local_2c = 0x1ba - (uint)(*(byte *)(DAT_80113388 + *(int *)(DAT_80113388 + 8) + 6) >> 1);
  Async_StopAllocCallback/*0x800165cc*/(0);
  iVar7 = 0;
  AsyncList_RecycleHead/*0x80011834*/();
  SetDispMask/*0x8004f4e8*/(0);
  FUN_80019e7c/*0x80019e7c*/(0);
  FUN_80016da8/*0x80016da8*/(auStack_58);
  FUN_8004d314/*0x8004d314*/(auStack_58,&DAT_80100618);
  func_0x8001d3d8();
  func_0x8001d404(1,&DAT_80100628,0xffffff);
  FUN_8001d9c0/*0x8001d9c0*/(auStack_58,0x200);
  FUN_8001d370/*0x8001d370*/();
  local_34 = func_0x8001a1e8(0,local_2c,0x280,
                             *(undefined1 *)(DAT_80113388 + *(int *)(DAT_80113388 + 8) + 6));
  local_38 = func_0x8001a24c(&local_68);
  CD_PlayTrack/*0x80043ce0*/(0);
  local_30 = -1;
  piVar1 = (int *)FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4),1);
  iVar2 = FUN_8001ac44/*0x8001ac44*/(DAT_80113390,0,0x80,0);
  V8_MemSet/*0x80044efc*/(iVar2 + 0x48,0,0xc);
  FUN_8001d708/*0x8001d708*/(iVar2);
  FUN_8001dc1c/*0x8001dc1c*/(iVar2);
  do {
    do {
      iVar6 = 0;
      FUN_80019e20/*0x80019e20*/();
      VSync/*0x80047e44*/(0);
      if (local_30 != 0) {
        func_0x8001a2ac(local_34,0,local_2c);
        local_30 = 0;
        func_0x8001a2ac(local_38,0x5a,0x128);
      }
      local_66 = 0x128;
      local_62 = *(byte *)(*piVar1 + 7) + 2;
      ppuVar4 = &PTR_s_1_PLAYER_80111da0;
      do {
        if (iVar6 == iVar7) {
          *(undefined1 *)(piVar1 + 1) = 0x80;
          *(undefined1 *)((int)piVar1 + 5) = 0x80;
          *(undefined1 *)((int)piVar1 + 6) = 0x80;
        }
        *(undefined1 *)(piVar1 + 1) = 0x7c;
        *(undefined1 *)((int)piVar1 + 5) = 0x60;
        *(undefined1 *)((int)piVar1 + 6) = 0;
        FUN_80019a58/*0x80019a58*/(piVar1,*ppuVar4,&local_68,0x4440);
        iVar6 = iVar6 + 1;
        local_66 = local_66 + local_62;
        ppuVar4 = ppuVar4 + 1;
      } while (iVar6 < 3);
      SetDispMask/*0x8004f4e8*/(1);
      local_5e = (*(byte *)(*piVar1 + 7) + 2) * (short)iVar7 + 0x118;
      uVar3 = FUN_8001a2cc/*0x8001a2cc*/(&local_60,0x20,0x20,0xffffffff);
      do {
        Buffer_StartOTagOther/*0x80011a10*/();
        *(short *)(iVar2 + 0x42) = *(short *)(iVar2 + 0x42) + 0x40;
        FUN_8001d708/*0x8001d708*/(iVar2);
        FUN_8001a4f8/*0x8001a4f8*/(uVar3,0);
        FUN_8001de08/*0x8001de08*/(iVar2);
        DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
        VSync/*0x80047e44*/(0);
        DrawSync/*0x8004f580*/(0);
        func_0x8001a584(uVar3);
        FUN_80017160/*0x80017160*/();
        Tick_PadOnly/*0x800126f0*/();
        uVar5 = _DAT_80065930 | _DAT_80065934;
        if (((uVar5 & 0x200000) != 0) && (-1 < DAT_80065319)) {
          func_0x8001a4ac(uVar3);
          iVar7 = -1;
          FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,8);
        }
      } while ((uVar5 & 0x58500000) == 0);
      FUN_8001a4f8/*0x8001a4f8*/(uVar3,0);
      func_0x8001a584(uVar3);
      DrawSync/*0x8004f580*/(0);
      func_0x8001a4ac(uVar3);
      FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,(uint)((uVar5 & 0x8400000) != 0) << 3);
      if (((uVar5 & 0x10000000) != 0) && (0 < iVar7)) {
        iVar7 = iVar7 + -1;
      }
      if (((uVar5 & 0x40000000) != 0) && (iVar7 < 2)) {
        iVar7 = iVar7 + 1;
      }
    } while ((uVar5 & 0x8400000) == 0);
    switch(iVar7) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      FUN_8001af48/*0x8001af48*/(iVar2);
      FUN_8001a994/*0x8001a994*/(DAT_80113390);
      FUN_800190a8/*0x800190a8*/(piVar1);
      FUN_800183ec/*0x800183ec*/(local_38);
      return 3;
    }
  } while( true );
}

