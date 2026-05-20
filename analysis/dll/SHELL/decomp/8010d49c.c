// addr: 0x8010d49c  name: FUN_8010d49c

/* WARNING: Removing unreachable block (ram,0x8010d4e0) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_8010d49c(undefined4 param_1,undefined4 param_2,uint param_3)

{
  int iVar1;
  
  while ((param_3 == 0 || (Pad_Tick/*0x800120d4*/(1), ((_DAT_80065930 | _DAT_80065934) & param_3) == 0))) {
    VSync/*0x80047e44*/(0);
    iVar1 = SetDispMask/*0x8004f4e8*/(1);
    if (iVar1 < 0) {
      return 1;
    }
  }
  return 1;
}

