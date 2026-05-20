// addr: 0x800441c8  name: FUN_800441c8

void FUN_800441c8(int param_1)

{
  if (param_1 != 0) {
    FUN_80043ff0(0,1 << (param_1 - 1U & 0x1f));
  }
  return;
}

