// addr: 0x801088b4  name: FUN_801088b4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_801088b4(int param_1,int param_2)

{
  byte bVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  undefined **ppuVar9;
  uint uVar10;
  uint uVar11;
  undefined **ppuVar12;
  uint uVar13;
  uint unaff_s1;
  int iVar14;
  int iVar15;
  int iVar16;
  int local_res0;
  undefined2 local_78;
  undefined2 local_76;
  undefined2 local_74;
  undefined2 local_72;
  short local_70 [3];
  undefined2 local_6a;
  undefined4 local_68;
  undefined **local_64;
  undefined4 local_60;
  uint local_5c;
  int local_58;
  uint local_54;
  uint local_50;
  uint local_48;
  uint local_44;
  ushort local_40;
  int local_38;
  byte *local_34;
  int local_30;
  int local_2c;
  
  local_68 = FUN_8001a2cc/*0x8001a2cc*/(&DAT_80100c38,0xb8,0x80,0);
  local_64 = (undefined **)FUN_8001ac44/*0x8001ac44*/(DAT_80113394,0x56,0x80,8);
  local_60 = 0;
  local_5c = 0;
  iVar3 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),1);
  local_38 = param_2 * 8 + -0x7ff9a6c0;
  local_30 = param_2 * 0x22;
  local_2c = local_30 + -0xc;
  iVar16 = 0;
  iVar15 = -1;
  *local_64 = (undefined *)((uint)*local_64 | 0x410);
  *(undefined2 *)(local_64[0xc] + 0x28) = 0xfff8;
  ppuVar12 = local_64;
  local_res0 = param_1;
  do {
    iVar4 = Player_GetType/*0x80011f8c*/(param_2);
    if ((iVar4 != iVar15) || ((*(char *)(local_38 + 2) == '\x02') != local_5c)) {
      iVar5 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4),1);
      local_78 = 0xe0;
      local_76 = 0x30;
      local_74 = 0x1a0;
      local_72 = 0x38;
      *(undefined1 *)(iVar5 + 4) = 0x32;
      *(undefined1 *)(iVar5 + 5) = 0x73;
      *(undefined1 *)(iVar5 + 6) = 100;
      *(undefined1 *)(iVar3 + 4) = 0x32;
      *(undefined1 *)(iVar3 + 5) = 0x73;
      *(undefined1 *)(iVar3 + 6) = 100;
      FUN_80019e20/*0x80019e20*/();
      FUN_8001a0ac/*0x8001a0ac*/(&local_78,0);
      iVar15 = Player_GetType/*0x80011f8c*/(param_2);
      iVar4 = iVar15 * 0x14;
      FUN_80019a58/*0x80019a58*/(iVar5,(&PTR_s_UNPLUGGED_801120a4)[iVar15 * 5],&local_78,2);
      FUN_800190a8/*0x800190a8*/(iVar5);
      local_5c = (uint)(*(char *)(local_38 + 2) == '\x02');
      if (local_5c != 0) {
        FUN_80019a58/*0x80019a58*/(iVar3,s__DUAL_SHOCK__80100c40,&local_78,6);
      }
      if (iVar16 != 0) {
        FUN_8001af48/*0x8001af48*/(iVar16);
      }
      iVar16 = FUN_8001ac44/*0x8001ac44*/(DAT_80113394,(&DAT_801120b0)[iVar4],0x80,0);
      ppuVar12 = DAT_80100818;
      uVar2 = DAT_80100814;
      *(undefined4 *)(iVar16 + 0x48) = DAT_80100810;
      *(undefined4 *)(iVar16 + 0x4c) = uVar2;
      *(undefined ***)(iVar16 + 0x50) = ppuVar12;
      *(undefined2 *)(iVar16 + 0x40) = 0x155;
      FUN_8001dc1c/*0x8001dc1c*/(iVar16);
      uVar8 = *(uint *)(&DAT_801120b4 + iVar4);
      *(uint *)(&DAT_801120b4 + iVar4) = uVar8 & 0xfffff7ff;
      if (local_5c != 0) {
        *(uint *)(&DAT_801120b4 + iVar4) = uVar8 & 0xfffff7ff | 0x800;
      }
      uVar8 = *(uint *)(&DAT_801120b4 + iVar4);
      unaff_s1 = 0;
      if (uVar8 != 0) {
        uVar6 = uVar8 & 1;
        while ((uVar6 == 0 && (unaff_s1 = unaff_s1 + 1, uVar8 != 0))) {
          uVar6 = uVar8 & 1 << (unaff_s1 & 0x1f);
        }
      }
      local_58 = 0;
      iVar4 = iVar15 * 8 + param_2 * 0x30;
      local_48 = *(uint *)(&DAT_80056774 + iVar4);
      local_44 = *(uint *)(&DAT_80056778 + iVar4);
      local_40 = *(ushort *)(&DAT_800567d4 + iVar15 * 2 + param_2 * 0xc);
    }
    if ((int)unaff_s1 < 0x10) {
      if ((int)(unaff_s1 << 0x1c) < 0) {
        ppuVar12 = (undefined **)0x0;
      }
      else {
        ppuVar12 = (undefined **)(local_44 >> ((unaff_s1 & 7) << 2));
      }
    }
    uVar8 = (int)(uint)local_40 >> ((unaff_s1 - 0x10) * 4 & 0x1f) & 0xf;
    FUN_8001a0ac/*0x8001a0ac*/(&DAT_80112124,0);
    if (local_res0 == 0) goto LAB_80108eec;
    *(undefined1 *)(iVar3 + 4) = 0x80;
    *(undefined1 *)(iVar3 + 5) = 0x80;
    *(undefined1 *)(iVar3 + 6) = 0x80;
    ppuVar9 = &PTR_PTR_801120a8 + iVar15 * 5;
    if (0xf < (int)unaff_s1) {
      ppuVar9 = (undefined **)(&DAT_801120ac + iVar15 * 0x14);
    }
    FUN_80019a58/*0x80019a58*/(iVar3,*(undefined4 *)(*ppuVar9 + uVar8 * 4),&DAT_80112134,10);
    if (unaff_s1 - 4 < 4) {
      *(uint *)(iVar3 + 4) =
           *(uint *)(iVar3 + 4) & 0xff000000 | *(uint *)((unaff_s1 - 4) * 4 + -0x7feede68);
    }
    if (((int)unaff_s1 < 0x10) || (iVar15 == 3)) {
      ppuVar12 = &PTR_s_UNPLUGGED_801120a4;
    }
    FUN_80019a58/*0x80019a58*/(iVar3,&DAT_80111fbc,&DAT_8011212c,10);
    if ((int)unaff_s1 < 0x10) goto LAB_80108e1c;
    if (local_5c != 0) goto LAB_80108e1c;
    *(undefined1 *)(iVar3 + 4) = 0x40;
    *(undefined1 *)(iVar3 + 5) = 0x40;
    *(undefined1 *)(iVar3 + 6) = 0x40;
    if (local_58 == 0) goto LAB_80108dfc;
