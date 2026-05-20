// addr: 0x800225d4  name: FUN_800225d4

undefined4 FUN_800225d4(void)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  
  iVar1 = FUN_80022524();
  if (iVar1 == -1) {
    uVar2 = 0;
  }
  else {
    iVar3 = 4;
    if (iVar1 != 0) {
      iVar3 = iVar1;
    }
    uVar2 = FUN_800116f4(iVar3);
    FUN_80015a20(uVar2,iVar1);
  }
  return uVar2;
}

