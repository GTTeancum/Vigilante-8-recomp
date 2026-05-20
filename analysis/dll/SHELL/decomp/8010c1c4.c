// addr: 0x8010c1c4  name: FUN_8010c1c4

/* WARNING: Removing unreachable block (ram,0x8010c2c4) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8010c1c4(void)

{
  ushort uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined *puVar4;
  ushort *puVar5;
  uint uVar6;
  undefined4 *puVar7;
  int iVar8;
  int iVar9;
  uint uVar10;
  undefined1 auStack_78 [4];
  int local_74;
  int local_70;
  undefined1 auStack_60 [32];
  undefined4 local_40 [8];
  undefined2 local_20;
  undefined2 local_1e;
  undefined2 local_1c;
  undefined2 local_1a;
  
  AsyncList_RecycleHead/*0x80011834*/();
  uVar10 = 0;
  uVar2 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_Options_BIN_80100f98);
  uVar3 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_Options_ANM_80100fac);
  DAT_80113394 = FUN_8001a640/*0x8001a640*/(uVar2,uVar3);
  DAT_80113398 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_OptTable_tbl_80100fc0);
  FUN_800185cc/*0x800185cc*/(auStack_78,DAT_80113398 + *(int *)(DAT_80113398 + 4));
  iVar8 = 0;
  if (0 < *(short *)(local_74 + 4)) {
    do {
      puVar5 = (ushort *)(iVar8 * 2 + local_70);
      uVar1 = *puVar5;
      *puVar5 = (short)((uVar1 & 0x1f) * 0x1d + (uVar1 >> 5 & 0x1f) * 0x96 +
                        (uVar1 >> 10 & 0x1f) * 0x1d >> 8) * 0x421;
      iVar8 = iVar8 + 1;
    } while (iVar8 < *(short *)(local_74 + 4));
  }
LAB_8010c2fc:
  CD_PlayTrack/*0x80043ce0*/(0);
  Async_StopAllocCallback/*0x800165cc*/(0);
  SetDispMask/*0x8004f4e8*/(0);
  FUN_80019e7c/*0x80019e7c*/(1);
  func_0x8001d3d8(1);
  func_0x8001d404(0,&DAT_80100628,0xffffff);
  func_0x8001d404(1,&DAT_80100fd4,0x404080);
  FUN_80016da8/*0x80016da8*/(auStack_60);
  FUN_8004d314/*0x8004d314*/(auStack_60,&DAT_80100798);
  FUN_8001d9c0/*0x8001d9c0*/(auStack_60,0x200);
  FUN_8001d370/*0x8001d370*/();
  FUN_80019e20/*0x80019e20*/();
  DAT_8011339c = func_0x8001a1e8(0,0x1ba - (uint)(*(byte *)(DAT_80113388 +
                                                            *(int *)(DAT_80113388 + 8) + 6) >> 1),
                                 0xe0);
  DAT_801122ba = (*(byte *)(DAT_80113388 + *(int *)(DAT_80113388 + 8) + 6) + 2) * 8;
  iVar8 = 0;
  do {
    uVar2 = func_0x8001a1e8((int)DAT_801122b4,
                            (int)DAT_801122b6 +
                            iVar8 * (*(byte *)(DAT_80113388 + *(int *)(DAT_80113388 + 8) + 6) + 2),
                            (int)DAT_801122b8);
    iVar9 = iVar8 + 1;
    local_40[iVar8] = uVar2;
    iVar8 = iVar9;
  } while (iVar9 < 8);
  do {
    FUN_8001a994/*0x8001a994*/(DAT_80113394);
    local_20 = 0xe0;
    local_1e = 0x30;
    local_1c = 0x1a0;
    local_1a = 0x174;
    FUN_8001a0ac/*0x8001a0ac*/(&local_20,0);
    SetDispMask/*0x8004f4e8*/(1);
    if (uVar10 < 8) {
      puVar4 = (&switchD_8010c4e4::switchdataD_80100fe0)[uVar10];
      switch(uVar10) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
        puVar4 = (undefined *)0x80060000;
      }
      if ((int)puVar4 < 0) goto LAB_8010c2fc;
    }
    uVar6 = _DAT_80065930 | _DAT_80065934;
    FUN_80019e20/*0x80019e20*/();
    func_0x8001a2ac(local_40[uVar10],(int)DAT_801122b4,
                    (int)DAT_801122b6 +
                    uVar10 * (*(byte *)(DAT_80113388 + *(int *)(DAT_80113388 + 8) + 6) + 2));
    if (((uVar6 & 0x10000000) != 0) && (0 < (int)uVar10)) {
      uVar10 = uVar10 - 1;
    }
    if (((uVar6 & 0x40000000) != 0) && ((int)uVar10 < 7)) {
      uVar10 = uVar10 + 1;
    }
    if ((uVar6 & 0x900000) != 0) {
      iVar8 = 0;
      puVar7 = local_40;
      do {
        uVar2 = *puVar7;
        puVar7 = puVar7 + 1;
        iVar8 = iVar8 + 1;
        FUN_800183ec/*0x800183ec*/(uVar2);
      } while (iVar8 < 8);
      FUN_800183ec/*0x800183ec*/(DAT_8011339c);
      FUN_8001aa38/*0x8001aa38*/(DAT_80113394);
      Heap_Free/*0x80045088*/(DAT_80113398);
      return;
    }
  } while( true );
}

