// addr: 0x8002239c  name: FUN_8002239c

bool FUN_8002239c(undefined4 param_1,int *param_2)

{
  int iVar1;
  bool bVar2;
  
  bVar2 = false;
  if (*(char *)(*param_2 + 4) == '\a') {
    iVar1 = FUN_80022320(param_1,*(undefined2 *)(*param_2 + 0xc));
    bVar2 = iVar1 != 0;
  }
  return bVar2;
}

