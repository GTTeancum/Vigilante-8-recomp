// addr: 0x8001fcb4  name: FUN_8001fcb4

void FUN_8001fcb4(int param_1,undefined2 param_2)

{
  int iVar1;
  
  iVar1 = FUN_8001f9cc(param_1,param_2);
  if ((-1 < iVar1) && (*(int *)(param_1 + 0x38) != 0)) {
    FUN_8001fc38(*(int *)(param_1 + 0x38),param_2);
  }
  return;
}

