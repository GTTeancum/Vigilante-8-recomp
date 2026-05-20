// addr: 0x80105660  name: FUN_80105660

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_80105660(uint param_1)

{
  ushort uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  ushort *puVar5;
  undefined4 uVar6;
  code *pcVar7;
  undefined1 *puVar8;
  uint uVar9;
  int iVar10;
  uint *puVar11;
  uint *puVar12;
  int iVar13;
  uint *puVar14;
  short *psVar15;
  int iVar16;
  int *piVar17;
  int iVar18;
  int iVar19;
  int iVar20;
  undefined1 auStack_138 [16];
  undefined1 auStack_128 [16];
  undefined1 auStack_118 [16];
  undefined2 local_108;
  undefined1 auStack_f8 [32];
  int local_d8 [2];
  undefined1 auStack_d0 [32];
  uint local_b0 [7];
  int local_94;
  int aiStack_90 [6];
  undefined1 auStack_78 [3];
  undefined1 local_75;
  uint local_74;
  undefined1 auStack_70 [3];
  undefined1 local_6d;
  undefined1 local_69;
  short local_68;
  short local_66;
  byte local_64;
  byte local_63;
  short local_60;
  short local_5e;
  int local_58;
  int local_54;
  int local_50;
  uint *local_4c;
  uint *local_48;
  int local_44;
  uint *local_40;
  int local_3c;
  int local_38;
  int local_30;
  
  local_b0[4] = 0;
  local_b0[5] = 6;
  V8_MemSet/*0x80044efc*/(local_b0 + 6,0,8);
  V8_MemSet/*0x80044efc*/(aiStack_90,0,0x18);
  AsyncList_RecycleHead/*0x80011834*/();
  local_58 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_CharSel2_tbl_80100858);
  Async_StopAllocCallback/*0x800165cc*/(0);
  SetDispMask/*0x8004f4e8*/(0);
  FUN_80019e7c/*0x80019e7c*/(1);
  FUN_80019e20/*0x80019e20*/();
  iVar3 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4),1);
  *(undefined1 *)(iVar3 + 4) = 0x7c;
  *(undefined1 *)(iVar3 + 5) = 0x60;
  *(undefined1 *)(iVar3 + 6) = 0;
  FUN_80019a58/*0x80019a58*/(iVar3,s_CHOOSE_8010086c,&DAT_80111e74,0x4442);
  FUN_80019a58/*0x80019a58*/(iVar3,s_PLAYERS_80100874,&DAT_80111e74,0x4446);
  *(undefined1 *)(iVar3 + 4) = 0x7c;
  *(undefined1 *)(iVar3 + 5) = 0x60;
  *(undefined1 *)(iVar3 + 6) = 0;
  FUN_800190a8/*0x800190a8*/();
  uVar4 = FUN_8001a2cc/*0x8001a2cc*/(&DAT_80111e54,0xa0,0,0);
  local_54 = FUN_8001ac44/*0x8001ac44*/(_DAT_800737d4,10,0x80,0);
  *(undefined2 *)(*(int *)(local_54 + 0x30) + 0x28) = 0x40;
  *(undefined2 *)(local_54 + 0x40) = 0xfc00;
  *(undefined4 *)(local_54 + 0x48) = 0x4000000;
  *(undefined4 *)(local_54 + 0x4c) = 0x2ff800;
  *(undefined4 *)(local_54 + 0x50) = 0x4000000;
  FUN_8001dc1c/*0x8001dc1c*/(local_54);
  FUN_8001d708/*0x8001d708*/(local_54);
  func_0x80018d00(auStack_118,local_58 + *(int *)(local_58 + 0x38),auStack_138,0x5e,0xaa);
  DrawPrim/*0x8004fb18*/(auStack_118);
  local_108 = 0x19e;
  DrawPrim/*0x8004fb18*/(auStack_118);
  FUN_8001884c/*0x8001884c*/(auStack_138);
  func_0x80018d00(auStack_118,local_58 + *(int *)(local_58 + 0x3c),auStack_138,0,0);
  func_0x80018d00(auStack_f8,local_58 + *(int *)(local_58 + 0x40),auStack_128,0,0);
  DrawSync/*0x8004f580*/(0);
  DAT_800659d2 = 0x80;
  local_b0[0] = func_0x8001a24c(&DAT_80111e44);
  local_b0[1] = func_0x8001a24c(&DAT_80111e4c);
  puVar14 = (uint *)&DAT_80111e64;
  local_b0[2] = func_0x8001a24c(&DAT_80111e64);
  local_b0[3] = func_0x8001a24c(&DAT_80111e6c);
  local_4c = local_b0 + 4;
  local_48 = local_b0 + 6;
  iVar3 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),1);
  func_0x8001a4ac(uVar4);
  func_0x8001d3d8();
  FUN_80016da8/*0x80016da8*/(auStack_d0);
  do {
    local_50 = _DAT_8005ffb4;
    Pad_Tick/*0x800120d4*/();
    iVar20 = 0;
    iVar19 = 0;
    local_44 = 0;
    local_3c = 1;
    local_38 = 0;
    local_40 = local_48;
    puVar11 = local_4c;
    do {
      Buffer_StartOTagOther/*0x80011a10*/();
      uVar9 = *local_40;
      if (uVar9 == 1) {
LAB_80105cf4:
        _DAT_80065a30 = (ushort)((int)(iVar20 * -0x100 + 0x140U & 0x3ff) >> 6) | 0x110;
        uVar4 = FUN_8001a2cc/*0x8001a2cc*/(&DAT_80111e54 + local_44,0xa0,0xffffffe4,
                                *(undefined4 *)((int)&DAT_80111e7c + iVar19));
        FUN_8001a4f8/*0x8001a4f8*/(uVar4,0);
        func_0x8001d404(0,&DAT_80100628,0xffffff);
        FUN_8004d314/*0x8004d314*/(auStack_d0,&DAT_8010087c);
        FUN_8001d9c0/*0x8001d9c0*/(auStack_d0,0x200);
        FUN_8001d370/*0x8001d370*/();
        piVar17 = (int *)((int)local_d8 + iVar19);
        RotMatrixY/*0x8004d914*/(0x20,*piVar17 + 0x10);
        iVar16 = *piVar17;
        pcVar7 = (code *)0x0;
        if (*(code **)(iVar16 + 100) != (code *)0x0) {
          (**(code **)(iVar16 + 100))(iVar16,0,0);
          iVar16 = *piVar17;
          pcVar7 = *(code **)(iVar16 + 100);
        }
        if (pcVar7 != (code *)0x0) {
          (*pcVar7)(iVar16,0,0);
        }
        if ((*(uint *)*piVar17 & 0x1100000) == 0x100000) {
          uVar1 = *(ushort *)(*piVar17 + 0xa2);
          uVar6 = 3;
          if ((0x1400 < uVar1) && (uVar6 = 2, uVar1 < 0x2001)) {
            uVar6 = 1;
          }
          func_0x8004454c(local_3c,DAT_8011338c,uVar6);
          **(uint **)((int)local_d8 + iVar19) = **(uint **)((int)local_d8 + iVar19) | 0x1000000;
        }
        iVar16 = *(int *)((int)local_d8 + iVar19);
        *(undefined4 *)(iVar16 + 0x2c) = 0x4000000;
        *(undefined4 *)(iVar16 + 0x24) = 0x4000000;
        FUN_8001de08/*0x8001de08*/(*(int *)((int)local_d8 + iVar19));
        FUN_8001de08/*0x8001de08*/(local_54);
        DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
        DrawSync/*0x8004f580*/(0);
        func_0x8001a584(uVar4);
        func_0x8001a4ac(uVar4);
      }
      else if ((((int)uVar9 < 2) && (puVar14 = (uint *)0x80110000, uVar9 == 0)) || (uVar9 != 2)) {
        psVar15 = (short *)((int)puVar14 + local_44 + 0x1e64);
        puVar5 = (ushort *)FUN_80018124/*0x80018124*/((int)psVar15[2],(int)psVar15[3],0x40,0x100,0x100,0x100);
        FUN_80019e20/*0x80019e20*/();
        func_0x8001a2ac(*(undefined4 *)((int)local_b0 + iVar19 + 8),(int)*psVar15,(int)psVar15[1]);
        local_75 = 1;
        uVar9 = (int)(short)(puVar5[1] & 0x200) << 2;
        local_74 = (int)((uint)(puVar5[1] & 0x100) << 0x10) >> 0x14 | (*puVar5 & 0x3ff) >> 6 | 0x120
                   | uVar9 | 0xe1000600;
        DrawPrim/*0x8004fb18*/(auStack_78,uVar9,0xe1000600,2);
        local_6d = 4;
        local_69 = 0x67;
        local_64 = (byte)*puVar5 & 0x3f;
        local_63 = (byte)puVar5[1];
        local_68 = *psVar15;
        local_66 = psVar15[1];
        local_60 = psVar15[2];
        local_5e = psVar15[3];
        DrawPrim/*0x8004fb18*/(auStack_70);
        DrawSync/*0x8004f580*/(0);
        FUN_800183ec/*0x800183ec*/(puVar5);
        psVar15 = (short *)(&DAT_80111e44 + local_44);
        func_0x8001a2ac(*(undefined4 *)((int)local_b0 + iVar19),(int)*psVar15,
                        (int)*(short *)(&DAT_80111e46 + local_44));
        FUN_80019a58/*0x80019a58*/(iVar3,(&PTR_s_Chassey_Blue_80111c68)[*puVar11 * 5],psVar15,10);
        iVar16 = local_44 + -0x7feee1cc;
        FUN_8001a0ac/*0x8001a0ac*/(iVar16,*(undefined4 *)((int)&DAT_80111e7c + iVar19));
        FUN_80019a58/*0x80019a58*/(iVar3,(&PTR_s__67_Rattler_80111c70)[*puVar11 * 5],iVar16,2);
        *(uint *)(iVar3 + 4) =
             *(uint *)(iVar3 + 4) & 0xff000000 | *(uint *)((int)&DAT_80111e84 + iVar19);
        FUN_80019a58/*0x80019a58*/(iVar3,&DAT_801007a8,iVar16,0);
        FUN_80019a58/*0x80019a58*/(iVar3,&DAT_801007ac,iVar16,1);
        *(undefined1 *)(iVar3 + 4) = 0x80;
        *(undefined1 *)(iVar3 + 5) = 0x80;
        *(undefined1 *)(iVar3 + 6) = 0x80;
        uVar4 = FUN_8001a2cc/*0x8001a2cc*/(&DAT_80111e54,0,0,0);
        puVar14 = (uint *)func_0x80021c20((short)*puVar11);
        piVar17 = (int *)((int)local_d8 + iVar19);
        *piVar17 = (int)puVar14;
        if ((int)*puVar11 < 0xc) {
          *puVar14 = *puVar14 | 8;
          uVar6 = FUN_8001bda0/*0x8001bda0*/(_DAT_800737d4,0xb);
          FUN_8003e598/*0x8003e598*/(*piVar17,uVar6);
        }
        uVar2 = DAT_80100818;
        uVar6 = DAT_80100814;
        iVar16 = *piVar17;
        *(undefined4 *)(iVar16 + 0x48) = DAT_80100810;
        *(undefined4 *)(iVar16 + 0x4c) = uVar6;
        *(undefined4 *)(iVar16 + 0x50) = uVar2;
        FUN_8001d708/*0x8001d708*/(*piVar17);
        FUN_8001dc1c/*0x8001dc1c*/(*piVar17);
        func_0x8001a4ac(uVar4);
        func_0x8004454c(local_3c,DAT_8011338c,4);
        *local_40 = *local_40 + 1;
        goto LAB_80105cf4;
      }
      iVar18 = 0;
      FUN_80019e20/*0x80019e20*/();
      local_30 = -0x7ffa15a0;
      iVar16 = local_38;
      do {
        piVar17 = (int *)((int)aiStack_90 + iVar16);
        iVar10 = (uint)*(byte *)(iVar18 + *(int *)((int)local_4c + iVar19) * 0x24 + local_30 + 0x20)
                 - *piVar17;
        if ((iVar10 < -1) || (iVar13 = 1, iVar10 < 2)) {
          iVar13 = -1;
        }
        *piVar17 = *piVar17 + iVar13;
        piVar17 = (int *)((int)aiStack_90 + iVar16);
        iVar16 = iVar16 + 4;
        iVar18 = iVar18 + 1;
      } while (iVar18 < 3);
      puVar14 = (uint *)(&DAT_80065930 + iVar19);
      if (((*puVar14 & 0xa8d00000) != 0) || ((_DAT_80065930 & 0x900000) != 0)) {
        FUN_8004445c/*0x8004445c*/(local_3c,DAT_8011338c,0,*piVar17 * 5,auStack_118,auStack_f8);
        if (*(int *)((int)local_48 + iVar19) != 2) {
          if ((((*puVar14 & 0x8400000) != 0) &&
              (local_4c[1 - iVar20] == *(uint *)((int)local_4c + iVar19))) &&
             ((_DAT_80065908 & 0x20) == 0)) {
            FUN_8004445c/*0x8004445c*/(local_3c,DAT_8011338c,0xb);
          }
          if (*puVar11 != local_4c[1 - iVar20]) {
            FUN_8001a994/*0x8001a994*/(*(undefined4 *)(*(int *)((int)local_d8 + iVar19) + 0x58));
          }
          func_0x8001d490(*(undefined4 *)((int)local_d8 + iVar19));
        }
        puVar12 = (uint *)(&DAT_80065930 + iVar19);
        if ((*puVar12 & 0x8400000) != 0) {
          if (*local_40 == 2) goto LAB_80106204;
          FUN_8004445c/*0x8004445c*/(iVar20 + 3,DAT_8011338c,(&DAT_80111c74)[*puVar11 * 0x14]);
          if (local_48[1 - iVar20] != 2) {
            puVar14 = (uint *)(local_44 + -0x7feee1cc);
            FUN_8001a0ac/*0x8001a0ac*/(puVar14,*(undefined4 *)((int)&DAT_80111e7c + iVar19));
            FUN_80019a58/*0x80019a58*/(iVar3,s_Waiting_for_other_player_80111b78,puVar14,2);
          }
          puVar12 = (uint *)0x1;
          *local_40 = 2;
        }
        *(undefined4 *)((int)local_48 + iVar19) = 0;
        do {
          iVar16 = (*puVar12 >> 0x1d & 1) + *puVar11;
          if ((int)*puVar12 < 0) {
            iVar16 = 0xd;
          }
          uVar9 = (iVar16 + 0xd) % 0xd;
          *puVar11 = uVar9;
        } while ((param_1 & 1 << (uVar9 & 0x1f)) != 0);
      }
LAB_80106204:
      SetDispMask/*0x8004f4e8*/(1);
      puVar11 = puVar11 + 1;
      iVar19 = iVar19 + 4;
      iVar20 = iVar20 + 1;
      local_44 = local_44 + 8;
      local_40 = local_40 + 1;
      local_3c = local_3c + 1;
      local_38 = local_38 + 0xc;
    } while (iVar20 < 2);
    FUN_80019e20/*0x80019e20*/();
    puVar11 = (uint *)(local_50 + 2);
    while (_DAT_8005ffb4 < (int)puVar11) {
      VSync/*0x80047e44*/(0);
      puVar14 = puVar11;
    }
    if (((local_b0[6] == 2) && (local_94 == 2)) ||
       (((_DAT_80065930 | _DAT_80065934) & 0x900000) != 0)) {
      if (((_DAT_80065930 | _DAT_80065934) & 0x900000) != 0) {
        FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,8);
      }
      puVar8 = &DAT_80065674;
      iVar19 = 0;
      do {
        puVar8[2] = 0xff;
        iVar20 = iVar19 + 1;
        puVar8 = &DAT_80065675 + iVar19;
        iVar19 = iVar20;
      } while (iVar20 < 6);
      FUN_800183ec/*0x800183ec*/(local_b0[0]);
      FUN_800183ec/*0x800183ec*/(local_b0[1]);
      FUN_800183ec/*0x800183ec*/(local_b0[2]);
      FUN_800183ec/*0x800183ec*/(local_b0[3]);
      FUN_8001af48/*0x8001af48*/(local_54);
      FUN_800190a8/*0x800190a8*/(iVar3);
      Buffer_StartOTag/*0x800119c0*/(_DAT_80065308);
      Heap_Free/*0x80045088*/(local_58);
      return local_b0[4] | local_b0[5] << 8;
    }
  } while( true );
}

