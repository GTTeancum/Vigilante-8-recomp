// addr: 0x80016fa8  name: FUN_80016fa8

void FUN_80016fa8(undefined2 *param_1,short *param_2)

{
  undefined4 uVar1;
  VECTOR local_28;
  VECTOR local_18;
  
  local_18.vx = -(int)*param_2;
  local_18.vy = -(int)param_2[1];
  local_18.vz = -(int)param_2[2];
  local_28.vx = -(int)param_2[1];
  local_28.vy = (long)*param_2;
  local_28.vz = 0;
  VectorNormal(&local_28,&local_28);
  VectorNormal(&local_18,&local_18);
  gte_ldR11R12(local_28.vx);
  gte_ldR22R23(local_28.vy);
  gte_ldR33(local_28.vz);
  gte_ldIR3(&local_18.vz);
  gte_ldIR1(&local_18);
  gte_ldIR2(&local_18.vy);
  gte_op12();
  *param_1 = (undefined2)local_28.vx;
  param_1[3] = (undefined2)local_28.vy;
  param_1[6] = (undefined2)local_28.vz;
  param_1[1] = (undefined2)local_18.vx;
  param_1[4] = (undefined2)local_18.vy;
  param_1[7] = (undefined2)local_18.vz;
  uVar1 = gte_stMAC1();
  param_1[2] = (short)uVar1;
  uVar1 = gte_stMAC2();
  param_1[5] = (short)uVar1;
  uVar1 = gte_stMAC3();
  param_1[8] = (short)uVar1;
  return;
}

