// addr: 0x80042f5c  name: FUN_80042f5c

void FUN_80042f5c(int param_1)

{
  if (*(int *)(param_1 + 4) != 0) {
    FUN_80045088();
    *(undefined4 *)(param_1 + 4) = 0;
  }
  return;
}

