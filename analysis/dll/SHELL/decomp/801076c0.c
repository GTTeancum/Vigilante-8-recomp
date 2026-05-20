// addr: 0x801076c0  name: FUN_801076c0

/* WARNING: Removing unreachable block (ram,0x801081e4) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801076c0(int param_1)

{
  int iVar1;
  uint uVar2;
  undefined *puVar3;
  byte *pbVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  undefined **ppuVar8;
  short sVar9;
  short *psVar10;
  undefined2 *puVar11;
  undefined1 *puVar12;
  undefined1 *puVar13;
  uint uVar14;
  int iVar15;
  int unaff_s8;
  undefined1 auStack_378 [16];
  undefined1 auStack_368 [16];
  undefined1 auStack_358 [16];
  undefined1 auStack_348 [16];
  undefined1 auStack_338 [10];
  undefined2 local_32e [87];
  undefined1 auStack_280 [32];
  undefined1 auStack_260 [32];
  undefined1 auStack_240 [32];
  undefined1 auStack_220 [32];
  undefined1 auStack_200 [364];
  undefined1 auStack_94 [22];
  undefined2 local_7e;
  undefined1 auStack_78 [32];
  short local_58;
  short local_56;
  undefined2 local_54;
  undefined2 local_52;
  byte local_50 [17];
  undefined1 local_3f;
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  
  uVar14 = 0;
  iVar15 = 0;
  iVar5 = 0;
  puVar12 = auStack_338;
  puVar13 = auStack_200;
  iVar1 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4),1);
  *(undefined1 *)(iVar1 + 4) = 0x32;
  *(undefined1 *)(iVar1 + 5) = 0x73;
  *(undefined1 *)(iVar1 + 6) = 100;
  FUN_80019a58/*0x80019a58*/(iVar1,s_GAME_STATUS_80100988,&DAT_80100994,2);
  FUN_800190a8/*0x800190a8*/(iVar1);
  psVar10 = &DAT_80111f04;
  do {
    func_0x80018d00(puVar13,DAT_80113398 + *(int *)(DAT_80113398 + iVar5 * 4 + 4),puVar12,
                    *psVar10 + 4,psVar10[1] + 4);
    if (1 < iVar5) {
      FUN_8001859c/*0x8001859c*/(*(undefined2 *)(puVar12 + 10));
      *(undefined2 *)(puVar12 + 10) = 0;
    }
    psVar10 = psVar10 + 2;
    puVar12 = puVar12 + 0xc;
    iVar5 = iVar5 + 1;
    puVar13 = puVar13 + 0x1c;
  } while (iVar5 < 0xf);
  func_0x80018d00(auStack_280,DAT_80113398 + *(int *)(DAT_80113398 + 0x40),auStack_378,0,0);
  func_0x80018c14(auStack_280,0x22);
  func_0x80018d00(auStack_260,DAT_80113398 + *(int *)(DAT_80113398 + 0x44),auStack_368,0,0);
  func_0x80018c14(auStack_260,0x22);
  func_0x80018d00(auStack_220,DAT_80113398 + *(int *)(DAT_80113398 + 0x48),auStack_348,0,0);
  func_0x80018d00(auStack_240,DAT_80113398 + *(int *)(DAT_80113398 + 0x4c),auStack_358,0,0);
  FUN_8001a0ac/*0x8001a0ac*/(&DAT_801009ac,0xc0f9,0x206,0x13d);
  FUN_8001a0ac/*0x8001a0ac*/(&DAT_801009b4,0xc0f9);
  FUN_8001a0ac/*0x8001a0ac*/(&DAT_801009bc,0xc0f9);
  FUN_8001a0ac/*0x8001a0ac*/(&DAT_801009c4,0xc0f9);
  if (((uint)DAT_801133b4 & 0x1000) == 0) {
    FUN_8001a0ac/*0x8001a0ac*/(&DAT_801009cc,0xc0f9);
    FUN_8001a0ac/*0x8001a0ac*/(&DAT_801009d4,0xc0f9);
  }
  uVar6 = 0;
  puVar12 = auStack_200;
  uVar2 = 0x80110000;
  puVar11 = &DAT_80111f04;
  do {
    puVar13 = auStack_240;
    if (uVar2 == 3) {
      puVar13 = auStack_220;
    }
    *(undefined2 *)(puVar13 + 0x10) = *puVar11;
    *(undefined2 *)(puVar13 + 0x12) = puVar11[1];
    iVar1 = 0;
    if (((uint)DAT_801133b4 & 1 << (uVar6 & 0x1f)) == 0) {
      if ((uVar6 == 0xc) || (iVar5 = 0, 1 < (int)uVar2)) {
        iVar1 = 1;
        iVar5 = 2;
      }
      *(undefined2 *)(puVar12 + 0x16) = local_32e[(iVar5 + iVar1) * 2];
      DrawPrim/*0x8004fb18*/(puVar12);
    }
    if ((uVar6 != 0xc) || (((uint)DAT_801133b4 & 0x1000) == 0)) {
      DrawPrim/*0x8004fb18*/(puVar13);
    }
    puVar12 = puVar12 + 0x1c;
    uVar6 = uVar6 + 1;
    uVar2 = (uint)((int)uVar6 < 0xd);
    puVar11 = puVar11 + 2;
  } while (uVar2 != 0);
  if (((uint)DAT_801133b4 & 0x2000) == 0) {
    local_7e = local_32e[6];
    DrawPrim/*0x8004fb18*/(auStack_94);
  }
  if (((uint)DAT_801133b4 & 0x4000) == 0) {
    local_7e = local_32e[6];
    DrawPrim/*0x8004fb18*/(auStack_78);
  }
