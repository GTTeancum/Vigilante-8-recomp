// addr: 0x800204dc  name: FUN_800204dc

void FUN_800204dc(int param_1)

{
  int iVar1;
  
  while (param_1 != 0) {
    FUN_800203fc(param_1);
    FUN_8001bddc(*(undefined4 *)(param_1 + 0x30));
    FUN_800204dc(*(undefined4 *)(param_1 + 0x38));
    iVar1 = *(int *)(param_1 + 0x34);
    FUN_80045088(param_1);
    param_1 = iVar1;
  }
  return;
}

