// addr: 0x80042e78  name: FUN_80042e78

int FUN_80042e78(ushort *param_1,short *param_2)

{
  if (*(int *)(param_1 + 2) != 0) {
    FUN_80045088();
  }
  *(short **)(param_1 + 2) = param_2;
  param_1[1] = 0;
  *param_1 = (ushort)(param_2 != (short *)0x0);
  if (param_2 != (short *)0x0) {
    *(int *)(param_1 + 4) = (int)*param_2 << 0x10;
    *(int *)(param_1 + 6) = (int)param_2[1] << 0x10;
  }
  return (int)(short)*param_1;
}

