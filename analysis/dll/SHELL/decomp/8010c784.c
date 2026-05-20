// addr: 0x8010c784  name: FUN_8010c784

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined * FUN_8010c784(void)

{
  byte bVar1;
  undefined1 *puVar2;
  uint uVar3;
  int iVar4;
  undefined4 uVar5;
  int extraout_v1;
  int iVar6;
  undefined1 auStack_68 [64];
  
  _DAT_8006532c = 1;
  FUN_8001d994/*0x8001d994*/(0x140,0xf0,0xa0,0x78);
  if (DAT_80065318 == '\0') {
    iVar6 = 0xb;
    puVar2 = &DAT_8006595b;
    do {
      *puVar2 = 1;
      iVar6 = iVar6 + -1;
      puVar2 = puVar2 + -1;
    } while (-1 < iVar6);
    func_0x80011f0c(0);
    if (((_DAT_80065930 | _DAT_80065934) & 0x8000000) == 0) {
      iVar6 = 0;
      uVar3 = _DAT_80065930 | _DAT_80065934;
      while ((uVar3 & 0x8400000) == 0) {
        iVar6 = iVar6 + 1;
        Tick_PadOnly/*0x800126f0*/();
        VSync/*0x80047e44*/(0);
        if (0x77 < iVar6) break;
        uVar3 = _DAT_80065930 | _DAT_80065934;
      }
      func_0x80015fb4();
      if (((_DAT_80065930 | _DAT_80065934) & 0x8000000) == 0) goto LAB_8010c8f8;
    }
    DAT_80065318 = '\x03';
  }
LAB_8010c8f8:
  iVar6 = (int)DAT_80065318;
  iVar4 = 2;
  if (iVar6 < 2) {
    Async_StopAllocCallback/*0x800165cc*/(0);
    iVar4 = -0x7ffa0000;
    iVar6 = extraout_v1;
  }
  if ((iVar6 == iVar4) && ((Async_StopAllocCallback/*0x800165cc*/(0), _DAT_80065930 != 0 || (_DAT_80065934 != 0)))) {
    DAT_80065318 = '\x04';
  }
  if (DAT_80065318 < '\x04') {
    DAT_80113388 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_Resource_tbl_80101024);
    uVar5 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_Cursor_bin_80101038);
    DAT_80113390 = func_0x8001a8fc(uVar5);
    DAT_8011338c = Audio_LoadSND/*0x80044360*/(s_Shell_Sounds_SND_8010104c);
    iVar6 = Async_StopAllocCallback/*0x800165cc*/(0);
    if (iVar6 == 0) {
      sprintf/*0x80053004*/(auStack_68,s_Video_Demo_c_str_80101060,0x31);
    }
    FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,8);
    func_0x80015fb4();
  }
  DAT_80113388 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_Resource_tbl_80101024);
  uVar5 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_Cursor_bin_80101038);
  DAT_80113390 = func_0x8001a8fc(uVar5);
  DAT_8011338c = Audio_LoadSND/*0x80044360*/(s_Shell_Sounds_SND_8010104c);
  uVar5 = 0x80060000;
  if ((DAT_80065318 == '\x04') && (uVar5 = 0x80060000, DAT_80065319 == '\0')) {
    if (_DAT_80065328 != 0) {
      bVar1 = (&DAT_80065950)[DAT_80065674];
      if ((int)DAT_80065904 == 0x1f - (int)(&DAT_80065950 + DAT_80065674)) {
        (&DAT_80065950)[DAT_80065674] =
             bVar1 & ~(byte)(1 << ((int)DAT_80065904 & 0x1fU)) |
             (byte)(2 << ((int)DAT_80065904 & 0x1fU));
        if (1 << (*(uint *)(_DAT_8006590c + DAT_80065674 * 8 + 4) & 0x1f) <=
            (int)(uint)(byte)(&DAT_80065950)[DAT_80065674]) {
          Async_StopAllocCallback/*0x800165cc*/(0);
        }
      }
      if (_DAT_80065924 != 0) {
        (&DAT_80065950)[DAT_80065674] =
             (&DAT_80065950)[DAT_80065674] | (byte)(1 << ((int)DAT_80065904 & 0x1fU));
      }
      if (((uint)(byte)(&DAT_80065950)[DAT_80065674] != (uint)bVar1) &&
         ((uint)(byte)(&DAT_80065950)[DAT_80065674] ==
          (2 << (*(uint *)(_DAT_8006590c + DAT_80065674 * 8 + 4) & 0x1f)) - 1U)) {
        DAT_80065319 = -1;
      }
    }
    uVar5 = 0x80060000;
    if (DAT_80065319 == '\0') {
      DAT_800658f8 = '\0';
      Async_StartLogo/*0x80016678*/(1);
      uVar5 = 0x80110000;
      if ((_DAT_80065930 & 0x8400000) != 0) goto LAB_8010cf58;
    }
  }
  DAT_801133b4 = uVar5;
  func_0x80011be4(0x80065968);
  do {
    uVar3 = Match_LoadConfig/*0x80011c58*/(0x80065968);
    Async_StartLogo/*0x80016678*/(1);
    if (uVar3 < 5) {
      switch(uVar3) {
      case 0:
        DAT_80065319 = '\0';
        _DAT_80065674 = CONCAT11(0xff,(char)(&switchD_8010ccfc::switchdataD_80101078)[uVar3]);
        Async_StartLogo/*0x80016678*/(1,0);
        if ((_DAT_80065930 & 0x900000) == 0) {
          DAT_800658f8 = '\0';
          Async_StartLogo/*0x80016678*/(1);
          if ((_DAT_80065930 & 0x100000) != 0) goto switchD_8010ccfc_caseD_1;
        }
        break;
      case 1:
switchD_8010ccfc_caseD_1:
        DAT_80065319 = '\x01';
        DAT_800658f8 = '\x01';
        Async_StartLogo/*0x80016678*/(1);
        if (((_DAT_80065930 | _DAT_80065934) & 0x900000) == 0) {
          _DAT_80065674 = 0xff00;
          Async_StartLogo/*0x80016678*/(1,1);
          if ((_DAT_80065930 & 0x100000) != 0) goto switchD_8010ccfc_caseD_2;
        }
        break;
      case 2:
switchD_8010ccfc_caseD_2:
        DAT_80065319 = '\x03';
        DAT_800658f8 = '\x03';
        Async_StartLogo/*0x80016678*/(1);
        if (((_DAT_80065930 | _DAT_80065934) & 0x900000) == 0) {
          _DAT_80065674 = 0;
          Async_StartLogo/*0x80016678*/(1);
          if (((_DAT_80065930 | _DAT_80065934) & 0x100000) != 0) goto switchD_8010ccfc_caseD_3;
        }
        break;
      case 3:
switchD_8010ccfc_caseD_3:
        DAT_80065319 = '\x04';
        DAT_800658f8 = '\x04';
        Async_StartLogo/*0x80016678*/(1);
        if (((_DAT_80065930 | _DAT_80065934) & 0x900000) == 0) {
          _DAT_80065674 = 0;
          Async_StartLogo/*0x80016678*/(1);
          if (((_DAT_80065930 | _DAT_80065934) & 0x100000) != 0) goto switchD_8010ccfc_caseD_4;
        }
        break;
      case 4:
switchD_8010ccfc_caseD_4:
        Async_StartLogo/*0x80016678*/(1);
      }
    }
    AsyncList_RecycleHead/*0x80011834*/();
  } while (((_DAT_80065930 | _DAT_80065934) & 0x900000) != 0);
  if (-1 < (int)uVar3) {
    _DAT_8006597c = 0;
    _DAT_80065978 = 0;
  }
LAB_8010cf58:
  FUN_8001aa38/*0x8001aa38*/(DAT_80113390);
  Heap_Free/*0x80045088*/(DAT_80113388);
  Audio_FreeSND/*0x80044394*/(DAT_8011338c);
  _DAT_8006532c = 0;
  DAT_80065318 = 4;
  return (&PTR_s_Terrain_ScrtBase_exp_80111bb0)[DAT_800658f8 * 5];
}