LAB_80108dd0:
    local_54 = (uint)*(byte *)((int)ppuVar12 + unaff_s1 + local_2c + 0x6458);
    local_58 = 1;
    local_50 = local_54;
LAB_80108dfc:
    FUN_80019a58/*0x80019a58*/(iVar3,&DAT_80100c60,&DAT_80112134,6);
LAB_80108e1c:
    *(undefined1 *)(iVar3 + 5) = 0x80;
    *(undefined1 *)(iVar3 + 4) = 0;
    *(undefined1 *)(iVar3 + 6) = 0;
    if ((iVar15 != 3) || ((int)unaff_s1 < 0x10)) {
      FUN_80019a58/*0x80019a58*/(iVar3,&DAT_801007a8,&DAT_80112134,8);
      FUN_80019a58/*0x80019a58*/(iVar3,&DAT_801007ac,&DAT_80112134,9);
    }
    FUN_80019a58/*0x80019a58*/(iVar3,&DAT_801006ec,&DAT_8011212c,2);
    FUN_80019a58/*0x80019a58*/(iVar3,&DAT_801006f0,&DAT_8011212c,6);
    local_60 = FUN_8001b038/*0x8001b038*/(iVar16,unaff_s1 - 0x8000 & 0xffff);
    FUN_8001b2fc/*0x8001b2fc*/(iVar16,local_60,local_64);
    *(undefined2 *)(local_64 + 0x10) = 0;
    *(undefined2 *)((int)local_64 + 0x42) = 0;
    *(undefined2 *)(local_64 + 0x11) = 0;
