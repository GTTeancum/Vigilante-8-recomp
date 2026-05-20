// addr: 0x80016cec  name: FUN_80016cec

short * FUN_80016cec(short *param_1,short *param_2)

{
  short sVar1;
  MATRIX local_30;
  
  local_30.m[0]._0_4_ = *(undefined4 *)param_1;
  local_30.m._4_4_ = *(undefined4 *)(param_1 + 2);
  local_30.m[1]._2_4_ = *(undefined4 *)(param_1 + 4);
  local_30.m[2]._0_4_ = *(undefined4 *)(param_1 + 6);
  local_30.m[2][2] = param_1[8];
  local_30._18_2_ = param_1[9];
  local_30.t[0] = *(long *)(param_1 + 10);
  local_30.t[1] = *(long *)(param_1 + 0xc);
  local_30.t[2] = *(long *)(param_1 + 0xe);
  sVar1 = FUN_80016c88(&local_30);
  param_2[1] = sVar1;
  RotMatrixY(-(int)sVar1,&local_30);
  sVar1 = FUN_80016c54(&local_30);
  *param_2 = sVar1;
  RotMatrixX(-(int)sVar1,&local_30);
  sVar1 = FUN_80016cb8(&local_30);
  param_2[2] = sVar1;
  return param_2;
}

