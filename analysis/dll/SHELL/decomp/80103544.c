// addr: 0x80103544  name: FUN_80103544

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined1 FUN_80103544(int param_1)

{
  byte bVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  undefined1 uVar5;
  byte *pbVar6;
  uint uVar7;
  int iVar8;
  int *piVar9;
  int iVar10;
  int iVar11;
  uint uVar12;
  int iVar13;
  undefined1 auStack_c8 [32];
  int local_a8 [8];
  undefined1 auStack_88 [3];
  undefined1 local_85;
  undefined1 local_84;
  undefined1 local_83;
  undefined1 local_82;
  undefined1 local_81;
  short local_80;
  short local_7e;
  short local_7c;
  short local_7a;
  short local_78;
  short local_76;
  short local_74;
  short local_72;
  undefined4 local_70;
  int local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_58;
  undefined4 local_50;
  undefined4 local_48;
  undefined4 local_44;
  int *local_40;
  undefined4 local_3c;
  int local_38;
  undefined4 local_34;
  undefined4 local_30;
  
  pbVar6 = &DAT_80065950 + param_1;
  iVar13 = 0;
  if (*pbVar6 == 0) {
    *pbVar6 = 1;
  }
  uVar4 = *(uint *)(_DAT_8006590c + param_1 * 8 + 4);
  uVar7 = (uint)*pbVar6;
  iVar2 = uVar4 - 1;
  if ((int)uVar7 < 1 << (uVar4 & 0x1f)) {
    uVar7 = 1;
  }
  uVar12 = 0;
  uVar4 = uVar7;
  if (0 < iVar2) {
    do {
      if ((uVar4 & 1) == 0) break;
      uVar12 = uVar12 + 1;
      uVar4 = (int)uVar7 >> (uVar12 & 0x1f);
    } while ((int)uVar12 < iVar2);
  }
  AsyncList_RecycleHead/*0x80011834*/();
  uVar3 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_Locatns_BIN_80100684);
  local_40 = (int *)func_0x8001a8fc(uVar3);
  uVar3 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_Map_bsd_80100698);
  Async_StopAllocCallback/*0x800165cc*/(0);
  SetDispMask/*0x8004f4e8*/(0);
  FUN_80019e7c/*0x80019e7c*/(0);
  FUN_80019e20/*0x80019e20*/(1);
  Heap_Free/*0x80045088*/(uVar3,0,0,0);
  iVar2 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4),1);
  *(undefined1 *)(iVar2 + 4) = 0x7c;
  *(undefined1 *)(iVar2 + 5) = 0x60;
  *(undefined1 *)(iVar2 + 6) = 0;
  FUN_80019a58/*0x80019a58*/(iVar2,s_QUEST_ROUTE_801006f4,&DAT_80111dfc,0x444a);
  FUN_800190a8/*0x800190a8*/(iVar2);
  local_3c = FUN_8001a2cc/*0x8001a2cc*/(&DAT_80111df4,
                             ((int)((uint)DAT_80111df8 << 0x10) >> 0x10) -
                             ((int)((uint)DAT_80111df8 << 0x10) >> 0x1f) >> 1,
                             (((int)DAT_80111dfa << 0x10) >> 0x10) -
                             (((int)DAT_80111dfa << 0x10) >> 0x1f) >> 1,0);
  local_30 = func_0x8001a24c(&DAT_80111de4);
  local_48 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),0);
  local_44 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 0xc),0);
  iVar2 = *local_40;
  iVar10 = 0;
  if (0 < *(int *)(iVar2 + 0x10)) {
    do {
      iVar2 = *(int *)(iVar10 * 4 + *(int *)(iVar2 + 0x14));
      *(uint *)(iVar2 + 4) = *(uint *)(iVar2 + 4) | 0x10;
      iVar2 = *local_40;
      iVar10 = iVar10 + 1;
    } while (iVar10 < *(int *)(iVar2 + 0x10));
  }
  FUN_80016da8/*0x80016da8*/(auStack_c8);
  FUN_8004d314/*0x8004d314*/(auStack_c8,&DAT_801006d0);
  iVar2 = 0;
  func_0x8001d3d8();
  func_0x8001d404(1,&DAT_80100724,0xffffff);
  FUN_8001d9c0/*0x8001d9c0*/(auStack_c8,0x200);
  local_85 = 5;
  local_81 = 0x28;
  if (0 < *(int *)(_DAT_8006590c + param_1 * 8 + 4)) {
    do {
      iVar8 = param_1 * 8;
      iVar11 = iVar2 * 0x10;
      iVar10 = FUN_8001ac44/*0x8001ac44*/(local_40,*(undefined2 *)
                                         (&DAT_80111ba0 +
                                         (uint)*(byte *)(iVar11 + *(int *)(_DAT_8006590c + iVar8 + 8
                                                                          )) * 0x14),0x80,0);
      local_a8[iVar2] = iVar10;
      local_70 = 0;
      local_68 = 0xffff8000;
      local_6c = ((iVar2 * 2 - *(int *)(_DAT_8006590c + iVar8 + 4)) + 1) * 0x1800;
      *(undefined4 *)(iVar10 + 0x48) = 0;
      *(int *)(iVar10 + 0x4c) = local_6c;
      *(undefined4 *)(iVar10 + 0x50) = 0xffff8000;
      FUN_8001d708/*0x8001d708*/(local_a8[iVar2]);
      if (iVar2 != 0) {
        local_60 = CONCAT22(*(short *)(&DAT_80111bae +
                                      (uint)*(byte *)(iVar11 + *(int *)(_DAT_8006590c + iVar8 + 8) +
                                                     -0x10) * 0x14),
                            *(short *)(&DAT_80111bac +
                                      (uint)*(byte *)(iVar11 + *(int *)(_DAT_8006590c + iVar8 + 8) +
                                                     -0x10) * 0x14));
        local_70 = local_60;
        local_58 = CONCAT22(*(short *)(&DAT_80111bae +
                                      (uint)*(byte *)(iVar11 + *(int *)(_DAT_8006590c + iVar8 + 8))
                                      * 0x14),
                            *(short *)(&DAT_80111bac +
                                      (uint)*(byte *)(iVar11 + *(int *)(_DAT_8006590c + iVar8 + 8))
                                      * 0x14));
        local_60 = local_58;
        local_50 = CONCAT22(*(short *)(&DAT_80111bae +
                                      (uint)*(byte *)(iVar11 + *(int *)(_DAT_8006590c + iVar8 + 8))
                                      * 0x14) -
                            *(short *)(&DAT_80111bae +
                                      (uint)*(byte *)(iVar11 + *(int *)(_DAT_8006590c + iVar8 + 8) +
                                                     -0x10) * 0x14),
                            *(short *)(&DAT_80111bac +
                                      (uint)*(byte *)(iVar11 + *(int *)(_DAT_8006590c + iVar8 + 8))
                                      * 0x14) -
                            *(short *)(&DAT_80111bac +
                                      (uint)*(byte *)(iVar11 + *(int *)(_DAT_8006590c + iVar8 + 8) +
                                                     -0x10) * 0x14));
        local_58 = local_50;
        func_0x800168b0(&local_58,&local_58);
        iVar10 = (int)local_58._2_2_;
        if (iVar10 < 0) {
          iVar10 = iVar10 + 0x3ff;
        }
        iVar8 = (int)(short)local_58;
        local_74 = (short)(iVar10 >> 10);
        local_80 = (short)local_70 + local_74;
        if (iVar8 < 0) {
          iVar8 = iVar8 + 0x3ff;
        }
        local_72 = (short)(iVar8 >> 10);
        local_7e = local_70._2_2_ - local_72;
        local_7c = (short)local_70 - local_74;
        local_7a = local_70._2_2_ + local_72;
        local_78 = (short)local_60 + local_74;
        local_76 = local_60._2_2_ - local_72;
        local_74 = (short)local_60 - local_74;
        local_72 = local_60._2_2_ + local_72;
        local_84 = 0xff;
        local_83 = 0;
        local_82 = 0;
        DrawPrim/*0x8004fb18*/(auStack_88);
      }
      iVar2 = iVar2 + 1;
    } while (iVar2 < *(int *)(_DAT_8006590c + param_1 * 8 + 4));
  }
  local_38 = FUN_8001ac44/*0x8001ac44*/(local_40,10,0x80,0);
  local_70 = 0;
  iVar2 = param_1 * 8;
  local_68 = 0xffff8000;
  iVar10 = ((uVar12 * 2 - *(int *)(_DAT_8006590c + iVar2 + 4)) + 1) * 0x1800;
  *(undefined4 *)(local_38 + 0x48) = 0;
  *(int *)(local_38 + 0x4c) = iVar10;
  *(undefined4 *)(local_38 + 0x50) = 0xffff8000;
  local_6c = iVar10;
  FUN_8001dc1c/*0x8001dc1c*/();
  iVar8 = FUN_8001ac44/*0x8001ac44*/(local_40,0xb,0x80,0);
  V8_MemSet/*0x80044efc*/(iVar8 + 0x48,0,0xc);
  FUN_8001d708/*0x8001d708*/(iVar8);
  FUN_8001dc1c/*0x8001dc1c*/(iVar8);
  pbVar6 = &DAT_80065950 + param_1;
  do {
    Buffer_StartOTagOther/*0x80011a10*/();
    if (iVar13 == 1) {
LAB_80103df4:
      iVar10 = ((uVar12 * 2 - *(int *)(_DAT_8006590c + iVar2 + 4)) + 1) * 0x1800 -
               *(int *)(local_38 + 0x4c);
      if (iVar10 < 0) {
        iVar10 = iVar10 + 0xf;
      }
      *(int *)(local_38 + 0x4c) = *(int *)(local_38 + 0x4c) + (iVar10 >> 4);
      if (iVar10 >> 4 == 0) {
        iVar13 = iVar13 + 1;
      }
      uVar4 = 0;
      FUN_8001d708/*0x8001d708*/(local_38);
      FUN_8001d370/*0x8001d370*/();
      FUN_8001a4f8/*0x8001a4f8*/(local_3c,0);
      FUN_8001de08/*0x8001de08*/(local_38);
      if (0 < *(int *)(_DAT_8006590c + iVar2 + 4)) {
        do {
          uVar5 = 0;
          if (1 << (uVar4 & 0x1f) <= (int)(uint)*pbVar6) {
            uVar5 = 0x80;
          }
          SetBackColor/*0x8004d4e4*/(uVar5,uVar5,uVar5);
          FUN_8001dcc8/*0x8001dcc8*/(local_a8[uVar4],0x8006f680);
          uVar4 = uVar4 + 1;
        } while ((int)uVar4 < *(int *)(_DAT_8006590c + iVar2 + 4));
      }
      DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
      DrawSync/*0x8004f580*/(0);
      func_0x8001a584(local_3c);
      Buffer_StartOTag/*0x800119c0*/(_DAT_80065308);
    }
    else if (((iVar13 < 2) && (iVar10 = -0x7fef0000, iVar13 == 0)) || (iVar13 != 2)) {
      iVar11 = uVar12 * 0x10;
      local_64 = 0x200020;
      local_68 = CONCAT22(*(short *)(&DAT_80111bae +
                                    (uint)*(byte *)(iVar11 + *(int *)(_DAT_8006590c + iVar2 + 8)) *
                                    0x14) + -0x10,
                          *(short *)((uint)*(byte *)(iVar11 + *(int *)(_DAT_8006590c + iVar2 + 8)) *
                                     0x14 + iVar10 + 0x1bac) + -0x10);
      local_70 = local_68;
      local_6c = 0x200020;
      local_34 = FUN_8001a2cc/*0x8001a2cc*/(&local_70,0x10,0x10,0xffffffff);
      FUN_80019e20/*0x80019e20*/();
      func_0x8001a2ac(local_30,(int)DAT_80111de4,(int)DAT_80111de6);
      FUN_80019a58/*0x80019a58*/(local_48,(&PTR_s_Secret_Base_80111ba4)
                               [(uint)*(byte *)(iVar11 + *(int *)(_DAT_8006590c + iVar2 + 8)) * 5],
                      &DAT_80111de4,2);
      FUN_80019a58/*0x80019a58*/(local_44,(&PTR_s_Nevada_80111ba8)
                               [(uint)*(byte *)(iVar11 + *(int *)(_DAT_8006590c + iVar2 + 8)) * 5],
                      &DAT_80111de4,6);
      FUN_8001a0ac/*0x8001a0ac*/(&DAT_80111dec,0);
      FUN_80019a58/*0x80019a58*/(local_48,s_Classified_80111b94,&DAT_80111dec,2);
      iVar13 = iVar13 + 1;
      *(undefined2 *)(iVar8 + 0x42) = 0;
      goto LAB_80103df4;
    }
    *(short *)(iVar8 + 0x42) = *(short *)(iVar8 + 0x42) + 0x40;
    FUN_8001d708/*0x8001d708*/(iVar8);
    FUN_8001a4f8/*0x8001a4f8*/(local_34,0);
    FUN_8001de08/*0x8001de08*/(iVar8);
    DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
    DrawSync/*0x8004f580*/(0);
    VSync/*0x80047e44*/(0);
    func_0x8001a584(local_34);
    Pad_Tick/*0x800120d4*/();
    SetDispMask/*0x8004f4e8*/(1);
    uVar3 = 0;
    if ((_DAT_80065930 & 0x58d00000) != 0) {
      if ((_DAT_80065930 & 0x50000000) != 0) {
        uVar3 = 9;
      }
      func_0x8004454c(1,DAT_8011338c,uVar3);
      FUN_8001a4f8/*0x8001a4f8*/(local_34,0);
      iVar13 = 0;
      func_0x8001a584(local_34);
      func_0x8001a4ac(local_34);
      if (((_DAT_80065930 & 0x40000000) != 0) &&
         ((int)uVar12 < *(int *)(_DAT_8006590c + iVar2 + 4) + -1)) {
        uVar12 = uVar12 + 1;
      }
      if (((_DAT_80065930 & 0x10000000) != 0) && (uVar12 != 0)) {
        uVar12 = uVar12 - 1;
      }
    }
    SetDispMask/*0x8004f4e8*/(1);
    iVar10 = -0x7ffa0000;
    if ((_DAT_80065930 & 0x8400000) != 0) {
      iVar11 = 1 << (uVar12 & 0x1f);
      if ((int)(uint)*pbVar6 < iVar11) {
        FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,0xb);
      }
      if (((_DAT_80065930 & 0x8400000) != 0) && (iVar11 <= (int)(uint)*pbVar6)) break;
    }
  } while ((_DAT_80065930 & 0x900000) == 0);
  uVar3 = 0;
  if ((_DAT_80065930 & 0x8400000) != 0) {
    uVar3 = 7;
  }
  FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,uVar3);
  iVar13 = 0;
  FUN_800183ec/*0x800183ec*/(local_30);
  func_0x8001a4ac(local_3c);
  if (0 < *(int *)(_DAT_8006590c + param_1 * 8 + 4)) {
    piVar9 = local_a8;
    do {
      iVar2 = *piVar9;
      piVar9 = piVar9 + 1;
      FUN_8001af48/*0x8001af48*/(iVar2);
      iVar13 = iVar13 + 1;
    } while (iVar13 < *(int *)(_DAT_8006590c + param_1 * 8 + 4));
  }
  iVar10 = 0;
  FUN_8001af48/*0x8001af48*/(local_38);
  FUN_8001af48/*0x8001af48*/(iVar8);
  FUN_8001aa38/*0x8001aa38*/(local_40);
  FUN_800190a8/*0x800190a8*/(local_48);
  FUN_800190a8/*0x800190a8*/(local_44);
  Buffer_StartOTag/*0x800119c0*/(_DAT_80065308);
  iVar2 = 0;
  DAT_80065904 = (undefined1)uVar12;
  iVar13 = 0;
  do {
    iVar11 = uVar12 * 0x10 + *(int *)(_DAT_8006590c + param_1 * 8 + 8);
    iVar8 = iVar13;
    if ((int)(uint)*(ushort *)(iVar11 + 6) <= iVar10) break;
    bVar1 = *(byte *)(iVar2 + *(int *)(iVar11 + 8));
    if (bVar1 != 0xff) {
      iVar8 = iVar13 + 1;
      (&DAT_80065676)[iVar13] = bVar1 & 0x7f;
      (&DAT_8006567c)[iVar13] = 1;
    }
    iVar2 = iVar2 + 6;
    iVar10 = iVar10 + 1;
    iVar13 = iVar8;
  } while (iVar8 < 6);
  for (; iVar8 < 6; iVar8 = iVar8 + 1) {
    (&DAT_80065676)[iVar8] = 0xff;
  }
  return *(undefined1 *)(uVar12 * 0x10 + *(int *)(_DAT_8006590c + param_1 * 8 + 8));
}

