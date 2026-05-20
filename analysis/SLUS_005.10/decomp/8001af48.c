// addr: 0x8001af48  name: FUN_8001af48

void FUN_8001af48(int param_1)

{
  int iVar1;
  
  while (param_1 != 0) {
    FUN_8001bddc(*(undefined4 *)(param_1 + 0x30));
    FUN_8001af48(*(undefined4 *)(param_1 + 0x38));
    iVar1 = *(int *)(param_1 + 0x34);
    FUN_80045088(param_1);
    param_1 = iVar1;
  }
  return;
}

