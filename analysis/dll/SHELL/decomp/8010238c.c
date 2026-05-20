// addr: 0x8010238c  name: FUN_8010238c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8010238c(void)

{
  undefined4 uVar1;
  int iVar2;
  
  SetDispMask/*0x8004f4e8*/(0);
  FUN_80019e7c/*0x80019e7c*/(0);
  uVar1 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_Slogan_fnt_80100588);
  uVar1 = FUN_80019034/*0x80019034*/(uVar1,0);
  iVar2 = FUN_80017160/*0x80017160*/(1);
  FUN_80019a58/*0x80019a58*/(uVar1,(&PTR_DAT_80111d6c)[(uint)(iVar2 * 0xb) >> 0xf],&DAT_8010059c,2);
  SetDispMask/*0x8004f4e8*/(1,0,0x60,0x70,0x60,0x70,0x1c0,0xa0);
  do {
    Pad_Tick/*0x800120d4*/();
  } while (((_DAT_80065930 | _DAT_80065934) & 0x840) == 0);
  FUN_800190d8/*0x800190d8*/(uVar1);
  return;
}

