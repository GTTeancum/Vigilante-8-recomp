// addr: 0x8010bfa4  name: FUN_8010bfa4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_8010bfa4(void)

{
  int iVar1;
  uint uVar2;
  undefined4 local_28 [2];
  
  iVar1 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4));
  *(undefined1 *)(iVar1 + 4) = 0x32;
  *(undefined1 *)(iVar1 + 5) = 0x73;
  *(undefined1 *)(iVar1 + 6) = 100;
  FUN_80019a58/*0x80019a58*/(iVar1,s_CREDITS_80100f7c,&DAT_80100994,2);
  FUN_800190a8/*0x800190a8*/(iVar1);
  do {
    local_28[0] = FUN_80018124/*0x80018124*/(0x140,0xf0,1,1,1,1);
    local_28[1] = FUN_80018124/*0x80018124*/(0x140,0xf0,1,1,1,1);
    iVar1 = 0xf0;
    do {
      func_0x8001a2ac(local_28[iVar1],0x110,0x60);
      Pad_Tick/*0x800120d4*/();
      iVar1 = 0;
    } while (((_DAT_80065930 | _DAT_80065934) & 0xffff0000) == 0);
    Tick_PadOnly/*0x800126f0*/(1);
    uVar2 = _DAT_80065930 | _DAT_80065934;
    FUN_800183ec/*0x800183ec*/(local_28[0]);
    FUN_800183ec/*0x800183ec*/(local_28[1]);
  } while ((uVar2 & 0x58d00000) == 0);
  FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,0);
  FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,0);
  return 0xffffffff;
}

