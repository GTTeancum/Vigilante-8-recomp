// addr: 0x80044574  name: FUN_80044574

void FUN_80044574(int param_1,undefined4 param_2)

{
  if (param_1 != 0) {
    *(undefined4 *)(param_1 * 0x10 + 0x1f801bf0) = param_2;
  }
  return;
}

