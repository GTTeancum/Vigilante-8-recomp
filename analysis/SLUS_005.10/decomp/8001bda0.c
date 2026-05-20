// addr: 0x8001bda0  name: FUN_8001bda0

void FUN_8001bda0(int *param_1,uint param_2)

{
  FUN_8001b49c(param_1,*(ushort *)(*param_1 + (param_2 & 0xffff) * 0x1c + 0x1c) & 0x7ff);
  return;
}

