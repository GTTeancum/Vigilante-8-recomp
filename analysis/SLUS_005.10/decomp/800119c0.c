// addr: 0x800119c0  name: FUN_800119c0

void FUN_800119c0(int param_1)

{
  puRam0000060c = (u_long *)(&UNK_800664a0 + param_1 * 0x4400);
  iRam00000004 = param_1;
  ClearOTagR(puRam0000060c,0x1000);
  FUN_80011914(param_1);
  return;
}

