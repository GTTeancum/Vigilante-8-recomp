// addr: 0x801061c4  name: FUN_801061c4

void FUN_801061c4(uint *param_1,uint param_2)

{
  *param_1 = *param_1 & 0xff000000 | param_2 & 0xffffff;
  return;
}