LAB_80107ad0:
  iVar1 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),1);
  local_34 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 0xc),1);
  iVar5 = 1;
  if (param_1 == 2) {
    iVar7 = 0;
    sVar9 = 0xec;
    local_56 = 0x168;
    local_54 = 0x14;
    local_52 = 0x40;
    do {
      local_50[0x10] = local_50[iVar7] + 0x41;
      if (0x19 < local_50[iVar7]) {
        local_50[0x10] = '_';
      }
      local_3f = 0;
      *(uint *)(iVar1 + 4) = *(uint *)(iVar1 + 4) & 0xff000000 | 0x404040;
      local_58 = sVar9;
      FUN_80019a58/*0x80019a58*/(iVar1,local_50 + 0x10,&local_58,10);
      iVar7 = iVar7 + 1;
      sVar9 = sVar9 + 0x1c;
    } while (iVar7 < 0xe);
    local_58 = (short)iVar15 * 0x1c + 0xec;
    *(undefined1 *)(iVar1 + 4) = 0;
    *(undefined1 *)(iVar1 + 5) = 0x80;
    *(undefined1 *)(iVar1 + 6) = 0;
    FUN_80019a58/*0x80019a58*/(iVar1,&DAT_801006ec,&local_58);
  }
  if (param_1 != iVar5) goto LAB_80107d7c;
  puVar12 = auStack_260;
  if (0xc < (int)uVar14) {
    puVar12 = auStack_280;
  }
  local_58 = (&DAT_80111f04)[uVar14 * 2] + -0xc;
  *(short *)(puVar12 + 0x10) = (&DAT_80111f04)[uVar14 * 2] + -0xc;
  local_56 = (&DAT_80111f06)[uVar14 * 2] + -0xc;
  *(short *)(puVar12 + 0x12) = (&DAT_80111f06)[uVar14 * 2] + -0xc;
  local_54 = *(undefined2 *)(puVar12 + 0x18);
  local_52 = *(undefined2 *)(puVar12 + 0x1a);
  local_38 = func_0x8001a24c(&local_58);
  DrawPrim/*0x8004fb18*/(puVar12);
  *(undefined1 *)(iVar1 + 4) = 0x80;
  *(undefined1 *)(iVar1 + 5) = 0x80;
  *(undefined1 *)(iVar1 + 6) = 0x80;
  puVar3 = (undefined *)0x80110000;
  if (((uint)DAT_801133b4 & 1 << (uVar14 & 0x1f)) == 0) {
    puVar3 = &DAT_80111ba0 + (0xe - uVar14) * 0x14;
  }
  FUN_80019a58/*0x80019a58*/(iVar1,puVar3 + 0x1b94,&DAT_80111f94,2);
  if (((uint)DAT_801133b4 & 1 << (uVar14 & 0x1f)) != 0) goto LAB_80107d7c;
  ppuVar8 = &PTR_s_Unlocked_80111f7c;
  if ((int)uVar14 < 0xc == 0) goto LAB_80107d54;
  uVar2 = (uint)((int)uVar14 < 0xc) << 2;
