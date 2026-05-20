// addr: 0x801097e0  name: FUN_801097e0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_801097e0(int param_1)

{
  short sVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  undefined2 uVar10;
  int iVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  uint uVar14;
  uint uVar15;
  int aiStack_60080 [98314];
  undefined1 auStack_58 [32];
  short local_38;
  short local_36;
  byte local_34;
  undefined1 *local_30;
  
  uVar4 = FUN_8001a2cc/*0x8001a2cc*/(&DAT_80100c9c,0xb8,0x80,0);
  iVar5 = FUN_8001ac44/*0x8001ac44*/(DAT_80113394,0x3c,0x80,0);
  local_38 = _DAT_80065c04;
  local_34 = DAT_800658ac;
  local_36 = _DAT_80065be8;
  iVar6 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4),1);
  *(undefined1 *)(iVar6 + 4) = 0x32;
  *(undefined1 *)(iVar6 + 5) = 0x73;
  *(undefined1 *)(iVar6 + 6) = 100;
  FUN_80019a58/*0x80019a58*/(iVar6,s_AUDIO_SETTINGS_80100ca4,&DAT_80100994,2);
  FUN_800190a8/*0x800190a8*/(iVar6);
  iVar7 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),1);
  uVar3 = DAT_80100cbc;
  uVar2 = DAT_80100cb8;
  uVar15 = 0;
  for (iVar6 = *(int *)(iVar5 + 0x38); iVar6 != 0; iVar6 = *(int *)(iVar6 + 0x34)) {
    if (*(short *)(iVar6 + 6) < 3) {
      iVar8 = *(short *)(iVar6 + 6) * 0xc;
      uVar12 = *(undefined4 *)(iVar6 + 0x4c);
      uVar13 = *(undefined4 *)(iVar6 + 0x50);
      *(undefined4 *)(auStack_58 + iVar8 + -0x28) = *(undefined4 *)(iVar6 + 0x48);
      *(undefined4 *)(auStack_58 + iVar8 + -0x24) = uVar12;
      *(undefined4 *)(auStack_58 + iVar8 + -0x20) = uVar13;
    }
  }
  *(undefined4 *)(iVar5 + 0x48) = DAT_80100cb4;
  *(undefined4 *)(iVar5 + 0x4c) = uVar2;
  *(undefined4 *)(iVar5 + 0x50) = uVar3;
  FUN_8001dc1c/*0x8001dc1c*/(iVar5);
  local_30 = auStack_58;
  *(undefined2 *)(iVar5 + 0x40) = 0xaa;
  do {
    FUN_80019e20/*0x80019e20*/();
    *(undefined1 *)(iVar7 + 4) = 0x80;
    *(undefined1 *)(iVar7 + 5) = 0x80;
    *(undefined1 *)(iVar7 + 6) = 0x80;
    VSync/*0x80047e44*/(0);
    FUN_8001a0ac/*0x8001a0ac*/(&DAT_801121b4,0);
    if (param_1 != 0) {
      sprintf/*0x80053004*/(local_30,&DAT_80100cc4,s_stereo_80100cd0);
      FUN_80019a58/*0x80019a58*/(iVar7,(&PTR_s_Music_801121a8)[uVar15],&DAT_801121bc,10);
      FUN_80019a58/*0x80019a58*/(iVar7,auStack_58,&DAT_801121c4,10);
      *(undefined1 *)(iVar7 + 4) = 0;
      *(undefined1 *)(iVar7 + 5) = 0x80;
      *(undefined1 *)(iVar7 + 6) = 0;
      FUN_80019a58/*0x80019a58*/(iVar7,&DAT_801007a8,&DAT_801121bc,8);
      FUN_80019a58/*0x80019a58*/(iVar7,&DAT_801007ac,&DAT_801121bc,9);
      FUN_80019a58/*0x80019a58*/(iVar7,&DAT_801006ec,&DAT_801121c4,2);
      FUN_80019a58/*0x80019a58*/(iVar7,&DAT_801006f0,&DAT_801121c4,6);
    }
    do {
      Buffer_StartOTagOther/*0x80011a10*/();
      for (iVar6 = *(int *)(iVar5 + 0x38); iVar6 != 0; iVar6 = *(int *)(iVar6 + 0x34)) {
        *(undefined2 *)(*(int *)(iVar6 + 0x30) + 0x28) = 0xfffc;
        sVar1 = *(short *)(iVar6 + 6);
        if (sVar1 == 1) {
LAB_80109c04:
          iVar11 = (int)*(short *)(iVar6 + 0x1a) * (int)_DAT_80065be8;
          if (iVar11 < 0) {
            iVar11 = iVar11 + 0xfff;
          }
          iVar8 = (int)*(short *)(iVar6 + 0x20) * (int)_DAT_80065be8;
          *(int *)(iVar6 + 0x4c) = aiStack_60080[0x18004] + (iVar11 >> 0xc);
          if (iVar8 < 0) {
            iVar8 = iVar8 + 0xfff;
          }
          iVar8 = iVar8 >> 0xc;
          *(int *)(iVar6 + 0x50) = aiStack_60080[0x18005] + iVar8;
        }
        else if (((sVar1 < 2) && (sVar1 == 0)) || (iVar8 = -0x7ffa0000, sVar1 != 2)) {
          iVar8 = (int)*(short *)(iVar6 + 0x1a) * (int)_DAT_80065c04;
          if (iVar8 < 0) {
            iVar8 = iVar8 + 0xfff;
          }
          *(int *)(iVar6 + 0x4c) = aiStack_60080[0x18001] + (iVar8 >> 0xc);
          goto LAB_80109c04;
        }
        uVar10 = 0x2aa;
        if (*(char *)(iVar8 + 0x58ac) != '\0') {
          uVar10 = 0xfd56;
        }
        *(undefined2 *)(iVar6 + 0x40) = uVar10;
        FUN_8001d708/*0x8001d708*/(iVar6);
      }
      *(short *)(iVar5 + 0x42) = *(short *)(iVar5 + 0x42) + 0x11;
      FUN_8001d708/*0x8001d708*/(iVar5);
      FUN_8001a4f8/*0x8001a4f8*/(uVar4,0);
      FUN_8001de08/*0x8001de08*/(iVar5);
      DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
      VSync/*0x80047e44*/(0);
      DrawSync/*0x8004f580*/(0);
      func_0x8001a584(uVar4);
      Pad_Tick/*0x800120d4*/();
      uVar14 = _DAT_80065930 | _DAT_80065934;
    } while (((uVar14 & 0x50000000) == 0) &&
            ((((param_1 == 0 || (1 < uVar15)) || ((uVar14 & 0x5000) == 0)) &&
             ((uVar14 & 0xa8d00000) == 0))));
    iVar6 = -0x7fef0000;
    if (param_1 == 0) {
LAB_80109f50:
      FUN_8004445c/*0x8004445c*/(1,*(undefined4 *)(iVar6 + 0x338c),(uint)((uVar14 & 0x8500000) != 0) << 3);
      if ((uVar14 & 0x50100000) != 0) break;
      if ((uVar14 & 0x8400000) != 0) {
        param_1 = 1;
      }
    }
    else {
      if (uVar15 == 1) {
LAB_80109dec:
        iVar8 = (int)_DAT_80065be8;
        if ((uVar14 & 0x4000) != 0) {
          iVar8 = 1;
        }
        iVar8 = iVar8 + (uVar14 >> 0xc & 1) * 0x100;
        iVar11 = 0;
        if (0 < iVar8) {
          iVar11 = iVar8;
        }
        iVar8 = 0x3fff;
        if (iVar11 < 0x3fff) {
          iVar8 = iVar11;
        }
        _DAT_80065be8 = (short)iVar8;
        uVar9 = FUN_80044574/*0x80044574*/(2,_DAT_80065be8 * 0x10001);
      }
      else if (((uVar15 < 2) && (uVar15 == 0)) || (uVar9 = 0x50000000, uVar15 != 2)) {
        _DAT_80065c04 = 1;
        goto LAB_80109dec;
      }
      if ((uVar14 & uVar9) != 0) {
        DAT_800658ac = (byte)(uVar14 >> 0x1c) & 1;
      }
      Audio_SetMasterVolume/*0x80044080*/(DAT_800658ac == 0,_DAT_80065c04,_DAT_80065be8);
      if ((int)uVar14 < 0) {
        uVar15 = uVar15 - 1;
      }
      uVar9 = uVar15 + 3;
      if ((uVar14 & 0x20000000) != 0) {
        uVar9 = uVar15 + 4;
      }
      uVar15 = uVar9 % 3;
      if ((uVar14 & 0x8500000) != 0) {
        param_1 = 0;
      }
      if (((uVar14 & 0xa8d00000) != 0) || ((1 < uVar15 && ((uVar14 & 0x50000000) != 0)))) {
        iVar8 = 0xd;
        if ((uVar14 & 0x900000) == 0) {
          iVar8 = (uint)((uVar14 & 0x8400000) != 0) << 3;
        }
        FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,iVar8);
        FUN_800441c8/*0x800441c8*/(2);
        goto LAB_80109f50;
      }
    }
    if ((uVar14 & 0x900000) != 0) {
      DAT_800658ac = local_34;
      _DAT_80065c04 = local_38;
      _DAT_80065be8 = local_36;
      Audio_SetMasterVolume/*0x80044080*/(local_34 == 0,local_38,local_36);
    }
    if (param_1 == 0) {
LAB_8010a010:
      FUN_800441c8/*0x800441c8*/(2);
    }
    else {
      if ((uVar15 == 1) && ((uVar14 & 0xa8400000) != 0)) {
        FUN_8004445c/*0x8004445c*/(2,DAT_8011338c,0xf);
      }
      if (uVar15 != 1) goto LAB_8010a010;
    }
  } while ((uVar14 & 0x800000) == 0);
  FUN_800190a8/*0x800190a8*/(iVar7);
  func_0x8001a4ac(uVar4);
  FUN_8001af48/*0x8001af48*/(iVar5);
  return;
}

