// addr: 0x8010e2b4  name: FUN_8010e2b4

void FUN_8010e2b4(uint *param_1,uint param_2)

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