LAB_80107d4c:
  uVar14 = uVar2 - uVar14;
LAB_80107d54:
  FUN_80019a58/*0x80019a58*/(local_34,*ppuVar8,&DAT_80111f94,6);
LAB_80107d7c:
  VSync/*0x80047e44*/(0);
  Tick_PadOnly/*0x800126f0*/();
  puVar12 = (undefined1 *)(_DAT_80065930 | _DAT_80065934);
  if (param_1 == 2) {
    if (((uint)puVar12 & 0xf000) == 0) {
LAB_80107ddc:
      unaff_s8 = 0;
    }
    else if (unaff_s8 != 0) {
      unaff_s8 = unaff_s8 + -1;
      if (unaff_s8 == 0) {
        puVar12 = (undefined1 *)((uint)puVar12 | ((uint)puVar12 & 0xf000) << 0x10);
        goto LAB_80107ddc;
      }
    }
    if (((uint)puVar12 & 0xf8500000) != 0) {
      unaff_s8 = 0x1e;
      goto LAB_80107df8;
    }
    goto LAB_80107e7c;
  }
LAB_80107df8:
  uVar2 = 0x58500000;
  uVar6 = uVar14;
  if (param_1 != 1) goto LAB_80107e6c;
  if (((uint)puVar12 & 0x8700000) != 0) goto LAB_80107e88;
  if (((uint)puVar12 & 0xf0000000) != 0) goto code_r0x80107e1c;
LAB_80107e7c:
  uVar6 = uVar14;
  uVar14 = uVar6;
  if (((uint)puVar12 & 0x800000) == 0) goto LAB_80107d7c;
  goto LAB_80107e88;
code_r0x80107e1c:
  uVar6 = (uint)*(char *)(uVar14 + 0x87911f31);
  uVar2 = 0xc;
  if ((int)uVar6 < 0) goto LAB_80107e7c;
  if (uVar6 == 0xc) {
    uVar2 = 0x1b;
    ppuVar8 = (undefined **)0xc;
    if (((uint)DAT_801133b4 & 0x1000) == 0) goto LAB_80107e6c;
    goto LAB_80107d4c;
  }
LAB_80107e6c:
  uVar14 = uVar6;
  if (((uint)puVar12 & uVar2) == 0) goto LAB_80107e7c;
LAB_80107e88:
  FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,(uint)(((uint)puVar12 & 0x8d00000) != 0) << 3);
  FUN_800190a8/*0x800190a8*/(iVar1);
  FUN_800190a8/*0x800190a8*/(local_34);
  iVar1 = 1;
  uVar14 = uVar6;
  if (param_1 != 2) goto LAB_801080f4;
  FUN_8001a0ac/*0x8001a0ac*/(&DAT_80111f8c,0);
  if (((uint)puVar12 & 0x8400000) == 0) goto LAB_80107fb8;
  DAT_801133b4 = (byte *)0x80060000;
  _DAT_80065908 = _DAT_80065908 | 1;
  if (((uint)puVar12 & 0x8400000) == 1) {
    DAT_801133b4 = (byte *)0x1680;
    _DAT_80065334 = 0x1680;
  }
  pbVar4 = (byte *)FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,0x10);
  puVar12 = auStack_338;
  if (pbVar4 != (byte *)0x0) goto LAB_80107fa4;
  local_30 = local_30 + -1;
  FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,0xb);
  iVar1 = local_30;
  if (local_30 == 0) {
    iVar1 = 0xd;
    pbVar4 = local_50 + 0xd;
    do {
      *pbVar4 = 0x1a;
      iVar1 = iVar1 + -1;
      pbVar4 = pbVar4 + -1;
    } while (-1 < iVar1);
    local_30 = 3;
LAB_80107fa4:
    iVar1 = -0x7fef0000;
    DAT_801133b4 = pbVar4;
  }