LAB_80108eec:
    local_34 = (byte *)(unaff_s1 + local_2c + -0x7ff99ba8);
    do {
      iVar4 = local_30;
      iVar5 = *(int *)(iVar16 + 0x38);
      if (iVar5 != 0) {
        iVar14 = local_30 + 4;
        do {
          uVar6 = (uint)(iVar15 < 4);
          if (iVar15 == 3) {
            iVar7 = *(short *)(iVar5 + 6) + iVar14;
            if (*(short *)(iVar5 + 6) == 0) {
              iVar7 = (*(byte *)(iVar4 + -0x7ff99ba4) - 0x80) * 0x10;
              *(short *)(iVar5 + 0x44) = (short)iVar7;
            }
            uVar6 = iVar7 + 0x80066458;
          }
          if ((uVar6 == 0) && (*(short *)(iVar5 + 6) < 2)) {
            iVar7 = (*(byte *)(*(short *)(iVar5 + 6) * 2 + iVar14 + -0x7ff99ba8) - 0x80) * 0x2aa;
            if (iVar7 < 0) {
              iVar7 = iVar7 + 0xff;
            }
            *(short *)(iVar5 + 0x44) = (short)((uint)iVar7 >> 8);
            iVar7 = (*(byte *)(*(short *)(iVar5 + 6) * 2 + iVar4 + -0x7ff99ba3) - 0x80) * 0x2aa;
            if (iVar7 < 0) {
              iVar7 = iVar7 + 0xff;
            }
            *(short *)(iVar5 + 0x40) = (short)((uint)iVar7 >> 8);
          }
          FUN_8001d708/*0x8001d708*/(iVar5);
          iVar5 = *(int *)(iVar5 + 0x34);
        } while (iVar5 != 0);
      }
      Buffer_StartOTagOther/*0x80011a10*/();
      FUN_8001a4f8/*0x8001a4f8*/(local_68,0);
      *(short *)(iVar16 + 0x42) = *(short *)(iVar16 + 0x42) + 0x11;
      FUN_8001d708/*0x8001d708*/(iVar16);
      iVar4 = (int)_DAT_800659d0;
      _DAT_800659d0 = _DAT_800659d0 + 1;
      FUN_8001fcb4/*0x8001fcb4*/(local_64,iVar4);
      if (((local_res0 != 0) && (local_58 != 0)) && (0xf < (int)unaff_s1)) {
        local_70[0] = (short)local_54 + 0x40;
        local_70[1] = 0xde;
        local_70[2] = (short)local_50 - (short)local_54;
        local_6a = 0x10;
        bVar1 = *local_34;
        FUN_8001a0ac/*0x8001a0ac*/(&DAT_80100c6c,0xffffff);
        FUN_8001a0ac/*0x8001a0ac*/(&DAT_80100c74,0);
        FUN_8001a0ac/*0x8001a0ac*/(local_70,0x8000);
        FUN_80019f44/*0x80019f44*/(bVar1 + 0x40,0xde,bVar1 + 0x40,0xee,0xff00);
      }
      FUN_8001de08/*0x8001de08*/(iVar16);
      DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
      VSync/*0x80047e44*/(0);
      DrawSync/*0x8004f580*/(0);
      func_0x8001a584(local_68);
      Pad_Tick/*0x800120d4*/();
      if (((local_res0 != 0) && (0xf < (int)unaff_s1)) && (local_5c == 0)) {
        if (local_58 != 0) {
          uVar10 = (uint)*local_34;
          uVar6 = *(uint *)(&DAT_80065930 + param_2 * 4);
          *(uint *)(&DAT_80065930 + param_2 * 4) = 0;
          if ((int)uVar10 < (int)local_54) {
            local_54 = uVar10;
          }
          if ((int)local_50 < (int)uVar10) {
            local_50 = uVar10;
          }
          if ((uVar6 & 0x8000000) == 0) goto LAB_801092f0;
          local_58 = 0;
        }
        ppuVar12 = (undefined **)0x80060000;
        if ((*(uint *)(&DAT_80065930 + param_2 * 4) & 0x200000) != 0) goto LAB_80108dd0;
      }
LAB_801092f0:
      iVar4 = Player_GetType/*0x80011f8c*/(param_2);
    } while (((iVar4 == iVar15) && ((*(char *)(local_38 + 2) == '\x02') == local_5c)) &&
            ((((_DAT_80065930 | _DAT_80065934) & 0x50900000) == 0 &&
             ((*(uint *)(&DAT_80065930 + param_2 * 4) & 0xa8400000) == 0))));
    ppuVar12 = local_64;
    if (local_64[0xf] != (undefined *)0x0) {
      FUN_8001d564/*0x8001d564*/(local_64);
    }
    iVar4 = 0xd;
    if (((_DAT_80065930 | _DAT_80065934) & 0x900000) == 0) {
      iVar4 = (uint)(((_DAT_80065930 | _DAT_80065934) & 0x8500000) != 0) << 3;
    }
    FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,iVar4);
    FUN_80019e20/*0x80019e20*/();
    if (local_res0 == 0) goto LAB_80109718;
    if ((iVar15 != 3) || ((int)unaff_s1 < 0x10)) {
      if ((*(int *)(&DAT_80065930 + param_2 * 4) < 0) && (uVar8 = uVar8 - 1, (int)uVar8 < 0)) {
        do {
          ppuVar12 = &PTR_PTR_801120a8 + iVar15 * 5;
          if (0xf < (int)unaff_s1) {
            ppuVar12 = (undefined **)(&DAT_801120ac + iVar15 * 0x14);
          }
          uVar8 = uVar8 + 1;
        } while (*(int *)(*ppuVar12 + uVar8 * 4 + 4) != 0);
      }
      if ((*(uint *)(&DAT_80065930 + param_2 * 4) & 0x20000000) != 0) {
        ppuVar12 = &PTR_PTR_801120a8 + iVar15 * 5;
        if (0xf < (int)unaff_s1) {
          ppuVar12 = (undefined **)(&DAT_801120ac + iVar15 * 0x14);
        }
        uVar8 = uVar8 + 1;
        if (*(int *)(*ppuVar12 + uVar8 * 4) == 0) {
          uVar8 = 0;
        }
      }
      if ((int)unaff_s1 < 0x10) {
        uVar6 = unaff_s1 * 4;
        if ((int)(unaff_s1 << 0x1c) < 0) {
          uVar10 = 0xf << (uVar6 & 0x1f);
          uVar11 = 0;
        }
        else {
          uVar10 = 0 << (uVar6 & 0x1f);
          if (unaff_s1 << 0x1c != 0) {
            uVar10 = uVar10 | 0xfU >> (unaff_s1 * -4 & 0x1f);
          }
          uVar11 = 0xf << (uVar6 & 0x1f);
        }
        if ((int)(unaff_s1 << 0x1c) < 0) {
          uVar13 = uVar8 << (uVar6 & 0x1f);
          uVar6 = 0;
        }
        else {
          uVar13 = ((int)uVar8 >> 0x1f) << (uVar6 & 0x1f);
          if (unaff_s1 << 0x1c != 0) {
            uVar13 = uVar13 | uVar8 >> (unaff_s1 * -4 & 0x1f);
          }
          uVar6 = uVar8 << (uVar6 & 0x1f);
        }
        local_48 = local_48 & ~uVar11 | uVar6;
        local_44 = local_44 & ~uVar10 | uVar13;
      }
      ppuVar12 = (undefined **)(uint)local_40;
      uVar6 = (unaff_s1 - 0x10) * 4;
      local_40 = local_40 & ~(ushort)(0xf << (uVar6 & 0x1f)) | (ushort)(uVar8 << (uVar6 & 0x1f));
    }
    uVar8 = *(uint *)(&DAT_80065930 + param_2 * 4);
    do {
      if ((uVar8 & 0x10000000) != 0) {
        unaff_s1 = unaff_s1 - 1;
      }
      if ((uVar8 & 0x40000000) != 0) {
        unaff_s1 = unaff_s1 + 1;
      }
      unaff_s1 = unaff_s1 & 0x1f;
    } while (((*(uint *)(&DAT_801120b4 + iVar15 * 0x14) != 0) &&
             ((*(uint *)(&DAT_801120b4 + iVar15 * 0x14) & 1 << unaff_s1) == 0)) &&
            ((uVar8 & 0x50000000) != 0));
    if ((*(uint *)(&DAT_80065930 + param_2 * 4) & 0x8400000) == 0) {
LAB_801096dc:
      if (((_DAT_80065930 | _DAT_80065934) & 0x100000) != 0) goto LAB_801096fc;
    }
    else {
      iVar4 = iVar15 * 8 + param_2 * 0x30;
      *(uint *)(&DAT_80056774 + iVar4) = local_48;
      *(uint *)(&DAT_80056778 + iVar4) = local_44;
      *(ushort *)(&DAT_800567d4 + iVar15 * 2 + param_2 * 0xc) = local_40;
      if ((*(uint *)(&DAT_80065930 + param_2 * 4) & 0x8400000) == 0) goto LAB_801096dc;
LAB_801096fc:
      iVar15 = -1;
      local_res0 = 0;
LAB_80109718:
      if (((_DAT_80065930 | _DAT_80065934) & 0x50100000) != 0) goto LAB_80109788;
      ppuVar12 = (undefined **)0x1;
      if (((_DAT_80065930 | _DAT_80065934) & 0x8400000) != 0) {
        local_res0 = 1;
      }
    }
    if (((_DAT_80065930 | _DAT_80065934) & 0x800000) != 0) {
LAB_80109788:
      func_0x8001a4ac(local_68);
      FUN_8001af48/*0x8001af48*/(iVar16);
      FUN_8001af48/*0x8001af48*/(local_64);
      FUN_800190a8/*0x800190a8*/(iVar3);
      return;
    }
  } while( true );
}

