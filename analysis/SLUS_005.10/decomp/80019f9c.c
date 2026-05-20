// addr: 0x80019f9c  name: FUN_80019f9c

void FUN_80019f9c(short *param_1,undefined4 param_2)

{
  undefined1 auStack_30 [3];
  undefined1 local_2d;
  undefined1 local_2c;
  undefined1 local_2b;
  undefined1 local_2a;
  undefined1 local_29;
  short local_28;
  short local_26;
  short local_24;
  short local_22;
  short local_20;
  short local_1e;
  short local_1c;
  short local_1a;
  undefined4 local_18;
  
  local_2d = 6;
  local_29 = 0x4c;
  local_2b = (undefined1)((uint)param_2 >> 8);
  local_18 = 0x55555555;
  local_2c = (undefined1)param_2;
  local_2a = (undefined1)((uint)param_2 >> 0x10);
  local_28 = *param_1;
  local_26 = param_1[1];
  local_24 = *param_1 + param_1[2] + -1;
  local_22 = param_1[1];
  local_20 = *param_1 + param_1[2] + -1;
  local_1e = param_1[1] + param_1[3] + -1;
  local_1c = *param_1;
  local_1a = param_1[1] + param_1[3] + -1;
  DrawPrim(auStack_30);
  FUN_80019f44((int)*param_1,(int)param_1[1],(int)*param_1,(int)param_1[1] + (int)param_1[3] + -1,
               param_2);
  return;
}

