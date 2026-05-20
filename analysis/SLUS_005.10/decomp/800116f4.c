// addr: 0x800116f4  name: FUN_800116f4

int FUN_800116f4(int param_1)

{
  int iVar1;
  
  iVar1 = FUN_80045004();
  if ((iVar1 == 0) && (param_1 != 0)) {
    DrawSync(0);
    FUN_80011914(1 - iRam00000004);
    FUN_800165cc(1);
    do {
      iVar1 = FUN_80045004(param_1);
      if (iVar1 != 0) {
        return iVar1;
      }
      iVar1 = FUN_80020d3c();
    } while (iVar1 != 0);
    _boot();
    iVar1 = 0;
  }
  return iVar1;
}

