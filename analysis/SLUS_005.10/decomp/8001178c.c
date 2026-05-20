// addr: 0x8001178c  name: FUN_8001178c

int FUN_8001178c(int param_1,undefined4 param_2)

{
  int iVar1;
  
  iVar1 = FUN_800451c0();
  if ((iVar1 == 0) && (param_1 != 0)) {
    DrawSync(0);
    FUN_80011914(1 - iRam00000004);
    FUN_800165cc(1);
    do {
      iVar1 = FUN_800451c0(param_1,param_2);
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

