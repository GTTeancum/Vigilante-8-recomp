// addr: 0x801024bc  name: FUN_801024bc

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801024bc(void)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  uVar1 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_Legal_bs_801005b8);
  SetDispMask/*0x8004f4e8*/(0);
  FUN_80019e7c/*0x80019e7c*/(0);
  SetDispMask/*0x8004f4e8*/(1,0);
  Heap_Free/*0x80045088*/(uVar1);
  uVar1 = func_0x8001a24c(&DAT_80111d98);
  iVar4 = 0;
  do {
    iVar3 = 0;
    iVar2 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 4),1);
    *(undefined1 *)(iVar2 + 4) = 0x7c;
    *(undefined1 *)(iVar2 + 5) = 0x60;
    *(undefined1 *)(iVar2 + 6) = 0;
    FUN_80019a58/*0x80019a58*/(iVar2,s_PRESS_START_801005c8,&DAT_80111d98,0x4446);
    FUN_800190a8/*0x800190a8*/(iVar2);
    do {
      VSync/*0x80047e44*/(0);
      Tick_PadOnly/*0x800126f0*/();
      if (((_DAT_80065930 | _DAT_80065934) & 0x8000000) != 0) {
        return 1;
      }
      iVar3 = iVar3 + 1;
    } while (iVar3 < 0x2d);
    iVar2 = 0;
    func_0x8001a2ac(uVar1,(int)DAT_80111d98,(int)DAT_80111d9a);
    do {
      VSync/*0x80047e44*/(0);
      Tick_PadOnly/*0x800126f0*/();
      iVar2 = iVar2 + 1;
    } while (iVar2 < 0xf);
    iVar4 = iVar4 + 1;
  } while (iVar4 < 0x1e);
  return 0;
}

