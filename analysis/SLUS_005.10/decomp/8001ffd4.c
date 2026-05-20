// addr: 0x8001ffd4  name: FUN_8001ffd4

undefined4 FUN_8001ffd4(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_8001ff58(param_1,param_2,0);
  uVar2 = 0;
  if (iVar1 != 0) {
    uVar2 = *(undefined4 *)(iVar1 + 8);
  }
  return uVar2;
}

