// addr: 0x8001a994  name: FUN_8001a994

void FUN_8001a994(int *param_1)

{
  int iVar1;
  int iVar2;
  
  if ((*param_1 != 0) && (iVar1 = 0, 0 < *(int *)(*param_1 + 0x10))) {
    iVar2 = 0xc;
    do {
      FUN_8001884c((int)param_1 + iVar2);
      iVar1 = iVar1 + 1;
      iVar2 = iVar2 + 0xc;
    } while (iVar1 < *(int *)(*param_1 + 0x10));
  }
  return;
}

