// addr: 0x80102bdc  name: FUN_80102bdc

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

byte FUN_80102bdc(uint param_1)

{
  bool bVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  undefined1 *puVar6;
  undefined1 *puVar7;
  byte *pbVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  uint uVar12;
  int iVar13;
  int iVar14;
  int iVar15;
  undefined1 auStack_128 [32];
  undefined1 auStack_108 [3];
  undefined1 local_105 [9];
  undefined1 local_fc [8];
  undefined1 local_f4 [8];
  undefined1 local_ec [116];
  undefined1 auStack_78 [4];
  undefined4 local_74;
  undefined1 local_6d;
  undefined4 local_6c;
  byte local_68 [16];
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  int local_48;
  int *local_44;
  undefined4 local_40;
  int local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined1 *local_30;
  
  iVar15 = 0;
  iVar14 = 0;
  AsyncList_RecycleHead/*0x80011834*/();
  uVar3 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_Locatns_BIN_80100684);
  local_44 = (int *)func_0x8001a8fc(uVar3);
  uVar3 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_Map_bsd_80100698);
  iVar9 = 0;
  iVar5 = 0;
  bVar1 = true;
  do {
    if ((!bVar1) || ((param_1 & 1 << (0xeU - iVar9 & 0x1f)) == 0)) {
      local_68[iVar5] = (byte)iVar9;
      iVar5 = iVar5 + 1;
    }
    iVar9 = iVar9 + 1;
    bVar1 = iVar9 < 2;
  } while (iVar9 < 10);
  Async_StopAllocCallback/*0x800165cc*/(0);
  SetDispMask/*0x8004f4e8*/(0);
  FUN_80019e7c/*0x80019e7c*/(1);
  iVar10 = 0;
  FUN_80019e20/*0x80019e20*/(1);
  Heap_Free/*0x80045088*/(uVar3,0,0,0);
  iVar9 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4),1);
  *(undefined1 *)(iVar9 + 4) = 0x7c;
  *(undefined1 *)(iVar9 + 5) = 0x60;
  *(undefined1 *)(iVar9 + 6) = 0;
  FUN_80019a58/*0x80019a58*/(iVar9,s_SELECT_LOCATION_801006a8,&DAT_80111dd4,0x444a);
  FUN_800190a8/*0x800190a8*/(iVar9);
  local_34 = func_0x8001a24c(&DAT_80111dc4,&DAT_801006b8);
  local_40 = FUN_8001a2cc/*0x8001a2cc*/(&DAT_80111dcc,
                             ((int)((uint)DAT_80111dd0 << 0x10) >> 0x10) -
                             ((int)((uint)DAT_80111dd0 << 0x10) >> 0x1f) >> 1,
                             (((int)DAT_80111dd2 << 0x10) >> 0x10) -
                             (((int)DAT_80111dd2 << 0x10) >> 0x1f) >> 1,0);
  iVar9 = *local_44;
  if (0 < *(int *)(iVar9 + 0x10)) {
    do {
      iVar9 = *(int *)(iVar10 * 4 + *(int *)(iVar9 + 0x14));
      *(uint *)(iVar9 + 4) = *(uint *)(iVar9 + 4) | 0x10;
      iVar9 = *local_44;
      iVar10 = iVar10 + 1;
    } while (iVar10 < *(int *)(iVar9 + 0x10));
  }
  iVar11 = -3;
  FUN_8001a91c/*0x8001a91c*/(local_44);
  iVar9 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),1);
  FUN_80016da8/*0x80016da8*/(auStack_128);
  FUN_8004d314/*0x8004d314*/(auStack_128,&DAT_801006d0);
  func_0x8001d3d8();
  func_0x8001d404(1,&DAT_80100628,0xffffff);
  FUN_8001d9c0/*0x8001d9c0*/(auStack_128,0x200);
  iVar10 = FUN_8001d470/*0x8001d470*/(0x80);
  local_48 = iVar9;
  if (-3 < iVar5 + 3) {
    iVar13 = -0x9000;
    do {
      iVar4 = iVar11 + iVar5;
      iVar11 = iVar11 + 1;
      iVar4 = FUN_8001ac44/*0x8001ac44*/(local_44,*(undefined2 *)
                                        (&DAT_80111ba0 + (uint)local_68[iVar4 % iVar5] * 0x14),0x80,
                              0);
      *(undefined4 *)(iVar4 + 0x48) = 0;
      *(int *)(iVar4 + 0x4c) = iVar13;
      *(undefined4 *)(iVar4 + 0x50) = 0xffff8000;
      FUN_8001d708/*0x8001d708*/(iVar4);
      FUN_8001d4f0/*0x8001d4f0*/(iVar10,iVar4);
      iVar13 = iVar13 + 0x3000;
    } while (iVar11 < iVar5 + 3);
  }
  FUN_8001dc1c/*0x8001dc1c*/(iVar10);
  uVar12 = 0;
  puVar7 = auStack_108;
  iVar11 = 0;
  do {
    local_105[iVar11] = 8;
    local_105[iVar11 + 4] = 0x3a;
    local_105[iVar11 + 1] = 0xff;
    local_105[iVar11 + 2] = 0xff;
    local_105[iVar11 + 3] = 0xff;
    local_fc[iVar11] = 0xff;
    local_fc[iVar11 + 1] = 0xff;
    local_fc[iVar11 + 2] = 0xff;
    local_f4[iVar11] = 0;
    local_f4[iVar11 + 1] = 0;
    local_f4[iVar11 + 2] = 0;
    local_ec[iVar11] = 0;
    local_ec[iVar11 + 1] = 0;
    local_ec[iVar11 + 2] = 0;
    *(undefined2 *)(puVar7 + 0x18) = 0;
    *(undefined2 *)(puVar7 + 8) = 0;
    *(undefined2 *)(puVar7 + 0x20) = 0xf0;
    *(undefined2 *)(puVar7 + 0x10) = 0xf0;
    puVar6 = puVar7;
    if ((uVar12 & 2) == 0) {
      puVar6 = (undefined1 *)0x1;
      _DAT_00000013 = 0x140;
    }
    *(undefined2 *)(puVar6 + 0x12) = 0;
    *(undefined2 *)(puVar6 + 10) = *(undefined2 *)(puVar6 + 0x12);
    puVar7 = puVar6;
    if ((uVar12 & 2) != 0) {
      puVar7 = (undefined1 *)0x1;
    }
    *(undefined2 *)(puVar7 + 0x22) = 0x120;
    puVar7 = puVar7 + 0x24;
    uVar12 = uVar12 + 1;
    *(undefined2 *)(puVar6 + 0x1a) = *(undefined2 *)(puVar6 + 0x22);
    iVar11 = iVar11 + 0x24;
  } while ((int)uVar12 < 4);
  auStack_78[3] = 1;
  local_74 = 0xe1000640;
  local_6d = 1;
  local_6c = 0xe1000640;
  local_3c = FUN_8001ac44/*0x8001ac44*/(local_44,10,0x80,0);
  uVar2 = DAT_801006e8;
  uVar3 = DAT_801006e4;
  *(undefined4 *)(local_3c + 0x48) = DAT_801006e0;
  *(undefined4 *)(local_3c + 0x4c) = uVar3;
  *(undefined4 *)(local_3c + 0x50) = uVar2;
  FUN_8001d708/*0x8001d708*/();
  pbVar8 = &DAT_80111dbc;
  FUN_8001dc1c/*0x8001dc1c*/(local_3c);
  iVar11 = FUN_8001ac44/*0x8001ac44*/(local_44,0xb,0x80,0);
  V8_MemSet/*0x80044efc*/(iVar11 + 0x48,0,0xc);
  FUN_8001d708/*0x8001d708*/(iVar11);
  FUN_8001dc1c/*0x8001dc1c*/(iVar11);
  *(undefined1 *)(iVar9 + 4) = 0;
  *(undefined1 *)(iVar9 + 5) = 0x80;
  *(undefined1 *)(iVar9 + 6) = 0;
  FUN_80019a58/*0x80019a58*/(iVar9,&DAT_801006ec,&DAT_80111dbc,2);
  FUN_80019a58/*0x80019a58*/(iVar9,&DAT_801006f0,&DAT_80111dbc,6);
  local_30 = auStack_108;
  *(undefined1 *)(iVar9 + 4) = 0x80;
  *(undefined1 *)(iVar9 + 5) = 0x80;
  *(undefined1 *)(iVar9 + 6) = 0x80;
  do {
    Buffer_StartOTagOther/*0x80011a10*/();
    if (iVar14 == 1) {
LAB_8010322c:
      iVar13 = iVar15 * -0x3000 - *(int *)(iVar10 + 0x4c);
      if (iVar13 < 0) {
        iVar13 = iVar13 + 0xf;
      }
      *(int *)(iVar10 + 0x4c) = *(int *)(iVar10 + 0x4c) + (iVar13 >> 4);
      if (iVar13 >> 4 == 0) {
        iVar14 = iVar14 + 1;
      }
      FUN_8001d708/*0x8001d708*/(iVar10);
      FUN_8001d370/*0x8001d370*/();
      FUN_8001a4f8/*0x8001a4f8*/(local_40,0);
      FUN_8001de08/*0x8001de08*/(local_3c);
      FUN_8001de08/*0x8001de08*/(iVar10);
      AddPrim/*0x80052274*/(_DAT_80065910,local_30 + _DAT_80065308 * 0x24);
      AddPrim/*0x80052274*/(_DAT_80065910,local_30 + _DAT_80065308 * 0x24 + 0x48);
      AddPrim/*0x80052274*/(_DAT_80065910,auStack_78 + _DAT_80065308 * 8);
      DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
      VSync/*0x80047e44*/(0);
      DrawSync/*0x8004f580*/(0);
      func_0x8001a584(local_40);
      Buffer_StartOTag/*0x800119c0*/(_DAT_80065308);
    }
    else if (((iVar14 < 2) && (pbVar8 = local_68, iVar14 == 0)) || (iVar14 != 2)) {
      pbVar8 = pbVar8 + iVar15;
      local_50 = CONCAT22(local_50._2_2_,*(short *)(&DAT_80111bac + (uint)*pbVar8 * 0x14) + -0x10);
      local_4c = 0x200020;
      local_50 = CONCAT22(*(short *)(&DAT_80111bae + (uint)*pbVar8 * 0x14) + -0x10,
                          *(short *)(&DAT_80111bac + (uint)*pbVar8 * 0x14) + -0x10);
      local_58 = local_50;
      local_54 = 0x200020;
      local_38 = FUN_8001a2cc/*0x8001a2cc*/(&local_58,0x10,0x10,0xffffffff);
      FUN_80019e20/*0x80019e20*/();
      iVar14 = 1;
      func_0x8001a2ac(local_34,(int)DAT_80111dc4,(int)DAT_80111dc6);
      FUN_80019a58/*0x80019a58*/(iVar9,(&PTR_s_Secret_Base_80111ba4)[(uint)*pbVar8 * 5],&DAT_80111dc4,2);
      FUN_80019a58/*0x80019a58*/(local_48,(&PTR_s_Nevada_80111ba8)[(uint)*pbVar8 * 5],&DAT_80111dc4,6);
      *(undefined2 *)(iVar11 + 0x42) = 0;
      goto LAB_8010322c;
    }
    *(short *)(iVar11 + 0x42) = *(short *)(iVar11 + 0x42) + 0x40;
    FUN_8001d708/*0x8001d708*/(iVar11);
    FUN_8001a4f8/*0x8001a4f8*/(local_38,0);
    FUN_8001de08/*0x8001de08*/(iVar11);
    DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
    if (iVar14 == 2) {
      VSync/*0x80047e44*/(0);
    }
    DrawSync/*0x8004f580*/(0);
    func_0x8001a584(local_38);
    Pad_Tick/*0x800120d4*/();
    SetDispMask/*0x8004f4e8*/(1);
    pbVar8 = (byte *)(_DAT_80065930 | _DAT_80065934);
    uVar3 = 0;
    if (((uint)pbVar8 & 0x58500000) != 0) {
      if (((uint)pbVar8 & 0x50000000) != 0) {
        uVar3 = 9;
      }
      func_0x8004454c(1,DAT_8011338c,uVar3);
      iVar14 = 0;
      FUN_8001a4f8/*0x8001a4f8*/(local_38,0);
      func_0x8001a584(local_38);
      func_0x8001a4ac(local_38);
      if ((((uint)pbVar8 & 0x40000000) != 0) && (iVar15 = iVar15 + 1, iVar15 == iVar5)) {
        iVar15 = 0;
        *(int *)(iVar10 + 0x4c) = *(int *)(iVar10 + 0x4c) + iVar5 * 0x3000;
      }
      if ((((uint)pbVar8 & 0x10000000) != 0) && (iVar15 = iVar15 + -1, iVar15 < 0)) {
        iVar15 = iVar5 + -1;
        *(int *)(iVar10 + 0x4c) = *(int *)(iVar10 + 0x4c) + iVar5 * -0x3000;
      }
    }
    SetDispMask/*0x8004f4e8*/(1);
    if (((uint)pbVar8 & 0x8500000) != 0) {
      uVar3 = 0;
      if (((uint)pbVar8 & 0x8400000) != 0) {
        uVar3 = 7;
      }
      FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,uVar3);
      FUN_800183ec/*0x800183ec*/(local_34);
      func_0x8001a4ac(local_40);
      FUN_8001af48/*0x8001af48*/(iVar10);
      FUN_8001af48/*0x8001af48*/(local_3c);
      FUN_8001af48/*0x8001af48*/(iVar11);
      FUN_8001aa38/*0x8001aa38*/(local_44);
      FUN_800190a8/*0x800190a8*/(iVar9);
      Buffer_StartOTag/*0x800119c0*/(_DAT_80065308);
      return local_68[iVar15];
    }
  } while( true );
}