LAB_80107fb8:
  if ((int)puVar12 < 0) {
    iVar1 = (int)((ulonglong)((longlong)(iVar15 + 0xd) * -0x6db6db6d) >> 0x20);
    iVar15 = (iVar15 + 0xd) % 0xe;
  }
  if (((uint)puVar12 & 0x20000000) != 0) {
    iVar1 = (int)((ulonglong)((longlong)(iVar15 + 1) * -0x6db6db6d) >> 0x20);
    iVar15 = (iVar15 + 1) % 0xe;
  }
  if (((uint)puVar12 & 0x10000000) != 0) {
    uVar2 = local_50[iVar15] + 0x1a;
    iVar1 = (int)((ulonglong)((longlong)(int)uVar2 * 0x4bda12f7) >> 0x20);
    local_50[iVar15] = (char)uVar2 + (char)(uVar2 / 0x1b) * -0x1b;
  }
  if (((uint)puVar12 & 0x40000000) != 0) {
    uVar2 = local_50[iVar15] + 1;
    iVar1 = (int)((ulonglong)((longlong)(int)uVar2 * 0x4bda12f7) >> 0x20);
    local_50[iVar15] = (char)uVar2 + (char)(uVar2 / 0x1b) * -0x1b;
  }
  if (((uint)puVar12 & 0x100000) != 0) {
    param_1 = 1;
LAB_801080f4:
    uVar2 = 0x50900000;
    if (param_1 == iVar1) {
      FUN_8001a0ac/*0x8001a0ac*/(&DAT_80111f8c,0);
      func_0x8001a2ac(local_38,(int)local_58,(int)local_56);
      FUN_800183ec/*0x800183ec*/(local_38);
      if (((uint)puVar12 & 0x8400000) != 0) {
        uVar2 = 0;
        if ((_DAT_80065908 & 0x10) != 0) {
          puVar12 = (undefined1 *)0x0;
          do {
            uVar2 = uVar2 + 1;
            if (((_DAT_80065930 | _DAT_80065934) & 0x8000000) != 0) {
              puVar12 = (undefined1 *)0x0;
              break;
            }
          } while (uVar2 < 0xc);
        }
        FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,0xb);
      }
      if (((uint)puVar12 & 0x200000) != 0) {
        param_1 = 2;
        local_30 = 3;
        iVar1 = 0xd;
        pbVar4 = local_50 + 0xd;
        do {
          *pbVar4 = 0x1a;
          iVar1 = iVar1 + -1;
          pbVar4 = pbVar4 + -1;
        } while (-1 < iVar1);
      }
      if (((uint)puVar12 & 0x100000) != 0) {
        param_1 = 0;
      }
      if (((uint)puVar12 & 0x800000) != 0) {
LAB_801082d4:
        iVar1 = 0;
        puVar12 = auStack_338;
        do {
          FUN_8001884c/*0x8001884c*/(puVar12);
          iVar1 = iVar1 + 1;
          puVar12 = puVar12 + 0xc;
        } while (iVar1 < 0xf);
        FUN_8001884c/*0x8001884c*/(auStack_378);
        FUN_8001884c/*0x8001884c*/(auStack_368);
        FUN_8001884c/*0x8001884c*/(auStack_348);
        FUN_8001884c/*0x8001884c*/(auStack_358);
        return 0;
      }
      uVar2 = 0;
    }
    if ((((uint)puVar12 & uVar2) != 0) || (((uint)puVar12 & 0x8400000) != 0)) goto LAB_801082d4;
  }
  goto LAB_80107ad0;
}

