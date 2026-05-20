// addr: 0x8002479c  name: FUN_8002479c

void FUN_8002479c(int *param_1)

{
  int iVar1;
  
  for (; param_1 != (int *)0x0; param_1 = (int *)*param_1) {
    iVar1 = param_1[2] + (uint)*(byte *)(param_1 + 4) * 2;
    *(ushort *)(iVar1 + 2) = *(ushort *)(iVar1 + 2) & 0x9fff;
  }
  return;
}

