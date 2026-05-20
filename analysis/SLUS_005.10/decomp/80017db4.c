// addr: 0x80017db4  name: FUN_80017db4

void FUN_80017db4(int param_1)

{
  if (*(int *)(param_1 + 8) - 2U < 2) {
    FUN_80017db4(*(undefined4 *)(param_1 + 0x10));
    FUN_80017db4(*(undefined4 *)(param_1 + 0x14));
  }
  FUN_80045088(param_1);
  return;
}

