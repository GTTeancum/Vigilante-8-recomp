// addr: 0x80106398  name: FUN_80106398

void FUN_80106398(uint *param_1,uint param_2)

{
  if ((param_2 & 1) != 0) {
    param_1 = (uint *)0x1;
  }
  *param_1 = *param_1 | 0x8000000;
  if ((param_2 & 2) != 0) {
    param_1 = (uint *)0x1;
  }
  *param_1 = *param_1 & 0xfdffffff;
  return;
}

