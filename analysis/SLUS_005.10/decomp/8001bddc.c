// addr: 0x8001bddc  name: FUN_8001bddc

void FUN_8001bddc(int param_1)

{
  if (param_1 != 0) {
    if (*(int *)(param_1 + iRam00000004 * 4 + 0x1c) != 0) {
      FUN_800118b4();
    }
    if (*(int *)(param_1 + (1 - iRam00000004) * 4 + 0x1c) != 0) {
      FUN_80045088();
    }
    FUN_80045088(param_1);
  }
  return;
}

