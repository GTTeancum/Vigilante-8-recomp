// addr: 0x80051318  name: SYS_OBJ_2114

undefined4 SYS_OBJ_2114(dword *param_1,int param_2)

{
  int iVar1;
  
  iVar1 = param_2 + -1;
  GPU_REG1 = 0x4000000;
  if (param_2 != 0) {
    do {
      GPU_REG0 = *param_1;
      param_1 = param_1 + 1;
      iVar1 = iVar1 + -1;
    } while (iVar1 != -1);
  }
  return 0;
}

